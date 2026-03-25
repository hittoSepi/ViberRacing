$input a_position
$output v_dir

#include <bgfx_shader.sh>

void main()
{
    // Position at infinity (no translation)
    vec4 pos = mul(u_model[0], a_position);
    
    // Remove translation from view, keep only rotation
    mat4 viewNoTrans = u_view;
    viewNoTrans[3][0] = 0.0;
    viewNoTrans[3][1] = 0.0;
    viewNoTrans[3][2] = 0.0;
    
    gl_Position = mul(u_proj, mul(viewNoTrans, pos));
    
    // Direction for cubemap sampling (from center to vertex)
    v_dir = a_position.xyz;
}
