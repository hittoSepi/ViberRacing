#version 450

layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_texcoord0;
layout(location = 2) in vec4 a_color0;

layout(location = 0) out vec2 v_texcoord0;
layout(location = 1) out vec4 v_color0;

layout(set = 0, binding = 0) uniform vec4 u_viewRect;
layout(set = 0, binding = 1) uniform vec4 u_tint;

void main() {
    vec2 pos = a_position;
    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
    v_texcoord0 = a_texcoord0;
    v_color0 = a_color0;
}
