#version 330

// From http://fabiensanglard.net/shadowmappingVSM/index.php

/////////////////////////////////////////////////
// 7x1 gaussian blur fragment shader
/////////////////////////////////////////////////

layout(location = 0) out vec4 outColor;

in vec2 uv;

uniform sampler2D texSource;
uniform vec2 scale;

vec3 blurGaussian(sampler2D tex, vec2 uv, vec2 scale);
float esmLogBlur(sampler2D tex, vec2 uv, vec2 scale);

void main()
{
	outColor.rgb = blurGaussian(texSource, uv, scale);
	outColor.b = esmLogBlur(texSource, uv, scale);
	outColor.a = 1.0;
}