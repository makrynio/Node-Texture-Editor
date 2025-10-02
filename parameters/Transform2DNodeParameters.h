#pragma once

#include "BaseParameter.h"
#include "../nodes/BaseNode.h"
#include <vector>
#include <memory>

class Transform2DNodeParameters {
public:
    Transform2DNodeParameters();
    ~Transform2DNodeParameters() = default;
    
    void SetNode(BaseNode* node);
    void Render();
    void Clear();
    
    bool IsValid() const { return m_node != nullptr; }
    
private:
    BaseNode* m_node;
    std::vector<std::unique_ptr<BaseParameter>> m_parameters;
    
    // Transform2D-specific parameters
    float m_translateX;
    float m_translateY;
    float m_scaleX;
    float m_scaleY;
    float m_rotation;
    bool m_clamp;
    
    void CreateParameters();
    void UpdateParameters();
    void ApplyChanges();
};
