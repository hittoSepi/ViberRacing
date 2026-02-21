#include "shader.hpp"
#include <spdlog/spdlog.h>
#include <fstream>
#include <vector>

namespace viber {

Shader::Shader() = default;

Shader::~Shader() {
    destroy();
}

bool Shader::loadFromFiles(const std::string& vsPath, const std::string& fsPath) {
    destroy();
    
    bgfx::ShaderHandle vs = loadShader(vsPath);
    bgfx::ShaderHandle fs = loadShader(fsPath);
    
    if (!bgfx::isValid(vs) || !bgfx::isValid(fs)) {
        if (bgfx::isValid(vs)) bgfx::destroy(vs);
        if (bgfx::isValid(fs)) bgfx::destroy(fs);
        return false;
    }
    
    m_program = bgfx::createProgram(vs, fs, true);
    
    if (!bgfx::isValid(m_program)) {
        spdlog::error("Failed to create shader program: {} + {}", vsPath, fsPath);
        return false;
    }
    
    spdlog::info("Loaded shader: {} + {}", vsPath, fsPath);
    return true;
}

void Shader::destroy() {
    for (auto& [name, handle] : m_uniforms) {
        if (bgfx::isValid(handle)) {
            bgfx::destroy(handle);
        }
    }
    m_uniforms.clear();
    
    if (bgfx::isValid(m_program)) {
        bgfx::destroy(m_program);
        m_program = BGFX_INVALID_HANDLE;
    }
}

bgfx::ShaderHandle Shader::loadShader(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        spdlog::error("Failed to open shader file: {}", path);
        return BGFX_INVALID_HANDLE;
    }
    
    size_t size = file.tellg();
    file.seekg(0);
    
    std::vector<u8> data(size);
    file.read(reinterpret_cast<char*>(data.data()), size);
    
    const bgfx::Memory* mem = bgfx::copy(data.data(), static_cast<u32>(size));
    return bgfx::createShader(mem);
}

bgfx::UniformHandle Shader::getUniform(const std::string& name) const {
    auto it = m_uniforms.find(name);
    return it != m_uniforms.end() ? it->second : BGFX_INVALID_HANDLE;
}

bgfx::UniformHandle Shader::createUniform(const std::string& name, 
                                           bgfx::UniformType::Enum type, u16 num) {
    auto it = m_uniforms.find(name);
    if (it != m_uniforms.end()) {
        return it->second;
    }
    
    bgfx::UniformHandle handle = bgfx::createUniform(name.c_str(), type, num);
    if (bgfx::isValid(handle)) {
        m_uniforms[name] = handle;
    }
    return handle;
}

void Shader::setUniform(const std::string& name, const void* value, size_t size) {
    bgfx::UniformHandle handle = getUniform(name);
    if (bgfx::isValid(handle)) {
        bgfx::setUniform(handle, value, static_cast<u16>(size / 4));
    }
}

void Shader::setTexture(const std::string& name, bgfx::TextureHandle texture, u8 stage) {
    bgfx::UniformHandle handle = getUniform(name);
    if (bgfx::isValid(handle)) {
        bgfx::setTexture(stage, handle, texture);
    }
}

void Shader::bind() const {
}

ShaderManager& ShaderManager::get() {
    static ShaderManager instance;
    return instance;
}

Shader* ShaderManager::load(const std::string& name, const std::string& vsPath, const std::string& fsPath) {
    auto shader = std::make_unique<Shader>();
    if (!shader->loadFromFiles(vsPath, fsPath)) {
        return nullptr;
    }
    
    m_shaders[name] = std::move(shader);
    return m_shaders[name].get();
}

Shader* ShaderManager::get(const std::string& name) {
    auto it = m_shaders.find(name);
    return it != m_shaders.end() ? it->second.get() : nullptr;
}

void ShaderManager::unload(const std::string& name) {
    m_shaders.erase(name);
}

void ShaderManager::unloadAll() {
    m_shaders.clear();
}

}
