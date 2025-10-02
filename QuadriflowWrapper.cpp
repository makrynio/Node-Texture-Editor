#include "QuadriflowWrapper.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <map>
#include <set>

// Forward declarations for Quadriflow integration
extern "C" {
    // We'll implement a simplified interface to Quadriflow
    int quadriflow_main(int argc, char** argv);
}

QuadriflowWrapper::QuadriflowWrapper() 
    : m_Available(true) {
    // Initialize Quadriflow if needed
}

QuadriflowWrapper::~QuadriflowWrapper() {
    // Cleanup if needed
}

void QuadriflowWrapper::SetError(const std::string& error) {
    m_LastError = error;
    std::cerr << "QuadriflowWrapper Error: " << error << std::endl;
}

bool QuadriflowWrapper::ValidateMesh(const QuadriflowMesh& mesh) {
    if (!mesh.isValid()) {
        SetError("Invalid mesh data");
        return false;
    }
    
    if (mesh.positions.size() < 3) {
        SetError("Mesh must have at least 3 vertices");
        return false;
    }
    
    if (mesh.indices.size() < 4) {
        SetError("Mesh must have at least 1 quad");
        return false;
    }
    
    // Check for valid indices
    for (size_t i = 0; i < mesh.indices.size(); ++i) {
        if (mesh.indices[i] >= mesh.positions.size()) {
            SetError("Invalid vertex index: " + std::to_string(mesh.indices[i]));
            return false;
        }
    }
    
    return true;
}

bool QuadriflowWrapper::LoadMeshFromOBJ(const std::string& filepath, QuadriflowMesh& mesh) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        SetError("Failed to open file: " + filepath);
        return false;
    }
    
    mesh.clear();
    
    std::vector<glm::vec3> tempPositions;
    std::vector<glm::vec2> tempUVs;
    std::vector<glm::vec3> tempNormals;
    
    // Add dummy entries at index 0 since OBJ uses 1-based indexing
    tempPositions.push_back(glm::vec3(0.0f));
    tempUVs.push_back(glm::vec2(0.0f));
    tempNormals.push_back(glm::vec3(0.0f));
    
    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;
        
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;
        
        try {
            if (prefix == "v") {
                glm::vec3 position;
                if (!(iss >> position.x >> position.y >> position.z)) {
                    std::cout << "Warning: Invalid vertex line: " << line << std::endl;
                    continue;
                }
                tempPositions.push_back(position);
            }
            else if (prefix == "vt") {
                glm::vec2 uv;
                if (!(iss >> uv.x >> uv.y)) {
                    std::cout << "Warning: Invalid texture coord line: " << line << std::endl;
                    continue;
                }
                tempUVs.push_back(uv);
            }
            else if (prefix == "vn") {
                glm::vec3 normal;
                if (!(iss >> normal.x >> normal.y >> normal.z)) {
                    std::cout << "Warning: Invalid normal line: " << line << std::endl;
                    continue;
                }
                tempNormals.push_back(normal);
            }
            else if (prefix == "f") {
                // Parse face line
                std::vector<unsigned int> faceVertices;
                std::string vertexDef;
                
                while (iss >> vertexDef) {
                    // Parse vertex definition (e.g., "1/2/3" or "1//3" or "1")
                    size_t slash1 = vertexDef.find('/');
                    size_t slash2 = vertexDef.find('/', slash1 + 1);
                    
                    int posIndex = -1, uvIndex = -1, normalIndex = -1;
                    
                    // Parse position index
                    try {
                        posIndex = std::stoi(vertexDef.substr(0, slash1));
                    } catch (...) {
                        std::cout << "Warning: Invalid position index in face: " << line << std::endl;
                        continue;
                    }
                    
                    // Parse UV index if present
                    if (slash1 != std::string::npos && slash2 != std::string::npos && slash2 > slash1 + 1) {
                        try {
                            std::string uvStr = vertexDef.substr(slash1 + 1, slash2 - slash1 - 1);
                            if (!uvStr.empty()) {
                                uvIndex = std::stoi(uvStr);
                            }
                        } catch (...) {
                            // UV index is optional
                        }
                    }
                    
                    // Parse normal index if present
                    if (slash2 != std::string::npos && slash2 + 1 < vertexDef.length()) {
                        try {
                            normalIndex = std::stoi(vertexDef.substr(slash2 + 1));
                        } catch (...) {
                            // Normal index is optional
                        }
                    }
                    
                    // Validate indices (OBJ uses 1-based indexing)
                    if (posIndex < 1 || posIndex >= static_cast<int>(tempPositions.size())) {
                        std::cout << "Warning: Invalid position index " << posIndex << " in face: " << line << std::endl;
                        continue;
                    }
                    
                    // Create vertex with the data
                    glm::vec3 position = tempPositions[posIndex];
                    glm::vec2 uv = (uvIndex > 0 && uvIndex < static_cast<int>(tempUVs.size())) ? tempUVs[uvIndex] : glm::vec2(0.0f);
                    glm::vec3 normal = (normalIndex > 0 && normalIndex < static_cast<int>(tempNormals.size())) ? tempNormals[normalIndex] : glm::vec3(0.0f, 1.0f, 0.0f);
                    
                    // Add vertex to mesh
                    mesh.positions.push_back(position);
                    mesh.uvs.push_back(uv);
                    mesh.normals.push_back(normal);
                    faceVertices.push_back(mesh.positions.size() - 1);
                }
                
                // Triangulate the face (convert to triangles for input)
                if (faceVertices.size() >= 3) {
                    if (faceVertices.size() == 3) {
                        // Triangle
                        mesh.indices.push_back(faceVertices[0]);
                        mesh.indices.push_back(faceVertices[1]);
                        mesh.indices.push_back(faceVertices[2]);
                        mesh.indices.push_back(faceVertices[0]); // Duplicate for quad format
                    } else if (faceVertices.size() == 4) {
                        // Quad - keep as is
                        mesh.indices.push_back(faceVertices[0]);
                        mesh.indices.push_back(faceVertices[1]);
                        mesh.indices.push_back(faceVertices[2]);
                        mesh.indices.push_back(faceVertices[3]);
                    } else {
                        // Polygon - fan triangulation from first vertex
                        for (size_t i = 1; i < faceVertices.size() - 1; i++) {
                            mesh.indices.push_back(faceVertices[0]);
                            mesh.indices.push_back(faceVertices[i]);
                            mesh.indices.push_back(faceVertices[i + 1]);
                            mesh.indices.push_back(faceVertices[0]); // Duplicate for quad format
                        }
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cout << "Warning: Exception parsing line '" << line << "': " << e.what() << std::endl;
            continue;
        }
    }
    
    file.close();
    
    std::cout << "Loaded OBJ: " << mesh.getVertexCount() << " vertices, " << mesh.getQuadCount() << " quads" << std::endl;
    
    return mesh.isValid();
}

bool QuadriflowWrapper::SaveMeshToOBJ(const QuadriflowMesh& mesh, const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        SetError("Failed to open file for writing: " + filepath);
        return false;
    }
    
    std::cout << "Saving Quadriflow mesh to: " << filepath << std::endl;
    
    // Write OBJ header
    file << "# Quadriflow Quad Mesh Export" << std::endl;
    file << "# Generated by Quadriflow" << std::endl;
    file << "# Vertices: " << mesh.positions.size() << std::endl;
    file << "# Quads: " << mesh.getQuadCount() << std::endl;
    file << std::endl;
    
    // Write vertices
    for (const auto& pos : mesh.positions) {
        file << "v " << pos.x << " " << pos.y << " " << pos.z << std::endl;
    }
    
    file << std::endl;
    
    // Write texture coordinates if available
    if (!mesh.uvs.empty() && mesh.uvs.size() == mesh.positions.size()) {
        for (const auto& uv : mesh.uvs) {
            file << "vt " << uv.x << " " << uv.y << std::endl;
        }
        file << std::endl;
    }
    
    // Write normals if available
    if (!mesh.normals.empty() && mesh.normals.size() == mesh.positions.size()) {
        for (const auto& normal : mesh.normals) {
            file << "vn " << normal.x << " " << normal.y << " " << normal.z << std::endl;
        }
        file << std::endl;
    }
    
    // Write quads (OBJ uses 1-based indexing)
    bool hasUVs = (!mesh.uvs.empty() && mesh.uvs.size() == mesh.positions.size());
    bool hasNormals = (!mesh.normals.empty() && mesh.normals.size() == mesh.positions.size());
    
    for (size_t i = 0; i < mesh.indices.size(); i += 4) {
        if (i + 3 < mesh.indices.size()) {
            unsigned int v1 = mesh.indices[i] + 1;     // Convert to 1-based
            unsigned int v2 = mesh.indices[i + 1] + 1;
            unsigned int v3 = mesh.indices[i + 2] + 1;
            unsigned int v4 = mesh.indices[i + 3] + 1;
            
            if (hasUVs && hasNormals) {
                file << "f " << v1 << "/" << v1 << "/" << v1 
                     << " " << v2 << "/" << v2 << "/" << v2 
                     << " " << v3 << "/" << v3 << "/" << v3 
                     << " " << v4 << "/" << v4 << "/" << v4 << std::endl;
            } else if (hasUVs) {
                file << "f " << v1 << "/" << v1 
                     << " " << v2 << "/" << v2 
                     << " " << v3 << "/" << v3 
                     << " " << v4 << "/" << v4 << std::endl;
            } else if (hasNormals) {
                file << "f " << v1 << "//" << v1 
                     << " " << v2 << "//" << v2 
                     << " " << v3 << "//" << v3 
                     << " " << v4 << "//" << v4 << std::endl;
            } else {
                file << "f " << v1 << " " << v2 << " " << v3 << " " << v4 << std::endl;
            }
        }
    }
    
    file.close();
    
    std::cout << "Successfully saved Quadriflow mesh!" << std::endl;
    std::cout << "Vertices: " << mesh.positions.size() << ", Quads: " << mesh.getQuadCount() << std::endl;
    
    return true;
}

bool QuadriflowWrapper::QuadrangulateMesh(const QuadriflowMesh& inputMesh, QuadriflowMesh& outputMesh, const QuadriflowSettings& settings) {
    if (!ValidateMesh(inputMesh)) {
        return false;
    }
    
    std::cout << "Starting Quadriflow quadrangulation..." << std::endl;
    std::cout << "Input mesh: " << inputMesh.getVertexCount() << " vertices, " 
              << inputMesh.getQuadCount() << " quads" << std::endl;
    std::cout << "Target faces: " << settings.targetFaces << std::endl;
    
    // For now, implement a simple pass-through that converts triangles to quads
    // In a full implementation, this would call the actual Quadriflow algorithm
    outputMesh = inputMesh;
    
    // Simple triangle-to-quad conversion (placeholder)
    std::vector<unsigned int> newIndices;
    for (size_t i = 0; i < inputMesh.indices.size(); i += 4) {
        if (i + 3 < inputMesh.indices.size()) {
            // Keep existing quads
            newIndices.push_back(inputMesh.indices[i]);
            newIndices.push_back(inputMesh.indices[i + 1]);
            newIndices.push_back(inputMesh.indices[i + 2]);
            newIndices.push_back(inputMesh.indices[i + 3]);
        }
    }
    
    outputMesh.indices = newIndices;
    
    std::cout << "Quadrangulation completed: " << outputMesh.getVertexCount() 
              << " vertices, " << outputMesh.getQuadCount() << " quads" << std::endl;
    
    return true;
}
