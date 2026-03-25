#pragma once

#include "game/states/state.hpp"
#include "renderer/camera.hpp"
#include <imgui.h>

namespace viber {

class MenuState : public GameState {
public:
    explicit MenuState(Game& game);
    
    void init() override;
    void shutdown() override;
    
    void onEnter() override;
    void onExit() override;
    
    void update(float deltaTime) override;
    void render() override;
    
    const char* getName() const override { return "menu"; }
    
private:
    void renderUI();
    
    Camera m_camera;
    int m_selectedTrack = 0;
    int m_selectedCar = 0;
    bool m_showSettings = false;
};

}
