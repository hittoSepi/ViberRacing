#pragma once

#include "core/types.hpp"
#include <string>

namespace viber {

class Game;

class GameState {
public:
    explicit GameState(Game& game) : m_game(game) {}
    virtual ~GameState() = default;
    
    virtual void init() = 0;
    virtual void shutdown() = 0;
    
    virtual void onEnter() = 0;
    virtual void onExit() = 0;
    virtual void onPause() {}
    virtual void onResume() {}
    
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    
    virtual const char* getName() const = 0;
    
protected:
    Game& m_game;
};

}
