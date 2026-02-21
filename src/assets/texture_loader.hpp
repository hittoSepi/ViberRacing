#pragma once

#include "renderer/texture.hpp"
#include <string>

namespace viber {

class TextureLoader {
public:
    static bool loadFromFile(const std::string& path, TextureDesc& outDesc, 
                             std::vector<u8>& outData);
    
    static bool loadHDR(const std::string& path, std::vector<float>& outData,
                        u32& outWidth, u32& outHeight);
    
    static bool generateMipmaps(const u8* data, u32 width, u32 height, u32 channels,
                                 std::vector<std::vector<u8>>& outMips);
};

}
