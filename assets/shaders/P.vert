#version 330

layout(location = 0) in vec3 vertPosition;

uniform mat4 M, V, P;

out vec4 position;
out vec3 normal;

void main()
{
	gl_Position = P * V * M * vec4(vertPosition, 1.0);
}
