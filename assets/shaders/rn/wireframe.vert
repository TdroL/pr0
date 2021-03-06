#version 440 core

layout(location = 0) in vec3 vertPosition;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

void main()
{
	gl_Position = P * V * M * vec4(vertPosition, 1.0);
}
