#include "ModelLoader.h"
#include <iostream>
#include <filesystem>
#include <fstream>

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
    : vertices(vertices), indices(indices) {
    // For normal loading, indices are already triangles, so use them directly
    renderIndices = indices;
    
    // Generate OpenGL buffers
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    // Bind VAO
    glBindVertexArray(VAO);
    
    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
    
    // Upload index data (triangles only)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, renderIndices.size() * sizeof(unsigned int), &renderIndices[0], GL_STATIC_DRAW);
    
    // Set vertex attributes
    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
    
    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    
    // Texture coordinates
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    
    glBindVertexArray(0);
}

Mesh::~Mesh() {
    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
    if (VBO != 0) glDeleteBuffers(1, &VBO);
    if (EBO != 0) glDeleteBuffers(1, &EBO);
}

void Mesh::Draw() const {
    // Debug: Check if VAO is valid
    if (VAO == 0) {
        std::cerr << "ERROR: Trying to draw mesh with invalid VAO (0)" << std::endl;
        return;
    }
    
    // Debug: Check for OpenGL errors before binding
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cout << "OpenGL error before VAO bind: " << error << std::endl;
    }
    
    glBindVertexArray(VAO);
    
    // Debug: Check for OpenGL errors after binding
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cout << "OpenGL error after VAO bind: " << error << std::endl;
    }
    
    // Debug: Check if we have indices to draw
    if (renderIndices.empty()) {
        std::cerr << "ERROR: Trying to draw mesh with no render indices" << std::endl;
        glBindVertexArray(0);
        return;
    }
    
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(renderIndices.size()), GL_UNSIGNED_INT, 0);
    
    // Debug: Check for OpenGL errors after draw call
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cout << "OpenGL error after glDrawElements: " << error << std::endl;
    }
    
    glBindVertexArray(0);
}

void Mesh::GenerateRenderIndices() {
    renderIndices.clear();
    
    // Process faces using markers
    for (size_t i = 0; i < indices.size(); ) {
        // Find the next marker
        size_t faceEnd = i;
        while (faceEnd < indices.size() && 
               indices[faceEnd] != 0xFFFFFFFF && 
               indices[faceEnd] != 0xFFFFFFFE) {
            faceEnd++;
        }
        
        if (faceEnd > i) {
            size_t faceSize = faceEnd - i;
            
            if (faceSize == 3) {
                // Triangle - add as is
                renderIndices.push_back(indices[i]);
                renderIndices.push_back(indices[i + 1]);
                renderIndices.push_back(indices[i + 2]);
            } else if (faceSize == 4) {
                // Quad - convert to two triangles
                // Triangle 1: 0, 1, 2
                renderIndices.push_back(indices[i]);
                renderIndices.push_back(indices[i + 1]);
                renderIndices.push_back(indices[i + 2]);
                // Triangle 2: 0, 2, 3
                renderIndices.push_back(indices[i]);
                renderIndices.push_back(indices[i + 2]);
                renderIndices.push_back(indices[i + 3]);
            }
            
            // Skip the marker
            i = faceEnd + 1;
        } else {
            i++;
        }
    }
}

ModelLoader::ModelLoader() {
}

ModelLoader::~ModelLoader() {
    ClearModel();
}

bool ModelLoader::LoadModel(const std::string& path) {
    std::cout << "Loading model: " << path << std::endl;
    
    // Clear previous model
    ClearModel();
    
    // Check if file exists
    if (!std::filesystem::exists(path)) {
        std::cerr << "Model file does not exist: " << path << std::endl;
        return false;
    }
    
    // Copy raw file to mesh_ops folder for remesh operations
    std::filesystem::path sourcePath(path);
    std::string rawFileName = sourcePath.stem().string() + "_raw" + sourcePath.extension().string();
    std::string rawFilePath = "mesh_ops/" + rawFileName;
    
    try {
        std::filesystem::create_directories("mesh_ops");
        std::filesystem::copy_file(path, rawFilePath, std::filesystem::copy_options::overwrite_existing);
        std::cout << "Copied raw model to: " << rawFilePath << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Warning: Failed to copy raw file to mesh_ops: " << e.what() << std::endl;
        // Continue with loading even if copy fails
    }
    
    // Load model with Assimp
    const aiScene* scene = importer.ReadFile(path, 
        aiProcess_FlipUVs | 
        aiProcess_CalcTangentSpace |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices);
    
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Assimp error: " << importer.GetErrorString() << std::endl;
        return false;
    }
    
    // Store model info
    modelPath = path;
    rawModelPath = rawFilePath;  // Store path to raw file for remesh operations
    
    // Only set original raw model path if it's empty (first load)
    if (originalRawModelPath.empty()) {
        originalRawModelPath = rawFilePath;
    }
    
    modelName = std::filesystem::path(path).stem().string();
    
    // Process the scene
    ProcessNode(scene->mRootNode, scene);
    
    std::cout << "Successfully loaded model: " << modelName << " (" << meshes.size() << " meshes)" << std::endl;
    return true;
}

void ModelLoader::ProcessNode(aiNode* node, const aiScene* scene) {
    // Process all meshes in this node
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(ProcessMesh(mesh, scene));
    }
    
    // Process child nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        ProcessNode(node->mChildren[i], scene);
    }
}

Mesh ModelLoader::ProcessMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    // Process vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        
        // Position
        vertex.Position = AssimpToGlm(mesh->mVertices[i]);
        
        // Normal
        if (mesh->mNormals) {
            vertex.Normal = AssimpToGlm(mesh->mNormals[i]);
        } else {
            vertex.Normal = glm::vec3(0.0f, 0.0f, 0.0f);
        }
        
        // Texture coordinates
        if (mesh->mTextureCoords[0]) {
            vertex.TexCoords = AssimpToGlm(mesh->mTextureCoords[0][i]);
        } else {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }
        
        vertices.push_back(vertex);
    }
    
    // Normalize the mesh to unit sphere size (radius 1.0)
    if (!vertices.empty()) {
        // Find bounding box
        glm::vec3 minBounds = vertices[0].Position;
        glm::vec3 maxBounds = vertices[0].Position;
        
        for (const auto& vertex : vertices) {
            minBounds = glm::min(minBounds, vertex.Position);
            maxBounds = glm::max(maxBounds, vertex.Position);
        }
        
        // Calculate center and scale
        glm::vec3 center = (minBounds + maxBounds) * 0.5f;
        glm::vec3 size = maxBounds - minBounds;
        float maxDimension = std::max({size.x, size.y, size.z});
        float scale = 2.0f / maxDimension; // Scale to fit in sphere with radius 1.0 (total size 2.0)
        
        std::cout << "Normalizing model to match sphere (radius 1.0): center=(" << center.x << "," << center.y << "," << center.z 
                  << ") scale=" << scale << " (max dimension: " << maxDimension << ")" << std::endl;
        
        // Apply normalization to all vertices
        for (auto& vertex : vertices) {
            vertex.Position = (vertex.Position - center) * scale;
            // Recalculate normals after scaling (they should remain unit length)
            if (glm::length(vertex.Normal) > 0.0f) {
                vertex.Normal = glm::normalize(vertex.Normal);
            }
        }
        
        // Verify final positioning
        if (!vertices.empty()) {
            glm::vec3 finalMin = vertices[0].Position;
            glm::vec3 finalMax = vertices[0].Position;
            for (const auto& vertex : vertices) {
                finalMin = glm::min(finalMin, vertex.Position);
                finalMax = glm::max(finalMax, vertex.Position);
            }
            glm::vec3 finalCenter = (finalMin + finalMax) * 0.5f;
            glm::vec3 finalSize = finalMax - finalMin;
            std::cout << "Final model bounds: center=(" << finalCenter.x << "," << finalCenter.y << "," << finalCenter.z 
                      << ") size=(" << finalSize.x << "," << finalSize.y << "," << finalSize.z << ")" << std::endl;
            
            // Check if model is actually centered
            float centerTolerance = 0.01f;
            bool isCentered = (std::abs(finalCenter.x) < centerTolerance && 
                              std::abs(finalCenter.y) < centerTolerance && 
                              std::abs(finalCenter.z) < centerTolerance);
            std::cout << "Model is " << (isCentered ? "CENTERED" : "NOT CENTERED") << " at origin" << std::endl;
            
            if (!isCentered) {
                std::cout << "WARNING: Model center is off by (" << finalCenter.x << ", " << finalCenter.y << ", " << finalCenter.z << ")" << std::endl;
            }
        }
    }
    
    // Process indices - convert quads to triangles for rendering, but preserve original faces
    std::vector<Face> originalFaces;
    std::vector<unsigned int> indicesWithMarkers; // For export with face markers
    
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        
        if (face.mNumIndices == 3) {
            // Triangle - add as is
            std::vector<unsigned int> faceIndices;
            for (unsigned int j = 0; j < 3; j++) {
                indices.push_back(face.mIndices[j]);
                indicesWithMarkers.push_back(face.mIndices[j]);
                faceIndices.push_back(face.mIndices[j]);
            }
            indicesWithMarkers.push_back(0xFFFFFFFE); // Triangle marker
            originalFaces.push_back(Face(faceIndices, false));
        } else if (face.mNumIndices == 4) {
            // Quad - convert to two triangles using fan triangulation
            // Triangle 1: 0, 1, 2
            indices.push_back(face.mIndices[0]);
            indices.push_back(face.mIndices[1]);
            indices.push_back(face.mIndices[2]);
            // Triangle 2: 0, 2, 3
            indices.push_back(face.mIndices[0]);
            indices.push_back(face.mIndices[2]);
            indices.push_back(face.mIndices[3]);
            
            // Add original quad to export array with markers (NOT the triangulated version)
            indicesWithMarkers.push_back(face.mIndices[0]);
            indicesWithMarkers.push_back(face.mIndices[1]);
            indicesWithMarkers.push_back(face.mIndices[2]);
            indicesWithMarkers.push_back(face.mIndices[3]);
            indicesWithMarkers.push_back(0xFFFFFFFF); // Quad marker
            
            // Store original quad for export
            std::vector<unsigned int> faceIndices;
            for (unsigned int j = 0; j < 4; j++) {
                faceIndices.push_back(face.mIndices[j]);
            }
            originalFaces.push_back(Face(faceIndices, true));
        } else if (face.mNumIndices > 4) {
            // N-gon (5+ vertices) - triangulate using fan method
            std::vector<unsigned int> faceIndices;
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                faceIndices.push_back(face.mIndices[j]);
            }
            
            // Fan triangulation: connect first vertex to all others
            for (unsigned int j = 1; j < face.mNumIndices - 1; j++) {
                // Triangle: 0, j, j+1
                indices.push_back(face.mIndices[0]);
                indices.push_back(face.mIndices[j]);
                indices.push_back(face.mIndices[j + 1]);
            }
            
            // Add original N-gon to export array with markers (NOT the triangulated version)
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indicesWithMarkers.push_back(face.mIndices[j]);
            }
            indicesWithMarkers.push_back(0xFFFFFFFF); // N-gon marker
            
            originalFaces.push_back(Face(faceIndices, true));
            std::cout << "Triangulated " << face.mNumIndices << "-gon into " << (face.mNumIndices - 2) << " triangles" << std::endl;
        } else if (face.mNumIndices == 2) {
            // Edge - skip (not a valid face for rendering)
            std::cout << "Warning: Skipping edge with 2 vertices" << std::endl;
        } else if (face.mNumIndices == 1) {
            // Point - skip (not a valid face for rendering)
            std::cout << "Warning: Skipping point with 1 vertex" << std::endl;
        } else if (face.mNumIndices == 0) {
            // Empty face - skip
            std::cout << "Warning: Skipping empty face" << std::endl;
        }
    }
    
    Mesh resultMesh(vertices, indices);
    resultMesh.originalFaces = originalFaces;
    resultMesh.indicesWithMarkers = indicesWithMarkers;
    return resultMesh;
}

glm::vec3 ModelLoader::AssimpToGlm(const aiVector3D& vec) {
    return glm::vec3(vec.x, vec.y, vec.z);
}

glm::vec2 ModelLoader::AssimpToGlm(const aiVector2D& vec) {
    return glm::vec2(vec.x, vec.y);
}

void ModelLoader::ClearModel() {
    meshes.clear();
    modelPath.clear();
    modelName.clear();
}

std::string ModelLoader::GetModelInfo() const {
    if (meshes.empty()) {
        return "No model loaded";
    }
    
    size_t totalVertices = 0;
    size_t totalIndices = 0;
    
    for (const auto& mesh : meshes) {
        totalVertices += mesh.vertices.size();
        totalIndices += mesh.indices.size();
    }
    
    return modelName + " (" + std::to_string(meshes.size()) + " meshes, " + 
           std::to_string(totalVertices) + " vertices, " + 
           std::to_string(totalIndices / 3) + " triangles)";
}

bool ModelLoader::ExportToOBJ(const std::string& filepath)
{
    if (meshes.empty()) {
        std::cerr << "No model loaded to export" << std::endl;
        return false;
    }
    
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filepath << std::endl;
        return false;
    }
    
    std::cout << "Exporting model to: " << filepath << std::endl;
    
    // Write OBJ header
    file << "# Model Export" << std::endl;
    file << "# Generated by Quadriflow" << std::endl;
    file << "# Original file: " << modelPath << std::endl;
    file << "# Meshes: " << meshes.size() << std::endl;
    file << std::endl;
    
    // Calculate total vertices and faces
    size_t totalVertices = 0;
    size_t totalFaces = 0;
    for (const auto& mesh : meshes) {
        totalVertices += mesh.vertices.size();
        totalFaces += mesh.indices.size() / 3;
    }
    
    file << "# Total vertices: " << totalVertices << std::endl;
    file << "# Total faces: " << totalFaces << std::endl;
    file << std::endl;
    
    // Write vertices only (minimal format for Quadriflow)
    for (const auto& mesh : meshes) {
        for (const auto& vertex : mesh.vertices) {
            file << "v " << vertex.Position.x << " " << vertex.Position.y << " " << vertex.Position.z << std::endl;
        }
    }
    
    file << std::endl;
    
    // Write faces (simple format for Quadriflow compatibility)
    unsigned int vertexOffset = 1; // OBJ indices are 1-based
    for (const auto& mesh : meshes) {
        // Export as triangles only (Quadriflow expects triangles)
        for (size_t i = 0; i < mesh.indices.size(); i += 3) {
            if (i + 2 < mesh.indices.size()) {
                unsigned int v1 = mesh.indices[i] + vertexOffset;
                unsigned int v2 = mesh.indices[i + 1] + vertexOffset;
                unsigned int v3 = mesh.indices[i + 2] + vertexOffset;
                
                // Simple face format without texture/normal indices
                file << "f " << v1 << " " << v2 << " " << v3 << std::endl;
            }
        }
        vertexOffset += mesh.vertices.size();
    }
    
    file.close();
    
    std::cout << "Successfully exported model to: " << filepath << std::endl;
    std::cout << "Vertices: " << totalVertices << ", Faces: " << totalFaces << std::endl;
    
    return true;
}

bool ModelLoader::ExportToOBJWithQuads(const std::string& filepath) {
    if (meshes.empty()) {
        std::cerr << "No model loaded to export" << std::endl;
        return false;
    }
    
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filepath << std::endl;
        return false;
    }
    
    std::cout << "Exporting model with quads to: " << filepath << std::endl;
    
    // Write OBJ header
    file << "# Model Export with Quads" << std::endl;
    file << "# Generated by Quadriflow" << std::endl;
    file << "# Original file: " << modelPath << std::endl;
    file << "# Meshes: " << meshes.size() << std::endl;
    file << std::endl;
    
    // Count total vertices and faces
    size_t totalVertices = 0;
    size_t totalFaces = 0;
    for (const auto& mesh : meshes) {
        totalVertices += mesh.vertices.size();
        totalFaces += mesh.originalFaces.size();
    }
    
    // Write vertices
    for (const auto& mesh : meshes) {
        for (const auto& vertex : mesh.vertices) {
            file << "v " << vertex.Position.x << " " << vertex.Position.y << " " << vertex.Position.z << std::endl;
        }
    }
    
    file << std::endl;
    
    // Write texture coordinates
    for (const auto& mesh : meshes) {
        for (const auto& vertex : mesh.vertices) {
            file << "vt " << vertex.TexCoords.x << " " << vertex.TexCoords.y << std::endl;
        }
    }
    
    file << std::endl;
    
    // Write normals
    for (const auto& mesh : meshes) {
        for (const auto& vertex : mesh.vertices) {
            file << "vn " << vertex.Normal.x << " " << vertex.Normal.y << " " << vertex.Normal.z << std::endl;
        }
    }
    
    file << std::endl;
    
    // Write faces using original face structure
    unsigned int vertexOffset = 1; // OBJ indices are 1-based
    for (const auto& mesh : meshes) {
        for (const auto& face : mesh.originalFaces) {
            if (face.isQuad && face.indices.size() == 4) {
                // Quad
                unsigned int v1 = face.indices[0] + vertexOffset;
                unsigned int v2 = face.indices[1] + vertexOffset;
                unsigned int v3 = face.indices[2] + vertexOffset;
                unsigned int v4 = face.indices[3] + vertexOffset;
                
                file << "f " << v1 << "/" << v1 << "/" << v1 
                     << " " << v2 << "/" << v2 << "/" << v2 
                     << " " << v3 << "/" << v3 << "/" << v3 
                     << " " << v4 << "/" << v4 << "/" << v4 << std::endl;
            } else if (!face.isQuad && face.indices.size() == 3) {
                // Triangle
                unsigned int v1 = face.indices[0] + vertexOffset;
                unsigned int v2 = face.indices[1] + vertexOffset;
                unsigned int v3 = face.indices[2] + vertexOffset;
                
                file << "f " << v1 << "/" << v1 << "/" << v1 
                     << " " << v2 << "/" << v2 << "/" << v2 
                     << " " << v3 << "/" << v3 << "/" << v3 << std::endl;
            }
        }
        vertexOffset += mesh.vertices.size();
    }
    
    file.close();
    
    std::cout << "Successfully exported model with quads to: " << filepath << std::endl;
    std::cout << "Vertices: " << totalVertices << ", Faces: " << totalFaces << std::endl;
    
    return true;
}

void ModelLoader::PostProcessMesh() {
    std::cout << "Post-processing loaded mesh..." << std::endl;
    
    for (auto& mesh : meshes) {
        // Compute normals if they don't exist or are invalid
        ComputeNormals(mesh);
        
        // Generate UVs if they don't exist or are invalid
        GenerateUVs(mesh);
        
        // Regenerate OpenGL buffers with updated vertex data
        mesh.RegenerateBuffers();
        
        std::cout << "Post-processed mesh: " << mesh.vertices.size() << " vertices" << std::endl;
    }
    
    std::cout << "Mesh post-processing completed!" << std::endl;
}

void ModelLoader::ComputeNormals(Mesh& mesh) {
    std::cout << "Computing normals for mesh..." << std::endl;
    
    // Initialize all normals to zero
    for (auto& vertex : mesh.vertices) {
        vertex.Normal = glm::vec3(0.0f);
    }
    
    // Compute face normals and accumulate them at vertices
    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
        if (i + 2 < mesh.indices.size()) {
            unsigned int i1 = mesh.indices[i];
            unsigned int i2 = mesh.indices[i + 1];
            unsigned int i3 = mesh.indices[i + 2];
            
            if (i1 < mesh.vertices.size() && i2 < mesh.vertices.size() && i3 < mesh.vertices.size()) {
                glm::vec3 v1 = mesh.vertices[i1].Position;
                glm::vec3 v2 = mesh.vertices[i2].Position;
                glm::vec3 v3 = mesh.vertices[i3].Position;
                
                glm::vec3 faceNormal = ComputeFaceNormal(v1, v2, v3);
                
                // Accumulate face normal at each vertex
                mesh.vertices[i1].Normal += faceNormal;
                mesh.vertices[i2].Normal += faceNormal;
                mesh.vertices[i3].Normal += faceNormal;
            }
        }
    }
    
    // Normalize all vertex normals
    for (auto& vertex : mesh.vertices) {
        float length = glm::length(vertex.Normal);
        if (length > 0.0f) {
            vertex.Normal = glm::normalize(vertex.Normal);
        } else {
            // Fallback to default normal if computation failed
            vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
        }
    }
    
    std::cout << "Normals computed for " << mesh.vertices.size() << " vertices" << std::endl;
}

void ModelLoader::GenerateUVs(Mesh& mesh) {
    std::cout << "Generating UV coordinates for mesh..." << std::endl;
    
    // Find bounding box for UV mapping
    glm::vec3 minBounds = mesh.vertices[0].Position;
    glm::vec3 maxBounds = mesh.vertices[0].Position;
    
    for (const auto& vertex : mesh.vertices) {
        minBounds = glm::min(minBounds, vertex.Position);
        maxBounds = glm::max(maxBounds, vertex.Position);
    }
    
    glm::vec3 size = maxBounds - minBounds;
    float maxDimension = glm::max(glm::max(size.x, size.y), size.z);
    
    if (maxDimension > 0.0f) {
        // Generate UVs using planar projection (XY plane)
        for (auto& vertex : mesh.vertices) {
            glm::vec3 normalizedPos = (vertex.Position - minBounds) / maxDimension;
            vertex.TexCoords = glm::vec2(normalizedPos.x, normalizedPos.y);
        }
        
        std::cout << "UV coordinates generated using planar projection" << std::endl;
    } else {
        // Fallback: set default UVs
        for (auto& vertex : mesh.vertices) {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }
        
        std::cout << "Using default UV coordinates (mesh has zero size)" << std::endl;
    }
    
    std::cout << "UV coordinates generated for " << mesh.vertices.size() << " vertices" << std::endl;
}

void Mesh::RegenerateBuffers() {
    // Delete existing buffers
    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
    if (VBO != 0) glDeleteBuffers(1, &VBO);
    if (EBO != 0) glDeleteBuffers(1, &EBO);
    
    // Generate new buffers
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    // Bind VAO
    glBindVertexArray(VAO);
    
    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
    
    // Upload index data (triangles only)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, renderIndices.size() * sizeof(unsigned int), &renderIndices[0], GL_STATIC_DRAW);
    
    // Set vertex attributes
    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
    
    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    
    // Texture coordinates
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    
    glBindVertexArray(0);
    
    std::cout << "Regenerated OpenGL buffers for mesh" << std::endl;
}

glm::vec3 ModelLoader::ComputeFaceNormal(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3) {
    glm::vec3 edge1 = v2 - v1;
    glm::vec3 edge2 = v3 - v1;
    glm::vec3 normal = glm::cross(edge1, edge2);
    
    float length = glm::length(normal);
    if (length > 0.0f) {
        return glm::normalize(normal);
    } else {
        // Degenerate triangle - return default normal
        return glm::vec3(0.0f, 0.0f, 1.0f);
    }
}
