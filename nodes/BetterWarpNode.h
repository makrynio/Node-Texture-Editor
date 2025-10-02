#pragma once
#include "BaseNode.h"
#include "../TextureGenerator.h" // For TextureData
#include <vector>

class BetterWarpNode : public BaseNode {
public:
    BetterWarpNode(int nodeId);
    ~BetterWarpNode() = default;

    void Render(ax::NodeEditor::EditorContext* editor) override;
    void Update() override;
    void GenerateTexture();

    // Getters
    PinId GetInputPin2Id() const { return m_inputPin2Id; }
    
    // Parameters
    float GetStrength() const { return m_strength; }
    void SetStrength(float strength) { m_strength = strength; m_needsUpdate = true; }
    
    float GetSpread() const { return m_spread; }
    void SetSpread(float spread) { m_spread = spread; m_needsUpdate = true; }

private:
    PinId m_inputPin2Id;  // Second input pin
    float m_strength;
    float m_spread;
    bool m_needsUpdate;
    
    void GenerateWarpTexture();
    float GetHeightValue(const std::vector<unsigned char>& data, int width, int height, int x, int y);
};
