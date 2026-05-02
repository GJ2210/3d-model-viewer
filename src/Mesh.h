#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

// vertex data
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

class Mesh {
public:
    Mesh();
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    void uploadFlat(const std::vector<Vertex>& vertices);
    void uploadSmooth(const std::vector<Vertex>& vertices,
                      const std::vector<unsigned int>& indices);

    void drawFlat() const;
    void drawSmooth() const;
    void drawFlatWireframe() const;
    void drawSmoothWireframe() const;

    bool isLoaded() const { return flatVertexCount_ > 0; }

private:
    GLuint flatVAO_ = 0, flatVBO_ = 0;
    GLuint smthVAO_ = 0, smthVBO_ = 0, smthEBO_ = 0;

    int flatVertexCount_ = 0;
    int smthIndexCount_ = 0;
};
