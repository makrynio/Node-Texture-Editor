#include "TileNode.h"
#include <glad/glad.h>
#include "../TextureGenerator.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace ax::NodeEditor;

TileNode::TileNode(int nodeId)
    : BaseNode(nodeId, NodeType::Utility, "Tile", ChannelType::Color, ColorMode::RGB),
      m_offset(0.5f), m_brickWidth(1.0f), m_brickHeight(1.0f), m_rows(6.0f), m_columns(6.0f), m_needsUpdate(true) {
    
    // Set up pins for tile node - properly set base class members
    m_inputPinId = nodeId * 100 + 1;   // Input pin (image)
    m_outputPinId = nodeId * 100 + 2;  // Output pin
    
    std::cout << "TileNode constructor: nodeId=" << nodeId << ", inputPin=" << m_inputPinId.Get() << ", outputPin=" << m_outputPinId.Get() << std::endl;
    
    
    
    // Defer texture generation to first render
}

void TileNode::Render(ax::NodeEditor::EditorContext* editor) {
    // Generate texture on first render if needed
    if (m_textureID == 0) {
        GenerateTexture();
    }
    
    ax::NodeEditor::BeginNode(m_nodeId);
    
    // Input pin on the left
    ax::NodeEditor::BeginPin(m_inputPinId, PinKind::Input);
    ImGui::Text("Image");
    ax::NodeEditor::EndPin();
    
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::Text("Tile");
    ImGui::Text("Offset: %.2f", m_offset);
    ImGui::Text("Size: %.2fx%.2f", m_brickWidth, m_brickHeight);
    ImGui::Text("Grid: %.0fx%.0f", m_rows, m_columns);
    
    // Show texture thumbnail
    ImVec2 nodeSize = ImGui::GetContentRegionAvail();
    float thumbnailSize = std::min(nodeSize.x - 20.0f, 100.0f);
    if (m_textureID != 0) {
        ImGui::Image((void*)(intptr_t)m_textureID,
                    ImVec2(thumbnailSize, thumbnailSize),
                    ImVec2(0, 0), ImVec2(1, 1),
                    ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 0.5f));
    }
    ImGui::EndGroup();
    
    ImGui::SameLine();
    // Output pin on the right
    ax::NodeEditor::BeginPin(m_outputPinId, PinKind::Output);
    ImGui::Text("Output ->");
    ax::NodeEditor::EndPin();

    ax::NodeEditor::EndNode();
}

void TileNode::Update() {
    // Update texture if parameters changed or input has changed
    if (m_needsUpdate || m_inputTextureData != nullptr) {
        GenerateTexture();
        m_needsUpdate = false;
    }
}

void TileNode::GenerateTexture() {
    GenerateTileTexture();
}

void TileNode::GenerateTileTexture() {
    // Use input texture size if available, otherwise default to 1024x1024
    int width = 1024;
    int height = 1024;
    
    if (m_inputTextureData) {
        width = m_inputTextureData->width;
        height = m_inputTextureData->height;
    }
    
    std::vector<unsigned char> data(width * height * 4);
    
    if (!m_inputTextureData) {
        // Generate a default texture if no input
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int index = (y * width + x) * 4;
                data[index] = 128;     // R
                data[index + 1] = 128; // G
                data[index + 2] = 128; // B
                data[index + 3] = 255; // A
            }
        }
    } else {
        // Apply tiling based on the shader logic
        const auto& inputData = m_inputTextureData->data;
        int inputWidth = m_inputTextureData->width;
        int inputHeight = m_inputTextureData->height;
        int inputChannels = m_inputTextureData->channels;
        
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int index = (y * width + x) * 4;
                
                // Convert to UV coordinates
                float u = (float)x / width;
                float v = (float)y / height;
                
                // Apply tiling logic from the shader
                float offset = m_offset;
                float tileSizeX = m_rows;
                float tileSizeY = m_columns;
                float posX = u * tileSizeX;
                float posY = v * tileSizeY;
                
                // Calculate offset for alternating rows
                float xOffset = 0.0f;
                if (std::fmod(posY * 0.5f, 1.0f) > 0.5f) {
                    xOffset = offset;
                }
                posX += xOffset;
                
                // Get brick ID
                int brickIdX = (int)std::floor(posX);
                int brickIdY = (int)std::floor(posY);
                
                // Wrap around x
                if (brickIdX > tileSizeX - 1) {
                    brickIdX = 0;
                }
                
                // Convert back to UV coordinates for sampling
                float sampleU = posX - std::floor(posX);
                float sampleV = posY - std::floor(posY);
                
                // Check if we're inside a brick
                float brickSizeX = m_brickWidth;
                float brickSizeY = m_brickHeight;
                float edgeSizeX = (1.0f - brickSizeX) * 0.5f;
                float edgeSizeY = (1.0f - brickSizeY) * 0.5f;
                
                bool isBrickX = (sampleU > edgeSizeX && sampleU < (1.0f - edgeSizeX));
                bool isBrickY = (sampleV > edgeSizeY && sampleV < (1.0f - edgeSizeY));
                bool isBrick = isBrickX && isBrickY;
                
                // Sample from input texture
                int sourceX = (int)(sampleU * inputWidth);
                int sourceY = (int)(sampleV * inputHeight);
                
                // Clamp coordinates
                sourceX = std::max(0, std::min(sourceX, inputWidth - 1));
                sourceY = std::max(0, std::min(sourceY, inputHeight - 1));
                
                int sourceIndex = (sourceY * inputWidth + sourceX) * inputChannels;
                
                if (sourceIndex + inputChannels - 1 < inputData.size()) {
                    if (inputChannels >= 4) {
                        // RGBA input
                        data[index] = inputData[sourceIndex];         // R
                        data[index + 1] = inputData[sourceIndex + 1]; // G
                        data[index + 2] = inputData[sourceIndex + 2]; // B
                        data[index + 3] = isBrick ? inputData[sourceIndex + 3] : 0; // A
                    } else if (inputChannels == 3) {
                        // RGB input
                        data[index] = inputData[sourceIndex];         // R
                        data[index + 1] = inputData[sourceIndex + 1]; // G
                        data[index + 2] = inputData[sourceIndex + 2]; // B
                        data[index + 3] = isBrick ? 255 : 0;          // A
                    } else if (inputChannels == 1) {
                        // Grayscale input
                        data[index] = inputData[sourceIndex];         // R
                        data[index + 1] = inputData[sourceIndex];     // G
                        data[index + 2] = inputData[sourceIndex];     // B
                        data[index + 3] = isBrick ? 255 : 0;          // A
                    }
                } else {
                    data[index] = 128;     // R
                    data[index + 1] = 128; // G
                    data[index + 2] = 128; // B
                    data[index + 3] = 255; // A
                }
            }
        }
    }
    
    // Generate or update OpenGL texture
    if (m_textureID == 0) {
        glGenTextures(1, &m_textureID);
    }
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Store texture data
    m_textureData = std::make_shared<TextureData>(width, height, 4);
    m_textureData->data = std::move(data);
}
