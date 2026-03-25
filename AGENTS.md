# AGENTS.md

Coding agent instructions for the ViberRacing codebase.

## Project Overview

ViberRacing is a 3D racing game with a collaborative track editor, built with modern C++20 and OpenGL 4.5 via bgfx.

### Technology Stack

| Component | Technology | Version/Notes |
|-----------|------------|---------------|
| Language | C++ | C++20 standard |
| Build System | CMake | 3.20+ required |
| Rendering | bgfx | OpenGL 4.5 backend |
| Physics | Bullet3 | btRaycastVehicle for vehicles |
| Networking | ENet | P2P multiplayer |
| Window/Input | GLFW | Cross-platform |
| Math | GLM | Column-major matrices |
| UI (Debug) | ImGui | Runtime debug panels |
| UI (Game) | RmlUi | Game HUD (HTML/CSS-like) |
| Logging | spdlog | File + console output |
| Model Loading | Assimp | Multiple format support |
| Image Loading | STB | stb_image for textures |
| Config/JSON | nlohmann/json | JSON-based config |
| Testing | Catch2 | v3 with Catch2::Catch2WithMain |
| String Format | fmt | Through spdlog dependency |
| Code Format | clang-format | LLVM style, 120 columns |

## Build Commands

### Initial Setup

```bash
# Clone with all submodules (required!)
git clone --recursive https://github.com/yourname/ViberRacing.git
cd ViberRacing

# If submodules are missing, initialize them:
git submodule update --init --recursive

# Build
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
```

### Build Options

```bash
# Debug build (default, defines VIBER_DEBUG)
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Release with debug info (defines VIBER_RELEASE)
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo

# Shipping release (defines VIBER_RELEASE and VIBER_SHIPPING)
cmake .. -DCMAKE_BUILD_TYPE=Release

# Disable tests
cmake .. -DVIBER_BUILD_TESTS=OFF

# Enable profiling
 cmake .. -DVIBER_ENABLE_PROFILING=ON
```

### Rebuild After Changes

```bash
cd build && make -j$(nproc)
```

### Run the Game

```bash
./bin/ViberRacing
```

### Format Code

```bash
find src tests -name "*.hpp" -o -name "*.cpp" | xargs clang-format -i
```

## Test Commands

```bash
# Run all tests (from build directory)
ctest --output-on-failure

# Run specific test by name (regex match)
ctest -R "Spline"
ctest -R "PhysicsWorld"
ctest -R "Vehicle"

# Run with verbose output
ctest -V -R "TestName"

# Run unit tests only
ctest -R "unit"

# Run integration tests only
ctest -R "integration"
```

## Project Structure

```
ViberRacing/
├── src/
│   ├── core/          # Window, input, config, logging, types
│   ├── renderer/      # bgfx wrapper, shaders, camera, mesh, texture
│   ├── physics/       # Bullet wrapper, vehicle, tire model, collision
│   ├── game/          # Game states, entities, track editor
│   ├── networking/    # ENet P2P, state sync
│   ├── assets/        # Resource manager, mesh/texture loaders
│   └── utils/         # Math, profiler, spline, serialization
├── tests/
│   ├── unit/          # Unit tests
│   └── integration/   # Integration tests
├── assets/            # Models, textures, shaders, config
├── tools/             # Shader compiler script
├── docs/              # Architecture documentation
└── third_party/       # Git submodules (dependencies)
```

### Module Dependencies

```
vr_core       <- glfw, spdlog, nlohmann_json
vr_utils      <- vr_core
vr_renderer   <- vr_core, vr_utils, bgfx, imgui, RmlUi
vr_physics    <- vr_core, vr_utils, Bullet3
vr_networking <- vr_core, vr_utils, enet
vr_assets     <- vr_core, vr_renderer, assimp
vr_game       <- vr_core, vr_renderer, vr_physics, vr_networking, vr_assets, vr_utils
```

Each module compiles to a static library with the naming pattern `vr_<module>`.

## Code Style Guidelines

### Naming Conventions

| Element | Convention | Example |
|---------|------------|---------|
| Files | snake_case | `vehicle.cpp`, `tire_model.hpp` |
| Classes/Structs | PascalCase | `PhysicsWorld`, `VehicleParams` |
| Functions/Methods | camelCase | `updateCamera`, `setSteering`, `addControlPoint` |
| Member variables | `m_` prefix | `m_world`, `m_chassisBody`, `m_initialized` |
| Constants | PascalCase constexpr | `constexpr f32 Pi = 3.14159f;` |
| Enum classes | PascalCase | `enum class GameStateType { ... }` |
| Enum values | PascalCase | `Menu, Racing, Editor, Lobby` |
| Template parameters | PascalCase | `typename T, typename Allocator` |

### Types

Always use type aliases from `core/types.hpp`:

```cpp
// Integral types
i8, i16, i32, i64    // Signed integers
u8, u16, u32, u64    // Unsigned integers

// Floating point
f32, f64             // float, double

// Math types (from GLM)
vec2, vec3, vec4     // float vectors
ivec2, ivec3, ivec4  // int vectors
uvec2, uvec3, uvec4  // uint vectors
mat2, mat3, mat4     // matrices
quat                 // quaternion

// Constants (defined in types.hpp)
PI, TWO_PI, HALF_PI
DEG_TO_RAD, RAD_TO_DEG
```

### Smart Pointers

```cpp
// Use these aliases instead of std:: directly
Scope<T>    // = std::unique_ptr<T>
Ref<T>      // = std::shared_ptr<T>

// Factory functions
createScope<T>(args...)    // = std::make_unique<T>(...)
createRef<T>(args...)      // = std::make_shared<T>(...)
```

Example:
```cpp
Scope<Vehicle> vehicle = createScope<Vehicle>();
auto vehicle = createScope<Vehicle>();  // Also OK with auto
```

### Headers and Includes

- Use `#pragma once` for header guards (NOT `#ifndef` guards)
- Include order:
  1. Standard library headers
  2. Third-party headers
  3. Project headers (relative to `src/`)

```cpp
#pragma once

#include <string>
#include <vector>
#include <memory>

#include <spdlog/spdlog.h>
#include <glm/glm.hpp>

#include "core/types.hpp"
#include "physics/world.hpp"
```

### Classes

Follow this class layout pattern:

```cpp
class MyClass {
public:
    // Constructor/Destructor
    MyClass() = default;
    explicit MyClass(const Params& params);
    ~MyClass() { destroy(); }
    
    // Disable copy (for resource-owning classes)
    MyClass(const MyClass&) = delete;
    MyClass& operator=(const MyClass&) = delete;
    
    // Enable move (if needed)
    MyClass(MyClass&&) noexcept = default;
    MyClass& operator=(MyClass&&) noexcept = default;
    
    // Public interface
    bool create();
    void destroy();
    void update(float deltaTime);
    
    // Getters/Setters
    bool isCreated() const { return m_created; }
    void setValue(int value) { m_value = value; }
    
private:
    // Member variables
    int m_value = 0;
    bool m_created = false;
    Scope<Resource> m_resource;
};
```

### RAII Pattern

For resource management, follow this pattern:

```cpp
class Resource {
public:
    Resource() = default;
    ~Resource() { destroy(); }
    
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;
    
    bool create() {
        if (m_created) return true;
        // ... create resource
        m_created = true;
        return true;
    }
    
    void destroy() {
        if (!m_created) return;
        // ... destroy resource
        m_created = false;
    }
    
private:
    bool m_created = false;
};
```

### State Pattern (Game States)

```cpp
class GameState {
public:
    explicit GameState(Game& game) : m_game(game) {}
    virtual ~GameState() = default;
    
    virtual void init() = 0;
    virtual void shutdown() = 0;
    virtual void onEnter() = 0;
    virtual void onExit() = 0;
    virtual void onPause() {}
    virtual void onResume() {}
    
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    
    virtual const char* getName() const = 0;
    
protected:
    Game& m_game;
};
```

### Error Handling

- Use exceptions for fatal/init errors (e.g., renderer initialization failure)
- Return `bool` for fallible operations (e.g., `loadTrack()`, `saveTrack()`)
- Log errors with `spdlog::error()` before returning failure
- Use `REQUIRE_NOTHROW` in tests for operations that shouldn't throw

```cpp
bool loadConfig(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        spdlog::error("Failed to open config: {}", path);
        return false;
    }
    // ... parse config
    return true;
}
```

## Testing Instructions

### Writing Tests

Use Catch2 with `TEST_CASE` and `SECTION` macros:

```cpp
#include <catch2/catch_test_macros.hpp>
#include "utils/spline.hpp"

TEST_CASE("Spline evaluation", "[spline]") {
    viber::Spline spline;
    
    SECTION("Empty spline") {
        REQUIRE(spline.getNumControlPoints() == 0);
    }
    
    SECTION("Add control points") {
        spline.addControlPoint(viber::vec3(0.0f, 0.0f, 0.0f));
        REQUIRE(spline.getNumControlPoints() == 1);
    }
}
```

### Floating-Point Comparisons

Always use `Approx()` with margin:

```cpp
REQUIRE(value == Approx(1.0f).margin(0.001f));
REQUIRE(vec.x == Approx(expectedX).margin(0.01f));
```

### Test Organization

- Unit tests: `tests/unit/test_<module>.cpp`
- Integration tests: `tests/integration/test_<feature>.cpp`
- Tag tests: `[physics]`, `[spline]`, `[networking]`, `[math]`, etc.

## Shader Compilation

Shaders use bgfx's shaderc compiler:

```bash
# Compile all shaders
cd tools && ./compile_shaders.sh

# Or set custom shaderc path
BGFX_SHADERC=/path/to/shaderc ./compile_shaders.sh
```

### Shader Naming

- `vs_*.sc` - Vertex shaders
- `fs_*.sc` - Fragment shaders
- `cs_*.sc` - Compute shaders

### Shader Directories

- Source: `assets/shaders/*.sc`
- Compiled: `assets/shaders/compiled/*.bin`

## Configuration

Default config is at `assets/config/default.json`:

```json
{
    "window": {
        "width": 1280,
        "height": 720,
        "fullscreen": false,
        "vsync": true,
        "title": "ViberRacing"
    },
    "graphics": {
        "msaa": 4,
        "shadowQuality": 2,
        "textureQuality": 2,
        "anisotropy": 16
    },
    "network": {
        "port": 7777,
        "playerName": "Player",
        "maxPlayers": 8
    }
}
```

## Logging

```cpp
#include <spdlog/spdlog.h>

// Get the logger
auto logger = spdlog::get("viber");

// Log levels
logger->trace("Trace message");
logger->debug("Debug message");
logger->info("Info message");
logger->warn("Warning message");
logger->error("Error message");
logger->critical("Critical message");
```

## Debugging Tips

- Enable bgfx debug text: `bgfx::setDebug(BGFX_DEBUG_TEXT)`
- Use `VIBER_DEBUG` macro for debug-only code
- Use ImGui for runtime debug UI panels
- Use `DebugDraw` for 3D visualization of physics/collision

## Key Third-Party Headers

```cpp
// GLM
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

// spdlog
#include <spdlog/spdlog.h>

// bgfx
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

// Bullet3
#include <btBulletDynamicsCommon.h>

// ENet
#include <enet/enet.h>

// ImGui
#include <imgui.h>

// JSON
#include <nlohmann/json.hpp>

// GLFW
#include <GLFW/glfw3.h>

// Catch2 (tests only)
#include <catch2/catch_test_macros.hpp>
```

## Development Status

Early development. See `docs/MILESTONES.md` for progress and `docs/ARCHITECTURE.md` for system design details.

## License

MIT License
