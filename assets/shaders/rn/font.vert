#version 330 core

layout(location = 0) in vec4 vertPosition;

out vec2 uv;

void main()
{
	gl_Position = vec4(vertPosition.xy, 0.0, 1.0);

	uv = vertPosition.zw;
}