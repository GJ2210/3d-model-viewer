#include "Mesh.h"
#include <cstddef>

// Creates the OpenGL vertex array and buffer objects used by this mesh.
Mesh::Mesh() {
    glGenVertexArrays(1, &flatVAO_);
    glGenBuffers(1, &flatVBO_);
    glGenVertexArrays(1, &smthVAO_);
    glGenBuffers(1, &smthVBO_);
    glGenBuffers(1, &smthEBO_);
}

// Releases the OpenGL objects owned by this mesh.
Mesh::~Mesh() {
    glDeleteVertexArrays(1, &flatVAO_);
    glDeleteBuffers(1, &flatVBO_);
    glDeleteVertexArrays(1, &smthVAO_);
    glDeleteBuffers(1, &smthVBO_);
    glDeleteBuffers(1, &smthEBO_);
}

// Uploads duplicated per-face vertices for flat shading.
void Mesh::uploadFlat(const std::vector<Vertex>& vertices) {
    flatVertexCount_ = static_cast<int>(vertices.size());

    glBindVertexArray(flatVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, flatVBO_);
    glBufferData(GL_ARRAY_BUFFER, flatVertexCount_ * sizeof(Vertex),
                 vertices.data(), GL_STATIC_DRAW);

    // position attrib
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
    // normal attrib
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));

    glBindVertexArray(0);
}

// Uploads shared vertices and indices for smooth shading.
void Mesh::uploadSmooth(const std::vector<Vertex>& vertices,
                        const std::vector<unsigned int>& indices) {
    smthIndexCount_ = static_cast<int>(indices.size());

    glBindVertexArray(smthVAO_);

    glBindBuffer(GL_ARRAY_BUFFER, smthVBO_);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
                 vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, smthEBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, smthIndexCount_ * sizeof(unsigned int),
                 indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));

    glBindVertexArray(0);
}

// Draws the flat-shaded vertex buffer without indices.
void Mesh::drawFlat() const {
    glBindVertexArray(flatVAO_);
    glDrawArrays(GL_TRIANGLES, 0, flatVertexCount_);
    glBindVertexArray(0);
}

// Draws the smooth-shaded indexed vertex buffer.
void Mesh::drawSmooth() const {
    glBindVertexArray(smthVAO_);
    glDrawElements(GL_TRIANGLES, smthIndexCount_, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

// Draws the flat mesh as lines, then restores filled polygon mode.
void Mesh::drawFlatWireframe() const {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawFlat();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// Draws the smooth mesh as lines, then restores filled polygon mode.
void Mesh::drawSmoothWireframe() const {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawSmooth();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
