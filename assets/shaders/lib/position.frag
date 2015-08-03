#version 330 core

uniform mat4 invP;
uniform vec4 projectionInfo;

vec3 positionReconstruct(float depth, vec2 uv)
{
	vec4 position = invP * vec4(uv * 2.0 - 1.0, depth, 1.0);
	return (position.xyz / position.w);
}

float linearizeZ(float depth, float zFar, float zNear)
{
	return -(zNear * zFar) / ((zNear - zFar) * depth + zFar);
}