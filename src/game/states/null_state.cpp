#include "state.hpp"

namespace viber {

class NullState : public GameState {
public:
    explicit NullState(Game& game) : GameState(game) {}
    
    void init() override {}
    void shutdown() override {}
    
    void onEnter() override {}
    void onExit() override {}
    
    void update(float) override {}
    void render() override {}
    
    const char* getName() const override { return "null"; }
};

}
