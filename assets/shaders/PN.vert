#version 330

layout(location = 0) in vec3 vertPosition;
layout(location = 2) in vec3 vertNormal;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

out vec4 position;
out vec3 normal;

void main()
{
	gl_Position = V * M * vec4(vertPosition, 1.0);

	position = gl_Position;
	normal = mat3(V * M) * vertNormal;

	gl_Position = P * gl_Position;
}
