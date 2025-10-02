#include "TriGradientNodeParameters.h"
#include "../nodes/TriGradientNode.h"
#include <iostream>

TriGradientNodeParameters::TriGradientNodeParameters() 
    : m_node(nullptr), m_colorA(0.0f, 0.0f, 0.0f, 1.0f), m_positionA(0.0f),
      m_colorB(1.0f, 1.0f, 1.0f, 1.0f), m_positionB(0.5f),
      m_colorC(0.0f, 0.0f, 0.0f, 1.0f), m_positionC(1.0f), m_mode(0) {
    CreateParameters();
}

void TriGradientNodeParameters::SetNode(BaseNode* node) {
    m_node = node;
    UpdateParameters();
}

void TriGradientNodeParameters::Render() {
    if (!m_node) {
        ImGui::Text("No node selected");
        return;
    }
    
    ImGui::Text("TriGradient Node Parameters");
    ImGui::Separator();
    
    // Display gradient information
    ImGui::Text("Type: Three-Color Gradient");
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
    
    // Color C
    ImGui::Text("Color C:");
    std::string colorCId = "##ColorC_" + std::to_string(m_node ? m_node->GetNodeId() : 0);
    if (ImGui::ColorEdit4(colorCId.c_str(), (float*)&m_colorC)) {
        ApplyChanges();
    }
    ImGui::Text("Position C: %.2f", m_positionC);
    std::string posCId = "##PosC_" + std::to_string(m_node ? m_node->GetNodeId() : 0);
    if (ImGui::SliderFloat(posCId.c_str(), &m_positionC, 0.0f, 1.0f)) {
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

void TriGradientNodeParameters::Clear() {
    m_node = nullptr;
}

void TriGradientNodeParameters::CreateParameters() {
    // For tri-gradient nodes, we mainly use direct UI controls
    // In a full implementation, we might add more complex parameters
}

void TriGradientNodeParameters::UpdateParameters() {
    // Update parameter values based on the current tri-gradient node
    if (m_node) {
        TriGradientNode* triGradientNode = dynamic_cast<TriGradientNode*>(m_node);
        if (triGradientNode) {
            m_colorA = triGradientNode->GetColorA();
            m_positionA = triGradientNode->GetPositionA();
            m_colorB = triGradientNode->GetColorB();
            m_positionB = triGradientNode->GetPositionB();
            m_colorC = triGradientNode->GetColorC();
            m_positionC = triGradientNode->GetPositionC();
            m_mode = triGradientNode->GetMode();
        }
    }
}

void TriGradientNodeParameters::ApplyChanges() {
    if (m_node) {
        TriGradientNode* triGradientNode = dynamic_cast<TriGradientNode*>(m_node);
        if (triGradientNode) {
            triGradientNode->SetColorA(m_colorA);
            triGradientNode->SetPositionA(m_positionA);
            triGradientNode->SetColorB(m_colorB);
            triGradientNode->SetPositionB(m_positionB);
            triGradientNode->SetColorC(m_colorC);
            triGradientNode->SetPositionC(m_positionC);
            triGradientNode->SetMode(m_mode);
            
            // Force texture regeneration
            triGradientNode->GenerateTexture();
        }
    }
}
