#pragma once

#include "core/types.hpp"
#include "tire_model.hpp"
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include <memory>
#include <array>

namespace viber {

struct VehicleParams {
    float mass = 1200.0f;
    float chassisWidth = 1.8f;
    float chassisHeight = 0.6f;
    float chassisLength = 4.2f;
    float chassisOffsetY = 0.5f;
    
    float wheelRadius = 0.35f;
    float wheelWidth = 0.25f;
    float wheelFriction = 1.5f;
    float wheelMass = 30.0f;
    
    float suspensionStiffness = 50.0f;
    float suspensionDamping = 2.3f;
    float suspensionCompression = 4.4f;
    float suspensionRestLength = 0.3f;
    float maxSuspensionTravel = 0.3f;
    float maxSuspensionForce = 6000.0f;
    
    float rollInfluence = 0.04f;
    
    float maxEngineForce = 3000.0f;
    float maxBrakeForce = 1000.0f;
    float maxSteeringAngle = 0.6f;
    
    vec3 wheelPositions[4] = {
        { 0.9f, 0.2f,  1.4f},
        {-0.9f, 0.2f,  1.4f},
        { 0.9f, 0.2f, -1.3f},
        {-0.9f, 0.2f, -1.3f}
    };
    
    bool isFrontWheel[4] = {true, true, false, false};
    bool isRearWheel[4] = {false, false, true, true};
};

struct VehicleState {
    vec3 position{0.0f};
    quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
    vec3 velocity{0.0f};
    vec3 angularVelocity{0.0f};
    
    float speed = 0.0f;
    float speedKmh = 0.0f;
    
    float engineForce = 0.0f;
    float brakeForce = 0.0f;
    float steeringAngle = 0.0f;
    
    float wheelRotation[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float wheelSuspensionLength[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    bool wheelOnGround[4] = {false, false, false, false};
    
    float lateralSlip[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float longitudinalSlip[4] = {0.0f, 0.0f, 0.0f, 0.0f};
};

class Vehicle {
public:
    Vehicle();
    ~Vehicle();
    
    bool create(PhysicsWorld* world, const VehicleParams& params = VehicleParams{});
    void destroy();
    
    void setThrottle(float throttle);
    void setBrake(float brake);
    void setSteering(float steering);
    
    void setPosition(const vec3& position);
    void setRotation(const quat& rotation);
    void reset(const vec3& position, const vec3& forward);
    
    void update(float deltaTime);
    
    const VehicleState& getState() const { return m_state; }
    const VehicleParams& getParams() const { return m_params; }
    
    vec3 getPosition() const;
    quat getRotation() const;
    vec3 getVelocity() const;
    vec3 getForward() const;
    vec3 getRight() const;
    vec3 getUp() const;
    
    float getSpeed() const;
    float getSpeedKmh() const;
    
    btRigidBody* getChassisBody() const { return m_chassisBody; }
    btRaycastVehicle* getBulletVehicle() const { return m_vehicle; }
    
    void setTireModel(std::unique_ptr<TireModel> model);
    TireModel* getTireModel() const { return m_tireModel.get(); }
    
    mat4 getWorldTransform() const;
    mat4 getWheelTransform(int wheelIndex) const;
    
private:
    void updateState();
    void applyTireForces();
    
    PhysicsWorld* m_world = nullptr;
    
    std::unique_ptr<btRigidBody> m_chassisBody;
    std::unique_ptr<btCollisionShape> m_chassisShape;
    std::unique_ptr<btVehicleRaycaster> m_vehicleRaycaster;
    std::unique_ptr<btRaycastVehicle> m_vehicle;
    std::unique_ptr<TireModel> m_tireModel;
    
    VehicleParams m_params;
    VehicleState m_state;
    
    bool m_created = false;
};

}
