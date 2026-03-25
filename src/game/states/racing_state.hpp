#pragma once

#include "game/states/state.hpp"
#include "renderer/camera.hpp"
#include "physics/world.hpp"
#include "physics/vehicle.hpp"
#include "game/entities/car.hpp"
#include "game/entities/track.hpp"
#include <memory>

namespace viber {

class RacingState : public GameState {
public:
    explicit RacingState(Game& game);
    
    void init() override;
    void shutdown() override;
    
    void onEnter() override;
    void onExit() override;
    
    void update(float deltaTime) override;
    void render() override;
    
    const char* getName() const override { return "racing"; }
    
private:
    void handleInput();
    void updateCamera(float deltaTime);
    void renderUI();
    
    Camera m_camera;
    std::unique_ptr<PhysicsWorld> m_physicsWorld;
    std::unique_ptr<Vehicle> m_vehicle;
    std::unique_ptr<Track> m_track;
    
    vec3 m_cameraOffset{0.0f, 5.0f, -12.0f};
    vec3 m_cameraLookOffset{0.0f, 1.0f, 5.0f};
    
    float m_lapTime = 0.0f;
    float m_bestLapTime = 0.0f;
    int m_currentLap = 1;
    int m_totalLaps = 3;
    bool m_raceFinished = false;
};

}
