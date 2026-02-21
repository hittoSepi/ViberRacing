#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
SHADER_SRC="$PROJECT_ROOT/assets/shaders"
SHADER_OUT="$PROJECT_ROOT/assets/shaders/compiled"
BGFX_SHADERC="${BGFX_SHADERC:-shaderc}"

mkdir -p "$SHADER_OUT"

PLATFORM="linux"
PROFILE="120"

compile_shader() {
    local input="$1"
    local output="$2"
    local type="$3"
    
    echo "Compiling $input -> $output"
    
    "$BGFX_SHADERC" \
        --platform "$PLATFORM" \
        --profile "$PROFILE" \
        --type "$type" \
        -i "$SHADER_SRC" \
        -o "$output" \
        -f "$input" \
        --bin2c
}

for shader in "$SHADER_SRC"/*.sc; do
    filename=$(basename "$shader")
    name="${filename%.*}"
    
    if [[ "$name" == vs_* ]]; then
        compile_shader "$shader" "$SHADER_OUT/$name.bin" "vertex"
    elif [[ "$name" == fs_* ]]; then
        compile_shader "$shader" "$SHADER_OUT/$name.bin" "fragment"
    elif [[ "$name" == cs_* ]]; then
        compile_shader "$shader" "$SHADER_OUT/$name.bin" "compute"
    fi
done

echo "Shader compilation complete"
