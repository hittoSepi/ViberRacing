#pragma once

#include "core/types.hpp"
#include <bgfx/bgfx.h>
#include <string>
#include <unordered_map>

namespace viber {

class Shader {
public:
    Shader();
    ~Shader();
    
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    
    bool loadFromFiles(const std::string& vsPath, const std::string& fsPath);
    void destroy();
    
    bool isValid() const { return bgfx::isValid(m_program); }
    bgfx::ProgramHandle getProgram() const { return m_program; }
    
    bgfx::UniformHandle getUniform(const std::string& name) const;
    bgfx::UniformHandle createUniform(const std::string& name, 
                                       bgfx::UniformType::Enum type, u16 num = 1);
    
    void setUniform(const std::string& name, const void* value, size_t size);
    void setTexture(const std::string& name, bgfx::TextureHandle texture, u8 stage = 0);
    
    void bind() const;
    
private:
    bgfx::ProgramHandle m_program = BGFX_INVALID_HANDLE;
    std::unordered_map<std::string, bgfx::UniformHandle> m_uniforms;
    
    bgfx::ShaderHandle loadShader(const std::string& path);
};

class ShaderManager {
public:
    static ShaderManager& get();
    
    Shader* load(const std::string& name, const std::string& vsPath, const std::string& fsPath);
    Shader* get(const std::string& name);
    void unload(const std::string& name);
    void unloadAll();
    
private:
    ShaderManager() = default;
    std::unordered_map<std::string, std::unique_ptr<Shader>> m_shaders;
};

}
