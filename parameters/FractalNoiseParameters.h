#pragma once

#include "BaseParameter.h"
#include <memory>

class FractalNoiseNode;

class FractalNoiseParameters {
public:
    FractalNoiseParameters();
    ~FractalNoiseParameters() = default;
    
    void SetNode(BaseNode* node);
    void Render();
    void Clear();
    
private:
    FractalNoiseNode* m_fractalNode;
    BaseNode* m_node;
    
    // Parameter values
    int m_scale;
    int m_scaleX;
    int m_scaleY;
    int m_startBand;
    int m_endBand;
    float m_persistence;
    float m_persistenceStart;
    float m_brightness;
    
    // Parameters
    std::unique_ptr<IntParameter> m_scaleParam;
    std::unique_ptr<IntParameter> m_scaleXParam;
    std::unique_ptr<IntParameter> m_scaleYParam;
    std::unique_ptr<IntParameter> m_startBandParam;
    std::unique_ptr<IntParameter> m_endBandParam;
    std::unique_ptr<FloatParameter> m_persistenceParam;
    std::unique_ptr<FloatParameter> m_persistenceStartParam;
    std::unique_ptr<FloatParameter> m_brightnessParam;
    
    void CreateParameters();
    void UpdateParameters();
    void OnParameterChanged();
};
