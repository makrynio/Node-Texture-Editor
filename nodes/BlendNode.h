#pragma once

#include "BaseNode.h"
#include <string>

enum class BlendMode {
    Add = 0,
    Multiply = 1,
    Subtract = 2,
    Divide = 3,
    Screen = 4,
    Overlay = 5,
    Darken = 6,
    Lighten = 7,
    Difference = 8,
    Count
};

class BlendNode : public BaseNode {
public:
    BlendNode(int nodeId);
    virtual ~BlendNode() = default;
    
    void Render(ax::NodeEditor::EditorContext* editor) override;
    void Update() override;
    
    // Getters
    BlendMode GetBlendMode() const { return m_blendMode; }
    float GetOpacity() const { return m_opacity; }
    
    // Setters
    void SetBlendMode(BlendMode mode) { m_blendMode = mode; }
    void SetOpacity(float opacity) { m_opacity = opacity; }
    
    // Additional input pin getters
    PinId GetInputPin2Id() const { return m_inputPin2Id; }
    
    // Additional input texture setters
    void SetInputTexture2Data(std::shared_ptr<TextureData> data) { m_inputTexture2Data = data; }
    void SetInputTexture2ID(unsigned int id) { m_inputTexture2ID = id; }
    
private:
    BlendMode m_blendMode;
    float m_opacity;
    PinId m_inputPin2Id;  // Second input pin
    std::shared_ptr<TextureData> m_inputTexture2Data;
    unsigned int m_inputTexture2ID;
    
    void ProcessBlend();
    std::string GetBlendModeName(BlendMode mode) const;
};
