#pragma once

#include "BaseNode.h"
#include <imgui.h>
#include <imgui_node_editor.h>

class GradientNode : public BaseNode {
public:
    GradientNode(int nodeId, const std::string& name = "Gradient");
    virtual ~GradientNode() = default;

    virtual void Render(ax::NodeEditor::EditorContext* editor) override;
    virtual void Update() override;
    
    // Gradient properties
    ImVec4 GetColorA() const { return m_colorA; }
    void SetColorA(const ImVec4& color) { m_colorA = color; m_needsUpdate = true; }
    
    float GetPositionA() const { return m_positionA; }
    void SetPositionA(float pos) { m_positionA = pos; m_needsUpdate = true; }
    
    ImVec4 GetColorB() const { return m_colorB; }
    void SetColorB(const ImVec4& color) { m_colorB = color; m_needsUpdate = true; }
    
    float GetPositionB() const { return m_positionB; }
    void SetPositionB(float pos) { m_positionB = pos; m_needsUpdate = true; }
    
    int GetMode() const { return m_mode; }
    void SetMode(int mode) { m_mode = mode; m_needsUpdate = true; }
    
    // Public method for parameter system
    void GenerateTexture();

private:
    
    ImVec4 m_colorA;
    float m_positionA;
    ImVec4 m_colorB;
    float m_positionB;
    int m_mode; // 0: Left to Right, 1: Right to Left, 2: Top to Bottom, 3: Bottom to Top
    bool m_needsUpdate;
};
