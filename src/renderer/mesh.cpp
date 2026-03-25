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

Mesh::Mesh(Mesh&& other) noexcept
    : m_vbh(other.m_vbh)
    , m_ibh(other.m_ibh)
    , m_layout(other.m_layout)
    , m_vertexCount(other.m_vertexCount)
    , m_indexCount(other.m_indexCount) {
    other.m_vbh.idx = bgfx::kInvalidHandle;
    other.m_ibh.idx = bgfx::kInvalidHandle;
    other.m_vertexCount = 0;
    other.m_indexCount = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        destroy();
        m_layout = other.m_layout;
        m_vbh = other.m_vbh;
        m_ibh = other.m_ibh;
        m_vertexCount = other.m_vertexCount;
        m_indexCount = other.m_indexCount;
        other.m_vbh.idx = bgfx::kInvalidHandle;
        other.m_ibh.idx = bgfx::kInvalidHandle;
        other.m_vertexCount = 0;
        other.m_indexCount = 0;
    }
    return *this;
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
    return createBox({1.0f, 1.0f, 1.0f});
}

Mesh Mesh::createBox(const vec3& size) {
    Mesh mesh;
    
    vec3 h = size * 0.5f; // half extents
    
    std::vector<Vertex> vertices = {
        // Front (+Z)
        {{-h.x, -h.y,  h.z}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {}, {}},
        {{ h.x, -h.y,  h.z}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {}, {}},
        {{ h.x,  h.y,  h.z}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {}, {}},
        {{-h.x,  h.y,  h.z}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {}, {}},
        // Right (+X)
        {{ h.x, -h.y, -h.z}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {}, {}},
        {{ h.x,  h.y, -h.z}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {}, {}},
        {{ h.x, -h.y,  h.z}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {}, {}},
        {{ h.x,  h.y,  h.z}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {}, {}},
        // Back (-Z)
        {{-h.x, -h.y, -h.z}, {0.0f, 0.0f,-1.0f}, {0.0f, 0.0f}, {}, {}},
        {{-h.x,  h.y, -h.z}, {0.0f, 0.0f,-1.0f}, {1.0f, 0.0f}, {}, {}},
        {{ h.x,  h.y, -h.z}, {0.0f, 0.0f,-1.0f}, {1.0f, 1.0f}, {}, {}},
        {{ h.x, -h.y, -h.z}, {0.0f, 0.0f,-1.0f}, {0.0f, 1.0f}, {}, {}},
        // Left (-X)
        {{-h.x, -h.y,  h.z}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {}, {}},
        {{-h.x,  h.y,  h.z}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {}, {}},
        {{-h.x,  h.y, -h.z}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {}, {}},
        {{-h.x, -h.y, -h.z}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {}, {}},
        // Top (+Y)
        {{-h.x,  h.y,  h.z}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {}, {}},
        {{ h.x,  h.y,  h.z}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {}, {}},
        {{ h.x,  h.y, -h.z}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {}, {}},
        {{-h.x,  h.y, -h.z}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {}, {}},
        // Bottom (-Y)
        {{-h.x, -h.y, -h.z}, {0.0f,-1.0f, 0.0f}, {0.0f, 0.0f}, {}, {}},
        {{ h.x, -h.y, -h.z}, {0.0f,-1.0f, 0.0f}, {1.0f, 0.0f}, {}, {}},
        {{ h.x, -h.y,  h.z}, {0.0f,-1.0f, 0.0f}, {1.0f, 1.0f}, {}, {}},
        {{-h.x, -h.y,  h.z}, {0.0f,-1.0f, 0.0f}, {0.0f, 1.0f}, {}, {}},
    };
    
    std::vector<u32> indices = {
        // Front (+Z) - CCW
        0, 2, 1, 0, 3, 2,
        // Right (+X) - CCW
        4, 6, 5, 5, 6, 7,
        // Back (-Z) - CCW
        8, 10, 9, 8, 11, 10,
        // Left (-X) - CCW
        12, 14, 13, 12, 15, 14,
        // Top (+Y) - CCW
        16, 18, 17, 16, 19, 18,
        // Bottom (-Y) - CCW
        20, 22, 21, 20, 23, 22
    };
    
    mesh.create(vertices, indices);
    return mesh;
}

Mesh Mesh::createSkyboxCube() {
    // Inward-facing cube for skybox rendering
    // We flip normals and winding order so inside faces are visible
    Mesh mesh;
    
    float h = 1.0f;  // half size (cube goes from -1 to 1)
    
    std::vector<Vertex> vertices = {
        // Front face (+Z) - normal points inward (-Z)
        {{-h, -h,  h}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {}, {}},
        {{ h, -h,  h}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}, {}, {}},
        {{ h,  h,  h}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}, {}, {}},
        {{-h,  h,  h}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}, {}, {}},
        // Right face (+X) - normal points inward (-X)
        {{ h, -h, -h}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {}, {}},
        {{ h,  h, -h}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {}, {}},
        {{ h, -h,  h}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {}, {}},
        {{ h,  h,  h}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {}, {}},
        // Back face (-Z) - normal points inward (+Z)
        {{-h, -h, -h}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {}, {}},
        {{-h,  h, -h}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {}, {}},
        {{ h,  h, -h}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {}, {}},
        {{ h, -h, -h}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {}, {}},
        // Left face (-X) - normal points inward (+X)
        {{-h, -h,  h}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {}, {}},
        {{-h,  h,  h}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {}, {}},
        {{-h,  h, -h}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {}, {}},
        {{-h, -h, -h}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {}, {}},
        // Top face (+Y) - normal points inward (-Y)
        {{-h,  h,  h}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {}, {}},
        {{ h,  h,  h}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}, {}, {}},
        {{ h,  h, -h}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}, {}, {}},
        {{-h,  h, -h}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}, {}, {}},
        // Bottom face (-Y) - normal points inward (+Y)
        {{-h, -h, -h}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {}, {}},
        {{ h, -h, -h}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {}, {}},
        {{ h, -h,  h}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {}, {}},
        {{-h, -h,  h}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {}, {}},
    };
    
    // CCW winding (from inside view)
    std::vector<u32> indices = {
        0, 2, 1, 0, 3, 2,       // Front
        4, 6, 5, 5, 6, 7,       // Right
        8, 10, 9, 8, 11, 10,    // Back
        12, 14, 13, 12, 15, 14, // Left
        16, 18, 17, 16, 19, 18, // Top
        20, 22, 21, 20, 23, 22  // Bottom
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
            
            indices.push_back(i0); indices.push_back(i1); indices.push_back(i2);
            indices.push_back(i0); indices.push_back(i2); indices.push_back(i3);
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
            
            indices.push_back(i0); indices.push_back(i1); indices.push_back(i2);
            indices.push_back(i1); indices.push_back(i3); indices.push_back(i2);
        }
    }
    
    mesh.create(vertices, indices);
    return mesh;
}

Mesh Mesh::createCylinder(float radius, float height, u32 segments) {
    Mesh mesh;
    
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
    
    float halfHeight = height * 0.5f;
    
    // Side vertices
    for (u32 i = 0; i <= segments; ++i) {
        float angle = 2.0f * PI * i / segments;
        float x = std::cos(angle) * radius;
        float z = std::sin(angle) * radius;
        vec3 normal = glm::normalize(vec3(x, 0.0f, z));
        
        // Top ring
        Vertex vTop;
        vTop.position = vec3(x, halfHeight, z);
        vTop.normal = normal;
        vTop.texCoord = vec2(i / static_cast<float>(segments), 1.0f);
        vTop.tangent = vec3(-normal.z, 0.0f, normal.x);
        vTop.bitangent = vec3(0.0f, 1.0f, 0.0f);
        vertices.push_back(vTop);
        
        // Bottom ring
        Vertex vBot;
        vBot.position = vec3(x, -halfHeight, z);
        vBot.normal = normal;
        vBot.texCoord = vec2(i / static_cast<float>(segments), 0.0f);
        vBot.tangent = vec3(-normal.z, 0.0f, normal.x);
        vBot.bitangent = vec3(0.0f, 1.0f, 0.0f);
        vertices.push_back(vBot);
    }
    
    // Side indices (CCW)
    for (u32 i = 0; i < segments; ++i) {
        u32 i0 = i * 2;
        u32 i1 = i0 + 1;
        u32 i2 = i0 + 2;
        u32 i3 = i0 + 3;
        
        indices.push_back(i0); indices.push_back(i2); indices.push_back(i1);
        indices.push_back(i1); indices.push_back(i2); indices.push_back(i3);
    }
    
    // Top cap center and ring vertices
    u32 topCenterIdx = static_cast<u32>(vertices.size());
    Vertex topCenter;
    topCenter.position = vec3(0.0f, halfHeight, 0.0f);
    topCenter.normal = vec3(0.0f, 1.0f, 0.0f);
    topCenter.texCoord = vec2(0.5f, 0.5f);
    topCenter.tangent = vec3(1.0f, 0.0f, 0.0f);
    topCenter.bitangent = vec3(0.0f, 0.0f, -1.0f);
    vertices.push_back(topCenter);
    
    u32 topRingStart = static_cast<u32>(vertices.size());
    for (u32 i = 0; i <= segments; ++i) {
        float angle = 2.0f * PI * i / segments;
        float x = std::cos(angle) * radius;
        float z = std::sin(angle) * radius;
        
        Vertex v;
        v.position = vec3(x, halfHeight, z);
        v.normal = vec3(0.0f, 1.0f, 0.0f);
        v.texCoord = vec2(x / radius * 0.5f + 0.5f, z / radius * 0.5f + 0.5f);
        v.tangent = vec3(1.0f, 0.0f, 0.0f);
        v.bitangent = vec3(0.0f, 0.0f, -1.0f);
        vertices.push_back(v);
    }
    
    // Top cap indices (CCW)
    for (u32 i = 0; i < segments; ++i) {
        indices.push_back(topCenterIdx);
        indices.push_back(topRingStart + i);
        indices.push_back(topRingStart + i + 1);
    }
    
    // Bottom cap center and ring vertices
    u32 botCenterIdx = static_cast<u32>(vertices.size());
    Vertex botCenter;
    botCenter.position = vec3(0.0f, -halfHeight, 0.0f);
    botCenter.normal = vec3(0.0f, -1.0f, 0.0f);
    botCenter.texCoord = vec2(0.5f, 0.5f);
    botCenter.tangent = vec3(1.0f, 0.0f, 0.0f);
    botCenter.bitangent = vec3(0.0f, 0.0f, 1.0f);
    vertices.push_back(botCenter);
    
    u32 botRingStart = static_cast<u32>(vertices.size());
    for (u32 i = 0; i <= segments; ++i) {
        float angle = 2.0f * PI * i / segments;
        float x = std::cos(angle) * radius;
        float z = std::sin(angle) * radius;
        
        Vertex v;
        v.position = vec3(x, -halfHeight, z);
        v.normal = vec3(0.0f, -1.0f, 0.0f);
        v.texCoord = vec2(x / radius * 0.5f + 0.5f, z / radius * 0.5f + 0.5f);
        v.tangent = vec3(1.0f, 0.0f, 0.0f);
        v.bitangent = vec3(0.0f, 0.0f, 1.0f);
        vertices.push_back(v);
    }
    
    // Bottom cap indices (CW for correct facing when looking from below, but we want CCW)
    for (u32 i = 0; i < segments; ++i) {
        indices.push_back(botCenterIdx);
        indices.push_back(botRingStart + i + 1);
        indices.push_back(botRingStart + i);
    }
    
    mesh.create(vertices, indices);
    return mesh;
}

}
