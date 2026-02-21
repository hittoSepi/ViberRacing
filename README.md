# ViberRacing

A 3D racing game with collaborative track editor, built with modern C++ and OpenGL 4.5 via bgfx.

## Features

- **3D Racing**: Arcade-style vehicle physics with realistic tire model
- **Procedural Tracks**: Spline-based track generation with terrain modification
- **Track Editor**: In-game collaborative editor for building custom tracks
- **Multiplayer**: P2P networking for racing and collaborative building
- **Cross-Platform**: Linux primary, Windows support planned

## Requirements

- C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2022+)
- CMake 3.20+
- OpenGL 4.5 capable GPU

## Build Instructions

### Linux

```bash
# Clone with submodules
git clone --recursive https://github.com/yourname/ViberRacing.git
cd ViberRacing

# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)

# Run
./bin/ViberRacing
```

### Windows (MSVC)

```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Debug
```

## Controls

### Driving
- **WASD / Arrow Keys**: Accelerate / Brake / Steer
- **Space**: Handbrake
- **R**: Reset position
- **Escape**: Menu

### Editor
- **Middle Mouse**: Rotate camera
- **Shift + MMB**: Pan camera
- **Scroll**: Zoom
- **G**: Toggle grid

## Project Structure

```
ViberRacing/
├── src/
│   ├── core/       # Window, input, config, logging
│   ├── renderer/   # bgfx wrapper, shaders, camera
│   ├── physics/    # Bullet wrapper, vehicle, tire model
│   ├── game/       # Game states, entities, track editor
│   ├── networking/ # ENet P2P, state sync
│   └── assets/     # Resource manager, loaders
├── assets/         # Models, textures, shaders, config
├── tests/          # Unit and integration tests
└── third_party/    # Git submodules
```

## Dependencies

| Library | Purpose |
|---------|---------|
| bgfx | Rendering |
| GLFW | Window/input |
| GLM | Math |
| Bullet3 | Physics |
| ENet | Networking |
| Assimp | Model loading |
| ImGui | Debug UI |
| RmlUi | Game HUD |
| spdlog | Logging |
| Catch2 | Testing |

## License

MIT License

## Development Status

Early development. See docs/MILESTONES.md for progress.
