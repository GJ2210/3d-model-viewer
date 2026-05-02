#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <memory>
#include <string>

#include "Shader.h"
#include "Camera.h"
#include "Mesh.h"
#include "OBJLoader.h"

enum class ShadingMode { Flat, Gouraud, Phong };

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

static void onFramebufferResize(GLFWwindow* /*w*/, int w, int h) {
    glViewport(0, 0, w, h);
    g_app->fbWidth  = w;
    g_app->fbHeight = h;
}

static void onMouseButton(GLFWwindow* /*w*/, int button, int action, int /*mods*/) {
    if (button == GLFW_MOUSE_BUTTON_LEFT)
        g_app->leftDown = (action == GLFW_PRESS);
    if (button == GLFW_MOUSE_BUTTON_RIGHT || button == GLFW_MOUSE_BUTTON_MIDDLE)
        g_app->rightDown = (action == GLFW_PRESS);
}

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

static void onScroll(GLFWwindow* /*w*/, double /*xoff*/, double yoff) {
    g_app->camera.zoom(static_cast<float>(yoff));
}

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

static void printInstructions(const std::string& objPath) {
    std::cout
        << "Model Viewer\n"
        << "============\n"
        << "Loaded model: " << objPath << "\n\n"
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

int main(int argc, char* argv[]) {
    std::string objPath = (argc >= 2) ? argv[1] : "models/cube.obj";
    printInstructions(objPath);

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

    // load model
    MeshData meshData;
    if (!loadOBJ(objPath, meshData)) {
        std::cerr << "Failed to load model: " << objPath << "\n"
                  << "Usage: ModelViewer [path/to/model.obj]\n";
        glfwTerminate();
        return 1;
    }

    Mesh mesh;
    mesh.uploadFlat(meshData.flatVertices);
    mesh.uploadSmooth(meshData.smoothVertices, meshData.smoothIndices);

    // render loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glViewport(0, 0, app.fbWidth, app.fbHeight);
        glClearColor(0.13f, 0.14f, 0.16f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float aspect = (app.fbHeight > 0)
            ? static_cast<float>(app.fbWidth) / static_cast<float>(app.fbHeight)
            : 1.0f;

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view  = app.camera.getView();
        glm::mat4 proj  = app.camera.getProjection(aspect);

        Shader* activeShader = nullptr;
        switch (app.shadingMode) {
            case ShadingMode::Flat:    activeShader = flatShader.get();    break;
            case ShadingMode::Gouraud: activeShader = gouraudShader.get(); break;
            case ShadingMode::Phong:   activeShader = phongShader.get();   break;
        }
        uploadLightUniforms(*activeShader, app, model, view, proj);

        if (app.shadingMode == ShadingMode::Flat)
            mesh.drawFlat();
        else
            mesh.drawSmooth();

        // wireframe overlay
        if (app.wireframe) {
            wireShader->use();
            wireShader->setMat4("model",      model);
            wireShader->setMat4("view",       view);
            wireShader->setMat4("projection", proj);
            wireShader->setVec3("wireColor",  glm::vec3(1.0f, 1.0f, 1.0f));

            glEnable(GL_POLYGON_OFFSET_LINE);
            glPolygonOffset(-1.0f, -1.0f);

            if (app.shadingMode == ShadingMode::Flat)
                mesh.drawFlatWireframe();
            else
                mesh.drawSmoothWireframe();

            glDisable(GL_POLYGON_OFFSET_LINE);
        }

        updateWindowTitle(window, app);
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
