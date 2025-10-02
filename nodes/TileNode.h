#pragma once

#include "BaseNode.h"
#include "../TextureGenerator.h" // For TextureData
#include <vector>

class TileNode : public BaseNode {
public:
    TileNode(int nodeId);
    virtual ~TileNode() = default;
    
    // Override virtual methods
    void Render(ax::NodeEditor::EditorContext* editor) override;
    void Update() override;
    
    // Parameter getters and setters
    float GetOffset() const { return m_offset; }
    void SetOffset(float offset) { m_offset = offset; m_needsUpdate = true; }
    
    float GetBrickWidth() const { return m_brickWidth; }
    void SetBrickWidth(float width) { m_brickWidth = width; m_needsUpdate = true; }
    
    float GetBrickHeight() const { return m_brickHeight; }
    void SetBrickHeight(float height) { m_brickHeight = height; m_needsUpdate = true; }
    
    float GetRows() const { return m_rows; }
    void SetRows(float rows) { m_rows = rows; m_needsUpdate = true; }
    
    float GetColumns() const { return m_columns; }
    void SetColumns(float columns) { m_columns = columns; m_needsUpdate = true; }

private:
    // Parameters
    float m_offset;
    float m_brickWidth;
    float m_brickHeight;
    float m_rows;
    float m_columns;
    
    // State
    bool m_needsUpdate;
    
    void GenerateTexture();
    void GenerateTileTexture();
};
