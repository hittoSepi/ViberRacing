#pragma once

#include <bgfx/bgfx.h>

namespace viber {

// Load basic color shader (requires compiled .bin files)
bgfx::ProgramHandle createBasicShader();

// Load mesh preview shader for POSITION/NORMAL meshes.
bgfx::ProgramHandle createMeshShader();

// Load textured shader (requires compiled .bin files)
bgfx::ProgramHandle createTexturedShader();

// Load atmosphere shader (fullscreen gradient)
bgfx::ProgramHandle createAtmosphereShader();

} // namespace viber
