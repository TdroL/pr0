#version 330

uniform mat4 invP;

vec3 reconstructPosition(float z, vec2 uv)
{
	vec4 position = invP * vec4(uv * 2.0 - 1.0, z, 1.0);
	return (position.xyz / position.w);
}