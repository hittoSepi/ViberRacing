$input v_dir

#include <bgfx_shader.sh>

uniform vec4 u_skyTop;
uniform vec4 u_skyBottom;
uniform vec4 u_horizonColor;
uniform vec4 u_sunDir;
uniform vec4 u_sunColor;
uniform vec4 u_atmoParams0;
uniform vec4 u_atmoParams1;

void main()
{
    vec3 viewDir = normalize(v_dir);
    vec3 sunDir = normalize(u_sunDir.xyz);

    float height = clamp(viewDir.y * 0.5 + 0.5, 0.0, 1.0);
    float zenith = pow(height, max(u_atmoParams0.w, 0.001));
    float horizon = pow(1.0 - abs(viewDir.y), max(u_atmoParams0.x, 0.001));

    vec3 sky = mix(u_skyBottom.rgb, u_skyTop.rgb, zenith);
    sky = mix(sky, u_horizonColor.rgb, horizon * u_atmoParams0.y);

    float sunAmount = max(dot(viewDir, sunDir), 0.0);
    float sunDisc = smoothstep(1.0 - u_sunDir.w, 1.0, sunAmount);
    float sunGlow = pow(sunAmount, u_atmoParams1.x) * u_atmoParams1.z
                  + pow(sunAmount, u_atmoParams1.y) * u_atmoParams1.w;

    sky += u_sunColor.rgb * (sunGlow * u_sunColor.a);
    sky = mix(sky, u_sunColor.rgb, sunDisc * min(1.0, u_sunColor.a * 1.5));

    float aerial = pow(1.0 - max(viewDir.y, 0.0), 2.0);
    sky = mix(sky, u_horizonColor.rgb, aerial * u_atmoParams0.z);

    gl_FragColor = vec4(clamp(sky, 0.0, 1.0), 1.0);
}
