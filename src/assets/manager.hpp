#pragma once

#include "core/types.hpp"
#include "renderer/mesh.hpp"
#include "renderer/texture.hpp"
#include "renderer/shader.hpp"
#include <unordered_map>
#include <string>
#include <memory>
#include <future>
#include <queue>
#include <mutex>
#include <functional>

namespace viber {

template<typename T>
using AssetHandle = u32;

constexpr AssetHandle<u32> INVALID_ASSET = 0;

struct AssetInfo {
    std::string path;
    std::string name;
    bool loaded = false;
    bool loading = false;
    bool failed = false;
};

class AssetManager {
public:
    AssetManager();
    ~AssetManager();
    
    AssetHandle<Mesh> loadMesh(const std::string& path);
    AssetHandle<Texture> loadTexture(const std::string& path);
    AssetHandle<Shader> loadShader(const std::string& name, 
                                    const std::string& vsPath, 
                                    const std::string& fsPath);
    
    Mesh* getMesh(AssetHandle<Mesh> handle);
    Texture* getTexture(AssetHandle<Texture> handle);
    Shader* getShader(AssetHandle<Shader> handle);
    
    void releaseMesh(AssetHandle<Mesh> handle);
    void releaseTexture(AssetHandle<Texture> handle);
    void releaseShader(AssetHandle<Shader> handle);
    
    bool isLoaded(AssetHandle<Mesh> handle) const;
    bool isLoaded(AssetHandle<Texture> handle) const;
    
    void processQueue();
    void waitForAll();
    
    void setAssetRoot(const std::string& root) { m_assetRoot = root; }
    const std::string& getAssetRoot() const { return m_assetRoot; }
    
    std::string resolvePath(const std::string& relativePath) const;
    
    void shutdown();
    
private:
    template<typename T>
    struct AssetEntry {
        std::unique_ptr<T> asset;
        AssetInfo info;
    };
    
    AssetHandle<u32> nextHandle() { return ++m_nextHandle; }
    
    std::unordered_map<AssetHandle<Mesh>, AssetEntry<Mesh>> m_meshes;
    std::unordered_map<AssetHandle<Texture>, AssetEntry<Texture>> m_textures;
    std::unordered_map<AssetHandle<Shader>, AssetEntry<Shader>> m_shaders;
    
    std::unordered_map<std::string, AssetHandle<Mesh>> m_meshPathMap;
    std::unordered_map<std::string, AssetHandle<Texture>> m_texturePathMap;
    
    std::queue<std::function<void()>> m_loadQueue;
    std::mutex m_queueMutex;
    
    std::string m_assetRoot = "assets/";
    AssetHandle<u32> m_nextHandle = 0;
};

}
