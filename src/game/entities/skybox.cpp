#include "skybox.hpp"
#include <spdlog/spdlog.h>
#include <cmath>

namespace viber {

void Skybox::init(u32 size) {
    if (m_initialized) return;

    // Create skybox cube (inward facing)
    m_cubeMesh = Mesh::createSkyboxCube();
    
    // Create uniform for cubemap
    m_skyboxUniform = bgfx::createUniform("s_skybox", bgfx::UniformType::Sampler);
    
    generateProceduralCubemap(size);
    
    m_initialized = true;
    spdlog::info("Skybox initialized with {}x{} cubemap", size, size);
}

void Skybox::destroy() {
    if (bgfx::isValid(m_cubemap)) {
        bgfx::destroy(m_cubemap);
        m_cubemap = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(m_skyboxUniform)) {
        bgfx::destroy(m_skyboxUniform);
        m_skyboxUniform = BGFX_INVALID_HANDLE;
    }
    m_cubeMesh.destroy();
    m_initialized = false;
}

std::vector<u8> Skybox::generateFaceData(u32 size, int faceIndex) {
    // Face indices: 0=+X, 1=-X, 2=+Y, 3=-Y, 4=+Z, 5=-Z
    std::vector<u8> data(size * size * 4);  // RGBA
    
    // Sky colors
    vec3 skyTop{0.2f, 0.5f, 0.9f};      // Deep blue
    vec3 skyHorizon{0.6f, 0.8f, 1.0f};  // Light blue/white
    vec3 skyBottom{0.9f, 0.7f, 0.5f};   // Orange-ish (sunset glow)
    
    for (u32 y = 0; y < size; ++y) {
        for (u32 x = 0; x < size; ++x) {
            // Normalized coordinates [-1, 1]
            float u = (x / float(size - 1)) * 2.0f - 1.0f;
            float v = (y / float(size - 1)) * 2.0f - 1.0f;
            
            vec3 dir;
            switch (faceIndex) {
                case 0: dir = vec3(1.0f, -v, -u); break;   // +X
                case 1: dir = vec3(-1.0f, -v, u); break;   // -X
                case 2: dir = vec3(u, 1.0f, v); break;     // +Y (top)
                case 3: dir = vec3(u, -1.0f, -v); break;   // -Y (bottom)
                case 4: dir = vec3(u, -v, 1.0f); break;    // +Z
                case 5: dir = vec3(-u, -v, -1.0f); break;  // -Z
            }
            dir = glm::normalize(dir);
            
            // Gradient based on Y (up) component
            float t = glm::clamp(dir.y, 0.0f, 1.0f);
            vec3 color;
            
            if (dir.y > 0.0f) {
                // Upper hemisphere: top blue -> horizon
                color = glm::mix(skyHorizon, skyTop, t);
            } else {
                // Lower hemisphere: horizon -> bottom glow
                t = glm::clamp(-dir.y, 0.0f, 1.0f);
                color = glm::mix(skyHorizon, skyBottom, t * 0.3f);  // subtle glow
            }
            
            // Add some "sun" directionality (towards +Z, upper)
            vec3 sunDir = glm::normalize(vec3(0.3f, 0.8f, 0.5f));
            float sunDot = glm::max(glm::dot(dir, sunDir), 0.0f);
            color += vec3(1.0f, 0.9f, 0.7f) * pow(sunDot, 32.0f) * 0.5f;  // Sun glow
            
            // Clamp
            color = glm::clamp(color, 0.0f, 1.0f);
            
            u32 idx = (y * size + x) * 4;
            data[idx + 0] = static_cast<u8>(color.r * 255);
            data[idx + 1] = static_cast<u8>(color.g * 255);
            data[idx + 2] = static_cast<u8>(color.b * 255);
            data[idx + 3] = 255;
        }
    }
    
    return data;
}

void Skybox::generateProceduralCubemap(u32 size) {
    // Create cubemap texture
    m_cubemap = bgfx::createTextureCube(
        static_cast<uint16_t>(size),   // width/height per face
        false,                         // hasMips
        1,                             // numLayers
        bgfx::TextureFormat::RGBA8,    // format
        BGFX_TEXTURE_NONE              // flags
    );
    
    // Update each face
    for (uint8_t face = 0; face < 6; ++face) {
        auto data = generateFaceData(size, face);
        const bgfx::Memory* mem = bgfx::copy(data.data(), static_cast<uint32_t>(data.size()));
        bgfx::updateTextureCube(m_cubemap, 0, face, 0, 0, 0, 
            static_cast<uint16_t>(size), static_cast<uint16_t>(size), mem);
    }
}

void Skybox::render(bgfx::ViewId viewId, bgfx::ProgramHandle program, const mat4& view, const mat4& proj) {
    if (!m_initialized || !bgfx::isValid(program)) return;
    
    // Remove translation from view matrix (skybox stays at camera position)
    mat4 viewNoTrans = view;
    viewNoTrans[3][0] = 0.0f;
    viewNoTrans[3][1] = 0.0f;
    viewNoTrans[3][2] = 0.0f;
    
    // Set cubemap texture
    bgfx::setTexture(0, m_skyboxUniform, m_cubemap);
    
    // Render without depth write, at max depth
    u64 state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | 
                BGFX_STATE_DEPTH_TEST_LEQUAL | BGFX_STATE_CULL_CW;
    
    // Submit with identity transform (skybox cube is at origin, view matrix handles rotation)
    mat4 identity(1.0f);
    bgfx::setTransform(&identity);
    bgfx::setVertexBuffer(0, m_cubeMesh.getVertexBuffer());
    bgfx::setIndexBuffer(m_cubeMesh.getIndexBuffer());
    bgfx::setState(state);
    
    // Use modified view matrix
    bgfx::setViewTransform(viewId, &viewNoTrans, &proj);
    bgfx::submit(viewId, program);
}

} // namespace viber
