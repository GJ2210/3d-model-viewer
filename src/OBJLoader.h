#pragma once

#include "Mesh.h"
#include <string>
#include <vector>

// parsed geometry
struct MeshData {
    std::vector<Vertex>       flatVertices;
    std::vector<Vertex>       smoothVertices;
    std::vector<unsigned int> smoothIndices;
};

bool loadOBJ(const std::string& path, MeshData& out);
