#version 440 core

layout(location = 0) in vec3 vertPosition;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

out vec4 position;

void main()
{
	position = V * M * vec4(vertPosition, 1.0);

	gl_Position = P * position;
}
