#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

struct QuadriflowSettings {
    int targetFaces = 8000;           // Target number of faces in output
    bool preserveSharp = false;       // Preserve sharp edges
    bool preserveBoundary = false;    // Preserve boundary
    bool adaptiveScale = false;       // Use adaptive scaling
    bool minimumCostFlow = false;     // Use minimum cost flow solver
    bool aggressiveSAT = false;       // Use aggressive SAT solver
    int rngSeed = 0;                  // Random number generator seed
};

struct QuadriflowMesh {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;
    
    void clear() {
        positions.clear();
        uvs.clear();
        normals.clear();
        indices.clear();
    }
    
    bool isValid() const {
        return !positions.empty() && !indices.empty() && indices.size() % 4 == 0; // Quad mesh
    }
    
    size_t getQuadCount() const {
        return indices.size() / 4;
    }
    
    size_t getVertexCount() const {
        return positions.size();
    }
};

class QuadriflowWrapper {
public:
    QuadriflowWrapper();
    ~QuadriflowWrapper();
    
    // Main quadrangulation function
    bool QuadrangulateMesh(const QuadriflowMesh& inputMesh, QuadriflowMesh& outputMesh, const QuadriflowSettings& settings);
    
    // Load mesh from OBJ file
    bool LoadMeshFromOBJ(const std::string& filepath, QuadriflowMesh& mesh);
    
    // Save mesh to OBJ file
    bool SaveMeshToOBJ(const QuadriflowMesh& mesh, const std::string& filepath);
    
    // Get last error message
    const std::string& GetLastError() const { return m_LastError; }
    
    // Check if Quadriflow is available
    bool IsAvailable() const { return m_Available; }

private:
    std::string m_LastError;
    bool m_Available;
    
    void SetError(const std::string& error);
    bool ValidateMesh(const QuadriflowMesh& mesh);
};
