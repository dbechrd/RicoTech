#version 330 core

in vs_out {
    vec3 P;
    vec2 UV;
    vec4 color;
} vertex;

out vec4 frag_color;

uniform vec4 u_color;
uniform bool u_grayscale;
uniform sampler2D u_tex;

void main()
{
	vec4 texel = texture(u_tex, vertex.UV);
	
	vec4 color;
	if (u_grayscale)
	{
		color = vec4(vec3(1.0), texel.r);
	}
	else
	{
		color = mix(vertex.color, texel, step(0.01, texel.a));
	}
	frag_color = color * u_color;
}

/*
void main()
{
	vec4 texel = texture(u_tex, vertex.UV);
	
	vec4 fullcolor = mix(vertex.color, texel, step(0.01, texel.a));
	vec4 grayscale = vec4(vec3(1.0), texel.r);

	vec4 color = mix(fullcolor, grayscale, u_grayscale);
	color = color * u_color;

	frag_color = color;
}
*/