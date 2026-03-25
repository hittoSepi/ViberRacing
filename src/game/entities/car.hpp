#pragma once

#include "car_body.hpp"
#include "car_damage.hpp"
#include "core/types.hpp"
#include "physics/vehicle.hpp"
#include <bgfx/bgfx.h>
#include <memory>

namespace viber {

class Car {
public:
    Car();
    ~Car();

    void init(PhysicsWorld* world);
    void init(PhysicsWorld* world, const CarDefinition& def);
    void shutdown();

    void update(float deltaTime);

    // Render using a bgfx shader program (view 0 by default)
    void render(bgfx::ViewId viewId, bgfx::ProgramHandle program) const;

    void setThrottle(float value);
    void setBrake(float value);
    void setSteering(float value);

    void reset(const vec3& position, const vec3& forward);
    void resetDamage();

    vec3 getPosition() const;
    quat getRotation() const;
    vec3 getVelocity() const;
    float getSpeedKmh() const;

    mat4 getTransform() const;
    mat4 getWheelTransform(int index) const;

    Vehicle* getVehicle() { return m_vehicle.get(); }
    const Vehicle* getVehicle() const { return m_vehicle.get(); }

    CarBody& getBody() { return m_carBody; }
    const CarBody& getBody() const { return m_carBody; }

    DamageModel& getDamage() { return m_damage; }
    const DamageModel& getDamage() const { return m_damage; }

private:
    void processCollisions();

    PhysicsWorld* m_world = nullptr;
    std::unique_ptr<Vehicle> m_vehicle;
    CarBody m_carBody;
    DamageModel m_damage;
};

}
