#version 330

// From http://fabiensanglard.net/shadowmappingVSM/index.php

/////////////////////////////////////////////////
// 7x1 gaussian blur fragment shader
/////////////////////////////////////////////////

in vec2 uv;

uniform vec2 scale;
uniform sampler2D texSource;

out vec4 outColor;

void main()
{
	vec4 outColor = vec4(0.0);

	outColor += texture(texSource, uv + vec2(-3.0 * scale.x, -3.0 * scale.y)) * 0.015625;
	outColor += texture(texSource, uv + vec2(-2.0 * scale.x, -2.0 * scale.y)) * 0.09375;
	outColor += texture(texSource, uv + vec2(-1.0 * scale.x, -1.0 * scale.y)) * 0.234375;
	outColor += texture(texSource, uv + vec2( 0.0 * scale.x,  0.0 * scale.y)) * 0.3125;
	outColor += texture(texSource, uv + vec2( 1.0 * scale.x,  1.0 * scale.y)) * 0.234375;
	outColor += texture(texSource, uv + vec2( 2.0 * scale.x,  2.0 * scale.y)) * 0.09375;
	outColor += texture(texSource, uv + vec2( 3.0 * scale.x,  3.0 * scale.y)) * 0.015625;
}