#version 330

in vec2 uv;

uniform vec3 color;
uniform sampler2D tex0;
out vec4 out_color;

void main()
{
	out_color = texture(tex0, uv);
}