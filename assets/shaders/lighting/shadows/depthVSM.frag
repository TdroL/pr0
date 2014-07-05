#version 330

layout(location = 0) out vec2 outColor;

in vec4 position;

// From http://fabiensanglard.net/shadowmappingVSM/index.php
void main()
{
	float depth = gl_FragCoord.z;

	float moment1 = depth;
	float moment2 = depth * depth;

	// Adjusting moments (this is sort of bias per pixel) using partial derivative
	float dx = dFdx(depth);
	float dy = dFdy(depth);
	moment2 += 0.25 * (dx * dx + dy * dy);

	outColor = vec2(moment1, moment2);
}