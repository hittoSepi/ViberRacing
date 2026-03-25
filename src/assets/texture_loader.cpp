#include "texture_loader.hpp"
#include <spdlog/spdlog.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <stb_image_resize2.h>

namespace viber {

bool TextureLoader::loadFromFile(const std::string& path, TextureDesc& outDesc,
                                  std::vector<u8>& outData) {
    int width, height, channels;
    stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channels, 4);
    
    if (!pixels) {
        spdlog::error("Failed to load texture: {}", path);
        return false;
    }
    
    outDesc.width = static_cast<u32>(width);
    outDesc.height = static_cast<u32>(height);
    outDesc.format = TextureFormat::RGBA8;
    
    size_t dataSize = width * height * 4;
    outData.resize(dataSize);
    std::memcpy(outData.data(), pixels, dataSize);
    
    stbi_image_free(pixels);
    
    return true;
}

bool TextureLoader::loadHDR(const std::string& path, std::vector<float>& outData,
                             u32& outWidth, u32& outHeight) {
    int width, height, channels;
    float* pixels = stbi_loadf(path.c_str(), &width, &height, &channels, 4);
    
    if (!pixels) {
        spdlog::error("Failed to load HDR texture: {}", path);
        return false;
    }
    
    outWidth = static_cast<u32>(width);
    outHeight = static_cast<u32>(height);
    
    size_t dataSize = width * height * 4;
    outData.resize(dataSize);
    std::memcpy(outData.data(), pixels, dataSize * sizeof(float));
    
    stbi_image_free(pixels);
    
    return true;
}

bool TextureLoader::generateMipmaps(const u8* data, u32 width, u32 height, u32 channels,
                                     std::vector<std::vector<u8>>& outMips) {
    outMips.clear();
    
    if (!data || width == 0 || height == 0) {
        return false;
    }
    
    outMips.push_back(std::vector<u8>(data, data + width * height * channels));
    
    u32 currentWidth = width;
    u32 currentHeight = height;
    
    while (currentWidth > 1 || currentHeight > 1) {
        u32 nextWidth = std::max(1u, currentWidth / 2);
        u32 nextHeight = std::max(1u, currentHeight / 2);
        
        std::vector<u8>& prevLevel = outMips.back();
        std::vector<u8> nextLevel(nextWidth * nextHeight * channels);
        
        stbir_resize_uint8_linear(
            prevLevel.data(), currentWidth, currentHeight, 0,
            nextLevel.data(), nextWidth, nextHeight, 0,
            static_cast<stbir_pixel_layout>(channels)
        );
        
        outMips.push_back(std::move(nextLevel));
        
        currentWidth = nextWidth;
        currentHeight = nextHeight;
    }
    
    return true;
}

}
