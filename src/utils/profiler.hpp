#pragma once

#include "core/types.hpp"
#include <chrono>
#include <string>
#include <unordered_map>
#include <fstream>

#ifdef VIBER_PROFILING_ENABLED
#define VR_PROFILE_SCOPE(name) viber::ProfilerScope _profileScope(name)
#define VR_PROFILE_FUNCTION() VR_PROFILE_SCOPE(__FUNCTION__)
#else
#define VR_PROFILE_SCOPE(name)
#define VR_PROFILE_FUNCTION()
#endif

namespace viber {

class Profiler {
public:
    struct Entry {
        std::string name;
        i64 totalNanos = 0;
        i64 minNanos = INT64_MAX;
        i64 maxNanos = 0;
        i32 callCount = 0;
    };
    
    static Profiler& get() {
        static Profiler instance;
        return instance;
    }
    
    void beginFrame();
    void endFrame();
    
    void pushScope(const std::string& name);
    void popScope(i64 durationNanos);
    
    const Entry* getEntry(const std::string& name) const;
    const std::unordered_map<std::string, Entry>& getAllEntries() const { return m_entries; }
    
    void reset();
    void dumpToFile(const std::string& path) const;
    void dumpToConsole() const;
    
    struct ScopeData {
        std::string name;
        std::chrono::high_resolution_clock::time_point start;
    };
    
private:
    Profiler() = default;
    
    std::unordered_map<std::string, Entry> m_entries;
    std::vector<ScopeData> m_scopeStack;
    std::chrono::high_resolution_clock::time_point m_frameStart;
    i32 m_frameCount = 0;
};

class ProfilerScope {
public:
    explicit ProfilerScope(const std::string& name);
    ~ProfilerScope();
    
private:
    std::string m_name;
    std::chrono::high_resolution_clock::time_point m_start;
};

}
