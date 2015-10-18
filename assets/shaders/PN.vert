#version 440 core

layout(location = 0) in vec3 vertPosition;
layout(location = 2) in vec3 vertNormal;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

out vec4 position;
out vec3 normal;

void main()
{
	position = V * M * vec4(vertPosition, 1.0);
	normal = mat3(V * M) * vertNormal;

	gl_Position = P * position;
}
