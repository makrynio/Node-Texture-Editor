#include "MirrorNode.h"
#include <glad/glad.h>
#include "../TextureGenerator.h"
#include <iostream>
#include <vector>

using namespace ax::NodeEditor;

MirrorNode::MirrorNode(int nodeId)
    : BaseNode(nodeId, NodeType::Utility, "Mirror", ChannelType::Color, ColorMode::RGB),
      m_mirrorMode(LeftToRight), m_offset(0.5f), m_clamp(true), m_needsUpdate(true) {
    
    // Set up pins for mirror node
    m_inputPinId = nodeId * 100 + 1;   // Input pin
    m_outputPinId = nodeId * 100 + 2;  // Output pin
    
    // Defer texture generation to first render
}

void MirrorNode::Render(ax::NodeEditor::EditorContext* editor) {
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
    
    // Node content
    ImGui::BeginGroup();
    ImGui::Text("Mirror");
    
    // Display current settings (read-only in node)
    const char* modeNames[] = { "Left To Right", "Right To Left", "Bottom To Top", "Top To Bottom" };
    ImGui::Text("Mode: %s", modeNames[m_mirrorMode]);
    ImGui::Text("Offset: %.2f", m_offset);
    ImGui::Text("Clamp: %s", m_clamp ? "Yes" : "No");
    
    // Calculate thumbnail size to fit within node
    ImVec2 nodeSize = ImGui::GetContentRegionAvail();
    float thumbnailSize = std::min(nodeSize.x - 20.0f, 100.0f); // Leave some padding

    // Display texture thumbnail
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

void MirrorNode::Update() {
    // Update texture if parameters changed or input has changed
    if (m_needsUpdate || m_inputTextureID != 0) {
        GenerateTexture();
        m_needsUpdate = false;
    }
}

void MirrorNode::GenerateTexture() {
    GenerateMirrorTexture();
}

void MirrorNode::GenerateMirrorTexture() {
    // Use input texture size if available, otherwise default to 512x512
    int width = 512;
    int height = 512;
    
    if (m_inputTextureData) {
        width = m_inputTextureData->width;
        height = m_inputTextureData->height;
    }
    
    
    std::vector<unsigned char> data(width * height * 4);
    
    if (m_inputTextureID == 0 || !m_inputTextureData) {
        // Generate a default checkerboard texture if no input
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int index = (y * width + x) * 4;
                bool checker = ((x / 32) + (y / 32)) % 2 == 0;
                unsigned char value = checker ? 255 : 128;
                data[index] = value;     // R
                data[index + 1] = value; // G
                data[index + 2] = value; // B
                data[index + 3] = 255;   // A
            }
        }
    } else {
        // Apply mirroring to input texture
        const auto& inputData = m_inputTextureData->data;
        int inputWidth = m_inputTextureData->width;
        int inputHeight = m_inputTextureData->height;
        
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int index = (y * width + x) * 4;
                
                // Calculate mirrored coordinates based on mode and offset
                int sourceX = x;
                int sourceY = y;
                
                // Convert offset to pixel coordinates
                int mirrorLineX = static_cast<int>(m_offset * width);
                int mirrorLineY = static_cast<int>(m_offset * height);
                
                switch (m_mirrorMode) {
                    case LeftToRight: {
                        // Mirror everything to the right of the offset line
                        if (x > mirrorLineX) {
                            sourceX = 2 * mirrorLineX - x;
                        }
                        break;
                    }
                    case RightToLeft: {
                        // Mirror everything to the left of the offset line
                        if (x < mirrorLineX) {
                            sourceX = 2 * mirrorLineX - x;
                        }
                        break;
                    }
                    case TopToBottom: {
                        // Mirror everything below the offset line
                        if (y > mirrorLineY) {
                            sourceY = 2 * mirrorLineY - y;
                        }
                        break;
                    }
                    case BottomToTop: {
                        // Mirror everything above the offset line
                        if (y < mirrorLineY) {
                            sourceY = 2 * mirrorLineY - y;
                        }
                        break;
                    }
                }
                
                // Apply clamping if enabled
                if (m_clamp) {
                    sourceX = std::max(0, std::min(sourceX, inputWidth - 1));
                    sourceY = std::max(0, std::min(sourceY, inputHeight - 1));
                }
                
                // Clamp coordinates to input texture bounds
                sourceX = std::max(0, std::min(sourceX, inputWidth - 1));
                sourceY = std::max(0, std::min(sourceY, inputHeight - 1));
                
                // Sample from input texture
                int inputChannels = m_inputTextureData->channels;
                int inputIndex = (sourceY * inputWidth + sourceX) * inputChannels;
                if (inputIndex + inputChannels - 1 < inputData.size()) {
                    data[index] = inputData[inputIndex];         // R
                    data[index + 1] = inputData[inputIndex + 1]; // G
                    data[index + 2] = inputData[inputIndex + 2]; // B
                    data[index + 3] = (inputChannels > 3) ? inputData[inputIndex + 3] : 255; // A
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
