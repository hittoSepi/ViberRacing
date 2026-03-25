#pragma once

#include "core/types.hpp"
#include "renderer/mesh.hpp"
#include <bgfx/bgfx.h>

namespace viber {

// Simple grid ground plane
class GroundPlane {
public:
    GroundPlane() = default;
    ~GroundPlane() { destroy(); }

    GroundPlane(const GroundPlane&) = delete;
    GroundPlane& operator=(const GroundPlane&) = delete;

    void init(float size = 50.0f, u32 gridLines = 50);
    void destroy();

    void render(bgfx::ViewId viewId, bgfx::ProgramHandle program, const mat4& view, const mat4& proj);

private:
    Mesh m_mesh;
    bool m_initialized = false;
};

} // namespace viber
