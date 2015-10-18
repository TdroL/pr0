#version 440 core

layout(location = 0) out vec4 outColor;

in vec2 uv;

uniform sampler2D texColor;

void main()
{
	outColor.rgb = texture(texColor, uv).rgb;
	outColor.a = 1.0;
}