#include "editor_state.hpp"
#include "game/game.hpp"
#include "core/window.hpp"
#include "core/input.hpp"
#include "renderer/renderer.hpp"
#include <imgui.h>

namespace viber {

EditorState::EditorState(Game& game)
    : GameState(game)
{
}

void EditorState::init() {
    m_editor = std::make_unique<TrackEditor>();
}

void EditorState::shutdown() {
    m_editor.reset();
}

void EditorState::onEnter() {
    auto size = m_game.getWindow().getSize();
    m_camera.setPerspective(60.0f * DEG_TO_RAD, size.x / size.y, 0.1f, 2000.0f);
    
    m_editor->init();
    
    m_cameraYaw = 0.0f;
    m_cameraPitch = -30.0f * DEG_TO_RAD;
    m_cameraDistance = 50.0f;
    m_cameraTarget = vec3(0.0f);
}

void EditorState::onExit() {
    m_editor->shutdown();
}

void EditorState::update(float deltaTime) {
    handleInput();
    updateCamera(deltaTime);
    
    m_editor->update(deltaTime);
}

void EditorState::render() {
    auto& renderer = m_game.getRenderer();
    
    renderer.beginFrame();
    renderer.setViewTransform(m_camera);
    
    m_editor->render(renderer, m_camera);
    
    renderUI();
}

void EditorState::handleInput() {
    auto& input = m_game.getInput();
    auto& window = m_game.getWindow();
    
    if (input.isMouseButtonPressed(MouseButton::Middle)) {
        auto mouseDelta = window.getMouseDelta();
        
        if (input.isKeyPressed(KeyCode::LeftShift)) {
            m_cameraTarget -= vec3(mouseDelta.x * 0.1f, 0.0f, 0.0f);
            m_cameraTarget += vec3(0.0f, -mouseDelta.y * 0.1f, 0.0f);
        } else {
            m_cameraYaw -= mouseDelta.x * 0.01f;
            m_cameraPitch = glm::clamp(m_cameraPitch + mouseDelta.y * 0.01f, 
                                        -HALF_PI + 0.1f, HALF_PI - 0.1f);
        }
    }
    
    float scroll = input.getMouseScroll();
    if (scroll != 0.0f) {
        m_cameraDistance = glm::clamp(m_cameraDistance - scroll * 5.0f, 5.0f, 200.0f);
    }
    
    if (input.isKeyJustPressed(KeyCode::Escape)) {
        m_game.changeState("menu");
    }
    
    if (input.isKeyJustPressed(KeyCode::G)) {
        m_editor->toggleGrid();
    }
}

void EditorState::updateCamera(float) {
    float x = m_cameraDistance * std::cos(m_cameraPitch) * std::sin(m_cameraYaw);
    float y = m_cameraDistance * std::sin(-m_cameraPitch);
    float z = m_cameraDistance * std::cos(m_cameraPitch) * std::cos(m_cameraYaw);
    
    vec3 cameraPos = m_cameraTarget + vec3(x, y, z);
    
    m_camera.setPosition(cameraPos);
    m_camera.setTarget(m_cameraTarget);
}

void EditorState::renderUI() {
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::Begin("Track Editor", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    
    const char* tools[] = {"Select", "Road", "Terrain", "Props"};
    int currentTool = static_cast<int>(m_editor->getCurrentTool());
    if (ImGui::Combo("Tool", &currentTool, tools, IM_ARRAYSIZE(tools))) {
        m_editor->setCurrentTool(static_cast<EditorTool>(currentTool));
    }
    
    ImGui::Separator();
    
    if (ImGui::Button("New Track")) {
        m_editor->newTrack();
    }
    ImGui::SameLine();
    if (ImGui::Button("Save")) {
        m_editor->saveTrack("track.json");
    }
    ImGui::SameLine();
    if (ImGui::Button("Load")) {
        m_editor->loadTrack("track.json");
    }
    
    ImGui::Separator();
    
    if (m_editor->getCurrentTool() == EditorTool::Road) {
        float width = m_editor->getRoadWidth();
        if (ImGui::SliderFloat("Road Width", &width, 5.0f, 30.0f)) {
            m_editor->setRoadWidth(width);
        }
        
        if (ImGui::Button("Add Control Point")) {
            m_editor->addControlPoint(m_cameraTarget);
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear Road")) {
            m_editor->clearRoad();
        }
    }
    
    if (m_editor->getCurrentTool() == EditorTool::Terrain) {
        float brushSize = m_editor->getBrushSize();
        if (ImGui::SliderFloat("Brush Size", &brushSize, 1.0f, 50.0f)) {
            m_editor->setBrushSize(brushSize);
        }
        
        float brushStrength = m_editor->getBrushStrength();
        if (ImGui::SliderFloat("Strength", &brushStrength, 0.1f, 5.0f)) {
            m_editor->setBrushStrength(brushStrength);
        }
    }
    
    ImGui::Separator();
    
    bool showGrid = m_editor->isGridVisible();
    if (ImGui::Checkbox("Show Grid", &showGrid)) {
        m_editor->toggleGrid();
    }
    
    ImGui::End();
    
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 210, 10), ImGuiCond_Always);
    ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Middle Mouse: Rotate");
    ImGui::Text("Shift+MMB: Pan");
    ImGui::Text("Scroll: Zoom");
    ImGui::Text("G: Toggle Grid");
    ImGui::Text("Esc: Back to Menu");
    ImGui::End();
}

}
