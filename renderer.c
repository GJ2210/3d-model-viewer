#include "renderer.h"

#include <GLFW/glfw3.h>

/* Returns a readable name for the active projection mode. */
const char *projection_mode_name(ProjectionMode mode)
{
    return mode == PROJECTION_ORTHOGRAPHIC ? "orthographic" : "perspective";
}

/* Draws a simple ground grid for spatial reference. */
static void draw_grid(void)
{
    glDisable(GL_LIGHTING);
    glColor3f(0.24f, 0.26f, 0.29f);
    glBegin(GL_LINES);
    for (int i = -10; i <= 10; ++i) {
        glVertex3f((float)i, -1.0f, -10.0f);
        glVertex3f((float)i, -1.0f, 10.0f);
        glVertex3f(-10.0f, -1.0f, (float)i);
        glVertex3f(10.0f, -1.0f, (float)i);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

/* Draws the loaded model with its stored vertices and normals. */
static void draw_model(const Model *model)
{
    glPushMatrix();
    glScalef(model->scale, model->scale, model->scale);
    glTranslatef(-model->center.x, -model->center.y, -model->center.z);

    glColor3f(0.76f, 0.82f, 0.92f);
    glBegin(GL_TRIANGLES);
    for (size_t i = 0; i < model->count; ++i) {
        Vertex vertex = model->vertices[i];
        glNormal3f(vertex.normal.x, vertex.normal.y, vertex.normal.z);
        glVertex3f(vertex.position.x, vertex.position.y, vertex.position.z);
    }
    glEnd();

    glPopMatrix();
}

/* Configures the OpenGL projection matrix for perspective or orthographic mode. */
static void apply_projection(int width, int height, float distance, ProjectionMode projection_mode)
{
    float aspect = height > 0 ? (float)width / (float)height : 1.0f;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (projection_mode == PROJECTION_ORTHOGRAPHIC) {
        float view_size = distance * 0.35f;
        glOrtho(
            -aspect * view_size,
             aspect * view_size,
            -view_size,
             view_size,
             0.1f,
             100.0f);
    } else {
        float view_size = 0.65f;
        glFrustum(
            -aspect * view_size,
             aspect * view_size,
            -view_size,
             view_size,
             1.0f,
             100.0f);
    }
}

/* Clears the frame and renders the grid and model from the current camera state. */
void render_scene(
    const Model *model,
    int width,
    int height,
    float yaw,
    float pitch,
    float distance,
    int show_wireframe,
    ProjectionMode projection_mode)
{
    glViewport(0, 0, width, height);
    glClearColor(0.08f, 0.09f, 0.11f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    apply_projection(width, height, distance, projection_mode);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -distance);
    glRotatef(pitch, 1.0f, 0.0f, 0.0f);
    glRotatef(yaw, 0.0f, 1.0f, 0.0f);

    GLfloat light_position[] = {3.0f, 5.0f, 4.0f, 1.0f};
    GLfloat light_color[] = {0.95f, 0.95f, 0.90f, 1.0f};
    GLfloat ambient[] = {0.28f, 0.30f, 0.34f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_color);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

    draw_grid();

    glPolygonMode(GL_FRONT_AND_BACK, show_wireframe ? GL_LINE : GL_FILL);
    draw_model(model);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
