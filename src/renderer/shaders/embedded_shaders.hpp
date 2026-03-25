#pragma once

#include <bgfx/bgfx.h>

namespace viber {

// Load basic color shader (requires compiled .bin files)
bgfx::ProgramHandle createBasicShader();

// Load textured shader (requires compiled .bin files)
bgfx::ProgramHandle createTexturedShader();

} // namespace viber
