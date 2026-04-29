#version 330 core
// ── Phong shading vertex shader ────────────────────────────────────────────
// Transforms position and normal into world space and passes them to the
// fragment shader, where lighting is evaluated per fragment.

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;

out vec3 vNormal;    // world-space normal (interpolated to fragment)
out vec3 vFragPos;   // world-space position

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    vFragPos      = vec3(worldPos);
    vNormal       = normalMatrix * aNormal;
    gl_Position   = projection * view * worldPos;
}
