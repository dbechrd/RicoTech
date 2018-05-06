#version 330 core

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

layout(location = 0) in vec3 vert_pos;

void main()
{
	//mat4 cam_inv = inverse(u_view);
	//vec3 camera_pos = u_view[3].xyz;
	//vec3 vpos = vert_pos * distance(camera_pos, vert_pos) / 1000.0f;
	//gl_Position = u_proj * u_view * u_model * vec4(vpos, 1.0f);

    gl_Position = u_proj * u_view * u_model * vec4(vert_pos, 1.0f);
}
