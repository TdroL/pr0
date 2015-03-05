#version 330

// From http://fabiensanglard.net/shadowmappingVSM/index.php

layout(location = 0) out vec4 outColor;

in vec2 uv;

uniform sampler2D texSource;
uniform vec2 scale;

vec4 blurGaussian7(sampler2D tex, vec2 uv, vec2 scale);
float esmLogBlur(sampler2D tex, vec2 uv, vec2 scale);

void main()
{
	outColor = blurGaussian7(texSource, uv, scale);
	outColor.b = esmLogBlur(texSource, uv, scale);
}