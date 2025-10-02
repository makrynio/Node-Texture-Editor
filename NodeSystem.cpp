#include "NodeSystem.h"
#include "nodes/NodeFactory.h"
#include "nodes/BlendNode.h"
#include "nodes/FractalNoiseNode.h"
#include <iostream>
#include <algorithm>
#include <cfloat>

using namespace ax::NodeEditor;

std::map<int, std::unique_ptr<BaseNode>> NodeSystem::s_nodes;
std::vector<NodeLink> NodeSystem::s_links;
int NodeSystem::s_nextLinkId = 1;
int NodeSystem::s_nextNodeId = 10; // Start from 10 to avoid conflicts with existing nodes
int NodeSystem::s_selectedNodeId = 0;
std::vector<int> NodeSystem::s_selectedNodes;
std::vector<ax::NodeEditor::LinkId> NodeSystem::s_selectedLinks;
std::map<int, ImVec2> NodeSystem::s_initialOutputNodePositions;

void NodeSystem::RegisterNode(std::unique_ptr<BaseNode> node) {
    if (node) {
        s_nodes[node->GetNodeId()] = std::move(node);
    }
}

void NodeSystem::CreateOutputNodes() {
    // Create 5 output nodes with different channels
    RegisterNode(NodeFactory::CreateOutputNode(1, ChannelType::Color));
    RegisterNode(NodeFactory::CreateOutputNode(2, ChannelType::Roughness));
    RegisterNode(NodeFactory::CreateOutputNode(3, ChannelType::Metallic));
    RegisterNode(NodeFactory::CreateOutputNode(4, ChannelType::Normal));
    RegisterNode(NodeFactory::CreateOutputNode(5, ChannelType::Height));
    
    // Set initial default positions for output nodes (arranged vertically)
    s_initialOutputNodePositions[1] = ImVec2(100, 100);  // Color
    s_initialOutputNodePositions[2] = ImVec2(100, 200);  // Roughness
    s_initialOutputNodePositions[3] = ImVec2(100, 300);  // Metallic
    s_initialOutputNodePositions[4] = ImVec2(100, 400);  // Normal
    s_initialOutputNodePositions[5] = ImVec2(100, 500);  // Height
}

void NodeSystem::CreateTextureNode() {
    // Create 1 texture node
    RegisterNode(NodeFactory::CreateTextureNode(6, "Checker", ChannelType::Color, ColorMode::Greyscale));
}

void NodeSystem::CreateBlendNode() {
    // Create 1 blend node
    RegisterNode(NodeFactory::CreateBlendNode(7));
}

void NodeSystem::CreateFractalNoiseNode() {
    // Create 1 fractal noise node
    RegisterNode(NodeFactory::CreateFractalNoiseNode(8));
}

void NodeSystem::CreateNormalMapNode() {
    // Create 1 normal map node
    RegisterNode(NodeFactory::CreateNormalMapNode(9));
}

void NodeSystem::CreateDefaultNormalConnection() {
    // Create a default connection between normal map node (9) and normal output (4)
    BaseNode* normalMapNode = GetNode(9);
    BaseNode* normalOutputNode = GetNode(4);
    
    if (normalMapNode && normalOutputNode) {
        // Get the output pin from normal map node and input pin from normal output
        ax::NodeEditor::PinId outputPinId = normalMapNode->GetOutputPinId();
        ax::NodeEditor::PinId inputPinId = normalOutputNode->GetInputPinId();
        
        if (outputPinId && inputPinId) {
            // Create the link
            ax::NodeEditor::LinkId linkId = s_nextLinkId++;
            s_links.emplace_back(linkId, inputPinId, outputPinId);
            
            // Update the connection
            UpdateNodeConnection(normalOutputNode, normalMapNode);
            
            std::cout << "Created default connection between normal map node and normal output" << std::endl;
        }
    }
}

BaseNode* NodeSystem::GetNode(int nodeId) {
    auto it = s_nodes.find(nodeId);
    return (it != s_nodes.end()) ? it->second.get() : nullptr;
}

void NodeSystem::RenderNode(int nodeId, ax::NodeEditor::EditorContext* editor) {
    BaseNode* node = GetNode(nodeId);
    if (node) {
        // Debug: Check if this is a Normal Map node
        if (node->GetName() == "Normal Map") {
            std::cout << "Rendering Normal Map node with ID: " << nodeId << std::endl;
        }
        node->Render(editor);
    } else {
        // Debug: Check if we're trying to render a missing node
        if (nodeId > 10) { // Only check for newly created nodes
            std::cout << "WARNING: Trying to render node ID " << nodeId << " but it doesn't exist!" << std::endl;
        }
    }
}

void NodeSystem::RenderConnections(ax::NodeEditor::EditorContext* editor) {
    SetCurrentEditor(editor);
    
    // Render existing links first (like in the working example)
    for (const auto& link : s_links) {
        ax::NodeEditor::Link(link.id, link.inputId, link.outputId);
    }
}

void NodeSystem::HandleConnections(ax::NodeEditor::EditorContext* editor) {
    SetCurrentEditor(editor);
    
    // Handle node selection
    NodeId hoveredNodeId = GetHoveredNode();
    if (hoveredNodeId && ImGui::IsMouseClicked(0)) {
        SetSelectedNode(hoveredNodeId.Get());
    }
    
    // Handle background clicks to deselect
    if (IsBackgroundClicked()) {
        SetSelectedNode(0);
    }
    
    // Handle new links
    if (BeginCreate()) {
        PinId inputPinId, outputPinId;
        if (QueryNewLink(&inputPinId, &outputPinId)) {
            // Check if both pins are valid
            if (inputPinId && outputPinId) {
                // Check if this is a valid connection (output to input)
                // Note: QueryNewLink sometimes returns pins in swapped order
                BaseNode* inputNode = GetNodeByInputPin(inputPinId);
                BaseNode* outputNode = GetNodeByOutputPin(outputPinId);
                
                // If not found, try swapping the pin IDs
                if (!inputNode || !outputNode) {
                    inputNode = GetNodeByInputPin(outputPinId);
                    outputNode = GetNodeByOutputPin(inputPinId);
                    // Swap the pin IDs to maintain correct order
                    if (inputNode && outputNode) {
                        std::swap(inputPinId, outputPinId);
                    }
                }
                
                if (inputNode && outputNode) {
                    // Validate connection compatibility
                    if (CanCreateConnection(inputNode, outputNode)) {
                        // Show visual feedback for valid connection
                        if (AcceptNewItem(ImColor(128, 255, 128), 4.0f)) {
                            // Disconnect any existing connection to the input pin
                            DisconnectInputPin(inputPinId);
                            
                            // Create the new link
                            s_links.emplace_back(s_nextLinkId++, inputPinId, outputPinId);
                            
                            // Draw new link immediately
                            ax::NodeEditor::Link(s_links.back().id, s_links.back().inputId, s_links.back().outputId);
                            
                            // Update the input node with the output node's texture
                            UpdateNodeConnection(inputNode, outputNode);
                            
                            // Propagate updates through the connection chain
                            PropagateNodeUpdates(inputNode);
                        }
                    } else {
                        // Show visual feedback for invalid connection
                        if (inputNode == outputNode) {
                            RejectNewItem(ImColor(255, 0, 0), 2.0f);
                        } else if (outputNode->GetType() == NodeType::Output) {
                            RejectNewItem(ImColor(255, 0, 0), 2.0f);
                        } else {
                            RejectNewItem(ImColor(255, 128, 128), 1.0f);
                        }
                    }
                }
            }
        }
    }
    EndCreate();
    
    // Handle link deletion
    if (BeginDelete()) {
        LinkId deletedLinkId;
        while (QueryDeletedLink(&deletedLinkId)) {
            if (AcceptDeletedItem()) {
                // Find and remove the link
                auto it = std::find_if(s_links.begin(), s_links.end(),
                    [deletedLinkId](const NodeLink& link) { return link.id == deletedLinkId; });
                
                if (it != s_links.end()) {
                    // Clear the input texture from the connected input node
                    BaseNode* inputNode = GetNodeByInputPin(it->inputId);
                    if (inputNode) {
                        inputNode->SetInputTextureData(nullptr);
                        inputNode->SetInputTextureID(0);
                        
                        // Propagate updates through the connection chain
                        PropagateNodeUpdates(inputNode);
                    }
                    
                    s_links.erase(it);
                }
            }
        }
    }
    EndDelete();
}

void NodeSystem::UpdateNodeConnections() {
    // Update texture connections based on current links
    for (const auto& link : s_links) {
        BaseNode* inputNode = GetNodeByInputPin(link.inputId);
        BaseNode* outputNode = GetNodeByOutputPin(link.outputId);
        
        if (inputNode && outputNode && outputNode->GetTextureID() != 0) {
            // Check if this is a BlendNode with two inputs
            if (inputNode->GetType() == NodeType::Utility) {
                BlendNode* blendNode = dynamic_cast<BlendNode*>(inputNode);
                if (blendNode) {
                    // Determine which input pin this is
                    if (link.inputId == blendNode->GetInputPinId()) {
                        // First input (texture A)
                        blendNode->SetInputTextureData(outputNode->GetTextureData());
                        blendNode->SetInputTextureID(outputNode->GetTextureID());
                    } else if (link.inputId == blendNode->GetInputPin2Id()) {
                        // Second input (texture B)
                        blendNode->SetInputTexture2Data(outputNode->GetTextureData());
                        blendNode->SetInputTexture2ID(outputNode->GetTextureID());
                    }
                }
            } else {
                // Regular single input node
                inputNode->SetInputTextureData(outputNode->GetTextureData());
                inputNode->SetInputTextureID(outputNode->GetTextureID());
            }
        }
    }
}

void NodeSystem::RenderTexturePreview() {
    ImGui::Begin("Texture Preview", nullptr, ImGuiWindowFlags_None);
    
    if (s_selectedNodeId > 0) {
        BaseNode* node = GetNode(s_selectedNodeId);
        if (node) {
            ImGui::Text("Selected: %s", node->GetName().c_str());
            ImGui::Separator();
            
            // Get the texture to display (input texture if available, otherwise node's own texture)
            unsigned int displayTextureID = 0;
            
            // For certain node types, always show the output texture
            bool showOutput = (node->GetName() == "Normal Map" || 
                              node->GetName() == "Mirror" || 
                              node->GetName() == "Warp" ||
                              node->GetName() == "Better Warp" ||
                              node->GetName() == "Tile" ||
                              node->GetName() == "Transform2D" ||
                              node->GetName() == "Blur");
            
            if (showOutput && node->GetTextureID() != 0) {
                displayTextureID = node->GetTextureID();
                ImGui::Text("Displaying: Node Output");
            } else if (node->GetInputTextureID() != 0) {
                displayTextureID = node->GetInputTextureID();
                ImGui::Text("Displaying: Connected Input");
            } else if (node->GetTextureID() != 0) {
                displayTextureID = node->GetTextureID();
                ImGui::Text("Displaying: Node Output");
            }
            
            if (displayTextureID != 0) {
                // Get available space for the texture
                ImVec2 availableSize = ImGui::GetContentRegionAvail();
                float previewSize = std::min(availableSize.x - 20.0f, availableSize.y - 60.0f);
                previewSize = std::max(previewSize, 100.0f); // Minimum size
                
                // Center the texture
                ImVec2 center = ImGui::GetCursorScreenPos();
                center.x += (availableSize.x - previewSize) * 0.5f;
                center.y += (availableSize.y - previewSize) * 0.5f;
                
                ImGui::SetCursorScreenPos(center);
                
                // Display the texture
                ImGui::Image((void*)(intptr_t)displayTextureID,
                            ImVec2(previewSize, previewSize),
                            ImVec2(0, 0), ImVec2(1, 1),
                            ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 0.5f));
                
                // Add texture info
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + previewSize + 10);
                ImGui::Text("Size: %.0fx%.0f", previewSize, previewSize);
                ImGui::Text("Channel: %s", node->GetName().c_str());
                ImGui::Text("Mode: %s", node->GetColorMode() == ColorMode::RGB ? "RGB" : "Greyscale");
            } else {
                ImGui::Text("No texture available");
            }
        } else {
            ImGui::Text("Selected node not found");
        }
    } else {
        ImGui::Text("No node selected");
        ImGui::Text("Click on a node to preview its texture output");
    }
    
    ImGui::End();
}

void NodeSystem::SetSelectedNode(int nodeId) {
    s_selectedNodeId = nodeId;
}

int NodeSystem::GetSelectedNode() {
    return s_selectedNodeId;
}

BaseNode* NodeSystem::GetSelectedNodePtr() {
    if (s_selectedNodeId == 0) return nullptr;
    return GetNode(s_selectedNodeId);
}

BaseNode* NodeSystem::GetNodeByInputPin(ax::NodeEditor::PinId pinId) {
    std::cout << "GetNodeByInputPin called with pinId=" << pinId.Get() << std::endl;
    for (auto& pair : s_nodes) {
        std::cout << "Checking node " << pair.first << " (" << pair.second->GetName() << "), inputPin=" << pair.second->GetInputPinId().Get() << std::endl;
        if (pair.second->GetInputPinId() == pinId) {
            std::cout << "Found matching node!" << std::endl;
            return pair.second.get();
        }
        // Check additional pins for BlendNode
        if (pair.second->GetType() == NodeType::Utility) {
            BlendNode* blendNode = dynamic_cast<BlendNode*>(pair.second.get());
            if (blendNode && blendNode->GetInputPin2Id() == pinId) {
                return pair.second.get();
            }
        }
    }
    std::cout << "No matching node found" << std::endl;
    return nullptr;
}

BaseNode* NodeSystem::GetNodeByOutputPin(ax::NodeEditor::PinId pinId) {
    for (auto& pair : s_nodes) {
        if (pair.second->GetOutputPinId() == pinId) {
            return pair.second.get();
        }
    }
    return nullptr;
}

void NodeSystem::AddToSelection(int nodeId) {
    if (std::find(s_selectedNodes.begin(), s_selectedNodes.end(), nodeId) == s_selectedNodes.end()) {
        s_selectedNodes.push_back(nodeId);
    }
}

void NodeSystem::RemoveFromSelection(int nodeId) {
    s_selectedNodes.erase(std::remove(s_selectedNodes.begin(), s_selectedNodes.end(), nodeId), s_selectedNodes.end());
}

void NodeSystem::ClearSelection() {
    s_selectedNodes.clear();
    s_selectedLinks.clear();
    s_selectedNodeId = 0;
}

const std::vector<int>& NodeSystem::GetSelectedNodes() {
    return s_selectedNodes;
}

void NodeSystem::DeleteSelectedNodes() {
    for (int nodeId : s_selectedNodes) {
        // Remove all links connected to this node
        s_links.erase(std::remove_if(s_links.begin(), s_links.end(),
            [nodeId](const NodeLink& link) {
                BaseNode* inputNode = GetNodeByInputPin(link.inputId);
                BaseNode* outputNode = GetNodeByOutputPin(link.outputId);
                return (inputNode && inputNode->GetNodeId() == nodeId) || 
                       (outputNode && outputNode->GetNodeId() == nodeId);
            }), s_links.end());
        
        // Remove the node
        s_nodes.erase(nodeId);
    }
    s_selectedNodes.clear();
    s_selectedNodeId = 0;
}

void NodeSystem::DeleteSelectedConnections() {
    for (ax::NodeEditor::LinkId linkId : s_selectedLinks) {
        s_links.erase(std::remove_if(s_links.begin(), s_links.end(),
            [linkId](const NodeLink& link) {
                return link.id == linkId;
            }), s_links.end());
    }
    s_selectedLinks.clear();
}

const std::map<int, std::unique_ptr<BaseNode>>& NodeSystem::GetNodes() {
    return s_nodes;
}

void NodeSystem::DeleteNode(int nodeId) {
    // Remove all links connected to this node
    s_links.erase(std::remove_if(s_links.begin(), s_links.end(),
        [nodeId](const NodeLink& link) {
            BaseNode* inputNode = GetNodeByInputPin(link.inputId);
            BaseNode* outputNode = GetNodeByOutputPin(link.outputId);
            return (inputNode && inputNode->GetNodeId() == nodeId) || 
                   (outputNode && outputNode->GetNodeId() == nodeId);
        }), s_links.end());
    
    // Remove the node
    s_nodes.erase(nodeId);
    
    // Remove from selection if it was selected
    s_selectedNodes.erase(std::remove(s_selectedNodes.begin(), s_selectedNodes.end(), nodeId), s_selectedNodes.end());
    if (s_selectedNodeId == nodeId) {
        s_selectedNodeId = 0;
    }
}

void NodeSystem::DeleteLink(ax::NodeEditor::LinkId linkId) {
    s_links.erase(std::remove_if(s_links.begin(), s_links.end(),
        [linkId](const NodeLink& link) {
            return link.id == linkId;
        }), s_links.end());
}

int NodeSystem::CreateNode(const std::string& nodeType, const std::string& channel) {
    int nodeId = s_nextNodeId++;
    
    if (nodeType == "output") {
        if (channel == "color") {
            RegisterNode(NodeFactory::CreateOutputNode(nodeId, ChannelType::Color));
        } else if (channel == "roughness") {
            RegisterNode(NodeFactory::CreateOutputNode(nodeId, ChannelType::Roughness));
        } else if (channel == "metallic") {
            RegisterNode(NodeFactory::CreateOutputNode(nodeId, ChannelType::Metallic));
        } else if (channel == "normal") {
            RegisterNode(NodeFactory::CreateOutputNode(nodeId, ChannelType::Normal));
        } else if (channel == "height") {
            RegisterNode(NodeFactory::CreateOutputNode(nodeId, ChannelType::Height));
        } else {
            // Default to color
            RegisterNode(NodeFactory::CreateOutputNode(nodeId, ChannelType::Color));
        }
    } else if (nodeType == "texture") {
        RegisterNode(NodeFactory::CreateTextureNode(nodeId, "Checker", ChannelType::Color, ColorMode::Greyscale));
    } else if (nodeType == "blend") {
        RegisterNode(NodeFactory::CreateBlendNode(nodeId));
    } else if (nodeType == "fractal_noise") {
        RegisterNode(NodeFactory::CreateFractalNoiseNode(nodeId));
    } else if (nodeType == "gradient") {
        RegisterNode(NodeFactory::CreateGradientNode(nodeId));
    } else if (nodeType == "trigradient") {
        RegisterNode(NodeFactory::CreateTriGradientNode(nodeId));
    } else if (nodeType == "mirror") {
        RegisterNode(NodeFactory::CreateMirrorNode(nodeId));
    } else if (nodeType == "normalmap") {
        RegisterNode(NodeFactory::CreateNormalMapNode(nodeId));
    } else if (nodeType == "warp") {
        RegisterNode(NodeFactory::CreateWarpNode(nodeId));
    } else if (nodeType == "betterwarp") {
        RegisterNode(NodeFactory::CreateBetterWarpNode(nodeId));
    } else if (nodeType == "tile") {
        RegisterNode(NodeFactory::CreateTileNode(nodeId));
    } else if (nodeType == "transform2d") {
        RegisterNode(NodeFactory::CreateTransform2DNode(nodeId));
    } else if (nodeType == "blurv2") {
        RegisterNode(NodeFactory::CreateBlurV2Node(nodeId));
    } else if (nodeType == "image") {
        RegisterNode(NodeFactory::CreateImageNode(nodeId, "Image", ChannelType::Color, ColorMode::RGB));
    }
    
    return nodeId;
}

int NodeSystem::GetNextNodeId() {
    return s_nextNodeId++;
}

const std::vector<NodeLink>& NodeSystem::GetLinks() {
    return s_links;
}

void NodeSystem::RestoreConnection(const NodeLink& connection) {
    // Add the connection to the links vector
    s_links.push_back(connection);
    
    // Update the next link ID to avoid conflicts
    if (reinterpret_cast<uintptr_t>(connection.id.AsPointer()) >= s_nextLinkId) {
        s_nextLinkId = reinterpret_cast<uintptr_t>(connection.id.AsPointer()) + 1;
    }
}

void NodeSystem::Cleanup() {
    s_nodes.clear();
    s_links.clear();
    s_selectedNodeId = 0;
    s_selectedNodes.clear();
    s_selectedLinks.clear();
    s_initialOutputNodePositions.clear();
}

unsigned int NodeSystem::GetOutputTexture(ChannelType channel) {
    // Find the output node for this channel
    for (const auto& pair : s_nodes) {
        BaseNode* node = pair.second.get();
        if (node && node->GetType() == NodeType::Output && node->GetChannel() == channel) {
            // Return the input texture ID if connected, otherwise 0
            return node->GetInputTextureID();
        }
    }
    return 0;
}

void NodeSystem::UpdatePBRTextures() {
    // This method can be called to trigger PBR texture updates
    // The actual texture updating will be handled by the PBR view
}

BaseNode* NodeSystem::GetNodeByOutputTexture(unsigned int textureID) {
    // Find the node that has this texture ID as its output
    for (const auto& pair : s_nodes) {
        BaseNode* node = pair.second.get();
        if (node && node->GetTextureID() == textureID) {
            return node;
        }
    }
    return nullptr;
}

void NodeSystem::CenterAndAlignNodes() {
    if (s_nodes.empty()) return;
    
    // Get all node positions from the node editor
    std::vector<ImVec2> positions;
    std::vector<int> nodeIds;
    
    for (const auto& pair : s_nodes) {
        BaseNode* node = pair.second.get();
        if (node) {
            // Get current position from node editor
            ImVec2 pos = ax::NodeEditor::GetNodePosition(node->GetNodeId());
            positions.push_back(pos);
            nodeIds.push_back(node->GetNodeId());
        }
    }
    
    if (positions.empty()) return;
    
    // Calculate the center position of all nodes
    float minX = FLT_MAX, maxX = -FLT_MAX;
    float minY = FLT_MAX, maxY = -FLT_MAX;
    
    for (const auto& pos : positions) {
        minX = std::min(minX, pos.x);
        maxX = std::max(maxX, pos.x);
        minY = std::min(minY, pos.y);
        maxY = std::max(maxY, pos.y);
    }
    
    // Calculate center point
    float centerX = (minX + maxX) / 2.0f;
    float centerY = (minY + maxY) / 2.0f;
    
    // Target center position (center of the editor)
    float targetCenterX = 400.0f; // Adjust this based on your editor size
    float targetCenterY = 300.0f; // Adjust this based on your editor size
    
    // Calculate offset to move all nodes to center
    float offsetX = targetCenterX - centerX;
    float offsetY = targetCenterY - centerY;
    
    // Apply offset to all nodes using node editor API
    for (size_t i = 0; i < nodeIds.size(); i++) {
        ImVec2 currentPos = positions[i];
        ImVec2 newPos = ImVec2(currentPos.x + offsetX, currentPos.y + offsetY);
        ax::NodeEditor::SetNodePosition(nodeIds[i], newPos);
    }
    
    std::cout << "Centered and aligned all nodes" << std::endl;
}

void NodeSystem::FocusOnNode(int nodeId) {
    // Get the node position
    ImVec2 nodePos = ax::NodeEditor::GetNodePosition(nodeId);
    
    // Select the node first
    ax::NodeEditor::SelectNode(nodeId);
    
    // Navigate to the selection (which will be our node)
    ax::NodeEditor::NavigateToSelection();
    
    std::cout << "Focused on node " << nodeId << " at position (" << nodePos.x << ", " << nodePos.y << ")" << std::endl;
}

void NodeSystem::RecordInitialOutputNodePositions() {
    // Record the current positions of output nodes (IDs 1-5) only if they are valid
    for (int i = 1; i <= 5; i++) {
        if (s_nodes.find(i) != s_nodes.end()) {
            ImVec2 pos = ax::NodeEditor::GetNodePosition(i);
            // Only record if position is valid (not FLT_MAX)
            if (pos.x < FLT_MAX && pos.y < FLT_MAX) {
                s_initialOutputNodePositions[i] = pos;
                std::cout << "Recorded initial position for output node " << i << ": (" << pos.x << ", " << pos.y << ")" << std::endl;
            } else {
                std::cout << "Skipping invalid position for output node " << i << ": (" << pos.x << ", " << pos.y << ")" << std::endl;
            }
        }
    }
}

void NodeSystem::ResetOutputNodesToInitialPositions() {
    // Reset only the 5 output nodes (IDs 1-5) to their initial positions
    for (int i = 1; i <= 5; i++) {
        if (s_initialOutputNodePositions.find(i) != s_initialOutputNodePositions.end()) {
            ImVec2 initialPos = s_initialOutputNodePositions[i];
            ax::NodeEditor::SetNodePosition(i, initialPos);
            std::cout << "Reset output node " << i << " to initial position: (" << initialPos.x << ", " << initialPos.y << ")" << std::endl;
        }
    }
}

ImVec2 NodeSystem::GetInitialOutputNodePosition(int nodeId) {
    if (s_initialOutputNodePositions.find(nodeId) != s_initialOutputNodePositions.end()) {
        return s_initialOutputNodePositions[nodeId];
    }
    return ImVec2(100, 100); // Default position if not found
}

void NodeSystem::UpdateAllNodes() {
    // Only update nodes that have pending changes
    // This is more efficient than updating all nodes every frame
    static int updateCounter = 0;
    updateCounter++;
    
    // Update every 10th frame to reduce performance impact
    if (updateCounter % 10 == 0) {
        for (const auto& pair : s_nodes) {
            if (pair.second) {
                pair.second->Update();
            }
        }
    }
    
    // Rebuild the connection chain to ensure all nodes are properly connected
    RebuildConnectionChain();
}

// Connection management functions
bool NodeSystem::CanCreateConnection(BaseNode* inputNode, BaseNode* outputNode) {
    if (!inputNode || !outputNode) {
        return false;
    }
    
    // Cannot connect a node to itself
    if (inputNode == outputNode) {
        return false;
    }
    
    // Check if the input node is an output node (output nodes can only have inputs)
    if (inputNode->GetType() == NodeType::Output) {
        return true; // Output nodes can accept any input
    }
    
    // Check if the output node is an output node (output nodes don't have outputs)
    if (outputNode->GetType() == NodeType::Output) {
        return false; // Output nodes don't have outputs
    }
    
    // Check channel type compatibility
    ChannelType inputChannel = inputNode->GetChannel();
    ChannelType outputChannel = outputNode->GetChannel();
    
    // Allow connections between compatible channel types
    if (inputChannel == outputChannel) {
        return true;
    }
    
    // Allow color outputs to connect to any input (for now)
    if (outputChannel == ChannelType::Color) {
        return true;
    }
    
    // Allow greyscale outputs to connect to greyscale inputs
    if (outputChannel == ChannelType::Roughness || outputChannel == ChannelType::Metallic || 
        outputChannel == ChannelType::Height) {
        if (inputChannel == ChannelType::Roughness || inputChannel == ChannelType::Metallic || 
            inputChannel == ChannelType::Height) {
            return true;
        }
    }
    
    return false;
}

void NodeSystem::DisconnectInputPin(ax::NodeEditor::PinId inputPinId) {
    // Find and remove any existing links connected to this input pin
    auto it = std::remove_if(s_links.begin(), s_links.end(),
        [inputPinId](const NodeLink& link) {
            return link.inputId == inputPinId;
        });
    
    // If we found links to remove, clear the input node's texture data
    if (it != s_links.end()) {
        BaseNode* inputNode = GetNodeByInputPin(inputPinId);
        if (inputNode) {
            inputNode->SetInputTextureData(nullptr);
            inputNode->SetInputTextureID(0);
        }
        
        // Remove the links from the vector
        s_links.erase(it, s_links.end());
    }
}

void NodeSystem::UpdateNodeConnection(BaseNode* inputNode, BaseNode* outputNode) {
    if (!inputNode || !outputNode) {
        return;
    }
    
    // Update the input node with the output node's texture data
    if (outputNode->GetTextureID() != 0) {
        inputNode->SetInputTextureData(outputNode->GetTextureData());
        inputNode->SetInputTextureID(outputNode->GetTextureID());
    }
}

void NodeSystem::PropagateNodeUpdates(BaseNode* startNode) {
    if (!startNode) {
        return;
    }
    
    // Update the starting node
    startNode->Update();
    
    // Find all nodes that are connected to this node's output
    std::vector<BaseNode*> connectedNodes;
    for (const auto& link : s_links) {
        BaseNode* outputNode = GetNodeByOutputPin(link.outputId);
        if (outputNode == startNode) {
            BaseNode* inputNode = GetNodeByInputPin(link.inputId);
            if (inputNode) {
                connectedNodes.push_back(inputNode);
            }
        }
    }
    
    // Recursively update all connected nodes
    for (BaseNode* connectedNode : connectedNodes) {
        PropagateNodeUpdates(connectedNode);
    }
}

void NodeSystem::InvalidateConnectionChain() {
    // Clear all input texture data from all nodes
    for (const auto& pair : s_nodes) {
        if (pair.second) {
            pair.second->SetInputTextureData(nullptr);
            pair.second->SetInputTextureID(0);
        }
    }
}

void NodeSystem::RebuildConnectionChain() {
    // Clear all input texture data first
    InvalidateConnectionChain();
    
    // Rebuild connections based on current links
    for (const auto& link : s_links) {
        BaseNode* inputNode = GetNodeByInputPin(link.inputId);
        BaseNode* outputNode = GetNodeByOutputPin(link.outputId);
        
        if (inputNode && outputNode) {
            UpdateNodeConnection(inputNode, outputNode);
        }
    }
    
    // Update all nodes to reflect the new connections
    for (const auto& pair : s_nodes) {
        if (pair.second) {
            pair.second->Update();
        }
    }
}
