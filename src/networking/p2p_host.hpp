#pragma once

#include "network.hpp"
#include <unordered_map>

namespace viber {

class P2PHost {
public:
    P2PHost();
    ~P2PHost();
    
    bool start(u16 port = 7777, u32 maxClients = 8);
    void stop();
    
    void update();
    
    void kickClient(u32 clientId);
    void kickAll();
    
    u32 getClientCount() const { return static_cast<u32>(m_clients.size()); }
    const std::unordered_map<u32, std::string>& getClients() const { return m_clients; }
    
    Network* getNetwork() { return &m_network; }
    
private:
    void onClientConnect(u32 clientId);
    void onClientDisconnect(u32 clientId);
    void onClientMessage(u32 clientId, const u8* data, size_t size, PacketChannel channel);
    
    Network m_network;
    std::unordered_map<u32, std::string> m_clients;
    u32 m_nextClientId = 1;
    bool m_running = false;
};

}
