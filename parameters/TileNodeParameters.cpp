#include "TileNodeParameters.h"
#include "../nodes/TileNode.h"
#include <string>
#include <imgui.h>

TileNodeParameters::TileNodeParameters()
    : m_tileNode(nullptr), m_offset(0.5f), m_brickWidth(1.0f), m_brickHeight(1.0f), 
      m_rows(6.0f), m_columns(6.0f), m_textureWidth(0), m_textureHeight(0) {}

void TileNodeParameters::SetNode(BaseNode* node) {
    // Only update if the node actually changed
    if (m_tileNode != static_cast<TileNode*>(node)) {
        m_tileNode = static_cast<TileNode*>(node);
        if (m_tileNode) {
            UpdateParameters();
        }
    }
}

void TileNodeParameters::Render() {
    if (!m_tileNode) {
        ImGui::Text("No Tile node selected");
        return;
    }
    
    ImGui::Text("Tile Node Parameters");
    ImGui::Separator();
    
    // Offset
    std::string offsetId = "##Offset_" + std::to_string(m_tileNode ? m_tileNode->GetNodeId() : 0);
    if (ImGui::SliderFloat(offsetId.c_str(), &m_offset, 0.0f, 1.0f, "%.2f")) {
        ApplyChanges();
    }
    ImGui::SameLine();
    ImGui::Text("Offset");
    
    // Brick Width
    std::string brickWidthId = "##BrickWidth_" + std::to_string(m_tileNode ? m_tileNode->GetNodeId() : 0);
    if (ImGui::SliderFloat(brickWidthId.c_str(), &m_brickWidth, 0.0f, 1.0f, "%.2f")) {
        ApplyChanges();
    }
    ImGui::SameLine();
    ImGui::Text("Tile Width");
    
    // Brick Height
    std::string brickHeightId = "##BrickHeight_" + std::to_string(m_tileNode ? m_tileNode->GetNodeId() : 0);
    if (ImGui::SliderFloat(brickHeightId.c_str(), &m_brickHeight, 0.0f, 1.0f, "%.2f")) {
        ApplyChanges();
    }
    ImGui::SameLine();
    ImGui::Text("Tile Height");
    
    // Rows
    std::string rowsId = "##Rows_" + std::to_string(m_tileNode ? m_tileNode->GetNodeId() : 0);
    if (ImGui::SliderFloat(rowsId.c_str(), &m_rows, 1.0f, 20.0f, "%.0f")) {
        ApplyChanges();
    }
    ImGui::SameLine();
    ImGui::Text("Rows");
    
    // Columns
    std::string columnsId = "##Columns_" + std::to_string(m_tileNode ? m_tileNode->GetNodeId() : 0);
    if (ImGui::SliderFloat(columnsId.c_str(), &m_columns, 1.0f, 20.0f, "%.0f")) {
        ApplyChanges();
    }
    ImGui::SameLine();
    ImGui::Text("Columns");
    
    ImGui::Separator();
    
    // Texture Information
    ImGui::Text("Texture Information");
    if (m_tileNode->GetTextureData()) {
        ImGui::Text("Output Size: %dx%d", m_textureWidth, m_textureHeight);
    } else {
        ImGui::Text("Output Size: No texture generated");
    }
    
    if (m_tileNode->GetInputTextureData()) {
        ImGui::Text("Input Size: %dx%d", 
                   m_tileNode->GetInputTextureData()->width, 
                   m_tileNode->GetInputTextureData()->height);
    } else {
        ImGui::Text("Input Size: No input connected");
    }
}

void TileNodeParameters::ApplyChanges() {
    if (!m_tileNode) return;
    
    // Apply changes to the node
    m_tileNode->SetOffset(m_offset);
    m_tileNode->SetBrickWidth(m_brickWidth);
    m_tileNode->SetBrickHeight(m_brickHeight);
    m_tileNode->SetRows(m_rows);
    m_tileNode->SetColumns(m_columns);
    
    // Update the node to process the changes
    m_tileNode->Update();
}

void TileNodeParameters::Clear() {
    m_tileNode = nullptr;
    m_offset = 0.5f;
    m_brickWidth = 1.0f;
    m_brickHeight = 1.0f;
    m_rows = 6.0f;
    m_columns = 6.0f;
}

void TileNodeParameters::UpdateParameters() {
    if (!m_tileNode) return;
    
    m_offset = m_tileNode->GetOffset();
    m_brickWidth = m_tileNode->GetBrickWidth();
    m_brickHeight = m_tileNode->GetBrickHeight();
    m_rows = m_tileNode->GetRows();
    m_columns = m_tileNode->GetColumns();
    
    // Update texture dimensions
    if (m_tileNode->GetTextureData()) {
        m_textureWidth = m_tileNode->GetTextureData()->width;
        m_textureHeight = m_tileNode->GetTextureData()->height;
    } else {
        m_textureWidth = 0;
        m_textureHeight = 0;
    }
}
