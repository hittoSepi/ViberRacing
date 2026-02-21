#include "config.hpp"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <sstream>

namespace viber {

bool Config::loadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        spdlog::warn("Config file not found: {}, using defaults", path);
        return false;
    }
    
    try {
        nlohmann::json json;
        file >> json;
        
        std::function<void(const nlohmann::json&, const std::string&)> flatten;
        flatten = [&](const nlohmann::json& j, const std::string& prefix) {
            for (auto it = j.begin(); it != j.end(); ++it) {
                std::string key = prefix.empty() ? it.key() : prefix + "." + it.key();
                
                if (it.value().is_object()) {
                    flatten(it.value(), key);
                } else if (it.value().is_string()) {
                    m_values[key] = it.value().get<std::string>();
                } else if (it.value().is_number_integer()) {
                    m_values[key] = std::to_string(it.value().get<int>());
                } else if (it.value().is_number_float()) {
                    m_values[key] = std::to_string(it.value().get<float>());
                } else if (it.value().is_boolean()) {
                    m_values[key] = it.value().get<bool>() ? "true" : "false";
                }
            }
        };
        
        flatten(json, "");
        
        spdlog::info("Loaded config from: {} ({} entries)", path, m_values.size());
        return true;
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to parse config file: {}", e.what());
        return false;
    }
}

bool Config::saveToFile(const std::string& path) {
    nlohmann::json json;
    
    for (const auto& [key, value] : m_values) {
        std::vector<std::string> parts;
        std::stringstream ss(key);
        std::string part;
        while (std::getline(ss, part, '.')) {
            parts.push_back(part);
        }
        
        nlohmann::json* current = &json;
        for (size_t i = 0; i < parts.size() - 1; ++i) {
            if (!current->contains(parts[i])) {
                (*current)[parts[i]] = nlohmann::json::object();
            }
            current = &(*current)[parts[i]];
        }
        
        (*current)[parts.back()] = value;
    }
    
    std::ofstream file(path);
    if (!file.is_open()) {
        spdlog::error("Failed to open config file for writing: {}", path);
        return false;
    }
    
    file << json.dump(4);
    spdlog::info("Saved config to: {}", path);
    return true;
}

bool Config::has(const std::string& key) const {
    return m_values.find(key) != m_values.end();
}

int Config::getInt(const std::string& key, int defaultValue) const {
    auto it = m_values.find(key);
    if (it != m_values.end()) {
        try {
            return std::stoi(it->second);
        } catch (...) {
            return defaultValue;
        }
    }
    return defaultValue;
}

float Config::getFloat(const std::string& key, float defaultValue) const {
    auto it = m_values.find(key);
    if (it != m_values.end()) {
        try {
            return std::stof(it->second);
        } catch (...) {
            return defaultValue;
        }
    }
    return defaultValue;
}

bool Config::getBool(const std::string& key, bool defaultValue) const {
    auto it = m_values.find(key);
    if (it != m_values.end()) {
        const auto& v = it->second;
        if (v == "true" || v == "1" || v == "yes") return true;
        if (v == "false" || v == "0" || v == "no") return false;
        return defaultValue;
    }
    return defaultValue;
}

std::string Config::getString(const std::string& key, const std::string& defaultValue) const {
    auto it = m_values.find(key);
    return it != m_values.end() ? it->second : defaultValue;
}

vec2 Config::getVec2(const std::string& key, vec2 defaultValue) const {
    return vec2(
        getFloat(key + ".x", defaultValue.x),
        getFloat(key + ".y", defaultValue.y)
    );
}

vec3 Config::getVec3(const std::string& key, vec3 defaultValue) const {
    return vec3(
        getFloat(key + ".x", defaultValue.x),
        getFloat(key + ".y", defaultValue.y),
        getFloat(key + ".z", defaultValue.z)
    );
}

vec4 Config::getVec4(const std::string& key, vec4 defaultValue) const {
    return vec4(
        getFloat(key + ".x", defaultValue.x),
        getFloat(key + ".y", defaultValue.y),
        getFloat(key + ".z", defaultValue.z),
        getFloat(key + ".w", defaultValue.w)
    );
}

void Config::set(const std::string& key, int value) {
    m_values[key] = std::to_string(value);
}

void Config::set(const std::string& key, float value) {
    m_values[key] = std::to_string(value);
}

void Config::set(const std::string& key, bool value) {
    m_values[key] = value ? "true" : "false";
}

void Config::set(const std::string& key, const std::string& value) {
    m_values[key] = value;
}

void Config::set(const std::string& key, const char* value) {
    m_values[key] = value;
}

void Config::set(const std::string& key, const vec2& value) {
    set(key + ".x", value.x);
    set(key + ".y", value.y);
}

void Config::set(const std::string& key, const vec3& value) {
    set(key + ".x", value.x);
    set(key + ".y", value.y);
    set(key + ".z", value.z);
}

void Config::set(const std::string& key, const vec4& value) {
    set(key + ".x", value.x);
    set(key + ".y", value.y);
    set(key + ".z", value.z);
    set(key + ".w", value.w);
}

void Config::remove(const std::string& key) {
    m_values.erase(key);
}

void Config::clear() {
    m_values.clear();
}

std::vector<std::string> Config::splitKey(const std::string& key) const {
    std::vector<std::string> parts;
    std::stringstream ss(key);
    std::string part;
    while (std::getline(ss, part, '.')) {
        parts.push_back(part);
    }
    return parts;
}

}
