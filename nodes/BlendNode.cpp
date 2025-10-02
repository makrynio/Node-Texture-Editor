#include "BlendNode.h"
#include <imgui_internal.h>
#include <algorithm>
#include <iostream>

BlendNode::BlendNode(int nodeId)
    : BaseNode(nodeId, NodeType::Utility, "Blend", ChannelType::Color, ColorMode::RGB),
      m_blendMode(BlendMode::Multiply), m_opacity(1.0f), m_inputPin2Id(0),
      m_inputTexture2Data(nullptr), m_inputTexture2ID(0) {
    
    // Set up pins for blend node
    m_inputPinId = nodeId * 100 + 1;   // Input pin 1 (texture A)
    m_inputPin2Id = nodeId * 100 + 2;  // Input pin 2 (texture B)
    m_outputPinId = nodeId * 100 + 3;  // Output pin
    
    // Generate initial texture
    ProcessBlend();
}

void BlendNode::Render(ax::NodeEditor::EditorContext* editor) {
    ax::NodeEditor::BeginNode(m_nodeId);

    // Input pins on the left
    ax::NodeEditor::BeginPin(m_inputPinId, PinKind::Input);
    ImGui::Text("Texture A");
    ax::NodeEditor::EndPin();
    
    ax::NodeEditor::BeginPin(m_inputPin2Id, PinKind::Input);
    ImGui::Text("Texture B");
    ax::NodeEditor::EndPin();
    
    ImGui::SameLine();
    
    // Node content
    ImGui::BeginGroup();
    ImGui::Text("Blend");
    
    
    // Display current settings (read-only in node)
    ImGui::Text("Mode: %s", GetBlendModeName(m_blendMode).c_str());
    ImGui::Text("Opacity: %.2f", m_opacity);
    
    // Calculate thumbnail size to fit within node
    ImVec2 nodeSize = ImGui::GetContentRegionAvail();
    float thumbnailSize = std::min(nodeSize.x - 20.0f, 100.0f);
    
    // Display result texture thumbnail
    if (m_textureID != 0) {
        ImGui::Image((void*)(intptr_t)m_textureID,
                    ImVec2(thumbnailSize, thumbnailSize),
                    ImVec2(0, 0), ImVec2(1, 1),
                    ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 0.5f));
    }
    ImGui::EndGroup();

    ImGui::SameLine();

    // Output pin on the right
    ax::NodeEditor::BeginPin(m_outputPinId, PinKind::Output);
    ImGui::Text("Output ->");
    ax::NodeEditor::EndPin();

    ax::NodeEditor::EndNode();
}

void BlendNode::Update() {
    ProcessBlend();
}

void BlendNode::ProcessBlend() {
    // Get input textures
    auto textureA = m_inputTextureData;
    auto textureB = m_inputTexture2Data;
    
    // Determine output size based on input textures
    int width = 512;
    int height = 512;
    
    if (textureA) {
        width = textureA->width;
        height = textureA->height;
    } else if (textureB) {
        width = textureB->width;
        height = textureB->height;
    }
    
    
    // Generate default textures if no inputs
    if (!textureA) {
        textureA = std::make_shared<TextureData>(width, height, 3);
        // Generate a simple gradient for texture A
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float u = (float)x / (width - 1.0f);
                float v = (float)y / (height - 1.0f);
                int index = (y * width + x) * 3;
                textureA->data[index + 0] = (unsigned char)(u * 255);
                textureA->data[index + 1] = (unsigned char)(v * 255);
                textureA->data[index + 2] = (unsigned char)((u + v) * 0.5f * 255);
            }
        }
    }
    
    if (!textureB) {
        textureB = std::make_shared<TextureData>(width, height, 3);
        // Generate checkerboard for texture B
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                bool checker = ((x / 32) + (y / 32)) % 2 == 0;
                unsigned char val = checker ? 255 : 128;
                int index = (y * width + x) * 3;
                textureB->data[index + 0] = val;
                textureB->data[index + 1] = val;
                textureB->data[index + 2] = val;
            }
        }
    }
    
    // Create result texture
    auto result = std::make_shared<TextureData>(width, height, 3);
    
    // Apply blend mode
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = (y * width + x) * 3;
            
            // Get pixel values (normalized to 0-1)
            float a_r = textureA->data[index + 0] / 255.0f;
            float a_g = textureA->data[index + 1] / 255.0f;
            float a_b = textureA->data[index + 2] / 255.0f;
            
            float b_r = textureB->data[index + 0] / 255.0f;
            float b_g = textureB->data[index + 1] / 255.0f;
            float b_b = textureB->data[index + 2] / 255.0f;
            
            float r, g, b;
            
            // Apply blend mode
            switch (m_blendMode) {
                case BlendMode::Add:
                    r = std::min(1.0f, a_r + b_r);
                    g = std::min(1.0f, a_g + b_g);
                    b = std::min(1.0f, a_b + b_b);
                    break;
                case BlendMode::Multiply:
                    r = a_r * b_r;
                    g = a_g * b_g;
                    b = a_b * b_b;
                    break;
                case BlendMode::Subtract:
                    r = std::max(0.0f, a_r - b_r);
                    g = std::max(0.0f, a_g - b_g);
                    b = std::max(0.0f, a_b - b_b);
                    break;
                case BlendMode::Divide:
                    r = (b_r > 0.1f) ? std::min(1.0f, a_r / b_r) : 0.0f;
                    g = (b_g > 0.1f) ? std::min(1.0f, a_g / b_g) : 0.0f;
                    b = (b_b > 0.1f) ? std::min(1.0f, a_b / b_b) : 0.0f;
                    break;
                case BlendMode::Screen:
                    r = 1.0f - (1.0f - a_r) * (1.0f - b_r);
                    g = 1.0f - (1.0f - a_g) * (1.0f - b_g);
                    b = 1.0f - (1.0f - a_b) * (1.0f - b_b);
                    break;
                case BlendMode::Overlay:
                    r = (b_r < 0.5f) ? 2.0f * a_r * b_r : 1.0f - 2.0f * (1.0f - a_r) * (1.0f - b_r);
                    g = (b_g < 0.5f) ? 2.0f * a_g * b_g : 1.0f - 2.0f * (1.0f - a_g) * (1.0f - b_g);
                    b = (b_b < 0.5f) ? 2.0f * a_b * b_b : 1.0f - 2.0f * (1.0f - a_b) * (1.0f - b_b);
                    break;
                case BlendMode::Darken:
                    r = std::min(a_r, b_r);
                    g = std::min(a_g, b_g);
                    b = std::min(a_b, b_b);
                    break;
                case BlendMode::Lighten:
                    r = std::max(a_r, b_r);
                    g = std::max(a_g, b_g);
                    b = std::max(a_b, b_b);
                    break;
                case BlendMode::Difference:
                    r = std::abs(a_r - b_r);
                    g = std::abs(a_g - b_g);
                    b = std::abs(a_b - b_b);
                    break;
                default:
                    r = a_r * b_r;
                    g = a_g * b_g;
                    b = a_b * b_b;
                    break;
            }
            
            // Apply opacity
            r = r * m_opacity + b_r * (1.0f - m_opacity);
            g = g * m_opacity + b_g * (1.0f - m_opacity);
            b = b * m_opacity + b_b * (1.0f - m_opacity);
            
            // Clamp values
            r = std::max(0.0f, std::min(1.0f, r));
            g = std::max(0.0f, std::min(1.0f, g));
            b = std::max(0.0f, std::min(1.0f, b));
            
            result->data[index + 0] = (unsigned char)(r * 255);
            result->data[index + 1] = (unsigned char)(g * 255);
            result->data[index + 2] = (unsigned char)(b * 255);
        }
    }
    
    m_textureData = result;
    m_textureID = TextureGenerator::CreateOpenGLTexture(*m_textureData);
}

std::string BlendNode::GetBlendModeName(BlendMode mode) const {
    switch (mode) {
        case BlendMode::Add: return "Add";
        case BlendMode::Multiply: return "Multiply";
        case BlendMode::Subtract: return "Subtract";
        case BlendMode::Divide: return "Divide";
        case BlendMode::Screen: return "Screen";
        case BlendMode::Overlay: return "Overlay";
        case BlendMode::Darken: return "Darken";
        case BlendMode::Lighten: return "Lighten";
        case BlendMode::Difference: return "Difference";
        default: return "Unknown";
    }
}
