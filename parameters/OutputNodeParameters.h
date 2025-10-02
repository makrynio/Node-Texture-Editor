#pragma once

#include "BaseParameter.h"
#include "../nodes/BaseNode.h"
#include <vector>
#include <memory>

class OutputNodeParameters {
public:
    OutputNodeParameters();
    ~OutputNodeParameters() = default;
    
    void SetNode(BaseNode* node);
    void Render();
    void Clear();
    
    bool IsValid() const { return m_node != nullptr; }
    
private:
    BaseNode* m_node;
    std::vector<std::unique_ptr<BaseParameter>> m_parameters;
    
    void CreateParameters();
    void UpdateParameters();
};
