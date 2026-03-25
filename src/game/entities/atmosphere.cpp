#include "atmosphere.hpp"
#include <spdlog/spdlog.h>

namespace viber {

void Atmosphere::init() {
    if (m_initialized) return;

    // Create fullscreen quad (triangle covering entire screen)
    // Using a single large triangle is more efficient than two triangles
    std::vector<Vertex> vertices = {
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {}, {}},
        {{ 3.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {2.0f, 0.0f}, {}, {}},
        {{-1.0f,  3.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 2.0f}, {}, {}},
    };
    
    std::vector<u32> indices = {0, 1, 2};
    
    m_fullscreenQuad.create(vertices, indices);
    
    // Create uniforms
    u_skyTop = bgfx::createUniform("u_skyTop", bgfx::UniformType::Vec4);
    u_skyBottom = bgfx::createUniform("u_skyBottom", bgfx::UniformType::Vec4);
    u_horizonColor = bgfx::createUniform("u_horizonColor", bgfx::UniformType::Vec4);
    u_sunDir = bgfx::createUniform("u_sunDir", bgfx::UniformType::Vec4);
    u_sunColor = bgfx::createUniform("u_sunColor", bgfx::UniformType::Vec4);
    u_atmoParams0 = bgfx::createUniform("u_atmoParams0", bgfx::UniformType::Vec4);
    u_atmoParams1 = bgfx::createUniform("u_atmoParams1", bgfx::UniformType::Vec4);
    
    m_initialized = true;
    spdlog::info("Atmosphere renderer initialized");
}

void Atmosphere::destroy() {
    if (bgfx::isValid(u_skyTop)) {
        bgfx::destroy(u_skyTop);
        u_skyTop = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(u_skyBottom)) {
        bgfx::destroy(u_skyBottom);
        u_skyBottom = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(u_horizonColor)) {
        bgfx::destroy(u_horizonColor);
        u_horizonColor = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(u_sunDir)) {
        bgfx::destroy(u_sunDir);
        u_sunDir = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(u_sunColor)) {
        bgfx::destroy(u_sunColor);
        u_sunColor = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(u_atmoParams0)) {
        bgfx::destroy(u_atmoParams0);
        u_atmoParams0 = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(u_atmoParams1)) {
        bgfx::destroy(u_atmoParams1);
        u_atmoParams1 = BGFX_INVALID_HANDLE;
    }
    
    m_fullscreenQuad.destroy();
    m_initialized = false;
}

void Atmosphere::render(bgfx::ViewId viewId, bgfx::ProgramHandle program, const mat4& view, const mat4& proj) {
    if (!m_initialized || !bgfx::isValid(program)) return;

    bgfx::setViewTransform(viewId, &view, &proj);

    // Render without depth test (always behind everything)
    u64 state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | 
                BGFX_STATE_DEPTH_TEST_ALWAYS;

    vec4 top4(m_settings.skyTopColor, 1.0f);
    vec4 bottom4(m_settings.skyBottomColor, 1.0f);
    vec4 horizon4(m_settings.horizonColor, 1.0f);
    vec4 sunDir4(glm::normalize(m_settings.sunDirection), m_settings.sunSize);
    vec4 sunColor4(m_settings.sunColor, m_settings.sunIntensity);
    vec4 atmoParams0(
        m_settings.horizonSoftness,
        m_settings.horizonStrength,
        m_settings.aerialStrength,
        m_settings.zenithCurve
    );
    vec4 atmoParams1(24.0f, 96.0f, 0.65f, 0.35f);

    bgfx::setUniform(u_skyTop, &top4);
    bgfx::setUniform(u_skyBottom, &bottom4);
    bgfx::setUniform(u_horizonColor, &horizon4);
    bgfx::setUniform(u_sunDir, &sunDir4);
    bgfx::setUniform(u_sunColor, &sunColor4);
    bgfx::setUniform(u_atmoParams0, &atmoParams0);
    bgfx::setUniform(u_atmoParams1, &atmoParams1);

    mat4 identity(1.0f);
    bgfx::setTransform(&identity);
    bgfx::setState(state);
    bgfx::setVertexBuffer(0, m_fullscreenQuad.getVertexBuffer());
    bgfx::setIndexBuffer(m_fullscreenQuad.getIndexBuffer());
    bgfx::submit(viewId, program);
}

void Atmosphere::setColors(const vec3& top, const vec3& bottom) {
    m_settings.skyTopColor = top;
    m_settings.skyBottomColor = bottom;
}

void Atmosphere::setSun(const vec3& direction, const vec3& color, float size) {
    m_settings.sunDirection = glm::normalize(direction);
    m_settings.sunColor = color;
    m_settings.sunSize = size;
}

void Atmosphere::setTimeOfDay(float timeOfDay) {
    const float angle = glm::mix(-0.35f * PI, 1.10f * PI, glm::clamp(timeOfDay, 0.0f, 1.0f));
    m_settings.sunDirection = glm::normalize(vec3(std::cos(angle), std::sin(angle), 0.25f));

    const float daylight = glm::smoothstep(-0.18f, 0.18f, m_settings.sunDirection.y);
    const float dusk = 1.0f - std::abs(daylight * 2.0f - 1.0f);

    const vec3 nightTop(0.015f, 0.025f, 0.07f);
    const vec3 nightBottom(0.05f, 0.07f, 0.12f);
    const vec3 dayTop(0.10f, 0.32f, 0.78f);
    const vec3 dayBottom(0.82f, 0.90f, 1.00f);
    const vec3 duskTop(0.26f, 0.16f, 0.42f);
    const vec3 duskBottom(1.00f, 0.56f, 0.28f);
    const vec3 dayHorizon(0.98f, 0.78f, 0.58f);
    const vec3 duskHorizon(1.00f, 0.42f, 0.22f);
    const vec3 nightHorizon(0.10f, 0.11f, 0.18f);

    vec3 skyTop = glm::mix(nightTop, dayTop, daylight);
    vec3 skyBottom = glm::mix(nightBottom, dayBottom, daylight);
    vec3 horizon = glm::mix(nightHorizon, dayHorizon, daylight);

    skyTop = glm::mix(skyTop, duskTop, dusk * 0.75f);
    skyBottom = glm::mix(skyBottom, duskBottom, dusk * 0.75f);
    horizon = glm::mix(horizon, duskHorizon, dusk);

    m_settings.skyTopColor = skyTop;
    m_settings.skyBottomColor = skyBottom;
    m_settings.horizonColor = horizon;
    m_settings.sunColor = glm::mix(vec3(1.00f, 0.82f, 0.58f), vec3(1.00f, 0.96f, 0.88f), daylight);
    m_settings.sunIntensity = glm::mix(0.10f, 1.15f, daylight) + dusk * 0.35f;
    m_settings.sunSize = glm::mix(0.010f, 0.020f, dusk);
    m_settings.horizonStrength = glm::mix(0.14f, 0.24f, daylight) + dusk * 0.10f;
    m_settings.horizonSoftness = glm::mix(4.8f, 2.8f, daylight);
    m_settings.aerialStrength = glm::mix(0.05f, 0.14f, daylight);
    m_settings.zenithCurve = glm::mix(0.55f, 0.32f, daylight);
}

} // namespace viber
