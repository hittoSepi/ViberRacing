#version 450

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_texcoord0;
layout(location = 2) in vec4 a_color0;

layout(location = 0) out vec2 v_texcoord0;
layout(location = 1) out vec4 v_color0;

layout(set = 0, binding = 0) uniform Matrices {
    mat4 model;
    mat4 view;
    mat4 proj;
} u_mat;

void main() {
    vec4 pos = u_mat.model * vec4(a_position, 1.0);
    gl_Position = u_mat.proj * u_mat.view * pos;
    v_texcoord0 = a_texcoord0;
    v_color0 = a_color0;
}
