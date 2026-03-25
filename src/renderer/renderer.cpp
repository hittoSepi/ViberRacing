#if defined(__linux__)
#define GLFW_EXPOSE_NATIVE_X11
#endif

#include "renderer.hpp"
#include <bgfx/platform.h>
#include <bx/bx.h>
#include <spdlog/spdlog.h>
#include <fstream>
#include <vector>



namespace bgfx {
    // Define operators for bgfx handles (needed for C++20)
    inline bool operator==(const TextureHandle& a, const TextureHandle& b) { return a.idx == b.idx; }
    inline bool operator!=(const TextureHandle& a, const TextureHandle& b) { return a.idx != b.idx; }
    inline bool operator==(const ShaderHandle& a, const ShaderHandle& b) { return a.idx == b.idx; }
    inline bool operator!=(const ShaderHandle& a, const ShaderHandle& b) { return a.idx != b.idx; }
    inline bool operator==(const ProgramHandle& a, const ProgramHandle& b) { return a.idx == b.idx; }
    inline bool operator!=(const ProgramHandle& a, const ProgramHandle& b) { return a.idx != b.idx; }
    inline bool operator==(const UniformHandle& a, const UniformHandle& b) { return a.idx == b.idx; }
    inline bool operator!=(const UniformHandle& a, const UniformHandle& b) { return a.idx != b.idx; }
}

#ifndef CMAKE_BUILD_TYPE
#define CMAKE_BUILD_TYPE "Debug"
#endif

namespace viber {

Renderer::Renderer(Window& window) : m_window(window) {
    m_windowSize = window.getSize();
    
    spdlog::info("Initializing renderer with window size: {}x{}", m_windowSize.x, m_windowSize.y);
    
    bgfx::Init init;
    // Use Vulkan on Linux for better Wayland compatibility
    init.type = bgfx::RendererType::Vulkan;
    init.vendorId = BGFX_PCI_ID_NONE;
    init.deviceId = 0;
    init.platformData.nwh = nullptr;
    init.platformData.ndt = nullptr;
    init.platformData.context = nullptr;
    init.platformData.backBuffer = nullptr;
    init.platformData.backBufferDS = nullptr;
    init.resolution.width = static_cast<u32>(m_windowSize.x);
    init.resolution.height = static_cast<u32>(m_windowSize.y);
    init.resolution.reset = BGFX_RESET_VSYNC | BGFX_RESET_MSAA_X4;
    
#ifdef _WIN32
    init.platformData.nwh = glfwGetWin32Window(window.getHandle());
    spdlog::info("Win32 platform data set");
#elif defined(__APPLE__)
    init.platformData.nwh = glfwGetCocoaWindow(window.getHandle());
    spdlog::info("Cocoa platform data set");
#elif defined(__linux__)
    init.platformData.ndt = window.getNativeDisplay();
    init.platformData.nwh = window.getNativeWindow();
    // Detect Wayland vs X11 based on display type
    if (window.isWayland()) {
        init.platformData.type = bgfx::NativeWindowHandleType::Wayland;
        spdlog::info("Wayland platform data set: type={}, display={}, window={}", 
            (int)init.platformData.type, init.platformData.ndt, init.platformData.nwh);
    } else {
        init.platformData.type = bgfx::NativeWindowHandleType::Default;
        spdlog::info("X11 platform data set: display={}, window={}", init.platformData.ndt, init.platformData.nwh);
    }
#endif
    
    m_init = init;
    
    if (!bgfx::init(init)) {
        spdlog::error("bgfx::init failed - platform data: ndt={}, nwh={}", init.platformData.ndt, init.platformData.nwh);
        throw std::runtime_error("Failed to initialize bgfx");
    }
    
    bgfx::setDebug(BGFX_DEBUG_TEXT);
    
    // Use different names to avoid conflict with bgfx internal uniforms
    // bgfx reserves names starting with certain prefixes
    m_u_model = bgfx::createUniform("model_mtx", bgfx::UniformType::Mat4);
    m_u_view = bgfx::createUniform("view_mtx", bgfx::UniformType::Mat4);
    m_u_projection = bgfx::createUniform("proj_mtx", bgfx::UniformType::Mat4);
    
    const bgfx::Caps* caps = bgfx::getCaps();
    spdlog::info("Renderer initialized: {} (vendor: {}, device: {})",
        caps->vendorId == 0x1002 ? "AMD" :
        caps->vendorId == 0x10DE ? "NVIDIA" :
        caps->vendorId == 0x8086 ? "Intel" : "Unknown",
        caps->vendorId,
        caps->deviceId
    );
    spdlog::info("Resolution: {}x{}", m_windowSize.x, m_windowSize.y);
    
    window.setResizeCallback([this](int width, int height) {
        m_windowSize = vec2(static_cast<float>(width), static_cast<float>(height));
        bgfx::reset(width, height, BGFX_RESET_VSYNC | BGFX_RESET_MSAA_X4);
    });
}

Renderer::~Renderer() {
    for (auto handle : m_loadedShaders) {
        if (bgfx::isValid(handle)) {
            bgfx::destroy(handle);
        }
    }
    for (auto handle : m_loadedTextures) {
        if (bgfx::isValid(handle)) {
            bgfx::destroy(handle);
        }
    }
    
    if (bgfx::isValid(m_u_model)) bgfx::destroy(m_u_model);
    if (bgfx::isValid(m_u_view)) bgfx::destroy(m_u_view);
    if (bgfx::isValid(m_u_projection)) bgfx::destroy(m_u_projection);
    
    bgfx::shutdown();
    spdlog::info("Renderer shutdown");
}

void Renderer::beginFrame() {
    bgfx::setViewClear(0, 
        BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL,
        *reinterpret_cast<const u32*>(&m_clearColor), 
        1.0f, 
        0
    );
    
    bgfx::setViewRect(0, 0, 0, 
        static_cast<u16>(m_windowSize.x),
        static_cast<u16>(m_windowSize.y)
    );
}

void Renderer::endFrame() {
    bgfx::frame();
    
    const bgfx::Stats* bgfxStats = bgfx::getStats();
    m_stats.cpuTimeMs = static_cast<float>(bgfxStats->cpuTimeEnd - bgfxStats->cpuTimeBegin) / 1000.0f;
    m_stats.gpuTimeMs = static_cast<float>(bgfxStats->gpuTimeEnd - bgfxStats->gpuTimeBegin) / 1000000.0f;
}

void Renderer::setClearColor(const vec4& color) {
    m_clearColor = color;
}

void Renderer::setViewport(int x, int y, int width, int height) {
    bgfx::setViewRect(0, static_cast<u16>(x), static_cast<u16>(y),
        static_cast<u16>(width), static_cast<u16>(height));
}

void Renderer::setViewProjection(const mat4& view, const mat4& projection) {
    bgfx::setViewTransform(0, &view, &projection);
}

void Renderer::setViewTransform(const Camera& camera) {
    mat4 view = camera.getViewMatrix();
    mat4 projection = camera.getProjectionMatrix();
    setViewProjection(view, projection);
}

bgfx::ProgramHandle Renderer::loadShader(const std::string& vsPath, const std::string& fsPath) {
    auto loadBinary = [](const std::string& path) -> std::vector<u8> {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            spdlog::error("Failed to load shader: {}", path);
            return {};
        }
        
        size_t size = file.tellg();
        file.seekg(0);
        
        std::vector<u8> data(size);
        file.read(reinterpret_cast<char*>(data.data()), size);
        return data;
    };
    
    auto vsData = loadBinary(vsPath);
    auto fsData = loadBinary(fsPath);
    
    if (vsData.empty() || fsData.empty()) {
        return BGFX_INVALID_HANDLE;
    }
    
    const bgfx::Memory* vsMem = bgfx::copy(vsData.data(), static_cast<u32>(vsData.size()));
    const bgfx::Memory* fsMem = bgfx::copy(fsData.data(), static_cast<u32>(fsData.size()));
    
    bgfx::ShaderHandle vs = bgfx::createShader(vsMem);
    bgfx::ShaderHandle fs = bgfx::createShader(fsMem);
    
    if (!bgfx::isValid(vs) || !bgfx::isValid(fs)) {
        if (bgfx::isValid(vs)) bgfx::destroy(vs);
        if (bgfx::isValid(fs)) bgfx::destroy(fs);
        return BGFX_INVALID_HANDLE;
    }
    
    bgfx::ProgramHandle program = bgfx::createProgram(vs, fs, true);
    
    if (bgfx::isValid(program)) {
        m_loadedShaders.push_back(program);
        spdlog::info("Loaded shader program: {} + {}", vsPath, fsPath);
    }
    
    return program;
}

void Renderer::destroyShader(bgfx::ProgramHandle handle) {
    if (bgfx::isValid(handle)) {
        bgfx::destroy(handle);
        auto it = std::find(m_loadedShaders.begin(), m_loadedShaders.end(), handle);
        if (it != m_loadedShaders.end()) {
            m_loadedShaders.erase(it);
        }
    }
}

bgfx::TextureHandle Renderer::loadTexture(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        spdlog::error("Failed to load texture: {}", path);
        return BGFX_INVALID_HANDLE;
    }
    
    size_t size = file.tellg();
    file.seekg(0);
    
    std::vector<u8> data(size);
    file.read(reinterpret_cast<char*>(data.data()), size);
    
    const bgfx::Memory* mem = bgfx::copy(data.data(), static_cast<u32>(size));
    bgfx::TextureHandle handle = bgfx::createTexture(mem, 0, 0);
    
    if (bgfx::isValid(handle)) {
        m_loadedTextures.push_back(handle);
        spdlog::info("Loaded texture: {} ({} bytes)", path, size);
    }
    
    return handle;
}

void Renderer::destroyTexture(bgfx::TextureHandle handle) {
    if (bgfx::isValid(handle)) {
        bgfx::destroy(handle);
        auto it = std::find(m_loadedTextures.begin(), m_loadedTextures.end(), handle);
        if (it != m_loadedTextures.end()) {
            m_loadedTextures.erase(it);
        }
    }
}

bgfx::VertexBufferHandle Renderer::createVertexBuffer(const void* data, size_t size, 
                                                       const bgfx::VertexLayout& layout) {
    const bgfx::Memory* mem = bgfx::copy(data, static_cast<u32>(size));
    return bgfx::createVertexBuffer(mem, layout);
}

bgfx::IndexBufferHandle Renderer::createIndexBuffer(const void* data, size_t size, bool indices32) {
    const bgfx::Memory* mem = bgfx::copy(data, static_cast<u32>(size));
    return bgfx::createIndexBuffer(mem, 
        indices32 ? BGFX_BUFFER_INDEX32 : 0);
}

void Renderer::destroyVertexBuffer(bgfx::VertexBufferHandle handle) {
    if (bgfx::isValid(handle)) {
        bgfx::destroy(handle);
    }
}

void Renderer::destroyIndexBuffer(bgfx::IndexBufferHandle handle) {
    if (bgfx::isValid(handle)) {
        bgfx::destroy(handle);
    }
}

void Renderer::submit(bgfx::ViewId viewId, bgfx::ProgramHandle program,
                      bgfx::VertexBufferHandle vbh, bgfx::IndexBufferHandle ibh,
                      const mat4& transform, u64 state) {
    bgfx::setTransform(&transform);
    bgfx::setVertexBuffer(0, vbh);
    bgfx::setIndexBuffer(ibh);
    bgfx::setState(state);
    bgfx::submit(viewId, program);
}

void Renderer::setUniform(bgfx::UniformHandle handle, const void* value, size_t size) {
    bgfx::setUniform(handle, value, static_cast<u16>(size));
}

bgfx::UniformHandle Renderer::createUniform(const std::string& name, bgfx::UniformType::Enum type, u16 num) {
    return bgfx::createUniform(name.c_str(), type, num);
}

void Renderer::destroyUniform(bgfx::UniformHandle handle) {
    if (bgfx::isValid(handle)) {
        bgfx::destroy(handle);
    }
}

}
