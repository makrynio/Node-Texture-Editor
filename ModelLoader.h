#pragma once

#include <glad/glad.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Face {
    std::vector<unsigned int> indices;
    bool isQuad;
    
    Face(const std::vector<unsigned int>& indices, bool isQuad) : indices(indices), isQuad(isQuad) {}
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;        // Triangle indices for rendering
    std::vector<unsigned int> renderIndices;  // Same as indices for normal loading
    std::vector<unsigned int> indicesWithMarkers; // For export with face markers
    std::vector<Face> originalFaces;          // Original face structure for export
    unsigned int VAO, VBO, EBO;
    
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    ~Mesh();
    void Draw() const;
    
    // Convert quads to triangles for rendering
    void GenerateRenderIndices();
    
    // Regenerate OpenGL buffers (call after modifying vertex data)
    void RegenerateBuffers();
};

class ModelLoader {
public:
    ModelLoader();
    ~ModelLoader();
    
    // Load a 3D model from file
    bool LoadModel(const std::string& path);
    
    // Get loaded mesh data
    const std::vector<Mesh>& GetMeshes() const { return meshes; }
    bool HasLoadedModel() const { return !meshes.empty(); }
    
    // Clear loaded model
    void ClearModel();
    
    // Get model info
    std::string GetModelInfo() const;
    
    // Get model path
    std::string GetModelPath() const { return modelPath; }
    
    // Get raw model path (for remesh operations)
    std::string GetRawModelPath() const { return rawModelPath; }
    
    // Get original raw model path (for revert operations)
    std::string GetOriginalRawModelPath() const { return originalRawModelPath; }
    
    // Export loaded model to OBJ file
    bool ExportToOBJ(const std::string& filepath);
    
    // Export loaded model to OBJ file with quads preserved (for Quadriflow)
    bool ExportToOBJWithQuads(const std::string& filepath);
    
    // Post-process loaded mesh to add normals and UVs
    void PostProcessMesh();
    
private:
    std::vector<Mesh> meshes;
    std::string modelPath;
    std::string rawModelPath;  // Path to raw file in mesh_ops folder
    std::string originalRawModelPath;  // Path to original raw file (never changes)
    std::string modelName;
    
    // Assimp importer
    Assimp::Importer importer;
    
    // Process Assimp scene
    void ProcessNode(aiNode* node, const aiScene* scene);
    Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
    
    // Convert Assimp vector to glm
    glm::vec3 AssimpToGlm(const aiVector3D& vec);
    glm::vec2 AssimpToGlm(const aiVector2D& vec);
    
    // Post-processing helper methods
    void ComputeNormals(Mesh& mesh);
    void GenerateUVs(Mesh& mesh);
    glm::vec3 ComputeFaceNormal(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3);
};
