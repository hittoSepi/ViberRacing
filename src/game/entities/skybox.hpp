#pragma once

#include "core/types.hpp"
#include "renderer/mesh.hpp"
#include <bgfx/bgfx.h>

namespace viber {

class Skybox {
public:
    Skybox() = default;
    ~Skybox() { destroy(); }

    Skybox(const Skybox&) = delete;
    Skybox& operator=(const Skybox&) = delete;

    // Initialize with procedural cubemap
    // size = texture resolution per face (default 512)
    void init(u32 size = 512);
    void destroy();

    // Render skybox - call first in frame, before other objects
    void render(bgfx::ViewId viewId, bgfx::ProgramHandle program, const mat4& view, const mat4& proj);

    bgfx::TextureHandle getCubemap() const { return m_cubemap; }

private:
    // Generate procedural sky gradient for each cubemap face
    void generateProceduralCubemap(u32 size);
    
    // Generate gradient data for one face
    std::vector<u8> generateFaceData(u32 size, int faceIndex);

    Mesh m_cubeMesh;
    bgfx::TextureHandle m_cubemap = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle m_skyboxUniform = BGFX_INVALID_HANDLE;
    bool m_initialized = false;
};

} // namespace viber
