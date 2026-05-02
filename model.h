#ifndef MODEL_H
#define MODEL_H

#include <stddef.h>

#define PATH_BUFFER_SIZE 2048

typedef struct {
    float x;
    float y;
    float z;
} Vec3;

typedef struct {
    Vec3 position;
    Vec3 flat_normal;
    Vec3 smooth_normal;
} Vertex;

typedef struct {
    Vertex *vertices;
    size_t count;
    size_t capacity;
    Vec3 center;
    float scale;
    char source_path[PATH_BUFFER_SIZE];
} Model;

void model_init(Model *model);
void model_free(Model *model);
int load_obj_model(const char *path, Model *out_model);

#endif
