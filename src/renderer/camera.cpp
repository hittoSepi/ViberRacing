#include "camera.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace viber {

Camera::Camera() {
    m_viewMatrix = mat4(1.0f);
    m_projectionMatrix = mat4(1.0f);
}

void Camera::setPosition(const vec3& position) {
    m_position = position;
    m_viewDirty = true;
}

void Camera::setTarget(const vec3& target) {
    m_target = target;
    m_viewDirty = true;
}

void Camera::setUp(const vec3& up) {
    m_up = up;
    m_viewDirty = true;
}

void Camera::setPerspective(float fovY, float aspectRatio, float nearPlane, float farPlane) {
    m_fovY = fovY;
    m_aspectRatio = aspectRatio;
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
    m_isPerspective = true;
    m_projectionDirty = true;
}

void Camera::setOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    m_orthoLeft = left;
    m_orthoRight = right;
    m_orthoBottom = bottom;
    m_orthoTop = top;
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
    m_isPerspective = false;
    m_projectionDirty = true;
}

void Camera::setFOV(float fovY) {
    m_fovY = fovY;
    m_projectionDirty = true;
}

void Camera::setAspectRatio(float aspectRatio) {
    m_aspectRatio = aspectRatio;
    m_projectionDirty = true;
}

void Camera::setNearFar(float nearPlane, float farPlane) {
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
    m_projectionDirty = true;
}

mat4 Camera::getViewMatrix() const {
    updateViewMatrix();
    return m_viewMatrix;
}

mat4 Camera::getProjectionMatrix() const {
    updateProjectionMatrix();
    return m_projectionMatrix;
}

mat4 Camera::getViewProjectionMatrix() const {
    return getProjectionMatrix() * getViewMatrix();
}

void Camera::lookAt(const vec3& position, const vec3& target, const vec3& up) {
    m_position = position;
    m_target = target;
    m_up = up;
    m_viewDirty = true;
}

void Camera::orbit(const vec3& target, float azimuth, float elevation, float distance) {
    m_target = target;
    
    float x = distance * std::cos(elevation) * std::sin(azimuth);
    float y = distance * std::sin(elevation);
    float z = distance * std::cos(elevation) * std::cos(azimuth);
    
    m_position = target + vec3(x, y, z);
    m_viewDirty = true;
}

void Camera::pan(const vec2& delta) {
    vec3 right = getRight();
    vec3 up = m_up;
    
    vec3 movement = right * delta.x + up * delta.y;
    m_position += movement;
    m_target += movement;
    m_viewDirty = true;
}

void Camera::zoom(float factor) {
    vec3 direction = m_target - m_position;
    float distance = glm::length(direction);
    direction = glm::normalize(direction);
    
    distance *= factor;
    distance = glm::max(distance, m_nearPlane + 0.1f);
    
    m_position = m_target - direction * distance;
    m_viewDirty = true;
}

void Camera::moveForward(float distance) {
    vec3 forward = getForward();
    m_position += forward * distance;
    m_target += forward * distance;
    m_viewDirty = true;
}

void Camera::moveRight(float distance) {
    vec3 right = getRight();
    m_position += right * distance;
    m_target += right * distance;
    m_viewDirty = true;
}

void Camera::moveUp(float distance) {
    m_position += m_up * distance;
    m_target += m_up * distance;
    m_viewDirty = true;
}

void Camera::updateViewMatrix() const {
    if (m_viewDirty) {
        m_viewMatrix = glm::lookAt(m_position, m_target, m_up);
        m_viewDirty = false;
    }
}

void Camera::updateProjectionMatrix() const {
    if (m_projectionDirty) {
        if (m_isPerspective) {
            m_projectionMatrix = glm::perspective(m_fovY, m_aspectRatio, m_nearPlane, m_farPlane);
        } else {
            m_projectionMatrix = glm::ortho(m_orthoLeft, m_orthoRight, 
                                            m_orthoBottom, m_orthoTop, 
                                            m_nearPlane, m_farPlane);
        }
        m_projectionDirty = false;
    }
}

Camera::Frustum Camera::getFrustum() const {
    Frustum frustum;
    mat4 vp = getViewProjectionMatrix();
    
    frustum.planes[0] = glm::row(vp, 3) + glm::row(vp, 0);
    frustum.planes[1] = glm::row(vp, 3) - glm::row(vp, 0);
    frustum.planes[2] = glm::row(vp, 3) + glm::row(vp, 1);
    frustum.planes[3] = glm::row(vp, 3) - glm::row(vp, 1);
    frustum.planes[4] = glm::row(vp, 3) + glm::row(vp, 2);
    frustum.planes[5] = glm::row(vp, 3) - glm::row(vp, 2);
    
    for (int i = 0; i < 6; ++i) {
        float length = glm::length(vec3(frustum.planes[i]));
        if (length > 0.0f) {
            frustum.planes[i] /= length;
        }
    }
    
    return frustum;
}

bool Camera::Frustum::containsPoint(const vec3& point) const {
    for (int i = 0; i < 6; ++i) {
        if (glm::dot(vec3(planes[i]), point) + planes[i].w < 0.0f) {
            return false;
        }
    }
    return true;
}

bool Camera::Frustum::containsSphere(const vec3& center, float radius) const {
    for (int i = 0; i < 6; ++i) {
        if (glm::dot(vec3(planes[i]), center) + planes[i].w < -radius) {
            return false;
        }
    }
    return true;
}

bool Camera::Frustum::containsAABB(const vec3& min, const vec3& max) const {
    vec3 corners[8] = {
        {min.x, min.y, min.z}, {max.x, min.y, min.z},
        {min.x, max.y, min.z}, {max.x, max.y, min.z},
        {min.x, min.y, max.z}, {max.x, min.y, max.z},
        {min.x, max.y, max.z}, {max.x, max.y, max.z}
    };
    
    for (int i = 0; i < 6; ++i) {
        bool allOutside = true;
        for (int j = 0; j < 8; ++j) {
            if (glm::dot(vec3(planes[i]), corners[j]) + planes[i].w >= 0.0f) {
                allOutside = false;
                break;
            }
        }
        if (allOutside) return false;
    }
    return true;
}

}
