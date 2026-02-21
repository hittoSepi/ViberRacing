#pragma once

#include "core/types.hpp"
#include <cstring>
#include <vector>
#include <type_traits>

namespace viber {

class BinaryWriter {
public:
    BinaryWriter() { m_buffer.reserve(1024); }
    explicit BinaryWriter(size_t initialSize) { m_buffer.reserve(initialSize); }
    
    void write(const void* data, size_t size) {
        const u8* bytes = static_cast<const u8*>(data);
        m_buffer.insert(m_buffer.end(), bytes, bytes + size);
    }
    
    template<typename T>
    void write(const T& value) {
        static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");
        write(&value, sizeof(T));
    }
    
    void writeString(const std::string& str) {
        write<u32>(static_cast<u32>(str.size()));
        write(str.data(), str.size());
    }
    
    void writeVec2(const vec2& v) { write(v); }
    void writeVec3(const vec3& v) { write(v); }
    void writeVec4(const vec4& v) { write(v); }
    void writeQuat(const quat& q) { write(q); }
    
    const std::vector<u8>& getData() const { return m_buffer; }
    std::vector<u8>&& takeData() { return std::move(m_buffer); }
    
    size_t getSize() const { return m_buffer.size(); }
    void clear() { m_buffer.clear(); }
    
private:
    std::vector<u8> m_buffer;
};

class BinaryReader {
public:
    BinaryReader() = default;
    explicit BinaryReader(const std::vector<u8>& data) : m_buffer(data) {}
    BinaryReader(const void* data, size_t size) 
        : m_buffer(static_cast<const u8*>(data), static_cast<const u8*>(data) + size) {}
    
    bool read(void* dest, size_t size) {
        if (m_position + size > m_buffer.size()) {
            return false;
        }
        std::memcpy(dest, m_buffer.data() + m_position, size);
        m_position += size;
        return true;
    }
    
    template<typename T>
    bool read(T& value) {
        static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");
        return read(&value, sizeof(T));
    }
    
    template<typename T>
    T read() {
        T value{};
        read(value);
        return value;
    }
    
    bool readString(std::string& str) {
        u32 length = 0;
        if (!read(length)) return false;
        if (m_position + length > m_buffer.size()) return false;
        str.assign(reinterpret_cast<const char*>(m_buffer.data() + m_position), length);
        m_position += length;
        return true;
    }
    
    std::string readString() {
        std::string str;
        readString(str);
        return str;
    }
    
    bool readVec2(vec2& v) { return read(v); }
    bool readVec3(vec3& v) { return read(v); }
    bool readVec4(vec4& v) { return read(v); }
    bool readQuat(quat& q) { return read(q); }
    
    size_t getPosition() const { return m_position; }
    size_t getSize() const { return m_buffer.size(); }
    size_t getRemaining() const { return m_buffer.size() - m_position; }
    bool isAtEnd() const { return m_position >= m_buffer.size(); }
    
    void setPosition(size_t pos) { m_position = pos; }
    void clear() { m_buffer.clear(); m_position = 0; }
    
private:
    std::vector<u8> m_buffer;
    size_t m_position = 0;
};

}
