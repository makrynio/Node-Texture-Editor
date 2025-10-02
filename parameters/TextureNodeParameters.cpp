#include "TextureNodeParameters.h"
#include "../nodes/TextureNode.h"
#include <iostream>

TextureNodeParameters::TextureNodeParameters() 
    : m_node(nullptr), m_width(512), m_height(512), m_channels(3), m_textureName("Checkerboard") {
    CreateParameters();
}

void TextureNodeParameters::SetNode(BaseNode* node) {
    m_node = node;
    UpdateParameters();
}

void TextureNodeParameters::Render() {
    if (!m_node) {
        ImGui::Text("No node selected");
        return;
    }
    
    ImGui::Text("Texture Node Parameters");
    ImGui::Separator();
    
    // Display texture information
    ImGui::Text("Texture: %s", m_textureName.c_str());
    ImGui::Text("Size: %dx%d", m_width, m_height);
    ImGui::Text("Channels: %d", m_channels);
    
    ImGui::Separator();
    ImGui::Text("Properties:");
    
    for (auto& param : m_parameters) {
        param->Render();
    }
}

void TextureNodeParameters::Clear() {
    m_node = nullptr;
}

void TextureNodeParameters::CreateParameters() {
    // For texture nodes, we mainly display information
    // In a full implementation, we might add parameters for:
    // - Texture size
    // - Checker size
    // - Color settings
    // - Generation parameters
}

void TextureNodeParameters::UpdateParameters() {
    // Update parameter values based on the current texture node
    if (m_node) {
        // Get texture information from the node
        // This would be implemented based on the actual TextureNode structure
    }
}
