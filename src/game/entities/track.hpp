#pragma once

#include "core/types.hpp"
#include "utils/spline.hpp"
#include <vector>
#include <memory>

namespace viber {

class Renderer;

struct TrackSegment {
    Spline::Frame frame;
    float width = 12.0f;
    float banking = 0.0f;
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
    
    void setRoadWidth(float width) { m_roadWidth = width; }
    float getRoadWidth() const { return m_roadWidth; }
    
    bool loadFromFile(const std::string& path);
    bool saveToFile(const std::string& path);
    
private:
    void generateMesh();
    void generateTerrain();
    
    Spline m_spline;
    std::vector<TrackSegment> m_segments;
    
    float m_roadWidth = 12.0f;
    int m_segmentCount = 100;
    
    std::vector<vec3> m_roadVertices;
    std::vector<u32> m_roadIndices;
};

}
