#!/usr/bin/env python3
"""
Simple shader compiler wrapper for bgfx.
Requires shaderc to be built or available in PATH.
"""

import os
import sys
import subprocess
import glob

BGFX_DIR = os.path.join(os.path.dirname(__file__), "..", "third_party", "bgfx")
BGFX_SRC = os.path.join(BGFX_DIR, "src")
SHADER_DIR = os.path.join(os.path.dirname(__file__), "..", "assets", "shaders")
SRC_DIR = os.path.join(SHADER_DIR, "src")
OUT_DIR = SHADER_DIR

# Shader type mapping
SHADER_TYPES = {
    "vs_": "vertex",
    "fs_": "fragment",
    "cs_": "compute",
}

# Platform settings
PLATFORMS = {
    "linux": ("linux", "spirv"),
    "windows": ("windows", "dx11"),
    "darwin": ("osx", "metal"),
}

def find_shaderc():
    """Find shaderc executable."""
    # Check common locations
    locations = [
        os.path.join(BGFX_DIR, "tools"),
        os.path.join(BGFX_DIR, ".build", "shaderc"),
        os.path.join(BGFX_DIR, "tools", "shaderc", "shaderc"),
        "/usr/bin/shaderc",
        "/usr/local/bin/shaderc",
    ]
    for loc in locations:
        if os.path.isfile(loc) and os.access(loc, os.X_OK):
            return loc
    return None

def compile_shader(shaderc, shader_file, output_file, shader_type):
    """Compile a single shader."""
    platform, profile = PLATFORMS.get(sys.platform, ("linux", "spirv"))
    
    cmd = [
        shaderc,
        "-f", shader_file,
        "-o", output_file,
        "--type", shader_type,
        "--platform", platform,
        "-p", profile,
        "-i", BGFX_SRC,
        "--varyingdef", os.path.join(SRC_DIR, "varying.def.sc"),
    ]
    
    print(f"Compiling {os.path.basename(shader_file)}...")
    result = subprocess.run(cmd, capture_output=True, text=True)
    
    if result.returncode != 0:
        print(f"ERROR: Failed to compile {shader_file}")
        print(result.stderr)
        return False
    
    print(f"  -> {output_file}")
    return True

def main():
    shaderc = find_shaderc()
    if not shaderc:
        print("ERROR: shaderc not found!")
        print("Please build bgfx tools first:")
        print("  cd third_party/bgfx && make shaderc")
        return 1
    
    print(f"Using shaderc: {shaderc}")
    
    # Find all .sc files
    shader_files = glob.glob(os.path.join(SRC_DIR, "*.sc"))
    if not shader_files:
        print(f"No shader files found in {SRC_DIR}")
        return 1
    
    success = True
    for shader_file in shader_files:
        basename = os.path.basename(shader_file)
        if basename == "varying.def.sc":
            continue
        
        # Determine shader type from prefix
        shader_type = None
        for prefix, stype in SHADER_TYPES.items():
            if basename.startswith(prefix):
                shader_type = stype
                break
        
        if not shader_type:
            print(f"WARNING: Unknown shader type for {basename}, skipping")
            continue
        
        output_file = os.path.join(OUT_DIR, basename.replace(".sc", ".bin"))
        
        if not compile_shader(shaderc, shader_file, output_file, shader_type):
            success = False
    
    if success:
        print("\nAll shaders compiled successfully!")
        return 0
    else:
        print("\nSome shaders failed to compile!")
        return 1

if __name__ == "__main__":
    sys.exit(main())
