#pragma once
#include "BaseNode.h"
#include "../TextureGenerator.h" // For TextureData
#include <vector>

class NormalMapNode : public BaseNode {
public:
    NormalMapNode(int nodeId);
    ~NormalMapNode() = default;

    void Render(ax::NodeEditor::EditorContext* editor) override;
    void Update() override;
    void GenerateTexture();

    // Parameters
    float GetStrength() const { return m_strength; }
    void SetStrength(float strength) { m_strength = strength; m_needsUpdate = true; }

private:
    float m_strength;
    bool m_needsUpdate;
    
    void GenerateNormalMapTexture();
    float GetHeightValue(const std::vector<unsigned char>& data, int width, int height, int x, int y);
};
