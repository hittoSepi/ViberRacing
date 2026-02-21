#pragma once

#include "core/types.hpp"

namespace viber {

struct CheckpointData {
    vec3 position{0.0f};
    vec3 normal{0.0f, 0.0f, 1.0f};
    float width = 10.0f;
    float height = 5.0f;
    int index = 0;
    bool isStartFinish = false;
};

class Checkpoint {
public:
    Checkpoint();
    explicit Checkpoint(const CheckpointData& data);
    ~Checkpoint() = default;
    
    void init(const CheckpointData& data);
    
    bool checkCrossing(const vec3& oldPos, const vec3& newPos) const;
    
    const vec3& getPosition() const { return m_data.position; }
    const vec3& getNormal() const { return m_data.normal; }
    int getIndex() const { return m_data.index; }
    bool isStartFinish() const { return m_data.isStartFinish; }
    
    void render(class Renderer& renderer);
    
private:
    CheckpointData m_data;
};

}
