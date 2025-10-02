#include "FractalNoiseNode.h"
#include <imgui_internal.h>
#include <algorithm>
#include <random>
#include <iostream>

FractalNoiseNode::FractalNoiseNode(int nodeId)
    : BaseNode(nodeId, NodeType::Texture, "Fractal Noise", ChannelType::Color, ColorMode::RGB),
      m_scale(1), m_scaleX(1), m_scaleY(1), m_startBand(4), m_endBand(8),
      m_persistence(0.6f), m_persistenceStart(1.0f), m_brightness(0.5f) {
    
    // Set up pins for texture node
    m_outputPinId = nodeId * 100 + 1;  // Output pin
    
    // Generate initial texture
    ProcessFractalNoise();
}

void FractalNoiseNode::Render(ax::NodeEditor::EditorContext* editor) {
    ax::NodeEditor::BeginNode(m_nodeId);

    // Node content
    ImGui::BeginGroup();
    ImGui::Text("Fractal Noise");
    
    
    // Display current settings (read-only in node)
    ImGui::Text("Scale: %d", m_scale);
    ImGui::Text("X/Y: %d/%d", m_scaleX, m_scaleY);
    ImGui::Text("Bands: %d-%d", m_startBand, m_endBand);
    ImGui::Text("Persistence: %.2f", m_persistence);
    ImGui::Text("Brightness: %.2f", m_brightness);
    
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

void FractalNoiseNode::Update() {
    ProcessFractalNoise();
}

void FractalNoiseNode::ProcessFractalNoise() {
    const int width = 512;
    const int height = 512;
    
    
    // Create result texture
    auto result = std::make_shared<TextureData>(width, height, 3);
    
    // Generate fractal noise
    for (int y = 0; y < 512; ++y) {
        for (int x = 0; x < 512; ++x) {
            // Convert pixel coordinates to UV coordinates (0-1)
            float2 uv = float2((float)x / 511.0f, (float)y / 511.0f);
            
            float scale_value = m_persistenceStart;
            float persistence = m_persistence;
            float total = 0.0f;
            float2 scale = float2((float)m_scaleX, (float)m_scaleY) * (float)m_scale;
            
            // Generate fractal noise
            for (int i = 1; i <= 20; i++) {
                if (i >= m_startBand && i <= m_endBand) {
                    total += pnoise(uv * scale, scale) * scale_value;
                    scale_value *= persistence;
                }
                scale = float2(scale.x * 2.0f, scale.y * 2.0f);
            }
            
            // Apply brightness and clamp
            float noise_value = total + m_brightness;
            noise_value = std::max(0.0f, std::min(1.0f, noise_value));
            
            int index = (y * 512 + x) * 3;
            unsigned char val = (unsigned char)(noise_value * 255);
            result->data[index + 0] = val;  // R
            result->data[index + 1] = val;  // G
            result->data[index + 2] = val;  // B
        }
    }
    
    m_textureData = result;
    m_textureID = TextureGenerator::CreateOpenGLTexture(*m_textureData);
}

// GLSL noise functions translated to C++

float FractalNoiseNode::mod289(float x) {
    return x - floor(x * (1.0f / 289.0f)) * 289.0f;
}

FractalNoiseNode::float4 FractalNoiseNode::mod289(float4 x) {
    return float4(
        x.x - floor(x.x * (1.0f / 289.0f)) * 289.0f,
        x.y - floor(x.y * (1.0f / 289.0f)) * 289.0f,
        x.z - floor(x.z * (1.0f / 289.0f)) * 289.0f,
        x.w - floor(x.w * (1.0f / 289.0f)) * 289.0f
    );
}

FractalNoiseNode::float4 FractalNoiseNode::permute(float4 x) {
    return mod289(float4(
        ((x.x * 34.0f) + 1.0f) * x.x,
        ((x.y * 34.0f) + 1.0f) * x.y,
        ((x.z * 34.0f) + 1.0f) * x.z,
        ((x.w * 34.0f) + 1.0f) * x.w
    ));
}

FractalNoiseNode::float4 FractalNoiseNode::taylorInvSqrt(float4 r) {
    return float4(
        1.79284291400159f - 0.85373472095314f * r.x,
        1.79284291400159f - 0.85373472095314f * r.y,
        1.79284291400159f - 0.85373472095314f * r.z,
        1.79284291400159f - 0.85373472095314f * r.w
    );
}

FractalNoiseNode::float2 FractalNoiseNode::fade(float2 t) {
    return float2(
        t.x * t.x * t.x * (t.x * (t.x * 6.0f - 15.0f) + 10.0f),
        t.y * t.y * t.y * (t.y * (t.y * 6.0f - 15.0f) + 10.0f)
    );
}

float FractalNoiseNode::pnoise(float2 P, float2 rep) {
    float4 Pi = float4(
        floor(P.x), floor(P.y),
        floor(P.x) + 1.0f, floor(P.y) + 1.0f
    );
    float4 Pf = float4(
        P.x - floor(P.x), P.y - floor(P.y),
        P.x - floor(P.x) - 1.0f, P.y - floor(P.y) - 1.0f
    );
    
    // To create noise with explicit period
    Pi = float4(
        fmod(Pi.x, rep.x), fmod(Pi.y, rep.y),
        fmod(Pi.z, rep.x), fmod(Pi.w, rep.y)
    );
    Pi = mod289(Pi);  // To avoid truncation effects in permutation
    
    float4 ix = float4(Pi.x, Pi.z, Pi.x, Pi.z);
    float4 iy = float4(Pi.y, Pi.y, Pi.w, Pi.w);
    float4 fx = float4(Pf.x, Pf.z, Pf.x, Pf.z);
    float4 fy = float4(Pf.y, Pf.y, Pf.w, Pf.w);
    
    float4 i = permute(permute(ix) + iy);
    
    float4 gx = float4(
        fmod(i.x * (1.0f / 41.0f), 1.0f) * 2.0f - 1.0f,
        fmod(i.y * (1.0f / 41.0f), 1.0f) * 2.0f - 1.0f,
        fmod(i.z * (1.0f / 41.0f), 1.0f) * 2.0f - 1.0f,
        fmod(i.w * (1.0f / 41.0f), 1.0f) * 2.0f - 1.0f
    );
    float4 gy = float4(
        fabs(gx.x) - 0.5f,
        fabs(gx.y) - 0.5f,
        fabs(gx.z) - 0.5f,
        fabs(gx.w) - 0.5f
    );
    float4 tx = float4(
        floor(gx.x + 0.5f),
        floor(gx.y + 0.5f),
        floor(gx.z + 0.5f),
        floor(gx.w + 0.5f)
    );
    gx = float4(gx.x - tx.x, gx.y - tx.y, gx.z - tx.z, gx.w - tx.w);
    
    float2 g00 = float2(gx.x, gy.x);
    float2 g10 = float2(gx.y, gy.y);
    float2 g01 = float2(gx.z, gy.z);
    float2 g11 = float2(gx.w, gy.w);
    
    float4 norm = taylorInvSqrt(float4(
        g00.x * g00.x + g00.y * g00.y,
        g01.x * g01.x + g01.y * g01.y,
        g10.x * g10.x + g10.y * g10.y,
        g11.x * g11.x + g11.y * g11.y
    ));
    
    g00 = float2(g00.x * norm.x, g00.y * norm.x);
    g01 = float2(g01.x * norm.y, g01.y * norm.y);
    g10 = float2(g10.x * norm.z, g10.y * norm.z);
    g11 = float2(g11.x * norm.w, g11.y * norm.w);
    
    float n00 = g00.x * fx.x + g00.y * fy.x;
    float n10 = g10.x * fx.y + g10.y * fy.y;
    float n01 = g01.x * fx.z + g01.y * fy.z;
    float n11 = g11.x * fx.w + g11.y * fy.w;
    
    float2 fade_xy = fade(float2(Pf.x, Pf.y));
    float2 n_x = float2(
        n00 + (n10 - n00) * fade_xy.x,
        n01 + (n11 - n01) * fade_xy.x
    );
    float n_xy = n_x.x + (n_x.y - n_x.x) * fade_xy.y;
    
    return 2.3f * n_xy;
}
