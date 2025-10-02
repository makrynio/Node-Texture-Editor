#pragma once

#include "BaseNode.h"
#include <string>

class ImageNode : public BaseNode {
public:
    ImageNode(int nodeId, const std::string& name, ChannelType channel, ColorMode colorMode);
    virtual ~ImageNode() = default;
    
    void Render(ax::NodeEditor::EditorContext* editor) override;
    void Update() override;
    
    // Image-specific methods
    bool LoadImage(const std::string& filePath);
    void ClearImage();
    const std::string& GetImagePath() const { return m_imagePath; }
    bool HasImage() const { return !m_imagePath.empty() && m_textureID != 0; }
    
private:
    std::string m_imagePath;
    void GenerateDefaultTexture();
};
