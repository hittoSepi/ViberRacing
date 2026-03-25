#include "racing_state.hpp"
#include "game/game.hpp"
#include "core/window.hpp"
#include "core/input.hpp"
#include "renderer/renderer.hpp"
#include "renderer/debug_draw.hpp"
#include <imgui.h>

namespace viber {

RacingState::RacingState(Game& game)
    : GameState(game)
{
}

void RacingState::init() {
    m_physicsWorld = std::make_unique<PhysicsWorld>();
    m_vehicle = std::make_unique<Vehicle>();
    m_track = std::make_unique<Track>();
}

void RacingState::shutdown() {
    m_atmosphere.destroy();
    if (bgfx::isValid(m_atmoProgram)) {
        bgfx::destroy(m_atmoProgram);
        m_atmoProgram = BGFX_INVALID_HANDLE;
    }
    m_vehicle.reset();
    m_track.reset();
    m_physicsWorld.reset();
}

void RacingState::onEnter() {
    auto size = m_game.getWindow().getSize();
    m_camera.setPerspective(70.0f * DEG_TO_RAD, size.x / size.y, 0.1f, 1000.0f);
    
    m_physicsWorld = std::make_unique<PhysicsWorld>();
    m_vehicle = std::make_unique<Vehicle>();
    m_vehicle->create(m_physicsWorld.get());
    m_vehicle->reset(vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
    
    m_track = std::make_unique<Track>();
    m_track->generateTestTrack();

    if (!bgfx::isValid(m_atmoProgram)) {
        m_atmoProgram = createAtmosphereShader();
    }

    if (bgfx::isValid(m_atmoProgram)) {
        m_atmosphere.init();
        m_atmosphere.setTimeOfDay(m_timeOfDay);
    }
    
    m_lapTime = 0.0f;
    m_currentLap = 1;
    m_raceFinished = false;
}

void RacingState::onExit() {
    m_atmosphere.destroy();
    m_vehicle.reset();
    m_track.reset();
    m_physicsWorld.reset();
}

void RacingState::update(float deltaTime) {
    handleInput();
    
    m_physicsWorld->step(deltaTime);
    m_vehicle->update(deltaTime);
    
    updateCamera(deltaTime);
    
    m_lapTime += deltaTime;
}

void RacingState::render() {
    auto& renderer = m_game.getRenderer();

    const mat4 view = m_camera.getViewMatrix();
    const mat4 proj = m_camera.getProjectionMatrix();

    renderer.setViewTransform(m_camera);

    if (bgfx::isValid(m_atmoProgram)) {
        m_atmosphere.render(0, m_atmoProgram, view, proj);
    }
    
    m_track->render(renderer);
    
    mat4 vehicleTransform = m_vehicle->getWorldTransform();
    
    renderUI();
}

void RacingState::handleInput() {
    auto& input = m_game.getInput();
    
    float steering = 0.0f;
    float throttle = 0.0f;
    float brake = 0.0f;
    
    if (input.isKeyPressed(KeyCode::A) || input.isKeyPressed(KeyCode::Left)) {
        steering = -1.0f;
    } else if (input.isKeyPressed(KeyCode::D) || input.isKeyPressed(KeyCode::Right)) {
        steering = 1.0f;
    }
    
    if (input.isKeyPressed(KeyCode::W) || input.isKeyPressed(KeyCode::Up)) {
        throttle = 1.0f;
    }
    
    if (input.isKeyPressed(KeyCode::S) || input.isKeyPressed(KeyCode::Down)) {
        brake = 1.0f;
    }
    
    if (input.isKeyPressed(KeyCode::Space)) {
        brake = 1.0f;
    }
    
    m_vehicle->setSteering(steering);
    m_vehicle->setThrottle(throttle);
    m_vehicle->setBrake(brake);
    
    if (input.isKeyJustPressed(KeyCode::R)) {
        m_vehicle->reset(vec3(0.0f, 1.0f, 0.0f), m_vehicle->getForward());
    }
    
    if (input.isKeyJustPressed(KeyCode::Escape)) {
        m_game.changeState("menu");
    }
}

void RacingState::updateCamera(float) {
    vec3 vehiclePos = m_vehicle->getPosition();
    quat vehicleRot = m_vehicle->getRotation();
    
    mat4 rotMat = glm::mat4_cast(vehicleRot);
    
    vec3 offset = vec3(rotMat * vec4(m_cameraOffset, 0.0f));
    vec3 lookOffset = vec3(rotMat * vec4(m_cameraLookOffset, 0.0f));
    
    vec3 targetPos = vehiclePos + offset;
    vec3 lookAt = vehiclePos + lookOffset;
    
    m_camera.setPosition(targetPos);
    m_camera.setTarget(lookAt);
}

void RacingState::renderUI() {
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::Begin("Racing HUD", nullptr, 
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | 
        ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);
    
    ImGui::SetWindowFontScale(1.5f);
    
    ImGui::Text("Speed: %.0f km/h", m_vehicle->getSpeedKmh());
    ImGui::Text("Lap: %d / %d", m_currentLap, m_totalLaps);
    
    int minutes = static_cast<int>(m_lapTime) / 60;
    int seconds = static_cast<int>(m_lapTime) % 60;
    int ms = static_cast<int>((m_lapTime - static_cast<int>(m_lapTime)) * 100);
    ImGui::Text("Time: %02d:%02d.%02d", minutes, seconds, ms);
    
    if (m_bestLapTime > 0.0f) {
        int bestMin = static_cast<int>(m_bestLapTime) / 60;
        int bestSec = static_cast<int>(m_bestLapTime) % 60;
        int bestMs = static_cast<int>((m_bestLapTime - static_cast<int>(m_bestLapTime)) * 100);
        ImGui::Text("Best: %02d:%02d.%02d", bestMin, bestSec, bestMs);
    }
    
    ImGui::End();
    
    ImGui::SetNextWindowPos(ImVec2(10, ImGui::GetIO().DisplaySize.y - 110), ImGuiCond_Always);
    ImGui::Begin("Controls", nullptr, 
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
        ImGuiWindowFlags_NoCollapse);
    
    ImGui::SetWindowFontScale(0.8f);
    ImGui::Text("WASD / Arrows: Drive");
    ImGui::Text("Space: Brake");
    ImGui::Text("R: Reset Position");
    ImGui::Text("Esc: Menu");
    ImGui::Separator();
    ImGui::Text("Sky: %.0f%%", m_timeOfDay * 100.0f);
    
    ImGui::End();
}

}
