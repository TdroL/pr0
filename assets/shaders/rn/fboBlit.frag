#version 440 core

in vec2 uv;

uniform sampler2D texSource;
out vec4 outColor;

void main()
{
	outColor = texture(texSource, uv);
}