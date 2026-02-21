#pragma once

#include "network.hpp"

namespace viber {

class P2PClient {
public:
    P2PClient();
    ~P2PClient();
    
    bool connect(const std::string& host, u16 port = 7777);
    void disconnect();
    
    void update();
    
    bool isConnected() const { return m_connected; }
    u32 getPing() const { return m_network.getPing(); }
    
    Network* getNetwork() { return &m_network; }
    
    using MessageCallback = std::function<void(const u8* data, size_t size)>;
    void setMessageCallback(MessageCallback callback) { m_onMessage = std::move(callback); }
    
private:
    void onServerMessage(const u8* data, size_t size, PacketChannel channel);
    
    Network m_network;
    bool m_connected = false;
    MessageCallback m_onMessage;
};

}
