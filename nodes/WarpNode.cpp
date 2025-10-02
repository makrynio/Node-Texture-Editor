#include "WarpNode.h"
#include <glad/glad.h>
#include "../TextureGenerator.h"
#include <iostream>
#include <vector>
#include <algorithm> // For std::min
#include <cmath> // For sin, cos, fmod

using namespace ax::NodeEditor;

WarpNode::WarpNode(int nodeId)
    : BaseNode(nodeId, NodeType::Utility, "Warp", ChannelType::Color, ColorMode::RGB),
      m_strength(0.1f), m_offsetX(0.0f), m_offsetY(0.0f), m_needsUpdate(true) {
    
    // Set up pins for warp node
    m_inputPinId = nodeId * 100 + 1;   // Input pin
    m_outputPinId = nodeId * 100 + 2;  // Output pin
    
    // Defer texture generation to first render
}

void WarpNode::Render(ax::NodeEditor::EditorContext* editor) {
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
    ImGui::Text("Warp");
    ImGui::Text("Strength: %.2f", m_strength);
    ImGui::Text("Offset X: %.2f", m_offsetX);
    ImGui::Text("Offset Y: %.2f", m_offsetY);
    
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

void WarpNode::Update() {
    // Update texture if parameters changed or input has changed
    if (m_needsUpdate || m_inputTextureData != nullptr) {
        GenerateTexture();
        m_needsUpdate = false;
    }
}

void WarpNode::GenerateTexture() {
    GenerateWarpTexture();
}

void WarpNode::GenerateWarpTexture() {
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
        // Apply simple warping based on position and parameters
        const auto& inputData = m_inputTextureData->data;
        int inputWidth = m_inputTextureData->width;
        int inputHeight = m_inputTextureData->height;
        
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int index = (y * width + x) * 4;
                
                // Calculate normalized coordinates
                float u = (float)x / width;
                float v = (float)y / height;
                
                // Apply simple warp effect
                float warpX = u + sin(v * 3.14159f * 2.0f) * m_strength + m_offsetX;
                float warpY = v + cos(u * 3.14159f * 2.0f) * m_strength + m_offsetY;
                
                // Wrap coordinates
                warpX = fmod(warpX + 1.0f, 1.0f);
                warpY = fmod(warpY + 1.0f, 1.0f);
                
                // Sample from input texture
                int sourceX = static_cast<int>(warpX * inputWidth);
                int sourceY = static_cast<int>(warpY * inputHeight);
                
                // Clamp coordinates
                sourceX = std::max(0, std::min(sourceX, inputWidth - 1));
                sourceY = std::max(0, std::min(sourceY, inputHeight - 1));
                
                // Sample from input texture with proper channel handling
                int inputChannels = m_inputTextureData->channels;
                int sourceIndex = (sourceY * inputWidth + sourceX) * inputChannels;
                
                if (sourceIndex + inputChannels - 1 < inputData.size()) {
                    if (inputChannels >= 4) {
                        // RGBA input
                        data[index] = inputData[sourceIndex];         // R
                        data[index + 1] = inputData[sourceIndex + 1]; // G
                        data[index + 2] = inputData[sourceIndex + 2]; // B
                        data[index + 3] = inputData[sourceIndex + 3]; // A
                    } else if (inputChannels == 3) {
                        // RGB input
                        data[index] = inputData[sourceIndex];         // R
                        data[index + 1] = inputData[sourceIndex + 1]; // G
                        data[index + 2] = inputData[sourceIndex + 2]; // B
                        data[index + 3] = 255;                        // A
                    } else if (inputChannels == 1) {
                        // Grayscale input
                        data[index] = inputData[sourceIndex];         // R
                        data[index + 1] = inputData[sourceIndex];     // G
                        data[index + 2] = inputData[sourceIndex];     // B
                        data[index + 3] = 255;                        // A
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
