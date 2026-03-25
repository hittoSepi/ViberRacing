#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
SHADER_SRC="$PROJECT_ROOT/assets/shaders/src"
SHADER_OUT="$PROJECT_ROOT/assets/shaders"
BGFX_SHADERC="$PROJECT_ROOT/tools/shaderc"
BGFX_INCLUDE="$PROJECT_ROOT/third_party/bgfx/src"

mkdir -p "$SHADER_OUT"

PLATFORM="linux"
# Use SPIR-V for Vulkan backend
PROFILE="spirv"

compile_shader() {
    local input="$1"
    local output="$2"
    local type="$3"
    
    echo "Compiling $input -> $output"
    
    "$BGFX_SHADERC" \
        --platform "$PLATFORM" \
        --profile "$PROFILE" \
        --type "$type" \
        -i "$BGFX_INCLUDE" \
        --varyingdef "$SHADER_SRC/varying.def.sc" \
        -o "$output" \
        -f "$input"
    
    if [ $? -eq 0 ]; then
        echo "  ✓ Success"
    else
        echo "  ✗ Failed"
    fi
}

echo "=== Compiling shaders for Vulkan (SPIR-V) ==="

for shader in "$SHADER_SRC"/*.sc; do
    filename=$(basename "$shader")
    name="${filename%.*}"
    
    # Skip varying.def.sc - it's not a shader
    if [[ "$filename" == "varying.def.sc" ]]; then
        continue
    fi
    
    echo "Processing: ${name}"
    if [[ "$name" == vs_* ]]; then
        compile_shader "$shader" "$SHADER_OUT/$name.bin" "vertex"
    elif [[ "$name" == fs_* ]]; then
        compile_shader "$shader" "$SHADER_OUT/$name.bin" "fragment"
    elif [[ "$name" == cs_* ]]; then
        compile_shader "$shader" "$SHADER_OUT/$name.bin" "compute"
    fi
done

echo ""
echo "=== Shader compilation complete ==="
echo "Output directory: $SHADER_OUT"
