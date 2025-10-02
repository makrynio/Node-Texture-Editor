#include "ImageNodeParameters.h"
#include "../nodes/ImageNode.h"
#include <iostream>
#include <filesystem>
#include <string>
#include <cstring>

// Native macOS file dialog implementation
#ifdef __APPLE__
#include "FileDialog.h"
#endif

ImageNodeParameters::ImageNodeParameters() 
    : m_node(nullptr), m_width(0), m_height(0), m_channels(0) {
    CreateParameters();
}

void ImageNodeParameters::SetNode(BaseNode* node) {
    m_node = node;
    UpdateParameters();
}

void ImageNodeParameters::Render() {
    if (!m_node) {
        ImGui::Text("No node selected");
        return;
    }
    
    ImGui::Text("Image Node Parameters");
    ImGui::Separator();
    
    // Display image information
    if (!m_imagePath.empty()) {
        ImGui::Text("Image: %s", m_imageName.c_str());
        ImGui::Text("Path: %s", m_imagePath.c_str());
        ImGui::Text("Size: %dx%d", m_width, m_height);
        ImGui::Text("Channels: %d", m_channels);
    } else {
        ImGui::Text("No image loaded");
    }
    
    ImGui::Separator();
    ImGui::Text("Actions:");
    
    // Load Image button
    OpenFileDialog();
    
    ImGui::SameLine();
    
    // Clear Image button
    if (ImGui::Button("Clear Image")) {
        if (m_node && m_node->GetName() == "Image") {
            ImageNode* imageNode = static_cast<ImageNode*>(m_node);
            imageNode->ClearImage();
            UpdateParameters();
        }
    }
    
    ImGui::Separator();
    ImGui::Text("Properties:");
    
    for (auto& param : m_parameters) {
        param->Render();
    }
}

void ImageNodeParameters::Clear() {
    m_node = nullptr;
}

void ImageNodeParameters::CreateParameters() {
    // For image nodes, we mainly display information and provide file loading
    // In a full implementation, we might add parameters for:
    // - Image scaling
    // - Color space conversion
    // - Channel selection
    // - Filtering options
}

void ImageNodeParameters::UpdateParameters() {
    // Update parameter values based on the current image node
    if (m_node && m_node->GetName() == "Image") {
        ImageNode* imageNode = static_cast<ImageNode*>(m_node);
        m_imagePath = imageNode->GetImagePath();
        
        if (!m_imagePath.empty()) {
            m_imageName = std::filesystem::path(m_imagePath).filename().string();
            
            if (imageNode->GetTextureData()) {
                m_width = imageNode->GetTextureData()->width;
                m_height = imageNode->GetTextureData()->height;
                m_channels = imageNode->GetTextureData()->channels;
            }
        } else {
            m_imageName.clear();
            m_width = m_height = m_channels = 0;
        }
    }
}

void ImageNodeParameters::OpenFileDialog() {
#ifdef __APPLE__
    // Native macOS file dialog
    if (ImGui::Button("Load Image")) {
        const char* result = showOpenFileDialog();
        if (result) {
            LoadImage(std::string(result));
            freeFileDialogResult(result);
        }
    }
#else
    // Fallback for non-macOS systems
    static char filePath[512] = "";
    static bool showFileInput = false;
    
    if (ImGui::Button("Load Image")) {
        showFileInput = true;
        strcpy(filePath, "");
    }
    
    if (showFileInput) {
        ImGui::OpenPopup("Load Image File");
    }
    
    if (ImGui::BeginPopupModal("Load Image File", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter image file path:");
        ImGui::InputText("##filepath", filePath, sizeof(filePath));
        
        ImGui::Text("Supported formats: PNG, JPG, JPEG, BMP, TGA, HDR, PSD, GIF, PIC, PPM, PGM");
        
        if (ImGui::Button("Load")) {
            if (strlen(filePath) > 0) {
                LoadImage(std::string(filePath));
                showFileInput = false;
                ImGui::CloseCurrentPopup();
            }
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            showFileInput = false;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
#endif
}

void ImageNodeParameters::LoadImage(const std::string& filePath) {
    if (m_node && m_node->GetName() == "Image") {
        ImageNode* imageNode = static_cast<ImageNode*>(m_node);
        if (imageNode->LoadImage(filePath)) {
            UpdateParameters();
        }
    }
}
