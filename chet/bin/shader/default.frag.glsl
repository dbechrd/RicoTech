#version 330 core

struct camera {
    vec3 position;
};

struct material {
    sampler2D diffuse;
    sampler2D specular;
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

uniform camera u_camera;
uniform material u_material;
uniform light u_light_point;

in vert_out {
    vec3 position;
    vec4 color;
    vec3 normal;
    vec2 uv;
} vert;

out vec4 color;

void main()
{
    // TODO: Specular texture doesn't need to be GL_RGBA. Use GL_RED.
    vec4 texel_diffuse = texture(u_material.diffuse, vert.uv);
    vec4 texel_specular = texture(u_material.specular, vert.uv);

    vec3 mat_diffuse = texel_diffuse.rgb;
    if (texel_diffuse.a == 0)
    {
        mat_diffuse = vert.color.rgb;
    }
    vec3 mat_specular = texel_specular.rgb;

    vec3 ambient = u_light_point.ambient * mat_diffuse;

    vec3 norm = normalize(vert.normal);
    vec3 light_vec = u_light_point.position - vert.position;
    float light_dist = length(light_vec);
    vec3 light_dir = normalize(light_vec);

    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = u_light_point.color * mat_diffuse * diff;

    vec3 specular = vec3(0);
    if (diff > 0.0)
    {
        vec3 eye_dir = normalize(u_camera.position - vert.position);

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
        specular = u_light_point.color * mat_specular * spec;
        // specular = mat_specular * spec;
    }

    float attenuation = 1.0 / (u_light_point.kc +
                               u_light_point.kl * light_dist +
                               u_light_point.kq * (light_dist * light_dist));

    // Ambient based on distance from light?
    //color.xyz = (ambient + diffuse + specular) * attenuation;
    // Ambient constant
    color = vec4(ambient + ((diffuse + specular) * attenuation), 1.0);

    // What is the point of this??
    //color.a = 1.0 + (vert.normal.x * 0.0000001); //texel_diffuse.a;
}