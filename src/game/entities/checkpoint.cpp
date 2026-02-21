#include "checkpoint.hpp"
#include "renderer/renderer.hpp"

namespace viber {

Checkpoint::Checkpoint() = default;

Checkpoint::Checkpoint(const CheckpointData& data) {
    init(data);
}

void Checkpoint::init(const CheckpointData& data) {
    m_data = data;
}

bool Checkpoint::checkCrossing(const vec3& oldPos, const vec3& newPos) const {
    float oldDist = glm::dot(oldPos - m_data.position, m_data.normal);
    float newDist = glm::dot(newPos - m_data.position, m_data.normal);
    
    if (oldDist * newDist >= 0.0f) {
        return false;
    }
    
    vec3 crossPoint = oldPos + (newPos - oldPos) * (oldDist / (oldDist - newDist));
    vec3 localPos = crossPoint - m_data.position;
    
    float halfWidth = m_data.width * 0.5f;
    float halfHeight = m_data.height * 0.5f;
    
    if (std::abs(localPos.x) > halfWidth) return false;
    if (std::abs(localPos.y) > halfHeight) return false;
    
    return true;
}

void Checkpoint::render(Renderer&) {
}

}
