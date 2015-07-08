#version 440 core

layout(location = 0) in vec2 vertPosition;
layout(location = 1) in vec2 vertUV;

uniform mat4 M;
out vec2 uv;

void main()
{
	gl_Position = M * vec4(vertPosition.xy, 0.0, 1.0);

	uv = vertUV.xy;
}