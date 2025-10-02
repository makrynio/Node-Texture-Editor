#include "NormalMapNodeParameters.h"
#include "../nodes/NormalMapNode.h"
#include <string>

NormalMapNodeParameters::NormalMapNodeParameters()
    : m_normalMapNode(nullptr), m_strength(0.2f), m_textureWidth(0), m_textureHeight(0) {}

void NormalMapNodeParameters::SetNode(BaseNode* node) {
    // Only update if the node actually changed
    if (m_normalMapNode != static_cast<NormalMapNode*>(node)) {
        m_normalMapNode = static_cast<NormalMapNode*>(node);
        if (m_normalMapNode) {
            UpdateParameters();
        }
    }
}

void NormalMapNodeParameters::Render() {
    if (!m_normalMapNode) {
        ImGui::Text("No Normal Map node selected");
        return;
    }
    
    ImGui::Text("Normal Map Node Parameters");
    ImGui::Separator();
    
    // Strength
    std::string strengthId = "##Strength_" + std::to_string(m_normalMapNode ? m_normalMapNode->GetNodeId() : 0);
    if (ImGui::SliderFloat(strengthId.c_str(), &m_strength, -10.0f, 10.0f, "%.3f")) {
        ApplyChanges();
    }
    ImGui::SameLine();
    ImGui::Text("Strength");
    
    ImGui::Separator();
    
    // Texture Information
    ImGui::Text("Texture Information");
    if (m_normalMapNode->GetTextureData()) {
        ImGui::Text("Output Size: %dx%d", m_textureWidth, m_textureHeight);
    } else {
        ImGui::Text("Output Size: No texture generated");
    }
    
    if (m_normalMapNode->GetInputTextureData()) {
        ImGui::Text("Input Size: %dx%d", 
                   m_normalMapNode->GetInputTextureData()->width, 
                   m_normalMapNode->GetInputTextureData()->height);
    } else {
        ImGui::Text("Input Size: No input connected");
    }
}

void NormalMapNodeParameters::ApplyChanges() {
    if (!m_normalMapNode) return;
    
    // Apply changes to the node
    m_normalMapNode->SetStrength(m_strength);
    
    // Update the node to process the changes
    m_normalMapNode->Update();
}

void NormalMapNodeParameters::Clear() {
    m_normalMapNode = nullptr;
    m_strength = 0.2f;
}

void NormalMapNodeParameters::UpdateParameters() {
    if (!m_normalMapNode) return;
    
    m_strength = m_normalMapNode->GetStrength();
    
    // Update texture dimensions
    if (m_normalMapNode->GetTextureData()) {
        m_textureWidth = m_normalMapNode->GetTextureData()->width;
        m_textureHeight = m_normalMapNode->GetTextureData()->height;
    } else {
        m_textureWidth = 0;
        m_textureHeight = 0;
    }
}
