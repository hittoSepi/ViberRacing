#include "manager.hpp"
#include "mesh_loader.hpp"
#include "texture_loader.hpp"
#include <spdlog/spdlog.h>

namespace viber {

AssetManager::AssetManager() = default;

AssetManager::~AssetManager() {
    shutdown();
}

std::string AssetManager::resolvePath(const std::string& relativePath) const {
    if (!relativePath.empty() && relativePath[0] == '/') {
        return relativePath;
    }
    return m_assetRoot + relativePath;
}

AssetHandle<Mesh> AssetManager::loadMesh(const std::string& path) {
    auto it = m_meshPathMap.find(path);
    if (it != m_meshPathMap.end()) {
        return it->second;
    }
    
    AssetHandle<Mesh> handle = nextHandle();
    
    AssetEntry<Mesh> entry;
    entry.info.path = path;
    entry.info.name = path.substr(path.find_last_of("/\\") + 1);
    entry.info.loading = true;
    
    m_meshes[handle] = std::move(entry);
    m_meshPathMap[path] = handle;
    
    std::lock_guard<std::mutex> lock(m_queueMutex);
    m_loadQueue.push([this, handle, path]() {
        auto& entry = m_meshes[handle];
        
        std::vector<Vertex> vertices;
        std::vector<u32> indices;
        
        if (MeshLoader::load(resolvePath(path), vertices, indices)) {
            entry.asset = std::make_unique<Mesh>();
            if (entry.asset->create(vertices, indices)) {
                entry.info.loaded = true;
                entry.info.failed = false;
                spdlog::info("Loaded mesh: {}", path);
            } else {
                entry.info.failed = true;
                spdlog::error("Failed to create mesh: {}", path);
            }
        } else {
            entry.info.failed = true;
            spdlog::error("Failed to load mesh: {}", path);
        }
        
        entry.info.loading = false;
    });
    
    return handle;
}

AssetHandle<Texture> AssetManager::loadTexture(const std::string& path) {
    auto it = m_texturePathMap.find(path);
    if (it != m_texturePathMap.end()) {
        return it->second;
    }
    
    AssetHandle<Texture> handle = nextHandle();
    
    AssetEntry<Texture> entry;
    entry.info.path = path;
    entry.info.name = path.substr(path.find_last_of("/\\") + 1);
    entry.info.loading = true;
    
    m_textures[handle] = std::move(entry);
    m_texturePathMap[path] = handle;
    
    std::lock_guard<std::mutex> lock(m_queueMutex);
    m_loadQueue.push([this, handle, path]() {
        auto& entry = m_textures[handle];
        
        entry.asset = std::make_unique<Texture>();
        if (entry.asset->loadFromFile(resolvePath(path))) {
            entry.info.loaded = true;
            entry.info.failed = false;
        } else {
            entry.info.failed = true;
        }
        
        entry.info.loading = false;
    });
    
    return handle;
}

AssetHandle<Shader> AssetManager::loadShader(const std::string& name,
                                              const std::string& vsPath,
                                              const std::string& fsPath) {
    AssetHandle<Shader> handle = nextHandle();
    
    AssetEntry<Shader> entry;
    entry.info.path = vsPath + ";" + fsPath;
    entry.info.name = name;
    
    entry.asset = std::make_unique<Shader>();
    if (entry.asset->loadFromFiles(resolvePath(vsPath), resolvePath(fsPath))) {
        entry.info.loaded = true;
        entry.info.failed = false;
    } else {
        entry.info.failed = true;
    }
    
    m_shaders[handle] = std::move(entry);
    
    return handle;
}

Mesh* AssetManager::getMesh(AssetHandle<Mesh> handle) {
    auto it = m_meshes.find(handle);
    return it != m_meshes.end() ? it->second.asset.get() : nullptr;
}

Texture* AssetManager::getTexture(AssetHandle<Texture> handle) {
    auto it = m_textures.find(handle);
    return it != m_textures.end() ? it->second.asset.get() : nullptr;
}

Shader* AssetManager::getShader(AssetHandle<Shader> handle) {
    auto it = m_shaders.find(handle);
    return it != m_shaders.end() ? it->second.asset.get() : nullptr;
}

void AssetManager::releaseMesh(AssetHandle<Mesh> handle) {
    auto it = m_meshes.find(handle);
    if (it != m_meshes.end()) {
        m_meshPathMap.erase(it->second.info.path);
        m_meshes.erase(it);
    }
}

void AssetManager::releaseTexture(AssetHandle<Texture> handle) {
    auto it = m_textures.find(handle);
    if (it != m_textures.end()) {
        m_texturePathMap.erase(it->second.info.path);
        m_textures.erase(it);
    }
}

void AssetManager::releaseShader(AssetHandle<Shader> handle) {
    m_shaders.erase(handle);
}

bool AssetManager::isLoaded(AssetHandle<Mesh> handle) const {
    auto it = m_meshes.find(handle);
    return it != m_meshes.end() && it->second.info.loaded;
}

bool AssetManager::isLoaded(AssetHandle<Texture> handle) const {
    auto it = m_textures.find(handle);
    return it != m_textures.end() && it->second.info.loaded;
}

void AssetManager::processQueue() {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    while (!m_loadQueue.empty()) {
        auto task = std::move(m_loadQueue.front());
        m_loadQueue.pop();
        task();
    }
}

void AssetManager::waitForAll() {
    while (true) {
        bool allDone = true;
        for (const auto& [handle, entry] : m_meshes) {
            if (entry.info.loading) {
                allDone = false;
                break;
            }
        }
        if (allDone) {
            for (const auto& [handle, entry] : m_textures) {
                if (entry.info.loading) {
                    allDone = false;
                    break;
                }
            }
        }
        if (allDone) break;
        
        processQueue();
    }
}

void AssetManager::shutdown() {
    m_meshes.clear();
    m_textures.clear();
    m_shaders.clear();
    m_meshPathMap.clear();
    m_texturePathMap.clear();
    
    std::lock_guard<std::mutex> lock(m_queueMutex);
    while (!m_loadQueue.empty()) {
        m_loadQueue.pop();
    }
}

}
