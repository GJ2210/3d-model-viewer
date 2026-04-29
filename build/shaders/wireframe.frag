#version 330 core
// ── Wireframe fragment shader ──────────────────────────────────────────────
// Outputs a flat colour supplied by the application (typically white or black).

uniform vec3 wireColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(wireColor, 1.0);
}
