#version 330 core

vec2 depthPack(float depth)
{
	return vec2(depth, fract(depth * 256.0));
}

float depthUnpack(in vec2 source)
{
	return source.x + source.y / 256.0;
}