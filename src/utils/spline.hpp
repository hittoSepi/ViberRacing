#pragma once

#include "core/types.hpp"
#include <vector>

namespace viber {

class Spline {
public:
    Spline() = default;
    explicit Spline(const std::vector<vec3>& controlPoints);
    
    void addControlPoint(const vec3& point);
    void insertControlPoint(size_t index, const vec3& point);
    void removeControlPoint(size_t index);
    void setControlPoint(size_t index, const vec3& point);
    
    const std::vector<vec3>& getControlPoints() const { return m_controlPoints; }
    size_t getNumControlPoints() const { return m_controlPoints.size(); }
    
    vec3 evaluate(float t) const;
    vec3 evaluate(size_t segment, float localT) const;
    
    vec3 getTangent(float t) const;
    vec3 getTangent(size_t segment, float localT) const;
    
    vec3 getNormal(float t) const;
    vec3 getBinormal(float t) const;
    
    float getLength() const;
    float getSegmentLength(size_t segment) const;
    
    std::vector<vec3> sample(size_t numSamples) const;
    std::vector<vec3> sampleWithDistance(float distanceBetweenSamples) const;
    
    struct Frame {
        vec3 position;
        vec3 tangent;
        vec3 normal;
        vec3 binormal;
    };
    
    Frame getFrame(float t) const;
    std::vector<Frame> sampleFrames(size_t numSamples) const;
    
    void closeLoop();
    bool isClosedLoop() const { return m_closedLoop; }
    
    void clear();
    
private:
    std::vector<vec3> m_controlPoints;
    bool m_closedLoop = false;
    
    vec3 catmullRom(const vec3& p0, const vec3& p1, const vec3& p2, const vec3& p3, float t) const;
    vec3 catmullRomTangent(const vec3& p0, const vec3& p1, const vec3& p2, const vec3& p3, float t) const;
    
    void getControlPointsForSegment(size_t segment, vec3& p0, vec3& p1, vec3& p2, vec3& p3) const;
};

}
