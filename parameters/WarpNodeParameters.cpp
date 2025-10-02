#include "WarpNodeParameters.h"
#include "../nodes/WarpNode.h"
#include <string>

WarpNodeParameters::WarpNodeParameters()
    : m_warpNode(nullptr), m_strength(0.1f), m_offsetX(0.0f), m_offsetY(0.0f), m_textureWidth(0), m_textureHeight(0) {}

void WarpNodeParameters::SetNode(BaseNode* node) {
    // Cast to WarpNode and update if different
    if (m_warpNode != static_cast<WarpNode*>(node)) {
        m_warpNode = static_cast<WarpNode*>(node);
        if (m_warpNode) {
            UpdateParameters();
        }
    }
}

void WarpNodeParameters::Render() {
    if (!m_warpNode) {
        ImGui::Text("No Warp node selected");
        return;
    }

    ImGui::Text("Warp Node Parameters");
    ImGui::Separator();

    // Strength parameter
    ImGui::Text("Strength:");
    std::string strengthId = "##Strength_" + std::to_string(m_warpNode ? m_warpNode->GetNodeId() : 0);
    if (ImGui::SliderFloat(strengthId.c_str(), &m_strength, 0.0f, 1.0f, "%.3f")) {
        ApplyChanges();
    }

    // Offset X parameter
    ImGui::Text("Offset X:");
    std::string offsetXId = "##OffsetX_" + std::to_string(m_warpNode ? m_warpNode->GetNodeId() : 0);
    if (ImGui::SliderFloat(offsetXId.c_str(), &m_offsetX, -1.0f, 1.0f, "%.3f")) {
        ApplyChanges();
    }

    // Offset Y parameter
    ImGui::Text("Offset Y:");
    std::string offsetYId = "##OffsetY_" + std::to_string(m_warpNode ? m_warpNode->GetNodeId() : 0);
    if (ImGui::SliderFloat(offsetYId.c_str(), &m_offsetY, -1.0f, 1.0f, "%.3f")) {
        ApplyChanges();
    }

    ImGui::Separator();
    ImGui::Text("Texture Info:");
    if (m_warpNode->GetTextureData()) {
        ImGui::Text("Output: %dx%d", m_textureWidth, m_textureHeight);
    }
    if (m_warpNode->GetInputTextureData()) {
        ImGui::Text("Input: %dx%d", 
                   m_warpNode->GetInputTextureData()->width, 
                   m_warpNode->GetInputTextureData()->height);
    }
}

void WarpNodeParameters::ApplyChanges() {
    if (!m_warpNode) return;

    // Apply parameter changes to the node
    m_warpNode->SetStrength(m_strength);
    m_warpNode->SetOffsetX(m_offsetX);
    m_warpNode->SetOffsetY(m_offsetY);

    // Trigger update
    m_warpNode->Update();
}

void WarpNodeParameters::Clear() {
    m_warpNode = nullptr;
}

void WarpNodeParameters::UpdateParameters() {
    if (!m_warpNode) return;

    // Get current parameter values from the node
    m_strength = m_warpNode->GetStrength();
    m_offsetX = m_warpNode->GetOffsetX();
    m_offsetY = m_warpNode->GetOffsetY();

    // Update texture info
    if (m_warpNode->GetTextureData()) {
        m_textureWidth = m_warpNode->GetTextureData()->width;
        m_textureHeight = m_warpNode->GetTextureData()->height;
    }
}
