$input a_position, a_normal
$output v_color0

#include <bgfx_shader.sh>

void main()
{
    vec4 worldPos = mul(u_model[0], vec4(a_position.xyz, 1.0));
    vec3 worldNormal = normalize(a_normal.xyz);
    vec3 lightDir = normalize(vec3(0.35, 0.8, 0.25));
    float ndotl = max(dot(worldNormal, lightDir), 0.0);
    vec3 baseColor = vec3(0.62, 0.67, 0.74);
    vec3 litColor = baseColor * (0.30 + ndotl * 0.70);

    gl_Position = mul(u_viewProj, worldPos);
    v_color0 = vec4(litColor, 1.0);
}
