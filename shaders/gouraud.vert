#version 330 core
// ── Gouraud shading vertex shader ─────────────────────────────────────────
// Computes the full Blinn-Phong lighting equation per vertex using smooth
// (averaged) normals. The resulting colour is interpolated across each
// triangle by the rasteriser, which produces the characteristic Gouraud
// colour banding that is visible on coarse geometry.

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;

// Lighting uniforms – evaluated here, in the vertex shader
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform vec3 viewPos;

out vec3 vColor;   // interpolated per-vertex colour

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    vec3 fragPos  = vec3(worldPos);

    vec3 N = normalize(normalMatrix * aNormal);
    vec3 L = normalize(lightPos - fragPos);
    vec3 V = normalize(viewPos  - fragPos);
    vec3 H = normalize(L + V);

    float ambient  = 0.15;
    float diffuse  = max(dot(N, L), 0.0);
    float specular = pow(max(dot(N, H), 0.0), 32.0);

    vColor = (ambient + diffuse) * objectColor * lightColor
           + specular * lightColor * 0.4;

    gl_Position = projection * view * worldPos;
}
