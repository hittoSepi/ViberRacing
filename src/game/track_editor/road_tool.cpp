#include "game/track_editor/road_tool.hpp"
#include "game/track_editor/editor.hpp"
#include <spdlog/spdlog.h>

namespace viber {

RoadTool::RoadTool(TrackEditor* editor)
    : m_editor(editor)
{
}

void RoadTool::onActivate()
{
    spdlog::info("Road tool activated");
}

void RoadTool::onDeactivate()
{
}

void RoadTool::update(const vec3& cursorPos, bool isMouseDown, bool isRightClick)
{
    if (isMouseDown && !m_wasMouseDown) {
        // Add control point
        m_controlPoints.push_back(cursorPos);
    }
    m_wasMouseDown = isMouseDown;
}

void RoadTool::render(Renderer& renderer)
{
    // Render road preview
}

void RoadTool::setRoadWidth(float width)
{
    m_roadWidth = width;
}

}
