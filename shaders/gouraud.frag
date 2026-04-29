#version 330 core
// ── Gouraud shading fragment shader ───────────────────────────────────────
// Simply outputs the colour that was computed per-vertex and interpolated by
// the rasteriser. No per-fragment lighting is performed here.

in  vec3 vColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(vColor, 1.0);
}
