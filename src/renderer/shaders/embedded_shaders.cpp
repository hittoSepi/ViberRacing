#include "embedded_shaders.hpp"
#include <spdlog/spdlog.h>
#include <fstream>
#include <vector>

namespace viber {

static bgfx::ShaderHandle loadShaderFromFile(const char* path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        spdlog::warn("Shader not found: {}", path);
        return BGFX_INVALID_HANDLE;
    }
    
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> data(size);
    file.read(reinterpret_cast<char*>(data.data()), size);
    
    const bgfx::Memory* mem = bgfx::copy(data.data(), static_cast<uint32_t>(size));
    return bgfx::createShader(mem);
}

bgfx::ProgramHandle createBasicShader() {
    bgfx::ShaderHandle vs = loadShaderFromFile("assets/shaders/vs_basic.bin");
    bgfx::ShaderHandle fs = loadShaderFromFile("assets/shaders/fs_basic.bin");
    
    if (!bgfx::isValid(vs) || !bgfx::isValid(fs)) {
        return BGFX_INVALID_HANDLE;
    }
    
    return bgfx::createProgram(vs, fs, true);
}

bgfx::ProgramHandle createTexturedShader() {
    bgfx::ShaderHandle vs = loadShaderFromFile("assets/shaders/vs_textured.bin");
    bgfx::ShaderHandle fs = loadShaderFromFile("assets/shaders/fs_textured.bin");
    
    if (!bgfx::isValid(vs) || !bgfx::isValid(fs)) {
        return BGFX_INVALID_HANDLE;
    }
    
    return bgfx::createProgram(vs, fs, true);
}

} // namespace viber
