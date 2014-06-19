#version 330

in vec2 uv;

uniform sampler2D texDS;

out vec4 outColor;

void main()
{
	outColor.rgb = texture(texDS, uv).rrr;
	outColor.a = 1.0;
}