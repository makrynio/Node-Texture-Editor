#pragma once

#include "BaseParameter.h"
#include "../nodes/BaseNode.h"
#include <vector>
#include <memory>

class GradientNodeParameters {
public:
    GradientNodeParameters();
    ~GradientNodeParameters() = default;
    
    void SetNode(BaseNode* node);
    void Render();
    void Clear();
    
    bool IsValid() const { return m_node != nullptr; }
    
private:
    BaseNode* m_node;
    std::vector<std::unique_ptr<BaseParameter>> m_parameters;
    
    // Gradient-specific parameters
    ImVec4 m_colorA;
    float m_positionA;
    ImVec4 m_colorB;
    float m_positionB;
    int m_mode;
    
    void CreateParameters();
    void UpdateParameters();
    void ApplyChanges();
};
