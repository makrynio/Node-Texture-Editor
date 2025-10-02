#include "BaseParameter.h"
#include "../nodes/BaseNode.h"

void FloatParameter::Render() {
    if (ImGui::SliderFloat(m_name.c_str(), &m_value, m_min, m_max, m_format.c_str())) {
        if (m_node) {
            m_node->Update();
        }
    }
    if (!m_description.empty() && ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", m_description.c_str());
    }
}

void IntParameter::Render() {
    if (ImGui::SliderInt(m_name.c_str(), &m_value, m_min, m_max)) {
        if (m_node) {
            m_node->Update();
        }
    }
    if (!m_description.empty() && ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", m_description.c_str());
    }
}

void ComboParameter::Render() {
    if (ImGui::Combo(m_name.c_str(), &m_value, [](void* data, int idx, const char** out_text) {
        auto* items = static_cast<std::vector<std::string>*>(data);
        *out_text = (*items)[idx].c_str();
        return true;
    }, &m_items, m_items.size())) {
        if (m_node) {
            m_node->Update();
        }
    }
    if (!m_description.empty() && ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", m_description.c_str());
    }
}

void BoolParameter::Render() {
    if (ImGui::Checkbox(m_name.c_str(), &m_value)) {
        if (m_node) {
            m_node->Update();
        }
    }
    if (!m_description.empty() && ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", m_description.c_str());
    }
}

void Vec2Parameter::Render() {
    if (ImGui::SliderFloat2(m_name.c_str(), m_value, m_min, m_max, m_format.c_str())) {
        if (m_node) {
            m_node->Update();
        }
    }
    if (!m_description.empty() && ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", m_description.c_str());
    }
}
