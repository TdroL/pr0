#version 440 core

layout(location = 0) in vec3 vertPosition;
layout(location = 2) in vec3 vertNormal;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

out vec4 position;
out vec4 positionM;
out vec3 normal;
out vec3 normalM;

void main()
{
	positionM = M * vec4(vertPosition, 1.0);
	position = V * positionM;

	normalM = mat3(M) * vertNormal;
	normal = mat3(V * M) * vertNormal;

	gl_Position = P * position;
}
