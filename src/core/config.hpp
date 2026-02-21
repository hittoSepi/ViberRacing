#pragma once

#include "types.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>

namespace viber {

class ConfigValue;

class Config {
public:
    Config() = default;
    
    bool loadFromFile(const std::string& path);
    bool saveToFile(const std::string& path);
    
    bool has(const std::string& key) const;
    
    int getInt(const std::string& key, int defaultValue = 0) const;
    float getFloat(const std::string& key, float defaultValue = 0.0f) const;
    bool getBool(const std::string& key, bool defaultValue = false) const;
    std::string getString(const std::string& key, const std::string& defaultValue = "") const;
    
    vec2 getVec2(const std::string& key, vec2 defaultValue = vec2(0.0f)) const;
    vec3 getVec3(const std::string& key, vec3 defaultValue = vec3(0.0f)) const;
    vec4 getVec4(const std::string& key, vec4 defaultValue = vec4(0.0f)) const;
    
    void set(const std::string& key, int value);
    void set(const std::string& key, float value);
    void set(const std::string& key, bool value);
    void set(const std::string& key, const std::string& value);
    void set(const std::string& key, const char* value);
    void set(const std::string& key, const vec2& value);
    void set(const std::string& key, const vec3& value);
    void set(const std::string& key, const vec4& value);
    
    void remove(const std::string& key);
    void clear();
    
    const auto& getAll() const { return m_values; }
    
private:
    std::unordered_map<std::string, std::string> m_values;
    
    std::vector<std::string> splitKey(const std::string& key) const;
};

}
