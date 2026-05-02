#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "Shader.h"
#include "Camera.h"
#include "Mesh.h"
#include "OBJLoader.h"

enum class ShadingMode { Flat, Gouraud, Phong };

struct LoadedModel {
    std::string path;
    std::unique_ptr<Mesh> mesh;
    glm::mat4 transform = glm::mat4(1.0f);
};

struct PendingModel {
    std::string path;
    MeshData meshData;
};

struct Bounds {
    glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());
    bool valid = false;
};

struct AppState {
    Camera camera;
    ShadingMode shadingMode = ShadingMode::Phong;
    bool wireframe = false;

    glm::vec3 lightPos   = glm::vec3(2.0f, 3.0f, 2.0f);
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 objectColor = glm::vec3(0.72f, 0.53f, 0.34f);

    // mouse state
    bool leftDown  = false;
    bool rightDown = false;
    double lastX = 0.0;
    double lastY = 0.0;

    int fbWidth  = 800;
    int fbHeight = 600;
};

// global app
static AppState* g_app = nullptr;

// Updates the OpenGL viewport and stored framebuffer size after window resizing.
static void onFramebufferResize(GLFWwindow* /*w*/, int w, int h) {
    glViewport(0, 0, w, h);
    g_app->fbWidth  = w;
    g_app->fbHeight = h;
}

// Tracks mouse button state so dragging can orbit or pan the camera.
static void onMouseButton(GLFWwindow* /*w*/, int button, int action, int /*mods*/) {
    if (button == GLFW_MOUSE_BUTTON_LEFT)
        g_app->leftDown = (action == GLFW_PRESS);
    if (button == GLFW_MOUSE_BUTTON_RIGHT || button == GLFW_MOUSE_BUTTON_MIDDLE)
        g_app->rightDown = (action == GLFW_PRESS);
}

// Converts mouse movement into camera orbiting or panning while a button is held.
static void onCursorMove(GLFWwindow* /*w*/, double xpos, double ypos) {
    double dx = xpos - g_app->lastX;
    double dy = ypos - g_app->lastY;
    g_app->lastX = xpos;
    g_app->lastY = ypos;

    if (g_app->leftDown)
        g_app->camera.orbit(static_cast<float>(dx) * 0.005f,
                            static_cast<float>(dy) * 0.005f);
    if (g_app->rightDown)
        g_app->camera.pan(static_cast<float>(dx), static_cast<float>(dy));
}

// Uses scroll wheel movement to zoom the camera toward or away from the target.
static void onScroll(GLFWwindow* /*w*/, double /*xoff*/, double yoff) {
    g_app->camera.zoom(static_cast<float>(yoff));
}

// Handles keyboard shortcuts for shading, projection, wireframe, light, and reset.
static void onKey(GLFWwindow* window, int key, int /*scan*/, int action, int /*mods*/) {
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

    const float lightStep = 0.15f;
    switch (key) {
        case GLFW_KEY_1: g_app->shadingMode = ShadingMode::Flat;    break;
        case GLFW_KEY_2: g_app->shadingMode = ShadingMode::Gouraud; break;
        case GLFW_KEY_3: g_app->shadingMode = ShadingMode::Phong;   break;

        case GLFW_KEY_W:
            g_app->wireframe = !g_app->wireframe;
            break;

        case GLFW_KEY_P:
            g_app->camera.projType =
                (g_app->camera.projType == ProjectionType::Perspective)
                    ? ProjectionType::Orthographic
                    : ProjectionType::Perspective;
            break;

        // move light
        case GLFW_KEY_LEFT:  g_app->lightPos.x -= lightStep; break;
        case GLFW_KEY_RIGHT: g_app->lightPos.x += lightStep; break;
        case GLFW_KEY_UP:    g_app->lightPos.z -= lightStep; break;
        case GLFW_KEY_DOWN:  g_app->lightPos.z += lightStep; break;
        case GLFW_KEY_EQUAL: g_app->lightPos.y += lightStep; break;
        case GLFW_KEY_MINUS: g_app->lightPos.y -= lightStep; break;

        case GLFW_KEY_R: g_app->camera.reset(); break;
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
        default: break;
    }
}

// Refreshes the window title with the current render mode and shortcut summary.
static void updateWindowTitle(GLFWwindow* window, const AppState& app) {
    const char* shadeName = (app.shadingMode == ShadingMode::Flat)    ? "Flat"
                          : (app.shadingMode == ShadingMode::Gouraud) ? "Gouraud"
                                                                       : "Phong";
    const char* projName = (app.camera.projType == ProjectionType::Perspective)
                         ? "Perspective" : "Orthographic";
    const char* wireName = app.wireframe ? "ON" : "OFF";

    char title[256];
    std::snprintf(title, sizeof(title),
        "Model Viewer  |  Shading: %s  |  Projection: %s  |  Wireframe: %s"
        "  |  [1/2/3] Shade  [W] Wire  [P] Proj  [Arrows/+-] Light  [R] Reset",
        shadeName, projName, wireName);
    glfwSetWindowTitle(window, title);
}

// Sends shared transform, camera, material, and light uniforms to a shader.
static void uploadLightUniforms(Shader& shader, const AppState& app,
                                const glm::mat4& model,
                                const glm::mat4& view,
                                const glm::mat4& proj) {
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));

    shader.use();
    shader.setMat4("model",        model);
    shader.setMat4("view",         view);
    shader.setMat4("projection",   proj);
    shader.setMat3("normalMatrix", normalMatrix);
    shader.setVec3("lightPos",     app.lightPos);
    shader.setVec3("lightColor",   app.lightColor);
    shader.setVec3("objectColor",  app.objectColor);
    shader.setVec3("viewPos",      app.camera.getPosition());
}

// Returns a lowercase copy of text for case-insensitive file extension checks.
static std::string lowercase(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

// Collects one OBJ file path or all OBJ file paths in a directory.
static std::vector<std::string> collectObjPaths(const std::string& inputPath) {
    namespace fs = std::filesystem;

    std::vector<std::string> paths;
    fs::path path(inputPath);

    if (fs::is_directory(path)) {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (!entry.is_regular_file()) continue;

            std::string extension = lowercase(entry.path().extension().string());
            if (extension == ".obj")
                paths.push_back(entry.path().string());
        }
        std::sort(paths.begin(), paths.end());
    } else {
        paths.push_back(inputPath);
    }

    return paths;
}

// Expands a bounding box so it includes one more point.
static void includePoint(Bounds& bounds, const glm::vec3& point) {
    bounds.min = glm::min(bounds.min, point);
    bounds.max = glm::max(bounds.max, point);
    bounds.valid = true;
}

// Adds all vertices from a parsed mesh to the scene bounds.
static void includeMeshData(Bounds& bounds, const MeshData& meshData) {
    for (const auto& vertex : meshData.smoothVertices)
        includePoint(bounds, vertex.position);

    if (meshData.smoothVertices.empty()) {
        for (const auto& vertex : meshData.flatVertices)
            includePoint(bounds, vertex.position);
    }
}

// Builds a transform that centers and scales the whole loaded scene into view.
static glm::mat4 makeSceneTransform(const Bounds& bounds) {
    if (!bounds.valid)
        return glm::mat4(1.0f);

    glm::vec3 center = (bounds.min + bounds.max) * 0.5f;
    glm::vec3 size = bounds.max - bounds.min;
    float maxExtent = std::max({ size.x, size.y, size.z });
    float scale = (maxExtent > 1e-6f) ? (2.0f / maxExtent) : 1.0f;

    return glm::scale(glm::mat4(1.0f), glm::vec3(scale))
         * glm::translate(glm::mat4(1.0f), -center);
}

// Prints startup controls and the model input summary to the terminal.
static void printInstructions(const std::string& inputPath, std::size_t modelCount) {
    std::cout
        << "Model Viewer\n"
        << "============\n"
        << "Input: " << inputPath << "\n"
        << "Loaded OBJ files: " << modelCount << "\n\n"
        << "Folder inputs preserve OBJ positions and scale the whole scene together.\n\n"
        << "Mouse:\n"
        << "  Left click + drag       Orbit camera around model\n"
        << "  Right click + drag      Pan camera\n"
        << "  Scroll wheel            Zoom in / out\n\n"
        << "Keyboard:\n"
        << "  1                       Flat shading\n"
        << "  2                       Gouraud shading\n"
        << "  3                       Phong shading\n"
        << "  W                       Toggle wireframe overlay\n"
        << "  P                       Switch projection mode\n"
        << "  R                       Reset camera\n"
        << "  Arrow keys              Move light left / right / forward / back\n"
        << "  = / -                   Move light up / down\n"
        << "  ESC                     Quit\n\n";
}

// Program entry point: loads OBJ input, initializes OpenGL, and runs the render loop.
int main(int argc, char* argv[]) {
    std::string inputPath = (argc >= 2) ? argv[1] : "models/cube.obj";
    std::vector<std::string> objPaths;

    try {
        objPaths = collectObjPaths(inputPath);
    } catch (const std::exception& e) {
        std::cerr << "Failed to read model path: " << inputPath << "\n"
                  << e.what() << "\n";
        return 1;
    }

    if (objPaths.empty()) {
        std::cerr << "No .obj files found in: " << inputPath << "\n"
                  << "Usage: ModelViewer [path/to/model.obj | path/to/folder]\n";
        return 1;
    }

    printInstructions(inputPath, objPaths.size());

    if (!glfwInit()) {
        std::cerr << "Failed to initialise GLFW\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(1024, 768, "Model Viewer", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialise GLEW\n";
        glfwTerminate();
        return 1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    AppState app;
    g_app = &app;

    glfwSetFramebufferSizeCallback(window, onFramebufferResize);
    glfwSetMouseButtonCallback(window, onMouseButton);
    glfwSetCursorPosCallback(window, onCursorMove);
    glfwSetScrollCallback(window, onScroll);
    glfwSetKeyCallback(window, onKey);

    {
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        app.fbWidth = w; app.fbHeight = h;
        glfwGetCursorPos(window, &app.lastX, &app.lastY);
    }

    // load shaders
    std::unique_ptr<Shader> flatShader, gouraudShader, phongShader, wireShader;
    try {
        flatShader    = std::make_unique<Shader>("shaders/flat.vert",      "shaders/flat.frag");
        gouraudShader = std::make_unique<Shader>("shaders/gouraud.vert",   "shaders/gouraud.frag");
        phongShader   = std::make_unique<Shader>("shaders/phong.vert",     "shaders/phong.frag");
        wireShader    = std::make_unique<Shader>("shaders/wireframe.vert", "shaders/wireframe.frag");
    } catch (const std::exception& e) {
        std::cerr << "Shader error: " << e.what() << "\n";
        glfwTerminate();
        return 1;
    }

    // load models
    std::vector<PendingModel> pendingModels;
    pendingModels.reserve(objPaths.size());
    Bounds sceneBounds;

    for (const auto& objPath : objPaths) {
        MeshData meshData;
        if (!loadOBJ(objPath, meshData, false)) {
            std::cerr << "Failed to load model: " << objPath << "\n"
                      << "Usage: ModelViewer [path/to/model.obj | path/to/folder]\n";
            glfwTerminate();
            return 1;
        }

        includeMeshData(sceneBounds, meshData);
        pendingModels.push_back({ objPath, std::move(meshData) });
    }

    std::vector<LoadedModel> models;
    models.reserve(pendingModels.size());
    glm::mat4 sceneTransform = makeSceneTransform(sceneBounds);

    for (auto& pendingModel : pendingModels) {
        auto mesh = std::make_unique<Mesh>();
        mesh->uploadFlat(pendingModel.meshData.flatVertices);
        mesh->uploadSmooth(pendingModel.meshData.smoothVertices, pendingModel.meshData.smoothIndices);

        models.push_back({ pendingModel.path, std::move(mesh), sceneTransform });
    }

    // render loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glViewport(0, 0, app.fbWidth, app.fbHeight);
        glClearColor(0.13f, 0.14f, 0.16f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float aspect = (app.fbHeight > 0)
            ? static_cast<float>(app.fbWidth) / static_cast<float>(app.fbHeight)
            : 1.0f;

        glm::mat4 view  = app.camera.getView();
        glm::mat4 proj  = app.camera.getProjection(aspect);

        Shader* activeShader = nullptr;
        switch (app.shadingMode) {
            case ShadingMode::Flat:    activeShader = flatShader.get();    break;
            case ShadingMode::Gouraud: activeShader = gouraudShader.get(); break;
            case ShadingMode::Phong:   activeShader = phongShader.get();   break;
        }

        for (const auto& model : models) {
            uploadLightUniforms(*activeShader, app, model.transform, view, proj);

            if (app.shadingMode == ShadingMode::Flat)
                model.mesh->drawFlat();
            else
                model.mesh->drawSmooth();

            // wireframe overlay
            if (app.wireframe) {
                wireShader->use();
                wireShader->setMat4("model",      model.transform);
                wireShader->setMat4("view",       view);
                wireShader->setMat4("projection", proj);
                wireShader->setVec3("wireColor",  glm::vec3(1.0f, 1.0f, 1.0f));

                glEnable(GL_POLYGON_OFFSET_LINE);
                glPolygonOffset(-1.0f, -1.0f);

                if (app.shadingMode == ShadingMode::Flat)
                    model.mesh->drawFlatWireframe();
                else
                    model.mesh->drawSmoothWireframe();

                glDisable(GL_POLYGON_OFFSET_LINE);
            }
        }

        updateWindowTitle(window, app);
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
