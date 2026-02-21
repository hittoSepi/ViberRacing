#include "spline.hpp"
#include <cmath>

namespace viber {

Spline::Spline(const std::vector<vec3>& controlPoints) 
    : m_controlPoints(controlPoints) {}

void Spline::addControlPoint(const vec3& point) {
    m_controlPoints.push_back(point);
}

void Spline::insertControlPoint(size_t index, const vec3& point) {
    if (index <= m_controlPoints.size()) {
        m_controlPoints.insert(m_controlPoints.begin() + index, point);
    }
}

void Spline::removeControlPoint(size_t index) {
    if (index < m_controlPoints.size()) {
        m_controlPoints.erase(m_controlPoints.begin() + index);
    }
}

void Spline::setControlPoint(size_t index, const vec3& point) {
    if (index < m_controlPoints.size()) {
        m_controlPoints[index] = point;
    }
}

vec3 Spline::evaluate(float t) const {
    if (m_controlPoints.size() < 2) {
        return m_controlPoints.empty() ? vec3(0.0f) : m_controlPoints[0];
    }
    
    t = glm::clamp(t, 0.0f, 1.0f);
    
    const size_t numSegments = m_closedLoop ? 
        m_controlPoints.size() : m_controlPoints.size() - 1;
    
    float scaledT = t * static_cast<float>(numSegments);
    size_t segment = static_cast<size_t>(std::floor(scaledT));
    float localT = scaledT - static_cast<float>(segment);
    
    if (segment >= numSegments) {
        segment = numSegments - 1;
        localT = 1.0f;
    }
    
    return evaluate(segment, localT);
}

vec3 Spline::evaluate(size_t segment, float localT) const {
    vec3 p0, p1, p2, p3;
    getControlPointsForSegment(segment, p0, p1, p2, p3);
    return catmullRom(p0, p1, p2, p3, localT);
}

vec3 Spline::getTangent(float t) const {
    if (m_controlPoints.size() < 2) {
        return vec3(1.0f, 0.0f, 0.0f);
    }
    
    t = glm::clamp(t, 0.0f, 1.0f);
    
    const size_t numSegments = m_closedLoop ? 
        m_controlPoints.size() : m_controlPoints.size() - 1;
    
    float scaledT = t * static_cast<float>(numSegments);
    size_t segment = static_cast<size_t>(std::floor(scaledT));
    float localT = scaledT - static_cast<float>(segment);
    
    if (segment >= numSegments) {
        segment = numSegments - 1;
        localT = 1.0f;
    }
    
    return getTangent(segment, localT);
}

vec3 Spline::getTangent(size_t segment, float localT) const {
    vec3 p0, p1, p2, p3;
    getControlPointsForSegment(segment, p0, p1, p2, p3);
    vec3 tangent = catmullRomTangent(p0, p1, p2, p3, localT);
    return glm::normalize(tangent);
}

vec3 Spline::getNormal(float t) const {
    auto frame = getFrame(t);
    return frame.normal;
}

vec3 Spline::getBinormal(float t) const {
    auto frame = getFrame(t);
    return frame.binormal;
}

float Spline::getLength() const {
    float totalLength = 0.0f;
    const size_t numSegments = m_closedLoop ? 
        m_controlPoints.size() : m_controlPoints.size() - 1;
    
    for (size_t i = 0; i < numSegments; ++i) {
        totalLength += getSegmentLength(i);
    }
    
    return totalLength;
}

float Spline::getSegmentLength(size_t segment) const {
    constexpr int numSamples = 10;
    float length = 0.0f;
    vec3 prevPoint = evaluate(segment, 0.0f);
    
    for (int i = 1; i <= numSamples; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(numSamples);
        vec3 point = evaluate(segment, t);
        length += glm::distance(prevPoint, point);
        prevPoint = point;
    }
    
    return length;
}

std::vector<vec3> Spline::sample(size_t numSamples) const {
    std::vector<vec3> samples;
    samples.reserve(numSamples);
    
    for (size_t i = 0; i < numSamples; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(numSamples - 1);
        samples.push_back(evaluate(t));
    }
    
    return samples;
}

std::vector<vec3> Spline::sampleWithDistance(float distanceBetweenSamples) const {
    std::vector<vec3> samples;
    float totalLength = getLength();
    
    if (totalLength < distanceBetweenSamples) {
        samples.push_back(evaluate(0.0f));
        return samples;
    }
    
    size_t numSamples = static_cast<size_t>(totalLength / distanceBetweenSamples) + 1;
    samples.reserve(numSamples);
    
    float accumulatedLength = 0.0f;
    samples.push_back(evaluate(0.0f));
    
    constexpr float step = 0.001f;
    float t = 0.0f;
    vec3 prevPoint = evaluate(0.0f);
    
    while (t < 1.0f) {
        t += step;
        vec3 point = evaluate(t);
        accumulatedLength += glm::distance(prevPoint, point);
        
        if (accumulatedLength >= distanceBetweenSamples) {
            samples.push_back(point);
            accumulatedLength = 0.0f;
        }
        
        prevPoint = point;
    }
    
    return samples;
}

Spline::Frame Spline::getFrame(float t) const {
    Frame frame;
    frame.position = evaluate(t);
    frame.tangent = getTangent(t);
    
    vec3 up(0.0f, 1.0f, 0.0f);
    if (std::abs(glm::dot(frame.tangent, up)) > 0.99f) {
        up = vec3(0.0f, 0.0f, 1.0f);
    }
    
    frame.binormal = glm::normalize(glm::cross(frame.tangent, up));
    frame.normal = glm::normalize(glm::cross(frame.binormal, frame.tangent));
    
    return frame;
}

std::vector<Spline::Frame> Spline::sampleFrames(size_t numSamples) const {
    std::vector<Frame> frames;
    frames.reserve(numSamples);
    
    for (size_t i = 0; i < numSamples; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(numSamples - 1);
        frames.push_back(getFrame(t));
    }
    
    return frames;
}

void Spline::closeLoop() {
    m_closedLoop = true;
}

void Spline::clear() {
    m_controlPoints.clear();
    m_closedLoop = false;
}

vec3 Spline::catmullRom(const vec3& p0, const vec3& p1, const vec3& p2, const vec3& p3, float t) const {
    const float t2 = t * t;
    const float t3 = t2 * t;
    
    return 0.5f * (
        (2.0f * p1) +
        (-p0 + p2) * t +
        (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
        (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3
    );
}

vec3 Spline::catmullRomTangent(const vec3& p0, const vec3& p1, const vec3& p2, const vec3& p3, float t) const {
    const float t2 = t * t;
    
    return 0.5f * (
        (-p0 + p2) +
        2.0f * (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t +
        3.0f * (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t2
    );
}

void Spline::getControlPointsForSegment(size_t segment, vec3& p0, vec3& p1, vec3& p2, vec3& p3) const {
    const size_t n = m_controlPoints.size();
    
    if (m_closedLoop) {
        p0 = m_controlPoints[(segment + n - 1) % n];
        p1 = m_controlPoints[segment];
        p2 = m_controlPoints[(segment + 1) % n];
        p3 = m_controlPoints[(segment + 2) % n];
    } else {
        p0 = (segment == 0) ? m_controlPoints[0] : m_controlPoints[segment - 1];
        p1 = m_controlPoints[segment];
        p2 = m_controlPoints[segment + 1];
        p3 = (segment + 2 >= n) ? m_controlPoints[n - 1] : m_controlPoints[segment + 2];
    }
}

}
