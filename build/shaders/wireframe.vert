#version 330 core
// ── Wireframe vertex shader ────────────────────────────────────────────────
// Minimal pass-through: only the position attribute is used.

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;  // bound but unused (keeps layout consistent)

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
