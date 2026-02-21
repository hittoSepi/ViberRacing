#include "track.hpp"
#include "renderer/renderer.hpp"
#include <spdlog/spdlog.h>

namespace viber {

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
    m_segments.clear();
    
    float totalLength = spline.getLength();
    float segmentLength = totalLength / m_segmentCount;
    
    auto frames = spline.sampleFrames(m_segmentCount);
    
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

bool Track::loadFromFile(const std::string&) {
    return false;
}

bool Track::saveToFile(const std::string&) {
    return false;
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
    
    for (size_t i = 0; i < m_segments.size(); ++i) {
        size_t next = (i + 1) % m_segments.size();
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
