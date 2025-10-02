#include "OutputNode.h"
#include <imgui_internal.h>
#include <algorithm>

OutputNode::OutputNode(int nodeId, ChannelType channel, ColorMode colorMode)
    : BaseNode(nodeId, NodeType::Output, GetChannelName(channel), channel, colorMode) {
}

std::string OutputNode::GetChannelName(ChannelType channel) {
    switch (channel) {
        case ChannelType::Color: return "Color";
        case ChannelType::Roughness: return "Roughness";
        case ChannelType::Metallic: return "Metallic";
        case ChannelType::Normal: return "Normal";
        case ChannelType::Height: return "Height";
        default: return "Unknown";
    }
}

void OutputNode::Render(ax::NodeEditor::EditorContext* editor) {
    ax::NodeEditor::BeginNode(m_nodeId);

    // Input pin on the left
    ax::NodeEditor::BeginPin(m_inputPinId, PinKind::Input);
    ImGui::Text("-> Input");
    ax::NodeEditor::EndPin();

    ImGui::SameLine();

    // Node content
    ImGui::BeginGroup();
    ImGui::Text("Output");
    
    
    ImGui::Text("Channel: %s", m_name.c_str());
    ImGui::Text("Mode: %s", m_colorMode == ColorMode::RGB ? "RGB" : "Greyscale");

    // Calculate thumbnail size to fit within node
    ImVec2 nodeSize = ImGui::GetContentRegionAvail();
    float thumbnailSize = std::min(nodeSize.x - 20.0f, 100.0f); // Match TextureNode size

    // Display input texture thumbnail if available, otherwise show channel-specific placeholder
    if (m_inputTextureID != 0) {
        ImGui::Image((void*)(intptr_t)m_inputTextureID,
                    ImVec2(thumbnailSize, thumbnailSize),
                    ImVec2(0, 0), ImVec2(1, 1),
                    ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 0.5f));
    } else {
        RenderChannelPlaceholder(thumbnailSize, m_nodeId);
    }
    ImGui::EndGroup();

    ax::NodeEditor::EndNode();
}

void OutputNode::RenderChannelPlaceholder(float thumbnailSize, int nodeId) {
    // Draw channel-specific placeholder thumbnail
    ImGui::InvisibleButton(("##blank_thumb_" + std::to_string(nodeId)).c_str(), ImVec2(thumbnailSize, thumbnailSize));
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();

    // Color based on channel type
    ImU32 bgColor;
    const char* label;
    switch (m_channel) {
        case ChannelType::Color:
            bgColor = ImGui::GetColorU32(ImVec4(1.0f, 0.0f, 0.0f, 0.3f));
            label = "Color";
            break;
        case ChannelType::Roughness:
            bgColor = ImGui::GetColorU32(ImVec4(0.5f, 0.5f, 0.5f, 0.3f));
            label = "Rough";
            break;
        case ChannelType::Metallic:
            bgColor = ImGui::GetColorU32(ImVec4(0.7f, 0.7f, 0.7f, 0.3f));
            label = "Metal";
            break;
        case ChannelType::Normal:
            bgColor = ImGui::GetColorU32(ImVec4(0.5f, 0.5f, 1.0f, 0.3f));
            label = "Normal";
            break;
        case ChannelType::Height:
            bgColor = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.3f, 0.3f));
            label = "Height";
            break;
    }

    drawList->AddRectFilled(pos, max, bgColor, 4.0f);
    drawList->AddText(ImVec2(pos.x + thumbnailSize/2 - 15, pos.y + thumbnailSize/2 - 5),
                     ImGui::GetColorU32(ImGuiCol_Text), label);
}
