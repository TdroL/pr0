#version 330

layout(location = 0) out vec2 outColor;

in vec4 position;

vec2 vsmMoments(float depth);

// From http://fabiensanglard.net/shadowmappingVSM/index.php
void main()
{
	outColor = vsmMoments(gl_FragCoord.z);
}