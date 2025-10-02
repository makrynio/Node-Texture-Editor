#pragma once
#include "../nodes/BaseNode.h"
#include "../nodes/WarpNode.h"
#include <imgui.h>
#include <vector>
#include <memory>
#include <string>

class WarpNodeParameters {
public:
    WarpNodeParameters();
    ~WarpNodeParameters() = default;

    void SetNode(BaseNode* node);
    void Render();
    void ApplyChanges();
    void Clear();

    bool IsValid() const { return m_warpNode != nullptr; }

private:
    WarpNode* m_warpNode;

    // Parameter values
    float m_strength;
    float m_offsetX;
    float m_offsetY;
    
    // Display information
    int m_textureWidth;
    int m_textureHeight;

    void UpdateParameters();
};
