#pragma once
#include "BaseNode.h"
#include "../TextureGenerator.h" // For TextureData
#include <imgui_node_editor.h>
#include <vector>
#include <iostream>

class BlurV2Node : public BaseNode {
public:
    BlurV2Node(int nodeId);
    ~BlurV2Node() = default;

    void Render(ax::NodeEditor::EditorContext* editor) override;
    void Update() override;
    void GenerateTexture();

    // Parameters
    float GetIntensity() const { return m_intensity; }
    void SetIntensity(float intensity) { 
        std::cout << "BlurV2Node::SetIntensity() called with: " << intensity << " (was: " << m_intensity << ")" << std::endl;
        m_intensity = intensity; 
        m_needsUpdate = true; 
    }
    
    int GetSamples() const { return m_samples; }
    void SetSamples(int samples) { 
        std::cout << "BlurV2Node::SetSamples() called with: " << samples << " (was: " << m_samples << ")" << std::endl;
        m_samples = samples; 
        m_needsUpdate = true; 
    }

private:
    float m_intensity;
    int m_samples;
    bool m_needsUpdate;
    unsigned int m_lastInputTextureID;
    
    void GenerateBlurTexture();
};