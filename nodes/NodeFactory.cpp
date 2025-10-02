#include "NodeFactory.h"
#include "OutputNode.h"
#include "TextureNode.h"
#include "BlendNode.h"
#include "FractalNoiseNode.h"
#include "GradientNode.h"
#include "TriGradientNode.h"
#include "MirrorNode.h"
#include "NormalMapNode.h"
#include "WarpNode.h"
#include "BetterWarpNode.h"
#include "TileNode.h"
#include "Transform2DNode.h"
#include "BlurV2Node.h"
#include "ImageNode.h"

std::unique_ptr<BaseNode> NodeFactory::CreateOutputNode(int nodeId, ChannelType channel) {
    ColorMode colorMode = GetColorModeForChannel(channel);
    return std::make_unique<OutputNode>(nodeId, channel, colorMode);
}

std::unique_ptr<BaseNode> NodeFactory::CreateTextureNode(int nodeId, const std::string& name, 
                                                         ChannelType channel, ColorMode colorMode) {
    return std::make_unique<TextureNode>(nodeId, name, channel, colorMode);
}

std::unique_ptr<BaseNode> NodeFactory::CreateBlendNode(int nodeId) {
    return std::make_unique<BlendNode>(nodeId);
}

std::unique_ptr<BaseNode> NodeFactory::CreateFractalNoiseNode(int nodeId) {
    return std::make_unique<FractalNoiseNode>(nodeId);
}

std::unique_ptr<BaseNode> NodeFactory::CreateGradientNode(int nodeId) {
    return std::make_unique<GradientNode>(nodeId);
}

std::unique_ptr<BaseNode> NodeFactory::CreateTriGradientNode(int nodeId) {
    return std::make_unique<TriGradientNode>(nodeId);
}

std::unique_ptr<BaseNode> NodeFactory::CreateMirrorNode(int nodeId) {
    return std::make_unique<MirrorNode>(nodeId);
}

std::unique_ptr<BaseNode> NodeFactory::CreateNormalMapNode(int nodeId) {
    return std::make_unique<NormalMapNode>(nodeId);
}

std::unique_ptr<BaseNode> NodeFactory::CreateWarpNode(int nodeId) {
    return std::make_unique<WarpNode>(nodeId);
}

std::unique_ptr<BaseNode> NodeFactory::CreateBetterWarpNode(int nodeId) {
    return std::make_unique<BetterWarpNode>(nodeId);
}

std::unique_ptr<BaseNode> NodeFactory::CreateTileNode(int nodeId) {
    return std::make_unique<TileNode>(nodeId);
}

std::unique_ptr<BaseNode> NodeFactory::CreateTransform2DNode(int nodeId) {
    return std::make_unique<Transform2DNode>(nodeId);
}

std::unique_ptr<BaseNode> NodeFactory::CreateBlurV2Node(int nodeId) {
    return std::make_unique<BlurV2Node>(nodeId);
}

std::unique_ptr<BaseNode> NodeFactory::CreateImageNode(int nodeId, const std::string& name, 
                                                       ChannelType channel, ColorMode colorMode) {
    return std::make_unique<ImageNode>(nodeId, name, channel, colorMode);
}

ColorMode NodeFactory::GetColorModeForChannel(ChannelType channel) {
    switch (channel) {
        case ChannelType::Color:
        case ChannelType::Normal:
            return ColorMode::RGB;
        case ChannelType::Roughness:
        case ChannelType::Metallic:
        case ChannelType::Height:
            return ColorMode::Greyscale;
        default:
            return ColorMode::RGB;
    }
}
