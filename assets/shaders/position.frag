#version 330 core

in vec3 position;

out vec4 out_color;

void main()
{
	out_color = vec4(position * 0.5 + 1.0, 1.0); //color;
}