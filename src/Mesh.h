/**
 * @file Mesh.h
 * @brief GPU mesh with separate flat-shaded and smooth-shaded storage.
 *
 * Two representations are uploaded to the GPU:
 *   Flat   – non-indexed triangles; every vertex carries the face normal.
 *            Used with the flat shading shader (flat interpolation qualifier).
 *   Smooth – indexed triangles; vertices share averaged normals across faces.
 *            Used with Gouraud and Phong shaders.
 *
 * Both share vertex layout: location 0 = position (vec3), location 1 = normal (vec3).
 */
#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

/// A single mesh vertex: world-space position and surface normal.
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

class Mesh {
public:
    Mesh();
    ~Mesh();

    // Non-copyable – each instance owns GL objects.
    Mesh(const Mesh&)            = delete;
    Mesh& operator=(const Mesh&) = delete;

    /// Upload the non-indexed flat-shading vertex buffer. Call once.
    void uploadFlat  (const std::vector<Vertex>& vertices);

    /// Upload the indexed smooth-shading buffers. Call once.
    void uploadSmooth(const std::vector<Vertex>& vertices,
                      const std::vector<unsigned int>& indices);

    // ── Draw calls ────────────────────────────────────────────────────────
    void drawFlat()            const;   ///< GL_TRIANGLES, face normals.
    void drawSmooth()          const;   ///< GL_TRIANGLES, smooth normals.
    void drawFlatWireframe()   const;   ///< Same geometry as drawFlat, GL_LINE mode.
    void drawSmoothWireframe() const;   ///< Same geometry as drawSmooth, GL_LINE mode.

    bool isLoaded() const { return flatVertexCount_ > 0; }

private:
    // ── GL object handles ─────────────────────────────────────────────────
    GLuint flatVAO_ = 0, flatVBO_ = 0;
    GLuint smthVAO_ = 0, smthVBO_ = 0, smthEBO_ = 0;

    int flatVertexCount_ = 0;
    int smthIndexCount_  = 0;
};
