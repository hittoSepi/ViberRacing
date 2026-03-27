#include "mesh_loader.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <spdlog/spdlog.h>
#include <fstream>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <cfloat>

namespace viber {

// Helper to check if file is binary STL
static bool isBinarySTL(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    // Read header (80 bytes)
    char header[80];
    file.read(header, 80);
    if (!file.good()) {
        return false;
    }
    
    // Read triangle count
    u32 triangleCount;
    file.read(reinterpret_cast<char*>(&triangleCount), sizeof(triangleCount));
    if (!file.good()) {
        return false;
    }
    
    // Check if file size matches binary format
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    
    // Binary STL: 80 byte header + 4 byte count + 50 bytes per triangle
    size_t expectedSize = 84 + (size_t)triangleCount * 50;
    
    return fileSize == expectedSize;
}

// Parse ASCII STL
static bool parseAsciiSTL(const std::string& path,
                          std::vector<Vertex>& outVertices,
                          std::vector<u32>& outIndices) {
    std::ifstream file(path);
    if (!file.is_open()) {
        spdlog::error("Failed to open STL file: {}", path);
        return false;
    }
    
    std::string line;
    vec3 normal(0.0f, 1.0f, 0.0f);
    std::vector<vec3> tempVertices;
    
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;
        
        if (token == "facet") {
            // Parse normal
            std::string normalStr;
            iss >> normalStr >> normal.x >> normal.y >> normal.z;
        } else if (token == "vertex") {
            // Parse vertex position
            vec3 pos;
            iss >> pos.x >> pos.y >> pos.z;
            tempVertices.push_back(pos);
            
            // When we have 3 vertices, create a triangle
            if (tempVertices.size() == 3) {
                u32 baseIndex = static_cast<u32>(outVertices.size());
                
                for (int i = 0; i < 3; ++i) {
                    Vertex v;
                    v.position = tempVertices[i];
                    v.normal = normal;
                    v.texCoord = vec2(0.0f);
                    v.tangent = vec3(1.0f, 0.0f, 0.0f);
                    v.bitangent = vec3(0.0f, 0.0f, 1.0f);
                    outVertices.push_back(v);
                    outIndices.push_back(baseIndex + i);
                }
                
                tempVertices.clear();
            }
        }
    }
    
    return !outVertices.empty();
}

// Parse binary STL
static bool parseBinarySTL(const std::string& path,
                           std::vector<Vertex>& outVertices,
                           std::vector<u32>& outIndices) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        spdlog::error("Failed to open STL file: {}", path);
        return false;
    }
    
    // Skip 80-byte header
    file.seekg(80);
    
    // Read triangle count
    u32 triangleCount;
    file.read(reinterpret_cast<char*>(&triangleCount), sizeof(triangleCount));
    
    struct STLTriangle {
        float normal[3];
        float vertices[3][3];
        u16 attributeByteCount;
    };
    
    for (u32 i = 0; i < triangleCount; ++i) {
        STLTriangle tri;
        file.read(reinterpret_cast<char*>(&tri), sizeof(STLTriangle));
        
        if (!file.good()) {
            spdlog::error("Failed to read triangle {} from STL file: {}", i, path);
            return false;
        }
        
        vec3 normal(tri.normal[0], tri.normal[1], tri.normal[2]);
        u32 baseIndex = static_cast<u32>(outVertices.size());
        
        for (int j = 0; j < 3; ++j) {
            Vertex v;
            v.position = vec3(tri.vertices[j][0], tri.vertices[j][1], tri.vertices[j][2]);
            v.normal = normal;
            v.texCoord = vec2(0.0f);
            v.tangent = vec3(1.0f, 0.0f, 0.0f);
            v.bitangent = vec3(0.0f, 0.0f, 1.0f);
            outVertices.push_back(v);
            outIndices.push_back(baseIndex + j);
        }
    }
    
    return !outVertices.empty();
}

bool MeshLoader::loadSTL(const std::string& path,
                         std::vector<Vertex>& outVertices,
                         std::vector<u32>& outIndices) {
    outVertices.clear();
    outIndices.clear();
    
    // Determine if binary or ASCII
    bool binary = isBinarySTL(path);
    
    bool success;
    if (binary) {
        spdlog::debug("Loading binary STL: {}", path);
        success = parseBinarySTL(path, outVertices, outIndices);
    } else {
        spdlog::debug("Loading ASCII STL: {}", path);
        success = parseAsciiSTL(path, outVertices, outIndices);
    }
    
    if (success) {
        spdlog::info("Loaded STL mesh: {} ({} vertices, {} indices)",
            path, outVertices.size(), outIndices.size());
    }
    
    return success;
}

bool MeshLoader::loadSTLWithMetadata(const std::string& path, MeshData& outData) {
    outData.vertices.clear();
    outData.indices.clear();
    outData.name = path.substr(path.find_last_of("/\\") + 1);
    outData.minExtents = vec3(FLT_MAX);
    outData.maxExtents = vec3(-FLT_MAX);
    
    if (!loadSTL(path, outData.vertices, outData.indices)) {
        return false;
    }
    
    // Calculate extents
    for (const auto& v : outData.vertices) {
        outData.minExtents = glm::min(outData.minExtents, v.position);
        outData.maxExtents = glm::max(outData.maxExtents, v.position);
    }
    
    return true;
}

bool MeshLoader::load(const std::string& path, 
                      std::vector<Vertex>& outVertices,
                      std::vector<u32>& outIndices) {
    // Check for STL extension and use dedicated STL loader
    std::string ext = path.substr(path.find_last_of('.') + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    
    if (ext == "stl") {
        return loadSTL(path, outVertices, outIndices);
    }
    
    Assimp::Importer importer;
    
    unsigned int flags = aiProcess_Triangulate |
                         aiProcess_GenNormals |
                         aiProcess_CalcTangentSpace |
                         aiProcess_PreTransformVertices |
                         aiProcess_JoinIdenticalVertices |
                         aiProcess_ImproveCacheLocality |
                         aiProcess_RemoveRedundantMaterials;
    
    const aiScene* scene = importer.ReadFile(path, flags);
    
    if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        spdlog::error("Assimp failed to load {}: {}", path, importer.GetErrorString());
        return false;
    }
    
    outVertices.clear();
    outIndices.clear();
    
    u32 vertexOffset = 0;
    
    for (u32 m = 0; m < scene->mNumMeshes; ++m) {
        const aiMesh* mesh = scene->mMeshes[m];
        
        for (u32 v = 0; v < mesh->mNumVertices; ++v) {
            Vertex vertex;
            
            vertex.position = vec3(
                mesh->mVertices[v].x,
                mesh->mVertices[v].y,
                mesh->mVertices[v].z
            );
            
            if (mesh->HasNormals()) {
                vertex.normal = vec3(
                    mesh->mNormals[v].x,
                    mesh->mNormals[v].y,
                    mesh->mNormals[v].z
                );
            } else {
                vertex.normal = vec3(0.0f, 1.0f, 0.0f);
            }
            
            if (mesh->HasTextureCoords(0)) {
                vertex.texCoord = vec2(
                    mesh->mTextureCoords[0][v].x,
                    mesh->mTextureCoords[0][v].y
                );
            } else {
                vertex.texCoord = vec2(0.0f);
            }
            
            if (mesh->HasTangentsAndBitangents()) {
                vertex.tangent = vec3(
                    mesh->mTangents[v].x,
                    mesh->mTangents[v].y,
                    mesh->mTangents[v].z
                );
                vertex.bitangent = vec3(
                    mesh->mBitangents[v].x,
                    mesh->mBitangents[v].y,
                    mesh->mBitangents[v].z
                );
            } else {
                vertex.tangent = vec3(1.0f, 0.0f, 0.0f);
                vertex.bitangent = vec3(0.0f, 0.0f, 1.0f);
            }
            
            outVertices.push_back(vertex);
        }
        
        for (u32 f = 0; f < mesh->mNumFaces; ++f) {
            const aiFace& face = mesh->mFaces[f];
            for (u32 i = 0; i < face.mNumIndices; ++i) {
                outIndices.push_back(vertexOffset + face.mIndices[i]);
            }
        }
        
        vertexOffset += mesh->mNumVertices;
    }
    
    spdlog::info("Loaded mesh: {} ({} vertices, {} indices)", 
        path, outVertices.size(), outIndices.size());
    
    return true;
}

bool MeshLoader::loadWithMetadata(const std::string& path, MeshData& outData) {
    // Check for STL extension and use dedicated STL loader
    std::string ext = path.substr(path.find_last_of('.') + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    
    if (ext == "stl") {
        return loadSTLWithMetadata(path, outData);
    }
    
    Assimp::Importer importer;
    
    unsigned int flags = aiProcess_Triangulate |
                         aiProcess_GenNormals |
                         aiProcess_CalcTangentSpace |
                         aiProcess_PreTransformVertices |
                         aiProcess_JoinIdenticalVertices |
                         aiProcess_ImproveCacheLocality;
    
    const aiScene* scene = importer.ReadFile(path, flags);
    
    if (!scene || !scene->mRootNode) {
        spdlog::error("Assimp failed to load {}: {}", path, importer.GetErrorString());
        return false;
    }
    
    outData.name = path.substr(path.find_last_of("/\\") + 1);
    outData.minExtents = vec3(FLT_MAX);
    outData.maxExtents = vec3(-FLT_MAX);
    
    u32 vertexOffset = 0;
    
    for (u32 m = 0; m < scene->mNumMeshes; ++m) {
        const aiMesh* mesh = scene->mMeshes[m];
        
        for (u32 v = 0; v < mesh->mNumVertices; ++v) {
            Vertex vertex;
            
            vertex.position = vec3(
                mesh->mVertices[v].x,
                mesh->mVertices[v].y,
                mesh->mVertices[v].z
            );
            
            outData.minExtents = glm::min(outData.minExtents, vertex.position);
            outData.maxExtents = glm::max(outData.maxExtents, vertex.position);
            
            if (mesh->HasNormals()) {
                vertex.normal = vec3(mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z);
            } else {
                vertex.normal = vec3(0.0f, 1.0f, 0.0f);
            }
            
            if (mesh->HasTextureCoords(0)) {
                vertex.texCoord = vec2(mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y);
            } else {
                vertex.texCoord = vec2(0.0f);
            }
            
            if (mesh->HasTangentsAndBitangents()) {
                vertex.tangent = vec3(mesh->mTangents[v].x, mesh->mTangents[v].y, mesh->mTangents[v].z);
                vertex.bitangent = vec3(mesh->mBitangents[v].x, mesh->mBitangents[v].y, mesh->mBitangents[v].z);
            } else {
                vertex.tangent = vec3(1.0f, 0.0f, 0.0f);
                vertex.bitangent = vec3(0.0f, 0.0f, 1.0f);
            }
            
            outData.vertices.push_back(vertex);
        }
        
        for (u32 f = 0; f < mesh->mNumFaces; ++f) {
            const aiFace& face = mesh->mFaces[f];
            for (u32 i = 0; i < face.mNumIndices; ++i) {
                outData.indices.push_back(vertexOffset + face.mIndices[i]);
            }
        }
        
        vertexOffset += mesh->mNumVertices;
    }
    
    return true;
}

}
