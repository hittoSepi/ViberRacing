#include <catch2/catch_test_macros.hpp>
#include "utils/serialization.hpp"

TEST_CASE("BinaryWriter", "[serialization]") {
    viber::BinaryWriter writer;
    
    SECTION("Write primitive types") {
        writer.write<u32>(42);
        writer.write<float>(3.14f);
        writer.write<bool>(true);
        
        REQUIRE(writer.getSize() == sizeof(u32) + sizeof(float) + sizeof(bool));
    }
    
    SECTION("Write string") {
        writer.writeString("hello");
        
        const auto& data = writer.getData();
        REQUIRE(data.size() == sizeof(u32) + 5);
    }
    
    SECTION("Write vectors") {
        writer.writeVec2(viber::vec2(1.0f, 2.0f));
        writer.writeVec3(viber::vec3(1.0f, 2.0f, 3.0f));
        writer.writeVec4(viber::vec4(1.0f, 2.0f, 3.0f, 4.0f));
        
        REQUIRE(writer.getSize() == sizeof(float) * 9);
    }
    
    SECTION("Clear") {
        writer.write<u32>(42);
        REQUIRE(writer.getSize() > 0);
        
        writer.clear();
        REQUIRE(writer.getSize() == 0);
    }
}

TEST_CASE("BinaryReader", "[serialization]") {
    viber::BinaryWriter writer;
    writer.write<u32>(42);
    writer.write<float>(3.14f);
    writer.writeString("test");
    
    auto data = writer.takeData();
    
    SECTION("Read primitive types") {
        viber::BinaryReader reader(data);
        
        u32 intValue = reader.read<u32>();
        REQUIRE(intValue == 42);
        
        float floatValue = reader.read<float>();
        REQUIRE(floatValue == Approx(3.14f));
    }
    
    SECTION("Read string") {
        viber::BinaryReader reader(data);
        reader.read<u32>();
        reader.read<float>();
        
        std::string str;
        REQUIRE(reader.readString(str));
        REQUIRE(str == "test");
    }
    
    SECTION("Position tracking") {
        viber::BinaryReader reader(data);
        
        REQUIRE(reader.getPosition() == 0);
        reader.read<u32>();
        REQUIRE(reader.getPosition() == sizeof(u32));
        REQUIRE(reader.getRemaining() == data.size() - sizeof(u32));
    }
    
    SECTION("End of data") {
        viber::BinaryReader reader(data);
        
        while (!reader.isAtEnd()) {
            reader.read<u8>();
        }
        
        u32 value;
        REQUIRE_FALSE(reader.read(value));
    }
}
