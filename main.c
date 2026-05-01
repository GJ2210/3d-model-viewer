#define GL_SILENCE_DEPRECATION

#include "model.h"
#include "renderer.h"

#include <GLFW/glfw3.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define MODEL_DIR "models"
#define MODEL_DIR_PREFIX MODEL_DIR "/"
#define MODEL_DIR_FRAGMENT "/" MODEL_DIR "/"
#define APP_TITLE "3D Model Viewer"

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

/* Prints GLFW errors with their numeric code and description. */
static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

/* Returns true when the path points to a Wavefront OBJ file. */
static int has_obj_extension(const char *path)
{
    const char *extension = strrchr(path, '.');
    return extension && strcasecmp(extension, ".obj") == 0;
}

/* Finds the first OBJ file in the models directory. */
static int find_first_model_file(char *path, size_t path_size)
{
    DIR *directory = opendir(MODEL_DIR);
    if (!directory) {
        fprintf(
            stderr,
            "Could not open %s/ folder: %s\n"
            "Run the app from the main project folder and make sure %s/ exists.\n",
            MODEL_DIR,
            strerror(errno),
            MODEL_DIR);
        return 0;
    }

    struct dirent *entry = NULL;
    int found = 0;
    while ((entry = readdir(directory)) != NULL) {
        if (entry->d_name[0] == '.' || !has_obj_extension(entry->d_name)) {
            continue;
        }

        snprintf(path, path_size, "%s/%s", MODEL_DIR, entry->d_name);
        found = 1;
        break;
    }

    closedir(directory);
    return found;
}

/* Opens the macOS file picker and stores the selected OBJ path. */
static int choose_obj_file(char *path, size_t path_size)
{
    const char *command =
        "osascript "
        "-e 'set modelFolder to POSIX file ((do shell script \"pwd\") & \"/" MODEL_DIR "/\")' "
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
    return path[0] != '\0';
}

/* Returns true when a path already points inside models/. */
static int is_in_model_folder(const char *path)
{
    return strncmp(path, MODEL_DIR_PREFIX, strlen(MODEL_DIR_PREFIX)) == 0 ||
        strstr(path, MODEL_DIR_FRAGMENT) != NULL;
}

/* Updates the window title with projection mode and loaded model name. */
static void update_window_title(GLFWwindow *window)
{
    char title[PATH_BUFFER_SIZE + 160];
    snprintf(
        title,
        sizeof(title),
        APP_TITLE " - %s - %s",
        projection_mode_name(app.projection_mode),
        app.model.source_path[0] ? app.model.source_path : "No model");
    glfwSetWindowTitle(window, title);
}

/* Loads a model selected through the file picker. */
static void load_model_with_dialog(GLFWwindow *window)
{
    char path[PATH_BUFFER_SIZE];

    if (!choose_obj_file(path, sizeof(path))) {
        return;
    }

    if (!has_obj_extension(path)) {
        fprintf(stderr, "Invalid file type: %s\nOnly .obj files can be loaded.\n", path);
        return;
    }

    if (!is_in_model_folder(path)) {
        fprintf(
            stderr,
            "Model rejected: %s\nChoose an .obj file from the %s/ folder.\n",
            path,
            MODEL_DIR);
        return;
    }

    if (load_obj_model(path, &app.model)) {
        update_window_title(window);
    } else {
        fprintf(stderr, "Failed to load selected model: %s\n", path);
    }
}

/* Switches between perspective and orthographic projection. */
static void toggle_projection(GLFWwindow *window)
{
    app.projection_mode = app.projection_mode == PROJECTION_PERSPECTIVE
        ? PROJECTION_ORTHOGRAPHIC
        : PROJECTION_PERSPECTIVE;
    printf("Projection: %s\n", projection_mode_name(app.projection_mode));
    update_window_title(window);
}

/* Handles keyboard shortcuts for loading, projection, wireframe, and quit. */
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    (void)scancode;
    (void)mods;

    if (action != GLFW_PRESS) {
        return;
    }

    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    } else if (key == GLFW_KEY_L) {
        load_model_with_dialog(window);
    } else if (key == GLFW_KEY_P) {
        toggle_projection(window);
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
        glfwGetCursorPos(window, &app.last_mouse_x, &app.last_mouse_y);
    }
}

/* Updates camera yaw and pitch while the left mouse button is dragged. */
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

/* Prints startup instructions and controls to the terminal. */
static void print_help(void)
{
    puts(APP_TITLE);
    printf("Run this app from the main project folder that contains Makefile, main.c, and %s/.\n", MODEL_DIR);
    puts("Controls:");
    printf("  L       Load an OBJ model from %s/\n", MODEL_DIR);
    puts("  Drag    Orbit camera");
    puts("  Scroll  Zoom");
    puts("  P       Toggle perspective/orthographic projection");
    puts("  W       Toggle wireframe");
    puts("  Esc     Quit");
}

/* Initializes the app, loads a model, and runs the render loop. */
int main(int argc, char **argv)
{
    glfwSetErrorCallback(glfw_error_callback);

    print_help();
    model_init(&app.model);
    app.projection_mode = PROJECTION_PERSPECTIVE;
    app.yaw = -35.0f;
    app.pitch = 20.0f;
    app.distance = 5.0f;

    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW. Check that GLFW is installed and linked correctly.\n");
        return EXIT_FAILURE;
    }

    GLFWwindow *window = glfwCreateWindow(1000, 700, APP_TITLE, NULL, NULL);
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
        snprintf(model_path, sizeof(model_path), "%s%s", MODEL_DIR_PREFIX, filename);

        if (!has_obj_extension(model_path)) {
            fprintf(stderr, "Invalid file type: %s\nOnly .obj files can be loaded.\n", model_path);
        } else if (!load_obj_model(model_path, &app.model)) {
            fprintf(stderr, "Failed to load requested model: %s\n", model_path);
        }
    } else {
        char model_path[PATH_BUFFER_SIZE];
        if (find_first_model_file(model_path, sizeof(model_path)) &&
            !load_obj_model(model_path, &app.model)) {
            fprintf(stderr, "Found a model file but could not load it: %s\n", model_path);
            fprintf(stderr, "Try another .obj file from the %s/ folder.\n", MODEL_DIR);
        } else if (!app.model.source_path[0]) {
            printf("No .obj files found in %s/.\n", MODEL_DIR);
            printf("Make sure you ran make run from the main project folder, then add an .obj file to %s/.\n", MODEL_DIR);
        }
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
