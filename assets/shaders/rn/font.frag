#version 440 core

in vec2 uv;

uniform vec3 color;
uniform sampler2D tex;
out vec4 outColor;

void main()
{
	outColor = vec4(color, texture(tex, uv).r);
}