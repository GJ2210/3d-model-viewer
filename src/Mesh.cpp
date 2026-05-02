#include "Mesh.h"
#include <cstddef>

Mesh::Mesh() {
    glGenVertexArrays(1, &flatVAO_);
    glGenBuffers(1, &flatVBO_);
    glGenVertexArrays(1, &smthVAO_);
    glGenBuffers(1, &smthVBO_);
    glGenBuffers(1, &smthEBO_);
}

Mesh::~Mesh() {
    glDeleteVertexArrays(1, &flatVAO_);
    glDeleteBuffers(1, &flatVBO_);
    glDeleteVertexArrays(1, &smthVAO_);
    glDeleteBuffers(1, &smthVBO_);
    glDeleteBuffers(1, &smthEBO_);
}

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

void Mesh::drawFlat() const {
    glBindVertexArray(flatVAO_);
    glDrawArrays(GL_TRIANGLES, 0, flatVertexCount_);
    glBindVertexArray(0);
}

void Mesh::drawSmooth() const {
    glBindVertexArray(smthVAO_);
    glDrawElements(GL_TRIANGLES, smthIndexCount_, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void Mesh::drawFlatWireframe() const {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawFlat();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Mesh::drawSmoothWireframe() const {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawSmooth();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
