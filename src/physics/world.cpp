#include "world.hpp"
#include <spdlog/spdlog.h>

namespace viber {

PhysicsWorld::PhysicsWorld() {
    m_collisionConfig = std::make_unique<btDefaultCollisionConfiguration>();
    m_dispatcher = std::make_unique<btCollisionDispatcher>(m_collisionConfig.get());
    m_broadphase = std::make_unique<btDbvtBroadphase>();
    m_solver = std::make_unique<btSequentialImpulseConstraintSolver>();
    
    m_world = std::make_unique<btDiscreteDynamicsWorld>(
        m_dispatcher.get(),
        m_broadphase.get(),
        m_solver.get(),
        m_collisionConfig.get()
    );
    
    m_world->setGravity(btVector3(
        m_settings.gravity.x,
        m_settings.gravity.y,
        m_settings.gravity.z
    ));
    
    spdlog::info("PhysicsWorld initialized");
}

PhysicsWorld::~PhysicsWorld() {
    for (auto* vehicle : m_vehicles) {
        m_world->removeVehicle(vehicle);
    }
    
    for (auto* body : m_rigidBodies) {
        m_world->removeRigidBody(body);
    }
    
    m_vehicles.clear();
    m_rigidBodies.clear();
    m_world.reset();
    m_solver.reset();
    m_broadphase.reset();
    m_dispatcher.reset();
    m_collisionConfig.reset();
    
    spdlog::info("PhysicsWorld destroyed");
}

void PhysicsWorld::step(float deltaTime) {
    m_world->stepSimulation(deltaTime, m_settings.maxSubSteps, m_settings.fixedTimeStep);
}

void PhysicsWorld::reset() {
    for (auto* body : m_rigidBodies) {
        body->clearForces();
        body->setLinearVelocity(btVector3(0, 0, 0));
        body->setAngularVelocity(btVector3(0, 0, 0));
    }
}

void PhysicsWorld::setGravity(const vec3& gravity) {
    m_settings.gravity = gravity;
    m_world->setGravity(btVector3(gravity.x, gravity.y, gravity.z));
}

vec3 PhysicsWorld::getGravity() const {
    return m_settings.gravity;
}

void PhysicsWorld::addRigidBody(btRigidBody* body) {
    m_world->addRigidBody(body);
    m_rigidBodies.push_back(body);
}

void PhysicsWorld::removeRigidBody(btRigidBody* body) {
    m_world->removeRigidBody(body);
    auto it = std::find(m_rigidBodies.begin(), m_rigidBodies.end(), body);
    if (it != m_rigidBodies.end()) {
        m_rigidBodies.erase(it);
    }
}

void PhysicsWorld::addVehicle(btRaycastVehicle* vehicle) {
    m_world->addVehicle(vehicle);
    m_vehicles.push_back(vehicle);
}

void PhysicsWorld::removeVehicle(btRaycastVehicle* vehicle) {
    m_world->removeVehicle(vehicle);
    auto it = std::find(m_vehicles.begin(), m_vehicles.end(), vehicle);
    if (it != m_vehicles.end()) {
        m_vehicles.erase(it);
    }
}

bool PhysicsWorld::raycast(const vec3& from, const vec3& to, vec3& hitPoint, vec3& hitNormal,
                           void** hitObject, short collisionGroup, short collisionMask) {
    btVector3 btFrom(from.x, from.y, from.z);
    btVector3 btTo(to.x, to.y, to.z);
    
    btCollisionWorld::ClosestRayResultCallback callback(btFrom, btTo);
    
    if (collisionGroup >= 0 && collisionMask >= 0) {
        callback.m_collisionFilterGroup = collisionGroup;
        callback.m_collisionFilterMask = collisionMask;
    }
    
    m_world->rayTest(btFrom, btTo, callback);
    
    if (callback.hasHit()) {
        hitPoint = vec3(
            callback.m_hitPointWorld.x(),
            callback.m_hitPointWorld.y(),
            callback.m_hitPointWorld.z()
        );
        hitNormal = vec3(
            callback.m_hitNormalWorld.x(),
            callback.m_hitNormalWorld.y(),
            callback.m_hitNormalWorld.z()
        );
        if (hitObject) {
            *hitObject = const_cast<void*>(static_cast<const void*>(callback.m_collisionObject));
        }
        return true;
    }
    
    return false;
}

bool PhysicsWorld::sphereCast(const vec3& from, const vec3& to, float radius,
                               vec3& hitPoint, vec3& hitNormal) {
    btVector3 btFrom(from.x, from.y, from.z);
    btVector3 btTo(to.x, to.y, to.z);
    btSphereShape sphere(radius);
    
    btCollisionWorld::ClosestConvexResultCallback callback(btFrom, btTo);
    m_world->convexSweepTest(&sphere, btTransform(btQuaternion::getIdentity(), btFrom),
                              btTransform(btQuaternion::getIdentity(), btTo), callback);
    
    if (callback.hasHit()) {
        hitPoint = vec3(
            callback.m_hitPointWorld.x(),
            callback.m_hitPointWorld.y(),
            callback.m_hitPointWorld.z()
        );
        hitNormal = vec3(
            callback.m_hitNormalWorld.x(),
            callback.m_hitNormalWorld.y(),
            callback.m_hitNormalWorld.z()
        );
        return true;
    }
    
    return false;
}

void PhysicsWorld::setSettings(const PhysicsSettings& settings) {
    m_settings = settings;
    setGravity(settings.gravity);
}

void PhysicsWorld::setDebugDrawEnabled(bool enabled) {
    m_debugDrawEnabled = enabled;
}

}
