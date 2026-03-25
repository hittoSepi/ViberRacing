#include "game/track_editor/prop_tool.hpp"
#include "game/track_editor/editor.hpp"
#include <spdlog/spdlog.h>

namespace viber {

PropTool::PropTool(TrackEditor* editor)
    : m_editor(editor)
{
    m_availableProps = {
        {"Tree Pine", "models/props/tree_pine.glb", vec3(2.0f, 8.0f, 2.0f), true},
        {"Tree Oak", "models/props/tree_oak.glb", vec3(3.0f, 6.0f, 3.0f), true},
        {"Barrier", "models/props/barrier.glb", vec3(2.0f, 1.0f, 0.5f), true},
        {"Tire Stack", "models/props/tire_stack.glb", vec3(1.5f, 1.0f, 1.5f), true},
        {"Grandstand", "models/props/grandstand.glb", vec3(20.0f, 10.0f, 5.0f), false},
    };
}

void PropTool::onActivate() {
    spdlog::info("Prop tool activated");
}

void PropTool::onDeactivate() {
}

void PropTool::update(const vec3& cursorPos, bool isMouseDown, bool) {
    m_cursorPosition = cursorPos;
    
    if (isMouseDown && !m_wasMouseDown) {
        placeProp(cursorPos);
    }
    
    m_wasMouseDown = isMouseDown;
}

void PropTool::render(Renderer&) {
}

void PropTool::setSelectedProp(int index) {
    if (index >= 0 && index < static_cast<int>(m_availableProps.size())) {
        m_selectedPropIndex = index;
    }
}

void PropTool::setRotation(float degrees) {
    m_rotation = degrees;
}

void PropTool::deleteSelected() {
    spdlog::info("Deleting selected prop");
}

void PropTool::placeProp(const vec3& position) {
    if (m_selectedPropIndex < 0 || m_selectedPropIndex >= static_cast<int>(m_availableProps.size())) {
        return;
    }
    
    const auto& prop = m_availableProps[m_selectedPropIndex];
    spdlog::info("Placing prop '{}' at ({:.1f}, {:.1f}, {:.1f})", 
        prop.name, position.x, position.y, position.z);
}

}
