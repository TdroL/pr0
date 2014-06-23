#version 330

layout(location = 0) out vec2 outColor;

in vec4 position;

// From http://fabiensanglard.net/shadowmappingVSM/index.php
void main()
{
	float depth = position.z / position.w * 0.5 + 0.5;

	float moment1 = depth;
	float moment2 = depth * depth;

	vec2 d = vec2(dFdx(depth), dFdy(depth));

	moment2 += dot(d, d) * 0.25;

	outColor = vec2(moment1, moment2);
}