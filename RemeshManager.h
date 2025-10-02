#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

struct RemeshSettings {
    int targetQuads = 8000;  // Target number of quad faces for Quadriflow
};

struct MeshData {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;
    
    // Clear all data
    void clear() {
        positions.clear();
        uvs.clear();
        normals.clear();
        indices.clear();
    }
    
    // Check if mesh is valid
    bool isValid() const {
        return !positions.empty() && !indices.empty() && indices.size() % 3 == 0;
    }
    
    // Get triangle count
    size_t getTriangleCount() const {
        return indices.size() / 3;
    }
    
    // Get vertex count
    size_t getVertexCount() const {
        return positions.size();
    }
};

class RemeshManager {
public:
    RemeshManager();
    ~RemeshManager();
    
    // Main Quadriflow remesh function
    bool QuadriflowRemesh(const std::string& inputPath, const std::string& outputPath, const RemeshSettings& settings);
    
    // Get current settings
    const RemeshSettings& GetSettings() const { return m_Settings; }
    
    // Set settings
    void SetSettings(const RemeshSettings& settings) { m_Settings = settings; }
    
    // Get last error message
    const std::string& GetLastError() const { return m_LastError; }
    
private:
    RemeshSettings m_Settings;
    std::string m_LastError;
    
    // Helper functions
    void SetError(const std::string& error);
};
