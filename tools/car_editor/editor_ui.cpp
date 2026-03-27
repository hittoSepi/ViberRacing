#include "editor_ui.hpp"

#include <imgui.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <string>

namespace car_editor {

namespace {

const char* getWorkspaceLabel(EditorWorkspace workspace) {
    switch (workspace) {
        case EditorWorkspace::Vehicles: return "Vehicles";
        case EditorWorkspace::Tracks: return "Tracks";
        default: return "Editor";
    }
}

const char* getActiveViewLabel(SidebarView view) {
    switch (view) {
        case SidebarView::CarBuilder: return "Car Builder";
        case SidebarView::VehicleInfo: return "Vehicle Info";
        case SidebarView::Atmosphere: return "Atmosphere";
        case SidebarView::TrackLayout: return "Track Layout";
        case SidebarView::TrackInfo: return "Track Info";
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

void drawTrackLayoutPanel(EditorState& state) {
    ImGui::TextDisabled("Track");
    ImGui::InputText("Track Name", state.trackName, IM_ARRAYSIZE(state.trackName));
    ImGui::InputText("Track File", state.trackFilePath, IM_ARRAYSIZE(state.trackFilePath));

    float roadWidth = state.track.getRoadWidth();
    if (ImGui::SliderFloat("Road Width", &roadWidth, 6.0f, 30.0f)) {
        state.track.setRoadWidth(roadWidth);
        state.track.generateFromSpline(state.track.getSpline());
        rebuildTrackPreview(state);
    }

    bool closedLoop = state.track.getSpline().isClosedLoop();
    if (ImGui::Checkbox("Closed Loop", &closedLoop)) {
        viber::Spline spline(state.track.getSpline().getControlPoints());
        if (closedLoop) {
            spline.closeLoop();
        }
        state.track.generateFromSpline(spline);
        state.trackClosedLoop = closedLoop;
        rebuildTrackPreview(state);
    }

    if (ImGui::Button("New Track")) {
        state.track = viber::Track{};
        state.track.setRoadWidth(roadWidth);
        state.trackClosedLoop = true;
        state.tunnelHoleA = 0;
        state.tunnelHoleB = 1;
        syncTrackEditorFieldsFromTrack(state);
        rebuildTrackPreview(state);
        resetTrackCamera(state);
    }
    ImGui::SameLine();
    if (ImGui::Button("Load")) {
        if (state.track.loadFromFile(state.trackFilePath)) {
            state.trackClosedLoop = state.track.getSpline().isClosedLoop();
            state.tunnelHoleA = 0;
            state.tunnelHoleB = state.track.holes().size() >= 2 ? 1 : 0;
            syncTrackEditorFieldsFromTrack(state);
            rebuildTrackPreview(state);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Save")) {
        state.track.setName(state.trackName);
        auto& terrain = state.track.terrainSettings();
        terrain.biome = state.terrainBiome;
        terrain.heightmapEditPath = state.terrainHeightmapEditPath;
        terrain.holeMaskPath = state.terrainHoleMaskPath;
        state.track.saveToFile(state.trackFilePath);
    }

    if (ImGui::Button("Add Point")) {
        state.track.getSpline().addControlPoint(state.camera.target);
        if (state.trackClosedLoop) {
            state.track.getSpline().closeLoop();
        }
        state.track.generateFromSpline(state.track.getSpline());
        rebuildTrackPreview(state);
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        state.track.getSpline().clear();
        state.track.generateFromSpline(state.track.getSpline());
        rebuildTrackPreview(state);
    }

    ImGui::Separator();
    ImGui::TextDisabled("Terrain");
    auto& terrain = state.track.terrainSettings();
    ImGui::InputScalar("Terrain Seed", ImGuiDataType_U32, &terrain.seed);
    ImGui::SliderInt("Base Resolution", &terrain.baseResolution, 128, 2048);
    ImGui::SliderFloat("World Size", &terrain.worldSize, 128.0f, 2048.0f);
    ImGui::SliderFloat("Max Height", &terrain.maxHeight, 16.0f, 256.0f);
    ImGui::InputText("Biome", state.terrainBiome, IM_ARRAYSIZE(state.terrainBiome));
    ImGui::InputText("Heightmap Edit", state.terrainHeightmapEditPath, IM_ARRAYSIZE(state.terrainHeightmapEditPath));
    ImGui::InputText("Hole Mask", state.terrainHoleMaskPath, IM_ARRAYSIZE(state.terrainHoleMaskPath));

    ImGui::Separator();
    ImGui::TextDisabled("Holes");
    ImGui::SliderFloat("Hole Radius", &state.trackHoleRadius, 4.0f, 40.0f);
    ImGui::SliderFloat("Hole Depth", &state.trackHoleDepth, 4.0f, 40.0f);
    if (ImGui::Button("Add Hole At Camera Target")) {
        viber::TrackHole hole;
        hole.position = state.camera.target;
        hole.radius = state.trackHoleRadius;
        hole.depth = state.trackHoleDepth;
        state.track.addHole(hole);
        if (state.track.holes().size() == 1) {
            state.tunnelHoleA = 0;
            state.tunnelHoleB = 0;
        } else if (state.track.holes().size() == 2) {
            state.tunnelHoleB = 1;
        }
        rebuildTrackPreview(state);
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear Holes")) {
        state.track.clearHoles();
        state.tunnelHoleA = 0;
        state.tunnelHoleB = 1;
        rebuildTrackPreview(state);
    }

    const int holeCount = static_cast<int>(state.track.holes().size());
    if (holeCount > 0) {
        state.tunnelHoleA = glm::clamp(state.tunnelHoleA, 0, holeCount - 1);
        state.tunnelHoleB = glm::clamp(state.tunnelHoleB, 0, holeCount - 1);
    } else {
        state.tunnelHoleA = 0;
        state.tunnelHoleB = 0;
    }
    ImGui::InputInt("Hole A", &state.tunnelHoleA);
    ImGui::InputInt("Hole B", &state.tunnelHoleB);
    state.tunnelHoleA = holeCount > 0 ? glm::clamp(state.tunnelHoleA, 0, holeCount - 1) : 0;
    state.tunnelHoleB = holeCount > 0 ? glm::clamp(state.tunnelHoleB, 0, holeCount - 1) : 0;

    ImGui::Separator();
    ImGui::TextDisabled("Tunnel Generator");
    ImGui::InputScalar("Tunnel Seed", ImGuiDataType_U32, &state.tunnelSeed);
    if (ImGui::Button("Connect Two Holes")) {
        if (state.track.generateTunnelBetweenHoles(
                static_cast<size_t>(state.tunnelHoleA),
                static_cast<size_t>(state.tunnelHoleB),
                state.tunnelSeed)) {
            rebuildTrackPreview(state);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Connect Last Two")) {
        if (holeCount >= 2 &&
            state.track.generateTunnelBetweenHoles(
                static_cast<size_t>(holeCount - 2),
                static_cast<size_t>(holeCount - 1),
                state.tunnelSeed)) {
            state.tunnelHoleA = holeCount - 2;
            state.tunnelHoleB = holeCount - 1;
            rebuildTrackPreview(state);
        }
    }

    ImGui::Separator();
    ImGui::TextDisabled("Edit");
    ImGui::TextWrapped("Use camera target as placement point. Add road points or holes from the current orbit focus, then connect two holes with a seeded tunnel generator.");
}

void drawTrackInfoPanel(EditorState& state) {
    const auto& spline = state.track.getSpline();
    drawMetricRow("Track Name", state.trackName);
    drawMetricRow("Control Points", std::to_string(static_cast<int>(spline.getNumControlPoints())));
    drawMetricRow("Length", formatMeters(state.track.getLength()));
    drawMetricRow("Road Width", formatMeters(state.track.getRoadWidth()));
    drawMetricRow("Loop", spline.isClosedLoop() ? "closed" : "open");
    drawMetricRow("Terrain Seed", std::to_string(state.track.terrainSettings().seed));
    drawMetricRow("World Size", formatMeters(state.track.terrainSettings().worldSize));
    drawMetricRow("Holes", std::to_string(static_cast<int>(state.track.holes().size())));
    drawMetricRow("Tunnels", std::to_string(static_cast<int>(state.track.tunnels().size())));
    drawMetricRow("File", state.trackFilePath);
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
        if (state.activeWorkspace == EditorWorkspace::Vehicles) {
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
        } else {
            if (ImGui::MenuItem("New Track")) {
                state.track = viber::Track{};
                state.trackClosedLoop = true;
                syncTrackEditorFieldsFromTrack(state);
                rebuildTrackPreview(state);
            }
            if (ImGui::MenuItem("Load Track")) {
                if (state.track.loadFromFile(state.trackFilePath)) {
                    state.trackClosedLoop = state.track.getSpline().isClosedLoop();
                    state.tunnelHoleA = 0;
                    state.tunnelHoleB = state.track.holes().size() >= 2 ? 1 : 0;
                    syncTrackEditorFieldsFromTrack(state);
                    rebuildTrackPreview(state);
                }
            }
            if (ImGui::MenuItem("Save Track")) {
                state.track.setName(state.trackName);
                auto& terrain = state.track.terrainSettings();
                terrain.biome = state.terrainBiome;
                terrain.heightmapEditPath = state.terrainHeightmapEditPath;
                terrain.holeMaskPath = state.terrainHoleMaskPath;
                state.track.saveToFile(state.trackFilePath);
            }
        }
        ImGui::EndPopup();
    }

    ImGui::SameLine();
    if (ImGui::Button(getWorkspaceLabel(state.activeWorkspace))) {
        ImGui::OpenPopup("WorkspaceMenu");
    }
    if (ImGui::BeginPopup("WorkspaceMenu")) {
        if (ImGui::MenuItem("Vehicles", nullptr, state.activeWorkspace == EditorWorkspace::Vehicles)) {
            state.activeWorkspace = EditorWorkspace::Vehicles;
            state.activeView = SidebarView::CarBuilder;
            resetCamera(state);
        }
        if (ImGui::MenuItem("Tracks", nullptr, state.activeWorkspace == EditorWorkspace::Tracks)) {
            state.activeWorkspace = EditorWorkspace::Tracks;
            state.activeView = SidebarView::TrackLayout;
            resetTrackCamera(state);
        }
        ImGui::EndPopup();
    }

    ImGui::SameLine();
    if (ImGui::Button("View")) {
        ImGui::OpenPopup("ViewMenu");
    }
    if (ImGui::BeginPopup("ViewMenu")) {
        if (state.activeWorkspace == EditorWorkspace::Vehicles) {
            if (ImGui::MenuItem("Car Builder", nullptr, state.activeView == SidebarView::CarBuilder)) {
                state.activeView = SidebarView::CarBuilder;
            }
            if (ImGui::MenuItem("Vehicle Info", nullptr, state.activeView == SidebarView::VehicleInfo)) {
                state.activeView = SidebarView::VehicleInfo;
            }
            if (ImGui::MenuItem("Atmosphere", nullptr, state.activeView == SidebarView::Atmosphere)) {
                state.activeView = SidebarView::Atmosphere;
            }
        } else {
            if (ImGui::MenuItem("Track Layout", nullptr, state.activeView == SidebarView::TrackLayout)) {
                state.activeView = SidebarView::TrackLayout;
            }
            if (ImGui::MenuItem("Track Info", nullptr, state.activeView == SidebarView::TrackInfo)) {
                state.activeView = SidebarView::TrackInfo;
            }
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
    const std::string status = std::string(getWorkspaceLabel(state.activeWorkspace)) + " / " + activeLabel +
        "  |  Seed " + std::to_string(state.seedInput) +
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
        ImGui::Text("Use the workspace switcher to move between vehicles and tracks.");
        ImGui::Text("File menu handles import/export for the active workspace.");
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
    if (state.activeWorkspace == EditorWorkspace::Vehicles) {
        ImGui::Text("Time %.0f%%", state.timeOfDay * 100.0f);
    } else {
        ImGui::Text("Track points %d", static_cast<int>(state.track.getSpline().getNumControlPoints()));
    }
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
    if (state.activeWorkspace == EditorWorkspace::Vehicles) {
        if (ImGui::Selectable("Car Builder", state.activeView == SidebarView::CarBuilder)) {
            state.activeView = SidebarView::CarBuilder;
        }
        if (ImGui::Selectable("Vehicle Info", state.activeView == SidebarView::VehicleInfo)) {
            state.activeView = SidebarView::VehicleInfo;
        }
        if (ImGui::Selectable("Atmosphere", state.activeView == SidebarView::Atmosphere)) {
            state.activeView = SidebarView::Atmosphere;
        }
    } else {
        if (ImGui::Selectable("Track Layout", state.activeView == SidebarView::TrackLayout)) {
            state.activeView = SidebarView::TrackLayout;
        }
        if (ImGui::Selectable("Track Info", state.activeView == SidebarView::TrackInfo)) {
            state.activeView = SidebarView::TrackInfo;
        }
    }
    ImGui::Separator();
    if (state.activeWorkspace == EditorWorkspace::Vehicles) {
        if (state.activeView == SidebarView::CarBuilder) {
            drawPartSelector(state, partsJson);
        } else if (state.activeView == SidebarView::VehicleInfo) {
            drawVehicleInfoPanel(state);
        } else {
            drawAtmospherePanel(state);
        }
    } else {
        if (state.activeView == SidebarView::TrackLayout) {
            drawTrackLayoutPanel(state);
        } else {
            drawTrackInfoPanel(state);
        }
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
