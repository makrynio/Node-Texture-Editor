#include "ParametersWindow.h"
#include "../nodes/BaseNode.h"
#include "../nodes/OutputNode.h"
#include "../nodes/TextureNode.h"
#include "../nodes/ImageNode.h"
#include <iostream>

ParametersWindow::ParametersWindow() 
    : m_visible(true), m_selectedNode(nullptr) {
    
    // Initialize parameter managers
    m_outputParams = std::make_unique<OutputNodeParameters>();
    m_textureParams = std::make_unique<TextureNodeParameters>();
    m_fractalNoiseParams = std::make_unique<FractalNoiseParameters>();
    m_gradientParams = std::make_unique<GradientNodeParameters>();
    m_triGradientParams = std::make_unique<TriGradientNodeParameters>();
    m_mirrorParams = std::make_unique<MirrorNodeParameters>();
    m_normalMapParams = std::make_unique<NormalMapNodeParameters>();
    m_warpParams = std::make_unique<WarpNodeParameters>();
    m_betterWarpParams = std::make_unique<BetterWarpNodeParameters>();
    m_tileParams = std::make_unique<TileNodeParameters>();
    m_transform2DParams = std::make_unique<Transform2DNodeParameters>();
    m_blurV2Params = std::make_unique<BlurV2NodeParameters>();
    m_imageParams = std::make_unique<ImageNodeParameters>();
}

void ParametersWindow::Render() {
    if (!m_visible) return;
    
    ImGui::Begin("Parameters", &m_visible);
    
    if (m_selectedNode) {
        RenderNodeInfo();
        ImGui::Separator();
        RenderParameters();
    } else {
        ImGui::Text("No node selected");
        ImGui::Text("Click on a node to view its parameters");
    }
    
    ImGui::End();
}

void ParametersWindow::SetSelectedNode(BaseNode* node) {
    // Only update if the node actually changed
    if (m_selectedNode == node) {
        return;  // Early return if same node
    }
    
    m_selectedNode = node;
    
    if (!node) {
        ClearSelection();
        return;
    }
    
    // Set the appropriate parameter manager based on node type
    switch (node->GetType()) {
        case NodeType::Output:
            m_outputParams->SetNode(node);
            break;
        case NodeType::Texture:
            // Check if it's an ImageNode
            if (node->GetName() == "Image") {
                m_imageParams->SetNode(node);
            } else {
                m_textureParams->SetNode(node);
            }
            break;
        case NodeType::Utility:
            // Check if it's a FractalNoiseNode (which is a texture generator)
            if (node->GetName() == "Fractal Noise") {
                m_fractalNoiseParams->SetNode(node);
            } else if (node->GetName() == "Gradient") {
                m_gradientParams->SetNode(node);
            } else if (node->GetName() == "TriGradient") {
                m_triGradientParams->SetNode(node);
            } else if (node->GetName() == "Mirror") {
                m_mirrorParams->SetNode(node);
            } else if (node->GetName() == "Normal Map") {
                m_normalMapParams->SetNode(node);
            } else if (node->GetName() == "Warp") {
                m_warpParams->SetNode(node);
            } else if (node->GetName() == "Better Warp") {
                m_betterWarpParams->SetNode(node);
            } else if (node->GetName() == "Tile") {
                m_tileParams->SetNode(node);
            } else if (node->GetName() == "Transform2D") {
                m_transform2DParams->SetNode(node);
            } else if (node->GetName() == "Blur") {
                m_blurV2Params->SetNode(node);
            }
            break;
        default:
            std::cout << "Unknown node type for parameters" << std::endl;
            break;
    }
}

void ParametersWindow::ClearSelection() {
    m_selectedNode = nullptr;
    m_outputParams->Clear();
    m_textureParams->Clear();
    m_fractalNoiseParams->Clear();
    m_gradientParams->Clear();
    m_triGradientParams->Clear();
    m_mirrorParams->Clear();
    m_normalMapParams->Clear();
    m_warpParams->Clear();
    m_betterWarpParams->Clear();
    m_tileParams->Clear();
    m_transform2DParams->Clear();
    m_blurV2Params->Clear();
    m_imageParams->Clear();
}

void ParametersWindow::RenderNodeInfo() {
    if (!m_selectedNode) return;
    
    ImGui::Text("Node ID: %d", m_selectedNode->GetNodeId());
    ImGui::Text("Type: %s", GetNodeTypeName(m_selectedNode).c_str());
    ImGui::Text("Name: %s", m_selectedNode->GetName().c_str());
}

void ParametersWindow::RenderParameters() {
    if (!m_selectedNode) return;
    
    switch (m_selectedNode->GetType()) {
        case NodeType::Output:
            m_outputParams->Render();
            break;
        case NodeType::Texture:
            // Check if it's an ImageNode
            if (m_selectedNode->GetName() == "Image") {
                m_imageParams->Render();
            } else {
                m_textureParams->Render();
            }
            break;
        case NodeType::Utility:
            // Check if it's a FractalNoiseNode (which is a texture generator)
            if (m_selectedNode->GetName() == "Fractal Noise") {
                m_fractalNoiseParams->Render();
            } else if (m_selectedNode->GetName() == "Gradient") {
                m_gradientParams->Render();
            } else if (m_selectedNode->GetName() == "TriGradient") {
                m_triGradientParams->Render();
            } else if (m_selectedNode->GetName() == "Mirror") {
                m_mirrorParams->Render();
            } else if (m_selectedNode->GetName() == "Normal Map") {
                m_normalMapParams->Render();
            } else if (m_selectedNode->GetName() == "Warp") {
                m_warpParams->Render();
            } else if (m_selectedNode->GetName() == "Better Warp") {
                m_betterWarpParams->Render();
            } else if (m_selectedNode->GetName() == "Tile") {
                m_tileParams->Render();
            } else if (m_selectedNode->GetName() == "Transform2D") {
                m_transform2DParams->Render();
            } else if (m_selectedNode->GetName() == "Blur") {
                m_blurV2Params->Render();
            } else {
                ImGui::Text("No parameters available for this utility node type");
            }
            break;
        default:
            ImGui::Text("No parameters available for this node type");
            break;
    }
}

std::string ParametersWindow::GetNodeTypeName(BaseNode* node) const {
    if (!node) return "Unknown";
    
    switch (node->GetType()) {
        case NodeType::Output: return "Output";
        case NodeType::Texture: return "Texture";
        case NodeType::Utility: return "Utility";
        default: return "Unknown";
    }
}