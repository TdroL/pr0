#version 330 core

in vec2 uv;

// uniform sampler2DArray texSource;
uniform sampler2DArray texSource;
uniform uint layer;

out vec4 outColor;

void main()
{
	outColor.rgb = vec3(pow(texture(texSource, vec3(uv, float(layer))).r, 6.0));
	outColor.a = 1.0;
}