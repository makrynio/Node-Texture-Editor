#pragma once

#include "BaseParameter.h"
#include "../nodes/BaseNode.h"
#include <vector>
#include <memory>

class TriGradientNodeParameters {
public:
    TriGradientNodeParameters();
    ~TriGradientNodeParameters() = default;
    
    void SetNode(BaseNode* node);
    void Render();
    void Clear();
    
    bool IsValid() const { return m_node != nullptr; }
    
private:
    BaseNode* m_node;
    std::vector<std::unique_ptr<BaseParameter>> m_parameters;
    
    // TriGradient-specific parameters
    ImVec4 m_colorA;
    float m_positionA;
    ImVec4 m_colorB;
    float m_positionB;
    ImVec4 m_colorC;
    float m_positionC;
    int m_mode;
    
    void CreateParameters();
    void UpdateParameters();
    void ApplyChanges();
};
