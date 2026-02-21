#pragma once

#include "core/types.hpp"
#include <enet/enet.h>
#include <string>
#include <functional>
#include <vector>

namespace viber {

enum class NetworkMode {
    None,
    Host,
    Client
};

enum class PacketChannel : u8 {
    Reliable = 0,
    Unreliable = 1,
    StateSync = 2,
    Voice = 3,
};

struct NetworkConfig {
    u16 port = 7777;
    u32 maxClients = 8;
    u32 bandwidthIn = 0;
    u32 bandwidthOut = 0;
    u32 connectionTimeout = 5000;
    u32 disconnectTimeout = 3000;
};

struct NetworkStats {
    u32 bytesSent = 0;
    u32 bytesReceived = 0;
    u32 packetsSent = 0;
    u32 packetsReceived = 0;
    u32 ping = 0;
};

class Network {
public:
    using ConnectCallback = std::function<void(u32 clientId)>;
    using DisconnectCallback = std::function<void(u32 clientId)>;
    using ReceiveCallback = std::function<void(u32 clientId, const u8* data, size_t size, PacketChannel channel)>;
    
    Network();
    ~Network();
    
    bool init();
    void shutdown();
    
    bool createHost(const NetworkConfig& config);
    bool connect(const std::string& host, u16 port);
    void disconnect();
    
    void processEvents();
    
    void sendToAll(const u8* data, size_t size, PacketChannel channel, bool reliable = false);
    void sendToClient(u32 clientId, const u8* data, size_t size, PacketChannel channel, bool reliable = false);
    void sendToHost(const u8* data, size_t size, PacketChannel channel, bool reliable = false);
    
    void setConnectCallback(ConnectCallback callback) { m_onConnect = std::move(callback); }
    void setDisconnectCallback(DisconnectCallback callback) { m_onDisconnect = std::move(callback); }
    void setReceiveCallback(ReceiveCallback callback) { m_onReceive = std::move(callback); }
    
    NetworkMode getMode() const { return m_mode; }
    bool isHost() const { return m_mode == NetworkMode::Host; }
    bool isClient() const { return m_mode == NetworkMode::Client; }
    bool isConnected() const { return m_mode != NetworkMode::None; }
    
    u32 getClientId() const { return m_clientId; }
    u32 getPing() const { return m_stats.ping; }
    
    const NetworkStats& getStats() const { return m_stats; }
    
private:
    void handleConnect(ENetEvent& event);
    void handleDisconnect(ENetEvent& event);
    void handleReceive(ENetEvent& event);
    
    ENetHost* m_host = nullptr;
    ENetPeer* m_serverPeer = nullptr;
    NetworkMode m_mode = NetworkMode::None;
    NetworkConfig m_config;
    u32 m_clientId = 0;
    
    NetworkStats m_stats;
    
    ConnectCallback m_onConnect;
    DisconnectCallback m_onDisconnect;
    ReceiveCallback m_onReceive;
    
    bool m_initialized = false;
};

}
