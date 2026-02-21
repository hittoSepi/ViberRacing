#pragma once

#include "core/types.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace viber {

class Camera {
public:
    Camera();
    
    void setPosition(const vec3& position);
    void setTarget(const vec3& target);
    void setUp(const vec3& up);
    
    void setPerspective(float fovY, float aspectRatio, float nearPlane, float farPlane);
    void setOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);
    
    void setFOV(float fovY);
    void setAspectRatio(float aspectRatio);
    void setNearFar(float nearPlane, float farPlane);
    
    const vec3& getPosition() const { return m_position; }
    const vec3& getTarget() const { return m_target; }
    const vec3& getUp() const { return m_up; }
    vec3 getForward() const { return glm::normalize(m_target - m_position); }
    vec3 getRight() const { return glm::normalize(glm::cross(getForward(), m_up)); }
    
    mat4 getViewMatrix() const;
    mat4 getProjectionMatrix() const;
    mat4 getViewProjectionMatrix() const;
    
    float getFOV() const { return m_fovY; }
    float getAspectRatio() const { return m_aspectRatio; }
    float getNearPlane() const { return m_nearPlane; }
    float getFarPlane() const { return m_farPlane; }
    
    void lookAt(const vec3& position, const vec3& target, const vec3& up = vec3(0.0f, 1.0f, 0.0f));
    
    void orbit(const vec3& target, float azimuth, float elevation, float distance);
    void pan(const vec2& delta);
    void zoom(float factor);
    
    void moveForward(float distance);
    void moveRight(float distance);
    void moveUp(float distance);
    
    struct Frustum {
        vec4 planes[6];
        
        bool containsPoint(const vec3& point) const;
        bool containsSphere(const vec3& center, float radius) const;
        bool containsAABB(const vec3& min, const vec3& max) const;
    };
    
    Frustum getFrustum() const;
    
private:
    vec3 m_position{0.0f, 5.0f, -10.0f};
    vec3 m_target{0.0f, 0.0f, 0.0f};
    vec3 m_up{0.0f, 1.0f, 0.0f};
    
    float m_fovY = 60.0f * DEG_TO_RAD;
    float m_aspectRatio = 16.0f / 9.0f;
    float m_nearPlane = 0.1f;
    float m_farPlane = 1000.0f;
    
    bool m_isPerspective = true;
    float m_orthoLeft = -10.0f;
    float m_orthoRight = 10.0f;
    float m_orthoBottom = -10.0f;
    float m_orthoTop = 10.0f;
    
    mutable bool m_viewDirty = true;
    mutable bool m_projectionDirty = true;
    mutable mat4 m_viewMatrix{};
    mutable mat4 m_projectionMatrix{};
    
    void updateViewMatrix() const;
    void updateProjectionMatrix() const;
};

}
