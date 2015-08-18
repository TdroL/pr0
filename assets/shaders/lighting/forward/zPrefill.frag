#version 330 core

layout(location = 0) out vec2 outNormal;
layout(location = 1) out vec3 outDebug;

in vec4 position;
in vec3 normal;

vec2 normalEncode(vec3 n);

void main()
{
	outNormal = normalEncode(normalize(normal));
	outDebug = normalize(normal);
}