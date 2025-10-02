#include "NormalMapNode.h"
#include <glad/glad.h>
#include "../TextureGenerator.h"
#include <iostream>
#include <vector>
#include <algorithm> // For std::min

using namespace ax::NodeEditor;

NormalMapNode::NormalMapNode(int nodeId)
    : BaseNode(nodeId, NodeType::Utility, "Normal Map", ChannelType::Color, ColorMode::RGB),
      m_strength(0.2f), m_needsUpdate(true) {
    
    // Set up pins for normal map node
    m_inputPinId = nodeId * 100 + 1;   // Input pin
    m_outputPinId = nodeId * 100 + 2;  // Output pin
    
    // Defer texture generation to first render
}

void NormalMapNode::Render(ax::NodeEditor::EditorContext* editor) {
    // Generate texture on first render if needed
    if (m_textureID == 0) {
        GenerateTexture();
    }
    
    ax::NodeEditor::BeginNode(m_nodeId);
    
    // Input pin on the left
    ax::NodeEditor::BeginPin(m_inputPinId, PinKind::Input);
    ImGui::Text("Height");
    ax::NodeEditor::EndPin();
    
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::Text("Normal Map");
    ImGui::Text("Strength: %.4f", m_strength);
    
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

void NormalMapNode::Update() {
    // Update texture if parameters changed or input has changed
    if (m_needsUpdate || m_inputTextureData != nullptr) {
        GenerateTexture();
        m_needsUpdate = false;
    }
}

void NormalMapNode::GenerateTexture() {
    GenerateNormalMapTexture();
}

void NormalMapNode::GenerateNormalMapTexture() {
    // Use input texture size if available, otherwise default to 512x512
    int width = 512;
    int height = 512;
    
    if (m_inputTextureData) {
        width = m_inputTextureData->width;
        height = m_inputTextureData->height;
    }
    
    
    std::vector<unsigned char> data(width * height * 4);
    
    if (!m_inputTextureData) {
        // Generate a default normal map texture if no input
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int index = (y * width + x) * 4;
                // Default normal pointing up (0.5, 0.5, 1.0) -> (128, 128, 255)
                data[index] = 128;     // R
                data[index + 1] = 128; // G
                data[index + 2] = 255; // B
                data[index + 3] = 255; // A
            }
        }
    } else {
        // Generate normal map from height map
        const auto& inputData = m_inputTextureData->data;
        int inputWidth = m_inputTextureData->width;
        int inputHeight = m_inputTextureData->height;
        
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int index = (y * width + x) * 4;
                
                // Sample height values
                float center = GetHeightValue(inputData, inputWidth, inputHeight, x, y);
                float right = GetHeightValue(inputData, inputWidth, inputHeight, x + 1, y);
                float left = GetHeightValue(inputData, inputWidth, inputHeight, x - 1, y);
                float top = GetHeightValue(inputData, inputWidth, inputHeight, x, y - 1);
                float bottom = GetHeightValue(inputData, inputWidth, inputHeight, x, y + 1);
                
                // Calculate gradients (Sobel operator)
                float dx = (right - left) * 0.5f;
                float dy = (bottom - top) * 0.5f;
                
                // Apply strength
                dx *= m_strength;
                dy *= m_strength;
                
                // Calculate normal vector
                float normalX = -dx;
                float normalY = -dy;
                float normalZ = 1.0f;
                
                // Normalize
                float length = sqrt(normalX * normalX + normalY * normalY + normalZ * normalZ);
                if (length > 0) {
                    normalX /= length;
                    normalY /= length;
                    normalZ /= length;
                }
                
                // Convert to 0-1 range
                data[index] = static_cast<unsigned char>((normalX + 1.0f) * 0.5f * 255);     // R
                data[index + 1] = static_cast<unsigned char>((normalY + 1.0f) * 0.5f * 255); // G
                data[index + 2] = static_cast<unsigned char>((normalZ + 1.0f) * 0.5f * 255); // B
                data[index + 3] = 255; // A
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

float NormalMapNode::GetHeightValue(const std::vector<unsigned char>& data, int width, int height, int x, int y) {
    // Clamp coordinates
    x = std::max(0, std::min(x, width - 1));
    y = std::max(0, std::min(y, height - 1));
    
    // Use the actual number of channels from input texture
    int channels = m_inputTextureData->channels;
    int index = (y * width + x) * channels;
    if (index + channels - 1 < data.size()) {
        return data[index] / 255.0f; // Use red channel as height
    }
    return 0.0f;
}
