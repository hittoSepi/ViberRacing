#include "network.hpp"
#include <spdlog/spdlog.h>

namespace viber {

Network::Network() = default;

Network::~Network() {
    shutdown();
}

bool Network::init() {
    if (m_initialized) return true;
    
    if (enet_initialize() != 0) {
        spdlog::error("Failed to initialize ENet");
        return false;
    }
    
    m_initialized = true;
    spdlog::info("Network initialized (ENet)");
    return true;
}

void Network::shutdown() {
    if (!m_initialized) return;
    
    disconnect();
    
    if (m_host) {
        enet_host_destroy(m_host);
        m_host = nullptr;
    }
    
    enet_deinitialize();
    m_initialized = false;
    
    spdlog::info("Network shutdown");
}

bool Network::createHost(const NetworkConfig& config) {
    if (!m_initialized) return false;
    
    disconnect();
    
    m_config = config;
    
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = config.port;
    
    m_host = enet_host_create(&address, config.maxClients, 4, 
                               config.bandwidthIn, config.bandwidthOut);
    
    if (!m_host) {
        spdlog::error("Failed to create ENet host on port {}", config.port);
        return false;
    }
    
    m_mode = NetworkMode::Host;
    m_clientId = 0;
    
    spdlog::info("Hosting on port {} (max {} clients)", config.port, config.maxClients);
    return true;
}

bool Network::connect(const std::string& host, u16 port) {
    if (!m_initialized) return false;
    
    disconnect();
    
    m_host = enet_host_create(nullptr, 1, 4, 0, 0);
    if (!m_host) {
        spdlog::error("Failed to create ENet client");
        return false;
    }
    
    ENetAddress address;
    enet_address_set_host(&address, host.c_str());
    address.port = port;
    
    m_serverPeer = enet_host_connect(m_host, &address, 4, 0);
    if (!m_serverPeer) {
        spdlog::error("Failed to initiate connection to {}:{}", host, port);
        enet_host_destroy(m_host);
        m_host = nullptr;
        return false;
    }
    
    ENetEvent event;
    if (enet_host_service(m_host, &event, m_config.connectionTimeout) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT) {
        m_mode = NetworkMode::Client;
        spdlog::info("Connected to {}:{}", host, port);
        return true;
    }
    
    enet_peer_reset(m_serverPeer);
    m_serverPeer = nullptr;
    enet_host_destroy(m_host);
    m_host = nullptr;
    
    spdlog::error("Connection to {}:{} timed out", host, port);
    return false;
}

void Network::disconnect() {
    if (m_serverPeer) {
        ENetEvent event;
        enet_peer_disconnect(m_serverPeer, 0);
        
        while (enet_host_service(m_host, &event, m_config.disconnectTimeout) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_RECEIVE:
                    enet_packet_destroy(event.packet);
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    spdlog::info("Disconnected from server");
                    break;
                default:
                    break;
            }
        }
        
        enet_peer_reset(m_serverPeer);
        m_serverPeer = nullptr;
    }
    
    if (m_host) {
        enet_host_destroy(m_host);
        m_host = nullptr;
    }
    
    m_mode = NetworkMode::None;
}

void Network::processEvents() {
    if (!m_host) return;
    
    ENetEvent event;
    while (enet_host_service(m_host, &event, 0) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                handleConnect(event);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                handleDisconnect(event);
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                handleReceive(event);
                break;
            default:
                break;
        }
    }
    
    if (m_serverPeer) {
        m_stats.ping = m_serverPeer->roundTripTime;
    }
}

void Network::handleConnect(ENetEvent& event) {
    u32 clientId = 0;
    
    if (m_mode == NetworkMode::Host) {
        clientId = static_cast<u32>(event.peer->incomingPeerID);
        event.peer->data = reinterpret_cast<void*>(static_cast<uintptr_t>(clientId));
    } else {
        m_clientId = 0;
    }
    
    spdlog::info("Client {} connected", clientId);
    
    if (m_onConnect) {
        m_onConnect(clientId);
    }
}

void Network::handleDisconnect(ENetEvent& event) {
    u32 clientId = event.peer->data ? 
        static_cast<u32>(reinterpret_cast<uintptr_t>(event.peer->data)) : 0;
    
    spdlog::info("Client {} disconnected", clientId);
    
    if (m_onDisconnect) {
        m_onDisconnect(clientId);
    }
    
    if (m_mode == NetworkMode::Client) {
        m_mode = NetworkMode::None;
        m_serverPeer = nullptr;
    }
}

void Network::handleReceive(ENetEvent& event) {
    u32 clientId = event.peer->data ? 
        static_cast<u32>(reinterpret_cast<uintptr_t>(event.peer->data)) : 0;
    
    PacketChannel channel = static_cast<PacketChannel>(event.channelID);
    
    m_stats.bytesReceived += event.packet->dataLength;
    m_stats.packetsReceived++;
    
    if (m_onReceive) {
        m_onReceive(clientId, event.packet->data, event.packet->dataLength, channel);
    }
    
    enet_packet_destroy(event.packet);
}

void Network::sendToAll(const u8* data, size_t size, PacketChannel channel, bool reliable) {
    if (!m_host || m_mode != NetworkMode::Host) return;
    
    u32 flags = reliable ? ENET_PACKET_FLAG_RELIABLE : 0;
    ENetPacket* packet = enet_packet_create(data, size, flags);
    
    enet_host_broadcast(m_host, static_cast<u8>(channel), packet);
    
    m_stats.bytesSent += static_cast<u32>(size);
    m_stats.packetsSent++;
}

void Network::sendToClient(u32 clientId, const u8* data, size_t size, PacketChannel channel, bool reliable) {
    if (!m_host || m_mode != NetworkMode::Host) return;
    
    ENetPeer* peer = nullptr;
    for (size_t i = 0; i < m_host->peerCount; ++i) {
        if (m_host->peers[i].data && 
            static_cast<u32>(reinterpret_cast<uintptr_t>(m_host->peers[i].data)) == clientId) {
            peer = &m_host->peers[i];
            break;
        }
    }
    
    if (!peer || peer->state != ENET_PEER_STATE_CONNECTED) return;
    
    u32 flags = reliable ? ENET_PACKET_FLAG_RELIABLE : 0;
    ENetPacket* packet = enet_packet_create(data, size, flags);
    
    enet_peer_send(peer, static_cast<u8>(channel), packet);
    
    m_stats.bytesSent += static_cast<u32>(size);
    m_stats.packetsSent++;
}

void Network::sendToHost(const u8* data, size_t size, PacketChannel channel, bool reliable) {
    if (!m_host || m_mode != NetworkMode::Client || !m_serverPeer) return;
    
    u32 flags = reliable ? ENET_PACKET_FLAG_RELIABLE : 0;
    ENetPacket* packet = enet_packet_create(data, size, flags);
    
    enet_peer_send(m_serverPeer, static_cast<u8>(channel), packet);
    
    m_stats.bytesSent += static_cast<u32>(size);
    m_stats.packetsSent++;
}

}
