#pragma once

#include "renderer/mesh.hpp"
#include <string>
#include <vector>

namespace viber {

class MeshLoader {
public:
    static bool load(const std::string& path, 
                     std::vector<Vertex>& outVertices,
                     std::vector<u32>& outIndices);
    
    struct MeshData {
        std::vector<Vertex> vertices;
        std::vector<u32> indices;
        std::string name;
        
        vec3 minExtents;
        vec3 maxExtents;
    };
    
    static bool loadWithMetadata(const std::string& path, MeshData& outData);
    
    // Dedicated STL loader (supports both ASCII and binary formats)
    static bool loadSTL(const std::string& path,
                        std::vector<Vertex>& outVertices,
                        std::vector<u32>& outIndices);
    
    static bool loadSTLWithMetadata(const std::string& path, MeshData& outData);
};

}
