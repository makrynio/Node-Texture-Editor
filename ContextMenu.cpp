#include "ContextMenu.h"
#include "NodeSystem.h"
#include "nodes/NodeFactory.h"
#include <algorithm>
#include <cstring>
#include <iostream>

bool ContextMenu::s_visible = false;
ImVec2 ContextMenu::s_position = ImVec2(0, 0);
std::vector<NodeTypeInfo> ContextMenu::s_nodeTypes;
char ContextMenu::s_searchBuffer[256] = "";

void ContextMenu::Show() {
    s_visible = true;
    s_position = ImGui::GetMousePos();
    strcpy(s_searchBuffer, "");
}

void ContextMenu::Hide() {
    s_visible = false;
}

bool ContextMenu::IsVisible() {
    return s_visible;
}

void ContextMenu::SetPosition(const ImVec2& pos) {
    s_position = pos;
}

void ContextMenu::RegisterNodeType(const std::string& name, const std::string& displayName, 
                                  const std::string& category, std::function<void()> createFunction) {
    s_nodeTypes.emplace_back(name, displayName, category, createFunction);
}

void ContextMenu::Render() {
    // Use ImGui's native popup system
    if (ImGui::BeginPopup("NodeContextMenu")) {
        std::cout << "Rendering context menu popup" << std::endl;
        // Search bar
        ImGui::Text("Search:");
        ImGui::InputText("##search", s_searchBuffer, sizeof(s_searchBuffer));
        ImGui::Separator();
        
        // Group nodes by category
        std::map<std::string, std::vector<NodeTypeInfo*>> categorizedNodes;
        for (auto& nodeType : s_nodeTypes) {
            categorizedNodes[nodeType.category].push_back(&nodeType);
        }
        
        // Render each category
        for (auto& [category, nodes] : categorizedNodes) {
            if (ImGui::CollapsingHeader(category.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                for (auto* nodeType : nodes) {
                    // Filter by search
                    std::string searchLower = s_searchBuffer;
                    std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
                    std::string displayNameLower = nodeType->displayName;
                    std::transform(displayNameLower.begin(), displayNameLower.end(), displayNameLower.begin(), ::tolower);
                    
                    if (searchLower.empty() || displayNameLower.find(searchLower) != std::string::npos) {
                        if (ImGui::MenuItem(nodeType->displayName.c_str())) {
                            nodeType->createFunction();
                            ImGui::CloseCurrentPopup();
                        }
                    }
                }
            }
        }
        
        ImGui::EndPopup();
    }
}

void ContextMenu::Clear() {
    s_nodeTypes.clear();
}