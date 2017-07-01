#version 330 core

struct material {
    sampler2D diff;
    sampler2D spec;
    float shiny;
};

// Point light
struct light {
    vec3 position;

    // Color
    vec3 ambient;
    vec3 color;

    // Attenuation
    float kc;  // Constant
    float kl;  // Linear
    float kq;  // Quadratic
};

uniform vec3 u_view_pos;
uniform material u_material;
uniform light u_light;

in vtx_out {
    vec4 col;
    vec3 normal;
    vec2 uv;
    vec3 frag_pos;
} vtx;

out vec4 color;

void main()
{
    // TODO: Specular texture doesn't need to be GL_RGBA. Use GL_RED.
    vec4 texel_diffuse = texture(u_material.diff, vtx.uv);
    vec4 texel_specular = texture(u_material.spec, vtx.uv);

    vec3 mat_diffuse = texel_diffuse.rgb;
    vec3 mat_specular = texel_specular.rgb;

    vec3 ambient = u_light.ambient * mat_diffuse;

    vec3 norm = normalize(vtx.normal);

    vec3 light_vec = u_light.position - vtx.frag_pos;
    float light_dist = length(light_vec);
    vec3 light_dir = normalize(light_vec);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = u_light.color * mat_diffuse * diff;

    vec3 specular;
    if (diff > 0.0)
    {
        vec3 eye_dir = normalize(u_view_pos - vtx.frag_pos);

        // Phong specular
        // vec3 reflect_dir = reflect(-light_dir, norm);
        // float spec = pow(max(dot(eye_dir, reflect_dir), 0.0),
        //                  u_material.shiny);

        // Blinn-Phong specular
        vec3 halfway = normalize(light_dir + eye_dir);
        float spec = pow(max(dot(norm, halfway), 0.0), u_material.shiny * 128);

        // TODO: Make this a material property
        //       Fresnel zero = Fresnel effect at dead center (direct ray)
        float fresZero = 0.018;
        float base = 1 - dot(eye_dir, halfway);
        float exponential = pow(base, 5.0);
        float fresnel = fresZero + exponential * (1.0 - fresZero);
        spec *= fresnel;

        // Light decides specular color? How to handle metallic reflections?
        specular = u_light.color * mat_specular * spec;
        // specular = mat_specular * spec;
    }
    else
    {
        specular = vec3(0, 0, 0);
    }

    float attenuation = 1.0 / (u_light.kc +
                               u_light.kl * light_dist +
                               u_light.kq * (light_dist * light_dist));

    vec3 gammaColor;

    // Ambient based on distance from light?
    //gammaColor = (ambient + diffuse + specular) * attenuation;
    // Ambient constant
    gammaColor = ambient + (diffuse + specular) * attenuation;

    // Gamma correct
    // TODO: Don't apply gamma correction to specular maps! (Or.. just get rid
    //       of specular maps altogether).
    color.rgb = pow(gammaColor.rgb, vec3(1/2.2));
    //color.rgb = gammaColor.rgb;

    // What is the point of this??
    color.a = 1.0 + (vtx.normal.x * 0.0000001); //texel_diffuse.a;
}