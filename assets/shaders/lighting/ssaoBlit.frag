#version 330

layout(location = 0) out vec4 outColor;

in vec2 uv;

uniform sampler2D texSource;

void main()
{
	outColor = vec4(0.0);
	outColor.a = texture(texSource, uv).r;
}