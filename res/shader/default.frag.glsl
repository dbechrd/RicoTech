#version 330 core

uniform vec3 u_view_pos;
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
    vec3 light_color = vec3(0.9, 0.8, 0.6);  // Orig ambient
    // vec3 light_color = vec3(0.5, 0.4, 0.3);  // Nice color
    // vec3 light_color = vec3(0.5, 1.0, 1.0);  // Test cyan
    ////////////////////////////////////////////////////////////////////////////

    float material_ambient = 0.2;
    float material_diffuse = 1.0;
    float material_specular = 0.6;

    // float material_ambient = 0.1;
    // float material_diffuse = 0.7;
    // float material_specular = 0.2;

    vec3 norm = normalize(vtx_normal);
    vec3 light_dir = normalize(light_pos - vtx_frag_pos);

    vec3 ambient = light_color * material_ambient;

    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = light_color * diff * material_diffuse;

    vec3 specular;
    if (diff > 0.0)
    {
        vec3 eye_dir = u_view_pos - vtx_frag_pos;

        // Phong specular
        // float shininess = 4;
        // vec3 reflect_dir = reflect(-light_dir, norm);
        // float spec = pow(max(dot(eye_dir, reflect_dir), 0.0), shininess);

        // Blinn-Phong specular
        float shininess = 64;
        vec3 halfway = normalize(light_dir + eye_dir);
        float spec = pow(max(dot(norm, halfway), 0.0), shininess);

        specular = light_color * spec * material_specular;
    }
    else
    {
        specular = vec3(0, 0, 0);
    }

    vec4 texel = texture(u_tex, vtx_uv);

    color = vec4(ambient + diffuse + specular, 1.0) * texel;
}