#pragma once
#include "../nodes/BaseNode.h"
#include "../nodes/TileNode.h"
#include <imgui.h>
#include <vector>
#include <memory>
#include <string>

class TileNodeParameters {
public:
    TileNodeParameters();
    ~TileNodeParameters() = default;

    void SetNode(BaseNode* node);
    void Render();
    void ApplyChanges();
    void Clear();

    bool IsValid() const { return m_tileNode != nullptr; }

private:
    TileNode* m_tileNode;

    // Parameter values
    float m_offset;
    float m_brickWidth;
    float m_brickHeight;
    float m_rows;
    float m_columns;
    
    // Display information
    int m_textureWidth;
    int m_textureHeight;

    void UpdateParameters();
};
