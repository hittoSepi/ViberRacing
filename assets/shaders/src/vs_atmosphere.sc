$input a_position
$output v_dir

#include <bgfx_shader.sh>

void main()
{
    gl_Position = vec4(a_position.xy, 1.0, 1.0);

    vec4 clip = vec4(a_position.xy, 1.0, 1.0);
    vec4 world = mul(u_invViewProj, clip);
    v_dir = normalize(world.xyz / world.w);
}
