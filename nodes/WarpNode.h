#pragma once
#include "BaseNode.h"
#include "../TextureGenerator.h" // For TextureData
#include <vector>

class WarpNode : public BaseNode {
public:
    WarpNode(int nodeId);
    ~WarpNode() = default;

    void Render(ax::NodeEditor::EditorContext* editor) override;
    void Update() override;
    void GenerateTexture();

    // Parameters
    float GetStrength() const { return m_strength; }
    void SetStrength(float strength) { m_strength = strength; m_needsUpdate = true; }
    
    float GetOffsetX() const { return m_offsetX; }
    void SetOffsetX(float offsetX) { m_offsetX = offsetX; m_needsUpdate = true; }
    
    float GetOffsetY() const { return m_offsetY; }
    void SetOffsetY(float offsetY) { m_offsetY = offsetY; m_needsUpdate = true; }

private:
    float m_strength;
    float m_offsetX;
    float m_offsetY;
    bool m_needsUpdate;
    
    void GenerateWarpTexture();
};
