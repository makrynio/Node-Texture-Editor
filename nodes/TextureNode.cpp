#include "TextureNode.h"
#include <algorithm>
#include <iostream>

TextureNode::TextureNode(int nodeId, const std::string& name, ChannelType channel, ColorMode colorMode)
    : BaseNode(nodeId, NodeType::Texture, name, channel, colorMode) {
    GenerateTexture();
}

void TextureNode::Render(ax::NodeEditor::EditorContext* editor) {
    ax::NodeEditor::BeginNode(m_nodeId);

    // Node content
    ImGui::BeginGroup();
    ImGui::Text("%s", m_name.c_str());
    
    
    ImGui::Text("Type: Texture");
    ImGui::Text("Mode: %s", m_colorMode == ColorMode::RGB ? "RGB" : "Greyscale");
    ImGui::Text("Size: 1024x1024");

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

void TextureNode::Update() {
    // Update logic if needed
}

void TextureNode::GenerateTexture() {
    // Generate texture for texture nodes
    m_textureData = TextureGenerator::GenerateCheckerboard(1024, 1024, 64);
    m_textureID = TextureGenerator::CreateOpenGLTexture(*m_textureData);
}
