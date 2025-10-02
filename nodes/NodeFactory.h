#pragma once

#include "BaseNode.h"
#include <memory>
#include <map>

class NodeFactory {
public:
    static std::unique_ptr<BaseNode> CreateOutputNode(int nodeId, ChannelType channel);
    static std::unique_ptr<BaseNode> CreateTextureNode(int nodeId, const std::string& name, 
                                                       ChannelType channel, ColorMode colorMode);
    static std::unique_ptr<BaseNode> CreateBlendNode(int nodeId);
    static std::unique_ptr<BaseNode> CreateFractalNoiseNode(int nodeId);
    static std::unique_ptr<BaseNode> CreateGradientNode(int nodeId);
    static std::unique_ptr<BaseNode> CreateTriGradientNode(int nodeId);
    static std::unique_ptr<BaseNode> CreateMirrorNode(int nodeId);
    static std::unique_ptr<BaseNode> CreateNormalMapNode(int nodeId);
    static std::unique_ptr<BaseNode> CreateWarpNode(int nodeId);
    static std::unique_ptr<BaseNode> CreateBetterWarpNode(int nodeId);
    static std::unique_ptr<BaseNode> CreateTileNode(int nodeId);
    static std::unique_ptr<BaseNode> CreateTransform2DNode(int nodeId);
    static std::unique_ptr<BaseNode> CreateBlurV2Node(int nodeId);
    static std::unique_ptr<BaseNode> CreateImageNode(int nodeId, const std::string& name, 
                                                     ChannelType channel, ColorMode colorMode);
    
    // Helper function to get color mode for channel
    static ColorMode GetColorModeForChannel(ChannelType channel);
};
