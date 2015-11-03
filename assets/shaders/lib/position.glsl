#version 440 core

vec3 positionReconstruct(float depth, vec2 uv, mat4 invP)
{
	vec4 position = invP * vec4(uv * 2.0 - 1.0, depth, 1.0);
	return (position.xyz / position.w);
}

vec4 positionReconstruct2(sampler2D texDepth, vec2 uv, mat4 invP)
{
	float depth = 1.0 - texture(texDepth, uv).r;
	vec4 position = invP * vec4(uv * 2.0 - 1.0, depth, 1.0);
	return vec4((position.xyz / position.w), depth);
}

float linearizeZ(float depth, float zFar, float zNear)
{
	return -(zNear * zFar) / ((zNear - zFar) * depth + zFar);
}