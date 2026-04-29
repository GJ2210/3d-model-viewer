#version 330 core
// ── Flat shading fragment shader ───────────────────────────────────────────
// Evaluates Blinn-Phong reflectance once per fragment, but because vNormal is
// declared 'flat', every fragment in the triangle receives the same normal,
// producing the characteristic per-face shading.

flat in vec3 vNormal;
     in vec3 vFragPos;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform vec3 viewPos;

out vec4 FragColor;

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(lightPos - vFragPos);    // light direction
    vec3 V = normalize(viewPos  - vFragPos);    // view direction
    vec3 H = normalize(L + V);                  // Blinn halfway vector

    // Blinn-Phong components
    float ambient  = 0.15;
    float diffuse  = max(dot(N, L), 0.0);
    float specular = pow(max(dot(N, H), 0.0), 32.0);

    vec3 color = (ambient + diffuse) * objectColor * lightColor
               + specular * lightColor * 0.4;

    FragColor = vec4(color, 1.0);
}
