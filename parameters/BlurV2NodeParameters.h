#pragma once
#include "../nodes/BaseNode.h"
#include <imgui.h>
#include <vector>
#include <memory>
#include <string>

// Forward declaration
class BlurV2Node;

class BlurV2NodeParameters {
public:
    BlurV2NodeParameters();
    ~BlurV2NodeParameters() = default;

    void SetNode(BaseNode* node);
    void Render();
    void ApplyChanges();
    void Clear();
    void UpdateParameters();

    bool IsValid() const { return m_blurNode != nullptr; }

private:
    BlurV2Node* m_blurNode;

    // Parameter values
    float m_intensity;
    int m_samples;
    
    // Display information
    int m_textureWidth;
    int m_textureHeight;

};
