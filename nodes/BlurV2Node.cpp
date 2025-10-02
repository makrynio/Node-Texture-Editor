#include "BlurV2Node.h"
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_node_editor.h>
#include "../TextureGenerator.h"
#include <iostream>
#include <vector>
#include <algorithm> // For std::min
#include <cmath> // For exp, pow

using namespace ax::NodeEditor;

BlurV2Node::BlurV2Node(int nodeId)
    : BaseNode(nodeId, NodeType::Utility, "Blur", ChannelType::Color, ColorMode::RGB),
      m_intensity(1.0f), m_samples(5), m_needsUpdate(true), m_lastInputTextureID(0) {
    
    // Set up pins for blur node
    m_inputPinId = nodeId * 100 + 1;   // Input pin
    m_outputPinId = nodeId * 100 + 2;  // Output pin
    
    // Defer texture generation to first render
}

void BlurV2Node::Render(ax::NodeEditor::EditorContext* editor) {
    // Generate texture on first render if needed
    if (m_textureID == 0) {
        GenerateTexture();
    }
    
    ax::NodeEditor::BeginNode(m_nodeId);
    
    // Input pin on the left
    ax::NodeEditor::BeginPin(m_inputPinId, PinKind::Input);
    ImGui::Text("Image");
    ax::NodeEditor::EndPin();
    
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::Text("Blur");
    ImGui::Text("Intensity: %.2f", m_intensity);
    ImGui::Text("Samples: %d", m_samples);
    
    // Show texture thumbnail
    ImVec2 nodeSize = ImGui::GetContentRegionAvail();
    float thumbnailSize = std::min(nodeSize.x - 20.0f, 100.0f);
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

void BlurV2Node::Update() {
    // Check if input texture has changed
    unsigned int currentInputTextureID = GetInputTextureID();
    if (currentInputTextureID != m_lastInputTextureID) {
        std::cout << "*** BLUR NODE " << m_nodeId << " - Input texture changed from " << m_lastInputTextureID << " to " << currentInputTextureID << " ***" << std::endl;
        m_lastInputTextureID = currentInputTextureID;
        m_needsUpdate = true;
    }
    
    // Always regenerate texture if parameters changed or input changed
    // This ensures the blur effect is always up to date
    if (m_needsUpdate) {
        std::cout << "*** BLUR NODE " << m_nodeId << " - Regenerating texture (needsUpdate=true) ***" << std::endl;
        GenerateTexture();
        m_needsUpdate = false;
    }
}

void BlurV2Node::GenerateTexture() {
    GenerateBlurTexture();
}

void BlurV2Node::GenerateBlurTexture() {
    // Use input texture size if available, otherwise default to 1024x1024
    int width = 1024;
    int height = 1024;
    
    std::cout << "BlurV2Node::GenerateBlurTexture() - Node ID: " << m_nodeId << std::endl;
    
    if (m_inputTextureData) {
        width = m_inputTextureData->width;
        height = m_inputTextureData->height;
        std::cout << "  Input texture: " << width << "x" << height << ", channels: " << m_inputTextureData->channels << std::endl;
    } else {
        std::cout << "  No input texture data" << std::endl;
    }
    
    std::vector<unsigned char> data(width * height * 4);
    
    if (!m_inputTextureData) {
        // Generate a default texture if no input
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int index = (y * width + x) * 4;
                data[index] = 128;     // R
                data[index + 1] = 128; // G
                data[index + 2] = 128; // B
                data[index + 3] = 255; // A
            }
        }
    } else {
        // Apply Gaussian blur based on the shader code
        if (!m_inputTextureData || m_inputTextureData->data.empty()) {
            // Generate a default texture if input data is invalid
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    int index = (y * width + x) * 4;
                    data[index] = 128;     // R
                    data[index + 1] = 128; // G
                    data[index + 2] = 128; // B
                    data[index + 3] = 255; // A
                }
            }
        } else {
            const auto& inputData = m_inputTextureData->data;
            int inputWidth = m_inputTextureData->width;
            int inputHeight = m_inputTextureData->height;
            int inputChannels = m_inputTextureData->channels;
            
            // Validate input data
            if (inputWidth <= 0 || inputHeight <= 0 || inputChannels <= 0 || inputData.empty()) {
                // Generate a default texture if input data is invalid
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        int index = (y * width + x) * 4;
                        data[index] = 128;     // R
                        data[index + 1] = 128; // G
                        data[index + 2] = 128; // B
                        data[index + 3] = 255; // A
                    }
                }
            } else {
                // Calculate pixel size for blur sampling
                float pixelSizeX = 1.0f / inputWidth;
                float pixelSizeY = 1.0f / inputHeight;
                
                // Calculate sigma based on samples (matching shader logic: float(prop_samples) * 0.25)
                float sigma = static_cast<float>(m_samples) * 0.25f;
                
                std::cout << "  Starting blur processing with sigma: " << sigma << ", samples: " << m_samples << std::endl;
                
                for (int y = 0; y < height; y++) {
                    if (y % 64 == 0) {
                        std::cout << "  Processing row " << y << " of " << height << std::endl;
                    }
                    for (int x = 0; x < width; x++) {
                        int index = (y * width + x) * 4;
                        
                        float r = 0.0f, g = 0.0f, b = 0.0f;
                        float totalWeight = 0.0f;
                        
                        // Apply Gaussian blur sampling
                        int halfSamples = std::max(1, m_samples / 2);
                        for (int dy = -halfSamples; dy < halfSamples; dy++) {
                            for (int dx = -halfSamples; dx < halfSamples; dx++) {
                                // Calculate offset (matching shader: ps * prop_intensity)
                                float offsetX = dx * pixelSizeX * m_intensity;
                                float offsetY = dy * pixelSizeY * m_intensity;
                                
                                // Calculate Gaussian weight
                                float distance = sqrt(dx * dx + dy * dy);
                                float weight = exp(-(distance * distance) / (2.0f * sigma * sigma)) / (2.0f * M_PI * sigma * sigma);
                                
                                // Sample from input texture
                                int sampleX = x + static_cast<int>(offsetX * width);
                                int sampleY = y + static_cast<int>(offsetY * height);
                                
                                // Clamp coordinates
                                sampleX = std::max(0, std::min(sampleX, inputWidth - 1));
                                sampleY = std::max(0, std::min(sampleY, inputHeight - 1));
                                
                                // Sample from input texture
                                int sourceIndex = (sampleY * inputWidth + sampleX) * inputChannels;
                                
                                if (sourceIndex + inputChannels - 1 < inputData.size()) {
                                    float sampleR, sampleG, sampleB;
                                    
                                    if (inputChannels >= 4) {
                                        // RGBA input
                                        sampleR = inputData[sourceIndex] / 255.0f;
                                        sampleG = inputData[sourceIndex + 1] / 255.0f;
                                        sampleB = inputData[sourceIndex + 2] / 255.0f;
                                    } else if (inputChannels == 3) {
                                        // RGB input
                                        sampleR = inputData[sourceIndex] / 255.0f;
                                        sampleG = inputData[sourceIndex + 1] / 255.0f;
                                        sampleB = inputData[sourceIndex + 2] / 255.0f;
                                    } else if (inputChannels == 1) {
                                        // Grayscale input
                                        sampleR = sampleG = sampleB = inputData[sourceIndex] / 255.0f;
                                    } else {
                                        sampleR = sampleG = sampleB = 0.5f;
                                    }
                                    
                                    r += sampleR * weight;
                                    g += sampleG * weight;
                                    b += sampleB * weight;
                                    totalWeight += weight;
                                }
                            }
                        }
                        
                        // Normalize by total weight
                        if (totalWeight > 0.0f) {
                            r /= totalWeight;
                            g /= totalWeight;
                            b /= totalWeight;
                        }
                        
                        // Clamp values and convert back to 0-255 range
                        data[index] = static_cast<unsigned char>(std::max(0.0f, std::min(1.0f, r)) * 255.0f);
                        data[index + 1] = static_cast<unsigned char>(std::max(0.0f, std::min(1.0f, g)) * 255.0f);
                        data[index + 2] = static_cast<unsigned char>(std::max(0.0f, std::min(1.0f, b)) * 255.0f);
                        data[index + 3] = 255; // A
                    }
                }
            }
        }
    }
    
    // Generate or update OpenGL texture
    if (m_textureID == 0) {
        glGenTextures(1, &m_textureID);
    }
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Store texture data
    m_textureData = std::make_shared<TextureData>(width, height, 4);
    m_textureData->data = std::move(data);
}
