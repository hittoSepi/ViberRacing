#include "editor_ui.hpp"

#include <imgui.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>
#include <string>

namespace car_editor {

namespace {

void drawPartSelector(EditorState& state, const std::string& partsJson) {
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
    {
        const char* items[] = {"hood_flat", "hood_scooped"};
        int idx = def.hoodVariant == "hood_flat" ? 0 : 1;
        if (ImGui::Combo("Hood", &idx, items, 2)) {
            def.hoodVariant = items[idx];
            changed = true;
        }
    }
    {
        const char* items[] = {"bumper_front_standard", "bumper_front_sport"};
        int idx = def.bumperFrontVariant == "bumper_front_standard" ? 0 : 1;
        if (ImGui::Combo("Front Bumper", &idx, items, 2)) {
            def.bumperFrontVariant = items[idx];
            changed = true;
        }
    }
    {
        const char* items[] = {"(none)", "spoiler_small", "spoiler_large"};
        int idx = def.spoilerVariant.empty() ? 0 : (def.spoilerVariant == "spoiler_small" ? 1 : 2);
        if (ImGui::Combo("Spoiler", &idx, items, 3)) {
            def.spoilerVariant = idx == 0 ? "" : items[idx];
            changed = true;
        }
    }
    {
        const char* items[] = {"wheel_standard", "wheel_sport", "wheel_offroad"};
        auto it = std::find(std::begin(items), std::end(items), def.wheelVariants[0]);
        int idx = static_cast<int>(it - std::begin(items));
        if (idx < 0 || idx >= 3) {
            idx = 0;
        }
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
        def = viber::CarBody::generateFromSeed(state.seedInput, partsJson);
        changed = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Export")) {
        def.toJson("car_export.json");
        spdlog::info("Car exported to car_export.json");
    }

    if (changed) {
        state.carBody.setDefinition(def);
    }
}

void drawDamagePanel(EditorState& state) {
    ImGui::TextDisabled("Damage Test");
    ImGui::SliderFloat("Impulse", &state.simImpulse, 0.0f, 1000.0f);
    ImGui::SliderInt("Target Wheel", &state.simWheelIndex, 0, 3);

    const viber::vec3 wheelPositions[4] = {
        { 0.9f, -0.1f,  1.4f}, {-0.9f, -0.1f,  1.4f},
        { 0.9f, -0.1f, -1.3f}, {-0.9f, -0.1f, -1.3f}
    };

    if (ImGui::Button("Hit Wheel")) {
        state.damageModel.processImpact(state.simImpulse, wheelPositions[state.simWheelIndex]);
    }
    ImGui::SameLine();
    if (ImGui::Button("Hit Front")) {
        state.damageModel.processImpact(state.simImpulse, viber::vec3{0.0f, 0.1f, 2.0f});
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
        state.damageModel.reset();
    }

    ImGui::Text("Total damage: %.1f%%", state.damageModel.getTotalDamage() * 100.0f);
}

void drawAtmospherePanel(EditorState& state) {
    if (ImGui::SliderFloat("Time Of Day", &state.timeOfDay, 0.0f, 1.0f)) {
        state.atmosphere.setTimeOfDay(state.timeOfDay);
    }

    viber::AtmosphereSettings& atmo = state.atmosphere.settings();

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

void drawTopBar(EditorState& state, const std::string& partsJson, const EditorLayout& layout) {
    ImGui::SetCursorScreenPos(layout.topBarPos);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.13f, 0.17f, 0.22f, 0.96f));
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
            applyCarDefinition(state, viber::CarBody::generateFromSeed(state.seedInput, partsJson));
        }
        if (ImGui::MenuItem("Reset Default")) {
            applyCarDefinition(state, viber::CarDefinition::makeDefault());
        }
        if (ImGui::MenuItem("Load car_export.json")) {
            applyCarDefinition(state, viber::CarDefinition::fromJson("car_export.json"));
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
    const float statusX = std::max(160.0f, layout.topBarSize.x - 360.0f);
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

void drawViewportFrame(EditorState& state, const EditorLayout& layout) {
    ImGui::SetCursorScreenPos(layout.viewportPos);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.07f, 0.10f, 0.16f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.35f, 0.47f, 0.60f, 0.45f));
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

} // namespace

void drawEditorShell(EditorState& state, const std::string& partsJson, const EditorLayout& layout) {
    ImGui::SetNextWindowPos(layout.boardPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(layout.boardSize, ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.15f, 0.23f, 0.98f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.24f, 0.34f, 0.50f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Board", nullptr,
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar);
    ImGui::PopStyleVar(3);
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
    if (ImGui::Selectable("Car Builder", state.activeView == SidebarView::CarBuilder)) {
        state.activeView = SidebarView::CarBuilder;
    }
    if (ImGui::Selectable("Atmosphere", state.activeView == SidebarView::Atmosphere)) {
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

} // namespace car_editor
