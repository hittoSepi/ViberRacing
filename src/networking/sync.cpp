#include "sync.hpp"

namespace viber {

StateSync::StateSync() = default;

void VehicleStateSync::serialize(BinaryWriter& writer) const {
    writer.write(messageType);
    writer.write(playerId);
    writer.write(sequence);
    writer.writeVec3(position);
    writer.writeQuat(rotation);
    writer.writeVec3(velocity);
    writer.writeVec3(angularVelocity);
    writer.write(steeringAngle);
    writer.write(throttle);
    writer.write(brake);
}

void VehicleStateSync::deserialize(BinaryReader& reader) {
    reader.read(messageType);
    reader.read(playerId);
    reader.read(sequence);
    reader.readVec3(position);
    reader.readQuat(rotation);
    reader.readVec3(velocity);
    reader.readVec3(angularVelocity);
    reader.read(steeringAngle);
    reader.read(throttle);
    reader.read(brake);
}

void VehicleInputSync::serialize(BinaryWriter& writer) const {
    writer.write(messageType);
    writer.write(playerId);
    writer.write(sequence);
    writer.write(steering);
    writer.write(throttle);
    writer.write(brake);
    writer.write(static_cast<u8>(handbrake ? 1 : 0));
}

void VehicleInputSync::deserialize(BinaryReader& reader) {
    reader.read(messageType);
    reader.read(playerId);
    reader.read(sequence);
    reader.read(steering);
    reader.read(throttle);
    reader.read(brake);
    u8 hb;
    reader.read(hb);
    handbrake = hb != 0;
}

void TrackEditSync::serialize(BinaryWriter& writer) const {
    writer.write(messageType);
    writer.write(playerId);
    writer.write(editType);
    writer.write(splineId);
    writer.write(pointIndex);
    writer.writeVec3(position);
    writer.writeVec3(rotation);
}

void TrackEditSync::deserialize(BinaryReader& reader) {
    reader.read(messageType);
    reader.read(playerId);
    reader.read(editType);
    reader.read(splineId);
    reader.read(pointIndex);
    reader.readVec3(position);
    reader.readVec3(rotation);
}

std::vector<u8> StateSync::packVehicleState(const VehicleStateSync& state) {
    BinaryWriter writer;
    state.serialize(writer);
    return writer.takeData();
}

std::vector<u8> StateSync::packVehicleInput(const VehicleInputSync& input) {
    BinaryWriter writer;
    input.serialize(writer);
    return writer.takeData();
}

std::vector<u8> StateSync::packTrackEdit(const TrackEditSync& edit) {
    BinaryWriter writer;
    edit.serialize(writer);
    return writer.takeData();
}

bool StateSync::unpackMessage(const u8* data, size_t size, u8& outType, std::vector<u8>& outPayload) {
    if (size < 1) return false;
    
    outType = data[0];
    outPayload.assign(data + 1, data + size);
    return true;
}

VehicleStateSync StateSync::unpackVehicleState(const u8* data, size_t size) {
    VehicleStateSync state;
    BinaryReader reader(data, size);
    state.deserialize(reader);
    return state;
}

VehicleInputSync StateSync::unpackVehicleInput(const u8* data, size_t size) {
    VehicleInputSync input;
    BinaryReader reader(data, size);
    input.deserialize(reader);
    return input;
}

TrackEditSync StateSync::unpackTrackEdit(const u8* data, size_t size) {
    TrackEditSync edit;
    BinaryReader reader(data, size);
    edit.deserialize(reader);
    return edit;
}

}
