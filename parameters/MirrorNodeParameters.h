#pragma once

#include "../nodes/BaseNode.h"
#include "../nodes/MirrorNode.h"
#include <imgui.h>

class MirrorNodeParameters {
public:
    MirrorNodeParameters();
    ~MirrorNodeParameters() = default;
    
    void SetNode(BaseNode* node);
    void Render();
    void ApplyChanges();
    void Clear();
    
    bool IsValid() const { return m_mirrorNode != nullptr; }

private:
    MirrorNode* m_mirrorNode;
    
    // Parameter values
    int m_mirrorMode;
    float m_offset;
    bool m_clamp;
    
    // Display information
    int m_textureWidth;
    int m_textureHeight;
    
    void UpdateParameters();
};
