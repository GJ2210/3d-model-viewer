#ifndef SHADING_H
#define SHADING_H

#include "model.h"

typedef enum {
    SHADING_FLAT,
    SHADING_GOURAUD,
    SHADING_PHONG
} ShadingMode;

const char *shading_mode_name(ShadingMode mode);
ShadingMode next_shading_mode(ShadingMode mode);
Vec3 shading_vertex_normal(const Vertex *vertex, ShadingMode mode);
void apply_shading_mode(ShadingMode mode);

#endif
