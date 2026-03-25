#include "embedded_shaders.hpp"
#include <spdlog/spdlog.h>
#include <filesystem>
#include <fstream>
#include <vector>

namespace viber {

static std::string findShaderPath(const char* name) {
    const std::vector<std::string> candidates = {
        std::string("assets/shaders/") + name,
        std::string("../assets/shaders/") + name,
        std::string("../../assets/shaders/") + name,
    };

    for (const auto& path : candidates) {
        if (std::filesystem::exists(path)) {
            return path;
        }
    }

    return candidates.front();
}

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

static bgfx::ProgramHandle createProgramFromFiles(const char* vsName, const char* fsName) {
    const std::string vsPath = findShaderPath(vsName);
    const std::string fsPath = findShaderPath(fsName);

    bgfx::ShaderHandle vs = loadShaderFromFile(vsPath.c_str());
    bgfx::ShaderHandle fs = loadShaderFromFile(fsPath.c_str());
    
    if (!bgfx::isValid(vs) || !bgfx::isValid(fs)) {
        if (bgfx::isValid(vs)) {
            bgfx::destroy(vs);
        }
        if (bgfx::isValid(fs)) {
            bgfx::destroy(fs);
        }
        return BGFX_INVALID_HANDLE;
    }
    
    return bgfx::createProgram(vs, fs, true);
}

bgfx::ProgramHandle createBasicShader() {
    return createProgramFromFiles("vs_basic.bin", "fs_basic.bin");
}

bgfx::ProgramHandle createTexturedShader() {
    return createProgramFromFiles("vs_textured.bin", "fs_textured.bin");
}

bgfx::ProgramHandle createAtmosphereShader() {
    return createProgramFromFiles("vs_atmosphere.bin", "fs_atmosphere.bin");
}

} // namespace viber
