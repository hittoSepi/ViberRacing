#pragma once

#include "core/types.hpp"
#include <bgfx/bgfx.h>

struct GLFWwindow;

namespace viber {

class ImGuiRenderer {
public:
    ImGuiRenderer();
    ~ImGuiRenderer();
    
    void init(GLFWwindow* window);
    void shutdown();
    
    void beginFrame();
    void endFrame();
    
    void render();
    
private:
    void setupKeyMappings();
    void setupStyle();
    bgfx::ProgramHandle createImGuiShader();
    bgfx::ShaderHandle loadShaderFromFile(const char* path);
    
    bgfx::VertexLayout m_layout;
    bgfx::ProgramHandle m_program = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle m_u_texture = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle m_fontTexture = BGFX_INVALID_HANDLE;
    bgfx::VertexBufferHandle m_vbh = BGFX_INVALID_HANDLE;
    bgfx::IndexBufferHandle m_ibh = BGFX_INVALID_HANDLE;
    
    GLFWwindow* m_window = nullptr;
    double m_time = 0.0;
    bool m_initialized = false;
};

}
