#include "menu_state.hpp"
#include "game/game.hpp"
#include "core/window.hpp"
#include "core/input.hpp"
#include "renderer/renderer.hpp"
#include <imgui.h>

namespace viber {

MenuState::MenuState(Game& game)
    : GameState(game)
{
}

void MenuState::init() {
}

void MenuState::shutdown() {
}

void MenuState::onEnter() {
    auto size = m_game.getWindow().getSize();
    m_camera.setPerspective(60.0f * DEG_TO_RAD, size.x / size.y, 0.1f, 1000.0f);
    m_camera.setPosition(vec3(0.0f, 10.0f, 20.0f));
    m_camera.setTarget(vec3(0.0f));
}

void MenuState::onExit() {
}

void MenuState::update(float) {
    auto size = m_game.getWindow().getSize();
    m_camera.setAspectRatio(size.x / size.y);
}

void MenuState::render() {
    auto& renderer = m_game.getRenderer();
    
    renderer.setViewTransform(m_camera);

    renderUI();
}

void MenuState::renderUI() {
    auto& io = ImGui::GetIO();
    
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), 
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    
    ImGui::Begin("ViberRacing", nullptr, 
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    
    ImGui::Text("Main Menu");
    ImGui::Separator();
    ImGui::Spacing();
    
    const char* tracks[] = {"Test Track", "Coastal Circuit", "Mountain Pass"};
    ImGui::Combo("Track", &m_selectedTrack, tracks, IM_ARRAYSIZE(tracks));
    
    const char* cars[] = {"Sports Car", "Muscle Car", "Go-Kart"};
    ImGui::Combo("Car", &m_selectedCar, cars, IM_ARRAYSIZE(cars));
    
    ImGui::Spacing();
    
    if (ImGui::Button("Race", ImVec2(200, 40))) {
        m_game.changeState("racing");
    }
    
    if (ImGui::Button("Track Editor", ImVec2(200, 40))) {
        m_game.changeState("editor");
    }
    
    if (ImGui::Button("Multiplayer", ImVec2(200, 40))) {
        m_game.changeState("lobby");
    }
    
    ImGui::Spacing();
    
    if (ImGui::Button("Settings", ImVec2(200, 40))) {
        m_showSettings = !m_showSettings;
    }
    
    ImGui::Spacing();
    
    if (ImGui::Button("Quit", ImVec2(200, 40))) {
        m_game.quit();
    }
    
    ImGui::End();
    
    if (m_showSettings) {
        ImGui::Begin("Settings", &m_showSettings);
        
        auto size = m_game.getWindow().getSize();
        static int resolution[2] = {static_cast<int>(size.x), static_cast<int>(size.y)};
        if (ImGui::InputInt2("Resolution", resolution)) {
            m_game.getWindow().setSize(resolution[0], resolution[1]);
        }
        
        static bool vsync = true;
        if (ImGui::Checkbox("VSync", &vsync)) {
        }
        
        static int aa = 2;
        ImGui::SliderInt("MSAA", &aa, 1, 8);
        
        ImGui::End();
    }
}

}
