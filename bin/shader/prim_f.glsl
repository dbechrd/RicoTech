#version 330 core

in vs_out {
    vec3 P;
    vec2 UV;
    vec4 color;
} vertex;

out vec4 frag_color;

uniform vec4 color;
uniform sampler2D tex;

void main()
{
	vec4 color = color;
	vec4 texel = texture(tex, vertex.UV);

	//vec4 texel_invert = vec4(vec3(1.0f) - texel.rgb, texel.a);
	//float selected = float(color.r == 0.25f &&
	//					   color.g == 0.25f &&
	//					   color.b == 0.25f);
	//texel = mix(texel, texel_invert, selected);

	float has_color = step(0.001f, max(texel.r, max(texel.g, texel.b)));
    frag_color = mix(color, texel, texel.a * has_color);
}