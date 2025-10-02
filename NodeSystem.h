#pragma once

#include "nodes/BaseNode.h"
#include <imgui.h>
#include <imgui_node_editor.h>
#include <memory>
#include <string>
#include <map>
#include <vector>

using namespace ax::NodeEditor;

struct NodeLink {
    ax::NodeEditor::LinkId id;
    ax::NodeEditor::PinId inputId;
    ax::NodeEditor::PinId outputId;

    NodeLink(ax::NodeEditor::LinkId linkId, ax::NodeEditor::PinId inPin, ax::NodeEditor::PinId outPin)
        : id(linkId), inputId(inPin), outputId(outPin) {}
};

class NodeSystem {
public:
    static void RegisterNode(std::unique_ptr<BaseNode> node);
    static void CreateOutputNodes();
    static void CreateTextureNode();
    static void CreateBlendNode();
    static void CreateFractalNoiseNode();
    static void CreateNormalMapNode();
    static void CreateDefaultNormalConnection();
    
    static BaseNode* GetNode(int nodeId);
    static void RenderNode(int nodeId, ax::NodeEditor::EditorContext* editor);
    static void RenderConnections(ax::NodeEditor::EditorContext* editor);
    static void HandleConnections(ax::NodeEditor::EditorContext* editor);
    static void UpdateNodeConnections();
    static void RenderTexturePreview();
    static void SetSelectedNode(int nodeId);
    
    // Connection management functions
    static bool CanCreateConnection(BaseNode* inputNode, BaseNode* outputNode);
    static void DisconnectInputPin(ax::NodeEditor::PinId inputPinId);
    static void UpdateNodeConnection(BaseNode* inputNode, BaseNode* outputNode);
    static void PropagateNodeUpdates(BaseNode* startNode);
    static void InvalidateConnectionChain();
    static void RebuildConnectionChain();
    static int GetSelectedNode();
    static BaseNode* GetSelectedNodePtr();
    static void AddToSelection(int nodeId);
    static void RemoveFromSelection(int nodeId);
    static void ClearSelection();
    static const std::vector<int>& GetSelectedNodes();
    static void DeleteSelectedNodes();
    static void DeleteSelectedConnections();
    static void DeleteNode(int nodeId);
    static const std::vector<NodeLink>& GetLinks();
    static void RestoreConnection(const NodeLink& connection);
    static void DeleteLink(ax::NodeEditor::LinkId linkId);
    static int CreateNode(const std::string& nodeType, const std::string& channel = "");
    static int GetNextNodeId();
    static const std::map<int, std::unique_ptr<BaseNode>>& GetNodes();
    static void Cleanup();
    
    // Layout methods
    static void CenterAndAlignNodes();
    static void FocusOnNode(int nodeId);
    static void RecordInitialOutputNodePositions();
    static void ResetOutputNodesToInitialPositions();
    static ImVec2 GetInitialOutputNodePosition(int nodeId);
    
    // Update methods
    static void UpdateAllNodes();
    
    // PBR output methods
    static unsigned int GetOutputTexture(ChannelType channel);
    static void UpdatePBRTextures();
    static BaseNode* GetNodeByOutputTexture(unsigned int textureID);
    
private:
    static std::map<int, std::unique_ptr<BaseNode>> s_nodes;
    static std::vector<NodeLink> s_links;
    static int s_nextLinkId;
    static int s_nextNodeId;
    static int s_selectedNodeId;
    static std::vector<int> s_selectedNodes;
    static std::vector<ax::NodeEditor::LinkId> s_selectedLinks;
    static std::map<int, ImVec2> s_initialOutputNodePositions; // Store initial positions of output nodes (1-5)
    static BaseNode* GetNodeByInputPin(ax::NodeEditor::PinId pinId);
    static BaseNode* GetNodeByOutputPin(ax::NodeEditor::PinId pinId);
};
