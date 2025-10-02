#pragma once

#include <imgui.h>
#include <memory>
#include <string>
#include "BaseParameter.h"
#include "OutputNodeParameters.h"
#include "TextureNodeParameters.h"
#include "FractalNoiseParameters.h"
#include "GradientNodeParameters.h"
#include "TriGradientNodeParameters.h"
#include "MirrorNodeParameters.h"
#include "NormalMapNodeParameters.h"
#include "WarpNodeParameters.h"
#include "BetterWarpNodeParameters.h"
#include "TileNodeParameters.h"
#include "Transform2DNodeParameters.h"
#include "BlurV2NodeParameters.h"
#include "ImageNodeParameters.h"

class ParametersWindow {
public:
    ParametersWindow();
    ~ParametersWindow() = default;
    
    void Render();
    void SetSelectedNode(BaseNode* node);
    void ClearSelection();
    
    bool IsVisible() const { return m_visible; }
    void SetVisible(bool visible) { m_visible = visible; }
    
private:
    bool m_visible;
    BaseNode* m_selectedNode;
    
    // Parameter managers for different node types
    std::unique_ptr<OutputNodeParameters> m_outputParams;
    std::unique_ptr<TextureNodeParameters> m_textureParams;
    std::unique_ptr<FractalNoiseParameters> m_fractalNoiseParams;
    std::unique_ptr<GradientNodeParameters> m_gradientParams;
    std::unique_ptr<TriGradientNodeParameters> m_triGradientParams;
    std::unique_ptr<MirrorNodeParameters> m_mirrorParams;
    std::unique_ptr<NormalMapNodeParameters> m_normalMapParams;
    std::unique_ptr<WarpNodeParameters> m_warpParams;
    std::unique_ptr<BetterWarpNodeParameters> m_betterWarpParams;
    std::unique_ptr<TileNodeParameters> m_tileParams;
    std::unique_ptr<Transform2DNodeParameters> m_transform2DParams;
    std::unique_ptr<BlurV2NodeParameters> m_blurV2Params;
    std::unique_ptr<ImageNodeParameters> m_imageParams;
    
    void RenderNodeInfo();
    void RenderParameters();
    std::string GetNodeTypeName(BaseNode* node) const;
};
