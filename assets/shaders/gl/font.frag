#version 330

in vec2 uv;

uniform vec3 color;
uniform sampler2D tex;
out vec4 out_color;

void main()
{
	out_color = vec4(color, texture(tex, uv).r);
}