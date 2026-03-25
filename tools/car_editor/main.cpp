#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__APPLE__)
#define GLFW_EXPOSE_NATIVE_COCOA
#else
#define GLFW_EXPOSE_NATIVE_X11
#endif

#include "game/entities/car_body.hpp"
#include "game/entities/car_damage.hpp"
#include "game/entities/atmosphere.hpp"
#include "game/entities/ground_plane.hpp"
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

namespace {

enum class SidebarView {
    CarBuilder = 0,
    Atmosphere
};

struct EditorLayout {
    ImVec2 boardPos;
    ImVec2 boardSize;
    ImVec2 topBarPos;
    ImVec2 topBarSize;
    ImVec2 sidebarPos;
    ImVec2 sidebarSize;
    ImVec2 viewportPos;
    ImVec2 viewportSize;
    ImVec2 simPos;
    ImVec2 simSize;
    ImVec2 controlsPos;
    ImVec2 controlsSize;
};

EditorLayout computeLayout() {
    const ImVec2 display = ImGui::GetIO().DisplaySize;
    const float shellMargin = 8.0f;
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

} // namespace

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
    
    // Atmosphere / Sky
    Atmosphere atmosphere;
    bgfx::ProgramHandle atmoProgram = BGFX_INVALID_HANDLE;
    float timeOfDay = 0.34f;
    
    // Ground plane
    GroundPlane groundPlane;
    bgfx::ProgramHandle groundProgram = BGFX_INVALID_HANDLE;
    SidebarView activeView = SidebarView::CarBuilder;
    bool autoRotate = true;
    bool showGround = true;
    bool showSimulation = true;
    bool showControls = true;
    bool showHelp = false;
    float frameTimeMs = 0.0f;
};

// ---------------------------------------------------------------------------
// ImGui panels
// ---------------------------------------------------------------------------

static void drawPartSelector(EditorState& state, const std::string& partsJson) {
    auto& def = state.currentDef;
    bool changed = false;

    ImGui::TextDisabled("Body");
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
    ImGui::TextDisabled("Color");
    ImGui::ColorEdit3("Primary Color", &def.primaryColor.x);
    ImGui::ColorEdit3("Accent Color", &def.accentColor.x);

    ImGui::Separator();
    ImGui::TextDisabled("Generation");
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
}

static void drawDamagePanel(EditorState& state) {
    ImGui::TextDisabled("Damage Test");
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
}

static void drawAtmospherePanel(EditorState& state) {
    if (ImGui::SliderFloat("Time Of Day", &state.timeOfDay, 0.0f, 1.0f)) {
        state.atmosphere.setTimeOfDay(state.timeOfDay);
    }

    AtmosphereSettings& atmo = state.atmosphere.settings();

    ImGui::ColorEdit3("Sky Top", &atmo.skyTopColor.x);
    ImGui::ColorEdit3("Sky Bottom", &atmo.skyBottomColor.x);
    ImGui::ColorEdit3("Horizon", &atmo.horizonColor.x);
    ImGui::ColorEdit3("Sun Color", &atmo.sunColor.x);
    ImGui::SliderFloat("Sun Size", &atmo.sunSize, 0.002f, 0.05f);
    ImGui::SliderFloat("Sun Intensity", &atmo.sunIntensity, 0.0f, 2.0f);
    ImGui::SliderFloat("Horizon Haze", &atmo.horizonStrength, 0.0f, 0.6f);
    ImGui::SliderFloat("Aerial Blend", &atmo.aerialStrength, 0.0f, 0.4f);

    if (ImGui::Button("Reset Day")) {
        state.timeOfDay = 0.34f;
        state.atmosphere.setTimeOfDay(state.timeOfDay);
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset Sunset")) {
        state.timeOfDay = 0.52f;
        state.atmosphere.setTimeOfDay(state.timeOfDay);
    }
}

static void applyCarDefinition(EditorState& state, const CarDefinition& def) {
    state.currentDef = def;
    state.seedInput = def.seed;
    state.carBody.setDefinition(def);
}

static void resetCamera(EditorState& state) {
    state.camera.yaw = 0.3f;
    state.camera.pitch = 0.35f;
    state.camera.radius = 8.0f;
    state.camera.target = vec3{0.0f, 0.5f, 0.0f};
}

static void drawTopBar(EditorState& state, const std::string& partsJson, const EditorLayout& layout) {
    ImGui::SetCursorScreenPos(layout.topBarPos);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.13f, 0.17f, 0.22f, 0.94f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.27f, 0.35f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.37f, 0.48f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.24f, 0.32f, 0.42f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.10f, 0.13f, 0.17f, 0.98f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.92f, 0.94f, 0.96f, 1.0f));
    ImGui::BeginChild("TopBar", layout.topBarSize, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::SetCursorPosY(14.0f);

    if (ImGui::Button("File")) {
        ImGui::OpenPopup("FileMenu");
    }
    if (ImGui::BeginPopup("FileMenu")) {
        if (ImGui::MenuItem("Randomize From Seed")) {
            applyCarDefinition(state, CarBody::generateFromSeed(state.seedInput, partsJson));
        }
        if (ImGui::MenuItem("Reset Default")) {
            applyCarDefinition(state, CarDefinition::makeDefault());
        }
        if (ImGui::MenuItem("Load car_export.json")) {
            applyCarDefinition(state, CarDefinition::fromJson("car_export.json"));
        }
        if (ImGui::MenuItem("Export car_export.json")) {
            state.currentDef.toJson("car_export.json");
        }
        ImGui::EndPopup();
    }

    ImGui::SameLine();
    if (ImGui::Button("View")) {
        ImGui::OpenPopup("ViewMenu");
    }
    if (ImGui::BeginPopup("ViewMenu")) {
        if (ImGui::MenuItem("Car Builder", nullptr, state.activeView == SidebarView::CarBuilder)) {
            state.activeView = SidebarView::CarBuilder;
        }
        if (ImGui::MenuItem("Atmosphere", nullptr, state.activeView == SidebarView::Atmosphere)) {
            state.activeView = SidebarView::Atmosphere;
        }
        ImGui::Separator();
        ImGui::MenuItem("Auto Rotate", nullptr, &state.autoRotate);
        ImGui::MenuItem("Ground Plane", nullptr, &state.showGround);
        ImGui::MenuItem("Simulation Panel", nullptr, &state.showSimulation);
        ImGui::MenuItem("Controls Panel", nullptr, &state.showControls);
        if (ImGui::MenuItem("Reset Camera")) {
            resetCamera(state);
        }
        ImGui::EndPopup();
    }

    const char* activeLabel = state.activeView == SidebarView::CarBuilder ? "Car Builder" : "Atmosphere";
    const float fps = state.frameTimeMs > 0.0f ? 1000.0f / state.frameTimeMs : 0.0f;
    const std::string status = std::string(activeLabel) + "  |  Seed " + std::to_string(state.seedInput) +
        "  |  " + std::to_string(static_cast<int>(fps + 0.5f)) + " FPS";
    const float statusX = glm::max(160.0f, layout.topBarSize.x - 360.0f);
    ImGui::SameLine(statusX);
    ImGui::TextDisabled("%s", status.c_str());

    ImGui::SameLine(layout.topBarSize.x - 68.0f);
    if (ImGui::Button("Help")) {
        state.showHelp = true;
        ImGui::OpenPopup("HelpPopup");
    }
    if (ImGui::BeginPopupModal("HelpPopup", &state.showHelp, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Orbit the car with left mouse and zoom with the wheel.");
        ImGui::Text("Use the sidebar to switch between build and atmosphere tools.");
        ImGui::Text("File menu handles import/export and presets.");
        if (ImGui::Button("Close")) {
            state.showHelp = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::EndChild();
    ImGui::PopStyleColor(6);
}

static void drawViewportFrame(EditorState& state, const EditorLayout& layout) {
    ImGui::SetCursorScreenPos(layout.viewportPos);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.07f, 0.10f, 0.18f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.35f, 0.47f, 0.60f, 0.65f));
    ImGui::BeginChild("ViewportFrame", layout.viewportSize, true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::TextDisabled("VIEWPORT");
    ImGui::Separator();
    ImGui::Text("Drag to orbit");
    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();
    ImGui::Text("Wheel to zoom");
    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();
    ImGui::Text("Time %.0f%%", state.timeOfDay * 100.0f);
    ImGui::EndChild();
    ImGui::PopStyleColor(2);
}

static void drawEditorShell(EditorState& state, const std::string& partsJson, const EditorLayout& layout) {
    ImGui::SetNextWindowPos(layout.boardPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(layout.boardSize, ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.07f, 0.10f, 0.14f, 0.06f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.40f, 0.51f, 0.66f, 0.35f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::Begin("Board", nullptr,
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar);
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);

    drawTopBar(state, partsJson, layout);
    drawViewportFrame(state, layout);

    ImGui::SetCursorScreenPos(layout.sidebarPos);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.15f, 0.19f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.20f, 0.29f, 0.39f, 0.90f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.28f, 0.39f, 0.52f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.24f, 0.34f, 0.46f, 1.0f));
    ImGui::BeginChild("Sidebar", layout.sidebarSize, false, ImGuiWindowFlags_NoScrollbar);
    ImGui::TextDisabled("CURRENT VIEW");
    const bool carBuilderSelected = state.activeView == SidebarView::CarBuilder;
    const bool atmosphereSelected = state.activeView == SidebarView::Atmosphere;
    if (ImGui::Selectable("Car Builder", carBuilderSelected)) {
        state.activeView = SidebarView::CarBuilder;
    }
    if (ImGui::Selectable("Atmosphere", atmosphereSelected)) {
        state.activeView = SidebarView::Atmosphere;
    }
    ImGui::Separator();
    if (state.activeView == SidebarView::CarBuilder) {
        drawPartSelector(state, partsJson);
    } else {
        drawAtmospherePanel(state);
    }
    ImGui::EndChild();
    ImGui::PopStyleColor(4);

    if (state.showSimulation) {
        ImGui::SetCursorScreenPos(layout.simPos);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.15f, 0.19f, 0.95f));
        ImGui::BeginChild("Simulation", layout.simSize, false, ImGuiWindowFlags_NoScrollbar);
        ImGui::TextDisabled("SIMULATION");
        ImGui::Separator();
        drawDamagePanel(state);
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

    if (state.showControls) {
        ImGui::SetCursorScreenPos(layout.controlsPos);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.15f, 0.19f, 0.95f));
        ImGui::BeginChild("Controls", layout.controlsSize, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::TextDisabled("CONTROLS");
        ImGui::Separator();
        ImGui::Text("Left Mouse: Orbit");
        ImGui::Text("Scroll: Zoom");
        ImGui::Text("Use View menu for toggles");
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

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
    
    // Initialize atmosphere
    state.atmoProgram = createAtmosphereShader();
    if (bgfx::isValid(state.atmoProgram)) {
        state.atmosphere.init();
        state.atmosphere.setTimeOfDay(state.timeOfDay);
    } else {
        spdlog::warn("Failed to load atmosphere shader");
    }
    
    // Initialize ground plane
    state.groundProgram = createBasicShader();
    if (bgfx::isValid(state.groundProgram)) {
        state.groundPlane.init(50.0f, 50);
    } else {
        spdlog::warn("Failed to load ground shader");
    }

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
        state.frameTimeMs = dt * 1000.0f;

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
        if (state.autoRotate) {
            autoYaw += dt * 0.3f;
        }

        state.carBody.update(dt);

        renderer.beginFrame();

        auto winSize = window.getSize();
        float aspect = winSize.x / winSize.y;
        mat4 view = state.camera.getView();
        mat4 proj = state.camera.getProj(aspect);
        bgfx::setViewRect(0, 0, 0, static_cast<u16>(winSize.x), static_cast<u16>(winSize.y));

        // Set view transform for all 3D objects
        bgfx::setViewTransform(0, &view, &proj);
        
        // 1. Render atmosphere first (background, no depth write)
        if (bgfx::isValid(state.atmoProgram)) {
            state.atmosphere.render(0, state.atmoProgram, view, proj);
        }
        
        // 2. Render ground plane (lowered so car is clearly above it)
        if (state.showGround && bgfx::isValid(state.groundProgram)) {
            state.groundPlane.render(0, state.groundProgram, view, proj);
        }

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
        const EditorLayout layout = computeLayout();
        drawEditorShell(state, partsJson, layout);
        imguiRenderer.endFrame();
        imguiRenderer.render();

        renderer.endFrame();
    }

    state.atmosphere.destroy();
    state.groundPlane.destroy();
    if (bgfx::isValid(state.atmoProgram)) {
        bgfx::destroy(state.atmoProgram);
    }
    if (bgfx::isValid(state.groundProgram)) {
        bgfx::destroy(state.groundProgram);
    }
    bgfx::destroy(program);
    imguiRenderer.shutdown();

    return 0;
}
