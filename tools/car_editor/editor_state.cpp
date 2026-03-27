#include "editor_state.hpp"

#include <glm/common.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <cmath>
#include <cstdio>
#include <vector>

namespace car_editor {

namespace {

void rebuildTunnelPreview(EditorState& state) {
    state.trackTunnelPreviewMesh.destroy();

    std::vector<viber::VertexSimple> vertices;
    std::vector<viber::u32> indices;

    constexpr int radialSegments = 12;
    for (const auto& tunnel : state.track.tunnels()) {
        if (tunnel.spline.getNumControlPoints() < 2) {
            continue;
        }

        const auto frames = tunnel.spline.sampleFrames(48);
        if (frames.size() < 2) {
            continue;
        }

        const viber::u32 baseIndex = static_cast<viber::u32>(vertices.size());
        for (const auto& frame : frames) {
            for (int segment = 0; segment < radialSegments; ++segment) {
                const float angle = viber::TWO_PI * static_cast<float>(segment) / static_cast<float>(radialSegments);
                const float x = std::cos(angle) * tunnel.radius;
                const float y = std::sin(angle) * tunnel.radius;
                const viber::vec3 offset = frame.binormal * x + frame.normal * y;
                vertices.push_back({
                    frame.position + offset,
                    viber::vec3{0.10f, 0.44f, 0.58f}
                });
            }
        }

        for (size_t ring = 0; ring + 1 < frames.size(); ++ring) {
            const viber::u32 ringBase = baseIndex + static_cast<viber::u32>(ring * radialSegments);
            const viber::u32 nextRingBase = ringBase + radialSegments;
            for (int segment = 0; segment < radialSegments; ++segment) {
                const viber::u32 nextSegment = static_cast<viber::u32>((segment + 1) % radialSegments);
                const viber::u32 i0 = ringBase + static_cast<viber::u32>(segment);
                const viber::u32 i1 = ringBase + nextSegment;
                const viber::u32 i2 = nextRingBase + static_cast<viber::u32>(segment);
                const viber::u32 i3 = nextRingBase + nextSegment;
                indices.push_back(i0);
                indices.push_back(i2);
                indices.push_back(i1);
                indices.push_back(i1);
                indices.push_back(i2);
                indices.push_back(i3);
            }
        }
    }

    if (!vertices.empty() && !indices.empty()) {
        state.trackTunnelPreviewMesh.create(vertices, indices);
    }
}

} // namespace

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
    return glm::perspective(glm::radians(50.0f), aspect, 0.1f, 4000.0f);
}

void OrbitCamera::orbit(float dx, float dy) {
    yaw += dx * 0.01f;
    pitch = glm::clamp(pitch + dy * 0.01f, -1.4f, 1.4f);
}

void OrbitCamera::zoom(float delta) {
    const float zoomStep = glm::max(1.0f, radius * 0.12f);
    radius = glm::clamp(radius - delta * zoomStep, 2.0f, 400.0f);
}

void applyCarDefinition(EditorState& state, const viber::CarDefinition& def) {
    state.currentDef = def;
    state.seedInput = def.seed;
    state.carBody.setDefinition(def);
}

void rebuildTrackPreview(EditorState& state) {
    state.trackPreviewMesh.destroy();
    rebuildTunnelPreview(state);

    const auto& vertices = state.track.getRoadVertices();
    const auto& indices = state.track.getRoadIndices();
    if (vertices.empty() || indices.empty()) {
        return;
    }

    std::vector<viber::VertexSimple> simpleVertices;
    simpleVertices.reserve(vertices.size());
    for (const auto& vertex : vertices) {
        simpleVertices.push_back({vertex, viber::vec3{0.18f, 0.20f, 0.22f}});
    }

    state.trackPreviewMesh.create(simpleVertices, indices);
}

void rebuildGroundPreview(EditorState& state) {
    state.groundPlane.destroy();
    float size = 50.0f;
    if (state.activeWorkspace == EditorWorkspace::Tracks) {
        size = glm::max(64.0f, state.track.terrainSettings().worldSize);
    }
    const viber::u32 gridLines = static_cast<viber::u32>(glm::clamp(size / 8.0f, 24.0f, 220.0f));
    state.groundPlane.init(size, gridLines);
}

void resetCamera(EditorState& state) {
    state.camera.yaw = 0.3f;
    state.camera.pitch = 0.35f;
    state.camera.radius = 8.0f;
    state.camera.target = viber::vec3{0.0f, 0.5f, 0.0f};
}

void resetTrackCamera(EditorState& state) {
    const float worldSize = glm::max(64.0f, state.track.terrainSettings().worldSize);
    state.camera.yaw = 0.4f;
    state.camera.pitch = 0.65f;
    state.camera.radius = glm::clamp(worldSize * 0.32f, 35.0f, 260.0f);
    state.camera.target = viber::vec3{0.0f, 0.0f, 0.0f};
}

void syncTrackEditorFieldsFromTrack(EditorState& state) {
    std::snprintf(state.trackName, sizeof(state.trackName), "%s", state.track.getName().c_str());
    const auto& terrain = state.track.terrainSettings();
    std::snprintf(state.terrainBiome, sizeof(state.terrainBiome), "%s", terrain.biome.c_str());
    std::snprintf(state.terrainHeightmapEditPath, sizeof(state.terrainHeightmapEditPath), "%s", terrain.heightmapEditPath.c_str());
    std::snprintf(state.terrainHoleMaskPath, sizeof(state.terrainHoleMaskPath), "%s", terrain.holeMaskPath.c_str());
}

} // namespace car_editor
