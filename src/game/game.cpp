#include "game.hpp"
#include "states/menu_state.hpp"
#include "states/racing_state.hpp"
#include "states/editor_state.hpp"
#include "states/lobby_state.hpp"
#include "core/window.hpp"
#include "core/input.hpp"
#include "renderer/renderer.hpp"
#include "core/config.hpp"
#include <spdlog/spdlog.h>

namespace viber {

Game::Game(Window& window, Input& input, Renderer& renderer, Config& config)
    : m_window(window)
    , m_input(input)
    , m_renderer(renderer)
    , m_config(config)
{
}

Game::~Game() {
    shutdown();
}

void Game::init() {
    if (m_initialized) return;
    
    // Initialize ImGui
    m_ui.init(m_window.getHandle());
    
    registerState("menu", std::make_unique<MenuState>(*this));
    registerState("racing", std::make_unique<RacingState>(*this));
    registerState("editor", std::make_unique<EditorState>(*this));
    registerState("lobby", std::make_unique<LobbyState>(*this));
    
    for (auto& [name, state] : m_states) {
        state->init();
    }
    
    changeState("menu");
    
    m_initialized = true;
    spdlog::info("Game initialized");
}

void Game::update(float deltaTime) {
    if (!m_initialized) return;
    
    updateFPS(deltaTime);
    
    if (m_currentState) {
        m_currentState->update(deltaTime);
    }
    
    if (m_window.isKeyPressed(256)) {
        m_window.setShouldClose(true);
    }
}

void Game::render() {
    if (!m_initialized) return;
    
    m_ui.beginFrame();
    
    m_renderer.beginFrame();
    
    if (m_currentState) {
        m_currentState->render();
    }
    
    m_ui.endFrame();
    m_ui.render();
    
    m_renderer.endFrame();
}

void Game::shutdown() {
    if (!m_initialized) return;
    
    m_stateStack.clear();
    
    for (auto& [name, state] : m_states) {
        state->shutdown();
    }
    
    m_states.clear();
    m_currentState = nullptr;
    
    m_ui.shutdown();
    
    m_initialized = false;
    spdlog::info("Game shutdown");
}

void Game::changeState(const std::string& stateName) {
    GameState* newState = getState(stateName);
    if (!newState) {
        spdlog::error("State not found: {}", stateName);
        return;
    }
    
    if (m_currentState) {
        m_currentState->onExit();
    }
    
    m_currentState = newState;
    m_stateStack.clear();
    m_stateStack.push_back(newState);
    
    m_currentState->onEnter();
    
    spdlog::info("Changed to state: {}", stateName);
}

void Game::pushState(const std::string& stateName) {
    GameState* newState = getState(stateName);
    if (!newState) {
        spdlog::error("State not found: {}", stateName);
        return;
    }
    
    if (m_currentState) {
        m_currentState->onPause();
    }
    
    m_stateStack.push_back(newState);
    m_currentState = newState;
    
    m_currentState->onEnter();
}

void Game::popState() {
    if (m_stateStack.size() <= 1) {
        spdlog::warn("Cannot pop last state");
        return;
    }
    
    if (m_currentState) {
        m_currentState->onExit();
    }
    
    m_stateStack.pop_back();
    m_currentState = m_stateStack.back();
    
    if (m_currentState) {
        m_currentState->onResume();
    }
}

GameState* Game::getCurrentState() {
    return m_currentState;
}

GameState* Game::getState(const std::string& name) {
    auto it = m_states.find(name);
    return it != m_states.end() ? it->second.get() : nullptr;
}

void Game::registerState(const std::string& name, std::unique_ptr<GameState> state) {
    m_states[name] = std::move(state);
}

void Game::updateFPS(float deltaTime) {
    m_frameTime = deltaTime;
    m_fpsAccumulator += deltaTime;
    m_frameCount++;
    
    if (m_fpsAccumulator >= 1.0f) {
        m_fps = static_cast<float>(m_frameCount) / m_fpsAccumulator;
        m_frameCount = 0;
        m_fpsAccumulator = 0.0f;
    }
}

}
