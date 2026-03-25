#include "ground_plane.hpp"
#include <spdlog/spdlog.h>

namespace viber {

void GroundPlane::init(float size, u32 gridLines) {
    if (m_initialized) return;

    float half = size * 0.5f;
    float lineSpacing = size / gridLines;

    const vec3 planeColor{0.16f, 0.20f, 0.26f};
    const vec3 gridColor{0.32f, 0.40f, 0.50f};
    const vec3 axisColor{0.55f, 0.68f, 0.82f};

    std::vector<VertexSimple> vertices;
    std::vector<u32> indices;

    float y = -0.5f;
    u32 baseIdx = static_cast<u32>(vertices.size());

    VertexSimple v;
    v.color = planeColor;
    v.position = vec3(-half, y, -half);
    vertices.push_back(v);
    v.position = vec3( half, y, -half);
    vertices.push_back(v);
    v.position = vec3( half, y,  half);
    vertices.push_back(v);
    v.position = vec3(-half, y,  half);
    vertices.push_back(v);
    
    indices.push_back(baseIdx + 0);
    indices.push_back(baseIdx + 1);
    indices.push_back(baseIdx + 2);
    indices.push_back(baseIdx + 0);
    indices.push_back(baseIdx + 2);
    indices.push_back(baseIdx + 3);

    float lineWidth = size * 0.002f;
    float yOffset = y + 0.001f;

    for (u32 i = 0; i <= gridLines; ++i) {
        float pos = -half + i * lineSpacing;
        const bool majorLine = (i == gridLines / 2);
        v.color = majorLine ? axisColor : gridColor;

        baseIdx = static_cast<u32>(vertices.size());
        v.position = vec3(-half, yOffset, pos - lineWidth);
        vertices.push_back(v);
        v.position = vec3( half, yOffset, pos - lineWidth);
        vertices.push_back(v);
        v.position = vec3( half, yOffset, pos + lineWidth);
        vertices.push_back(v);
        v.position = vec3(-half, yOffset, pos + lineWidth);
        vertices.push_back(v);
        
        indices.push_back(baseIdx + 0); indices.push_back(baseIdx + 1); indices.push_back(baseIdx + 2);
        indices.push_back(baseIdx + 0); indices.push_back(baseIdx + 2); indices.push_back(baseIdx + 3);

        baseIdx = static_cast<u32>(vertices.size());
        v.position = vec3(pos - lineWidth, yOffset, -half);
        vertices.push_back(v);
        v.position = vec3(pos + lineWidth, yOffset, -half);
        vertices.push_back(v);
        v.position = vec3(pos + lineWidth, yOffset,  half);
        vertices.push_back(v);
        v.position = vec3(pos - lineWidth, yOffset,  half);
        vertices.push_back(v);
        
        indices.push_back(baseIdx + 0); indices.push_back(baseIdx + 1); indices.push_back(baseIdx + 2);
        indices.push_back(baseIdx + 0); indices.push_back(baseIdx + 2); indices.push_back(baseIdx + 3);
    }

    m_mesh.create(vertices, indices);

    m_initialized = true;
    spdlog::info("Ground plane initialized: {}x{} with {} grid lines", size, size, gridLines);
}

void GroundPlane::destroy() {
    m_mesh.destroy();
    m_initialized = false;
}

void GroundPlane::render(bgfx::ViewId viewId, bgfx::ProgramHandle program, const mat4& view, const mat4& proj) {
    if (!m_initialized || !bgfx::isValid(program)) return;
    
    bgfx::setViewTransform(viewId, &view, &proj);
    
    u64 state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z |
                BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CCW;
    
    mat4 identity(1.0f);
    m_mesh.submit(viewId, program, identity, state);
}

} // namespace viber
