#version 330

layout(location = 0) in vec3 vert_position;

uniform mat4 M, V, P;

out vec4 position;
out vec3 normal;

void main()
{
	gl_Position = P * V * M * vec4(vert_position, 1.0);
}
