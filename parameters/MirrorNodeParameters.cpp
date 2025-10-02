
#include "MirrorNodeParameters.h"
#include <imgui.h>

MirrorNodeParameters::MirrorNodeParameters()
    : m_mirrorNode(nullptr), m_mirrorMode(0), m_offset(0.5f), m_clamp(true), 
      m_textureWidth(0), m_textureHeight(0) {
}

void MirrorNodeParameters::SetNode(BaseNode* node) {
    // Only update if the node actually changed
    if (m_mirrorNode != static_cast<MirrorNode*>(node)) {
        m_mirrorNode = static_cast<MirrorNode*>(node);
        if (m_mirrorNode) {
            UpdateParameters();
        }
    }
}

void MirrorNodeParameters::Render() {
    if (!m_mirrorNode) {
        ImGui::Text("No Mirror node selected");
        return;
    }
    
    ImGui::Text("Mirror Node Parameters");
    ImGui::Separator();
    
    // Mirror Mode
    std::string modeId = "##MirrorMode_" + std::to_string(m_mirrorNode ? m_mirrorNode->GetNodeId() : 0);
    const char* modeItems[] = { "Left To Right", "Right To Left", "Bottom To Top", "Top To Bottom" };
    if (ImGui::Combo(modeId.c_str(), &m_mirrorMode, modeItems, 4)) {
        ApplyChanges();
    }
    ImGui::SameLine();
    ImGui::Text("Mirror Mode");
    
    // Offset
    std::string offsetId = "##Offset_" + std::to_string(m_mirrorNode ? m_mirrorNode->GetNodeId() : 0);
    if (ImGui::SliderFloat(offsetId.c_str(), &m_offset, 0.0f, 1.0f, "%.2f")) {
        ApplyChanges();
    }
    ImGui::SameLine();
    ImGui::Text("Offset");
    
    // Clamp
    std::string clampId = "##Clamp_" + std::to_string(m_mirrorNode ? m_mirrorNode->GetNodeId() : 0);
    if (ImGui::Checkbox(clampId.c_str(), &m_clamp)) {
        ApplyChanges();
    }
    ImGui::SameLine();
    ImGui::Text("Clamp");
    
    ImGui::Separator();
    
    // Texture Information
    ImGui::Text("Texture Information");
    if (m_mirrorNode->GetTextureData()) {
        ImGui::Text("Output Size: %dx%d", m_textureWidth, m_textureHeight);
    } else {
        ImGui::Text("Output Size: No texture generated");
    }
    
    if (m_mirrorNode->GetInputTextureData()) {
        ImGui::Text("Input Size: %dx%d", 
                   m_mirrorNode->GetInputTextureData()->width, 
                   m_mirrorNode->GetInputTextureData()->height);
    } else {
        ImGui::Text("Input Size: No input connected");
    }
}

void MirrorNodeParameters::ApplyChanges() {
    if (!m_mirrorNode) return;
    
    // Apply changes to the node
    m_mirrorNode->SetMirrorMode(static_cast<MirrorNode::MirrorMode>(m_mirrorMode));
    m_mirrorNode->SetOffset(m_offset);
    m_mirrorNode->SetClamp(m_clamp);
    
    // Update the node to process the changes
    m_mirrorNode->Update();
}

void MirrorNodeParameters::Clear() {
    m_mirrorNode = nullptr;
    m_mirrorMode = 0;
    m_offset = 0.5f;
    m_clamp = true;
}

void MirrorNodeParameters::UpdateParameters() {
    if (!m_mirrorNode) return;
    
    m_mirrorMode = static_cast<int>(m_mirrorNode->GetMirrorMode());
    m_offset = m_mirrorNode->GetOffset();
    m_clamp = m_mirrorNode->GetClamp();
    
    // Update texture dimensions
    if (m_mirrorNode->GetTextureData()) {
        m_textureWidth = m_mirrorNode->GetTextureData()->width;
        m_textureHeight = m_mirrorNode->GetTextureData()->height;
    } else {
        m_textureWidth = 0;
        m_textureHeight = 0;
    }
}
