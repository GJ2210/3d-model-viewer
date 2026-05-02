#include <GLFW/glfw3.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "model.h"
#include "renderer.h"
#include "shading.h"

typedef struct {
    Model model;
    ProjectionMode projection_mode;
    ShadingMode shading_mode;
    float yaw;
    float pitch;
    float distance;
    int is_dragging;
    int show_wireframe;
    double mouse_x;
    double mouse_y;
} AppState;

static AppState app = {0};

/* Opens the macOS file picker and stores a validated OBJ path. */
static int choose_obj_file(char *path, size_t path_size)
{
    const char *command =
        "osascript "
        "-e 'set modelFolder to POSIX file ((do shell script \"pwd\") & \"/" "models" "/\")' "
        "-e 'POSIX path of (choose file of type {\"obj\"} default location modelFolder "
        "with prompt \"Choose an OBJ model from the models folder\")' 2>/dev/null";

    FILE *pipe = popen(command, "r");
    if (!pipe) {
        fprintf(stderr, "Could not open macOS file picker: %s\n", strerror(errno));
        return 0;
    }

    if (!fgets(path, (int)path_size, pipe)) {
        pclose(pipe);
        fprintf(stderr, "No model selected from the file picker.\n");
        return 0;
    }

    int status = pclose(pipe);
    if (status != 0) {
        fprintf(stderr, "File picker did not return a usable .obj file.\n");
        return 0;
    }

    path[strcspn(path, "\r\n")] = '\0';

    if (path[0] == '\0') {
        fprintf(stderr, "File picker returned an empty path.\n");
        return 0;
    }

    return 1;
}

/* Loads a model selected through the file picker. */
static void load_model(GLFWwindow *window)
{
    (void)window;

    char path[PATH_BUFFER_SIZE];

    if (!choose_obj_file(path, sizeof(path))) {
        return;
    }

    if (!load_obj_model(path, &app.model)) {
        fprintf(stderr, "Failed to load selected model: %s\n", path);
    }
}

/* Switches between perspective and orthographic projection. */
static void toggle_projection(GLFWwindow *window)
{
    (void)window;

    app.projection_mode = app.projection_mode == PROJECTION_PERSPECTIVE
        ? PROJECTION_ORTHOGRAPHIC
        : PROJECTION_PERSPECTIVE;
    printf("Projection: %s\n", projection_mode_name(app.projection_mode));
}

/* Cycles through flat, Gouraud, and Phong-style shading modes. */
static void toggle_shading(GLFWwindow *window)
{
    (void)window;

    app.shading_mode = next_shading_mode(app.shading_mode);
    printf("Shading: %s\n", shading_mode_name(app.shading_mode));
}

/* Handles keyboard shortcuts for loading, projection, wireframe, and quit. */
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    (void)scancode;
    (void)mods;

    if (action != GLFW_PRESS) {
        return;
    }

    if (key == GLFW_KEY_Q) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    } else if (key == GLFW_KEY_L) {
        load_model(window);
    } else if (key == GLFW_KEY_P) {
        toggle_projection(window);
    } else if (key == GLFW_KEY_S) {
        toggle_shading(window);
    } else if (key == GLFW_KEY_W) {
        app.show_wireframe = !app.show_wireframe;
    }
}

/* Starts or stops camera orbit dragging. */
static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    (void)mods;

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        app.is_dragging = action == GLFW_PRESS;
        glfwGetCursorPos(window, &app.mouse_x, &app.mouse_y);
    }
}

/* Updates camera yaw and pitch while the left mouse button is dragged. */
static void cursor_position_callback(GLFWwindow *window, double x, double y)
{
    (void)window;

    if (!app.is_dragging) {
        return;
    }

    double dx = x - app.mouse_x;
    double dy = y - app.mouse_y;
    app.yaw += (float)dx * 0.35f;
    app.pitch += (float)dy * 0.35f;

    if (app.pitch > 89.0f) {
        app.pitch = 89.0f;
    } else if (app.pitch < -89.0f) {
        app.pitch = -89.0f;
    }

    app.mouse_x = x;
    app.mouse_y = y;
}

/* Zooms the camera in or out using the mouse wheel. */
static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    (void)window;
    (void)xoffset;

    app.distance -= (float)yoffset * 0.35f;
    if (app.distance < 1.4f) {
        app.distance = 1.4f;
    } else if (app.distance > 30.0f) {
        app.distance = 30.0f;
    }
}

/* Prints GLFW errors with their numeric code and description. */
static void error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

/* Initializes the app, loads a model, and runs the render loop. */
int main(int argc, char **argv)
{
    glfwSetErrorCallback(error_callback);

    puts("3D Model Viewer");
    printf("Run this app from the main project folder\n");
    printf("Controls:");
    printf("  L      Load an OBJ model\n");
    printf("  Drag    Orbit camera\n");
    printf("  Scroll  Zoom\n");
    printf("  P       Toggle perspective/orthographic projection\n");
    printf("  S       Toggle flat/Gouraud/Phong shading\n");
    printf("  W       Toggle wireframe\n");
    printf("  Q       Quit\n");

    model_init(&app.model);
    app.projection_mode = PROJECTION_PERSPECTIVE;
    app.shading_mode = SHADING_FLAT;
    app.yaw = -35.0f;
    app.pitch = 20.0f;
    app.distance = 5.0f;

    if (!glfwInit()) {
        return EXIT_FAILURE;
    }

    GLFWwindow *window = glfwCreateWindow(1000, 700, "3D Model Viewer", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window. Your system may not support the requested OpenGL context.\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    if (argc > 1) {
        char model_path[PATH_BUFFER_SIZE];
        const char *slash = strrchr(argv[1], '/');
        const char *filename = slash ? slash + 1 : argv[1];
        snprintf(model_path, sizeof(model_path), "%s%s", "models/", filename);

        const char *extension = strrchr(model_path, '.');
        if (!extension || strcasecmp(extension, ".obj") != 0) {
            fprintf(stderr, "Invalid file type: %s\nOnly .obj files can be loaded.\n", model_path);
        } else if (!load_obj_model(model_path, &app.model)) {
            fprintf(stderr, "Failed to load requested model: %s\n", model_path);
        }
        } else {
            fprintf(
                stderr,
                "No model file was provided.\n"
                "press L to choose an OBJ file\n");
        }

    while (!glfwWindowShouldClose(window)) {
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(window, &width, &height);

        render_scene(
            &app.model,
            width,
            height,
            app.yaw,
            app.pitch,
            app.distance,
            app.show_wireframe,
            app.projection_mode,
            app.shading_mode);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    model_free(&app.model);
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
