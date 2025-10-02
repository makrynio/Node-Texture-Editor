#include "ImageNode.h"
#include "../TextureGenerator.h"
#include <stb_image.h>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <glad/glad.h>

ImageNode::ImageNode(int nodeId, const std::string& name, ChannelType channel, ColorMode colorMode)
    : BaseNode(nodeId, NodeType::Texture, name, channel, colorMode) {
    GenerateDefaultTexture();
}

void ImageNode::Render(ax::NodeEditor::EditorContext* editor) {
    ax::NodeEditor::BeginNode(m_nodeId);

    // Node content
    ImGui::BeginGroup();
    ImGui::Text("%s", m_name.c_str());
    
    ImGui::Text("Type: Image");
    ImGui::Text("Mode: %s", m_colorMode == ColorMode::RGB ? "RGB" : "Greyscale");
    
    if (HasImage()) {
        ImGui::Text("Size: %dx%d", m_textureData->width, m_textureData->height);
        ImGui::Text("Channels: %d", m_textureData->channels);
        
        // Show filename
        std::string filename = m_imagePath.substr(m_imagePath.find_last_of("/\\") + 1);
        ImGui::Text("File: %s", filename.c_str());
    } else {
        ImGui::Text("No image loaded");
    }

    // Calculate thumbnail size to fit within node
    ImVec2 nodeSize = ImGui::GetContentRegionAvail();
    float thumbnailSize = std::min(nodeSize.x - 20.0f, 100.0f); // Leave some padding

    // Display texture thumbnail
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

void ImageNode::Update() {
    // Update logic if needed
}

bool ImageNode::LoadImage(const std::string& filePath) {
    int width, height, channels;
    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);
    
    if (!data) {
        std::cerr << "Failed to load image: " << filePath << std::endl;
        return false;
    }
    
    // Clean up previous texture
    if (m_textureID != 0) {
        glDeleteTextures(1, &m_textureID);
    }
    
    // Create new texture data
    m_textureData = std::make_shared<TextureData>(width, height, channels);
    
    // Copy image data
    size_t dataSize = width * height * channels;
    std::memcpy(m_textureData->data.data(), data, dataSize);
    
    // Free stb_image data
    stbi_image_free(data);
    
    // Create OpenGL texture
    m_textureID = TextureGenerator::CreateOpenGLTexture(*m_textureData);
    
    // Store the file path
    m_imagePath = filePath;
    
    std::cout << "Successfully loaded image: " << filePath 
              << " (" << width << "x" << height << ", " << channels << " channels)" << std::endl;
    
    return true;
}

void ImageNode::ClearImage() {
    if (m_textureID != 0) {
        glDeleteTextures(1, &m_textureID);
        m_textureID = 0;
    }
    m_textureData.reset();
    m_imagePath.clear();
    GenerateDefaultTexture();
}

void ImageNode::GenerateDefaultTexture() {
    // Generate a default "no image" texture
    m_textureData = TextureGenerator::GenerateCheckerboard(1024, 1024, 64);
    m_textureID = TextureGenerator::CreateOpenGLTexture(*m_textureData);
}
