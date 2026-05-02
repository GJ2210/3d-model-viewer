#include "shading.h"

#include <GLFW/glfw3.h>

/* Returns a readable name for the active shading mode. */
const char *shading_mode_name(ShadingMode mode)
{
    if (mode == SHADING_FLAT) {
        return "flat";
    }
    if (mode == SHADING_GOURAUD) {
        return "gouraud";
    }
    return "phong";
}

/* Returns the next shading mode in the UI cycle. */
ShadingMode next_shading_mode(ShadingMode mode)
{
    if (mode == SHADING_FLAT) {
        return SHADING_GOURAUD;
    }
    if (mode == SHADING_GOURAUD) {
        return SHADING_PHONG;
    }
    return SHADING_FLAT;
}

/* Selects the correct normal for the current shading mode. */
Vec3 shading_vertex_normal(const Vertex *vertex, ShadingMode mode)
{
    return mode == SHADING_FLAT ? vertex->flat_normal : vertex->smooth_normal;
}

/* Applies fixed-function OpenGL shading and material settings. */
void apply_shading_mode(ShadingMode mode)
{
    GLfloat specular[] = {0.65f, 0.70f, 0.78f, 1.0f};
    GLfloat no_specular[] = {0.0f, 0.0f, 0.0f, 1.0f};

    glShadeModel(mode == SHADING_FLAT ? GL_FLAT : GL_SMOOTH);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mode == SHADING_PHONG ? specular : no_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mode == SHADING_PHONG ? 64.0f : 0.0f);
}
