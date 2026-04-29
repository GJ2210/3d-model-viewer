#version 330 core
// ── Flat shading vertex shader ─────────────────────────────────────────────
// Transforms position and passes the face normal to the fragment shader.
// The 'flat' interpolation qualifier ensures the provoking vertex's normal is
// used for the entire triangle (no interpolation), giving the flat-faceted look.

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;

flat out vec3 vNormal;      // flat: no interpolation across the triangle
     out vec3 vFragPos;     // world-space fragment position

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    vFragPos      = vec3(worldPos);
    vNormal       = normalMatrix * aNormal;
    gl_Position   = projection * view * worldPos;
}
