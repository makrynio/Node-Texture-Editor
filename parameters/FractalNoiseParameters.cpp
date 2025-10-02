#include "FractalNoiseParameters.h"
#include "../nodes/FractalNoiseNode.h"
#include <iostream>

FractalNoiseParameters::FractalNoiseParameters() 
    : m_fractalNode(nullptr), m_node(nullptr),
      m_scale(1), m_scaleX(1), m_scaleY(1), m_startBand(4), m_endBand(8),
      m_persistence(0.6f), m_persistenceStart(1.0f), m_brightness(0.5f) {
    CreateParameters();
}

void FractalNoiseParameters::CreateParameters() {
    // Create parameter objects with references to member variables
    m_scaleParam = std::make_unique<IntParameter>("scale", m_scale, 1, 10, "Scale");
    m_scaleXParam = std::make_unique<IntParameter>("scaleX", m_scaleX, 1, 15, "X Scale");
    m_scaleYParam = std::make_unique<IntParameter>("scaleY", m_scaleY, 1, 15, "Y Scale");
    m_startBandParam = std::make_unique<IntParameter>("startBand", m_startBand, 1, 10, "Start Band");
    m_endBandParam = std::make_unique<IntParameter>("endBand", m_endBand, 1, 10, "End Band");
    m_persistenceParam = std::make_unique<FloatParameter>("persistence", m_persistence, 0.0f, 1.0f, "%.2f", "Persistence");
    m_persistenceStartParam = std::make_unique<FloatParameter>("persistenceStart", m_persistenceStart, 0.0f, 1.0f, "%.2f", "Persistence Start");
    m_brightnessParam = std::make_unique<FloatParameter>("brightness", m_brightness, 0.0f, 1.0f, "%.2f", "Brightness");
}

void FractalNoiseParameters::SetNode(BaseNode* node) {
    m_node = node;
    m_fractalNode = dynamic_cast<FractalNoiseNode*>(node);
    
    if (m_fractalNode) {
        UpdateParameters();
    }
}

void FractalNoiseParameters::UpdateParameters() {
    if (!m_fractalNode) return;
    
    // Update parameter values from node
    m_scale = m_fractalNode->GetScale();
    m_scaleX = m_fractalNode->GetScaleX();
    m_scaleY = m_fractalNode->GetScaleY();
    m_startBand = m_fractalNode->GetStartBand();
    m_endBand = m_fractalNode->GetEndBand();
    m_persistence = m_fractalNode->GetPersistence();
    m_persistenceStart = m_fractalNode->GetPersistenceStart();
    m_brightness = m_fractalNode->GetBrightness();
}

void FractalNoiseParameters::Render() {
    if (!m_node || !m_fractalNode) {
        ImGui::Text("No fractal noise node selected");
        return;
    }
    
    ImGui::Text("Fractal Noise Parameters");
    ImGui::Separator();
    
    // Render all parameters
    m_scaleParam->Render();
    m_scaleXParam->Render();
    m_scaleYParam->Render();
    m_startBandParam->Render();
    m_endBandParam->Render();
    m_persistenceParam->Render();
    m_persistenceStartParam->Render();
    m_brightnessParam->Render();
    
    // Check if any parameter changed and update the node
    if (m_fractalNode) {
        OnParameterChanged();
    }
}

void FractalNoiseParameters::OnParameterChanged() {
    if (!m_fractalNode) return;
    
    // Update node with current parameter values
    m_fractalNode->SetScale(m_scale);
    m_fractalNode->SetScaleX(m_scaleX);
    m_fractalNode->SetScaleY(m_scaleY);
    m_fractalNode->SetStartBand(m_startBand);
    m_fractalNode->SetEndBand(m_endBand);
    m_fractalNode->SetPersistence(m_persistence);
    m_fractalNode->SetPersistenceStart(m_persistenceStart);
    m_fractalNode->SetBrightness(m_brightness);
    
    // Trigger texture regeneration
    m_fractalNode->Update();
}

void FractalNoiseParameters::Clear() {
    m_fractalNode = nullptr;
    m_node = nullptr;
}
