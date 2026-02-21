#include "debug_draw.hpp"
#include <spdlog/spdlog.h>

namespace viber {

DebugDraw::DebugDraw() {
    m_vertices.reserve(4096);
}

DebugDraw::~DebugDraw() {
    shutdown();
}

void DebugDraw::init() {
    m_layout.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
        .end();
    
    // Create empty vertex buffer with dynamic usage
    m_vbh = bgfx::createDynamicVertexBuffer(m_maxVertices, m_layout, BGFX_BUFFER_ALLOW_RESIZE);
    
    if (!bgfx::isValid(m_vbh)) {
        spdlog::error("Failed to create DebugDraw vertex buffer");
        return;
    }
    
    m_u_viewProj = bgfx::createUniform("u_viewProj", bgfx::UniformType::Mat4);
    
    spdlog::info("DebugDraw initialized");
}

void DebugDraw::shutdown() {
    if (bgfx::isValid(m_vbh)) {
        bgfx::destroy(m_vbh);
        m_vbh = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(m_program)) {
        bgfx::destroy(m_program);
        m_program = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(m_u_viewProj)) {
        bgfx::destroy(m_u_viewProj);
        m_u_viewProj = BGFX_INVALID_HANDLE;
    }
    
    m_vertices.clear();
}

void DebugDraw::beginFrame(const mat4& view, const mat4& projection) {
    m_view = view;
    m_projection = projection;
    m_vertices.clear();
}

void DebugDraw::endFrame() {
    if (m_vertices.empty() || !bgfx::isValid(m_program)) {
        return;
    }
    
    mat4 viewProj = m_projection * m_view;
    bgfx::setUniform(m_u_viewProj, &viewProj);
    
    bgfx::update(m_vbh, 0, bgfx::copy(m_vertices.data(), 
        static_cast<u32>(m_vertices.size() * sizeof(Vertex))));
    
    bgfx::setVertexBuffer(0, m_vbh, 0, static_cast<u32>(m_vertices.size()));
    
    u64 state = BGFX_STATE_DEFAULT | BGFX_STATE_PT_LINES;
    if (!m_depthTest) {
        state &= ~BGFX_STATE_DEPTH_TEST_MASK;
    }
    bgfx::setState(state);
    
    bgfx::submit(255, m_program);
}

u32 DebugDraw::toABGR(const vec4& color) {
    u8 r = static_cast<u8>(glm::clamp(color.r * 255.0f, 0.0f, 255.0f));
    u8 g = static_cast<u8>(glm::clamp(color.g * 255.0f, 0.0f, 255.0f));
    u8 b = static_cast<u8>(glm::clamp(color.b * 255.0f, 0.0f, 255.0f));
    u8 a = static_cast<u8>(glm::clamp(color.a * 255.0f, 0.0f, 255.0f));
    return (a << 24) | (b << 16) | (g << 8) | r;
}

void DebugDraw::ensureCapacity(size_t vertexCount) {
    if (m_vertices.size() + vertexCount > m_maxVertices) {
        m_vertices.clear();
    }
}

void DebugDraw::drawLine(const vec3& start, const vec3& end, const vec4& color) {
    drawLine(start, end, toABGR(color));
}

void DebugDraw::drawLine(const vec3& start, const vec3& end, u32 color) {
    ensureCapacity(2);
    m_vertices.push_back({start.x, start.y, start.z, color});
    m_vertices.push_back({end.x, end.y, end.z, color});
}

void DebugDraw::drawBox(const vec3& min, const vec3& max, const vec4& color) {
    drawBoxWireframe(min, max, color);
}

void DebugDraw::drawBoxWireframe(const vec3& min, const vec3& max, const vec4& color) {
    u32 c = toABGR(color);
    
    vec3 v000(min.x, min.y, min.z);
    vec3 v001(min.x, min.y, max.z);
    vec3 v010(min.x, max.y, min.z);
    vec3 v011(min.x, max.y, max.z);
    vec3 v100(max.x, min.y, min.z);
    vec3 v101(max.x, min.y, max.z);
    vec3 v110(max.x, max.y, min.z);
    vec3 v111(max.x, max.y, max.z);
    
    drawLine(v000, v001, c);
    drawLine(v001, v011, c);
    drawLine(v011, v010, c);
    drawLine(v010, v000, c);
    
    drawLine(v100, v101, c);
    drawLine(v101, v111, c);
    drawLine(v111, v110, c);
    drawLine(v110, v100, c);
    
    drawLine(v000, v100, c);
    drawLine(v001, v101, c);
    drawLine(v011, v111, c);
    drawLine(v010, v110, c);
}

void DebugDraw::drawSphere(const vec3& center, float radius, const vec4& color, u32 segments) {
    drawSphereWireframe(center, radius, color, segments);
}

void DebugDraw::drawSphereWireframe(const vec3& center, float radius, const vec4& color, u32 segments) {
    u32 c = toABGR(color);
    float step = TWO_PI / static_cast<float>(segments);
    
    for (u32 i = 0; i < segments; ++i) {
        float angle0 = i * step;
        float angle1 = (i + 1) * step;
        
        vec3 p0 = center + vec3(std::cos(angle0) * radius, 0.0f, std::sin(angle0) * radius);
        vec3 p1 = center + vec3(std::cos(angle1) * radius, 0.0f, std::sin(angle1) * radius);
        drawLine(p0, p1, c);
        
        p0 = center + vec3(std::cos(angle0) * radius, std::sin(angle0) * radius, 0.0f);
        p1 = center + vec3(std::cos(angle1) * radius, std::sin(angle1) * radius, 0.0f);
        drawLine(p0, p1, c);
        
        p0 = center + vec3(0.0f, std::cos(angle0) * radius, std::sin(angle0) * radius);
        p1 = center + vec3(0.0f, std::cos(angle1) * radius, std::sin(angle1) * radius);
        drawLine(p0, p1, c);
    }
}

void DebugDraw::drawCylinder(const vec3& start, const vec3& end, float radius, const vec4& color) {
    u32 c = toABGR(color);
    constexpr u32 segments = 16;
    float step = TWO_PI / static_cast<float>(segments);
    
    vec3 axis = end - start;
    float height = glm::length(axis);
    vec3 dir = glm::normalize(axis);
    
    vec3 up = std::abs(dir.y) < 0.99f ? vec3(0.0f, 1.0f, 0.0f) : vec3(1.0f, 0.0f, 0.0f);
    vec3 right = glm::normalize(glm::cross(up, dir));
    up = glm::cross(dir, right);
    
    for (u32 i = 0; i < segments; ++i) {
        float angle0 = i * step;
        float angle1 = (i + 1) * step;
        
        vec3 offset0 = (right * std::cos(angle0) + up * std::sin(angle0)) * radius;
        vec3 offset1 = (right * std::cos(angle1) + up * std::sin(angle1)) * radius;
        
        drawLine(start + offset0, start + offset1, c);
        drawLine(end + offset0, end + offset1, c);
        drawLine(start + offset0, end + offset0, c);
    }
}

void DebugDraw::drawPlane(const vec3& point, const vec3& normal, float size, const vec4& color) {
    vec3 u, v;
    if (std::abs(normal.y) < 0.99f) {
        u = glm::normalize(glm::cross(normal, vec3(0.0f, 1.0f, 0.0f)));
    } else {
        u = glm::normalize(glm::cross(normal, vec3(1.0f, 0.0f, 0.0f)));
    }
    v = glm::cross(normal, u);
    
    vec3 p0 = point - u * size - v * size;
    vec3 p1 = point + u * size - v * size;
    vec3 p2 = point + u * size + v * size;
    vec3 p3 = point - u * size + v * size;
    
    u32 c = toABGR(color);
    drawLine(p0, p1, c);
    drawLine(p1, p2, c);
    drawLine(p2, p3, c);
    drawLine(p3, p0, c);
    
    drawLine(point, point + normal * size * 0.5f, c);
}

void DebugDraw::drawAxes(const vec3& origin, float size) {
    drawLine(origin, origin + vec3(size, 0.0f, 0.0f), vec4(1.0f, 0.0f, 0.0f, 1.0f));
    drawLine(origin, origin + vec3(0.0f, size, 0.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f));
    drawLine(origin, origin + vec3(0.0f, 0.0f, size), vec4(0.0f, 0.0f, 1.0f, 1.0f));
}

void DebugDraw::drawTransform(const vec3& position, const quat& rotation, float size) {
    mat3 rot = glm::mat3_cast(rotation);
    vec3 right(rot[0]);
    vec3 up(rot[1]);
    vec3 forward(rot[2]);
    
    drawLine(position, position + right * size, vec4(1.0f, 0.0f, 0.0f, 1.0f));
    drawLine(position, position + up * size, vec4(0.0f, 1.0f, 0.0f, 1.0f));
    drawLine(position, position + forward * size, vec4(0.0f, 0.0f, 1.0f, 1.0f));
}

void DebugDraw::drawGrid(const vec3& center, const vec3& normal, float size, u32 divisions, const vec4& color) {
    u32 c = toABGR(color);
    float step = size / static_cast<float>(divisions);
    float half = size * 0.5f;
    
    vec3 u, v;
    if (std::abs(normal.y) > 0.9f) {
        u = vec3(1.0f, 0.0f, 0.0f);
        v = vec3(0.0f, 0.0f, 1.0f);
    } else {
        u = glm::normalize(glm::cross(normal, vec3(0.0f, 1.0f, 0.0f)));
        v = glm::cross(normal, u);
    }
    
    for (u32 i = 0; i <= divisions; ++i) {
        float offset = -half + i * step;
        drawLine(center + u * offset - v * half, center + u * offset + v * half, c);
        drawLine(center + v * offset - u * half, center + v * offset + u * half, c);
    }
}

void DebugDraw::setLineWidth(float width) {
    m_lineWidth = width;
}

void DebugDraw::setDepthTest(bool enabled) {
    m_depthTest = enabled;
}

}
