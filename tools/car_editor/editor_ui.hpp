#pragma once

#include "editor_state.hpp"

#include <string>

namespace car_editor {

void drawEditorShell(EditorState& state, const std::string& partsJson, const EditorLayout& layout);
const char* getTrackToolLabel(TrackTool tool);

} // namespace car_editor
