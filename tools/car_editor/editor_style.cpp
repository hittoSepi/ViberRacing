#include "editor_style.hpp"

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <fstream>

namespace car_editor {

namespace {

using json = nlohmann::json;

float getFloat(const json& node, const char* key, float fallback) {
    const auto it = node.find(key);
    return it != node.end() && it->is_number() ? it->get<float>() : fallback;
}

ImVec4 getColor(const json& node, const char* key, const ImVec4& fallback) {
    const auto it = node.find(key);
    if (it == node.end() || !it->is_array() || it->size() != 4) {
        return fallback;
    }

    return ImVec4(
        (*it)[0].get<float>(),
        (*it)[1].get<float>(),
        (*it)[2].get<float>(),
        (*it)[3].get<float>());
}

} // namespace

EditorStyle loadEditorStyle(const std::string& path) {
    EditorStyle style;

    std::ifstream file(path);
    if (!file.is_open()) {
        spdlog::warn("EditorStyle: cannot open {}, using defaults", path);
        return style;
    }

    try {
        const json root = json::parse(file);
        const json& layout = root.contains("layout") ? root["layout"] : json::object();
        const json& colors = root.contains("colors") ? root["colors"] : json::object();

        style.shellMargin = getFloat(layout, "shellMargin", style.shellMargin);
        style.innerMargin = getFloat(layout, "innerMargin", style.innerMargin);
        style.gap = getFloat(layout, "gap", style.gap);
        style.minBoardWidth = getFloat(layout, "minBoardWidth", style.minBoardWidth);
        style.minBoardHeight = getFloat(layout, "minBoardHeight", style.minBoardHeight);
        style.sidebarWidthRatio = getFloat(layout, "sidebarWidthRatio", style.sidebarWidthRatio);
        style.sidebarMinWidth = getFloat(layout, "sidebarMinWidth", style.sidebarMinWidth);
        style.sidebarMaxWidth = getFloat(layout, "sidebarMaxWidth", style.sidebarMaxWidth);
        style.topBarHeight = getFloat(layout, "topBarHeight", style.topBarHeight);
        style.simHeightRatio = getFloat(layout, "simHeightRatio", style.simHeightRatio);
        style.simMinHeight = getFloat(layout, "simMinHeight", style.simMinHeight);
        style.simMaxHeight = getFloat(layout, "simMaxHeight", style.simMaxHeight);
        style.controlsWidth = getFloat(layout, "controlsWidth", style.controlsWidth);
        style.controlsHeight = getFloat(layout, "controlsHeight", style.controlsHeight);
        style.controlsInset = getFloat(layout, "controlsInset", style.controlsInset);
        style.panelPaddingX = getFloat(layout, "panelPaddingX", style.panelPaddingX);
        style.panelPaddingY = getFloat(layout, "panelPaddingY", style.panelPaddingY);
        style.sidebarPaddingX = getFloat(layout, "sidebarPaddingX", style.sidebarPaddingX);
        style.sidebarPaddingY = getFloat(layout, "sidebarPaddingY", style.sidebarPaddingY);
        style.topBarPaddingX = getFloat(layout, "topBarPaddingX", style.topBarPaddingX);
        style.topBarPaddingY = getFloat(layout, "topBarPaddingY", style.topBarPaddingY);
        style.viewportPaddingX = getFloat(layout, "viewportPaddingX", style.viewportPaddingX);
        style.viewportPaddingY = getFloat(layout, "viewportPaddingY", style.viewportPaddingY);

        style.boardBg = getColor(colors, "boardBg", style.boardBg);
        style.boardBorder = getColor(colors, "boardBorder", style.boardBorder);
        style.topBarBg = getColor(colors, "topBarBg", style.topBarBg);
        style.sidebarBg = getColor(colors, "sidebarBg", style.sidebarBg);
        style.panelBg = getColor(colors, "panelBg", style.panelBg);
        style.viewportBg = getColor(colors, "viewportBg", style.viewportBg);
        style.viewportBorder = getColor(colors, "viewportBorder", style.viewportBorder);
        style.button = getColor(colors, "button", style.button);
        style.buttonHovered = getColor(colors, "buttonHovered", style.buttonHovered);
        style.buttonActive = getColor(colors, "buttonActive", style.buttonActive);
        style.popupBg = getColor(colors, "popupBg", style.popupBg);
        style.text = getColor(colors, "text", style.text);
        style.header = getColor(colors, "header", style.header);
        style.headerHovered = getColor(colors, "headerHovered", style.headerHovered);
        style.headerActive = getColor(colors, "headerActive", style.headerActive);
    } catch (const std::exception& e) {
        spdlog::error("EditorStyle: failed to parse {}: {}", path, e.what());
    }

    return style;
}

} // namespace car_editor
