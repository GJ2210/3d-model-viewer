#ifndef RENDERER_H
#define RENDERER_H

#include "model.h"

typedef enum {
    PROJECTION_PERSPECTIVE,
    PROJECTION_ORTHOGRAPHIC
} ProjectionMode;

const char *projection_mode_name(ProjectionMode mode);
void render_scene(
    const Model *model,
    int width,
    int height,
    float yaw,
    float pitch,
    float distance,
    int show_wireframe,
    ProjectionMode projection_mode);

#endif
