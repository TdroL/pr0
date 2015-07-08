#version 440 core

float frandom(vec2 co)
{
	return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

int irandom(ivec2 co)
{
	return (3 * co.x ^ co.y + co.x * co.y) * 7;
}

vec2 pack2(float source)
{
	return vec2(floor(source * 256.0) / 256.0, fract(source * 256.0));
}

float unpack2(in vec2 source)
{
	return source[0] + source[1] / 256.0;
}

vec3 pack3(float source)
{
	return vec3(floor(source * 256.0) / 256.0, fract(source * 256.0), fract(source * 256.0 * 256.0));
}

float unpack3(in vec3 source)
{
	return source[0] + source[1] / 256.0 + source[2] / (256.0 * 256.0);
}