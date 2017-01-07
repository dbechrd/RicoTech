#version 330 core

struct material {
    sampler2D diff;
    sampler2D spec;
    float shiny;
};

uniform vec3 u_view_pos;
uniform material u_material;

in vec4 vtx_col;
in vec3 vtx_normal;
in vec2 vtx_uv;
in vec3 vtx_frag_pos;

out vec4 color;

void main()
{
    ////////////////////////////////////////////////////////////////////////////
    // HACK: Make proper uniforms
    vec3 light_pos = vec3(1, 6, 0);
    vec3 light_color = vec3(0.9, 0.8, 0.6);  // Orig ambient
    // vec3 light_color = vec3(0.5, 0.4, 0.3);  // Nice color
    // vec3 light_color = vec3(0.5, 1.0, 1.0);  // Test cyan
    ////////////////////////////////////////////////////////////////////////////

    vec4 mat_diffuse = texture(u_material.diff, vtx_uv);
    vec3 mat_ambient = mat_diffuse.xyz;
    vec3 mat_specular = texture(u_material.spec, vtx_uv).rgb;

    float k_ambient = 0.1;
    float k_diffuse = 0.7;
    float k_specular = 0.7;

    vec3 light_ambient = k_ambient * light_color;
    vec3 light_diffuse = k_diffuse * light_color;
    vec3 light_specular = k_specular * vec3(1.0, 1.0, 0.2);

    vec3 ambient = light_ambient * mat_ambient;

    vec3 norm = normalize(vtx_normal);
    vec3 light_dir = normalize(light_pos - vtx_frag_pos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = light_diffuse * mat_diffuse.xyz * diff;

    vec3 specular;
    if (diff > 0.0)
    {
        vec3 eye_dir = normalize(u_view_pos - vtx_frag_pos);

        // Phong specular
        // vec3 reflect_dir = reflect(-light_dir, norm);
        // float spec = pow(max(dot(eye_dir, reflect_dir), 0.0),
        //                  u_material.shiny);

        // Blinn-Phong specular
        vec3 halfway = normalize(light_dir + eye_dir);
        float spec = pow(max(dot(norm, halfway), 0.0), u_material.shiny * 128);

        specular = light_specular * mat_specular * spec;
    }
    else
    {
        specular = vec3(0, 0, 0);
    }

    color = vec4(ambient + diffuse + specular, mat_diffuse.a);
}