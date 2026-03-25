#!/usr/bin/env python3
"""
Pack GLSL shaders into bgfx shader binary format (.bin) for OpenGL renderer.
Format: magic(4) + hashIn(4) + hashOut(4) + numUniforms(2) + uniforms + codeSize(4) + code + nul
"""
import struct
import os

def make_uniform(name: str, utype: int = 4, num: int = 1, reg_idx: int = 0, reg_cnt: int = 4) -> bytes:
    """Create a uniform entry. type 4 = Mat4."""
    buf = bytearray()
    name_bytes = name.encode('ascii')
    buf += struct.pack('B', len(name_bytes))
    buf += name_bytes
    buf += struct.pack('B', utype)    # UniformType (4=Mat4, 2=Vec4, 0=Sampler)
    buf += struct.pack('B', num)      # num
    buf += struct.pack('<H', reg_idx) # regIndex
    buf += struct.pack('<H', reg_cnt) # regCount
    buf += struct.pack('<H', 0)        # texInfo  (uint16, version >= 8)
    buf += struct.pack('<H', 0)        # texFormat (uint16, version >= 10)
    return bytes(buf)


def pack_shader(glsl: str, is_vertex: bool, uniforms: list[bytes]) -> bytes:
    magic = b'VSH\x0b' if is_vertex else b'FSH\x0b'
    code = glsl.encode('utf-8') + b'\x00'

    buf = bytearray()
    buf += magic
    buf += struct.pack('<I', 0)  # hashIn
    buf += struct.pack('<I', 0)  # hashOut
    buf += struct.pack('<H', len(uniforms))
    for u in uniforms:
        buf += u
    buf += struct.pack('<I', len(code))
    buf += code
    return bytes(buf)


# --- Shader sources (OpenGL 2.1 / GLSL 120 compatible, matching bgfx style) ---

vs_basic_glsl = """\
attribute vec3 a_position;
attribute vec4 a_color0;
varying vec4 v_color0;
uniform mat4 u_modelViewProj;
void main ()
{
  vec4 tmpvar_1;
  tmpvar_1.w = 1.0;
  tmpvar_1.xyz = a_position;
  gl_Position = (u_modelViewProj * tmpvar_1);
  v_color0 = a_color0;
}
"""

fs_basic_glsl = """\
varying vec4 v_color0;
void main ()
{
  gl_FragColor = v_color0;
}
"""

vs_textured_glsl = """\
attribute vec2 a_position;
attribute vec2 a_texcoord0;
attribute vec4 a_color0;
varying vec2 v_texcoord0;
varying vec4 v_color0;
uniform mat4 u_viewProj;
void main ()
{
  gl_Position = (u_viewProj * vec4(a_position, 0.0, 1.0));
  v_texcoord0 = a_texcoord0;
  v_color0 = a_color0;
}
"""

fs_textured_glsl = """\
varying vec2 v_texcoord0;
varying vec4 v_color0;
uniform sampler2D s_tex;
void main ()
{
  gl_FragColor = texture2D(s_tex, v_texcoord0) * v_color0;
}
"""

def main():
    out_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'assets', 'shaders')
    os.makedirs(out_dir, exist_ok=True)

    # uniform helpers
    u_mvp = make_uniform('u_modelViewProj', utype=4, num=1, reg_idx=0, reg_cnt=4)
    u_vp  = make_uniform('u_viewProj',      utype=4, num=1, reg_idx=0, reg_cnt=4)
    u_tex = make_uniform('s_tex',            utype=0, num=1, reg_idx=0, reg_cnt=1)

    shaders = [
        ('vs_basic.bin',    vs_basic_glsl,    True,  [u_mvp]),
        ('fs_basic.bin',    fs_basic_glsl,    False, []),
        ('vs_textured.bin', vs_textured_glsl, True,  [u_vp]),
        ('fs_textured.bin', fs_textured_glsl, False, [u_tex]),
    ]

    for name, source, is_vertex, uniforms in shaders:
        path = os.path.join(out_dir, name)
        data = pack_shader(source, is_vertex, uniforms)
        with open(path, 'wb') as f:
            f.write(data)
        print(f'  {name} ({len(data)} bytes)')

    print('Done - shaders packed for OpenGL renderer.')

if __name__ == '__main__':
    main()
