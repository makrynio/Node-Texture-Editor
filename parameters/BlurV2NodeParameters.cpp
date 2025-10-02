#include "BlurV2NodeParameters.h"
#include "../nodes/BlurV2Node.h"
#include <iostream>

BlurV2NodeParameters::BlurV2NodeParameters()
    : m_blurNode(nullptr), m_intensity(1.0f), m_samples(5), m_textureWidth(512), m_textureHeight(512) {
}

void BlurV2NodeParameters::SetNode(BaseNode* node) {
    std::cout << "BlurV2NodeParameters::SetNode() called with node: " << (node ? node->GetName() : "null") << std::endl;
    if (node && node->GetName() == "Blur") {
        std::cout << "  Setting Blur node ID: " << node->GetNodeId() << std::endl;
        m_blurNode = static_cast<BlurV2Node*>(node);
        // Update parameters when setting the node
        if (m_blurNode) {
            UpdateParameters();
            std::cout << "  Initialized parameters - intensity: " << m_intensity << ", samples: " << m_samples << std::endl;
        }
    } else {
        std::cout << "  Not a Blur node, clearing" << std::endl;
        m_blurNode = nullptr;
    }
}

void BlurV2NodeParameters::Render() {
    std::cout << "BlurV2NodeParameters::Render() called" << std::endl;
    if (!m_blurNode) {
        std::cout << "  No Blur node selected" << std::endl;
        ImGui::Text("No Blur node selected");
        return;
    }

    std::cout << "  Rendering parameters for Blur node ID: " << m_blurNode->GetNodeId() << std::endl;
    ImGui::Text("Blur Node Parameters");
    ImGui::Separator();

    // Intensity parameter (0-10, step 0.1)
    ImGui::Text("Intensity:");
    std::string intensityId = "##Intensity_" + std::to_string(m_blurNode ? m_blurNode->GetNodeId() : 0);
    
    if (ImGui::SliderFloat(intensityId.c_str(), &m_intensity, 0.0f, 10.0f, "%.1f")) {
        std::cout << "*** INTENSITY SLIDER CHANGED TO: " << m_intensity << " ***" << std::endl;
        ApplyChanges();
    }

    // Samples parameter (1-10, step 1)
    ImGui::Text("Samples:");
    std::string samplesId = "##Samples_" + std::to_string(m_blurNode ? m_blurNode->GetNodeId() : 0);
    
    if (ImGui::SliderInt(samplesId.c_str(), &m_samples, 1, 10, "%d")) {
        std::cout << "*** SAMPLES SLIDER CHANGED TO: " << m_samples << " ***" << std::endl;
        ApplyChanges();
    }

    ImGui::Separator();
    ImGui::Text("Texture Info:");
    if (m_blurNode->GetTextureData()) {
        ImGui::Text("Size: %dx%d", m_textureWidth, m_textureHeight);
    } else {
        ImGui::Text("No texture generated");
    }
}

void BlurV2NodeParameters::UpdateParameters() {
    if (!m_blurNode) {
        return;
    }
    
    // Sync parameter values from the node
    m_intensity = m_blurNode->GetIntensity();
    m_samples = m_blurNode->GetSamples();
    
    // Update texture info if available
    if (m_blurNode->GetTextureData()) {
        m_textureWidth = m_blurNode->GetTextureData()->width;
        m_textureHeight = m_blurNode->GetTextureData()->height;
    }
}

void BlurV2NodeParameters::ApplyChanges() {
    if (!m_blurNode) {
        std::cout << "BlurV2NodeParameters::ApplyChanges() - No blur node!" << std::endl;
        return;
    }
    
    std::cout << "*** APPLYING CHANGES - Setting intensity: " << m_intensity << ", samples: " << m_samples << " ***" << std::endl;
    m_blurNode->SetIntensity(m_intensity);
    m_blurNode->SetSamples(m_samples);
    
    // Force immediate texture regeneration
    std::cout << "*** FORCING IMMEDIATE TEXTURE REGENERATION ***" << std::endl;
    m_blurNode->GenerateTexture();
    
    std::cout << "*** CHANGES APPLIED - Node intensity: " << m_blurNode->GetIntensity() << ", samples: " << m_blurNode->GetSamples() << " ***" << std::endl;
}

void BlurV2NodeParameters::Clear() {
    m_blurNode = nullptr;
    m_intensity = 1.0f;
    m_samples = 10;
    m_textureWidth = 512;
    m_textureHeight = 512;
}

