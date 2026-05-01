#define GL_SILENCE_DEPRECATION

#include "model.h"
#include "renderer.h"

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    Model model;
    ProjectionMode projection_mode;
    float yaw;
    float pitch;
    float distance;
    int is_dragging;
    int show_wireframe;
    double last_mouse_x;
    double last_mouse_y;
} AppState;

static AppState app = {0};

static int choose_obj_file(char *path, size_t path_size)
{
    const char *command =
        "osascript -e 'POSIX path of (choose file of type {\"obj\"} "
        "with prompt \"Choose an OBJ model to load\")' 2>/dev/null";

    FILE *pipe = popen(command, "r");
    if (!pipe) {
        return 0;
    }

    if (!fgets(path, (int)path_size, pipe)) {
        pclose(pipe);
        return 0;
    }

    int status = pclose(pipe);
    if (status != 0) {
        return 0;
    }

    path[strcspn(path, "\r\n")] = '\0';
    return path[0] != '\0';
}

static void update_window_title(GLFWwindow *window)
{
    char title[PATH_BUFFER_SIZE + 160];
    snprintf(
        title,
        sizeof(title),
        "3D Model Viewer - %s - %s",
        projection_mode_name(app.projection_mode),
        app.model.source_path[0] ? app.model.source_path : "No model");
    glfwSetWindowTitle(window, title);
}

static void reset_camera(void)
{
    app.yaw = -35.0f;
    app.pitch = 20.0f;
    app.distance = 5.0f;
}

static void load_model_with_dialog(GLFWwindow *window)
{
    char path[PATH_BUFFER_SIZE];
    if (choose_obj_file(path, sizeof(path)) && load_obj_model(path, &app.model)) {
        reset_camera();
        update_window_title(window);
    }
}

static void toggle_projection(GLFWwindow *window)
{
    app.projection_mode = app.projection_mode == PROJECTION_PERSPECTIVE
        ? PROJECTION_ORTHOGRAPHIC
        : PROJECTION_PERSPECTIVE;
    printf("Projection: %s\n", projection_mode_name(app.projection_mode));
    update_window_title(window);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    (void)scancode;
    (void)mods;

    if (action != GLFW_PRESS) {
        return;
    }

    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    } else if (key == GLFW_KEY_L || key == GLFW_KEY_O) {
        load_model_with_dialog(window);
    } else if (key == GLFW_KEY_P) {
        toggle_projection(window);
    } else if (key == GLFW_KEY_R) {
        reset_camera();
    } else if (key == GLFW_KEY_W) {
        app.show_wireframe = !app.show_wireframe;
    }
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    (void)mods;

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        app.is_dragging = action == GLFW_PRESS;
        glfwGetCursorPos(window, &app.last_mouse_x, &app.last_mouse_y);
    }
}

static void cursor_position_callback(GLFWwindow *window, double x, double y)
{
    (void)window;

    if (!app.is_dragging) {
        return;
    }

    double dx = x - app.last_mouse_x;
    double dy = y - app.last_mouse_y;
    app.yaw += (float)dx * 0.35f;
    app.pitch += (float)dy * 0.35f;

    if (app.pitch > 89.0f) {
        app.pitch = 89.0f;
    } else if (app.pitch < -89.0f) {
        app.pitch = -89.0f;
    }

    app.last_mouse_x = x;
    app.last_mouse_y = y;
}

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

static void print_help(void)
{
    puts("3D Model Viewer");
    puts("Controls:");
    puts("  L or O  Load an OBJ model");
    puts("  Drag    Orbit camera");
    puts("  Scroll  Zoom");
    puts("  P       Toggle perspective/orthographic projection");
    puts("  W       Toggle wireframe");
    puts("  R       Reset camera");
    puts("  Esc     Quit");
}

int main(int argc, char **argv)
{
    print_help();
    model_init(&app.model);
    create_demo_model(&app.model);
    app.projection_mode = PROJECTION_PERSPECTIVE;
    reset_camera();

    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW.\n");
        return EXIT_FAILURE;
    }

    GLFWwindow *window = glfwCreateWindow(1000, 700, "3D Model Viewer", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window.\n");
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
        load_obj_model(argv[1], &app.model);
    }
    update_window_title(window);

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
            app.projection_mode);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    model_free(&app.model);
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
