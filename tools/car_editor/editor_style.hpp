#pragma once

#include <imgui.h>

#include <string>

namespace car_editor {

struct EditorStyle {
    float shellMargin = 0.0f;
    float innerMargin = 10.0f;
    float gap = 10.0f;
    float minBoardWidth = 640.0f;
    float minBoardHeight = 420.0f;
    float sidebarWidthRatio = 0.23f;
    float sidebarMinWidth = 230.0f;
    float sidebarMaxWidth = 300.0f;
    float topBarHeight = 52.0f;
    float simHeightRatio = 0.28f;
    float simMinHeight = 150.0f;
    float simMaxHeight = 210.0f;
    float controlsWidth = 220.0f;
    float controlsHeight = 110.0f;
    float controlsInset = 12.0f;

    ImVec4 boardBg{0.10f, 0.15f, 0.23f, 0.0f};
    ImVec4 boardBorder{0.24f, 0.34f, 0.50f, 0.0f};
    ImVec4 topBarBg{0.13f, 0.17f, 0.22f, 0.96f};
    ImVec4 sidebarBg{0.12f, 0.15f, 0.19f, 0.95f};
    ImVec4 panelBg{0.12f, 0.15f, 0.19f, 0.95f};
    ImVec4 viewportBg{0.05f, 0.07f, 0.10f, 0.0f};
    ImVec4 viewportBorder{0.35f, 0.47f, 0.60f, 0.45f};
    ImVec4 button{0.20f, 0.27f, 0.35f, 1.0f};
    ImVec4 buttonHovered{0.28f, 0.37f, 0.48f, 1.0f};
    ImVec4 buttonActive{0.24f, 0.32f, 0.42f, 1.0f};
    ImVec4 popupBg{0.10f, 0.13f, 0.17f, 0.98f};
    ImVec4 text{0.92f, 0.94f, 0.96f, 1.0f};
    ImVec4 header{0.20f, 0.29f, 0.39f, 0.90f};
    ImVec4 headerHovered{0.28f, 0.39f, 0.52f, 0.95f};
    ImVec4 headerActive{0.24f, 0.34f, 0.46f, 1.0f};
};

EditorStyle loadEditorStyle(const std::string& path);

} // namespace car_editor
