#version 440 core

layout(location = 0) in vec2 vertPosition;
layout(location = 1) in vec2 vertUV;

out vec2 uv;

void main()
{
	uv = vertUV.xy;

	gl_Position = vec4(vertPosition.xy, 0.0, 1.0);
}