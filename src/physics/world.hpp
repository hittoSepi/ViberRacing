#pragma once

#include "core/types.hpp"
#include <btBulletDynamicsCommon.h>
#include <memory>
#include <vector>

namespace viber {

struct PhysicsSettings {
    vec3 gravity{0.0f, -9.81f, 0.0f};
    int maxSubSteps = 4;
    float fixedTimeStep = 1.0f / 240.0f;
    float defaultRestitution = 0.0f;
    float defaultFriction = 0.5f;
};

class PhysicsWorld {
public:
    PhysicsWorld();
    ~PhysicsWorld();
    
    PhysicsWorld(const PhysicsWorld&) = delete;
    PhysicsWorld& operator=(const PhysicsWorld&) = delete;
    
    void step(float deltaTime);
    void reset();
    
    void setGravity(const vec3& gravity);
    vec3 getGravity() const;
    
    void addRigidBody(btRigidBody* body);
    void removeRigidBody(btRigidBody* body);
    
    void addVehicle(btRaycastVehicle* vehicle);
    void removeVehicle(btRaycastVehicle* vehicle);
    
    bool raycast(const vec3& from, const vec3& to, vec3& hitPoint, vec3& hitNormal, 
                 void** hitObject = nullptr, short collisionGroup = -1, short collisionMask = -1);
    
    bool sphereCast(const vec3& from, const vec3& to, float radius, 
                    vec3& hitPoint, vec3& hitNormal);
    
    btDiscreteDynamicsWorld* getWorld() const { return m_world.get(); }
    
    const PhysicsSettings& getSettings() const { return m_settings; }
    void setSettings(const PhysicsSettings& settings);
    
    void setDebugDrawEnabled(bool enabled);
    bool isDebugDrawEnabled() const { return m_debugDrawEnabled; }
    
private:
    std::unique_ptr<btDefaultCollisionConfiguration> m_collisionConfig;
    std::unique_ptr<btCollisionDispatcher> m_dispatcher;
    std::unique_ptr<btBroadphaseInterface> m_broadphase;
    std::unique_ptr<btSequentialImpulseConstraintSolver> m_solver;
    std::unique_ptr<btDiscreteDynamicsWorld> m_world;
    
    PhysicsSettings m_settings;
    bool m_debugDrawEnabled = false;
    
    std::vector<btRigidBody*> m_rigidBodies;
    std::vector<btRaycastVehicle*> m_vehicles;
};

}
