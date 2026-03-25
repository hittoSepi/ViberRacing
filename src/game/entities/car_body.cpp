#include "car_body.hpp"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <fstream>
#include <random>

using json = nlohmann::json;

namespace viber {

// ---------------------------------------------------------------------------
// CarDefinition serialization
// ---------------------------------------------------------------------------

CarDefinition CarDefinition::makeDefault() {
    CarDefinition def;
    def.seed = 0;
    def.chassisVariant = "sedan_a";
    def.hoodVariant = "hood_flat";
    def.bumperFrontVariant = "bumper_front_standard";
    def.bumperRearVariant = "bumper_rear_standard";
    def.roofVariant = "roof_sedan";
    def.spoilerVariant = "";
    def.wheelVariants.fill("wheel_standard");
    def.primaryColor = vec3{0.8f, 0.15f, 0.1f};
    def.accentColor = vec3{0.1f, 0.1f, 0.1f};
    return def;
}

CarDefinition CarDefinition::fromJson(const std::string& path) {
    CarDefinition def = makeDefault();
    std::ifstream f(path);
    if (!f.is_open()) {
        spdlog::warn("CarDefinition: cannot open {}", path);
        return def;
    }
    try {
        json j = json::parse(f);
        def.seed = j.value("seed", 0u);
        def.chassisVariant = j.value("chassis", def.chassisVariant);
        def.hoodVariant = j.value("hood", def.hoodVariant);
        def.bumperFrontVariant = j.value("bumper_front", def.bumperFrontVariant);
        def.bumperRearVariant = j.value("bumper_rear", def.bumperRearVariant);
        def.roofVariant = j.value("roof", def.roofVariant);
        def.spoilerVariant = j.value("spoiler", def.spoilerVariant);
        if (j.contains("wheels")) {
            auto& wh = j["wheels"];
            for (int i = 0; i < 4 && i < static_cast<int>(wh.size()); ++i)
                def.wheelVariants[i] = wh[i].get<std::string>();
        }
        if (j.contains("primary_color")) {
            auto& c = j["primary_color"];
            def.primaryColor = vec3{c[0].get<float>(), c[1].get<float>(), c[2].get<float>()};
        }
        if (j.contains("accent_color")) {
            auto& c = j["accent_color"];
            def.accentColor = vec3{c[0].get<float>(), c[1].get<float>(), c[2].get<float>()};
        }
    } catch (const std::exception& e) {
        spdlog::error("CarDefinition::fromJson: {}", e.what());
    }
    return def;
}

void CarDefinition::toJson(const std::string& path) const {
    json j;
    j["seed"] = seed;
    j["chassis"] = chassisVariant;
    j["hood"] = hoodVariant;
    j["bumper_front"] = bumperFrontVariant;
    j["bumper_rear"] = bumperRearVariant;
    j["roof"] = roofVariant;
    j["spoiler"] = spoilerVariant;
    j["wheels"] = wheelVariants;
    j["primary_color"] = {primaryColor.x, primaryColor.y, primaryColor.z};
    j["accent_color"] = {accentColor.x, accentColor.y, accentColor.z};

    std::ofstream f(path);
    if (!f.is_open()) {
        spdlog::error("CarDefinition::toJson: cannot write {}", path);
        return;
    }
    f << j.dump(2);
}

// ---------------------------------------------------------------------------
// Proc gen
// ---------------------------------------------------------------------------

CarDefinition CarBody::generateFromSeed(u32 seed, const std::string& partsJsonPath) {
    CarDefinition def;
    def.seed = seed;

    std::mt19937 rng(seed);

    std::ifstream f(partsJsonPath);
    if (!f.is_open()) {
        spdlog::warn("CarBody::generateFromSeed: parts.json not found at {}, using defaults", partsJsonPath);
        return CarDefinition::makeDefault();
    }

    json parts;
    try {
        parts = json::parse(f);
    } catch (const std::exception& e) {
        spdlog::error("CarBody::generateFromSeed: parse error: {}", e.what());
        return CarDefinition::makeDefault();
    }

    auto pickRandom = [&](const json& arr) -> std::string {
        if (arr.empty()) return "";
        std::uniform_int_distribution<size_t> dist(0, arr.size() - 1);
        return arr[dist(rng)]["id"].get<std::string>();
    };

    def.chassisVariant = pickRandom(parts["chassis"]);

    // Pick parts compatible with chosen chassis
    auto pickCompatible = [&](const json& arr) -> std::string {
        std::vector<std::string> candidates;
        for (auto& item : arr) {
            if (!item.contains("compatible_chassis")) {
                candidates.push_back(item["id"].get<std::string>());
                continue;
            }
            for (auto& cc : item["compatible_chassis"]) {
                if (cc.get<std::string>() == def.chassisVariant) {
                    candidates.push_back(item["id"].get<std::string>());
                    break;
                }
            }
        }
        if (candidates.empty()) return "";
        std::uniform_int_distribution<size_t> dist(0, candidates.size() - 1);
        return candidates[dist(rng)];
    };

    def.hoodVariant = pickCompatible(parts["hood"]);
    def.bumperFrontVariant = pickCompatible(parts["bumper_front"]);
    def.bumperRearVariant = pickCompatible(parts["bumper_rear"]);
    def.roofVariant = pickCompatible(parts["roof"]);
    def.spoilerVariant = pickCompatible(parts["spoiler"]);

    std::string wheelVariant = pickRandom(parts["wheels"]);
    def.wheelVariants.fill(wheelVariant);

    // Generate colors from HSV
    auto hsvToRgb = [](float h, float s, float v) -> vec3 {
        float r, g, b;
        int i = static_cast<int>(h * 6.0f);
        float f = h * 6.0f - i;
        float p = v * (1.0f - s);
        float q = v * (1.0f - f * s);
        float t = v * (1.0f - (1.0f - f) * s);
        switch (i % 6) {
            case 0: r = v; g = t; b = p; break;
            case 1: r = q; g = v; b = p; break;
            case 2: r = p; g = v; b = t; break;
            case 3: r = p; g = q; b = v; break;
            case 4: r = t; g = p; b = v; break;
            default: r = v; g = p; b = q; break;
        }
        return {r, g, b};
    };

    std::uniform_real_distribution<float> hueDist(0.0f, 1.0f);
    std::uniform_real_distribution<float> satDist(0.6f, 1.0f);
    std::uniform_real_distribution<float> valDist(0.4f, 0.9f);
    def.primaryColor = hsvToRgb(hueDist(rng), satDist(rng), valDist(rng));
    def.accentColor = vec3{0.05f, 0.05f, 0.05f};  // dark accent

    return def;
}

// ---------------------------------------------------------------------------
// CarBody init / shutdown
// ---------------------------------------------------------------------------

bool CarBody::init(const CarDefinition& def, const std::string& partsJsonPath) {
    m_definition = def;
    return buildPartsFromDef(def, partsJsonPath);
}

void CarBody::shutdown() {
    for (auto& part : m_parts)
        part.mesh.destroy();
    m_parts.clear();
}

void CarBody::setDefinition(const CarDefinition& def) {
    shutdown();
    m_definition = def;
    buildPartsFromDef(def, "assets/cars/parts/parts.json");
}

// ---------------------------------------------------------------------------
// Build parts
// ---------------------------------------------------------------------------

static mat4 makeOffsetTransform(const json& partDef) {
    mat4 t{1.0f};
    if (partDef.contains("local_offset")) {
        auto& o = partDef["local_offset"];
        t = glm::translate(mat4{1.0f}, vec3{o[0].get<float>(), o[1].get<float>(), o[2].get<float>()});
    }
    return t;
}

bool CarBody::buildPartsFromDef(const CarDefinition& def, const std::string& partsJsonPath) {
    m_parts.clear();

    json parts;
    std::ifstream f(partsJsonPath);
    if (f.is_open()) {
        try { parts = json::parse(f); }
        catch (const std::exception& e) {
            spdlog::error("CarBody: parts.json parse error: {}", e.what());
        }
    } else {
        spdlog::warn("CarBody: parts.json not found at {}, using placeholder meshes", partsJsonPath);
    }

    // Helper: find part definition by id in a json array
    auto findPart = [&](const std::string& category, const std::string& id) -> const json* {
        if (!parts.contains(category)) return nullptr;
        for (auto& item : parts[category]) {
            if (item.value("id", "") == id) return &item;
        }
        return nullptr;
    };

    struct SlotSpec {
        CarPartSlot slot;
        std::string category;
        std::string id;
    };

    std::vector<SlotSpec> specs = {
        { CarPartSlot::Chassis,     "chassis",      def.chassisVariant },
        { CarPartSlot::HoodFront,   "hood",         def.hoodVariant },
        { CarPartSlot::BumperFront, "bumper_front", def.bumperFrontVariant },
        { CarPartSlot::BumperRear,  "bumper_rear",  def.bumperRearVariant },
        { CarPartSlot::Roof,        "roof",         def.roofVariant },
        { CarPartSlot::Spoiler,     "spoiler",      def.spoilerVariant },
    };

    for (auto& spec : specs) {
        if (spec.id.empty()) continue;

        CarPart part;
        part.slot = spec.slot;
        part.id = spec.id;

        const json* jPart = findPart(spec.category, spec.id);
        if (jPart) {
            part.meshPath = jPart->value("mesh", "");
            part.detachable = jPart->value("detachable", false);
            part.localTransform = makeOffsetTransform(*jPart);
        }

        // Use placeholder mesh (real mesh loading from file is deferred)
        part.mesh = buildPlaceholderMesh(spec.slot);
        if (!part.mesh.isValid()) {
            spdlog::warn("CarBody: placeholder mesh failed for slot {}", static_cast<int>(spec.slot));
        }

        m_parts.push_back(std::move(part));
    }

    // Wheels — slots WheelFL..WheelRR, transforms handled separately in render()
    const CarPartSlot wheelSlots[4] = {
        CarPartSlot::WheelFL, CarPartSlot::WheelFR,
        CarPartSlot::WheelRL, CarPartSlot::WheelRR
    };
    for (int i = 0; i < 4; ++i) {
        if (def.wheelVariants[i].empty()) continue;
        CarPart part;
        part.slot = wheelSlots[i];
        part.id = def.wheelVariants[i];
        part.detachable = true;
        part.mesh = buildPlaceholderMesh(CarPartSlot::WheelFL);
        m_parts.push_back(std::move(part));
    }

    spdlog::info("CarBody: built {} parts", m_parts.size());
    return true;
}

Mesh CarBody::buildPlaceholderMesh(CarPartSlot slot) const {
    switch (slot) {
        case CarPartSlot::Chassis:
            // Car body: longer and lower box
            return Mesh::createBox({1.8f, 0.5f, 4.0f});
        case CarPartSlot::WheelFL: case CarPartSlot::WheelFR:
        case CarPartSlot::WheelRL: case CarPartSlot::WheelRR:
            // Cylinder wheel (tire-like)
            return Mesh::createCylinder(0.35f, 0.25f, 16);
        case CarPartSlot::HoodFront:
            // Hood slopes down toward front
            return Mesh::createBox({1.6f, 0.1f, 1.2f});
        case CarPartSlot::BumperFront:
            return Mesh::createBox({1.7f, 0.4f, 0.4f});
        case CarPartSlot::BumperRear:
            return Mesh::createBox({1.7f, 0.4f, 0.4f});
        case CarPartSlot::Roof:
            return Mesh::createBox({1.4f, 0.1f, 1.8f});
        case CarPartSlot::Spoiler:
            // Wing-like shape
            return Mesh::createBox({1.6f, 0.15f, 0.4f});
        default:
            return Mesh::createBox({1.0f, 1.0f, 1.0f});
    }
}

// ---------------------------------------------------------------------------
// Update
// ---------------------------------------------------------------------------

void CarBody::update(float deltaTime) {
    for (auto& part : m_parts) {
        if (!part.detached) continue;
        // Simple gravity for detached parts
        part.detachVelocity.y -= 9.81f * deltaTime;
        mat4 t = glm::translate(mat4{1.0f}, vec3{
            part.detachVelocity.x * deltaTime,
            part.detachVelocity.y * deltaTime,
            part.detachVelocity.z * deltaTime
        });
        part.detachedTransform = t * part.detachedTransform;
    }
}

// ---------------------------------------------------------------------------
// Render
// ---------------------------------------------------------------------------

void CarBody::render(bgfx::ViewId viewId, bgfx::ProgramHandle program,
                     const mat4& chassisTransform,
                     const std::array<mat4, 4>& wheelTransforms) const {
    if (!bgfx::isValid(program)) return;

    const u64 state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z |
                      BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW | BGFX_STATE_MSAA;

    for (const auto& part : m_parts) {
        if (!part.mesh.isValid()) continue;

        // Wheels use wheel transforms from physics
        if (part.slot >= CarPartSlot::WheelFL && part.slot <= CarPartSlot::WheelRR) {
            int idx = static_cast<int>(part.slot) - static_cast<int>(CarPartSlot::WheelFL);
            mat4 worldTransform = wheelTransforms[idx];

            // Scale wheel down visually if detached / heavily damaged
            if (part.detached) {
                worldTransform = part.detachedTransform;
            } else if (part.damage > 0.7f) {
                float s = 1.0f - (part.damage - 0.7f) * 0.5f;
                worldTransform = glm::scale(worldTransform, vec3{s, s, s});
            }

            part.mesh.submit(viewId, program, worldTransform, state);
            continue;
        }

        // Body parts
        mat4 worldTransform = part.detached
            ? part.detachedTransform
            : chassisTransform * part.localTransform;

        // Visually deform heavily damaged parts (simple scale/tilt)
        if (!part.detached && part.damage > 0.3f) {
            float lean = (part.damage - 0.3f) * 0.4f;
            mat4 deform = glm::rotate(mat4{1.0f}, lean, vec3{1.0f, 0.0f, 0.0f});
            worldTransform = worldTransform * deform;
        }

        part.mesh.submit(viewId, program, worldTransform, state);
    }
}

// ---------------------------------------------------------------------------
// Wheel part access
// ---------------------------------------------------------------------------

CarPart* CarBody::getWheelPart(int wheelIndex) {
    const CarPartSlot target = static_cast<CarPartSlot>(
        static_cast<int>(CarPartSlot::WheelFL) + wheelIndex);
    for (auto& part : m_parts) {
        if (part.slot == target) return &part;
    }
    return nullptr;
}

}
