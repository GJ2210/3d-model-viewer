/**
 * @file OBJLoader.h
 * @brief Wavefront OBJ file parser.
 *
 * Parses vertex positions, optional normals and texture coordinates, and
 * face definitions. Polygon faces are fan-triangulated. The parsed geometry
 * is emitted in two forms:
 *
 *   flatVertices  – one Vertex per triangle corner, normal = face normal.
 *                   Used directly (non-indexed) for flat shading.
 *
 *   smoothVertices / smoothIndices – vertices keyed by position index with
 *                   area-weighted averaged normals. Used with index buffer
 *                   for Gouraud/Phong shading.
 *
 * The mesh is automatically centered at the origin and scaled so its
 * longest axis fits within [-1, 1].
 */
#pragma once

#include "Mesh.h"
#include <string>
#include <vector>

/// Parsed mesh data ready for GPU upload.
struct MeshData {
    std::vector<Vertex>        flatVertices;    ///< Non-indexed, face normals.
    std::vector<Vertex>        smoothVertices;  ///< Indexed, averaged normals.
    std::vector<unsigned int>  smoothIndices;
};

/**
 * Parse @p path as a Wavefront OBJ file and populate @p out.
 * @return true on success; prints an error to stderr and returns false on failure.
 */
bool loadOBJ(const std::string& path, MeshData& out);
