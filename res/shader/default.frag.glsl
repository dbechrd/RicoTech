#version 330 core

uniform vec3 u_view_pos;
uniform vec3 u_ambient;
uniform sampler2D u_tex;

in vec4 vtx_col;
in vec3 vtx_normal;
in vec2 vtx_uv;
in vec3 vtx_frag_pos;

out vec4 color;

void main()
{
    ////////////////////////////////////////////////////////////////////////////
    // HACK: Make proper uniforms
    vec3 light_pos = vec3(1, 8, 0);
    // vec3 light_color = vec3(0.5, 0.4, 0.3);
    vec3 light_color = vec3(0.5, 0.4, 0.4);
    ////////////////////////////////////////////////////////////////////////////

    vec3 norm = normalize(vtx_normal);
    vec3 light_dir = normalize(light_pos - vtx_frag_pos);

    float ambient_factor = 0.1;
    vec3 ambient = ambient_factor * u_ambient;

    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * light_color;

    float specular_factor = 0.3f;
    vec3 view_dir = u_view_pos - vtx_frag_pos;
    vec3 reflect_dir = reflect(-light_dir, norm);
    float shininess = 1;
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), shininess);
    vec3 specular = specular_factor * spec * light_color;

    vec4 texel = texture(u_tex, vtx_uv);

    color = vec4(ambient + diffuse + specular, 1.0) * texel;
}