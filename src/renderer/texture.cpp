#include "texture.hpp"
#include <spdlog/spdlog.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace viber {

Texture::Texture() = default;

Texture::Texture(const TextureDesc& desc) {
    create(desc);
}

Texture::~Texture() {
    destroy();
}

bgfx::TextureFormat::Enum Texture::toBgfxFormat(TextureFormat format) {
    switch (format) {
        case TextureFormat::R8:          return bgfx::TextureFormat::R8;
        case TextureFormat::RG8:         return bgfx::TextureFormat::RG8;
        case TextureFormat::RGB8:        return bgfx::TextureFormat::RGB8;
        case TextureFormat::RGBA8:       return bgfx::TextureFormat::RGBA8;
        case TextureFormat::R16F:        return bgfx::TextureFormat::R16F;
        case TextureFormat::RG16F:       return bgfx::TextureFormat::RG16F;
        case TextureFormat::RGB16F:      return bgfx::TextureFormat::RGBA16F;  // RGB16F not available, use RGBA16F
        case TextureFormat::RGBA16F:     return bgfx::TextureFormat::RGBA16F;
        case TextureFormat::R32F:        return bgfx::TextureFormat::R32F;
        case TextureFormat::RG32F:       return bgfx::TextureFormat::RG32F;
        case TextureFormat::RGB32F:      return bgfx::TextureFormat::RGBA32F;  // RGB32F not available, use RGBA32F
        case TextureFormat::RGBA32F:     return bgfx::TextureFormat::RGBA32F;
        case TextureFormat::Depth:       return bgfx::TextureFormat::D32;
        case TextureFormat::DepthStencil: return bgfx::TextureFormat::D24S8;
        default:                         return bgfx::TextureFormat::Unknown;
    }
}

bool Texture::create(const TextureDesc& desc, const void* data, size_t dataSize) {
    destroy();
    
    uint64_t flags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_W_CLAMP;
    
    if (desc.generateMips) {
        flags |= BGFX_CAPS_FORMAT_TEXTURE_MIP_AUTOGEN;
    }
    
    if (desc.sRGB) {
        flags |= BGFX_TEXTURE_SRGB;
    }
    
    if (desc.renderTarget) {
        flags |= BGFX_TEXTURE_RT;
    }
    
    bgfx::TextureFormat::Enum bgfxFormat = toBgfxFormat(desc.format);
    
    if (data && dataSize > 0) {
        const bgfx::Memory* mem = bgfx::copy(data, static_cast<u32>(dataSize));
        m_handle = bgfx::createTexture2D(
            desc.width, desc.height,
            desc.generateMips,
            1,
            bgfxFormat,
            flags,
            mem
        );
    } else {
        m_handle = bgfx::createTexture2D(
            desc.width, desc.height,
            desc.generateMips,
            1,
            bgfxFormat,
            flags
        );
    }
    
    if (!bgfx::isValid(m_handle)) {
        spdlog::error("Failed to create texture {}x{}", desc.width, desc.height);
        return false;
    }
    
    m_width = desc.width;
    m_height = desc.height;
    m_format = desc.format;
    
    return true;
}

bool Texture::loadFromFile(const std::string& path) {
    destroy();
    
    int width, height, channels;
    stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channels, 4);
    
    if (!pixels) {
        spdlog::error("Failed to load texture: {}", path);
        return false;
    }
    
    TextureDesc desc;
    desc.width = static_cast<u32>(width);
    desc.height = static_cast<u32>(height);
    desc.format = TextureFormat::RGBA8;
    desc.generateMips = true;
    desc.sRGB = true;
    
    bool result = create(desc, pixels, width * height * 4);
    
    stbi_image_free(pixels);
    
    if (result) {
        spdlog::info("Loaded texture: {} ({}x{})", path, width, height);
    }
    
    return result;
}

void Texture::destroy() {
    if (bgfx::isValid(m_handle)) {
        bgfx::destroy(m_handle);
        m_handle = BGFX_INVALID_HANDLE;
    }
    m_width = 0;
    m_height = 0;
    m_format = TextureFormat::Unknown;
}

void Texture::bind(u8 stage) const {
    if (isValid()) {
        bgfx::setTexture(stage, {static_cast<u16>(stage)}, m_handle);
    }
}

}
