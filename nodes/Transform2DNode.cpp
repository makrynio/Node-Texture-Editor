#include "Transform2DNode.h"
#include "../TextureGenerator.h"
#include <imgui_internal.h>
#include <algorithm>
#include <iostream>
#include <cmath>

using namespace ax::NodeEditor;

Transform2DNode::Transform2DNode(int nodeId)
    : BaseNode(nodeId, NodeType::Utility, "Transform2D", ChannelType::Color, ColorMode::RGB),
      m_translateX(0.0f), m_translateY(0.0f), m_scaleX(1.0f), m_scaleY(1.0f), 
      m_rotation(0.0f), m_clamp(true) {
    
    // Set up pins for transform node
    m_inputPinId = nodeId * 100 + 1;   // Input pin
    m_outputPinId = nodeId * 100 + 2;  // Output pin
    
    // Generate initial texture
    ProcessTransform();
}

void Transform2DNode::Render(ax::NodeEditor::EditorContext* editor) {
    ax::NodeEditor::BeginNode(m_nodeId);

    // Input pin on the left
    ax::NodeEditor::BeginPin(m_inputPinId, PinKind::Input);
    ImGui::Text("Image");
    ax::NodeEditor::EndPin();
    
    ImGui::SameLine();
    
    // Node content
    ImGui::BeginGroup();
    ImGui::Text("Transform2D");
    
    // Display current settings (read-only in node)
    ImGui::Text("Translate: (%.2f, %.2f)", m_translateX, m_translateY);
    ImGui::Text("Scale: (%.2f, %.2f)", m_scaleX, m_scaleY);
    ImGui::Text("Rotation: %.1f°", m_rotation);
    ImGui::Text("Clamp: %s", m_clamp ? "Yes" : "No");
    
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

void Transform2DNode::Update() {
    ProcessTransform();
}

void Transform2DNode::ProcessTransform() {
    // Get input texture
    auto inputTexture = m_inputTextureData;
    
    // Determine output size based on input texture
    int width = 512;
    int height = 512;
    
    if (inputTexture) {
        width = inputTexture->width;
        height = inputTexture->height;
    }
    
    // Generate default texture if no input
    if (!inputTexture) {
        inputTexture = std::make_shared<TextureData>(width, height, 3);
        // Generate a simple checkerboard pattern
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                bool checker = ((x / 32) + (y / 32)) % 2 == 0;
                unsigned char val = checker ? 255 : 128;
                int index = (y * width + x) * 3;
                inputTexture->data[index + 0] = val;
                inputTexture->data[index + 1] = val;
                inputTexture->data[index + 2] = val;
            }
        }
    }
    
    // Create result texture
    auto result = std::make_shared<TextureData>(width, height, 3);
    
    // Convert rotation from degrees to radians
    float rotationRad = m_rotation * M_PI / 180.0f;
    
    // Pre-calculate transformation matrix components
    float cosR = cos(rotationRad);
    float sinR = sin(rotationRad);
    
    // Apply transform to each pixel
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Convert to normalized coordinates (-0.5 to 0.5)
            float u = (float)x / (width - 1.0f) - 0.5f;
            float v = (float)y / (height - 1.0f) - 0.5f;
            
            // Apply inverse transformation
            // 1. Translate by (0.5, 0.5) to center
            // 2. Apply inverse translation
            // 3. Apply inverse rotation
            // 4. Apply inverse scale
            // 5. Translate back by (-0.5, -0.5)
            
            // Step 1: Translate to center
            float u1 = u + 0.5f;
            float v1 = v + 0.5f;
            
            // Step 2: Apply inverse translation
            float u2 = u1 - m_translateX;
            float v2 = v1 - m_translateY;
            
            // Step 3: Apply inverse rotation
            float u3 = u2 * cosR + v2 * sinR;
            float v3 = -u2 * sinR + v2 * cosR;
            
            // Step 4: Apply inverse scale
            float u4 = u3 / m_scaleX;
            float v4 = v3 / m_scaleY;
            
            // Step 5: Translate back from center
            float u5 = u4 - 0.5f;
            float v5 = v4 - 0.5f;
            
            // Convert back to texture coordinates (0 to 1)
            float sourceU = u5 + 0.5f;
            float sourceV = v5 + 0.5f;
            
            // Apply clamping if enabled
            if (m_clamp) {
                sourceU = std::max(0.0f, std::min(1.0f, sourceU));
                sourceV = std::max(0.0f, std::min(1.0f, sourceV));
            }
            
            // Sample the input texture
            int sourceX = (int)(sourceU * (width - 1));
            int sourceY = (int)(sourceV * (height - 1));
            
            // Clamp source coordinates to valid range
            sourceX = std::max(0, std::min(width - 1, sourceX));
            sourceY = std::max(0, std::min(height - 1, sourceY));
            
            // Get pixel from source texture
            int sourceIndex = (sourceY * width + sourceX) * 3;
            int resultIndex = (y * width + x) * 3;
            
            result->data[resultIndex + 0] = inputTexture->data[sourceIndex + 0];
            result->data[resultIndex + 1] = inputTexture->data[sourceIndex + 1];
            result->data[resultIndex + 2] = inputTexture->data[sourceIndex + 2];
        }
    }
    
    m_textureData = result;
    m_textureID = TextureGenerator::CreateOpenGLTexture(*m_textureData);
}
