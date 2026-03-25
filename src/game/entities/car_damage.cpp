#include "car_damage.hpp"
#include "physics/vehicle.hpp"
#include <spdlog/spdlog.h>
#include <glm/gtx/norm.hpp>
#include <cmath>

namespace viber {

void DamageModel::init(CarBody* body, Vehicle* vehicle, const DamageSettings& settings) {
    m_body = body;
    m_vehicle = vehicle;
    m_settings = settings;
    m_wheelDetached.fill(false);
}

void DamageModel::reset() {
    if (!m_body) return;
    for (auto& part : m_body->getParts())
        part.damage = 0.0f, part.detached = false;
    m_wheelDetached.fill(false);
}

// ---------------------------------------------------------------------------
// processImpact
// ---------------------------------------------------------------------------

void DamageModel::processImpact(float impulse, const vec3& localHitPos) {
    if (!m_body) return;
    if (impulse < m_settings.minDamageImpulse) return;

    float dmg = impulse * m_settings.damageSensitivity;

    // Check direct wheel impact first (wheels sit at extremes)
    const vec3 wheelLocalPositions[4] = {
        { 0.9f, -0.1f,  1.4f},
        {-0.9f, -0.1f,  1.4f},
        { 0.9f, -0.1f, -1.3f},
        {-0.9f, -0.1f, -1.3f}
    };
    for (int i = 0; i < 4; ++i) {
        float dist = glm::length(localHitPos - wheelLocalPositions[i]);
        if (dist < 0.5f && impulse >= m_settings.wheelDetachImpulse) {
            if (!m_wheelDetached[i]) {
                m_wheelDetached[i] = true;
                CarPart* wp = m_body->getWheelPart(i);
                if (wp) {
                    wp->damage = 1.0f;
                    detachPart(*wp, localHitPos, impulse);
                }
                spdlog::info("DamageModel: wheel {} detached (impulse={:.1f})", i, impulse);
                applyToVehicle();
                return;
            }
        }
    }

    // Damage closest body part
    CarPart* closest = findClosestPart(localHitPos);
    if (!closest || closest->detached) return;

    closest->damage = std::min(1.0f, closest->damage + dmg);

    if (closest->detachable && closest->damage >= m_settings.detachThreshold) {
        detachPart(*closest, localHitPos, impulse);
    }

    applyToVehicle();
}

// ---------------------------------------------------------------------------
// applyToVehicle
// ---------------------------------------------------------------------------

void DamageModel::applyToVehicle() {
    if (!m_vehicle || !m_body) return;

    // Wheel friction: detached wheel gets near-zero friction
    for (int i = 0; i < 4; ++i) {
        float friction = m_wheelDetached[i] ? 0.05f : -1.0f; // -1 = use default
        m_vehicle->setWheelFriction(i, friction < 0.0f ? m_vehicle->getParams().wheelFriction : friction);
    }

    // Chassis damage reduces max engine force
    float chassisDamage = 0.0f;
    for (auto& part : m_body->getParts()) {
        if (part.slot == CarPartSlot::Chassis)
            chassisDamage = part.damage;
    }
    float engineScale = 1.0f - chassisDamage * m_settings.engineDamageScale;
    m_vehicle->setMaxEngineForce(m_vehicle->getParams().maxEngineForce * engineScale);
}

float DamageModel::getTotalDamage() const {
    if (!m_body) return 0.0f;
    float total = 0.0f;
    int count = 0;
    for (auto& part : m_body->getParts()) {
        total += part.damage;
        ++count;
    }
    return count > 0 ? total / count : 0.0f;
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

CarPart* DamageModel::findClosestPart(const vec3& localPos) const {
    CarPart* best = nullptr;
    float bestDist = 1e9f;
    for (auto& part : m_body->getParts()) {
        if (part.detached) continue;
        // Use the translation component of localTransform as the part center
        vec3 center = vec3(part.localTransform[3]);
        float dist = glm::length2(localPos - center);
        if (dist < bestDist) {
            bestDist = dist;
            best = const_cast<CarPart*>(&part);
        }
    }
    return best;
}

void DamageModel::detachPart(CarPart& part, const vec3& localHitPos, float impulse) {
    part.detached = true;
    part.damage = 1.0f;

    // Launch the part away from the hit point (in world space approximation)
    vec3 dir = glm::normalize(vec3(part.localTransform[3]) - localHitPos);
    float speed = std::min(impulse * 0.01f, 8.0f);
    part.detachVelocity = dir * speed + vec3{0.0f, 3.0f, 0.0f};
    part.detachedTransform = part.localTransform;  // starts at current world pos (set by caller)
}

}
