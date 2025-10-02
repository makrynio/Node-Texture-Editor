#pragma once

#include "BaseParameter.h"
#include "../nodes/BaseNode.h"
#include <vector>
#include <memory>

class TextureNodeParameters {
public:
    TextureNodeParameters();
    ~TextureNodeParameters() = default;
    
    void SetNode(BaseNode* node);
    void Render();
    void Clear();
    
    bool IsValid() const { return m_node != nullptr; }
    
private:
    BaseNode* m_node;
    std::vector<std::unique_ptr<BaseParameter>> m_parameters;
    
    // Texture-specific parameters
    int m_width;
    int m_height;
    int m_channels;
    std::string m_textureName;
    
    void CreateParameters();
    void UpdateParameters();
};
