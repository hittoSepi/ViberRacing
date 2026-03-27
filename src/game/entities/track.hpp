#pragma once

#include "core/types.hpp"
#include "utils/spline.hpp"
#include "renderer/mesh.hpp"
#include <vector>
#include <memory>

namespace viber {

class Renderer;

struct TrackSegment {
    Spline::Frame frame;
    float width = 12.0f;
    float banking = 0.0f;
};

struct TrackTerrainSettings {
    u32 seed = 1337;
    int baseResolution = 512;
    float worldSize = 512.0f;
    float maxHeight = 96.0f;
    std::string biome = "temperate";
    std::string heightmapEditPath = "heightmap_edit.png";
    std::string holeMaskPath = "hole_mask.png";
};

struct TrackHole {
    std::string id;
    vec3 position{0.0f};
    float radius = 10.0f;
    float depth = 12.0f;
};

struct TrackTunnel {
    std::string id;
    Spline spline;
    float radius = 7.0f;
    float floorWidth = 11.0f;
    float roughness = 6.0f;
    u32 seed = 1337;
};

class Track {
public:
    Track();
    ~Track();
    
    void generateTestTrack();
    void generateFromSpline(const Spline& spline);
    
    void render(Renderer& renderer);
    
    const Spline& getSpline() const { return m_spline; }
    Spline& getSpline() { return m_spline; }
    
    float getLength() const;
    vec3 getPositionAt(float t) const;
    vec3 getDirectionAt(float t) const;

    void setName(const std::string& name) { m_name = name; }
    const std::string& getName() const { return m_name; }
    
    void setRoadWidth(float width) { m_roadWidth = width; }
    float getRoadWidth() const { return m_roadWidth; }

    TrackTerrainSettings& terrainSettings() { return m_terrain; }
    const TrackTerrainSettings& terrainSettings() const { return m_terrain; }

    std::vector<TrackHole>& holes() { return m_holes; }
    const std::vector<TrackHole>& holes() const { return m_holes; }

    std::vector<TrackTunnel>& tunnels() { return m_tunnels; }
    const std::vector<TrackTunnel>& tunnels() const { return m_tunnels; }

    void addHole(const TrackHole& hole);
    void clearHoles();
    bool generateTunnelBetweenHoles(size_t holeA, size_t holeB, u32 seed);
    
    bool loadFromFile(const std::string& path);
    bool saveToFile(const std::string& path);

    const std::vector<vec3>& getRoadVertices() const { return m_roadVertices; }
    const std::vector<u32>& getRoadIndices() const { return m_roadIndices; }
    
private:
    void generateMesh();
    void generateTerrain();
    
    Spline m_spline;
    std::vector<TrackSegment> m_segments;
    std::string m_name = "Harbor Loop";
    
    float m_roadWidth = 12.0f;
    int m_segmentCount = 100;
    TrackTerrainSettings m_terrain;
    std::vector<TrackHole> m_holes;
    std::vector<TrackTunnel> m_tunnels;
    
    std::vector<vec3> m_roadVertices;
    std::vector<u32> m_roadIndices;
};

}
