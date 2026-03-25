# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

ViberRacing is a 3D racing game with collaborative track editor, built with C++20 and OpenGL 4.5 via bgfx. P2P multiplayer via ENet. Bullet3 for physics. Single-threaded game loop.

## Build Commands

```bash
# First time: clone with --recursive, or init submodules after clone
git submodule update --init --recursive

# Configure (from repo root)
cmake -B build -DCMAKE_BUILD_TYPE=Debug        # Debug (defines VIBER_DEBUG)
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo  # defines VIBER_RELEASE
cmake -B build -DCMAKE_BUILD_TYPE=Release         # defines VIBER_RELEASE + VIBER_SHIPPING

# Build
cmake --build build -j$(nproc)          # Linux
cmake --build build --config Debug      # Windows/MSVC

# Run
./build/bin/ViberRacing

# Tests (from build directory)
cd build && ctest --output-on-failure
ctest -R "Spline"            # Run specific test by name regex
ctest -R "unit"              # Unit tests only
ctest -R "integration"       # Integration tests only

# Format
find src tests -name "*.hpp" -o -name "*.cpp" | xargs clang-format -i

# CMake options
-DVIBER_BUILD_TESTS=OFF
-DVIBER_ENABLE_PROFILING=ON
```

## Architecture

Each `src/` subdirectory compiles to a static library (`vr_core`, `vr_utils`, `vr_renderer`, `vr_physics`, `vr_networking`, `vr_assets`, `vr_game`). Dependency graph:

```
vr_core       <- glfw, spdlog, nlohmann_json
vr_utils      <- vr_core
vr_renderer   <- vr_core, vr_utils, bgfx, imgui, RmlUi
vr_physics    <- vr_core, vr_utils, Bullet3
vr_networking <- vr_core, vr_utils, enet
vr_assets     <- vr_core, vr_renderer, assimp
vr_game       <- all of the above
```

Game uses a state machine pattern: `MenuState`, `RacingState`, `EditorState`, `LobbyState` — all inherit from `GameState` in [state.hpp](src/game/states/state.hpp).

Track editor uses spline-based track generation with road/terrain/prop tools.

All 15 dependencies are git submodules under `third_party/`.

## Code Conventions

- **Naming**: PascalCase classes, camelCase methods, `m_` prefix for members, snake_case files
- **Types**: Use aliases from `core/types.hpp` — `i32`, `f32`, `vec3`, `mat4`, `Scope<T>` (unique_ptr), `Ref<T>` (shared_ptr), `createScope<T>()`, `createRef<T>()`
- **Headers**: `#pragma once`, include order: stdlib → third-party → project
- **RAII**: Resource classes use `create()`/`destroy()` pattern with `m_created` guard
- **Error handling**: `bool` returns for fallible ops, `spdlog::error()` before returning false, exceptions only for fatal init errors
- **Tests**: Catch2 v3, `TEST_CASE`/`SECTION` macros, `Approx().margin()` for floats. Files: `tests/unit/test_<module>.cpp`, `tests/integration/test_<feature>.cpp`
- **Format**: clang-format LLVM style, 120 column limit
