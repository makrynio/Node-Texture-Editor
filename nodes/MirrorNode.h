#pragma once

#include "BaseNode.h"

class MirrorNode : public BaseNode {
public:
    MirrorNode(int nodeId);
    virtual ~MirrorNode() = default;
    
    virtual void Render(ax::NodeEditor::EditorContext* editor) override;
    virtual void Update() override;
    
    // Mirror properties
    enum MirrorMode {
        LeftToRight = 0,
        RightToLeft = 1,
        BottomToTop = 2,
        TopToBottom = 3
    };
    
    void SetMirrorMode(MirrorMode mode) { m_mirrorMode = mode; m_needsUpdate = true; }
    MirrorMode GetMirrorMode() const { return m_mirrorMode; }
    
    void SetOffset(float offset) { m_offset = offset; m_needsUpdate = true; }
    float GetOffset() const { return m_offset; }
    
    void SetClamp(bool clamp) { m_clamp = clamp; m_needsUpdate = true; }
    bool GetClamp() const { return m_clamp; }
    
    void GenerateTexture();

private:
    MirrorMode m_mirrorMode;
    float m_offset;
    bool m_clamp;
    bool m_needsUpdate;
    
    void GenerateMirrorTexture();
};
