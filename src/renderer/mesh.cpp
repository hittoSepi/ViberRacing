#include "mesh.hpp"
#include <bgfx/bgfx.h>
#include <spdlog/spdlog.h>

namespace viber {

Mesh::Mesh() {
    Vertex::init(m_layout);
}

Mesh::~Mesh() {
    destroy();
}

bool Mesh::create(const std::vector<Vertex>& vertices, const std::vector<u32>& indices) {
    destroy();
    
    if (vertices.empty() || indices.empty()) {
        spdlog::error("Cannot create mesh with empty vertices or indices");
        return false;
    }
    
    const bgfx::Memory* vbMem = bgfx::copy(vertices.data(), 
        static_cast<u32>(vertices.size() * sizeof(Vertex)));
    m_vbh = bgfx::createVertexBuffer(vbMem, m_layout);
    
    bool use32BitIndices = indices.size() > 65535;
    const bgfx::Memory* ibMem = bgfx::copy(indices.data(), 
        static_cast<u32>(indices.size() * (use32BitIndices ? sizeof(u32) : sizeof(u16))));
    
    if (use32BitIndices) {
        m_ibh = bgfx::createIndexBuffer(ibMem, BGFX_BUFFER_INDEX32);
    } else {
        std::vector<u16> indices16(indices.begin(), indices.end());
        ibMem = bgfx::copy(indices16.data(), static_cast<u32>(indices16.size() * sizeof(u16)));
        m_ibh = bgfx::createIndexBuffer(ibMem, 0);
    }
    
    m_vertexCount = vertices.size();
    m_indexCount = indices.size();
    
    if (!isValid()) {
        spdlog::error("Failed to create mesh buffers");
        destroy();
        return false;
    }
    
    return true;
}

bool Mesh::create(const std::vector<VertexSimple>& vertices, const std::vector<u32>& indices) {
    bgfx::VertexLayout simpleLayout;
    VertexSimple::init(simpleLayout);
    
    destroy();
    
    if (vertices.empty() || indices.empty()) {
        return false;
    }
    
    const bgfx::Memory* vbMem = bgfx::copy(vertices.data(),
        static_cast<u32>(vertices.size() * sizeof(VertexSimple)));
    m_vbh = bgfx::createVertexBuffer(vbMem, simpleLayout);
    
    bool use32BitIndices = indices.size() > 65535;
    const bgfx::Memory* ibMem = bgfx::copy(indices.data(),
        static_cast<u32>(indices.size() * (use32BitIndices ? sizeof(u32) : sizeof(u16))));
    
    if (use32BitIndices) {
        m_ibh = bgfx::createIndexBuffer(ibMem, BGFX_BUFFER_INDEX32);
    } else {
        std::vector<u16> indices16(indices.begin(), indices.end());
        ibMem = bgfx::copy(indices16.data(), static_cast<u32>(indices16.size() * sizeof(u16)));
        m_ibh = bgfx::createIndexBuffer(ibMem, 0);
    }
    
    m_vertexCount = vertices.size();
    m_indexCount = indices.size();
    m_layout = simpleLayout;
    
    return isValid();
}

void Mesh::destroy() {
    if (bgfx::isValid(m_vbh)) {
        bgfx::destroy(m_vbh);
        m_vbh = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(m_ibh)) {
        bgfx::destroy(m_ibh);
        m_ibh = BGFX_INVALID_HANDLE;
    }
    m_vertexCount = 0;
    m_indexCount = 0;
}

bool Mesh::isValid() const {
    return bgfx::isValid(m_vbh) && bgfx::isValid(m_ibh);
}

void Mesh::submit(bgfx::ViewId viewId, bgfx::ProgramHandle program,
                  const mat4& transform, u64 state) const {
    if (!isValid()) return;
    
    bgfx::setTransform(&transform);
    bgfx::setVertexBuffer(0, m_vbh);
    bgfx::setIndexBuffer(m_ibh);
    bgfx::setState(state);
    bgfx::submit(viewId, program);
}

Mesh Mesh::createCube() {
    Mesh mesh;
    
    std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {}, {}},
        {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {}, {}},
        {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {}, {}},
        {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {}, {}},
        {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {}, {}},
        {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {}, {}},
        {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {}, {}},
        {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {}, {}},
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f,-1.0f}, {0.0f, 0.0f}, {}, {}},
        {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f,-1.0f}, {1.0f, 0.0f}, {}, {}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f,-1.0f}, {1.0f, 1.0f}, {}, {}},
        {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f,-1.0f}, {0.0f, 1.0f}, {}, {}},
        {{-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {}, {}},
        {{-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {}, {}},
        {{-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {}, {}},
        {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {}, {}},
        {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {}, {}},
        {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {}, {}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {}, {}},
        {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {}, {}},
        {{-0.5f, -0.5f, -0.5f}, {0.0f,-1.0f, 0.0f}, {0.0f, 0.0f}, {}, {}},
        {{ 0.5f, -0.5f, -0.5f}, {0.0f,-1.0f, 0.0f}, {1.0f, 0.0f}, {}, {}},
        {{ 0.5f, -0.5f,  0.5f}, {0.0f,-1.0f, 0.0f}, {1.0f, 1.0f}, {}, {}},
        {{-0.5f, -0.5f,  0.5f}, {0.0f,-1.0f, 0.0f}, {0.0f, 1.0f}, {}, {}},
    };
    
    std::vector<u32> indices = {
        0, 1, 2, 0, 2, 3,
        4, 5, 6, 5, 7, 6,
        8, 9, 10, 8, 10, 11,
        12, 13, 14, 12, 14, 15,
        16, 17, 18, 16, 18, 19,
        20, 21, 22, 20, 22, 23
    };
    
    mesh.create(vertices, indices);
    return mesh;
}

Mesh Mesh::createSphere(float radius, u32 rings, u32 sectors) {
    Mesh mesh;
    
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
    
    float const R = 1.0f / static_cast<float>(rings - 1);
    float const S = 1.0f / static_cast<float>(sectors - 1);
    
    for (u32 r = 0; r < rings; ++r) {
        for (u32 s = 0; s < sectors; ++s) {
            float const y = std::sin(-HALF_PI + PI * r * R);
            float const x = std::cos(2.0f * PI * s * S) * std::sin(PI * r * R);
            float const z = std::sin(2.0f * PI * s * S) * std::sin(PI * r * R);
            
            Vertex v;
            v.position = vec3(x * radius, y * radius, z * radius);
            v.normal = glm::normalize(v.position);
            v.texCoord = vec2(s * S, r * R);
            v.tangent = vec3(0.0f);
            v.bitangent = vec3(0.0f);
            vertices.push_back(v);
        }
    }
    
    for (u32 r = 0; r < rings - 1; ++r) {
        for (u32 s = 0; s < sectors - 1; ++s) {
            u32 i0 = r * sectors + s;
            u32 i1 = r * sectors + (s + 1);
            u32 i2 = (r + 1) * sectors + (s + 1);
            u32 i3 = (r + 1) * sectors + s;
            
            indices.push_back(i0); indices.push_back(i2); indices.push_back(i1);
            indices.push_back(i0); indices.push_back(i3); indices.push_back(i2);
        }
    }
    
    mesh.create(vertices, indices);
    return mesh;
}

Mesh Mesh::createPlane(float width, float depth, u32 divisionsX, u32 divisionsZ) {
    Mesh mesh;
    
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
    
    float halfW = width * 0.5f;
    float halfD = depth * 0.5f;
    float stepX = width / static_cast<float>(divisionsX);
    float stepZ = depth / static_cast<float>(divisionsZ);
    
    for (u32 z = 0; z <= divisionsZ; ++z) {
        for (u32 x = 0; x <= divisionsX; ++x) {
            Vertex v;
            v.position = vec3(
                -halfW + static_cast<float>(x) * stepX,
                0.0f,
                -halfD + static_cast<float>(z) * stepZ
            );
            v.normal = vec3(0.0f, 1.0f, 0.0f);
            v.texCoord = vec2(
                static_cast<float>(x) / static_cast<float>(divisionsX),
                static_cast<float>(z) / static_cast<float>(divisionsZ)
            );
            v.tangent = vec3(1.0f, 0.0f, 0.0f);
            v.bitangent = vec3(0.0f, 0.0f, 1.0f);
            vertices.push_back(v);
        }
    }
    
    for (u32 z = 0; z < divisionsZ; ++z) {
        for (u32 x = 0; x < divisionsX; ++x) {
            u32 i0 = z * (divisionsX + 1) + x;
            u32 i1 = i0 + 1;
            u32 i2 = i0 + divisionsX + 1;
            u32 i3 = i2 + 1;
            
            indices.push_back(i0); indices.push_back(i2); indices.push_back(i1);
            indices.push_back(i1); indices.push_back(i2); indices.push_back(i3);
        }
    }
    
    mesh.create(vertices, indices);
    return mesh;
}

}
