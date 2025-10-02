#pragma once

#include "../TextureGenerator.h"
#include <imgui.h>
#include <imgui_node_editor.h>
#include <memory>
#include <string>

using namespace ax::NodeEditor;

enum class NodeType {
    Output,
    Texture,
    Utility
};

enum class ChannelType {
    Color,
    Roughness,
    Metallic,
    Normal,
    Height
};

enum class ColorMode {
    RGB,
    Greyscale
};

class BaseNode {
public:
    BaseNode(int nodeId, NodeType type, const std::string& name, 
             ChannelType channel = ChannelType::Color, 
             ColorMode colorMode = ColorMode::RGB);
    
    virtual ~BaseNode() = default;
    
    // Getters
    int GetNodeId() const { return m_nodeId; }
    NodeType GetType() const { return m_type; }
    const std::string& GetName() const { return m_name; }
    ChannelType GetChannel() const { return m_channel; }
    ColorMode GetColorMode() const { return m_colorMode; }
    unsigned int GetTextureID() const { return m_textureID; }
    unsigned int GetInputTextureID() const { return m_inputTextureID; }
    PinId GetInputPinId() const { return m_inputPinId; }
    PinId GetOutputPinId() const { return m_outputPinId; }
    std::shared_ptr<TextureData> GetTextureData() const { return m_textureData; }
    std::shared_ptr<TextureData> GetInputTextureData() const { return m_inputTextureData; }
    
    // Setters
    void SetInputTextureData(std::shared_ptr<TextureData> data) { m_inputTextureData = data; }
    void SetInputTextureID(unsigned int id) { m_inputTextureID = id; }
    
    // Virtual methods
    virtual void Render(ax::NodeEditor::EditorContext* editor) = 0;
    virtual void Update() {}
    
protected:
    int m_nodeId;
    NodeType m_type;
    std::string m_name;
    ChannelType m_channel;
    ColorMode m_colorMode;
    std::shared_ptr<TextureData> m_textureData;
    unsigned int m_textureID;
    std::shared_ptr<TextureData> m_inputTextureData;
    unsigned int m_inputTextureID;
    PinId m_inputPinId;
    PinId m_outputPinId;
};
