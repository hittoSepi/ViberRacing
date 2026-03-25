#version 450

layout(location = 0) in vec2 v_texcoord0;
layout(location = 1) in vec4 v_color0;
layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 1) uniform sampler2D s_tex;

void main() {
    fragColor = texture(s_tex, v_texcoord0) * v_color0;
}
