#include "p2p_client.hpp"
#include <spdlog/spdlog.h>

namespace viber {

P2PClient::P2PClient() {
    m_network.setDisconnectCallback([this](u32) {
        m_connected = false;
        spdlog::info("Disconnected from server");
    });
    
    m_network.setReceiveCallback([this](u32, const u8* data, size_t size, PacketChannel ch) {
        onServerMessage(data, size, ch);
    });
}

P2PClient::~P2PClient() {
    disconnect();
}

bool P2PClient::connect(const std::string& host, u16 port) {
    if (m_connected) return true;
    
    if (!m_network.init()) {
        return false;
    }
    
    if (!m_network.connect(host, port)) {
        m_network.shutdown();
        return false;
    }
    
    m_connected = true;
    spdlog::info("Connected to {}:{}", host, port);
    return true;
}

void P2PClient::disconnect() {
    if (!m_connected) return;
    
    m_network.disconnect();
    m_network.shutdown();
    m_connected = false;
    
    spdlog::info("Disconnected");
}

void P2PClient::update() {
    if (!m_connected) return;
    m_network.processEvents();
}

void P2PClient::onServerMessage(const u8* data, size_t size, PacketChannel channel) {
    if (m_onMessage) {
        m_onMessage(data, size);
    }
}

}
