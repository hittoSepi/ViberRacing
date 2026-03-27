$input a_position, a_normal
$output v_position, v_normal

#include <bgfx_shader.sh>

void main()
{
    vec4 worldPos = mul(u_model[0], vec4(a_position.xyz, 1.0));
    vec3 worldNormal = normalize(mul(u_model[0], vec4(a_normal.xyz, 0.0)).xyz);

    gl_Position = mul(u_viewProj, worldPos);
    v_position = worldPos.xyz;
    v_normal = worldNormal;
}
