#include "model.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 1024

static Vec3 vec3_sub(Vec3 a, Vec3 b)
{
    Vec3 result = {a.x - b.x, a.y - b.y, a.z - b.z};
    return result;
}

static Vec3 vec3_cross(Vec3 a, Vec3 b)
{
    Vec3 result = {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
    return result;
}

static Vec3 vec3_normalize(Vec3 value)
{
    float length = sqrtf(value.x * value.x + value.y * value.y + value.z * value.z);
    if (length <= 0.00001f) {
        Vec3 fallback = {0.0f, 1.0f, 0.0f};
        return fallback;
    }

    Vec3 result = {value.x / length, value.y / length, value.z / length};
    return result;
}

void model_init(Model *model)
{
    memset(model, 0, sizeof(*model));
    model->scale = 1.0f;
}

void model_free(Model *model)
{
    free(model->vertices);
    model_init(model);
}

static int model_reserve(Model *model, size_t required_capacity)
{
    if (required_capacity <= model->capacity) {
        return 1;
    }

    size_t next_capacity = model->capacity == 0 ? INITIAL_CAPACITY : model->capacity;
    while (next_capacity < required_capacity) {
        next_capacity *= 2;
    }

    Vertex *next_vertices = realloc(model->vertices, next_capacity * sizeof(*next_vertices));
    if (!next_vertices) {
        return 0;
    }

    model->vertices = next_vertices;
    model->capacity = next_capacity;
    return 1;
}

static int model_add_triangle(Model *model, Vec3 a, Vec3 b, Vec3 c)
{
    if (!model_reserve(model, model->count + 3)) {
        return 0;
    }

    Vec3 normal = vec3_normalize(vec3_cross(vec3_sub(b, a), vec3_sub(c, a)));
    model->vertices[model->count++] = (Vertex){a, normal};
    model->vertices[model->count++] = (Vertex){b, normal};
    model->vertices[model->count++] = (Vertex){c, normal};
    return 1;
}

static void model_update_bounds(Model *model)
{
    if (model->count == 0) {
        model->center = (Vec3){0.0f, 0.0f, 0.0f};
        model->scale = 1.0f;
        return;
    }

    Vec3 min = model->vertices[0].position;
    Vec3 max = model->vertices[0].position;

    for (size_t i = 1; i < model->count; ++i) {
        Vec3 p = model->vertices[i].position;
        min.x = fminf(min.x, p.x);
        min.y = fminf(min.y, p.y);
        min.z = fminf(min.z, p.z);
        max.x = fmaxf(max.x, p.x);
        max.y = fmaxf(max.y, p.y);
        max.z = fmaxf(max.z, p.z);
    }

    model->center = (Vec3){
        (min.x + max.x) * 0.5f,
        (min.y + max.y) * 0.5f,
        (min.z + max.z) * 0.5f,
    };

    float largest_span = fmaxf(fmaxf(max.x - min.x, max.y - min.y), max.z - min.z);
    model->scale = largest_span > 0.00001f ? 2.0f / largest_span : 1.0f;
}

static int parse_obj_index(const char *token, int position_count)
{
    char *end = NULL;
    long raw_index = strtol(token, &end, 10);
    if (end == token || raw_index == 0) {
        return -1;
    }

    if (raw_index < 0) {
        raw_index = position_count + raw_index + 1;
    }

    if (raw_index < 1 || raw_index > position_count) {
        return -1;
    }

    return (int)raw_index - 1;
}

static int append_position(Vec3 **positions, size_t *count, size_t *capacity, Vec3 position)
{
    if (*count >= *capacity) {
        size_t next_capacity = *capacity == 0 ? INITIAL_CAPACITY : *capacity * 2;
        Vec3 *next_positions = realloc(*positions, next_capacity * sizeof(**positions));
        if (!next_positions) {
            return 0;
        }

        *positions = next_positions;
        *capacity = next_capacity;
    }

    (*positions)[(*count)++] = position;
    return 1;
}

int load_obj_model(const char *path, Model *out_model)
{
    FILE *file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "Could not open model: %s\n", path);
        return 0;
    }

    Model loaded;
    model_init(&loaded);

    Vec3 *positions = NULL;
    size_t position_count = 0;
    size_t position_capacity = 0;
    char line[4096];
    int success = 1;

    while (fgets(line, sizeof(line), file)) {
        char *cursor = line;
        while (isspace((unsigned char)*cursor)) {
            cursor++;
        }

        if (cursor[0] == 'v' && isspace((unsigned char)cursor[1])) {
            Vec3 position;
            if (sscanf(cursor + 1, "%f %f %f", &position.x, &position.y, &position.z) == 3) {
                if (!append_position(&positions, &position_count, &position_capacity, position)) {
                    success = 0;
                    break;
                }
            }
        } else if (cursor[0] == 'f' && isspace((unsigned char)cursor[1])) {
            int face_indices[64];
            int face_count = 0;
            char *token = strtok(cursor + 1, " \t\r\n");

            while (token && face_count < (int)(sizeof(face_indices) / sizeof(face_indices[0]))) {
                int index = parse_obj_index(token, (int)position_count);
                if (index >= 0) {
                    face_indices[face_count++] = index;
                }
                token = strtok(NULL, " \t\r\n");
            }

            for (int i = 1; i + 1 < face_count; ++i) {
                Vec3 a = positions[face_indices[0]];
                Vec3 b = positions[face_indices[i]];
                Vec3 c = positions[face_indices[i + 1]];
                if (!model_add_triangle(&loaded, a, b, c)) {
                    success = 0;
                    break;
                }
            }
        }
    }

    fclose(file);
    free(positions);

    if (!success || loaded.count == 0) {
        model_free(&loaded);
        fprintf(stderr, "Could not load triangles from OBJ: %s\n", path);
        return 0;
    }

    model_update_bounds(&loaded);
    snprintf(loaded.source_path, sizeof(loaded.source_path), "%s", path);
    model_free(out_model);
    *out_model = loaded;

    printf("Loaded %zu triangles from %s\n", out_model->count / 3, out_model->source_path);
    return 1;
}
