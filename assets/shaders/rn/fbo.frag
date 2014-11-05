#version 330

in vec2 uv;

uniform sampler2D tex0;
out vec4 outColor;

void main()
{
	outColor = texture(tex0, uv);
	outColor.a = 1.0;
}