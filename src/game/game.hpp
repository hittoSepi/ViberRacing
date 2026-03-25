#pragma once

#include "core/types.hpp"
#include "renderer/ui/imgui_renderer.hpp"
#include <string>
#include <memory>
#include <unordered_map>

namespace viber {

class GameState;

class Game {
public:
    Game(class Window& window, class Input& input, class Renderer& renderer, class Config& config);
    ~Game();
    
    void init();
    void update(float deltaTime);
    void render();
    void shutdown();
    
    void changeState(const std::string& stateName);
    void pushState(const std::string& stateName);
    void popState();
    GameState* getCurrentState();
    GameState* getState(const std::string& name);
    
    Window& getWindow() { return m_window; }
    Input& getInput() { return m_input; }
    Renderer& getRenderer() { return m_renderer; }
    Config& getConfig() { return m_config; }
    ImGuiRenderer& getUI() { return m_ui; }
    
    void registerState(const std::string& name, std::unique_ptr<GameState> state);
    
    bool isRunning() const { return m_running; }
    void quit() { m_running = false; }
    
    float getFPS() const { return m_fps; }
    float getFrameTime() const { return m_frameTime; }
    
private:
    void updateFPS(float deltaTime);
    
    Window& m_window;
    Input& m_input;
    Renderer& m_renderer;
    Config& m_config;
    
    ImGuiRenderer m_ui;
    
    std::unordered_map<std::string, std::unique_ptr<GameState>> m_states;
    std::vector<GameState*> m_stateStack;
    GameState* m_currentState = nullptr;
    
    bool m_initialized = false;
    bool m_running = true;
    
    float m_fps = 0.0f;
    float m_frameTime = 0.0f;
    float m_fpsAccumulator = 0.0f;
    u32 m_frameCount = 0;
};

}
