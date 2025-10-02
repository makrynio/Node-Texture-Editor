#pragma once
#include "../nodes/BaseNode.h"
#include "../nodes/BetterWarpNode.h"
#include <imgui.h>
#include <vector>
#include <memory>

class BetterWarpNodeParameters {
public:
    BetterWarpNodeParameters();
    ~BetterWarpNodeParameters() = default;

    void SetNode(BaseNode* node);
    void Render();
    void ApplyChanges();
    void Clear();

    bool IsValid() const { return m_betterWarpNode != nullptr; }

private:
    BetterWarpNode* m_betterWarpNode;

    // Parameter values
    float m_strength;
    float m_spread;
    
    // Display information
    int m_textureWidth;
    int m_textureHeight;

    void UpdateParameters();
};
