#include "BaseNode.h"
#include <iostream>

BaseNode::BaseNode(int nodeId, NodeType type, const std::string& name, 
                   ChannelType channel, ColorMode colorMode)
    : m_nodeId(nodeId), m_type(type), m_name(name), m_channel(channel), 
      m_colorMode(colorMode), m_textureID(0), m_inputTextureID(0),
      m_inputPinId(0), m_outputPinId(0) {
    
    // Set pin IDs with different ranges to avoid conflicts
    if (type == NodeType::Output) {
        m_inputPinId = nodeId * 100 + 1; // Input pin
    } else if (type == NodeType::Texture) {
        m_outputPinId = nodeId * 100 + 1; // Output pin
    } else if (type == NodeType::Utility) {
        m_outputPinId = nodeId * 100 + 1; // Output pin for utility nodes
    }
}
