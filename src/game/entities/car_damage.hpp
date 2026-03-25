#pragma once

#include "car_body.hpp"
#include "core/types.hpp"
#include <array>

namespace viber {

class Vehicle;

struct DamageSettings {
    float minDamageImpulse = 50.0f;    // impulse threshold to start damage
    float damageSensitivity = 0.002f;  // damage per unit impulse
    float detachThreshold = 0.85f;     // damage level at which detachable parts fly off
    float wheelDetachImpulse = 300.0f; // direct impulse needed to detach a wheel
    float engineDamageScale = 0.6f;    // max engine force reduction at full chassis damage
};

class DamageModel {
public:
    DamageModel() = default;

    void init(CarBody* body, Vehicle* vehicle, const DamageSettings& settings = DamageSettings{});

    // Call after physics step with impact data in local car space
    void processImpact(float impulse, const vec3& localHitPos);

    // Apply current damage state to vehicle physics parameters
    void applyToVehicle();

    // Reset all damage
    void reset();

    float getTotalDamage() const;
    bool isWheelDetached(int wheelIndex) const { return m_wheelDetached[wheelIndex]; }

    const DamageSettings& getSettings() const { return m_settings; }
    void setSettings(const DamageSettings& s) { m_settings = s; }

private:
    CarPart* findClosestPart(const vec3& localPos) const;
    void detachPart(CarPart& part, const vec3& localHitPos, float impulse);

    CarBody* m_body = nullptr;
    Vehicle* m_vehicle = nullptr;
    DamageSettings m_settings;
    std::array<bool, 4> m_wheelDetached{false, false, false, false};
};

}
