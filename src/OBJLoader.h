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

// Loads a Wavefront OBJ into flat and smooth mesh buffers.
bool loadOBJ(const std::string& path, MeshData& out, bool normalize = true);
