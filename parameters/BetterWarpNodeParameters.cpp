#include "BetterWarpNodeParameters.h"
#include "../nodes/BetterWarpNode.h"
#include <string>

BetterWarpNodeParameters::BetterWarpNodeParameters()
    : m_betterWarpNode(nullptr), m_strength(0.001f), m_spread(0.1f), m_textureWidth(0), m_textureHeight(0) {}

void BetterWarpNodeParameters::SetNode(BaseNode* node) {
    // Only update if the node actually changed
    if (m_betterWarpNode != static_cast<BetterWarpNode*>(node)) {
        m_betterWarpNode = static_cast<BetterWarpNode*>(node);
        if (m_betterWarpNode) {
            UpdateParameters();
        }
    }
}

void BetterWarpNodeParameters::Render() {
    if (!m_betterWarpNode) {
        ImGui::Text("No Better Warp node selected");
        return;
    }
    
    ImGui::Text("Better Warp Node Parameters");
    ImGui::Separator();
    
    // Strength
    std::string strengthId = "##Strength_" + std::to_string(m_betterWarpNode ? m_betterWarpNode->GetNodeId() : 0);
    if (ImGui::SliderFloat(strengthId.c_str(), &m_strength, -0.02f, 0.02f, "%.5f")) {
        ApplyChanges();
    }
    ImGui::SameLine();
    ImGui::Text("Strength");
    
    // Spread
    std::string spreadId = "##Spread_" + std::to_string(m_betterWarpNode ? m_betterWarpNode->GetNodeId() : 0);
    if (ImGui::SliderFloat(spreadId.c_str(), &m_spread, 0.0f, 1.0f, "%.2f")) {
        ApplyChanges();
    }
    ImGui::SameLine();
    ImGui::Text("Spread");
    
    ImGui::Separator();
    
    // Texture Information
    ImGui::Text("Texture Information");
    if (m_betterWarpNode->GetTextureData()) {
        ImGui::Text("Output Size: %dx%d", m_textureWidth, m_textureHeight);
    } else {
        ImGui::Text("Output Size: No texture generated");
    }
    
    if (m_betterWarpNode->GetInputTextureData()) {
        ImGui::Text("Input Size: %dx%d", 
                   m_betterWarpNode->GetInputTextureData()->width, 
                   m_betterWarpNode->GetInputTextureData()->height);
    } else {
        ImGui::Text("Input Size: No input connected");
    }
}

void BetterWarpNodeParameters::ApplyChanges() {
    if (!m_betterWarpNode) return;
    
    // Apply changes to the node
    m_betterWarpNode->SetStrength(m_strength);
    m_betterWarpNode->SetSpread(m_spread);
    
    // Update the node to process the changes
    m_betterWarpNode->Update();
}

void BetterWarpNodeParameters::Clear() {
    m_betterWarpNode = nullptr;
    m_strength = 0.001f;
    m_spread = 0.1f;
}

void BetterWarpNodeParameters::UpdateParameters() {
    if (!m_betterWarpNode) return;
    
    m_strength = m_betterWarpNode->GetStrength();
    m_spread = m_betterWarpNode->GetSpread();
    
    // Update texture dimensions
    if (m_betterWarpNode->GetTextureData()) {
        m_textureWidth = m_betterWarpNode->GetTextureData()->width;
        m_textureHeight = m_betterWarpNode->GetTextureData()->height;
    } else {
        m_textureWidth = 0;
        m_textureHeight = 0;
    }
}
