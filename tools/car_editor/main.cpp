#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__APPLE__)
#define GLFW_EXPOSE_NATIVE_COCOA
#else
#define GLFW_EXPOSE_NATIVE_X11
#endif

#include "game/entities/car_body.hpp"
#include "game/entities/car_damage.hpp"
#include "core/window.hpp"
#include "core/input.hpp"
#include "renderer/renderer.hpp"
#include "renderer/camera.hpp"
#include "renderer/shaders/embedded_shaders.hpp"
#include "renderer/ui/imgui_renderer.hpp"
#include "physics/world.hpp"
#include "physics/vehicle.hpp"

#include <imgui.h>
#include <GLFW/glfw3.h>
#include <bgfx/bgfx.h>
#include <spdlog/spdlog.h>

#include <array>
#include <string>

using namespace viber;

// ---------------------------------------------------------------------------
// Simple orbit camera
// ---------------------------------------------------------------------------

struct OrbitCamera {
    float yaw = 0.3f;
    float pitch = 0.35f;
    float radius = 8.0f;
    vec3 target{0.0f, 0.5f, 0.0f};

    mat4 getView() const {
        vec3 pos{
            target.x + radius * std::cos(pitch) * std::sin(yaw),
            target.y + radius * std::sin(pitch),
            target.z + radius * std::cos(pitch) * std::cos(yaw)
        };
        return glm::lookAt(pos, target, vec3{0, 1, 0});
    }

    mat4 getProj(float aspect) const {
        return glm::perspective(glm::radians(50.0f), aspect, 0.1f, 200.0f);
    }

    void orbit(float dx, float dy) {
        yaw += dx * 0.01f;
        pitch = glm::clamp(pitch + dy * 0.01f, -1.4f, 1.4f);
    }

    void zoom(float delta) {
        radius = glm::clamp(radius - delta * 0.5f, 2.0f, 40.0f);
    }
};

// ---------------------------------------------------------------------------
// Application state
// ---------------------------------------------------------------------------

struct EditorState {
    CarBody carBody;
    CarDefinition currentDef = CarDefinition::makeDefault();
    u32 seedInput = 42;

    PhysicsWorld physicsWorld;
    std::unique_ptr<Vehicle> previewVehicle;

    OrbitCamera camera;
    bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;

    bool dragging = false;
    double lastMouseX = 0, lastMouseY = 0;

    // damage simulation
    float simImpulse = 200.0f;
    int simWheelIndex = 0;

    DamageModel damageModel;
};

// ---------------------------------------------------------------------------
// ImGui panels
// ---------------------------------------------------------------------------

static void drawPartSelector(EditorState& state, const std::string& partsJson) {
    ImGui::SetNextWindowPos({10, 10}, ImGuiCond_Always);
    ImGui::SetNextWindowSize({260, 500}, ImGuiCond_Always);
    ImGui::Begin("Car Parts", nullptr, ImGuiCond_Always);

    auto& def = state.currentDef;
    bool changed = false;

    // Chassis
    {
        const char* items[] = {"sedan_a", "hatchback_b"};
        int idx = def.chassisVariant == "sedan_a" ? 0 : 1;
        if (ImGui::Combo("Chassis", &idx, items, 2)) {
            def.chassisVariant = items[idx];
            changed = true;
        }
    }
    // Hood
    {
        const char* items[] = {"hood_flat", "hood_scooped"};
        int idx = def.hoodVariant == "hood_flat" ? 0 : 1;
        if (ImGui::Combo("Hood", &idx, items, 2)) {
            def.hoodVariant = items[idx];
            changed = true;
        }
    }
    // Bumper front
    {
        const char* items[] = {"bumper_front_standard", "bumper_front_sport"};
        int idx = def.bumperFrontVariant == "bumper_front_standard" ? 0 : 1;
        if (ImGui::Combo("Front Bumper", &idx, items, 2)) {
            def.bumperFrontVariant = items[idx];
            changed = true;
        }
    }
    // Spoiler
    {
        const char* items[] = {"(none)", "spoiler_small", "spoiler_large"};
        int idx = def.spoilerVariant.empty() ? 0 : (def.spoilerVariant == "spoiler_small" ? 1 : 2);
        if (ImGui::Combo("Spoiler", &idx, items, 3)) {
            def.spoilerVariant = idx == 0 ? "" : items[idx];
            changed = true;
        }
    }
    // Wheels
    {
        const char* items[] = {"wheel_standard", "wheel_sport", "wheel_offroad"};
        auto it = std::find(std::begin(items), std::end(items), def.wheelVariants[0]);
        int idx = static_cast<int>(it - std::begin(items));
        if (idx < 0 || idx >= 3) idx = 0;
        if (ImGui::Combo("Wheels", &idx, items, 3)) {
            def.wheelVariants.fill(items[idx]);
            changed = true;
        }
    }

    ImGui::Separator();
    ImGui::ColorEdit3("Primary Color", &def.primaryColor.x);
    ImGui::ColorEdit3("Accent Color", &def.accentColor.x);

    ImGui::Separator();
    ImGui::InputScalar("Seed", ImGuiDataType_U32, &state.seedInput);
    if (ImGui::Button("Randomize")) {
        def = CarBody::generateFromSeed(state.seedInput, partsJson);
        changed = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Export")) {
        def.toJson("car_export.json");
        spdlog::info("Car exported to car_export.json");
    }

    if (changed)
        state.carBody.setDefinition(def);

    ImGui::End();
}

static void drawDamagePanel(EditorState& state) {
    ImGui::SetNextWindowPos({10, 520}, ImGuiCond_Always);
    ImGui::SetNextWindowSize({260, 180}, ImGuiCond_Always);
    ImGui::Begin("Damage Simulation", nullptr, ImGuiCond_Always);

    ImGui::SliderFloat("Impulse", &state.simImpulse, 0.0f, 1000.0f);
    ImGui::SliderInt("Target Wheel", &state.simWheelIndex, 0, 3);

    const vec3 wheelPositions[4] = {
        { 0.9f, -0.1f,  1.4f}, {-0.9f, -0.1f,  1.4f},
        { 0.9f, -0.1f, -1.3f}, {-0.9f, -0.1f, -1.3f}
    };

    if (ImGui::Button("Hit Wheel")) {
        state.damageModel.processImpact(state.simImpulse, wheelPositions[state.simWheelIndex]);
    }
    ImGui::SameLine();
    if (ImGui::Button("Hit Front")) {
        state.damageModel.processImpact(state.simImpulse, vec3{0.0f, 0.1f, 2.0f});
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
        state.damageModel.reset();
    }

    ImGui::Text("Total damage: %.1f%%", state.damageModel.getTotalDamage() * 100.0f);

    ImGui::End();
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

int main() {
    spdlog::set_level(spdlog::level::info);

    WindowConfig winCfg;
    winCfg.title = "ViberRacing - Car Editor";
    winCfg.width = 1280;
    winCfg.height = 720;
    Window window(winCfg);

    Renderer renderer(window);
    renderer.setClearColor(vec4{0.15f, 0.15f, 0.18f, 1.0f});

    ImGuiRenderer imguiRenderer;
    imguiRenderer.init(window.getHandle());

    // Shader
    bgfx::ProgramHandle program = createBasicShader();
    if (!bgfx::isValid(program)) {
        spdlog::error("Failed to load basic shader");
        return 1;
    }

    const std::string partsJson = "assets/cars/parts/parts.json";

    EditorState state;
    state.program = program;
    state.carBody.init(state.currentDef, partsJson);

    // Dummy vehicle for damage model (no physics world needed for editor preview)
    state.physicsWorld.setGravity(vec3{0, -9.81f, 0});
    state.previewVehicle = std::make_unique<Vehicle>();
    state.previewVehicle->create(&state.physicsWorld, VehicleParams{});
    state.damageModel.init(&state.carBody, state.previewVehicle.get());

    auto* glfwWindow = window.getHandle();

    // Scroll callback for orbit camera zoom
    window.setScrollCallback([&state](double, double dy) {
        if (!ImGui::GetIO().WantCaptureMouse) state.camera.zoom(static_cast<float>(dy));
    });

    double prevTime = glfwGetTime();

    while (!window.shouldClose()) {
        double now = glfwGetTime();
        float dt = static_cast<float>(now - prevTime);
        prevTime = now;

        window.pollEvents();

        // Orbit drag
        if (!ImGui::GetIO().WantCaptureMouse) {
            double mx, my;
            glfwGetCursorPos(glfwWindow, &mx, &my);
            if (glfwGetMouseButton(glfwWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                if (state.dragging)
                    state.camera.orbit(
                        static_cast<float>(mx - state.lastMouseX),
                        static_cast<float>(my - state.lastMouseY));
                state.dragging = true;
            } else {
                state.dragging = false;
            }
            state.lastMouseX = mx;
            state.lastMouseY = my;
        }

        // Slowly rotate car for preview
        static float autoYaw = 0.0f;
        autoYaw += dt * 0.3f;

        state.carBody.update(dt);

        renderer.beginFrame();

        auto winSize = window.getSize();
        float aspect = winSize.x / winSize.y;
        mat4 view = state.camera.getView();
        mat4 proj = state.camera.getProj(aspect);
        bgfx::setViewTransform(0, &view, &proj);
        bgfx::setViewRect(0, 0, 0, static_cast<u16>(winSize.x), static_cast<u16>(winSize.y));

        // Build a simple rotating chassis transform (no real physics)
        mat4 chassisTransform = glm::rotate(mat4{1.0f}, autoYaw, vec3{0, 1, 0});
        std::array<mat4, 4> wheelTransforms;
        const vec3 wheelOffsets[4] = {
            { 0.9f, 0.0f,  1.4f}, {-0.9f, 0.0f,  1.4f},
            { 0.9f, 0.0f, -1.3f}, {-0.9f, 0.0f, -1.3f}
        };
        for (int i = 0; i < 4; ++i)
            wheelTransforms[i] = chassisTransform * glm::translate(mat4{1.0f}, wheelOffsets[i]);

        state.carBody.render(0, program, chassisTransform, wheelTransforms);

        imguiRenderer.beginFrame();
        drawPartSelector(state, partsJson);
        drawDamagePanel(state);
        imguiRenderer.endFrame();
        imguiRenderer.render();

        renderer.endFrame();
    }

    bgfx::destroy(program);
    imguiRenderer.shutdown();

    return 0;
}
