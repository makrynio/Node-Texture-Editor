#pragma once

#include "BaseNode.h"
#include <imgui.h>
#include <imgui_node_editor.h>
#include <string>

class Transform2DNode : public BaseNode {
public:
    Transform2DNode(int nodeId);
    virtual ~Transform2DNode() = default;
    
    void Render(ax::NodeEditor::EditorContext* editor) override;
    void Update() override;
    
    // Getters
    float GetTranslateX() const { return m_translateX; }
    float GetTranslateY() const { return m_translateY; }
    float GetScaleX() const { return m_scaleX; }
    float GetScaleY() const { return m_scaleY; }
    float GetRotation() const { return m_rotation; }
    bool GetClamp() const { return m_clamp; }
    
    // Setters
    void SetTranslateX(float value) { m_translateX = value; }
    void SetTranslateY(float value) { m_translateY = value; }
    void SetScaleX(float value) { m_scaleX = value; }
    void SetScaleY(float value) { m_scaleY = value; }
    void SetRotation(float value) { m_rotation = value; }
    void SetClamp(bool value) { m_clamp = value; }
    
private:
    float m_translateX;
    float m_translateY;
    float m_scaleX;
    float m_scaleY;
    float m_rotation;
    bool m_clamp;
    
    void ProcessTransform();
};
