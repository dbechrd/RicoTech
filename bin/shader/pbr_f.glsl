#version 330 core

#define ERR_UNKNOWN_LIGHT_TYPE vec4(1.0, 0.0, 1.0, 1.0) // Magenta

in vs_out {
    vec3 P;
    vec2 UV;
    vec4 color;
    vec3 N;
    vec4 light_space;
} vertex;

out vec4 frag_color;

const float PI = 3.14159265359;
const float gamma = 1.0;

uniform float time;

struct Camera {
    vec3 P;
};
uniform Camera camera;

// NOTE: Using SRGB_ALPHA in texture_upload, so don't need this atm
#if 0
#  define mtl_albedo    pow(texture(material.tex0, vertex.UV).rgb, vec3(gamma))
#  define mtl_opacity   texture(material.tex0, vertex.UV).a
#  define mtl_metallic  pow(texture(material.tex1, vertex.UV).r, gamma)
#  define mtl_roughness pow(texture(material.tex1, vertex.UV).g, gamma)
#  define mtl_ao        pow(texture(material.tex1, vertex.UV).b, gamma)
#  define mtl_emission  pow(texture(material.tex2, vertex.UV).rgb, vec3(gamma))
#  define mtl_emit      step(0.01, texture(material.tex2, vertex.UV).a)
#else
#  define mtl_albedo    texture(material.tex0, vertex.UV).rgb
#  define mtl_opacity   texture(material.tex0, vertex.UV).a
#  define mtl_metallic  texture(material.tex1, vertex.UV).r
#  define mtl_roughness texture(material.tex1, vertex.UV).g
#  define mtl_ao        texture(material.tex1, vertex.UV).b
#  define mtl_emission  texture(material.tex2, vertex.UV).rgb
#  define mtl_emit      step(0.01, texture(material.tex2, vertex.UV).a)
#endif

struct Material {
    // rgb: metallic ? specular.rgb : albedo.rgb
    //   a: metallic ?            1 : opacity
    sampler2D tex0;

    // r: metallic
    // g: roughness
    // b: ao
    // a: UNUSED
    sampler2D tex1;

    // rgb: emission color
    //   a: UNUSED
    sampler2D tex2;
};
uniform Material material;

#define LIGHT_AMBIENT       0
#define LIGHT_DIRECTIONAL   1
#define LIGHT_POINT         2
#define LIGHT_SPOT          3

#define NUM_LIGHT_DIR 1
#define NUM_LIGHT_POINT 4

struct Light {
    int type;
    bool on;
    vec3 col;
    vec3 pos;
    float intensity;
    vec3 dir;
};
uniform Light lights[NUM_LIGHT_DIR + NUM_LIGHT_POINT];

uniform vec2 near_far;

// Note: lights = [[directional lights], [point lights]]
#define TEXTURE_IDX i
#define CUBEMAP_IDX i - NUM_LIGHT_DIR

uniform sampler2D shadow_textures[NUM_LIGHT_DIR];
uniform samplerCube shadow_cubemaps[NUM_LIGHT_POINT];

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main()
{
    vec4 debug_color = vec4(0.0);

    vec3 N = normalize(vertex.N);
    vec3 V = normalize(camera.P - vertex.P);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, mtl_albedo, mtl_metallic);

    vec3 L0 = vec3(0.0);
    for (int i = 0; i < NUM_LIGHT_DIR + NUM_LIGHT_POINT; ++i)
    {
        if (!lights[i].on)
            continue;

        vec3 fragToLight;
        float shadow_map_depth;
        float dist;
        float attenuation;

        if (lights[i].type == LIGHT_DIRECTIONAL)
        {
            fragToLight = -lights[i].dir;

            vec3 projCoords = vertex.light_space.xyz / vertex.light_space.w;
            projCoords = projCoords * 0.5 + 0.5;
            shadow_map_depth = texture(shadow_textures[TEXTURE_IDX],
                                       projCoords.xy).r;

            dist = projCoords.z;
            attenuation = lights[i].intensity;
            //debug_color = vec4(projCoords.xy, 0.0, 1.0);
        }
        else if (lights[i].type == LIGHT_POINT)
        {
            fragToLight = lights[i].pos - vertex.P;

            shadow_map_depth = texture(shadow_cubemaps[CUBEMAP_IDX],
                                       -fragToLight).r;
            shadow_map_depth *= near_far.y;

            dist = length(fragToLight);
            attenuation = lights[i].intensity / (dist * dist);
        }
        else
        {
            debug_color = ERR_UNKNOWN_LIGHT_TYPE;
        }

        float bias = 0.0; //0.01;
        float darkness = 0.9; //0.75;
        float shadow = (dist - bias > shadow_map_depth) ? darkness : 0.0;

        //debug_color = vec4(vec3(shadow_map_depth), 1.0);
        //debug_color = vec4(vec3(dist), 1.0);

        vec3 radiance = lights[i].col * attenuation;

        vec3 L = normalize(fragToLight);
        vec3 H = normalize(V + L);

        float D = DistributionGGX(N, H, mtl_roughness);
        float G = GeometrySmith(N, V, L, mtl_roughness);
        vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - mtl_metallic;

        float NdotL = max(dot(N, L), 0.0);

        vec3 nom = D * F * G;
        float denom = 4.0 * max(dot(N, V), 0.0) * NdotL;
        vec3 specular = nom / max(denom, 0.001);

        L0 += (kD * mtl_albedo / PI + specular) * radiance * NdotL * (1.0 - shadow);
    }

    vec3 ambient = vec3(0.01) * mtl_albedo * mtl_ao;
    vec3 color = ambient + L0;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    vec3 emission = mix(vec3(0), mtl_emission, mtl_emit);
    color = color + emission;

    // TODO: Make this uniform int debug_lightmap_idx = -1
    // Render lightmap
    //int i = 0;
    //float mapDepth = texture(lightmaps[i], vertex.P - lights[i].pos).r;
    //color = vec3(mapDepth) + (color * 0.000001);

    //color *= 0.000001;
    //color = normalize(lights[0].dir) + (color * 0.000001);
    //color = vec3(1.0, 0.0, 0.0) + (color * 0.000001);

    frag_color = mix(vec4(color, mtl_opacity), debug_color, debug_color.a > 0.0);
}