#include "car.hpp"
#include "physics/world.hpp"
#include <btBulletDynamicsCommon.h>
#include <spdlog/spdlog.h>
#include <array>

namespace viber {

Car::Car() {
    m_vehicle = std::make_unique<Vehicle>();
}

Car::~Car() {
    shutdown();
}

void Car::init(PhysicsWorld* world) {
    init(world, CarDefinition::makeDefault());
}

void Car::init(PhysicsWorld* world, const CarDefinition& def) {
    m_world = world;

    VehicleParams params;
    params.mass = 1200.0f;
    params.maxEngineForce = 4000.0f;
    params.maxBrakeForce = 1500.0f;
    params.maxSteeringAngle = 0.7f;

    m_vehicle->create(world, params);

    m_carBody.init(def);
    m_damage.init(&m_carBody, m_vehicle.get());
}

void Car::shutdown() {
    m_carBody.shutdown();
    m_vehicle.reset();
    m_world = nullptr;
}

void Car::update(float deltaTime) {
    m_vehicle->update(deltaTime);
    processCollisions();
    m_carBody.update(deltaTime);
}

void Car::render(bgfx::ViewId viewId, bgfx::ProgramHandle program) const {
    std::array<mat4, 4> wheelTransforms;
    for (int i = 0; i < 4; ++i)
        wheelTransforms[i] = m_vehicle->getWheelTransform(i);

    m_carBody.render(viewId, program, m_vehicle->getWorldTransform(), wheelTransforms);
}

void Car::processCollisions() {
    if (!m_world) return;

    btCollisionDispatcher* dispatcher =
        static_cast<btCollisionDispatcher*>(m_world->getWorld()->getDispatcher());
    if (!dispatcher) return;

    const btRigidBody* chassis = m_vehicle->getChassisBody();
    int numManifolds = dispatcher->getNumManifolds();

    for (int i = 0; i < numManifolds; ++i) {
        btPersistentManifold* manifold = dispatcher->getManifoldByIndexInternal(i);
        const btCollisionObject* bodyA = manifold->getBody0();
        const btCollisionObject* bodyB = manifold->getBody1();

        if (bodyA != chassis && bodyB != chassis) continue;
        bool chassisIsA = (bodyA == chassis);

        for (int j = 0; j < manifold->getNumContacts(); ++j) {
            const btManifoldPoint& pt = manifold->getContactPoint(j);
            float impulse = pt.getAppliedImpulse();
            if (impulse < 1.0f) continue;

            btVector3 worldPos = chassisIsA
                ? pt.getPositionWorldOnA()
                : pt.getPositionWorldOnB();

            // Transform to local car space
            mat4 invWorld = glm::inverse(m_vehicle->getWorldTransform());
            vec4 localPos4 = invWorld * vec4{worldPos.x(), worldPos.y(), worldPos.z(), 1.0f};
            vec3 localPos{localPos4.x, localPos4.y, localPos4.z};

            m_damage.processImpact(impulse, localPos);
        }
    }
}

void Car::resetDamage() {
    m_damage.reset();
}

void Car::setThrottle(float value) { m_vehicle->setThrottle(value); }
void Car::setBrake(float value)    { m_vehicle->setBrake(value); }
void Car::setSteering(float value) { m_vehicle->setSteering(value); }

void Car::reset(const vec3& position, const vec3& forward) {
    m_vehicle->reset(position, forward);
    m_damage.reset();
}

vec3  Car::getPosition()  const { return m_vehicle->getPosition(); }
quat  Car::getRotation()  const { return m_vehicle->getRotation(); }
vec3  Car::getVelocity()  const { return m_vehicle->getVelocity(); }
float Car::getSpeedKmh()  const { return m_vehicle->getSpeedKmh(); }
mat4  Car::getTransform() const { return m_vehicle->getWorldTransform(); }
mat4  Car::getWheelTransform(int index) const { return m_vehicle->getWheelTransform(index); }

}
