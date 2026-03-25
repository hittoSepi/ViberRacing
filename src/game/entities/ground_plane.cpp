#include "ground_plane.hpp"
#include <spdlog/spdlog.h>

namespace viber {

void GroundPlane::init(float size, u32 gridLines) {
    if (m_initialized) return;

    // Create a large plane with grid
    float half = size * 0.5f;
    float lineSpacing = size / gridLines;
    
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
    
    // Main plane (y = -0.5, well below car)
    float y = -0.5f;
    u32 baseIdx = static_cast<u32>(vertices.size());
    
    // Simple quad for the ground
    Vertex v;
    v.normal = vec3(0.0f, 1.0f, 0.0f);
    v.texCoord = vec2(0.0f, 0.0f);
    v.tangent = vec3(1.0f, 0.0f, 0.0f);
    v.bitangent = vec3(0.0f, 0.0f, 1.0f);
    
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
    
    // Grid lines (as thin quads)
    float lineWidth = size * 0.002f;
    float yOffset = y + 0.001f;  // Slightly above ground
    
    for (u32 i = 0; i <= gridLines; ++i) {
        float pos = -half + i * lineSpacing;
        
        // Line along X
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
        
        // Line along Z
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
