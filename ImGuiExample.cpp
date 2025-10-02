#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_node_editor.h>
#include <fstream>
#include <map>
#include <sstream>
#include "OpenGLView.h"
#include "PBRView.h"
#include "EmbreeView.h"
#include "TriangleRenderer.h"
#include "NodeSystem.h"
#include "parameters/ParametersWindow.h"
#include "RemeshManager.h"

using namespace ax::NodeEditor;

#include <iostream>
#include <algorithm>
#include <cfloat>
#include <cstdlib>
#include <iomanip>
#include <filesystem>
#include <vector>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>

// Include stb_image_write for PNG export
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "external/glfw/deps/stb_image_write.h"

// Include file dialog header
#include "FileDialog.h"

// Helper function to set node position
void SetNodePosition(int nodeId, ImVec2 position) {
    ax::NodeEditor::SetNodePosition(nodeId, position);
}

// Function declarations
void ExportTextures();
void NewProject();
void SaveProject();
void LoadProject();
std::string ShowDirectoryPicker();
std::string ShowSaveFilePicker();
std::string ShowOpenFilePicker();
bool SaveTextureAsPNG(unsigned int textureID, const std::string& filepath, int width, int height);

// Global variable to remember last export location
static std::string g_lastExportPath = "";

// Global variables for save/load paths
static std::string g_lastSavePath = "";
static std::string g_lastLoadPath = "";

// Global flag to indicate nodes need positioning after new project
static bool g_needsNodePositioning = false;

// Global variable to store loaded color node position
static ImVec2 g_loadedColorNodePos = ImVec2(0, 0);

// Global variable to store current color node position for saving
static ImVec2 g_currentColorNodePos = ImVec2(100, 100);

// Global map to store all node positions for saving
static std::map<int, ImVec2> g_allNodePositions;

// Function to show native macOS file browser and get directory path
std::string ShowDirectoryPicker() {
    const char* result = ShowDirectoryPicker(g_lastExportPath.empty() ? nullptr : g_lastExportPath.c_str());
    if (result) {
        std::string path(result);
        free((void*)result); // Free the malloc'd string from C function
        g_lastExportPath = path; // Remember for next time
        return path;
    }
    return "";
}

// Function to show native macOS file picker for saving
std::string ShowSaveFilePicker() {
    const char* result = ShowSaveFilePicker("Save Project", "nod", g_lastSavePath.empty() ? nullptr : g_lastSavePath.c_str());
    if (result) {
        std::string path(result);
        free((void*)result);
        g_lastSavePath = path;
        return path;
    }
    return "";
}

// Function to show native macOS file picker for opening
std::string ShowOpenFilePicker() {
    std::cout << "=== SHOW OPEN FILE PICKER CALLED ===" << std::endl;
    const char* result = ShowOpenFilePicker("Open Project", "nod", g_lastLoadPath.empty() ? nullptr : g_lastLoadPath.c_str());
    if (result) {
        std::string path(result);
        std::cout << "File picker returned: " << path << std::endl;
        free((void*)result);
        g_lastLoadPath = path;
        return path;
    }
    std::cout << "File picker cancelled or failed" << std::endl;
    return "";
}

// Function to save texture as PNG
bool SaveTextureAsPNG(unsigned int textureID, const std::string& filepath, int width, int height) {
    if (textureID == 0) return false;
    
    // Bind the texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // Read texture data
    std::vector<unsigned char> pixels(width * height * 4);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    
    // Convert RGBA to RGB for PNG (remove alpha channel)
    std::vector<unsigned char> rgbPixels(width * height * 3);
    for (int i = 0; i < width * height; i++) {
        rgbPixels[i * 3 + 0] = pixels[i * 4 + 0]; // R
        rgbPixels[i * 3 + 1] = pixels[i * 4 + 1]; // G
        rgbPixels[i * 3 + 2] = pixels[i * 4 + 2]; // B
    }
    
    // Write PNG file using stb_image_write
    int result = stbi_write_png(filepath.c_str(), width, height, 3, rgbPixels.data(), width * 3);
    
    return result != 0;
}

// Main export function
void ExportTextures() {
    // Show directory picker
    std::string exportPath = ShowDirectoryPicker();
    if (exportPath.empty()) {
        std::cout << "Export cancelled by user" << std::endl;
        return;
    }
    
    std::cout << "Exporting textures to: " << exportPath << std::endl;
    
    // Get output textures from NodeSystem
    std::vector<std::pair<std::string, unsigned int>> texturesToExport = {
        {"graphname_color", NodeSystem::GetOutputTexture(ChannelType::Color)},
        {"graphname_roughness", NodeSystem::GetOutputTexture(ChannelType::Roughness)},
        {"graphname_metallic", NodeSystem::GetOutputTexture(ChannelType::Metallic)},
        {"graphname_normal", NodeSystem::GetOutputTexture(ChannelType::Normal)},
        {"graphname_height", NodeSystem::GetOutputTexture(ChannelType::Height)}
    };
    
    int exportedCount = 0;
    for (const auto& texturePair : texturesToExport) {
        const std::string& name = texturePair.first;
        unsigned int textureID = texturePair.second;
        
        if (textureID != 0) {
            std::string filepath = exportPath + "/" + name + ".png";
            
            // Get texture dimensions (assuming 512x512 for now)
            int width = 512, height = 512;
            
            if (SaveTextureAsPNG(textureID, filepath, width, height)) {
                std::cout << "Exported: " << filepath << std::endl;
                exportedCount++;
            } else {
                std::cout << "Failed to export: " << filepath << std::endl;
            }
        } else {
            std::cout << "Skipping " << name << " (no texture data)" << std::endl;
        }
    }
    
    std::cout << "Export complete. " << exportedCount << " textures exported." << std::endl;
}

// Function to save the current project
void SaveProject() {
    std::cout << "Saving project..." << std::endl;
    
    // Show file picker to get save location
    std::string filepath = ShowSaveFilePicker();
    if (filepath.empty()) {
        std::cout << "Save cancelled by user" << std::endl;
        return;
    }
    
    try {
        // Create JSON-like format for saving node data
        std::ofstream file(filepath);
        if (!file.is_open()) {
            std::cout << "Error: Could not create file " << filepath << std::endl;
            return;
        }
        
        // Get all nodes from the node system
        const auto& nodes = NodeSystem::GetNodes();
        std::cout << "SaveProject: Found " << nodes.size() << " nodes to save" << std::endl;
        
        // Write JSON format with all nodes
        file << "{\n";
        file << "  \"version\": \"1.0\",\n";
        file << "  \"nodeCount\": " << nodes.size() << ",\n";
        file << "  \"nodes\": [\n";
        
        bool firstNode = true;
        int savedCount = 0;
        for (const auto& pair : nodes) {
            if (pair.second) {
                if (!firstNode) {
                    file << ",\n";
                }
                firstNode = false;
                
                BaseNode* node = pair.second.get();
                int nodeId = node->GetNodeId();
                std::string nodeName = node->GetName();
                std::string nodeType = nodeName; // Use node name as type for simplicity
                
                // Get node position from stored data
                ImVec2 nodePos = ImVec2(100, 100); // Default position
                
                // Try to get position from stored data
                auto posIt = g_allNodePositions.find(nodeId);
                if (posIt != g_allNodePositions.end()) {
                    nodePos = posIt->second;
                } else {
                    // Fallback to default positions based on node type
                    if (nodeName == "Color") nodePos = ImVec2(100, 100);
                    else if (nodeName == "Roughness") nodePos = ImVec2(100, 200);
                    else if (nodeName == "Metallic") nodePos = ImVec2(100, 300);
                    else if (nodeName == "Normal") nodePos = ImVec2(100, 400);
                    else if (nodeName == "Height") nodePos = ImVec2(100, 500);
                    else if (nodeName == "Normal Map") nodePos = ImVec2(-50, 700);
                    else if (nodeName == "Checker") nodePos = ImVec2(-200, 100);
                    else {
                        // For any other node type, use a default position
                        nodePos = ImVec2(100 + (savedCount * 50), 100 + (savedCount * 50));
                    }
                }
                
                file << "    {\n";
                file << "      \"id\": " << nodeId << ",\n";
                file << "      \"name\": \"" << nodeName << "\",\n";
                file << "      \"type\": \"" << nodeType << "\",\n";
                file << "      \"position\": {\n";
                file << "        \"x\": " << nodePos.x << ",\n";
                file << "        \"y\": " << nodePos.y << "\n";
                file << "      }\n";
                file << "    }";
                
                std::cout << "Saving node " << nodeId << " (" << nodeName << ", " << nodeType << ") at position (" << nodePos.x << ", " << nodePos.y << ")" << std::endl;
                savedCount++;
            }
        }
        
        file << "\n  ],\n";
        file << "  \"savedCount\": " << savedCount << ",\n";
        
        // Save connections/links
        const auto& links = NodeSystem::GetLinks();
        file << "  \"connections\": [\n";
        bool firstConnection = true;
        int savedConnections = 0;
        for (const auto& link : links) {
            if (!firstConnection) {
                file << ",\n";
            }
            firstConnection = false;
            
            file << "    {\n";
            file << "      \"id\": " << reinterpret_cast<uintptr_t>(link.id.AsPointer()) << ",\n";
            file << "      \"inputPin\": " << reinterpret_cast<uintptr_t>(link.inputId.AsPointer()) << ",\n";
            file << "      \"outputPin\": " << reinterpret_cast<uintptr_t>(link.outputId.AsPointer()) << "\n";
            file << "    }";
            
            std::cout << "Saving connection " << reinterpret_cast<uintptr_t>(link.id.AsPointer()) << " from pin " << reinterpret_cast<uintptr_t>(link.outputId.AsPointer()) << " to pin " << reinterpret_cast<uintptr_t>(link.inputId.AsPointer()) << std::endl;
            savedConnections++;
        }
        file << "\n  ],\n";
        file << "  \"savedConnections\": " << savedConnections << "\n";
        file << "}\n";
        
        file.close();
        std::cout << "Project saved successfully to: " << filepath << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error saving project: " << e.what() << std::endl;
    } catch (...) {
        std::cout << "Unknown error saving project" << std::endl;
    }
}

// Function to load a project
void LoadProject() {
    std::cout << "=== LOAD PROJECT CALLED ===" << std::endl;
    std::cout << "Loading project..." << std::endl;
    
    // Show file picker to get load location
    std::string filepath = ShowOpenFilePicker();
    if (filepath.empty()) {
        std::cout << "Load cancelled by user" << std::endl;
        return;
    }
    
    try {
        // First create a new project (like File > New)
        NewProject();
        
        std::cout << "Attempting to load file: " << filepath << std::endl;
        
        // Read the saved file
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cout << "Error: Could not open file " << filepath << std::endl;
            std::cout << "File path length: " << filepath.length() << std::endl;
            return;
        }
        
        std::cout << "File opened successfully, starting to parse..." << std::endl;
        
        // Parse JSON to extract all node data
        std::string line;
        std::map<int, ImVec2> loadedNodePositions;
        std::map<int, std::string> loadedNodeTypes;
        std::map<int, std::string> loadedNodeNames;
        std::vector<NodeLink> loadedConnections;
        int currentNodeId = -1;
        float currentX = 0, currentY = 0;
        std::string currentNodeType = "";
        std::string currentNodeName = "";
        bool inNode = false, inPosition = false, foundX = false, foundY = false;
        
        // Connection parsing variables
        bool inConnections = false, inConnection = false;
        int connectionId = -1, inputPin = -1, outputPin = -1;
        
        while (std::getline(file, line)) {
            // Look for node ID
            if (line.find("\"id\":") != std::string::npos) {
                size_t pos = line.find(":");
                if (pos != std::string::npos) {
                    std::string value = line.substr(pos + 1);
                    value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
                    value.erase(std::remove(value.begin(), value.end(), ','), value.end());
                    try {
                        currentNodeId = std::stoi(value);
                        inNode = true;
                        foundX = false;
                        foundY = false;
                        currentNodeType = "";
                        currentNodeName = "";
                        std::cout << "Found node ID: " << currentNodeId << std::endl;
                    } catch (const std::exception& e) {
                        std::cout << "Error parsing node ID: " << e.what() << std::endl;
                    }
                }
            }
            // Look for node name
            else if (inNode && line.find("\"name\":") != std::string::npos) {
                size_t pos = line.find(":");
                if (pos != std::string::npos) {
                    std::string value = line.substr(pos + 1);
                    value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
                    value.erase(std::remove(value.begin(), value.end(), '"'), value.end());
                    value.erase(std::remove(value.begin(), value.end(), ','), value.end());
                    currentNodeName = value;
                    std::cout << "Found node name: " << currentNodeName << std::endl;
                }
            }
            // Look for node type
            else if (inNode && line.find("\"type\":") != std::string::npos) {
                size_t pos = line.find(":");
                if (pos != std::string::npos) {
                    std::string value = line.substr(pos + 1);
                    value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
                    value.erase(std::remove(value.begin(), value.end(), '"'), value.end());
                    value.erase(std::remove(value.begin(), value.end(), ','), value.end());
                    currentNodeType = value;
                    std::cout << "Found node type: " << currentNodeType << std::endl;
                }
            }
            // Look for position object
            else if (inNode && line.find("\"position\":") != std::string::npos) {
                inPosition = true;
            }
            // Look for x coordinate
            else if (inPosition && line.find("\"x\":") != std::string::npos) {
                size_t pos = line.find(":");
                if (pos != std::string::npos) {
                    std::string value = line.substr(pos + 1);
                    value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
                    value.erase(std::remove(value.begin(), value.end(), ','), value.end());
                    try {
                        currentX = std::stof(value);
                        foundX = true;
                        std::cout << "Parsed x coordinate: " << currentX << std::endl;
                    } catch (const std::exception& e) {
                        std::cout << "Error parsing x coordinate: " << e.what() << std::endl;
                    }
                }
            }
            // Look for y coordinate
            else if (inPosition && line.find("\"y\":") != std::string::npos) {
                size_t pos = line.find(":");
                if (pos != std::string::npos) {
                    std::string value = line.substr(pos + 1);
                    value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
                    value.erase(std::remove(value.begin(), value.end(), ','), value.end());
                    try {
                        currentY = std::stof(value);
                        foundY = true;
                        std::cout << "Parsed y coordinate: " << currentY << std::endl;
                    } catch (const std::exception& e) {
                        std::cout << "Error parsing y coordinate: " << e.what() << std::endl;
                    }
                }
            }
            // Check if we've completed a node (look for closing brace of node object)
            else if (inNode && line.find("    }") != std::string::npos) {
                if (foundX && foundY && currentNodeId != -1) {
                    loadedNodePositions[currentNodeId] = ImVec2(currentX, currentY);
                    if (!currentNodeType.empty()) {
                        loadedNodeTypes[currentNodeId] = currentNodeType;
                    }
                    if (!currentNodeName.empty()) {
                        loadedNodeNames[currentNodeId] = currentNodeName;
                    }
                    std::cout << "Stored node " << currentNodeId << " (" << currentNodeName << ", " << currentNodeType << ") at position (" << currentX << ", " << currentY << ")" << std::endl;
                }
                inNode = false;
                inPosition = false;
                foundX = false;
                foundY = false;
                currentNodeId = -1;
                currentNodeType = "";
                currentNodeName = "";
            }
            // Look for connections array
            else if (line.find("\"connections\":") != std::string::npos) {
                inConnections = true;
                std::cout << "Found connections array" << std::endl;
            }
            // Look for connection ID
            else if (inConnections && line.find("\"id\":") != std::string::npos && !inNode) {
                size_t pos = line.find(":");
                if (pos != std::string::npos) {
                    std::string value = line.substr(pos + 1);
                    value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
                    value.erase(std::remove(value.begin(), value.end(), ','), value.end());
                    try {
                        connectionId = std::stoi(value);
                        inConnection = true;
                        inputPin = -1;
                        outputPin = -1;
                        std::cout << "Found connection ID: " << connectionId << std::endl;
                    } catch (const std::exception& e) {
                        std::cout << "Error parsing connection ID: " << e.what() << std::endl;
                    }
                }
            }
            // Look for input pin
            else if (inConnection && line.find("\"inputPin\":") != std::string::npos) {
                size_t pos = line.find(":");
                if (pos != std::string::npos) {
                    std::string value = line.substr(pos + 1);
                    value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
                    value.erase(std::remove(value.begin(), value.end(), ','), value.end());
                    try {
                        inputPin = std::stoi(value);
                        std::cout << "Found input pin: " << inputPin << std::endl;
                    } catch (const std::exception& e) {
                        std::cout << "Error parsing input pin: " << e.what() << std::endl;
                    }
                }
            }
            // Look for output pin
            else if (inConnection && line.find("\"outputPin\":") != std::string::npos) {
                size_t pos = line.find(":");
                if (pos != std::string::npos) {
                    std::string value = line.substr(pos + 1);
                    value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
                    value.erase(std::remove(value.begin(), value.end(), ','), value.end());
                    try {
                        outputPin = std::stoi(value);
                        std::cout << "Found output pin: " << outputPin << std::endl;
                    } catch (const std::exception& e) {
                        std::cout << "Error parsing output pin: " << e.what() << std::endl;
                    }
                }
            }
            // Check if we've completed a connection (look for closing brace of connection object)
            else if (inConnection && line.find("    }") != std::string::npos) {
                if (connectionId != -1 && inputPin != -1 && outputPin != -1) {
                    loadedConnections.emplace_back(connectionId, inputPin, outputPin);
                    std::cout << "Stored connection " << connectionId << " from pin " << outputPin << " to pin " << inputPin << std::endl;
                }
                inConnection = false;
                connectionId = -1;
                inputPin = -1;
                outputPin = -1;
            }
        }
        
        file.close();
        
        std::cout << "Loaded " << loadedNodePositions.size() << " node positions" << std::endl;
        for (const auto& pair : loadedNodePositions) {
            std::cout << "  - Node " << pair.first << " at (" << pair.second.x << ", " << pair.second.y << ")" << std::endl;
        }
        
        std::cout << "Loaded " << loadedConnections.size() << " connections" << std::endl;
        for (const auto& connection : loadedConnections) {
            std::cout << "  - Connection " << reinterpret_cast<uintptr_t>(connection.id.AsPointer()) << " from pin " << reinterpret_cast<uintptr_t>(connection.outputId.AsPointer()) << " to pin " << reinterpret_cast<uintptr_t>(connection.inputId.AsPointer()) << std::endl;
        }
        
        if (!loadedNodePositions.empty()) {
            // Create any additional nodes that aren't in the default set
            for (const auto& pair : loadedNodePositions) {
                int nodeId = pair.first;
                std::string nodeType = "";
                std::string nodeName = "";
                
                // Get node type and name if available
                auto typeIt = loadedNodeTypes.find(nodeId);
                if (typeIt != loadedNodeTypes.end()) {
                    nodeType = typeIt->second;
                }
                auto nameIt = loadedNodeNames.find(nodeId);
                if (nameIt != loadedNodeNames.end()) {
                    nodeName = nameIt->second;
                }
                
                // Create nodes that aren't in the default set (IDs 1-9)
                if (nodeId > 9) {
                    std::cout << "Creating additional node ID " << nodeId << " (" << nodeName << ", " << nodeType << ")..." << std::endl;
                    
                    // Create appropriate node type based on parsed data
                    if (nodeType == "Checker" || nodeName == "Checker") {
                        NodeSystem::CreateTextureNode();
                    } else if (nodeType == "Blend" || nodeName == "Blend") {
                        NodeSystem::CreateBlendNode();
                    } else if (nodeType == "FractalNoise" || nodeName == "Fractal Noise") {
                        NodeSystem::CreateFractalNoiseNode();
                    } else if (nodeType == "Gradient" || nodeName == "Gradient") {
                        // Create gradient node - need to add this function
                        std::cout << "Gradient node type not yet implemented, creating texture node instead" << std::endl;
                        NodeSystem::CreateTextureNode();
                    } else {
                        // Default fallback for unknown node types
                        std::cout << "Unknown node type '" << nodeType << "', creating texture node as fallback" << std::endl;
                        NodeSystem::CreateTextureNode();
                    }
                }
            }
            
            // Store loaded positions globally
            g_allNodePositions = loadedNodePositions;
            
            // Set flag to position nodes at loaded positions
            g_needsNodePositioning = true;
            
            // For backward compatibility, set color node position if available
            auto colorIt = loadedNodePositions.find(1);
            if (colorIt != loadedNodePositions.end()) {
                g_loadedColorNodePos = colorIt->second;
            }
            
            // Restore connections after nodes are created and positioned
            if (!loadedConnections.empty()) {
                std::cout << "Restoring " << loadedConnections.size() << " connections..." << std::endl;
                for (const auto& connection : loadedConnections) {
                    // Add the connection to the node system
                    NodeSystem::RestoreConnection(connection);
                    std::cout << "Restored connection " << reinterpret_cast<uintptr_t>(connection.id.AsPointer()) << " from pin " << reinterpret_cast<uintptr_t>(connection.outputId.AsPointer()) << " to pin " << reinterpret_cast<uintptr_t>(connection.inputId.AsPointer()) << std::endl;
                }
                
                // Update node connections to propagate texture data
                NodeSystem::RebuildConnectionChain();
                std::cout << "Connection chain rebuilt successfully" << std::endl;
            }
            
            std::cout << "Project loaded successfully from: " << filepath << std::endl;
            std::cout << "All nodes will be positioned at their saved locations" << std::endl;
        } else {
            std::cout << "Error: Could not parse any node positions from file" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cout << "Error loading project: " << e.what() << std::endl;
    } catch (...) {
        std::cout << "Unknown error loading project" << std::endl;
    }
}

// Function to create a new empty graph
void NewProject() {
    std::cout << "Creating new project..." << std::endl;
    
    try {
        // Clear all existing nodes and links
        NodeSystem::Cleanup();
        std::cout << "Cleared all existing nodes and links" << std::endl;
        
        // Clear all stored node positions to force reset to defaults
        g_allNodePositions.clear();
        g_currentColorNodePos = ImVec2(100, 100); // Reset to default
        g_loadedColorNodePos = ImVec2(0, 0); // Clear loaded position
        
        // Set the exact initial positions for all nodes (including normal map)
        g_allNodePositions[1] = ImVec2(100, 100);  // Color
        g_allNodePositions[2] = ImVec2(100, 200);  // Roughness  
        g_allNodePositions[3] = ImVec2(100, 300);  // Metallic
        g_allNodePositions[4] = ImVec2(100, 400);  // Normal
        g_allNodePositions[5] = ImVec2(100, 500);  // Height
        g_allNodePositions[9] = ImVec2(-50, 700);  // Normal Map
        
        // Create fresh startup nodes (same as initial startup)
        std::cout << "Creating output nodes..." << std::endl;
        NodeSystem::CreateOutputNodes();
        
        std::cout << "Creating normal map node..." << std::endl;
        NodeSystem::CreateNormalMapNode();
        
        // Set flag to position nodes in the main loop where node editor context is active
        g_needsNodePositioning = true;
        
        std::cout << "New project created successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error creating new project: " << e.what() << std::endl;
    } catch (...) {
        std::cout << "Unknown error creating new project" << std::endl;
    }
}

// QuadriFlow functions removed - keeping GUI elements for later implementation


// Function to export sphere mesh to OBJ format
void ExportSphereToOBJ(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to create OBJ file: " << filename << std::endl;
        return;
    }
    
    const unsigned int X_SEGMENTS = 64;
    const unsigned int Y_SEGMENTS = 64;
    const float PI = 3.14159265359f;
    
    // Generate vertices
    for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
        for (unsigned int y = 0; y <= Y_SEGMENTS; ++y) {
            float xSegment = (float)x / (float)X_SEGMENTS;
            float ySegment = (float)y / (float)Y_SEGMENTS;
            float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            float yPos = std::cos(ySegment * PI);
            float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            
            file << "v " << xPos << " " << yPos << " " << zPos << std::endl;
        }
    }
    
    // Generate faces
    bool oddRow = false;
    for (unsigned int y = 0; y < Y_SEGMENTS; ++y) {
        if (!oddRow) {
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
                unsigned int current = y * (X_SEGMENTS + 1) + x;
                unsigned int next = (y + 1) * (X_SEGMENTS + 1) + x;
                
                if (x < X_SEGMENTS) {
                    file << "f " << current + 1 << " " << next + 1 << " " << current + 2 << std::endl;
                    file << "f " << current + 2 << " " << next + 1 << " " << next + 2 << std::endl;
                }
            }
        } else {
            for (int x = X_SEGMENTS; x >= 0; --x) {
                unsigned int current = (y + 1) * (X_SEGMENTS + 1) + x;
                unsigned int next = y * (X_SEGMENTS + 1) + x;
                
                if (x > 0) {
                    file << "f " << current + 1 << " " << next + 1 << " " << current - 1 << std::endl;
                    file << "f " << current - 1 << " " << next + 1 << " " << next - 1 << std::endl;
                }
            }
        }
        oddRow = !oddRow;
    }
    
    file.close();
    std::cout << "Sphere exported to: " << filename << std::endl;
}

// Function declarations
// QuadriFlow function declarations removed - keeping GUI elements for later implementation





// callback functions for GLFW that will be used to resize the window
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void file_drop_callback(GLFWwindow* window, int count, const char* paths[]);

// Global view pointers for file drop callback
PBRView* g_pbrView = nullptr;
EmbreeView* g_embreeView = nullptr;

// settings for the window
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;

void HandleNodeSelection(ax::NodeEditor::EditorContext* editor) {
    using namespace ax::NodeEditor;
    
    // Handle selection changes from NodeEditor
    if (HasSelectionChanged()) {
        // Get selected nodes from NodeEditor
        int selectedCount = GetSelectedObjectCount();
        
        if (selectedCount > 0) {
            NodeId* selectedNodes = new NodeId[selectedCount];
            LinkId* selectedLinks = new LinkId[selectedCount];
            
            int nodeCount = GetSelectedNodes(selectedNodes, selectedCount);
            int linkCount = GetSelectedLinks(selectedLinks, selectedCount);
            
            // Update our internal selection
            NodeSystem::ClearSelection();
            for (int i = 0; i < nodeCount; i++) {
                NodeSystem::AddToSelection(selectedNodes[i].Get());
            }
            
            delete[] selectedNodes;
            delete[] selectedLinks;
        } else {
            NodeSystem::ClearSelection();
        }
    }
    
    // Handle deletion using NodeEditor's BeginDelete/EndDelete pattern
    if (BeginDelete()) {
        LinkId linkId;
        while (QueryDeletedLink(&linkId)) {
            // Remove the link from our data structures
            NodeSystem::DeleteLink(linkId);
            AcceptDeletedItem();
        }
        
        NodeId nodeId;
        while (QueryDeletedNode(&nodeId)) {
            // Remove the node from our data structures
            NodeSystem::DeleteNode(nodeId.Get());
            AcceptDeletedItem();
        }
        
        EndDelete();
    }
}

// main function
int main()
{
    std::cout << "Starting ImGui Example..." << std::endl;
    
    // glfw: initialize and configure the window
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

// for Apple platform, we need to enable forward compatibility
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    std::cout << "Creating GLFW window..." << std::endl;
    
    // Load window configuration
    int windowX = 100, windowY = 100;
    int windowWidth = SCR_WIDTH, windowHeight = SCR_HEIGHT;
    bool maximized = false;
    
    // Try to load window config from file
    std::ifstream configFile("window_config.txt");
    if (configFile.is_open()) {
        configFile >> windowX >> windowY >> windowWidth >> windowHeight >> maximized;
        configFile.close();
        std::cout << "Loaded window config: " << windowX << ", " << windowY << ", " << windowWidth << "x" << windowHeight << ", maximized: " << maximized << std::endl;
    }
    
    // Set window hints
    if (maximized) {
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    }
    
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "ImGui + Node Editor Example", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    std::cout << "GLFW window created successfully" << std::endl;
    glfwMakeContextCurrent(window);
    
    // Set window position if not maximized
    if (!maximized) {
        glfwSetWindowPos(window, windowX, windowY);
    }
    
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetDropCallback(window, file_drop_callback);

    // glad: load all OpenGL function pointers to use OpenGL functions
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    
    // Set specific configuration file for this project to avoid conflicts
    io.IniFilename = "/Users/thomasmakryniotis/Desktop/Quadriflow/imgui.ini";
    
    // Clear any existing configuration to start fresh
    // ImGui::ClearIniSettings(); // This function doesn't exist in this version
    
    std::cout << "ImGui configuration file set to: " << io.IniFilename << std::endl;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Setup Node Editor
    Config config;
    config.SettingsFile = "/Users/thomasmakryniotis/Desktop/Quadriflow/nodeeditor.json";
    std::cout << "Node Editor configuration file set to: " << config.SettingsFile << std::endl;
    auto* nodeEditor = CreateEditor(&config);
    SetCurrentEditor(nodeEditor);
    
    // Enable shortcuts for selection and deletion
    EnableShortcuts(true);


    // Setup OpenGL View
    OpenGLView openglView;
    PBRView pbrView;
    EmbreeView embreeView;
    TriangleRenderer triangleRenderer;
    
    // Set global view pointers for file drop callback
    g_pbrView = &pbrView;
    g_embreeView = &embreeView;
    
    // Create parameters window
    ParametersWindow parametersWindow;
    
    if (!openglView.Initialize(800, 600))
    {
        std::cout << "Failed to initialize OpenGL view" << std::endl;
        return -1;
    }
    
    if (!pbrView.Initialize(800, 600))
    {
        std::cout << "Failed to initialize PBR view" << std::endl;
        return -1;
    }
    
    if (!embreeView.Initialize(800, 600))
    {
        std::cout << "Failed to initialize Embree ray-traced view" << std::endl;
        return -1;
    }
    
    if (!triangleRenderer.Initialize())
    {
        std::cout << "Failed to initialize triangle renderer" << std::endl;
        return -1;
    }
    
    // Set up the triangle render callback
    openglView.SetRenderCallback([&triangleRenderer]() {
        triangleRenderer.Render();
    });

    // Our state
    bool show_demo_window = false;
    bool show_opengl_view = true;
    bool show_pbr_view = true;
    bool show_embree_view = false;
    bool show_outliner = true;
    bool show_remesh_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    std::cout << "About to start main render loop..." << std::endl;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Debug frame counter
        static int frame_count = 0;
        frame_count++;

        // Create the main docking space first
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGuiID dockspace_id = ImGui::DockSpaceOverViewport(0, viewport, ImGuiDockNodeFlags_PassthruCentralNode);

        // Main menu bar (using BeginMainMenuBar for global menu)
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New")) {
                    NewProject();
                }
                if (ImGui::MenuItem("Open")) {
                    LoadProject();
                }
                if (ImGui::MenuItem("Save")) {
                    SaveProject();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Export Textures")) {
                    // Export textures functionality will be implemented below
                    ExportTextures();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) { glfwSetWindowShouldClose(window, true); }
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("View"))
            {
                ImGui::MenuItem("Demo Window", nullptr, &show_demo_window);
                ImGui::MenuItem("OpenGL View", nullptr, &show_opengl_view);
                ImGui::MenuItem("PBR Material View", nullptr, &show_pbr_view);
                ImGui::MenuItem("Ray-Traced View", nullptr, &show_embree_view);
                ImGui::MenuItem("Outliner", nullptr, &show_outliner);
                ImGui::MenuItem("Remesh", nullptr, &show_remesh_window);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
        
        if (frame_count % 60 == 0) {
            std::cout << "Main loop running, frame: " << frame_count << std::endl;
        }
        
        // Debug: Check if we reach this point
        if (frame_count <= 5) {
            std::cout << "Frame " << frame_count << " - after ImGui::NewFrame()" << std::endl;
        }


        // Create DockSpace
        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
        ImGuiWindowFlags window_flags2 = ImGuiWindowFlags_NoDocking;
        
        const ImGuiViewport* viewport2 = ImGui::GetMainViewport();
        // Adjust for menu bar height - use WorkPos but offset by menu bar height
        ImVec2 dockspace_pos = viewport2->WorkPos;
        ImVec2 dockspace_size = viewport2->WorkSize;
        
        // Account for menu bar height
        float menu_bar_height = ImGui::GetFrameHeight();
        dockspace_pos.y += menu_bar_height;
        dockspace_size.y -= menu_bar_height;
        
        ImGui::SetNextWindowPos(dockspace_pos);
        ImGui::SetNextWindowSize(dockspace_size);
        ImGui::SetNextWindowViewport(viewport2->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags2 |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags2 |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", nullptr, window_flags2);
        ImGui::PopStyleVar();
        ImGui::PopStyleVar(2);

        // DockSpace
        ImGuiID dockspace_id2 = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id2, ImVec2(0.0f, 0.0f), dockspace_flags);

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // Remesh window - QuadriFlow functionality implemented
        if (show_remesh_window)
        {
            ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(450, 500), ImGuiCond_FirstUseEver);
            ImGui::Begin("Remesh", &show_remesh_window, ImGuiWindowFlags_AlwaysAutoResize);
            
            ImGui::Text("Mesh Remeshing Tool");
            ImGui::Separator();
            
            // Show current model info
            ImGui::Text("Current Model: %s", pbrView.GetModelInfo().c_str());
            ImGui::Separator();
            
            // Check if remesh is available
            if (!pbrView.IsRemeshAvailable()) {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Remesh functionality not available");
                ImGui::Text("Please check that the manifold library is properly linked.");
                ImGui::End();
                continue;
            }
            
            // Check if Quadriflow is available
            if (!pbrView.GetQuadriflowWrapper().IsAvailable()) {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Quadriflow functionality not available");
                ImGui::Text("Quadriflow quadrangulation is not available.");
                ImGui::End();
                continue;
            }
            
            // Get remesh manager and settings
            auto& remeshManager = pbrView.GetRemeshManager();
            auto settings = remeshManager.GetSettings();
            
            // Quadriflow Settings
            ImGui::Text("Quadriflow Settings");
            ImGui::Separator();
            
            // Target quad count setting
            if (ImGui::SliderInt("Target Quads", &settings.targetQuads, 1000, 50000, "%d")) {
                pbrView.GetRemeshManager().SetSettings(settings);
            }
            ImGui::SameLine();
            if (ImGui::Button("?")) {
                ImGui::SetTooltip("Target number of quad faces for Quadriflow remeshing.\nRange: 1000-50000\nRecommended: 8000-15000 for most models.\nOptimal: 8000 for quality/performance balance.");
            }
            
            
            ImGui::Separator();
            
            
            ImGui::Separator();
            
            // Action buttons
            ImGui::Text("Actions");
            ImGui::Separator();
            
            // Remesh button
            if (ImGui::Button("Remesh Current Model", ImVec2(200, 30))) {
                std::cout << "Starting remesh process..." << std::endl;
                
                // Update settings
                remeshManager.SetSettings(settings);
                
                // Perform remeshing
                if (pbrView.RemeshCurrentModel(settings)) {
                    std::cout << "Remeshing completed successfully!" << std::endl;
                } else {
                    std::cout << "Remeshing failed!" << std::endl;
                }
            }
            
            
            
            ImGui::SameLine();
            if (ImGui::Button("Revert to Original", ImVec2(150, 30))) {
                std::cout << "Reverting to original model..." << std::endl;
                pbrView.RevertToOriginalModel();
            }
            
            // Status and info
            ImGui::Separator();
            ImGui::Text("Status: Ready");
            ImGui::Text("Remesh functionality: Available");
            
            // Show last error if any
            const std::string& lastError = remeshManager.GetLastError();
            if (!lastError.empty()) {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Last Error: %s", lastError.c_str());
            }
            
            ImGui::End();
        }



        // 4. Show Node Editor
        {
            ImGui::Begin("Node Editor");
            
            SetCurrentEditor(nodeEditor);
            Begin("My Editor", ImVec2(0.0f, 0.0f));
            
            // Handle node positioning after new project
            if (g_needsNodePositioning) {
                std::cout << "Positioning nodes after new project..." << std::endl;
                
                // Clear node editor selection first
                ax::NodeEditor::ClearSelection();
                
                // Position all nodes using loaded positions or initial defaults
                const auto& nodes = NodeSystem::GetNodes();
                std::cout << "Positioning " << nodes.size() << " nodes..." << std::endl;
                std::cout << "Available loaded positions: " << g_allNodePositions.size() << std::endl;
                
                for (const auto& pair : nodes) {
                    if (pair.second) {
                        int nodeId = pair.second->GetNodeId();
                        std::string nodeName = pair.second->GetName();
                        ImVec2 pos;
                        
                        // Check if we have a loaded position for this node
                        auto loadedPosIt = g_allNodePositions.find(nodeId);
                        if (loadedPosIt != g_allNodePositions.end()) {
                            pos = loadedPosIt->second;
                            std::cout << "Using loaded position for node " << nodeId << " (" << nodeName << "): (" << pos.x << ", " << pos.y << ")" << std::endl;
                        } else {
                            // Use initial position for new project
                            pos = NodeSystem::GetInitialOutputNodePosition(nodeId);
                            if (pos.x >= FLT_MAX || pos.y >= FLT_MAX) {
                                // Fallback for non-output nodes
                                if (nodeName == "Normal Map") pos = ImVec2(-50, 700);
                                else if (nodeName == "Checker") pos = ImVec2(-200, 100);
                                else pos = ImVec2(100, 100);
                            }
                            std::cout << "Using initial/fallback position for node " << nodeId << " (" << nodeName << "): (" << pos.x << ", " << pos.y << ")" << std::endl;
                        }
                        
                        ax::NodeEditor::SetNodePosition(nodeId, pos);
                        
                        // Update stored position
                        g_allNodePositions[nodeId] = pos;
                        if (nodeId == 1) {
                            g_currentColorNodePos = pos; // Keep for backward compatibility
                        }
                    }
                }
                
                // Create default normal connection
                std::cout << "Creating default normal connection..." << std::endl;
                NodeSystem::CreateDefaultNormalConnection();
                
                g_needsNodePositioning = false;
                g_loadedColorNodePos = ImVec2(0, 0); // Reset loaded position
                std::cout << "Node positioning complete!" << std::endl;
            }
            
            // Periodically update stored node positions for saving
            static int positionUpdateCounter = 0;
            positionUpdateCounter++;
            if (positionUpdateCounter % 60 == 0) { // Update every 60 frames
                try {
                    // Update all node positions
                    const auto& nodes = NodeSystem::GetNodes();
                    for (const auto& pair : nodes) {
                        if (pair.second) {
                            int nodeId = pair.second->GetNodeId();
                            ImVec2 currentPos = ax::NodeEditor::GetNodePosition(nodeId);
                            if (currentPos.x != 0 || currentPos.y != 0) { // Valid position
                                g_allNodePositions[nodeId] = currentPos;
                                if (nodeId == 1) {
                                    g_currentColorNodePos = currentPos; // Keep for backward compatibility
                                }
                            }
                        }
                    }
                } catch (...) {
                    // Ignore errors - positions will be updated next time
                }
            }
            
            // Register nodes if not already registered
            static bool nodesRegistered = false;
            static bool initialPositionsSet = false;
            if (!nodesRegistered) {
                // Create output nodes (5 different channels)
                NodeSystem::CreateOutputNodes();
                
                // Create normal map node
                NodeSystem::CreateNormalMapNode();
                
                nodesRegistered = true;
            }
            
            // Set initial positions of output nodes after they're created
            if (nodesRegistered && !initialPositionsSet) {
                // Position the output nodes at their initial positions
                for (int i = 1; i <= 5; i++) {
                    ImVec2 pos = NodeSystem::GetInitialOutputNodePosition(i);
                    if (pos.x < FLT_MAX && pos.y < FLT_MAX) {
                        ax::NodeEditor::SetNodePosition(i, pos);
                    }
                }
                initialPositionsSet = true;
                
                // Position the normal map node to the left of the normal output
                ax::NodeEditor::SetNodePosition(9, ImVec2(-50, 700)); // Normal map node
                
                // Create default normal connection
                NodeSystem::CreateDefaultNormalConnection();
            }
            
            // Render all nodes dynamically with error handling
            try {
                const auto& nodes = NodeSystem::GetNodes();
                static int frameCount = 0;
                frameCount++;
                
                // Debug: Print node count every 60 frames (about once per second)
                if (frameCount % 60 == 0) {
                    std::cout << "Frame " << frameCount << ": Rendering " << nodes.size() << " nodes" << std::endl;
                    for (const auto& pair : nodes) {
                        if (pair.second) {
                            std::cout << "  - Node ID " << pair.first << ": " << pair.second->GetName() << std::endl;
                        }
                    }
                }
                
                for (const auto& pair : nodes) {
                    if (pair.second) {
                        NodeSystem::RenderNode(pair.first, nodeEditor);
                    }
                }
            } catch (const std::exception& e) {
                std::cout << "Error rendering nodes: " << e.what() << std::endl;
            } catch (...) {
                std::cout << "Unknown error rendering nodes" << std::endl;
            }
            
            // Render connections with error handling
            try {
                NodeSystem::RenderConnections(nodeEditor);
                NodeSystem::HandleConnections(nodeEditor);
            } catch (const std::exception& e) {
                std::cout << "Error handling connections: " << e.what() << std::endl;
            } catch (...) {
                std::cout << "Unknown error handling connections" << std::endl;
            }
            
            // Handle multiple selection and deletion with error handling
            try {
                HandleNodeSelection(nodeEditor);
            } catch (const std::exception& e) {
                std::cout << "Error handling node selection: " << e.what() << std::endl;
            } catch (...) {
                std::cout << "Unknown error handling node selection" << std::endl;
            }
            
            // Handle context menus using proper NodeEditor API
            static NodeId contextNodeId = 0;
            static LinkId contextLinkId = 0;
            static PinId contextPinId = 0;
            auto openPopupPosition = ImGui::GetMousePos();
            
            Suspend();
            if (ShowNodeContextMenu(&contextNodeId)) {
                ImGui::OpenPopup("Node Context Menu");
            }
            else if (ShowPinContextMenu(&contextPinId)) {
                ImGui::OpenPopup("Pin Context Menu");
            }
            else if (ShowLinkContextMenu(&contextLinkId)) {
                ImGui::OpenPopup("Link Context Menu");
            }
            else if (ShowBackgroundContextMenu()) {
                ImGui::OpenPopup("Create New Node");
            }
            Resume();
            
            Suspend();
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
            
            if (ImGui::BeginPopup("Node Context Menu")) {
                ImGui::Text("Node Options");
                ImGui::Separator();
                if (ImGui::MenuItem("Delete")) {
                    DeleteNode(contextNodeId);
                    NodeSystem::DeleteNode(contextNodeId.Get());
                }
                ImGui::EndPopup();
            }
            
            if (ImGui::BeginPopup("Pin Context Menu")) {
                ImGui::Text("Pin Options");
                ImGui::EndPopup();
            }
            
            if (ImGui::BeginPopup("Link Context Menu")) {
                ImGui::Text("Link Options");
                ImGui::Separator();
                if (ImGui::MenuItem("Delete")) {
                    DeleteLink(contextLinkId);
                    NodeSystem::DeleteLink(contextLinkId);
                }
                ImGui::EndPopup();
            }
            
            if (ImGui::BeginPopup("Create New Node")) {
                // Get the current mouse position in canvas coordinates
                ImVec2 mousePos = ImGui::GetMousePos();
                ImVec2 canvasPos = ScreenToCanvas(mousePos);
                ImGui::Text("Create Node");
                ImGui::Separator();
                
                // Debug: Show all menu items
                static int popupCount = 0;
                if (++popupCount % 60 == 0) { // Only print every 60 frames
                    std::cout << "Rendering Create New Node popup (frame " << popupCount << ")" << std::endl;
                }
                
                if (ImGui::MenuItem("Output - Color")) {
                    int nodeId = NodeSystem::CreateNode("output", "color");
                    SetNodePosition(nodeId, canvasPos);
                    NodeSystem::ClearSelection();
                    NodeSystem::SetSelectedNode(nodeId);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Output - Roughness")) {
                    int nodeId = NodeSystem::CreateNode("output", "roughness");
                    SetNodePosition(nodeId, canvasPos);
                    NodeSystem::ClearSelection();
                    NodeSystem::SetSelectedNode(nodeId);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Output - Metallic")) {
                    int nodeId = NodeSystem::CreateNode("output", "metallic");
                    SetNodePosition(nodeId, canvasPos);
                    NodeSystem::ClearSelection();
                    NodeSystem::SetSelectedNode(nodeId);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Output - Normal")) {
                    int nodeId = NodeSystem::CreateNode("output", "normal");
                    SetNodePosition(nodeId, canvasPos);
                    NodeSystem::ClearSelection();
                    NodeSystem::SetSelectedNode(nodeId);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Output - Height")) {
                    int nodeId = NodeSystem::CreateNode("output", "height");
                    SetNodePosition(nodeId, canvasPos);
                    NodeSystem::ClearSelection();
                    NodeSystem::SetSelectedNode(nodeId);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Checker Texture")) {
                    int nodeId = NodeSystem::CreateNode("texture");
                    SetNodePosition(nodeId, canvasPos);
                    NodeSystem::ClearSelection();
                    NodeSystem::SetSelectedNode(nodeId);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Blend")) {
                    int nodeId = NodeSystem::CreateNode("blend");
                    SetNodePosition(nodeId, canvasPos);
                    NodeSystem::ClearSelection();
                    NodeSystem::SetSelectedNode(nodeId);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Fractal Noise")) {
                    int nodeId = NodeSystem::CreateNode("fractal_noise");
                    SetNodePosition(nodeId, canvasPos);
                    NodeSystem::ClearSelection();
                    NodeSystem::SetSelectedNode(nodeId);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Gradient")) {
                    int nodeId = NodeSystem::CreateNode("gradient");
                    SetNodePosition(nodeId, canvasPos);
                    NodeSystem::ClearSelection();
                    NodeSystem::SetSelectedNode(nodeId);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("TriGradient")) {
                    int nodeId = NodeSystem::CreateNode("trigradient");
                    SetNodePosition(nodeId, canvasPos);
                    NodeSystem::ClearSelection();
                    NodeSystem::SetSelectedNode(nodeId);
                    ImGui::CloseCurrentPopup();
                }
                std::cout << "About to render Mirror menu item" << std::endl;
                if (ImGui::MenuItem("Mirror")) {
                    std::cout << "Creating Mirror node" << std::endl;
                    int nodeId = NodeSystem::CreateNode("mirror");
                    SetNodePosition(nodeId, canvasPos);
                    NodeSystem::ClearSelection();
                    NodeSystem::SetSelectedNode(nodeId);
                    ImGui::CloseCurrentPopup();
                }
                std::cout << "Mirror menu item rendered" << std::endl;
                std::cout << "About to render Normal Map menu item" << std::endl;
                if (ImGui::MenuItem("Normal Map")) {
                    std::cout << "Creating Normal Map node" << std::endl;
                    int nodeId = NodeSystem::CreateNode("normalmap");
                    std::cout << "Normal Map node created with ID: " << nodeId << std::endl;
                    SetNodePosition(nodeId, canvasPos);
                    std::cout << "Normal Map node positioned at: (" << canvasPos.x << ", " << canvasPos.y << ")" << std::endl;
                    NodeSystem::ClearSelection();
                    NodeSystem::SetSelectedNode(nodeId);
                    std::cout << "Normal Map node selected" << std::endl;
                    ImGui::CloseCurrentPopup();
                }
                std::cout << "Normal Map menu item rendered" << std::endl;
                if (ImGui::MenuItem("Warp")) {
                    int nodeId = NodeSystem::CreateNode("warp");
                    SetNodePosition(nodeId, canvasPos);
                    NodeSystem::ClearSelection();
                    NodeSystem::SetSelectedNode(nodeId);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Better Warp")) {
                    int nodeId = NodeSystem::CreateNode("betterwarp");
                    SetNodePosition(nodeId, canvasPos);
                    NodeSystem::ClearSelection();
                    NodeSystem::SetSelectedNode(nodeId);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Tile")) {
                    int nodeId = NodeSystem::CreateNode("tile");
                    SetNodePosition(nodeId, canvasPos);
                    NodeSystem::ClearSelection();
                    NodeSystem::SetSelectedNode(nodeId);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Transform2D")) {
                    int nodeId = NodeSystem::CreateNode("transform2d");
                    SetNodePosition(nodeId, canvasPos);
                    NodeSystem::ClearSelection();
                    NodeSystem::SetSelectedNode(nodeId);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Blur")) {
                    int nodeId = NodeSystem::CreateNode("blurv2");
                    SetNodePosition(nodeId, canvasPos);
                    NodeSystem::ClearSelection();
                    NodeSystem::SetSelectedNode(nodeId);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Image")) {
                    int nodeId = NodeSystem::CreateNode("image");
                    SetNodePosition(nodeId, canvasPos);
                    NodeSystem::ClearSelection();
                    NodeSystem::SetSelectedNode(nodeId);
                    ImGui::CloseCurrentPopup();
                }
                
                ImGui::EndPopup();
            }
            
            ImGui::PopStyleVar();
            Resume();
            
            NodeSystem::UpdateNodeConnections();
            
            // Handle keyboard shortcuts inside node editor context
            static bool cKeyPressed = false;
            if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
                if (!cKeyPressed) {
                    std::cout << "C key pressed - resetting output nodes to initial positions" << std::endl;
                    // Reset only the 5 output nodes to their initial positions
                    NodeSystem::ResetOutputNodesToInitialPositions();
                    cKeyPressed = true;
                }
            } else {
                cKeyPressed = false;
            }
            
            // Nodes and connections are already rendered above
            
            End();
            SetCurrentEditor(nullptr);
            ImGui::End();
        }
        
        // Render texture preview window
        NodeSystem::RenderTexturePreview();
        
        // Update parameters window with selected node (only when selection changes)
        static BaseNode* lastSelectedNode = nullptr;
        BaseNode* selectedNode = NodeSystem::GetSelectedNodePtr();
        if (selectedNode != lastSelectedNode) {
            parametersWindow.SetSelectedNode(selectedNode);
            lastSelectedNode = selectedNode;
        }
        
        // Render parameters window
        parametersWindow.Render();
        
                            // Context menu is now handled within the NodeEditor context above



        // 7. Show OpenGL View
        if (show_opengl_view)
        {
            ImGui::Begin("OpenGL View", &show_opengl_view);
            ImGui::Text("OpenGL Triangle Renderer");
            ImGui::Separator();
            openglView.Render("OpenGL Content");
            ImGui::End();
        }

        // 8. Show PBR Material View
        if (show_pbr_view)
        {
            ImGui::Begin("PBR Material View", &show_pbr_view);
            ImGui::Text("Physically Based Rendering Material Preview");
            ImGui::Separator();
            
            // Show drag and drop instructions
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Drop 3D model files anywhere on this window");
            ImGui::Text("Supported: OBJ, FBX, 3DS, DAE, STL, PLY, and 40+ other formats");
            
            // Show current model info
            ImGui::Text("Current Model: %s", pbrView.GetModelInfo().c_str());
            
            pbrView.Render("PBR Content");
            ImGui::End();
        }

        // 9. Show Ray-Traced View (Embree)
        if (show_embree_view)
        {
            static int windowCounter = 0;
            if (windowCounter < 5) {
                std::cout << "EmbreeView window being rendered " << windowCounter << " times" << std::endl;
                windowCounter++;
            }
            ImGui::Begin("Ray-Traced View", &show_embree_view);
            ImGui::Text("Ray-Traced Rendering using Embree");
            ImGui::Separator();
            
            // Sync with PBR view data
            embreeView.SyncWithPBRView(&pbrView);
            
            // Show current model info
            ImGui::Text("Current Model: %s", embreeView.GetModelInfo().c_str());
            
            // Ray tracing controls
            ImGui::Text("Ray Tracing Controls:");
            
            // Get current parameters from EmbreeView
            static int samples = 1;
            static int maxDepth = 5;
            static bool usePathTracing = false;
            
            // Update parameters from EmbreeView
            samples = embreeView.m_SamplesPerPixel;
            maxDepth = embreeView.m_MaxDepth;
            usePathTracing = embreeView.m_UsePathTracing;
            
            if (ImGui::SliderInt("Samples per Pixel", &samples, 1, 16)) {
                embreeView.m_SamplesPerPixel = samples;
                embreeView.MarkNeedsRender();
            }
            if (ImGui::SliderInt("Max Ray Depth", &maxDepth, 1, 10)) {
                embreeView.m_MaxDepth = maxDepth;
                embreeView.MarkNeedsRender();
            }
            if (ImGui::Checkbox("Path Tracing", &usePathTracing)) {
                embreeView.m_UsePathTracing = usePathTracing;
                embreeView.MarkNeedsRender();
            }
            
            // Performance info
            ImGui::Separator();
            ImGui::Text("Performance:");
            ImGui::Text("Resolution: %dx%d", (int)embreeView.GetSize().x, (int)embreeView.GetSize().y);
            ImGui::Text("Total Rays: %d", (int)(embreeView.GetSize().x * embreeView.GetSize().y * samples));
            
            embreeView.Render("Ray-Traced Content");
            ImGui::End();
        }


        // End main docking space
        ImGui::End();

        // Show Outliner as a separate window (outside docking space)
        if (show_outliner)
        {
            // Set window size and position to make it visible
            ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
            
            ImGui::Begin("Outliner", &show_outliner);
            ImGui::Text("Node List");
            ImGui::Separator();
            
            try {
                // Safe access to nodes with error handling
                const auto& nodes = NodeSystem::GetNodes();
                ImGui::Text("Node count: %d", (int)nodes.size());
                
                // Simple list of node IDs with error handling
                ImGui::Text("Nodes:");
                for (const auto& pair : nodes) {
                    if (pair.second) { // Check if node exists
                        ImGui::Text("Node ID: %d", pair.first);
                    }
                }
            } catch (const std::exception& e) {
                ImGui::Text("Error loading nodes: %s", e.what());
            } catch (...) {
                ImGui::Text("Unknown error loading nodes");
            }
            
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Save window configuration before cleanup
    int saveX, saveY, saveWidth, saveHeight;
    glfwGetWindowPos(window, &saveX, &saveY);
    glfwGetWindowSize(window, &saveWidth, &saveHeight);
    bool isMaximized = glfwGetWindowAttrib(window, GLFW_MAXIMIZED);
    
    std::ofstream saveConfigFile("window_config.txt");
    if (saveConfigFile.is_open()) {
        saveConfigFile << saveX << " " << saveY << " " << saveWidth << " " << saveHeight << " " << isMaximized;
        saveConfigFile.close();
        std::cout << "Saved window config: " << saveX << ", " << saveY << ", " << saveWidth << "x" << saveHeight << ", maximized: " << isMaximized << std::endl;
    }
    
    // Cleanup
    NodeSystem::Cleanup();
    
    DestroyEditor(nodeEditor);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: file drop callback for handling 3D model file drops
// --------------------------------------------------------
void file_drop_callback(GLFWwindow* window, int count, const char* paths[])
{
    if (count > 0 && g_pbrView != nullptr && g_embreeView != nullptr) {
        std::string filepath(paths[0]);
        std::cout << "File dropped: " << filepath << std::endl;
        
        // Check if it's a supported 3D model file
        std::string extension = filepath.substr(filepath.find_last_of(".") + 1);
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        
        // List of supported extensions
        std::vector<std::string> supportedExtensions = {
            "obj", "fbx", "3ds", "dae", "stl", "ply", "blend", "3mf", "x3d", "ase", "ifc",
            "x", "gltf", "glb", "bvh", "csm", "xml", "irrmesh", "irr", "mdl", "hmp",
            "mesh", "ms3d", "ndo", "nff", "off", "ogex", "pmd", "prt", "smd", "ter",
            "3d", "ac", "ac3d", "acc", "amf", "ase", "awd", "b3d", "blend", "bvh",
            "cob", "csm", "dae", "dxf", "enff", "fbx", "glb", "gltf", "hmp", "ifc",
            "ifczip", "iqm", "irr", "irrmesh", "lwo", "lws", "lxo", "md2", "md3",
            "md5anim", "md5camera", "md5mesh", "mdc", "mdl", "mesh", "mesh.xml",
            "mot", "ms3d", "ndo", "nff", "obj", "off", "ogex", "pk3", "ply", "pmd",
            "prt", "q3o", "q3s", "raw", "scn", "sib", "smd", "stl", "ter", "uc",
            "vta", "x", "x3d", "xgl", "xml", "zgl"
        };
        
        bool isSupported = std::find(supportedExtensions.begin(), supportedExtensions.end(), extension) != supportedExtensions.end();
        
        if (isSupported) {
            std::cout << "Loading 3D model: " << filepath << std::endl;
            bool pbrSuccess = g_pbrView->LoadModelFromFile(filepath);
            bool embreeSuccess = g_embreeView->LoadModelFromFile(filepath);
            
            if (pbrSuccess && embreeSuccess) {
                std::cout << "Successfully loaded 3D model in both views: " << filepath << std::endl;
            } else if (pbrSuccess) {
                std::cout << "Successfully loaded 3D model in PBR view only: " << filepath << std::endl;
            } else if (embreeSuccess) {
                std::cout << "Successfully loaded 3D model in Embree view only: " << filepath << std::endl;
            } else {
                std::cerr << "Failed to load 3D model in both views: " << filepath << std::endl;
            }
        } else {
            std::cout << "Unsupported file format: " << extension << std::endl;
            std::cout << "Supported formats: OBJ, FBX, 3DS, DAE, STL, PLY, and 40+ other formats" << std::endl;
        }
    }
}
