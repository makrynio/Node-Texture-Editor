#include "GradientNodeParameters.h"
#include "../nodes/GradientNode.h"
#include <iostream>

GradientNodeParameters::GradientNodeParameters() 
    : m_node(nullptr), m_colorA(0.0f, 0.0f, 0.0f, 1.0f), m_positionA(0.0f),
      m_colorB(1.0f, 1.0f, 1.0f, 1.0f), m_positionB(1.0f), m_mode(0) {
    CreateParameters();
}

void GradientNodeParameters::SetNode(BaseNode* node) {
    // Only update if the node actually changed
    if (m_node != node) {
        m_node = node;
        if (node) {
            UpdateParameters();
        }
    }
}

void GradientNodeParameters::Render() {
    if (!m_node) {
        ImGui::Text("No node selected");
        return;
    }
    
    ImGui::Text("Gradient Node Parameters");
    ImGui::Separator();
    
    // Display gradient information
    ImGui::Text("Type: Two-Color Gradient");
    ImGui::Text("Size: 512x512");
    
    ImGui::Separator();
    ImGui::Text("Properties:");
    
    // Color A
    ImGui::Text("Color A:");
    std::string colorAId = "##ColorA_" + std::to_string(m_node ? m_node->GetNodeId() : 0);
    if (ImGui::ColorEdit4(colorAId.c_str(), (float*)&m_colorA)) {
        ApplyChanges();
    }
    ImGui::Text("Position A: %.2f", m_positionA);
    std::string posAId = "##PosA_" + std::to_string(m_node ? m_node->GetNodeId() : 0);
    if (ImGui::SliderFloat(posAId.c_str(), &m_positionA, 0.0f, 1.0f)) {
        ApplyChanges();
    }
    
    ImGui::Separator();
    
    // Color B
    ImGui::Text("Color B:");
    std::string colorBId = "##ColorB_" + std::to_string(m_node ? m_node->GetNodeId() : 0);
    if (ImGui::ColorEdit4(colorBId.c_str(), (float*)&m_colorB)) {
        ApplyChanges();
    }
    ImGui::Text("Position B: %.2f", m_positionB);
    std::string posBId = "##PosB_" + std::to_string(m_node ? m_node->GetNodeId() : 0);
    if (ImGui::SliderFloat(posBId.c_str(), &m_positionB, 0.0f, 1.0f)) {
        ApplyChanges();
    }
    
    ImGui::Separator();
    
    // Direction
    ImGui::Text("Direction:");
    const char* modes[] = { "Left To Right", "Right To Left", "Top To Bottom", "Bottom To Top" };
    std::string modeId = "##Mode_" + std::to_string(m_node ? m_node->GetNodeId() : 0);
    if (ImGui::Combo(modeId.c_str(), &m_mode, modes, 4)) {
        ApplyChanges();
    }
    
    // Render parameter widgets
    for (auto& param : m_parameters) {
        param->Render();
    }
}

void GradientNodeParameters::Clear() {
    m_node = nullptr;
}

void GradientNodeParameters::CreateParameters() {
    // For gradient nodes, we mainly use direct UI controls
    // In a full implementation, we might add more complex parameters
}

void GradientNodeParameters::UpdateParameters() {
    // Update parameter values based on the current gradient node
    if (m_node) {
        GradientNode* gradientNode = dynamic_cast<GradientNode*>(m_node);
        if (gradientNode) {
            m_colorA = gradientNode->GetColorA();
            m_positionA = gradientNode->GetPositionA();
            m_colorB = gradientNode->GetColorB();
            m_positionB = gradientNode->GetPositionB();
            m_mode = gradientNode->GetMode();
        }
    }
}

void GradientNodeParameters::ApplyChanges() {
    if (m_node) {
        GradientNode* gradientNode = dynamic_cast<GradientNode*>(m_node);
        if (gradientNode) {
            gradientNode->SetColorA(m_colorA);
            gradientNode->SetPositionA(m_positionA);
            gradientNode->SetColorB(m_colorB);
            gradientNode->SetPositionB(m_positionB);
            gradientNode->SetMode(m_mode);
            
            // Force texture regeneration
            gradientNode->GenerateTexture();
        }
    }
}
