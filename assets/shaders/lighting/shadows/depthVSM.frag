#version 330

layout(location = 0) out vec3 outColor;

in vec4 position;

vec2 vsmMoments(float depth);

void main()
{
	outColor.rg = vsmMoments(gl_FragCoord.z);
	outColor.b = exp(gl_FragCoord.z);
}