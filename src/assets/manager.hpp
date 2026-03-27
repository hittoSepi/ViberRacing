#pragma once

#include "core/types.hpp"
#include "renderer/mesh.hpp"
#include "renderer/texture.hpp"
#include "renderer/shader.hpp"
#include <unordered_map>
#include <string>
#include <memory>
#include <queue>
#include <mutex>
#include <functional>
#include <thread>
#include <condition_variable>
#include <variant>
#include <atomic>

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
    
    bool isMeshLoaded(AssetHandle<Mesh> handle) const;
    bool isTextureLoaded(AssetHandle<Texture> handle) const;
    
    void processQueue();
    void waitForAll();
    
    void setAssetRoot(const std::string& root) { m_assetRoot = root; }
    const std::string& getAssetRoot() const { return m_assetRoot; }
    
    std::string resolvePath(const std::string& relativePath) const;
    
    void shutdown();
    
private:
    enum class RequestType {
        Mesh,
        Texture,
        Shader
    };

    struct LoadRequest {
        RequestType type;
        AssetHandle<u32> handle;
        std::string path;
    };

    struct MeshLoadResult {
        std::vector<Vertex> vertices;
        std::vector<u32> indices;
    };

    struct TextureLoadResult {
        TextureDesc desc;
        std::vector<u8> data;
    };

    struct ShaderLoadResult {
        std::vector<u8> vsData;
        std::vector<u8> fsData;
        std::string name;
        std::string vsPath;
        std::string fsPath;
    };

    struct CompletedRequest {
        RequestType type;
        AssetHandle<u32> handle;
        std::string path;
        bool success = false;
        std::string error;
        std::variant<std::monostate, MeshLoadResult, TextureLoadResult, ShaderLoadResult> payload;
    };

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
    
    std::queue<LoadRequest> m_requestQueue;
    std::queue<CompletedRequest> m_completedQueue;
    std::mutex m_queueMutex;
    std::condition_variable m_queueCv;
    std::thread m_workerThread;
    std::atomic<u32> m_pendingLoads{0};
    bool m_stopWorker = false;
    
    std::string m_assetRoot = "assets/";
    AssetHandle<u32> m_nextHandle = 0;

    void workerLoop();
};

}
