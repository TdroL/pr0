#version 330

layout(location = 0) in vec3 vert_position;
layout(location = 2) in vec3 vert_normal;

uniform mat4 M, V, P;
uniform mat3 N;

out vec3 position;
out vec3 normal;

void main()
{
	gl_Position = V * M * vec4(vert_position, 1.0);

	position = vec3(gl_Position);
	normal = N * vert_normal;

	gl_Position = P * gl_Position;
}
