#pragma once

#include "states/state.hpp"
#include "renderer/camera.hpp"
#include "game/track_editor/editor.hpp"
#include <memory>

namespace viber {

class EditorState : public GameState {
public:
    explicit EditorState(Game& game);
    
    void init() override;
    void shutdown() override;
    
    void onEnter() override;
    void onExit() override;
    
    void update(float deltaTime) override;
    void render() override;
    
    const char* getName() const override { return "editor"; }
    
private:
    void handleInput();
    void updateCamera(float deltaTime);
    void renderUI();
    
    Camera m_camera;
    std::unique_ptr<TrackEditor> m_editor;
    
    float m_cameraYaw = 0.0f;
    float m_cameraPitch = -30.0f * DEG_TO_RAD;
    float m_cameraDistance = 50.0f;
    vec3 m_cameraTarget{0.0f};
    
    bool m_isDragging = false;
    bool m_isPanning = false;
};

}
