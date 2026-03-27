#pragma once

#include "editor_style.hpp"
#include "game/entities/car_body.hpp"
#include "game/entities/car_damage.hpp"
#include "game/entities/atmosphere.hpp"
#include "game/entities/ground_plane.hpp"
#include "physics/world.hpp"
#include "physics/vehicle.hpp"

#include <bgfx/bgfx.h>
#include <imgui.h>

#include <memory>

namespace car_editor {

enum class SidebarView {
    CarBuilder = 0,
    VehicleInfo,
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

struct OrbitCamera {
    float yaw = 0.3f;
    float pitch = 0.35f;
    float radius = 8.0f;
    viber::vec3 target{0.0f, 0.5f, 0.0f};

    viber::mat4 getView() const;
    viber::mat4 getProj(float aspect) const;
    void orbit(float dx, float dy);
    void zoom(float delta);
};

struct EditorState {
    viber::CarBody carBody;
    viber::CarDefinition currentDef = viber::CarDefinition::makeDefault();
    viber::u32 seedInput = 42;
    char vehicleName[64] = "Prototype 42";

    viber::PhysicsWorld physicsWorld;
    std::unique_ptr<viber::Vehicle> previewVehicle;

    OrbitCamera camera;
    bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle previewTint = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle previewLightDir = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle previewViewPos = BGFX_INVALID_HANDLE;

    bool dragging = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;

    float simImpulse = 200.0f;
    int simWheelIndex = 0;

    viber::DamageModel damageModel;
    viber::Atmosphere atmosphere;
    bgfx::ProgramHandle atmoProgram = BGFX_INVALID_HANDLE;
    float timeOfDay = 0.34f;

    viber::GroundPlane groundPlane;
    bgfx::ProgramHandle groundProgram = BGFX_INVALID_HANDLE;

    SidebarView activeView = SidebarView::CarBuilder;
    bool autoRotate = true;
    bool showGround = true;
    bool showSimulation = true;
    bool showControls = true;
    bool showHelp = false;
    float frameTimeMs = 0.0f;
    EditorStyle style;
};

EditorLayout computeLayout(const ImVec2& display, const EditorStyle& style);
void applyCarDefinition(EditorState& state, const viber::CarDefinition& def);
void resetCamera(EditorState& state);

} // namespace car_editor
