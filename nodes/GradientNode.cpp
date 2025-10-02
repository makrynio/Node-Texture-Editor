#include "GradientNode.h"
#include "../TextureGenerator.h"
#include <iostream>
#include <algorithm>

using namespace ax::NodeEditor;

GradientNode::GradientNode(int nodeId, const std::string& name)
    : BaseNode(nodeId, NodeType::Utility, name, ChannelType::Color, ColorMode::RGB)
    , m_colorA(0.0f, 0.0f, 0.0f, 1.0f)  // Black
    , m_positionA(0.0f)
    , m_colorB(1.0f, 1.0f, 1.0f, 1.0f)  // White
    , m_positionB(1.0f)
    , m_mode(0)  // Left to Right
    , m_needsUpdate(true)
{
    // Defer texture generation to first render
}

void GradientNode::Render(ax::NodeEditor::EditorContext* editor) {
    // Generate texture on first render if needed
    if (m_needsUpdate && m_textureID == 0) {
        GenerateTexture();
        m_needsUpdate = false;
    }

    ax::NodeEditor::BeginNode(m_nodeId);

    // Node content
    ImGui::BeginGroup();
    ImGui::Text("%s", m_name.c_str());
    
    ImGui::Text("Type: Gradient");
    ImGui::Text("Mode: %s", m_colorMode == ColorMode::RGB ? "RGB" : "Greyscale");
    ImGui::Text("Size: 512x512");

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

void GradientNode::Update() {
    // Only update when parameters change, not every frame
    // This is handled by the parameter system now
}

void GradientNode::GenerateTexture() {
    // Generate gradient texture
    const int width = 512;
    const int height = 512;
    const int channels = 3;
    
    
    try {
        m_textureData = std::make_shared<TextureData>(width, height, channels);
        
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                float t = 0.0f;
                
                // Calculate gradient based on mode
                if (m_mode == 0) t = (float)x / width;           // Left to Right
                else if (m_mode == 1) t = 1.0f - (float)x / width; // Right to Left
                else if (m_mode == 2) t = 1.0f - (float)y / height; // Top to Bottom
                else if (m_mode == 3) t = (float)y / height;       // Bottom to Top
                
                // Interpolate between colors
                float clampedT = (t - m_positionA) / (m_positionB - m_positionA);
                clampedT = std::max(0.0f, std::min(1.0f, clampedT));
                
                float r = m_colorA.x + (m_colorB.x - m_colorA.x) * clampedT;
                float g = m_colorA.y + (m_colorB.y - m_colorA.y) * clampedT;
                float b = m_colorA.z + (m_colorB.z - m_colorA.z) * clampedT;
                
                int index = (y * width + x) * channels;
                if (index + 2 < m_textureData->data.size()) {
                    m_textureData->data[index] = (unsigned char)(r * 255);
                    m_textureData->data[index + 1] = (unsigned char)(g * 255);
                    m_textureData->data[index + 2] = (unsigned char)(b * 255);
                }
            }
        }
        
        // Delete old texture if it exists
        if (m_textureID != 0) {
            TextureGenerator::DeleteTexture(m_textureID);
            m_textureID = 0;
        }
        
        m_textureID = TextureGenerator::CreateOpenGLTexture(*m_textureData);
    } catch (const std::exception& e) {
        std::cout << "Error generating gradient texture: " << e.what() << std::endl;
    }
}
