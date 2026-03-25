#include "editor_state.hpp"

#include <glm/common.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <cmath>

namespace car_editor {

EditorLayout computeLayout(const ImVec2& display) {
    const float shellMargin = 0.0f;
    const float innerMargin = 10.0f;
    const float gap = 10.0f;
    const float boardWidth = glm::max(640.0f, display.x - shellMargin * 2.0f);
    const float boardHeight = glm::max(420.0f, display.y - shellMargin * 2.0f);
    const float sidebarWidth = glm::clamp(boardWidth * 0.23f, 230.0f, 300.0f);
    const float topBarHeight = 52.0f;
    const float simHeight = glm::clamp(boardHeight * 0.28f, 150.0f, 210.0f);

    EditorLayout layout{};
    layout.boardPos = {shellMargin, shellMargin};
    layout.boardSize = {boardWidth, boardHeight};

    layout.topBarPos = {
        layout.boardPos.x + innerMargin,
        layout.boardPos.y + innerMargin
    };
    layout.topBarSize = {
        boardWidth - innerMargin * 2.0f,
        topBarHeight
    };

    layout.sidebarPos = {
        layout.topBarPos.x,
        layout.topBarPos.y + layout.topBarSize.y + gap
    };
    layout.sidebarSize = {
        sidebarWidth,
        boardHeight - topBarHeight - simHeight - innerMargin * 2.0f - gap * 2.0f
    };

    layout.viewportPos = {
        layout.sidebarPos.x + layout.sidebarSize.x + gap,
        layout.sidebarPos.y
    };
    layout.viewportSize = {
        boardWidth - sidebarWidth - innerMargin * 2.0f - gap,
        boardHeight - topBarHeight - innerMargin * 2.0f - gap
    };

    layout.simPos = {
        layout.sidebarPos.x,
        layout.sidebarPos.y + layout.sidebarSize.y + gap
    };
    layout.simSize = {
        sidebarWidth,
        simHeight
    };

    layout.controlsSize = {220.0f, 110.0f};
    layout.controlsPos = {
        layout.viewportPos.x + layout.viewportSize.x - 12.0f - layout.controlsSize.x,
        layout.viewportPos.y + layout.viewportSize.y - 12.0f - layout.controlsSize.y
    };
    return layout;
}

viber::mat4 OrbitCamera::getView() const {
    const viber::vec3 pos{
        target.x + radius * std::cos(pitch) * std::sin(yaw),
        target.y + radius * std::sin(pitch),
        target.z + radius * std::cos(pitch) * std::cos(yaw)
    };
    return glm::lookAt(pos, target, viber::vec3{0, 1, 0});
}

viber::mat4 OrbitCamera::getProj(float aspect) const {
    return glm::perspective(glm::radians(50.0f), aspect, 0.1f, 200.0f);
}

void OrbitCamera::orbit(float dx, float dy) {
    yaw += dx * 0.01f;
    pitch = glm::clamp(pitch + dy * 0.01f, -1.4f, 1.4f);
}

void OrbitCamera::zoom(float delta) {
    radius = glm::clamp(radius - delta * 0.5f, 2.0f, 40.0f);
}

void applyCarDefinition(EditorState& state, const viber::CarDefinition& def) {
    state.currentDef = def;
    state.seedInput = def.seed;
    state.carBody.setDefinition(def);
}

void resetCamera(EditorState& state) {
    state.camera.yaw = 0.3f;
    state.camera.pitch = 0.35f;
    state.camera.radius = 8.0f;
    state.camera.target = viber::vec3{0.0f, 0.5f, 0.0f};
}

} // namespace car_editor
