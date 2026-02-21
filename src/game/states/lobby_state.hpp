#pragma once

#include "states/state.hpp"
#include "renderer/camera.hpp"
#include "networking/p2p_host.hpp"
#include "networking/p2p_client.hpp"
#include <memory>

namespace viber {

class LobbyState : public GameState {
public:
    explicit LobbyState(Game& game);
    
    void init() override;
    void shutdown() override;
    
    void onEnter() override;
    void onExit() override;
    
    void update(float deltaTime) override;
    void render() override;
    
    const char* getName() const override { return "lobby"; }
    
private:
    void renderUI();
    void updatePlayerList();
    
    Camera m_camera;
    
    std::unique_ptr<P2PHost> m_host;
    std::unique_ptr<P2PClient> m_client;
    
    bool m_isHost = false;
    bool m_isConnected = false;
    
    char m_serverAddress[64] = "127.0.0.1";
    char m_playerName[32] = "Player";
    u16 m_serverPort = 7777;
    
    std::vector<std::pair<u32, std::string>> m_players;
};

}
