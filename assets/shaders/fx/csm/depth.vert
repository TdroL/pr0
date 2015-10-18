#version 440 core

layout(location = 0) in vec3 vertPosition;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

out vec4 position;
// out vec3 normal;

void main()
{
	mat4 P2 = P;
	// P2[3].z *= 2.0;
	position = V * M * vec4(vertPosition, 1.0);
	gl_Position = P * position;
}
