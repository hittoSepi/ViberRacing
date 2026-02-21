#pragma once

#include "core/types.hpp"
#include <glm/gtc/quaternion.hpp>
#include <cmath>

namespace viber {

inline vec3 lerp(const vec3& a, const vec3& b, float t) {
    return a + (b - a) * t;
}

inline quat slerp(const quat& a, const quat& b, float t) {
    return glm::slerp(a, b, t);
}

inline mat4 composeMatrix(const vec3& position, const quat& rotation, const vec3& scale) {
    mat4 m = glm::mat4_cast(rotation);
    m = glm::scale(m, scale);
    m[3] = vec4(position, 1.0f);
    return m;
}

inline void decomposeMatrix(const mat4& matrix, vec3& position, quat& rotation, vec3& scale) {
    position = vec3(matrix[3]);
    
    vec3 col0(matrix[0]);
    vec3 col1(matrix[1]);
    vec3 col2(matrix[2]);
    
    scale.x = glm::length(col0);
    scale.y = glm::length(col1);
    scale.z = glm::length(col2);
    
    if (scale.x != 0.0f) col0 /= scale.x;
    if (scale.y != 0.0f) col1 /= scale.y;
    if (scale.z != 0.0f) col2 /= scale.z;
    
    mat3 rotMat(col0, col1, col2);
    rotation = glm::quat_cast(rotMat);
}

inline float clamp01(float value) {
    return glm::clamp(value, 0.0f, 1.0f);
}

inline vec3 clampMagnitude(const vec3& v, float maxLength) {
    float mag = glm::length(v);
    if (mag > maxLength && mag > 0.0f) {
        return v * (maxLength / mag);
    }
    return v;
}

inline float smoothstep(float edge0, float edge1, float x) {
    float t = clamp01((x - edge0) / (edge1 - edge0));
    return t * t * (3.0f - 2.0f * t);
}

inline float smootherstep(float edge0, float edge1, float x) {
    float t = clamp01((x - edge0) / (edge1 - edge0));
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

inline vec3 reflect(const vec3& v, const vec3& normal) {
    return v - 2.0f * glm::dot(v, normal) * normal;
}

inline vec3 projectOnPlane(const vec3& v, const vec3& normal) {
    return v - glm::dot(v, normal) * normal;
}

inline bool isFinite(float v) {
    return std::isfinite(v);
}

inline bool isFinite(const vec3& v) {
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}

}
