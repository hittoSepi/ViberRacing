#include "game/track_editor/terrain_tool.hpp"
#include "game/track_editor/editor.hpp"
#include <spdlog/spdlog.h>

namespace viber {

TerrainTool::TerrainTool(TrackEditor* editor)
    : m_editor(editor)
{
}

void TerrainTool::onActivate()
{
    spdlog::info("Terrain tool activated");
}

void TerrainTool::onDeactivate()
{
}

void TerrainTool::update(const vec3& cursorPos, bool isMouseDown, bool isRightClick)
{
    m_brushPosition = cursorPos;
    
    if (isMouseDown) {
        applyBrush(cursorPos, m_mode);
    }
}

void TerrainTool::render(Renderer& renderer)
{
    // Render brush preview
}

void TerrainTool::setBrushMode(BrushMode mode)
{
    m_mode = mode;
}

void TerrainTool::applyBrush(const vec3& position, BrushMode mode)
{
    // Apply terrain modification
}

}
