#include "OBJLoader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <array>
#include <cfloat>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// ── Internal types ────────────────────────────────────────────────────────────

/// One corner of an OBJ face: 0-based indices into the parsed arrays.
struct FaceVertex {
    int p = -1;   ///< Position index.
    int t = -1;   ///< Texture-coord index (may be -1).
    int n = -1;   ///< Normal index (may be -1).
};

// ── Helpers ───────────────────────────────────────────────────────────────────

/**
 * Parse an OBJ face vertex token of the form:
 *   "v", "v/t", "v//n", "v/t/n"
 * All indices are converted to 0-based.
 */
static FaceVertex parseFaceVertex(const std::string& token) {
    FaceVertex fv;
    std::size_t slash1 = token.find('/');
    if (slash1 == std::string::npos) {
        fv.p = std::stoi(token) - 1;
        return fv;
    }
    fv.p = std::stoi(token.substr(0, slash1)) - 1;
    std::size_t slash2 = token.find('/', slash1 + 1);
    if (slash2 == std::string::npos) {
        // v/t
        fv.t = std::stoi(token.substr(slash1 + 1)) - 1;
    } else if (slash2 == slash1 + 1) {
        // v//n
        fv.n = std::stoi(token.substr(slash2 + 1)) - 1;
    } else {
        // v/t/n
        fv.t = std::stoi(token.substr(slash1 + 1, slash2 - slash1 - 1)) - 1;
        fv.n = std::stoi(token.substr(slash2 + 1)) - 1;
    }
    return fv;
}

// ── Main loader ───────────────────────────────────────────────────────────────

bool loadOBJ(const std::string& path, MeshData& out) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "[OBJLoader] Cannot open: " << path << "\n";
        return false;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> fileNormals;

    // Each triangle: three FaceVertex corners.
    using Triangle = std::array<FaceVertex, 3>;
    std::vector<Triangle> triangles;

    // ── Parse ─────────────────────────────────────────────────────────────
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ss(line);
        std::string token;
        ss >> token;

        if (token == "v") {
            glm::vec3 p;
            ss >> p.x >> p.y >> p.z;
            positions.push_back(p);
        } else if (token == "vn") {
            glm::vec3 n;
            ss >> n.x >> n.y >> n.z;
            fileNormals.push_back(n);
        } else if (token == "vt") {
            glm::vec2 t;
            ss >> t.x >> t.y;
            texCoords.push_back(t);
        } else if (token == "f") {
            // Collect face vertices and fan-triangulate
            std::vector<FaceVertex> face;
            std::string vtxToken;
            while (ss >> vtxToken)
                face.push_back(parseFaceVertex(vtxToken));
            for (std::size_t i = 1; i + 1 < face.size(); ++i)
                triangles.push_back({ face[0], face[i], face[i + 1] });
        }
        // mtllib, usemtl, g, o, s – silently ignored for now
    }

    if (positions.empty() || triangles.empty()) {
        std::cerr << "[OBJLoader] No geometry found in: " << path << "\n";
        return false;
    }

    // ── Center and scale to fit in [-1, 1] on the longest axis ───────────
    glm::vec3 bbMin( FLT_MAX), bbMax(-FLT_MAX);
    for (const auto& p : positions) {
        bbMin = glm::min(bbMin, p);
        bbMax = glm::max(bbMax, p);
    }
    glm::vec3 center   = (bbMin + bbMax) * 0.5f;
    float     maxExtent = std::max({ bbMax.x - bbMin.x,
                                     bbMax.y - bbMin.y,
                                     bbMax.z - bbMin.z });
    float     invScale  = (maxExtent > 1e-6f) ? (2.0f / maxExtent) : 1.0f;

    for (auto& p : positions)
        p = (p - center) * invScale;

    // ── Build flat vertices (non-indexed, face normals) ───────────────────
    out.flatVertices.reserve(triangles.size() * 3);
    for (const auto& tri : triangles) {
        const glm::vec3& p0 = positions[tri[0].p];
        const glm::vec3& p1 = positions[tri[1].p];
        const glm::vec3& p2 = positions[tri[2].p];

        glm::vec3 edge1 = p1 - p0;
        glm::vec3 edge2 = p2 - p0;
        glm::vec3 cross = glm::cross(edge1, edge2);
        float     len   = glm::length(cross);
        if (len < 1e-8f) continue;   // skip degenerate triangles
        glm::vec3 faceNormal = cross / len;

        out.flatVertices.push_back({ p0, faceNormal });
        out.flatVertices.push_back({ p1, faceNormal });
        out.flatVertices.push_back({ p2, faceNormal });
    }

    // ── Build smooth vertices (indexed, area-weighted averaged normals) ───
    out.smoothVertices.resize(positions.size());
    for (std::size_t i = 0; i < positions.size(); ++i)
        out.smoothVertices[i] = { positions[i], glm::vec3(0.0f) };

    out.smoothIndices.reserve(triangles.size() * 3);
    for (const auto& tri : triangles) {
        const glm::vec3& p0 = positions[tri[0].p];
        const glm::vec3& p1 = positions[tri[1].p];
        const glm::vec3& p2 = positions[tri[2].p];

        // Area-weighted normal: the un-normalized cross product has magnitude = 2 * area
        glm::vec3 weighted = glm::cross(p1 - p0, p2 - p0);
        float len = glm::length(weighted);
        if (len < 1e-8f) continue;

        out.smoothVertices[tri[0].p].normal += weighted;
        out.smoothVertices[tri[1].p].normal += weighted;
        out.smoothVertices[tri[2].p].normal += weighted;

        out.smoothIndices.push_back(static_cast<unsigned int>(tri[0].p));
        out.smoothIndices.push_back(static_cast<unsigned int>(tri[1].p));
        out.smoothIndices.push_back(static_cast<unsigned int>(tri[2].p));
    }

    // Normalize accumulated normals; use Y-up fallback for isolated vertices
    for (auto& v : out.smoothVertices) {
        float len = glm::length(v.normal);
        v.normal  = (len > 1e-6f) ? (v.normal / len) : glm::vec3(0.0f, 1.0f, 0.0f);
    }

    std::cout << "[OBJLoader] Loaded " << path
              << " — " << triangles.size() << " triangles, "
              << positions.size() << " unique positions\n";
    return true;
}
