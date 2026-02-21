#include "p2p_host.hpp"
#include <spdlog/spdlog.h>

namespace viber {

P2PHost::P2PHost() {
    m_network.setConnectCallback([this](u32 id) { onClientConnect(id); });
    m_network.setDisconnectCallback([this](u32 id) { onClientDisconnect(id); });
    m_network.setReceiveCallback([this](u32 id, const u8* data, size_t size, PacketChannel ch) {
        onClientMessage(id, data, size, ch);
    });
}

P2PHost::~P2PHost() {
    stop();
}

bool P2PHost::start(u16 port, u32 maxClients) {
    if (m_running) return true;
    
    if (!m_network.init()) {
        return false;
    }
    
    NetworkConfig config;
    config.port = port;
    config.maxClients = maxClients;
    
    if (!m_network.createHost(config)) {
        m_network.shutdown();
        return false;
    }
    
    m_running = true;
    m_nextClientId = 1;
    
    spdlog::info("P2P Host started on port {}", port);
    return true;
}

void P2PHost::stop() {
    if (!m_running) return;
    
    m_clients.clear();
    m_network.shutdown();
    m_running = false;
    
    spdlog::info("P2P Host stopped");
}

void P2PHost::update() {
    if (!m_running) return;
    m_network.processEvents();
}

void P2PHost::kickClient(u32 clientId) {
    auto it = m_clients.find(clientId);
    if (it != m_clients.end()) {
        spdlog::info("Kicking client {}", clientId);
        m_network.sendToClient(clientId, 
            reinterpret_cast<const u8*>("KICK"), 4, 
            PacketChannel::Reliable, true);
    }
}

void P2PHost::kickAll() {
    for (const auto& [id, name] : m_clients) {
        kickClient(id);
    }
}

void P2PHost::onClientConnect(u32 clientId) {
    m_clients[clientId] = "Player_" + std::to_string(clientId);
    spdlog::info("Client {} connected (total: {})", clientId, m_clients.size());
}

void P2PHost::onClientDisconnect(u32 clientId) {
    m_clients.erase(clientId);
    spdlog::info("Client {} disconnected (total: {})", clientId, m_clients.size());
}

void P2PHost::onClientMessage(u32 clientId, const u8* data, size_t size, PacketChannel channel) {
    m_network.sendToAll(data, size, channel, channel == PacketChannel::Reliable);
}

}
