#include "profiler.hpp"
#include <spdlog/spdlog.h>
#include <algorithm>

namespace viber {

void Profiler::beginFrame() {
    m_frameStart = std::chrono::high_resolution_clock::now();
    m_frameCount++;
}

void Profiler::endFrame() {
    // Frame timing tracked internally
}

void Profiler::pushScope(const std::string& name) {
    m_scopeStack.push_back({name, std::chrono::high_resolution_clock::now()});
}

void Profiler::popScope(i64 durationNanos) {
    if (m_scopeStack.empty()) return;
    
    const auto& scope = m_scopeStack.back();
    auto& entry = m_entries[scope.name];
    entry.name = scope.name;
    entry.totalNanos += durationNanos;
    entry.minNanos = std::min(entry.minNanos, durationNanos);
    entry.maxNanos = std::max(entry.maxNanos, durationNanos);
    entry.callCount++;
    
    m_scopeStack.pop_back();
}

const Profiler::Entry* Profiler::getEntry(const std::string& name) const {
    auto it = m_entries.find(name);
    return it != m_entries.end() ? &it->second : nullptr;
}

void Profiler::reset() {
    m_entries.clear();
    m_scopeStack.clear();
    m_frameCount = 0;
}

void Profiler::dumpToFile(const std::string& path) const {
    std::ofstream file(path);
    if (!file.is_open()) {
        spdlog::error("Failed to open profiler output file: {}", path);
        return;
    }
    
    file << "Name,Calls,Total(ms),Avg(ms),Min(ms),Max(ms)\n";
    
    std::vector<std::pair<std::string, Entry>> sorted(m_entries.begin(), m_entries.end());
    std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) {
        return a.second.totalNanos > b.second.totalNanos;
    });
    
    for (const auto& [name, entry] : sorted) {
        const double totalMs = entry.totalNanos / 1000000.0;
        const double avgMs = totalMs / entry.callCount;
        const double minMs = entry.minNanos / 1000000.0;
        const double maxMs = entry.maxNanos / 1000000.0;
        
        file << name << "," << entry.callCount << ","
             << totalMs << "," << avgMs << ","
             << minMs << "," << maxMs << "\n";
    }
    
    spdlog::info("Profiler data written to: {}", path);
}

void Profiler::dumpToConsole() const {
    spdlog::info("=== Profiler Results ===");
    
    std::vector<std::pair<std::string, Entry>> sorted(m_entries.begin(), m_entries.end());
    std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) {
        return a.second.totalNanos > b.second.totalNanos;
    });
    
    for (const auto& [name, entry] : sorted) {
        const double totalMs = entry.totalNanos / 1000000.0;
        const double avgMs = totalMs / entry.callCount;
        
        spdlog::info("  {}: {} calls, {:.2f}ms total, {:.3f}ms avg",
            name, entry.callCount, totalMs, avgMs);
    }
}

ProfilerScope::ProfilerScope(const std::string& name)
    : m_name(name)
    , m_start(std::chrono::high_resolution_clock::now())
{
    Profiler::get().pushScope(name);
}

ProfilerScope::~ProfilerScope() {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - m_start).count();
    Profiler::get().popScope(duration);
}

}
