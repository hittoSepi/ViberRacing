#pragma once

#include "core/types.hpp"
#include "utils/serialization.hpp"
#include <vector>

namespace viber {

namespace SyncMessageType {
    enum : u8 {
        Invalid = 0,
        VehicleState = 1,
        VehicleInput = 2,
        TrackEdit = 3,
        TrackRequest = 4,
        TrackData = 5,
        PlayerJoin = 6,
        PlayerLeave = 7,
        Chat = 8,
        GameEvent = 9,
    };
}

struct VehicleStateSync {
    u8 messageType = SyncMessageType::VehicleState;
    u32 playerId = 0;
    u32 sequence = 0;
    
    vec3 position;
    quat rotation;
    vec3 velocity;
    vec3 angularVelocity;
    float steeringAngle;
    float throttle;
    float brake;
    
    void serialize(BinaryWriter& writer) const;
    void deserialize(BinaryReader& reader);
};

struct VehicleInputSync {
    u8 messageType = SyncMessageType::VehicleInput;
    u32 playerId = 0;
    u32 sequence = 0;
    
    float steering;
    float throttle;
    float brake;
    bool handbrake;
    
    void serialize(BinaryWriter& writer) const;
    void deserialize(BinaryReader& reader);
};

struct TrackEditSync {
    u8 messageType = SyncMessageType::TrackEdit;
    u32 playerId = 0;
    u8 editType = 0;
    
    u32 splineId = 0;
    u32 pointIndex = 0;
    vec3 position;
    vec3 rotation;
    
    void serialize(BinaryWriter& writer) const;
    void deserialize(BinaryReader& reader);
};

class StateSync {
public:
    StateSync();
    
    void setLocalPlayerId(u32 playerId) { m_localPlayerId = playerId; }
    u32 getLocalPlayerId() const { return m_localPlayerId; }
    
    u32 getNextSequence() { return m_sequence++; }
    
    std::vector<u8> packVehicleState(const VehicleStateSync& state);
    std::vector<u8> packVehicleInput(const VehicleInputSync& input);
    std::vector<u8> packTrackEdit(const TrackEditSync& edit);
    
    bool unpackMessage(const u8* data, size_t size, u8& outType, std::vector<u8>& outPayload);
    
    VehicleStateSync unpackVehicleState(const u8* data, size_t size);
    VehicleInputSync unpackVehicleInput(const u8* data, size_t size);
    TrackEditSync unpackTrackEdit(const u8* data, size_t size);
    
    void setSendRate(u32 hz) { m_sendRate = hz; }
    u32 getSendRate() const { return m_sendRate; }
    
private:
    u32 m_localPlayerId = 0;
    u32 m_sequence = 0;
    u32 m_sendRate = 30;
};

}
