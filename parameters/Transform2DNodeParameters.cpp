#include "Transform2DNodeParameters.h"
#include "../nodes/Transform2DNode.h"
#include <iostream>
#include <string>

Transform2DNodeParameters::Transform2DNodeParameters() 
    : m_node(nullptr), m_translateX(0.0f), m_translateY(0.0f), 
      m_scaleX(1.0f), m_scaleY(1.0f), m_rotation(0.0f), m_clamp(true) {
    CreateParameters();
}

void Transform2DNodeParameters::SetNode(BaseNode* node) {
    // Only update if the node actually changed
    if (m_node != node) {
        m_node = node;
        if (node) {
            UpdateParameters();
        }
    }
}

void Transform2DNodeParameters::Render() {
    if (!m_node) {
        ImGui::Text("No node selected");
        return;
    }
    
    ImGui::Text("Transform2D Node Parameters");
    ImGui::Separator();
    
    // Display transform information
    ImGui::Text("Type: 2D Transform");
    ImGui::Text("Size: 512x512");
    
    ImGui::Separator();
    ImGui::Text("Properties:");
    
    // Translation
    ImGui::Text("Translation:");
    std::string translateXId = "##TranslateX_" + std::to_string(m_node ? m_node->GetNodeId() : 0);
    if (ImGui::SliderFloat(translateXId.c_str(), &m_translateX, -1.0f, 1.0f, "X: %.2f")) {
        ApplyChanges();
    }
    std::string translateYId = "##TranslateY_" + std::to_string(m_node ? m_node->GetNodeId() : 0);
    if (ImGui::SliderFloat(translateYId.c_str(), &m_translateY, -1.0f, 1.0f, "Y: %.2f")) {
        ApplyChanges();
    }
    
    ImGui::Separator();
    
    // Scale
    ImGui::Text("Scale:");
    std::string scaleXId = "##ScaleX_" + std::to_string(m_node ? m_node->GetNodeId() : 0);
    if (ImGui::SliderFloat(scaleXId.c_str(), &m_scaleX, -2.0f, 2.0f, "X: %.2f")) {
        ApplyChanges();
    }
    std::string scaleYId = "##ScaleY_" + std::to_string(m_node ? m_node->GetNodeId() : 0);
    if (ImGui::SliderFloat(scaleYId.c_str(), &m_scaleY, -2.0f, 2.0f, "Y: %.2f")) {
        ApplyChanges();
    }
    
    ImGui::Separator();
    
    // Rotation
    ImGui::Text("Rotation:");
    std::string rotationId = "##Rotation_" + std::to_string(m_node ? m_node->GetNodeId() : 0);
    if (ImGui::SliderFloat(rotationId.c_str(), &m_rotation, 0.0f, 360.0f, "%.1f°")) {
        ApplyChanges();
    }
    
    ImGui::Separator();
    
    // Clamp
    ImGui::Text("Clamp:");
    std::string clampId = "##Clamp_" + std::to_string(m_node ? m_node->GetNodeId() : 0);
    if (ImGui::Checkbox(clampId.c_str(), &m_clamp)) {
        ApplyChanges();
    }
    
    // Render parameter widgets
    for (auto& param : m_parameters) {
        param->Render();
    }
}

void Transform2DNodeParameters::Clear() {
    m_node = nullptr;
}

void Transform2DNodeParameters::CreateParameters() {
    // For transform nodes, we mainly use direct UI controls
    // In a full implementation, we might add more complex parameters
}

void Transform2DNodeParameters::UpdateParameters() {
    // Update parameter values based on the current transform node
    if (m_node) {
        Transform2DNode* transformNode = dynamic_cast<Transform2DNode*>(m_node);
        if (transformNode) {
            m_translateX = transformNode->GetTranslateX();
            m_translateY = transformNode->GetTranslateY();
            m_scaleX = transformNode->GetScaleX();
            m_scaleY = transformNode->GetScaleY();
            m_rotation = transformNode->GetRotation();
            m_clamp = transformNode->GetClamp();
        }
    }
}

void Transform2DNodeParameters::ApplyChanges() {
    if (m_node) {
        Transform2DNode* transformNode = dynamic_cast<Transform2DNode*>(m_node);
        if (transformNode) {
            transformNode->SetTranslateX(m_translateX);
            transformNode->SetTranslateY(m_translateY);
            transformNode->SetScaleX(m_scaleX);
            transformNode->SetScaleY(m_scaleY);
            transformNode->SetRotation(m_rotation);
            transformNode->SetClamp(m_clamp);
            
            // Force texture regeneration
            transformNode->Update();
        }
    }
}
