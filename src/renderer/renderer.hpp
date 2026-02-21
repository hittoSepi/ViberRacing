#pragma once

#include "core/types.hpp"
#include "core/window.hpp"
#include "camera.hpp"
#include <bgfx/bgfx.h>
#include <functional>
#include <vector>

namespace viber {

struct RenderStats {
    u32 drawCalls = 0;
    u32 vertices = 0;
    u32 triangles = 0;
    float cpuTimeMs = 0.0f;
    float gpuTimeMs = 0.0f;
};

class Renderer {
public:
    explicit Renderer(Window& window);
    ~Renderer();
    
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    
    void beginFrame();
    void endFrame();
    
    void setClearColor(const vec4& color);
    void setViewport(int x, int y, int width, int height);
    
    void setViewProjection(const mat4& view, const mat4& projection);
    void setViewTransform(const Camera& camera);
    
    bgfx::ProgramHandle loadShader(const std::string& vsPath, const std::string& fsPath);
    void destroyShader(bgfx::ProgramHandle handle);
    
    bgfx::TextureHandle loadTexture(const std::string& path);
    void destroyTexture(bgfx::TextureHandle handle);
    
    bgfx::VertexBufferHandle createVertexBuffer(const void* data, size_t size, const bgfx::VertexLayout& layout);
    bgfx::IndexBufferHandle createIndexBuffer(const void* data, size_t size, bool indices32 = false);
    
    void destroyVertexBuffer(bgfx::VertexBufferHandle handle);
    void destroyIndexBuffer(bgfx::IndexBufferHandle handle);
    
    void submit(bgfx::ViewId viewId, bgfx::ProgramHandle program,
                bgfx::VertexBufferHandle vbh, bgfx::IndexBufferHandle ibh,
                const mat4& transform, u64 state = BGFX_STATE_DEFAULT);
    
    void setUniform(bgfx::UniformHandle handle, const void* value, size_t size);
    bgfx::UniformHandle createUniform(const std::string& name, bgfx::UniformType::Enum type, u16 num = 1);
    void destroyUniform(bgfx::UniformHandle handle);
    
    const RenderStats& getStats() const { return m_stats; }
    
    vec2 getWindowSize() const { return m_windowSize; }
    float getAspectRatio() const { return m_windowSize.x / m_windowSize.y; }
    
    bgfx::Init getInit() const { return m_init; }
    
private:
    Window& m_window;
    bgfx::Init m_init{};
    vec2 m_windowSize{1280.0f, 720.0f};
    vec4 m_clearColor{0.1f, 0.1f, 0.2f, 1.0f};
    RenderStats m_stats;
    
    bgfx::UniformHandle m_u_model{};
    bgfx::UniformHandle m_u_view{};
    bgfx::UniformHandle m_u_projection{};
    
    std::vector<bgfx::ProgramHandle> m_loadedShaders;
    std::vector<bgfx::TextureHandle> m_loadedTextures;
};

}
