#version 330 core

in vs_out {
    vec3 P;
    vec3 N;
    vec2 UV;
    vec4 color;
} vertex;

out vec4 frag_color;

const float PI = 3.14159265359;
const float gamma = 1.0;

struct Camera {
    vec3 P;
};

// NOTE: Using SRGB_ALPHA in texture_upload, so don't need this atm
//#define mtl_albedo    pow(texture(material.tex0, vertex.UV).rgb, vec3(gamma))
//#define mtl_opacity   texture(material.tex0, vertex.UV).a
//#define mtl_metallic  pow(texture(material.tex1, vertex.UV).r, gamma)
//#define mtl_roughness pow(texture(material.tex1, vertex.UV).g, gamma)
//#define mtl_ao        pow(texture(material.tex1, vertex.UV).b, gamma)

#define mtl_albedo    texture(material.tex0, vertex.UV).rgb
#define mtl_opacity   texture(material.tex0, vertex.UV).a
#define mtl_metallic  texture(material.tex1, vertex.UV).r
#define mtl_roughness texture(material.tex1, vertex.UV).g
#define mtl_ao        texture(material.tex1, vertex.UV).b
#define mtl_emission  texture(material.tex2, vertex.UV)

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

struct PointLight {
    vec3 P;
    vec3 color;
    float intensity;
};

uniform Camera camera;
uniform Material material;
uniform PointLight light;

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
    vec3 N = normalize(vertex.N);
    vec3 V = normalize(camera.P - vertex.P);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, mtl_albedo, mtl_metallic);

    vec3 L0 = vec3(0.0);
    for (int i = 0; i < 1; ++i)
    {
        vec3 L = normalize(light.P - vertex.P);
        vec3 H = normalize(V + L);
        float dist = length(light.P - vertex.P);
        float attenuation = light.intensity;// / (dist * dist);
        vec3 radiance = light.color * attenuation;

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

        L0 += (kD * mtl_albedo / PI + specular) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.01) * mtl_albedo * mtl_ao;
    vec3 color = ambient + L0;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    color = mix(color, mtl_emission.rgb, 0.8 * step(0.01, mtl_emission.a));

    frag_color = vec4(color, 1.0);
}