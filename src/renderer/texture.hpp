#pragma once

#include "core/types.hpp"
#include <bgfx/bgfx.h>
#include <string>

namespace viber {

enum class TextureFormat {
    Unknown,
    R8,
    RG8,
    RGB8,
    RGBA8,
    R16F,
    RG16F,
    RGB16F,
    RGBA16F,
    R32F,
    RG32F,
    RGB32F,
    RGBA32F,
    Depth,
    DepthStencil,
};

struct TextureDesc {
    u32 width = 1;
    u32 height = 1;
    TextureFormat format = TextureFormat::RGBA8;
    bool generateMips = true;
    bool sRGB = false;
    bool renderTarget = false;
    bool depthStencil = false;
};

class Texture {
public:
    Texture();
    explicit Texture(const TextureDesc& desc);
    ~Texture();
    
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    
    bool create(const TextureDesc& desc, const void* data = nullptr, size_t dataSize = 0);
    bool loadFromFile(const std::string& path);
    void destroy();
    
    bool isValid() const { return bgfx::isValid(m_handle); }
    bgfx::TextureHandle getHandle() const { return m_handle; }
    
    u32 getWidth() const { return m_width; }
    u32 getHeight() const { return m_height; }
    TextureFormat getFormat() const { return m_format; }
    
    void bind(u8 stage = 0) const;
    
    static bgfx::TextureFormat::Enum toBgfxFormat(TextureFormat format);
    
private:
    bgfx::TextureHandle m_handle = BGFX_INVALID_HANDLE;
    u32 m_width = 0;
    u32 m_height = 0;
    TextureFormat m_format = TextureFormat::Unknown;
};

}
