#version 330 core
// ── Phong shading fragment shader ─────────────────────────────────────────
// Evaluates Blinn-Phong reflectance per fragment using the smoothly
// interpolated surface normal. This gives the highest-quality result with
// correct specular highlights even on low-polygon geometry.

in vec3 vNormal;
in vec3 vFragPos;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform vec3 viewPos;

out vec4 FragColor;

void main() {
    vec3 N = normalize(vNormal);                // re-normalise after interpolation
    vec3 L = normalize(lightPos - vFragPos);    // light direction
    vec3 V = normalize(viewPos  - vFragPos);    // view direction
    vec3 H = normalize(L + V);                  // Blinn halfway vector

    // Blinn-Phong components
    float ambient  = 0.15;
    float diffuse  = max(dot(N, L), 0.0);
    float specular = pow(max(dot(N, H), 0.0), 64.0);   // higher shininess than Gouraud

    vec3 color = (ambient + diffuse) * objectColor * lightColor
               + specular * lightColor * 0.4;

    FragColor = vec4(color, 1.0);
}
