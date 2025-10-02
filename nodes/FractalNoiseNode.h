#pragma once

#include "BaseNode.h"
#include <string>
#include <cmath>

class FractalNoiseNode : public BaseNode {
public:
    FractalNoiseNode(int nodeId);
    virtual ~FractalNoiseNode() = default;
    
    void Render(ax::NodeEditor::EditorContext* editor) override;
    void Update() override;
    
    // Parameter getters
    int GetScale() const { return m_scale; }
    int GetScaleX() const { return m_scaleX; }
    int GetScaleY() const { return m_scaleY; }
    int GetStartBand() const { return m_startBand; }
    int GetEndBand() const { return m_endBand; }
    float GetPersistence() const { return m_persistence; }
    float GetPersistenceStart() const { return m_persistenceStart; }
    float GetBrightness() const { return m_brightness; }
    
    // Parameter setters
    void SetScale(int scale) { m_scale = scale; }
    void SetScaleX(int scaleX) { m_scaleX = scaleX; }
    void SetScaleY(int scaleY) { m_scaleY = scaleY; }
    void SetStartBand(int startBand) { m_startBand = startBand; }
    void SetEndBand(int endBand) { m_endBand = endBand; }
    void SetPersistence(float persistence) { m_persistence = persistence; }
    void SetPersistenceStart(float persistenceStart) { m_persistenceStart = persistenceStart; }
    void SetBrightness(float brightness) { m_brightness = brightness; }
    
private:
    // Parameters
    int m_scale;
    int m_scaleX;
    int m_scaleY;
    int m_startBand;
    int m_endBand;
    float m_persistence;
    float m_persistenceStart;
    float m_brightness;
    
    void ProcessFractalNoise();
    
    // Helper structs for vector operations
    struct float4 {
        float x, y, z, w;
        float4(float x = 0, float y = 0, float z = 0, float w = 0) : x(x), y(y), z(z), w(w) {}
        
        float4 operator+(const float4& other) const {
            return float4(x + other.x, y + other.y, z + other.z, w + other.w);
        }
        
        float4 operator*(float scalar) const {
            return float4(x * scalar, y * scalar, z * scalar, w * scalar);
        }
    };
    
    struct float2 {
        float x, y;
        float2(float x = 0, float y = 0) : x(x), y(y) {}
        
        float2 operator*(const float2& other) const {
            return float2(x * other.x, y * other.y);
        }
        
        float2 operator*(float scalar) const {
            return float2(x * scalar, y * scalar);
        }
    };
    
    // Noise functions (translated from GLSL)
    float mod289(float x);
    float4 mod289(float4 x);
    float4 permute(float4 x);
    float4 taylorInvSqrt(float4 r);
    float2 fade(float2 t);
    float pnoise(float2 P, float2 rep);
};
