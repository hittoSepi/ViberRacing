#pragma once

#include "core/types.hpp"
#include <bgfx/bgfx.h>
#include <vector>

namespace viber {

struct Vertex {
    vec3 position;
    vec3 normal;
    vec2 texCoord;
    vec3 tangent;
    vec3 bitangent;
    
    static void init(bgfx::VertexLayout& layout) {
        layout.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Tangent, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Bitangent, 3, bgfx::AttribType::Float)
            .end();
    }
};

struct VertexSimple {
    vec3 position;
    vec3 color;
    
    static void init(bgfx::VertexLayout& layout) {
        layout.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 3, bgfx::AttribType::Float)
            .end();
    }
};

class Mesh {
public:
    Mesh();
    ~Mesh();
    
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;
    
    bool create(const std::vector<Vertex>& vertices, const std::vector<u32>& indices);
    bool create(const std::vector<VertexSimple>& vertices, const std::vector<u32>& indices);
    
    void destroy();
    bool isValid() const;
    
    void submit(bgfx::ViewId viewId, bgfx::ProgramHandle program, 
                const mat4& transform, u64 state = BGFX_STATE_DEFAULT) const;
    
    size_t getVertexCount() const { return m_vertexCount; }
    size_t getIndexCount() const { return m_indexCount; }
    
    static Mesh createCube();
    static Mesh createSphere(float radius = 1.0f, u32 rings = 16, u32 sectors = 32);
    static Mesh createPlane(float width = 10.0f, float depth = 10.0f, 
                           u32 divisionsX = 1, u32 divisionsZ = 1);
    
    bgfx::VertexBufferHandle getVertexBuffer() const { return m_vbh; }
    bgfx::IndexBufferHandle getIndexBuffer() const { return m_ibh; }
    
private:
    bgfx::VertexBufferHandle m_vbh = BGFX_INVALID_HANDLE;
    bgfx::IndexBufferHandle m_ibh = BGFX_INVALID_HANDLE;
    bgfx::VertexLayout m_layout;
    size_t m_vertexCount = 0;
    size_t m_indexCount = 0;
};

}
