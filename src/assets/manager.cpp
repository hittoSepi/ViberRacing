#include "manager.hpp"
#include "mesh_loader.hpp"
#include "texture_loader.hpp"
#include <spdlog/spdlog.h>
#include <chrono>
#include <fstream>

namespace viber {

namespace {

bool readBinaryFile(const std::string& path, std::vector<u8>& outData) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return false;
    }

    outData.resize(static_cast<size_t>(file.tellg()));
    file.seekg(0);
    file.read(reinterpret_cast<char*>(outData.data()), static_cast<std::streamsize>(outData.size()));
    return true;
}

}

AssetManager::AssetManager() {
    m_workerThread = std::thread(&AssetManager::workerLoop, this);
}

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
    
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_requestQueue.push({RequestType::Mesh, handle, path});
    }
    ++m_pendingLoads;
    m_queueCv.notify_one();
    
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
    
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_requestQueue.push({RequestType::Texture, handle, path});
    }
    ++m_pendingLoads;
    m_queueCv.notify_one();
    
    return handle;
}

AssetHandle<Shader> AssetManager::loadShader(const std::string& name,
                                              const std::string& vsPath,
                                              const std::string& fsPath) {
    AssetHandle<Shader> handle = nextHandle();
    
    AssetEntry<Shader> entry;
    entry.info.path = vsPath + ";" + fsPath;
    entry.info.name = name;
    
    entry.info.loading = true;

    m_shaders[handle] = std::move(entry);

    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_requestQueue.push({RequestType::Shader, handle, name + ";" + vsPath + ";" + fsPath});
    }
    ++m_pendingLoads;
    m_queueCv.notify_one();
    
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

bool AssetManager::isMeshLoaded(AssetHandle<Mesh> handle) const {
    auto it = m_meshes.find(handle);
    return it != m_meshes.end() && it->second.info.loaded;
}

bool AssetManager::isTextureLoaded(AssetHandle<Texture> handle) const {
    auto it = m_textures.find(handle);
    return it != m_textures.end() && it->second.info.loaded;
}

void AssetManager::processQueue() {
    std::queue<CompletedRequest> completed;
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        std::swap(completed, m_completedQueue);
    }

    while (!completed.empty()) {
        CompletedRequest request = std::move(completed.front());
        completed.pop();

        if (request.type == RequestType::Mesh) {
            auto it = m_meshes.find(request.handle);
            if (it != m_meshes.end()) {
                auto& entry = it->second;
                if (request.success) {
                    const auto& data = std::get<MeshLoadResult>(request.payload);
                    entry.asset = std::make_unique<Mesh>();
                    if (entry.asset->create(data.vertices, data.indices)) {
                        entry.info.loaded = true;
                        entry.info.failed = false;
                        spdlog::info("Loaded mesh: {}", request.path);
                    } else {
                        entry.info.failed = true;
                        spdlog::error("Failed to create mesh: {}", request.path);
                    }
                } else {
                    entry.info.failed = true;
                    spdlog::error("Failed to load mesh: {} ({})", request.path, request.error);
                }
                entry.info.loading = false;
            }
        } else if (request.type == RequestType::Texture) {
            auto it = m_textures.find(request.handle);
            if (it != m_textures.end()) {
                auto& entry = it->second;
                if (request.success) {
                    const auto& data = std::get<TextureLoadResult>(request.payload);
                    entry.asset = std::make_unique<Texture>();
                    if (entry.asset->create(data.desc, data.data.data(), data.data.size())) {
                        entry.info.loaded = true;
                        entry.info.failed = false;
                        spdlog::info("Loaded texture: {}", request.path);
                    } else {
                        entry.info.failed = true;
                        spdlog::error("Failed to create texture: {}", request.path);
                    }
                } else {
                    entry.info.failed = true;
                    spdlog::error("Failed to load texture: {} ({})", request.path, request.error);
                }
                entry.info.loading = false;
            }
        } else if (request.type == RequestType::Shader) {
            auto it = m_shaders.find(request.handle);
            if (it != m_shaders.end()) {
                auto& entry = it->second;
                if (request.success) {
                    const auto& data = std::get<ShaderLoadResult>(request.payload);
                    entry.asset = std::make_unique<Shader>();
                    if (entry.asset->loadFromBinary(data.vsData, data.fsData)) {
                        entry.info.loaded = true;
                        entry.info.failed = false;
                        spdlog::info("Loaded shader: {} + {}", data.vsPath, data.fsPath);
                    } else {
                        entry.info.failed = true;
                        spdlog::error("Failed to create shader program: {} + {}", data.vsPath, data.fsPath);
                    }
                } else {
                    entry.info.failed = true;
                    spdlog::error("Failed to load shader: {} ({})", request.path, request.error);
                }
                entry.info.loading = false;
            }
        }

        if (m_pendingLoads > 0) {
            --m_pendingLoads;
        }
    }
}

void AssetManager::waitForAll() {
    while (m_pendingLoads > 0) {
        processQueue();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void AssetManager::shutdown() {
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_stopWorker = true;
        while (!m_requestQueue.empty()) {
            m_requestQueue.pop();
        }
        while (!m_completedQueue.empty()) {
            m_completedQueue.pop();
        }
    }
    m_queueCv.notify_all();

    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }

    m_pendingLoads = 0;
    m_meshes.clear();
    m_textures.clear();
    m_shaders.clear();
    m_meshPathMap.clear();
    m_texturePathMap.clear();
}

void AssetManager::workerLoop() {
    while (true) {
        LoadRequest request;
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_queueCv.wait(lock, [this]() {
                return m_stopWorker || !m_requestQueue.empty();
            });

            if (m_stopWorker && m_requestQueue.empty()) {
                return;
            }

            request = std::move(m_requestQueue.front());
            m_requestQueue.pop();
        }

        CompletedRequest completed;
        completed.type = request.type;
        completed.handle = request.handle;
        completed.path = request.path;

        if (request.type == RequestType::Mesh) {
            MeshLoadResult result;
            if (MeshLoader::load(resolvePath(request.path), result.vertices, result.indices)) {
                completed.success = true;
                completed.payload = std::move(result);
            } else {
                completed.success = false;
                completed.error = "decode/import failed";
            }
        } else if (request.type == RequestType::Texture) {
            TextureLoadResult result;
            if (TextureLoader::loadFromFile(resolvePath(request.path), result.desc, result.data)) {
                result.desc.generateMips = true;
                result.desc.sRGB = true;
                completed.success = true;
                completed.payload = std::move(result);
            } else {
                completed.success = false;
                completed.error = "decode failed";
            }
        } else if (request.type == RequestType::Shader) {
            ShaderLoadResult result;
            const size_t firstSep = request.path.find(';');
            const size_t secondSep = request.path.find(';', firstSep == std::string::npos ? firstSep : firstSep + 1);
            if (firstSep == std::string::npos || secondSep == std::string::npos) {
                completed.success = false;
                completed.error = "invalid shader request";
            } else {
                result.name = request.path.substr(0, firstSep);
                result.vsPath = request.path.substr(firstSep + 1, secondSep - firstSep - 1);
                result.fsPath = request.path.substr(secondSep + 1);
                if (readBinaryFile(resolvePath(result.vsPath), result.vsData) &&
                    readBinaryFile(resolvePath(result.fsPath), result.fsData)) {
                    completed.success = true;
                    completed.payload = std::move(result);
                } else {
                    completed.success = false;
                    completed.error = "binary read failed";
                }
            }
        }

        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_completedQueue.push(std::move(completed));
        }
    }
}

}
