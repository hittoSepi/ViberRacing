#pragma once

#include "core/types.hpp"
#include "renderer/mesh.hpp"
#include <bgfx/bgfx.h>

namespace viber {

struct AtmosphereSettings {
    vec3 skyTopColor = vec3(0.10f, 0.32f, 0.78f);
    vec3 skyBottomColor = vec3(0.82f, 0.90f, 1.00f);
    vec3 horizonColor = vec3(0.98f, 0.78f, 0.58f);
    vec3 sunDirection = glm::normalize(vec3(0.35f, 0.72f, 0.35f));
    vec3 sunColor = vec3(1.00f, 0.93f, 0.80f);
    float sunSize = 0.015f;
    float sunIntensity = 1.0f;
    float horizonSoftness = 3.0f;
    float horizonStrength = 0.22f;
    float aerialStrength = 0.12f;
    float zenithCurve = 0.35f;
};

// Fullscreen atmosphere/sky gradient renderer
// Renders before all 3D objects with depth test disabled
class Atmosphere {
public:
    Atmosphere() = default;
    ~Atmosphere() { destroy(); }

    Atmosphere(const Atmosphere&) = delete;
    Atmosphere& operator=(const Atmosphere&) = delete;

    void init();
    void destroy();

    // Render atmosphere - call first in frame
    // Uses current view/projection matrices
    void render(bgfx::ViewId viewId, bgfx::ProgramHandle program, const mat4& view, const mat4& proj);

    // Set colors (can be called each frame for dynamic sky)
    void setColors(const vec3& top, const vec3& bottom);
    void setSun(const vec3& direction, const vec3& color, float size);
    void setTimeOfDay(float timeOfDay);

    AtmosphereSettings& settings() { return m_settings; }
    const AtmosphereSettings& settings() const { return m_settings; }

private:
    Mesh m_fullscreenQuad;
    bgfx::UniformHandle u_skyTop = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_skyBottom = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_horizonColor = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_sunDir = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_sunColor = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_atmoParams0 = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_atmoParams1 = BGFX_INVALID_HANDLE;

    AtmosphereSettings m_settings;
    
    bool m_initialized = false;
};

} // namespace viber
