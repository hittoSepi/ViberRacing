#include "vehicle.hpp"
#include "world.hpp"
#include <spdlog/spdlog.h>

namespace viber {

Vehicle::Vehicle() {
    m_tireModel = std::make_unique<TireModel>();
}

Vehicle::~Vehicle() {
    destroy();
}

bool Vehicle::create(PhysicsWorld* world, const VehicleParams& params) {
    if (m_created) {
        destroy();
    }
    
    m_world = world;
    m_params = params;
    
    btVector3 chassisExtents(
        params.chassisWidth * 0.5f,
        params.chassisHeight * 0.5f,
        params.chassisLength * 0.5f
    );
    
    m_chassisShape = std::make_unique<btBoxShape>(chassisExtents);
    
    btVector3 localInertia(0, 0, 0);
    m_chassisShape->calculateLocalInertia(params.mass, localInertia);
    
    btTransform startTransform;
    startTransform.setIdentity();
    startTransform.setOrigin(btVector3(0, params.chassisOffsetY + params.suspensionRestLength + params.wheelRadius, 0));
    
    btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(params.mass, motionState, m_chassisShape.get(), localInertia);
    
    m_chassisBody = std::make_unique<btRigidBody>(rbInfo);
    m_chassisBody->setActivationState(DISABLE_DEACTIVATION);
    
    m_world->addRigidBody(m_chassisBody.get());
    
    m_vehicleRaycaster = std::make_unique<btDefaultVehicleRaycaster>(world->getWorld());
    
    btRaycastVehicle::btVehicleTuning tuning;
    m_vehicle = std::make_unique<btRaycastVehicle>(tuning, m_chassisBody.get(), m_vehicleRaycaster.get());
    
    m_vehicle->setCoordinateSystem(0, 1, 2);
    
    m_world->addVehicle(m_vehicle.get());
    
    for (int i = 0; i < 4; ++i) {
        btVector3 connectionPoint(
            params.wheelPositions[i].x,
            params.wheelPositions[i].y,
            params.wheelPositions[i].z
        );
        
        bool isFront = params.isFrontWheel[i];
        
        btVector3 wheelDirection(0, -1, 0);
        btVector3 wheelAxle(-1, 0, 0);
        
        tuning.m_suspensionStiffness = params.suspensionStiffness;
        tuning.m_suspensionDamping = params.suspensionDamping;
        tuning.m_suspensionCompression = params.suspensionCompression;
        tuning.m_maxSuspensionTravelCm = params.maxSuspensionTravel * 100.0f;
        tuning.m_maxSuspensionForce = params.maxSuspensionForce;
        tuning.m_frictionSlip = params.wheelFriction;
        
        m_vehicle->addWheel(
            connectionPoint,
            wheelDirection,
            wheelAxle,
            params.suspensionRestLength,
            params.wheelRadius,
            tuning,
            isFront
        );
    }
    
    m_created = true;
    spdlog::info("Vehicle created with mass {} kg", params.mass);
    
    return true;
}

void Vehicle::destroy() {
    if (!m_created) return;
    
    if (m_vehicle) {
        m_world->removeVehicle(m_vehicle.get());
        m_vehicle.reset();
    }
    
    if (m_chassisBody) {
        m_world->removeRigidBody(m_chassisBody.get());
        m_chassisBody.reset();
    }
    
    m_chassisShape.reset();
    m_vehicleRaycaster.reset();
    
    m_created = false;
}

void Vehicle::setThrottle(float throttle) {
    float maxForce = m_maxEngineForceOverride >= 0.0f ? m_maxEngineForceOverride : m_params.maxEngineForce;
    m_state.engineForce = maxForce * glm::clamp(throttle, -1.0f, 1.0f);
    
    for (int i = 2; i < 4; ++i) {
        m_vehicle->applyEngineForce(m_state.engineForce, i);
    }
}

void Vehicle::setBrake(float brake) {
    m_state.brakeForce = m_params.maxBrakeForce * glm::clamp(brake, 0.0f, 1.0f);
    
    for (int i = 0; i < 4; ++i) {
        m_vehicle->setBrake(m_state.brakeForce, i);
    }
}

void Vehicle::setSteering(float steering) {
    m_state.steeringAngle = m_params.maxSteeringAngle * glm::clamp(steering, -1.0f, 1.0f);
    
    m_vehicle->setSteeringValue(m_state.steeringAngle, 0);
    m_vehicle->setSteeringValue(m_state.steeringAngle, 1);
}

void Vehicle::setPosition(const vec3& position) {
    btTransform transform = m_chassisBody->getWorldTransform();
    transform.setOrigin(btVector3(position.x, position.y, position.z));
    m_chassisBody->setWorldTransform(transform);
    
    if (m_chassisBody->getMotionState()) {
        m_chassisBody->getMotionState()->setWorldTransform(transform);
    }
}

void Vehicle::setRotation(const quat& rotation) {
    btTransform transform = m_chassisBody->getWorldTransform();
    transform.setRotation(btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w));
    m_chassisBody->setWorldTransform(transform);
}

void Vehicle::reset(const vec3& position, const vec3& forward) {
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(btVector3(position.x, position.y, position.z));
    
    if (glm::length2(forward) > 0.001f) {
        vec3 f = glm::normalize(forward);
        vec3 up(0.0f, 1.0f, 0.0f);
        vec3 right = glm::cross(up, f);
        if (glm::length2(right) < 0.001f) {
            up = vec3(0.0f, 0.0f, 1.0f);
            right = glm::cross(up, f);
        }
        right = glm::normalize(right);
        f = glm::cross(right, up);
        
        mat3 rotMatrix(right, up, f);
        quat rotation = glm::quat_cast(rotMatrix);
        transform.setRotation(btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w));
    }
    
    m_chassisBody->setWorldTransform(transform);
    m_chassisBody->setLinearVelocity(btVector3(0, 0, 0));
    m_chassisBody->setAngularVelocity(btVector3(0, 0, 0));
    m_chassisBody->activate();
}

void Vehicle::update(float deltaTime) {
    if (!m_created) return;
    
    updateState();
    applyTireForces();
}

void Vehicle::updateState() {
    btTransform transform = m_chassisBody->getWorldTransform();
    
    m_state.position = vec3(
        transform.getOrigin().x(),
        transform.getOrigin().y(),
        transform.getOrigin().z()
    );
    
    m_state.rotation = quat(
        transform.getRotation().w(),
        transform.getRotation().x(),
        transform.getRotation().y(),
        transform.getRotation().z()
    );
    
    const btVector3& vel = m_chassisBody->getLinearVelocity();
    m_state.velocity = vec3(vel.x(), vel.y(), vel.z());
    
    const btVector3& angVel = m_chassisBody->getAngularVelocity();
    m_state.angularVelocity = vec3(angVel.x(), angVel.y(), angVel.z());
    
    vec3 forward = getForward();
    m_state.speed = glm::dot(m_state.velocity, forward);
    m_state.speedKmh = m_state.speed * 3.6f;
    
    for (int i = 0; i < 4; ++i) {
        const btWheelInfo& wheelInfo = m_vehicle->getWheelInfo(i);
        m_state.wheelSuspensionLength[i] = wheelInfo.m_raycastInfo.m_suspensionLength;
        m_state.wheelOnGround[i] = wheelInfo.m_raycastInfo.m_isInContact;
        
        m_state.lateralSlip[i] = std::abs(wheelInfo.m_skidInfo);
    }
}

void Vehicle::applyTireForces() {
    if (!m_tireModel) return;
    
    for (int i = 0; i < 4; ++i) {
        btWheelInfo& wheelInfo = m_vehicle->getWheelInfo(i);
        
        if (!wheelInfo.m_raycastInfo.m_isInContact) continue;
        
        // Calculate suspension force from wheel info
        float normalForce = wheelInfo.m_wheelsSuspensionForce;
        
        btVector3 wheelForward = wheelInfo.m_raycastInfo.m_wheelDirectionWS;
        btVector3 wheelAxle = wheelInfo.m_raycastInfo.m_wheelAxleWS;
        btVector3 contactVel = m_chassisBody->getVelocityInLocalPoint(
            wheelInfo.m_raycastInfo.m_contactPointWS - m_chassisBody->getWorldTransform().getOrigin()
        );
        
        float forwardVel = contactVel.dot(wheelForward);
        float lateralVel = contactVel.dot(wheelAxle);
        
        float slipRatio = 0.0f;
        float slipAngle = 0.0f;
        
        if (std::abs(forwardVel) > 0.1f) {
            slipRatio = (m_state.engineForce > 0 ? 1.0f : -1.0f) * 
                        std::abs(lateralVel) / std::abs(forwardVel);
            slipAngle = std::atan2(lateralVel, forwardVel);
        }
        
        float grip = m_tireModel->calculateGrip(slipRatio, slipAngle, normalForce);
        float baseFriction = m_wheelFrictionOverride[i] >= 0.0f
            ? m_wheelFrictionOverride[i]
            : m_params.wheelFriction;
        wheelInfo.m_frictionSlip = baseFriction * grip;
        
        m_state.lateralSlip[i] = slipAngle;
        m_state.longitudinalSlip[i] = slipRatio;
    }
}

vec3 Vehicle::getPosition() const {
    return m_state.position;
}

quat Vehicle::getRotation() const {
    return m_state.rotation;
}

vec3 Vehicle::getVelocity() const {
    return m_state.velocity;
}

vec3 Vehicle::getForward() const {
    mat3 rot = glm::mat3_cast(m_state.rotation);
    return -rot[2];
}

vec3 Vehicle::getRight() const {
    mat3 rot = glm::mat3_cast(m_state.rotation);
    return rot[0];
}

vec3 Vehicle::getUp() const {
    mat3 rot = glm::mat3_cast(m_state.rotation);
    return rot[1];
}

float Vehicle::getSpeed() const {
    return m_state.speed;
}

float Vehicle::getSpeedKmh() const {
    return m_state.speedKmh;
}

void Vehicle::setTireModel(std::unique_ptr<TireModel> model) {
    m_tireModel = std::move(model);
}

void Vehicle::setWheelFriction(int wheelIndex, float friction) {
    if (wheelIndex < 0 || wheelIndex >= 4) return;
    m_wheelFrictionOverride[wheelIndex] = friction;
}

void Vehicle::setMaxEngineForce(float maxForce) {
    m_maxEngineForceOverride = maxForce;
}

mat4 Vehicle::getWorldTransform() const {
    mat4 translation = glm::translate(mat4(1.0f), m_state.position);
    mat4 rotation = glm::mat4_cast(m_state.rotation);
    return translation * rotation;
}

mat4 Vehicle::getWheelTransform(int wheelIndex) const {
    if (wheelIndex < 0 || wheelIndex >= 4 || !m_vehicle) {
        return mat4(1.0f);
    }
    
    m_vehicle->updateWheelTransform(wheelIndex, false);
    
    const btTransform& wheelTransform = m_vehicle->getWheelInfo(wheelIndex).m_worldTransform;
    
    vec3 pos(
        wheelTransform.getOrigin().x(),
        wheelTransform.getOrigin().y(),
        wheelTransform.getOrigin().z()
    );
    
    quat rot(
        wheelTransform.getRotation().w(),
        wheelTransform.getRotation().x(),
        wheelTransform.getRotation().y(),
        wheelTransform.getRotation().z()
    );
    
    return glm::translate(mat4(1.0f), pos) * glm::mat4_cast(rot);
}

}
