#pragma once

#include "core/types.hpp"
#include <bgfx/bgfx.h>
#include <vector>

namespace viber {

class DebugDraw {
public:
    DebugDraw();
    ~DebugDraw();
    
    void init();
    void shutdown();
    
    void beginFrame(const mat4& view, const mat4& projection);
    void endFrame();
    
    void drawLine(const vec3& start, const vec3& end, const vec4& color);
    void drawLine(const vec3& start, const vec3& end, u32 color);
    
    void drawBox(const vec3& min, const vec3& max, const vec4& color);
    void drawBoxWireframe(const vec3& min, const vec3& max, const vec4& color);
    
    void drawSphere(const vec3& center, float radius, const vec4& color, u32 segments = 16);
    void drawSphereWireframe(const vec3& center, float radius, const vec4& color, u32 segments = 16);
    
    void drawCylinder(const vec3& start, const vec3& end, float radius, const vec4& color);
    
    void drawPlane(const vec3& point, const vec3& normal, float size, const vec4& color);
    
    void drawAxes(const vec3& origin, float size = 1.0f);
    void drawTransform(const vec3& position, const quat& rotation, float size = 1.0f);
    
    void drawGrid(const vec3& center, const vec3& normal, float size, u32 divisions, const vec4& color);
    
    void setLineWidth(float width);
    void setDepthTest(bool enabled);
    
    static u32 toABGR(const vec4& color);
    
private:
    struct Vertex {
        float x, y, z;
        u32 abgr;
    };
    
    void ensureCapacity(size_t vertexCount);
    
    bgfx::VertexLayout m_layout;
    bgfx::DynamicVertexBufferHandle m_vbh = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle m_program = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle m_u_viewProj = BGFX_INVALID_HANDLE;
    
    std::vector<Vertex> m_vertices;
    size_t m_maxVertices = 65536;
    
    mat4 m_view;
    mat4 m_projection;
    float m_lineWidth = 1.0f;
    bool m_depthTest = true;
};

}
