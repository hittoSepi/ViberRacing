#include "editor_state.hpp"

#include <glm/common.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <cmath>

namespace car_editor {

EditorLayout computeLayout(const ImVec2& display, const EditorStyle& style) {
    const float boardWidth = glm::max(style.minBoardWidth, display.x - style.shellMargin * 2.0f);
    const float boardHeight = glm::max(style.minBoardHeight, display.y - style.shellMargin * 2.0f);
    const float sidebarWidth = glm::clamp(boardWidth * style.sidebarWidthRatio, style.sidebarMinWidth, style.sidebarMaxWidth);
    const float simHeight = glm::clamp(boardHeight * style.simHeightRatio, style.simMinHeight, style.simMaxHeight);

    EditorLayout layout{};
    layout.boardPos = {style.shellMargin, style.shellMargin};
    layout.boardSize = {boardWidth, boardHeight};

    layout.topBarPos = {
        layout.boardPos.x + style.innerMargin,
        layout.boardPos.y + style.innerMargin
    };
    layout.topBarSize = {
        boardWidth - style.innerMargin * 2.0f,
        style.topBarHeight
    };

    layout.sidebarPos = {
        layout.topBarPos.x,
        layout.topBarPos.y + layout.topBarSize.y + style.gap
    };
    layout.sidebarSize = {
        sidebarWidth,
        boardHeight - style.topBarHeight - simHeight - style.innerMargin * 2.0f - style.gap * 2.0f
    };

    layout.viewportPos = {
        layout.sidebarPos.x + layout.sidebarSize.x + style.gap,
        layout.sidebarPos.y
    };
    layout.viewportSize = {
        boardWidth - sidebarWidth - style.innerMargin * 2.0f - style.gap,
        boardHeight - style.topBarHeight - style.innerMargin * 2.0f - style.gap
    };

    layout.simPos = {
        layout.sidebarPos.x,
        layout.sidebarPos.y + layout.sidebarSize.y + style.gap
    };
    layout.simSize = {
        sidebarWidth,
        simHeight
    };

    layout.controlsSize = {style.controlsWidth, style.controlsHeight};
    layout.controlsPos = {
        layout.viewportPos.x + layout.viewportSize.x - style.controlsInset - layout.controlsSize.x,
        layout.viewportPos.y + layout.viewportSize.y - style.controlsInset - layout.controlsSize.y
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
