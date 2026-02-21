# Architecture Overview

## System Design

ViberRacing follows a modular architecture with clear separation of concerns:

```
┌─────────────────────────────────────────────────────────┐
│                      Game Layer                         │
│  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────────┐   │
│  │  Menu   │ │ Racing  │ │ Editor  │ │   Lobby     │   │
│  │  State  │ │  State  │ │  State  │ │   State     │   │
│  └────┬────┘ └────┬────┘ └────┬────┘ └──────┬──────┘   │
└───────┼───────────┼───────────┼───────────────┼─────────┘
        │           │           │               │
┌───────┴───────────┴───────────┴───────────────┴─────────┐
│                     Core Systems                        │
│  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────────┐   │
│  │ Window  │ │  Input  │ │ Config  │ │   Logger    │   │
│  └─────────┘ └─────────┘ └─────────┘ └─────────────┘   │
└─────────────────────────────────────────────────────────┘
        │           │           │               │
┌───────┴───────────┴───────────┴───────────────┴─────────┐
│                    Engine Systems                        │
│  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────────┐   │
│  │Renderer │ │ Physics │ │Network  │ │   Assets    │   │
│  │ (bgfx)  │ │(Bullet) │ │ (ENet)  │ │  (Assimp)   │   │
│  └─────────┘ └─────────┘ └─────────┘ └─────────────┘   │
└─────────────────────────────────────────────────────────┘
```

## Key Modules

### Core (`src/core/`)
- **Window**: GLFW wrapper with event callbacks
- **Input**: Keyboard, mouse, gamepad abstraction
- **Config**: JSON-based configuration system
- **Logger**: spdlog wrapper with file/console output

### Renderer (`src/renderer/`)
- **Renderer**: bgfx initialization and frame management
- **Camera**: Perspective/orthographic cameras with frustum culling
- **Mesh**: Vertex/index buffer management
- **Texture**: Texture loading and management
- **Shader**: Shader compilation and uniform management
- **DebugDraw**: Line/shape debug visualization
- **UI**: ImGui and RmlUi backends

### Physics (`src/physics/`)
- **World**: Bullet dynamics world wrapper
- **Vehicle**: btRaycastVehicle wrapper with custom tire model
- **TireModel**: Pacejka-inspired grip calculation
- **Collision**: Collision filtering and groups

### Networking (`src/networking/`)
- **Network**: ENet wrapper for host/client
- **P2PHost**: Server-side P2P logic
- **P2PClient**: Client-side P2P logic
- **Sync**: State serialization and synchronization

### Game (`src/game/`)
- **Game**: State machine and game loop
- **States**: Menu, Racing, Editor, Lobby
- **Entities**: Car, Track, Checkpoint
- **TrackEditor**: Spline-based track building tools

### Assets (`src/assets/`)
- **Manager**: Async asset loading and caching
- **MeshLoader**: Assimp model loading
- **TextureLoader**: STB image loading

## Data Flow

### Game Loop
```
Input → Game::update() → State::update() → Physics::step()
                                    ↓
                              Entities::update()
                                    ↓
                          Game::render() → State::render()
                                    ↓
                           Renderer::endFrame()
```

### Network Sync (Multiplayer)
```
Local Input → VehicleInputSync → Network::sendToHost
                                      ↓
                              Server broadcasts
                                      ↓
Network::onReceive → VehicleInputSync → Remote Vehicle
```

### Track Editor
```
Mouse Click → Editor::handleInput → Tool::update
                                      ↓
                          Track::modify (spline/terrain)
                                      ↓
                          Track::generateMesh
                                      ↓
                          Renderer::submit
```

## Threading Model

Current: Single-threaded main loop
Future considerations:
- Physics on separate thread (Bullet supports this)
- Asset loading on worker threads (already async-ready)
- Network processing on separate thread
