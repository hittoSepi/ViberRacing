#include "lobby_state.hpp"
#include "game/game.hpp"
#include "core/window.hpp"
#include "renderer/renderer.hpp"
#include <imgui.h>

namespace viber {

LobbyState::LobbyState(Game& game)
    : GameState(game)
{
}

void LobbyState::init() {
}

void LobbyState::shutdown() {
}

void LobbyState::onEnter() {
    auto size = m_game.getWindow().getSize();
    m_camera.setPerspective(60.0f * DEG_TO_RAD, size.x / size.y, 0.1f, 1000.0f);
    m_camera.setPosition(vec3(0.0f, 10.0f, 20.0f));
    m_camera.setTarget(vec3(0.0f));
    
    m_players.clear();
    m_isHost = false;
    m_isConnected = false;
}

void LobbyState::onExit() {
    if (m_host) {
        m_host->stop();
        m_host.reset();
    }
    if (m_client) {
        m_client->disconnect();
        m_client.reset();
    }
}

void LobbyState::update(float) {
    if (m_host) {
        m_host->update();
        updatePlayerList();
    }
    if (m_client) {
        m_client->update();
    }
}

void LobbyState::render() {
    auto& renderer = m_game.getRenderer();
    
    renderer.setViewTransform(m_camera);
    
    renderUI();
}

void LobbyState::renderUI() {
    auto& io = ImGui::GetIO();
    
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), 
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    
    ImGui::Begin("Multiplayer Lobby", nullptr, 
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    
    ImGui::InputText("Player Name", m_playerName, IM_ARRAYSIZE(m_playerName));
    
    ImGui::Separator();
    
    if (!m_isConnected) {
        ImGui::Text("Host a Game");
        ImGui::InputScalar("Port", ImGuiDataType_U16, &m_serverPort);
        
        if (ImGui::Button("Host Game", ImVec2(200, 30))) {
            m_host = std::make_unique<P2PHost>();
            if (m_host->start(m_serverPort)) {
                m_isHost = true;
                m_isConnected = true;
                m_players.push_back({0, std::string(m_playerName)});
            } else {
                m_host.reset();
            }
        }
        
        ImGui::Separator();
        
        ImGui::Text("Join a Game");
        ImGui::InputText("Server Address", m_serverAddress, IM_ARRAYSIZE(m_serverAddress));
        ImGui::InputScalar("Port###JoinPort", ImGuiDataType_U16, &m_serverPort);
        
        if (ImGui::Button("Join Game", ImVec2(200, 30))) {
            m_client = std::make_unique<P2PClient>();
            if (m_client->connect(m_serverAddress, m_serverPort)) {
                m_isConnected = true;
            } else {
                m_client.reset();
            }
        }
    } else {
        ImGui::Text(m_isHost ? "Hosting on port %u" : "Connected to %s", 
                    m_serverPort, m_serverAddress);
        
        ImGui::Separator();
        ImGui::Text("Players (%zu):", m_players.size());
        
        ImGui::BeginChild("PlayerList", ImVec2(250, 150), true);
        for (const auto& [id, name] : m_players) {
            ImGui::BulletText("%s", name.c_str());
        }
        ImGui::EndChild();
        
        ImGui::Spacing();
        
        if (m_isHost) {
            if (ImGui::Button("Start Race", ImVec2(120, 30))) {
                m_game.changeState("racing");
            }
            ImGui::SameLine();
        }
        
        if (ImGui::Button("Leave", ImVec2(120, 30))) {
            if (m_host) {
                m_host->stop();
                m_host.reset();
            }
            if (m_client) {
                m_client->disconnect();
                m_client.reset();
            }
            m_isHost = false;
            m_isConnected = false;
            m_players.clear();
        }
    }
    
    ImGui::Spacing();
    
    if (ImGui::Button("Back to Menu", ImVec2(200, 30))) {
        m_game.changeState("menu");
    }
    
    ImGui::End();
}

void LobbyState::updatePlayerList() {
    if (m_host) {
        const auto& clients = m_host->getClients();
        m_players.clear();
        m_players.push_back({0, std::string(m_playerName) + " (Host)"});
        for (const auto& [id, name] : clients) {
            m_players.push_back({id, name});
        }
    }
}

}
