#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__APPLE__)
#define GLFW_EXPOSE_NATIVE_COCOA
#else
#define GLFW_EXPOSE_NATIVE_X11
#endif

#include "editor_state.hpp"
#include "editor_style.hpp"
#include "editor_ui.hpp"

#include "core/window.hpp"
#include "renderer/renderer.hpp"
#include "renderer/shaders/embedded_shaders.hpp"
#include "renderer/ui/imgui_renderer.hpp"

#include <GLFW/glfw3.h>
#include <bgfx/bgfx.h>
#include <spdlog/spdlog.h>

#include <array>
#include <string>

using namespace viber;

namespace {

bool pointInRect(const ImVec2& point, const ImVec2& pos, const ImVec2& size) {
    return point.x >= pos.x && point.y >= pos.y &&
           point.x < pos.x + size.x && point.y < pos.y + size.y;
}

bool isMouseInInteractiveViewport(const car_editor::EditorState& state, const car_editor::EditorLayout& layout,
                                  double mouseX, double mouseY) {
    const ImVec2 point(static_cast<float>(mouseX), static_cast<float>(mouseY));
    if (!pointInRect(point, layout.viewportPos, layout.viewportSize)) {
        return false;
    }
    if (state.showControls && pointInRect(point, layout.controlsPos, layout.controlsSize)) {
        return false;
    }
    return true;
}

void initEditorState(car_editor::EditorState& state, const std::string& partsJson) {
    state.carBody.init(state.currentDef, partsJson);
    state.trackPointMarkerMesh = Mesh::createBox({1.2f, 1.2f, 1.2f});
    state.track.generateTestTrack();
    car_editor::syncTrackEditorFieldsFromTrack(state);
    car_editor::rebuildTrackPreview(state);

    state.physicsWorld.setGravity(vec3{0, -9.81f, 0});
    state.previewVehicle = std::make_unique<Vehicle>();
    state.previewVehicle->create(&state.physicsWorld, VehicleParams{});
    state.damageModel.init(&state.carBody, state.previewVehicle.get());

    state.atmoProgram = createAtmosphereShader();
    if (bgfx::isValid(state.atmoProgram)) {
        state.atmosphere.init();
        state.atmosphere.setTimeOfDay(state.timeOfDay);
    } else {
        spdlog::warn("Failed to load atmosphere shader");
    }

    state.groundProgram = createBasicShader();
    if (bgfx::isValid(state.groundProgram)) {
        state.groundPlane.init(50.0f, 50);
    } else {
        spdlog::warn("Failed to load ground shader");
    }

    state.previewTint = bgfx::createUniform("u_tintColor", bgfx::UniformType::Vec4);
    state.previewLightDir = bgfx::createUniform("u_previewLightDir", bgfx::UniformType::Vec4);
    state.previewViewPos = bgfx::createUniform("u_previewViewPos", bgfx::UniformType::Vec4);
}

void shutdownEditorState(car_editor::EditorState& state) {
    state.trackPreviewMesh.destroy();
    state.trackTunnelPreviewMesh.destroy();
    state.trackPointMarkerMesh.destroy();
    state.atmosphere.destroy();
    state.groundPlane.destroy();
    if (bgfx::isValid(state.atmoProgram)) {
        bgfx::destroy(state.atmoProgram);
    }
    if (bgfx::isValid(state.groundProgram)) {
        bgfx::destroy(state.groundProgram);
    }
    if (bgfx::isValid(state.previewTint)) {
        bgfx::destroy(state.previewTint);
    }
    if (bgfx::isValid(state.previewLightDir)) {
        bgfx::destroy(state.previewLightDir);
    }
    if (bgfx::isValid(state.previewViewPos)) {
        bgfx::destroy(state.previewViewPos);
    }
}

void updateOrbitDrag(car_editor::EditorState& state, GLFWwindow* glfwWindow, const car_editor::EditorLayout& layout) {
    double mouseX = 0.0;
    double mouseY = 0.0;
    glfwGetCursorPos(glfwWindow, &mouseX, &mouseY);
    const bool allowViewportInput = isMouseInInteractiveViewport(state, layout, mouseX, mouseY);

    if (allowViewportInput && glfwGetMouseButton(glfwWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (state.dragging) {
            state.camera.orbit(
                static_cast<float>(mouseX - state.lastMouseX),
                static_cast<float>(mouseY - state.lastMouseY));
        }
        state.dragging = true;
    } else {
        state.dragging = false;
    }

    state.lastMouseX = mouseX;
    state.lastMouseY = mouseY;
}

void renderPreviewScene(car_editor::EditorState& state, const Window& window, float dt) {
    static float autoYaw = 0.0f;
    if (state.activeWorkspace == car_editor::EditorWorkspace::Vehicles && state.autoRotate) {
        autoYaw += dt * 0.3f;
    }

    if (state.activeWorkspace == car_editor::EditorWorkspace::Vehicles) {
        state.carBody.update(dt);
    }

    const auto winSize = window.getSize();
    const float aspect = winSize.x / winSize.y;
    const mat4 view = state.camera.getView();
    const mat4 proj = state.camera.getProj(aspect);

    bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(winSize.x), static_cast<uint16_t>(winSize.y));
    bgfx::setViewTransform(0, &view, &proj);

    if (bgfx::isValid(state.atmoProgram)) {
        state.atmosphere.render(0, state.atmoProgram, view, proj);
    }

    if (state.showGround && bgfx::isValid(state.groundProgram)) {
        state.groundPlane.render(0, state.groundProgram, view, proj);
    }

    const vec3 cameraPos{
        state.camera.target.x + state.camera.radius * std::cos(state.camera.pitch) * std::sin(state.camera.yaw),
        state.camera.target.y + state.camera.radius * std::sin(state.camera.pitch),
        state.camera.target.z + state.camera.radius * std::cos(state.camera.pitch) * std::cos(state.camera.yaw)
    };

    const vec4 lightDirAmbient(glm::normalize(vec3{-0.45f, -0.85f, -0.30f}), 0.24f);
    const vec4 viewPosSpec(cameraPos, 0.20f);
    bgfx::setUniform(state.previewLightDir, &lightDirAmbient);
    bgfx::setUniform(state.previewViewPos, &viewPosSpec);

    const u64 previewState = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z |
                             BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CCW | BGFX_STATE_MSAA;

    if (state.activeWorkspace == car_editor::EditorWorkspace::Vehicles) {
        const mat4 chassisTransform = glm::rotate(mat4{1.0f}, autoYaw, vec3{0, 1, 0});
        std::array<mat4, 4> wheelTransforms;
        const vec3 wheelOffsets[4] = {
            { 0.9f, 0.0f,  1.4f}, {-0.9f, 0.0f,  1.4f},
            { 0.9f, 0.0f, -1.3f}, {-0.9f, 0.0f, -1.3f}
        };
        for (int i = 0; i < 4; ++i) {
            wheelTransforms[i] = chassisTransform * glm::translate(mat4{1.0f}, wheelOffsets[i]);
        }

        for (const auto& part : state.carBody.getParts()) {
            if (!part.mesh.isValid()) {
                continue;
            }

            mat4 worldTransform = chassisTransform * part.localTransform;
            vec4 tint(state.currentDef.primaryColor, 1.0f);

            if (part.slot >= CarPartSlot::WheelFL && part.slot <= CarPartSlot::WheelRR) {
                const int idx = static_cast<int>(part.slot) - static_cast<int>(CarPartSlot::WheelFL);
                worldTransform = wheelTransforms[idx];
                tint = vec4(0.12f, 0.12f, 0.14f, 1.0f);
            } else if (part.slot == CarPartSlot::Spoiler) {
                tint = vec4(state.currentDef.accentColor, 1.0f);
            } else if (part.slot == CarPartSlot::BumperFront || part.slot == CarPartSlot::BumperRear) {
                tint = vec4(state.currentDef.primaryColor * 0.92f, 1.0f);
            } else if (part.slot == CarPartSlot::Roof) {
                tint = vec4(glm::mix(state.currentDef.primaryColor, state.currentDef.accentColor, 0.15f), 1.0f);
            }

            bgfx::setUniform(state.previewTint, &tint);
            part.mesh.submit(0, state.program, worldTransform, previewState);
        }
    } else {
        if (state.trackPreviewMesh.isValid() && bgfx::isValid(state.groundProgram)) {
            const u64 roadState = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z |
                                  BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CCW | BGFX_STATE_MSAA;
            state.trackPreviewMesh.submit(0, state.groundProgram, mat4{1.0f}, roadState);
        }

        if (state.trackTunnelPreviewMesh.isValid() && bgfx::isValid(state.groundProgram)) {
            const u64 tunnelState = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z |
                                    BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CCW | BGFX_STATE_MSAA;
            state.trackTunnelPreviewMesh.submit(0, state.groundProgram, mat4{1.0f}, tunnelState);
        }

        for (const auto& point : state.track.getSpline().getControlPoints()) {
            vec4 pointTint(0.90f, 0.72f, 0.18f, 1.0f);
            bgfx::setUniform(state.previewTint, &pointTint);
            state.trackPointMarkerMesh.submit(
                0,
                state.program,
                glm::translate(mat4{1.0f}, point + vec3{0.0f, 0.6f, 0.0f}),
                previewState);
        }

        for (const auto& hole : state.track.holes()) {
            const vec3 holeScale{hole.radius * 2.0f, glm::max(2.0f, hole.depth), hole.radius * 2.0f};
            const vec3 holePos = hole.position + vec3{0.0f, -hole.depth * 0.5f, 0.0f};
            vec4 holeTint(0.18f, 0.82f, 0.86f, 1.0f);
            bgfx::setUniform(state.previewTint, &holeTint);
            state.trackPointMarkerMesh.submit(
                0,
                state.program,
                glm::translate(mat4{1.0f}, holePos) * glm::scale(mat4{1.0f}, holeScale),
                previewState);
        }
    }
}

} // namespace

int main() {
    spdlog::set_level(spdlog::level::info);

    WindowConfig winCfg;
    winCfg.title = "ViberRacing - Game Editor";
    winCfg.width = 1280;
    winCfg.height = 720;
    Window window(winCfg);

    Renderer renderer(window);
    renderer.setClearColor(vec4{0.10f, 0.15f, 0.23f, 1.0f});

    ImGuiRenderer imguiRenderer;
    imguiRenderer.init(window.getHandle());

    const bgfx::ProgramHandle program = createMeshShader();
    if (!bgfx::isValid(program)) {
        spdlog::error("Failed to load mesh shader");
        return 1;
    }

    const std::string partsJson = "assets/cars/parts/parts.json";

    car_editor::EditorState state;
    state.program = program;
    state.style = car_editor::loadEditorStyle("assets/editor/editor_styles.json");
    initEditorState(state, partsJson);

    auto* glfwWindow = window.getHandle();
    window.setScrollCallback([&state, &window](double, double dy) {
        const auto size = window.getSize();
        const car_editor::EditorLayout layout = car_editor::computeLayout(ImVec2(size.x, size.y), state.style);
        const vec2 mouse = window.getMousePosition();
        if (isMouseInInteractiveViewport(state, layout, mouse.x, mouse.y)) {
            state.camera.zoom(static_cast<float>(dy));
        }
    });

    double prevTime = glfwGetTime();
    while (!window.shouldClose()) {
        const double now = glfwGetTime();
        const float dt = static_cast<float>(now - prevTime);
        prevTime = now;
        state.frameTimeMs = dt * 1000.0f;

        window.pollEvents();
        const auto size = window.getSize();
        const car_editor::EditorLayout layout = car_editor::computeLayout(ImVec2(size.x, size.y), state.style);
        updateOrbitDrag(state, glfwWindow, layout);

        renderer.beginFrame();
        renderPreviewScene(state, window, dt);

        imguiRenderer.beginFrame();
        car_editor::drawEditorShell(state, partsJson, layout);
        imguiRenderer.endFrame();
        imguiRenderer.render();

        renderer.endFrame();
    }

    shutdownEditorState(state);
    bgfx::destroy(program);
    imguiRenderer.shutdown();
    return 0;
}
