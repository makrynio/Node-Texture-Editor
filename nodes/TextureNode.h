#pragma once

#include "BaseNode.h"

class TextureNode : public BaseNode {
public:
    TextureNode(int nodeId, const std::string& name, ChannelType channel, ColorMode colorMode);
    virtual ~TextureNode() = default;
    
    void Render(ax::NodeEditor::EditorContext* editor) override;
    void Update() override;
    
private:
    void GenerateTexture();
};
