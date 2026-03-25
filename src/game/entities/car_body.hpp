#pragma once

#include "core/types.hpp"
#include "renderer/mesh.hpp"
#include <bgfx/bgfx.h>
#include <array>
#include <optional>
#include <string>
#include <vector>

namespace viber {

enum class CarPartSlot {
    Chassis,
    HoodFront,
    BumperFront,
    BumperRear,
    Roof,
    Spoiler,
    WheelFL,
    WheelFR,
    WheelRL,
    WheelRR,
    COUNT
};

struct CarPart {
    CarPartSlot slot;
    std::string id;
    std::string meshPath;
    mat4 localTransform{1.0f};  // offset from chassis origin
    float damage = 0.0f;        // 0 = pristine, 1 = destroyed
    bool detached = false;
    bool detachable = false;
    vec3 detachVelocity{0.0f};  // world-space velocity when detached
    mat4 detachedTransform{1.0f};

    // Mesh owned by CarBody (not AssetManager) since parts may be procedural
    Mesh mesh;
};

struct CarDefinition {
    u32 seed = 0;
    std::string chassisVariant;
    std::string hoodVariant;
    std::string bumperFrontVariant;
    std::string bumperRearVariant;
    std::string roofVariant;
    std::string spoilerVariant;  // empty = no spoiler
    std::array<std::string, 4> wheelVariants;
    vec3 primaryColor{0.8f, 0.1f, 0.1f};
    vec3 accentColor{0.1f, 0.1f, 0.1f};

    // Serialization helpers
    static CarDefinition fromJson(const std::string& path);
    void toJson(const std::string& path) const;
    static CarDefinition makeDefault();
};

class CarBody {
public:
    CarBody() = default;
    ~CarBody() = default;

    CarBody(const CarBody&) = delete;
    CarBody& operator=(const CarBody&) = delete;

    // Build parts from a definition. partsJsonPath = assets/cars/parts/parts.json
    bool init(const CarDefinition& def, const std::string& partsJsonPath = "assets/cars/parts/parts.json");
    void shutdown();

    // Generate a random definition from seed
    static CarDefinition generateFromSeed(u32 seed, const std::string& partsJsonPath = "assets/cars/parts/parts.json");

    // Called each frame: animate detached parts falling/sliding
    void update(float deltaTime);

    // Render all parts. chassisTransform = world matrix of chassis,
    // wheelTransforms[4] = world matrices from Vehicle::getWheelTransform()
    void render(bgfx::ViewId viewId, bgfx::ProgramHandle program,
                const mat4& chassisTransform,
                const std::array<mat4, 4>& wheelTransforms) const;

    // Access individual parts (for DamageModel)
    std::vector<CarPart>& getParts() { return m_parts; }
    const std::vector<CarPart>& getParts() const { return m_parts; }
    CarPart* getWheelPart(int wheelIndex);  // 0=FL,1=FR,2=RL,3=RR

    const CarDefinition& getDefinition() const { return m_definition; }
    void setDefinition(const CarDefinition& def);

    vec3 getPrimaryColor() const { return m_definition.primaryColor; }
    vec3 getAccentColor() const { return m_definition.accentColor; }

private:
    bool buildPartsFromDef(const CarDefinition& def, const std::string& partsJsonPath);
    Mesh buildPlaceholderMesh(CarPartSlot slot) const;

    std::vector<CarPart> m_parts;
    CarDefinition m_definition;
};

}
