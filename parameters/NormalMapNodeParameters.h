#pragma once
#include "../nodes/BaseNode.h"
#include "../nodes/NormalMapNode.h"
#include <imgui.h>
#include <vector>
#include <memory>

class NormalMapNodeParameters {
public:
    NormalMapNodeParameters();
    ~NormalMapNodeParameters() = default;

    void SetNode(BaseNode* node);
    void Render();
    void ApplyChanges();
    void Clear();

    bool IsValid() const { return m_normalMapNode != nullptr; }

private:
    NormalMapNode* m_normalMapNode;

    // Parameter values
    float m_strength;
    
    // Display information
    int m_textureWidth;
    int m_textureHeight;

    void UpdateParameters();
};
