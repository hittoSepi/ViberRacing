#pragma once

#include "editor_state.hpp"

#include <string>

namespace car_editor {

void drawEditorShell(EditorState& state, const std::string& partsJson, const EditorLayout& layout);

} // namespace car_editor
