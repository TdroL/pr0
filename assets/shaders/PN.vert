#version 330

layout(location = 0) in vec3 vert_position;
layout(location = 2) in vec3 vert_normal;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

out vec3 position;
out vec3 normal;

void main()
{
	position = vert_position;
	normal = vert_normal;

	gl_Position = P * V * M * vec4(position, 1.0);
}
