#include "editor_ui.hpp"

#include <imgui.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <string>

namespace car_editor {

namespace {

const char* getActiveViewLabel(SidebarView view) {
    switch (view) {
        case SidebarView::CarBuilder: return "Car Builder";
        case SidebarView::VehicleInfo: return "Vehicle Info";
        case SidebarView::Atmosphere: return "Atmosphere";
        default: return "Editor";
    }
}

void drawMetricRow(const char* label, const std::string& value) {
    ImGui::TextDisabled("%s", label);
    ImGui::SameLine(160.0f);
    ImGui::Text("%s", value.c_str());
}

std::string formatMeters(float value) {
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%.2f m", value);
    return buffer;
}

std::string formatScalar(float value, const char* suffix) {
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%.2f%s", value, suffix);
    return buffer;
}

std::string describeVehicle(const EditorState& state) {
    std::string shape = state.currentDef.chassisVariant == "hatchback_b" ? "Hatchback" : "Sedan";
    std::string wheels = state.currentDef.wheelVariants[0] == "wheel_offroad"
        ? "off-road package"
        : (state.currentDef.wheelVariants[0] == "wheel_sport" ? "sport package" : "standard package");
    std::string aero = state.currentDef.spoilerVariant.empty() ? "clean rear deck" : "rear aero package";
    return shape + " build with " + wheels + " and " + aero + ".";
}

void drawRatingBar(const char* label, float value, const ImVec4& color) {
    ImGui::TextDisabled("%s", label);
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
    ImGui::ProgressBar(glm::clamp(value, 0.0f, 1.0f), ImVec2(-1.0f, 8.0f), "");
    ImGui::PopStyleColor();
}

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

void drawVehicleInfoPanel(EditorState& state) {
    ImGui::TextDisabled("Identity");
    ImGui::InputText("Car Name", state.vehicleName, IM_ARRAYSIZE(state.vehicleName));
    ImGui::Separator();

    const viber::VehicleParams params = state.previewVehicle
        ? state.previewVehicle->getParams()
        : viber::VehicleParams{};
    const viber::VehicleState liveState = state.previewVehicle
        ? state.previewVehicle->getState()
        : viber::VehicleState{};

    const float wheelbase = std::abs(params.wheelPositions[0].z - params.wheelPositions[2].z);
    const float trackWidth = std::abs(params.wheelPositions[0].x - params.wheelPositions[1].x);
    const float estPowerHp = params.maxEngineForce * 0.1f;
    const float powerRating = params.maxEngineForce / 4000.0f;
    const float gripRating = params.wheelFriction / 2.2f;
    const float agilityRating = (glm::degrees(params.maxSteeringAngle) / 40.0f) + (1400.0f - params.mass) / 1200.0f;
    const float durabilityRating = 1.0f - state.damageModel.getTotalDamage();

    ImGui::Text("%s", state.vehicleName);
    ImGui::TextDisabled("%s", describeVehicle(state).c_str());
    ImGui::Spacing();

    if (ImGui::BeginTable("VehicleSummary", 3, ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableNextColumn();
        ImGui::TextDisabled("CLASS");
        ImGui::Text("%s", state.currentDef.chassisVariant == "hatchback_b" ? "Urban Hatch" : "Road Sedan");
        ImGui::TableNextColumn();
        ImGui::TextDisabled("SEED");
        ImGui::Text("%u", state.seedInput);
        ImGui::TableNextColumn();
        ImGui::TextDisabled("LIVE");
        ImGui::Text("%d km/h", static_cast<int>(liveState.speedKmh));
        ImGui::EndTable();
    }

    ImGui::Separator();
    ImGui::TextDisabled("Ratings");
    drawRatingBar("Power", powerRating, ImVec4(0.82f, 0.24f, 0.18f, 1.0f));
    drawRatingBar("Grip", gripRating, ImVec4(0.26f, 0.62f, 0.40f, 1.0f));
    drawRatingBar("Agility", agilityRating, ImVec4(0.28f, 0.52f, 0.82f, 1.0f));
    drawRatingBar("Durability", durabilityRating, ImVec4(0.78f, 0.68f, 0.28f, 1.0f));

    ImGui::Separator();
    ImGui::TextDisabled("Package");
    drawMetricRow("Platform", state.currentDef.chassisVariant);
    drawMetricRow("Hood", state.currentDef.hoodVariant);
    drawMetricRow("Front Bumper", state.currentDef.bumperFrontVariant);
    drawMetricRow("Spoiler", state.currentDef.spoilerVariant.empty() ? "none" : state.currentDef.spoilerVariant);
    drawMetricRow("Wheel Package", state.currentDef.wheelVariants[0]);

    ImGui::Separator();
    ImGui::TextDisabled("Specs");
    drawMetricRow("Mass", std::to_string(static_cast<int>(params.mass)) + " kg");
    drawMetricRow("Length", formatMeters(params.chassisLength));
    drawMetricRow("Width", formatMeters(params.chassisWidth));
    drawMetricRow("Height", formatMeters(params.chassisHeight));
    drawMetricRow("Wheelbase", formatMeters(wheelbase));
    drawMetricRow("Track Width", formatMeters(trackWidth));
    drawMetricRow("Wheel Radius", formatMeters(params.wheelRadius));

    ImGui::Separator();
    ImGui::TextDisabled("Performance");
    drawMetricRow("Engine Force", std::to_string(static_cast<int>(params.maxEngineForce)) + " N");
    drawMetricRow("Brake Force", std::to_string(static_cast<int>(params.maxBrakeForce)) + " N");
    drawMetricRow("Est. Power", std::to_string(static_cast<int>(estPowerHp)) + " hp");
    drawMetricRow("Steering Lock", std::to_string(static_cast<int>(glm::degrees(params.maxSteeringAngle))) + " deg");
    drawMetricRow("Grip", formatScalar(params.wheelFriction, ""));

    ImGui::Separator();
    ImGui::TextDisabled("Condition");
    drawMetricRow("Damage", std::to_string(static_cast<int>(state.damageModel.getTotalDamage() * 100.0f)) + " %");
    drawMetricRow("Primary Paint", formatScalar(state.currentDef.primaryColor.r, "") + ", " +
        formatScalar(state.currentDef.primaryColor.g, "") + ", " +
        formatScalar(state.currentDef.primaryColor.b, ""));
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
    const EditorStyle& style = state.style;
    ImGui::SetCursorScreenPos(layout.topBarPos);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, style.topBarBg);
    ImGui::PushStyleColor(ImGuiCol_Button, style.button);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, style.buttonHovered);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, style.buttonActive);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, style.popupBg);
    ImGui::PushStyleColor(ImGuiCol_Text, style.text);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(style.topBarPaddingX, style.topBarPaddingY));
    ImGui::BeginChild("TopBar", layout.topBarSize, true,
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse);

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
        if (ImGui::MenuItem("Vehicle Info", nullptr, state.activeView == SidebarView::VehicleInfo)) {
            state.activeView = SidebarView::VehicleInfo;
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

    const char* activeLabel = getActiveViewLabel(state.activeView);
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
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(6);
}

void drawViewportFrame(EditorState& state, const EditorLayout& layout) {
    const EditorStyle& style = state.style;
    ImGui::SetCursorScreenPos(layout.viewportPos);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, style.viewportBg);
    ImGui::PushStyleColor(ImGuiCol_Border, style.viewportBorder);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(style.viewportPaddingX, style.viewportPaddingY));
    ImGui::BeginChild("ViewportFrame", layout.viewportSize, true,
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse);
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
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
}

} // namespace

void drawEditorShell(EditorState& state, const std::string& partsJson, const EditorLayout& layout) {
    const EditorStyle& style = state.style;
    ImGui::SetNextWindowPos(layout.boardPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(layout.boardSize, ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, style.boardBg);
    ImGui::PushStyleColor(ImGuiCol_Border, style.boardBorder);
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
    ImGui::PushStyleColor(ImGuiCol_ChildBg, style.sidebarBg);
    ImGui::PushStyleColor(ImGuiCol_Header, style.header);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, style.headerHovered);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, style.headerActive);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(style.sidebarPaddingX, style.sidebarPaddingY));
    ImGui::BeginChild("Sidebar", layout.sidebarSize, true, ImGuiWindowFlags_NoScrollbar);
    ImGui::TextDisabled("CURRENT VIEW");
    if (ImGui::Selectable("Car Builder", state.activeView == SidebarView::CarBuilder)) {
        state.activeView = SidebarView::CarBuilder;
    }
    if (ImGui::Selectable("Vehicle Info", state.activeView == SidebarView::VehicleInfo)) {
        state.activeView = SidebarView::VehicleInfo;
    }
    if (ImGui::Selectable("Atmosphere", state.activeView == SidebarView::Atmosphere)) {
        state.activeView = SidebarView::Atmosphere;
    }
    ImGui::Separator();
    if (state.activeView == SidebarView::CarBuilder) {
        drawPartSelector(state, partsJson);
    } else if (state.activeView == SidebarView::VehicleInfo) {
        drawVehicleInfoPanel(state);
    } else {
        drawAtmospherePanel(state);
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);

    if (state.showSimulation) {
        ImGui::SetCursorScreenPos(layout.simPos);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, style.panelBg);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(style.panelPaddingX, style.panelPaddingY));
        ImGui::BeginChild("Simulation", layout.simSize, true, ImGuiWindowFlags_NoScrollbar);
        ImGui::TextDisabled("SIMULATION");
        ImGui::Separator();
        drawDamagePanel(state);
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }

    if (state.showControls) {
        ImGui::SetCursorScreenPos(layout.controlsPos);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, style.panelBg);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(style.panelPaddingX, style.panelPaddingY));
        ImGui::BeginChild("Controls", layout.controlsSize, true,
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::TextDisabled("CONTROLS");
        ImGui::Separator();
        ImGui::Text("Left Mouse: Orbit");
        ImGui::Text("Scroll: Zoom");
        ImGui::Text("Use View menu for toggles");
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }

    ImGui::End();
}

} // namespace car_editor
