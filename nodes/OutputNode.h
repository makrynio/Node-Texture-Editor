#pragma once

#include "BaseNode.h"

class OutputNode : public BaseNode {
public:
    OutputNode(int nodeId, ChannelType channel, ColorMode colorMode);
    virtual ~OutputNode() = default;
    
    void Render(ax::NodeEditor::EditorContext* editor) override;
    
private:
    static std::string GetChannelName(ChannelType channel);
    void RenderChannelPlaceholder(float thumbnailSize, int nodeId);
};
