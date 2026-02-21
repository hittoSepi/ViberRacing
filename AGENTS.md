# AGENTS.md

Coding agent instructions for the ViberRacing codebase.

## Build Commands

```bash
# Initial setup (from project root)
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)

# Rebuild after changes
cd build && make -j$(nproc)

# Release build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Run the game
./bin/ViberRacing
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

## Code Style

### Project Structure
- `src/` - Main source organized by module (core, renderer, physics, game, networking, assets, utils)
- `tests/` - Unit tests (`unit/`) and integration tests (`integration/`)
- `third_party/` - Git submodules for dependencies
- Each module compiles to a static library: `vr_core`, `vr_renderer`, `vr_physics`, `vr_game`, `vr_networking`, `vr_assets`, `vr_utils`

### Naming Conventions
- **Files**: snake_case (`vehicle.cpp`, `tire_model.hpp`)
- **Classes/Structs**: PascalCase (`PhysicsWorld`, `VehicleParams`)
- **Functions/Methods**: camelCase (`updateCamera`, `setSteering`, `addControlPoint`)
- **Member variables**: `m_` prefix (`m_world`, `m_chassisBody`, `m_initialized`)
- **Constants**: `SCREAMING_SNAKE_CASE` for macros, `PascalCase` for constexpr
- **Enums**: PascalCase for enum class names, PascalCase for values

### Types
- Use type aliases from `core/types.hpp`: `i8`, `i32`, `u32`, `f32`, `f64`, `vec2`, `vec3`, `mat4`, `quat`
- Smart pointers: `Scope<T>` (unique_ptr), `Ref<T>` (shared_ptr)
- Factory functions: `createScope<T>()`, `createRef<T>()`

### Headers and Includes
- Use `#pragma once` for header guards
- Include order: standard library → third-party → project headers
- Use relative includes from src root: `"core/types.hpp"`, `"physics/world.hpp"`

### Classes
- Mark single-argument constructors `explicit`
- Delete copy constructor/assignment for resource-owning classes
- Use `= default` for trivial constructors/destructors
- Initialize member variables in-class or in initializer lists
- Group public methods before private members

### Error Handling
- Use exceptions for fatal/init errors (e.g., renderer initialization failure)
- Return `bool` for fallible operations (e.g., `loadTrack()`, `saveTrack()`)
- Log errors with `spdlog::error()` before returning failure
- Use `REQUIRE_NOTHROW` in tests for operations that shouldn't throw

### Testing
- Use Catch2 with `TEST_CASE` and `SECTION` macros
- Tag tests appropriately: `[physics]`, `[spline]`, `[networking]`
- Use `Approx().margin()` for floating-point comparisons
- Place unit tests in `tests/unit/`, integration tests in `tests/integration/`

## Third-Party Libraries

| Library | Purpose | Key Headers |
|---------|---------|-------------|
| GLM | Math | `<glm/glm.hpp>`, `<glm/gtc/quaternion.hpp>`, `<glm/gtx/euler_angles.hpp>` |
| spdlog | Logging | `<spdlog/spdlog.h>` |
| bgfx | Rendering | `<bgfx/bgfx.h>`, `<bgfx/platform.h>` |
| Bullet3 | Physics | `<btBulletDynamicsCommon.h>` |
| ENet | Networking | `<enet/enet.h>` |
| ImGui | Debug UI | `<imgui.h>` |
| Catch2 | Testing | `<catch2/catch_test_macros.hpp>` |
| nlohmann/json | Config | `<nlohmann/json.hpp>` |
| GLFW | Window | `<GLFW/glfw3.h>` |

## Common Patterns

### Resource Management (RAII)
```cpp
class Resource {
public:
    Resource() = default;
    ~Resource() { destroy(); }
    
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;
    
    bool create();
    void destroy();
    
private:
    std::unique_ptr<Handle> m_handle;
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
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    
protected:
    Game& m_game;
};
```

### Callbacks
```cpp
using ConnectCallback = std::function<void(u32 clientId)>;
using ReceiveCallback = std::function<void(u32 clientId, const u8* data, size_t size)>;
```

### Physics Objects
```cpp
bool create(PhysicsWorld* world, const Params& params = {});
void destroy();
void update(float deltaTime);
```

## Formatting

Run clang-format before committing:
```bash
find src tests -name "*.hpp" -o -name "*.cpp" | xargs clang-format -i
```

## Module Dependencies

```
vr_core      <- glfw, spdlog, nlohmann_json
vr_utils     <- vr_core
vr_renderer  <- vr_core, bgfx, imgui
vr_physics   <- vr_core, bullet3
vr_networking<- vr_core, enet
vr_assets    <- vr_core, assimp, stb
vr_game      <- vr_core, vr_renderer, vr_physics, vr_networking
```

## Debugging

- Use `spdlog::info()`, `spdlog::warn()`, `spdlog::error()` for logging
- Get logger: `auto logger = spdlog::get("viber");`
- Enable bgfx debug text: `bgfx::setDebug(BGFX_DEBUG_TEXT)`
- `VIBER_DEBUG` macro is defined in Debug builds
- Use ImGui for runtime debug UI panels
