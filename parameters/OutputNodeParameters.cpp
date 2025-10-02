#include "OutputNodeParameters.h"
#include "../nodes/OutputNode.h"
#include <iostream>

OutputNodeParameters::OutputNodeParameters() : m_node(nullptr) {
    CreateParameters();
}

void OutputNodeParameters::SetNode(BaseNode* node) {
    m_node = node;
    UpdateParameters();
}

void OutputNodeParameters::Render() {
    if (!m_node) {
        ImGui::Text("No node selected");
        return;
    }
    
    ImGui::Text("Output Node Parameters");
    ImGui::Separator();
    
    for (auto& param : m_parameters) {
        param->Render();
    }
}

void OutputNodeParameters::Clear() {
    m_node = nullptr;
}

void OutputNodeParameters::CreateParameters() {
    // Output nodes don't have many parameters, but we can add some basic ones
    // For now, we'll just show the node information
}

void OutputNodeParameters::UpdateParameters() {
    // Update parameter values based on the current node
    // For output nodes, we mainly display information rather than editable parameters
}
