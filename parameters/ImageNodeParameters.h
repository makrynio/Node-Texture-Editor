#pragma once

#include "BaseParameter.h"
#include "../nodes/BaseNode.h"
#include <vector>
#include <memory>
#include <string>

class ImageNodeParameters {
public:
    ImageNodeParameters();
    ~ImageNodeParameters() = default;
    
    void SetNode(BaseNode* node);
    void Render();
    void Clear();
    
    bool IsValid() const { return m_node != nullptr; }
    
private:
    BaseNode* m_node;
    std::vector<std::unique_ptr<BaseParameter>> m_parameters;
    
    // Image-specific parameters
    std::string m_imagePath;
    std::string m_imageName;
    int m_width;
    int m_height;
    int m_channels;
    
    void CreateParameters();
    void UpdateParameters();
    void OpenFileDialog();
    void LoadImage(const std::string& filePath);
};
