$input v_position, v_normal

#include <bgfx_shader.sh>

uniform vec4 u_tintColor;
uniform vec4 u_previewLightDir;
uniform vec4 u_previewViewPos;

void main()
{
    vec3 n = normalize(v_normal);
    vec3 l = normalize(-u_previewLightDir.xyz);
    vec3 v = normalize(u_previewViewPos.xyz - v_position);
    vec3 h = normalize(l + v);

    float ambient = u_previewLightDir.w;
    float diffuse = max(dot(n, l), 0.0);
    float specular = pow(max(dot(n, h), 0.0), 24.0) * u_previewViewPos.w;
    float rim = pow(1.0 - max(dot(n, v), 0.0), 2.0) * 0.18;

    vec3 lit = u_tintColor.rgb * (ambient + diffuse * 0.78 + rim) + vec3(specular);
    gl_FragColor = vec4(lit, u_tintColor.a);
}
