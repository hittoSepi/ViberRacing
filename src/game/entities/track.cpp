#include "track.hpp"
#include "renderer/renderer.hpp"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <filesystem>
#include <fstream>
#include <random>

namespace viber {

using json = nlohmann::json;

namespace {

json vecToJson(const vec3& value) {
    return json::array({value.x, value.y, value.z});
}

vec3 vecFromJson(const json& value) {
    return vec3{
        value.at(0).get<float>(),
        value.at(1).get<float>(),
        value.at(2).get<float>()
    };
}

std::string makeHoleId(size_t index) {
    return "hole_" + std::to_string(index + 1);
}

std::string makeTunnelId(size_t index) {
    return "tunnel_" + std::to_string(index + 1);
}

} // namespace

Track::Track() = default;

Track::~Track() = default;

void Track::generateTestTrack() {
    m_spline.clear();
    
    m_spline.addControlPoint(vec3(0.0f, 0.0f, 0.0f));
    m_spline.addControlPoint(vec3(50.0f, 0.0f, 0.0f));
    m_spline.addControlPoint(vec3(100.0f, 5.0f, 30.0f));
    m_spline.addControlPoint(vec3(100.0f, 0.0f, 80.0f));
    m_spline.addControlPoint(vec3(50.0f, -3.0f, 100.0f));
    m_spline.addControlPoint(vec3(0.0f, 0.0f, 80.0f));
    m_spline.addControlPoint(vec3(-30.0f, 2.0f, 40.0f));
    m_spline.addControlPoint(vec3(-20.0f, 0.0f, 0.0f));
    
    m_spline.closeLoop();
    
    generateFromSpline(m_spline);
    
    spdlog::info("Generated test track with {} control points, length: {:.1f}m",
        m_spline.getNumControlPoints(), m_spline.getLength());
}

void Track::generateFromSpline(const Spline& spline) {
    m_spline = spline;
    m_segments.clear();
    
    float totalLength = spline.getLength();
    if (m_spline.getNumControlPoints() < 2 || totalLength <= 0.001f) {
        generateMesh();
        return;
    }
    
    auto frames = m_spline.sampleFrames(m_segmentCount);
    
    for (const auto& frame : frames) {
        TrackSegment segment;
        segment.frame = frame;
        segment.width = m_roadWidth;
        m_segments.push_back(segment);
    }
    
    generateMesh();
}

void Track::render(Renderer&) {
}

float Track::getLength() const {
    return m_spline.getLength();
}

vec3 Track::getPositionAt(float t) const {
    return m_spline.evaluate(t);
}

vec3 Track::getDirectionAt(float t) const {
    return m_spline.getTangent(t);
}

void Track::addHole(const TrackHole& hole) {
    TrackHole stored = hole;
    if (stored.id.empty()) {
        stored.id = makeHoleId(m_holes.size());
    }
    m_holes.push_back(stored);
}

void Track::clearHoles() {
    m_holes.clear();
    m_tunnels.clear();
}

bool Track::generateTunnelBetweenHoles(size_t holeA, size_t holeB, u32 seed) {
    if (holeA >= m_holes.size() || holeB >= m_holes.size() || holeA == holeB) {
        return false;
    }

    const TrackHole& startHole = m_holes[holeA];
    const TrackHole& endHole = m_holes[holeB];

    const vec3 start = startHole.position + vec3{0.0f, -startHole.depth, 0.0f};
    const vec3 end = endHole.position + vec3{0.0f, -endHole.depth, 0.0f};
    const vec3 delta = end - start;
    const float distance = glm::length(delta);
    if (distance < 1.0f) {
        return false;
    }

    vec3 forward = delta / distance;
    vec3 up{0.0f, 1.0f, 0.0f};
    vec3 right = glm::cross(forward, up);
    if (glm::length2(right) < 0.0001f) {
        right = vec3{1.0f, 0.0f, 0.0f};
    } else {
        right = glm::normalize(right);
    }

    const float tunnelRadius = glm::max(4.0f, (startHole.radius + endHole.radius) * 0.45f);
    const float floorWidth = glm::max(tunnelRadius * 1.5f, tunnelRadius + 2.5f);
    const float roughness = glm::max(2.0f, distance * 0.05f);
    const float sinkDepth = glm::max(startHole.depth, endHole.depth) + tunnelRadius * 0.8f;

    std::mt19937 rng(seed ^ static_cast<u32>(holeA * 73856093u) ^ static_cast<u32>(holeB * 19349663u));
    std::uniform_real_distribution<float> lateralDist(-roughness, roughness);
    std::uniform_real_distribution<float> verticalDist(0.0f, roughness * 0.6f);

    const vec3 p0 = start;
    const vec3 p1 = start + forward * (distance * 0.22f) + right * lateralDist(rng) + vec3{0.0f, -sinkDepth - verticalDist(rng), 0.0f};
    const vec3 p2 = (start + end) * 0.5f + right * lateralDist(rng) * 1.4f + vec3{0.0f, -sinkDepth - roughness - verticalDist(rng), 0.0f};
    const vec3 p3 = start + forward * (distance * 0.78f) + right * lateralDist(rng) + vec3{0.0f, -sinkDepth - verticalDist(rng), 0.0f};
    const vec3 p4 = end;

    TrackTunnel tunnel;
    tunnel.id = makeTunnelId(m_tunnels.size());
    tunnel.radius = tunnelRadius;
    tunnel.floorWidth = floorWidth;
    tunnel.roughness = roughness;
    tunnel.seed = seed;
    tunnel.spline = Spline({p0, p1, p2, p3, p4});
    m_tunnels.push_back(std::move(tunnel));
    return true;
}

bool Track::loadFromFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        spdlog::error("Track::loadFromFile: cannot open {}", path);
        return false;
    }

    try {
        json j = json::parse(f);
        m_spline.clear();
        m_holes.clear();
        m_tunnels.clear();

        const json* road = &j;
        if (j.contains("road") && j["road"].is_object()) {
            road = &j["road"];
        }

        const json* terrain = nullptr;
        if (j.contains("terrain") && j["terrain"].is_object()) {
            terrain = &j["terrain"];
        }
        const json* files = nullptr;
        if (j.contains("files") && j["files"].is_object()) {
            files = &j["files"];
        }

        m_roadWidth = road->value("road_width", j.value("road_width", m_roadWidth));

        const bool closedLoop = road->value("closed_loop", j.value("closed_loop", false));
        if (terrain != nullptr) {
            m_terrain.seed = terrain->value("seed", m_terrain.seed);
            m_terrain.baseResolution = terrain->value("base_resolution", m_terrain.baseResolution);
            m_terrain.worldSize = terrain->value("world_size", m_terrain.worldSize);
            m_terrain.maxHeight = terrain->value("max_height", m_terrain.maxHeight);
            m_terrain.biome = terrain->value("biome", m_terrain.biome);
        }
        if (files != nullptr) {
            m_terrain.heightmapEditPath = files->value("heightmap_edit", m_terrain.heightmapEditPath);
            m_terrain.holeMaskPath = files->value("hole_mask", m_terrain.holeMaskPath);
        }
        if (j.contains("meta") && j["meta"].is_object()) {
            m_name = j["meta"].value("name", m_name);
        } else {
            m_name = std::filesystem::path(path).stem().string();
        }

        if (closedLoop) {
            m_spline.closeLoop();
        }

        const json* controlPoints = nullptr;
        if (road->contains("control_points")) {
            controlPoints = &(*road)["control_points"];
        } else if (j.contains("control_points")) {
            controlPoints = &j["control_points"];
        }
        if (controlPoints != nullptr) {
            for (const auto& point : *controlPoints) {
                m_spline.addControlPoint(vecFromJson(point));
            }
        }

        if (j.contains("holes") && j["holes"].is_array()) {
            for (const auto& holeJson : j["holes"]) {
                TrackHole hole;
                hole.id = holeJson.value("id", makeHoleId(m_holes.size()));
                if (holeJson.contains("position")) {
                    hole.position = vecFromJson(holeJson["position"]);
                }
                hole.radius = holeJson.value("radius", hole.radius);
                hole.depth = holeJson.value("depth", hole.depth);
                m_holes.push_back(std::move(hole));
            }
        }

        if (j.contains("tunnels") && j["tunnels"].is_array()) {
            for (const auto& tunnelJson : j["tunnels"]) {
                TrackTunnel tunnel;
                tunnel.id = tunnelJson.value("id", makeTunnelId(m_tunnels.size()));
                tunnel.radius = tunnelJson.value("radius", tunnel.radius);
                tunnel.floorWidth = tunnelJson.value("floor_width", tunnel.floorWidth);
                tunnel.roughness = tunnelJson.value("roughness", tunnel.roughness);
                tunnel.seed = tunnelJson.value("seed", tunnel.seed);
                if (tunnelJson.contains("control_points") && tunnelJson["control_points"].is_array()) {
                    for (const auto& point : tunnelJson["control_points"]) {
                        tunnel.spline.addControlPoint(vecFromJson(point));
                    }
                }
                if (tunnel.spline.getNumControlPoints() >= 2) {
                    m_tunnels.push_back(std::move(tunnel));
                }
            }
        }

        generateFromSpline(m_spline);
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Track::loadFromFile: {}", e.what());
        return false;
    }
}

bool Track::saveToFile(const std::string& path) {
    json j;
    j["meta"] = {
        {"name", m_name},
        {"version", 1}
    };
    j["terrain"] = {
        {"seed", m_terrain.seed},
        {"base_resolution", m_terrain.baseResolution},
        {"world_size", m_terrain.worldSize},
        {"max_height", m_terrain.maxHeight},
        {"biome", m_terrain.biome}
    };
    j["files"] = {
        {"heightmap_edit", m_terrain.heightmapEditPath},
        {"hole_mask", m_terrain.holeMaskPath}
    };
    j["road"] = {
        {"road_width", m_roadWidth},
        {"closed_loop", m_spline.isClosedLoop()},
        {"control_points", json::array()}
    };

    for (const auto& point : m_spline.getControlPoints()) {
        j["road"]["control_points"].push_back(vecToJson(point));
    }

    j["holes"] = json::array();
    for (const auto& hole : m_holes) {
        j["holes"].push_back({
            {"id", hole.id},
            {"position", vecToJson(hole.position)},
            {"radius", hole.radius},
            {"depth", hole.depth}
        });
    }

    j["tunnels"] = json::array();
    for (const auto& tunnel : m_tunnels) {
        json tunnelJson = {
            {"id", tunnel.id},
            {"radius", tunnel.radius},
            {"floor_width", tunnel.floorWidth},
            {"roughness", tunnel.roughness},
            {"seed", tunnel.seed},
            {"control_points", json::array()}
        };
        for (const auto& point : tunnel.spline.getControlPoints()) {
            tunnelJson["control_points"].push_back(vecToJson(point));
        }
        j["tunnels"].push_back(std::move(tunnelJson));
    }

    std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    std::ofstream f(path);
    if (!f.is_open()) {
        spdlog::error("Track::saveToFile: cannot write {}", path);
        return false;
    }

    f << j.dump(2);
    return true;
}

void Track::generateMesh() {
    m_roadVertices.clear();
    m_roadIndices.clear();
    
    if (m_segments.empty()) return;
    
    float halfWidth = m_roadWidth * 0.5f;
    
    for (const auto& segment : m_segments) {
        vec3 left = segment.frame.position - segment.frame.binormal * halfWidth;
        vec3 right = segment.frame.position + segment.frame.binormal * halfWidth;
        
        m_roadVertices.push_back(left);
        m_roadVertices.push_back(right);
    }
    
    const bool closedLoop = m_spline.isClosedLoop();
    const size_t quadCount = closedLoop ? m_segments.size() : (m_segments.size() > 1 ? m_segments.size() - 1 : 0);
    for (size_t i = 0; i < quadCount; ++i) {
        size_t next = closedLoop ? (i + 1) % m_segments.size() : i + 1;
        size_t base = i * 2;
        size_t nextBase = next * 2;
        
        m_roadIndices.push_back(static_cast<u32>(base));
        m_roadIndices.push_back(static_cast<u32>(nextBase));
        m_roadIndices.push_back(static_cast<u32>(base + 1));
        
        m_roadIndices.push_back(static_cast<u32>(base + 1));
        m_roadIndices.push_back(static_cast<u32>(nextBase));
        m_roadIndices.push_back(static_cast<u32>(nextBase + 1));
    }
}

void Track::generateTerrain() {
}

}
