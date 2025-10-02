#pragma once
#include <imgui.h>
#include <string>
#include <vector>
#include <functional>

struct NodeTypeInfo {
    std::string name;
    std::string displayName;
    std::string category;
    std::function<void()> createFunction;
    
    NodeTypeInfo(const std::string& n, const std::string& dn, const std::string& cat, std::function<void()> func)
        : name(n), displayName(dn), category(cat), createFunction(func) {}
};

class ContextMenu {
public:
    static void Show();
    static void Hide();
    static bool IsVisible();
    static void SetPosition(const ImVec2& pos);
    static void RegisterNodeType(const std::string& name, const std::string& displayName, 
                                const std::string& category, std::function<void()> createFunction);
    static void Render();
    static void Clear();

private:
    static bool s_visible;
    static ImVec2 s_position;
    static std::vector<NodeTypeInfo> s_nodeTypes;
    static char s_searchBuffer[256];
};
