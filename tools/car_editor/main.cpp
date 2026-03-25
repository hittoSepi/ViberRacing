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

void initEditorState(car_editor::EditorState& state, const std::string& partsJson) {
    state.carBody.init(state.currentDef, partsJson);

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
}

void shutdownEditorState(car_editor::EditorState& state) {
    state.atmosphere.destroy();
    state.groundPlane.destroy();
    if (bgfx::isValid(state.atmoProgram)) {
        bgfx::destroy(state.atmoProgram);
    }
    if (bgfx::isValid(state.groundProgram)) {
        bgfx::destroy(state.groundProgram);
    }
}

void updateOrbitDrag(car_editor::EditorState& state, GLFWwindow* glfwWindow) {
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }

    double mouseX = 0.0;
    double mouseY = 0.0;
    glfwGetCursorPos(glfwWindow, &mouseX, &mouseY);

    if (glfwGetMouseButton(glfwWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
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
    if (state.autoRotate) {
        autoYaw += dt * 0.3f;
    }

    state.carBody.update(dt);

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

    const mat4 chassisTransform = glm::rotate(mat4{1.0f}, autoYaw, vec3{0, 1, 0});
    std::array<mat4, 4> wheelTransforms;
    const vec3 wheelOffsets[4] = {
        { 0.9f, 0.0f,  1.4f}, {-0.9f, 0.0f,  1.4f},
        { 0.9f, 0.0f, -1.3f}, {-0.9f, 0.0f, -1.3f}
    };
    for (int i = 0; i < 4; ++i) {
        wheelTransforms[i] = chassisTransform * glm::translate(mat4{1.0f}, wheelOffsets[i]);
    }

    state.carBody.render(0, state.program, chassisTransform, wheelTransforms);
}

} // namespace

int main() {
    spdlog::set_level(spdlog::level::info);

    WindowConfig winCfg;
    winCfg.title = "ViberRacing - Car Editor";
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
    window.setScrollCallback([&state](double, double dy) {
        if (!ImGui::GetIO().WantCaptureMouse) {
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
        updateOrbitDrag(state, glfwWindow);

        renderer.beginFrame();
        renderPreviewScene(state, window, dt);

        imguiRenderer.beginFrame();
        const car_editor::EditorLayout layout = car_editor::computeLayout(ImGui::GetIO().DisplaySize, state.style);
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
