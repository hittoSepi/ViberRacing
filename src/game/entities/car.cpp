#include "car.hpp"

namespace viber {

Car::Car() {
    m_vehicle = std::make_unique<Vehicle>();
}

Car::~Car() {
    shutdown();
}

void Car::init(PhysicsWorld* world) {
    VehicleParams params;
    params.mass = 1200.0f;
    params.maxEngineForce = 4000.0f;
    params.maxBrakeForce = 1500.0f;
    params.maxSteeringAngle = 0.7f;
    
    m_vehicle->create(world, params);
}

void Car::shutdown() {
    m_vehicle.reset();
}

void Car::update(float deltaTime) {
    m_vehicle->update(deltaTime);
}

void Car::setThrottle(float value) {
    m_vehicle->setThrottle(value);
}

void Car::setBrake(float value) {
    m_vehicle->setBrake(value);
}

void Car::setSteering(float value) {
    m_vehicle->setSteering(value);
}

void Car::reset(const vec3& position, const vec3& forward) {
    m_vehicle->reset(position, forward);
}

vec3 Car::getPosition() const {
    return m_vehicle->getPosition();
}

quat Car::getRotation() const {
    return m_vehicle->getRotation();
}

vec3 Car::getVelocity() const {
    return m_vehicle->getVelocity();
}

float Car::getSpeedKmh() const {
    return m_vehicle->getSpeedKmh();
}

mat4 Car::getTransform() const {
    return m_vehicle->getWorldTransform();
}

mat4 Car::getWheelTransform(int index) const {
    return m_vehicle->getWheelTransform(index);
}

}
