#pragma once

#include "core/types.hpp"
#include "physics/vehicle.hpp"
#include <memory>

namespace viber {

class Car {
public:
    Car();
    ~Car();
    
    void init(PhysicsWorld* world);
    void shutdown();
    
    void update(float deltaTime);
    
    void setThrottle(float value);
    void setBrake(float value);
    void setSteering(float value);
    
    void reset(const vec3& position, const vec3& forward);
    
    vec3 getPosition() const;
    quat getRotation() const;
    vec3 getVelocity() const;
    float getSpeedKmh() const;
    
    mat4 getTransform() const;
    mat4 getWheelTransform(int index) const;
    
    Vehicle* getVehicle() { return m_vehicle.get(); }
    const Vehicle* getVehicle() const { return m_vehicle.get(); }
    
private:
    std::unique_ptr<Vehicle> m_vehicle;
};

}
