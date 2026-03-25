# ViberRacing Assets

## Current Status
The game currently uses procedural generation for most content and does not require external asset files to run.

## Asset Structure

```
assets/
├── config/
│   └── default.json          # Game configuration (already exists)
├── shaders/
│   ├── vs_basic.bin          # Basic vertex shader (bgfx format)
│   ├── fs_basic.bin          # Basic fragment shader (bgfx format)
│   ├── vs_textured.bin       # Textured vertex shader
│   ├── fs_textured.bin       # Textured fragment shader
│   └── ...
├── textures/
│   ├── ui/
│   │   ├── button.png
│   │   ├── panel.png
│   │   └── logo.png
│   ├── vehicles/
│   │   ├── car_body.png
│   │   ├── car_wheel.png
│   │   └── car_interior.png
│   └── tracks/
│       ├── asphalt_diffuse.png
│       ├── grass_diffuse.png
│       └── road_normal.png
├── models/
│   ├── vehicles/
│   │   ├── car_chassis.obj
│   │   ├── car_wheel.obj
│   │   └── car_steering_wheel.obj
│   └── tracks/
│       ├── barrier.obj
│       ├── checkpoint.obj
│       └── tree.obj
├── audio/
│   ├── sfx/
│   │   ├── engine_loop.wav
│   │   ├── tire_squeal.wav
│   │   ├── crash.wav
│   │   └── ui_click.wav
│   └── music/
│       ├── menu_theme.ogg
│       └── race_music_*.ogg
└── fonts/
    ├── main.ttf              # UI font
    └── hud.ttf               # HUD/digital font
```

## Supported Formats

### Meshes
- `.obj` - Wavefront OBJ (recommended for simple models)
- `.fbx` - Autodesk FBX (via Assimp)
- `.gltf` / `.glb` - glTF 2.0 (recommended for modern workflows)
- `.dae` - Collada

### Textures
- `.png` - PNG with alpha
- `.jpg` - JPEG (no alpha)
- `.tga` - Targa
- `.dds` - DirectDraw Surface (compressed)
- `.ktx` - Khronos Texture

### Shaders
Shaders must be compiled to bgfx's binary format (`.bin`):
- Use `shaderc` tool from bgfx to compile from:
  - Vertex shaders: `.vs` or `.vert`
  - Fragment shaders: `.fs` or `.frag`

Example compilation:
```bash
shaderc -f vs_basic.sc -o vs_basic.bin \
    --type vertex --platform linux \
    -p spirv -i $BGFX_DIR/src
```

### Audio
- `.wav` - WAV for short sound effects
- `.ogg` - Ogg Vorbis for music (streaming)

## Procedurally Generated Content

Currently implemented:
- **Track mesh**: Generated from Catmull-Rom spline
- **Vehicle physics**: Raycast vehicle, no visual mesh required
- **Debug visualization**: Wireframe rendering

## Required for Full Game

### Minimum Viable
1. **Shaders**: Basic lit shaders for track and vehicle
2. **Vehicle Model**: Simple chassis + 4 wheels
3. **Track Textures**: Road surface, grass, barriers
4. **UI Font**: For text rendering (blocks issue needs fixing first)

### Nice to Have
1. **Skybox**: For atmosphere
2. **Particle Effects**: Exhaust, tire smoke, sparks
3. **Audio**: Engine sounds, collision sounds
4. **Environment**: Trees, buildings, decorations

## TODO

- [ ] Fix ImGui font rendering (shows as blocks)
- [ ] Create basic shader set
- [ ] Add simple vehicle model
- [ ] Add track textures
- [ ] Implement audio system
