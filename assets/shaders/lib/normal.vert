#version 330

vec2 encodeNormal(vec3 n)
{
	float scale = 1.7777;
	return (n.xy / (n.z+1)) / scale * 0.5 + 0.5;
}