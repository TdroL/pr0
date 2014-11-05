#version 330

layout(location = 0) out vec4 outColor;

in vec2 uv;

uniform sampler2D texSource;
uniform vec2 scale;

vec4 blurBox4(sampler2D tex, vec2 uv, vec2 scale);

void main()
{
	outColor = blurBox4(texSource, uv, scale);
}