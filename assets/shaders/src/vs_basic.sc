$input a_position, a_color0
$output v_color0

#include <bgfx_shader.sh>

void main()
{
    vec4 pos = mul(u_model[0], vec4(a_position.xyz, 1.0));
    gl_Position = mul(u_viewProj, pos);
    v_color0 = a_color0;
}
