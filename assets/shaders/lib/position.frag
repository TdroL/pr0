#version 330

uniform mat4 invP;
uniform float zFar;
uniform float zNear;
uniform vec4 projectionInfo;

vec3 positionReconstruct(float depth, vec2 uv)
{
	vec4 position = invP * vec4(uv * 2.0 - 1.0, depth, 1.0);
	return (position.xyz / position.w);
}

float cameraZLinearize(float depth)
{
	return -(zNear * zFar) / ((zNear - zFar) * depth + zFar);
}

vec3 cameraPositionReconstruct(float depth, vec2 xy)
{
	float z = cameraZLinearize(depth);
	/**
	 * projectionInfo:
	 *  x = -2.0 / (width  * P[0].x)
	 *  y = -2.0 / (height * P[1].y)
	 *  z = (1.0 - P[2].x) / P[0].x
	 *  w = (1.0 + P[2].y) / P[1].y)
	 */
	return vec3((xy * projectionInfo.xy + projectionInfo.zw) * z, z);
}