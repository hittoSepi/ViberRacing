#include "mesh_loader.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <spdlog/spdlog.h>

namespace viber {

bool MeshLoader::load(const std::string& path, 
                      std::vector<Vertex>& outVertices,
                      std::vector<u32>& outIndices) {
    Assimp::Importer importer;
    
    unsigned int flags = aiProcess_Triangulate |
                         aiProcess_GenNormals |
                         aiProcess_CalcTangentSpace |
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
    Assimp::Importer importer;
    
    unsigned int flags = aiProcess_Triangulate |
                         aiProcess_GenNormals |
                         aiProcess_CalcTangentSpace |
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
