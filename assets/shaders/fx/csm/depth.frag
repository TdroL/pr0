#version 440 core

layout(location = 0) out vec4 outColor;

in vec4 position;

uniform bool writeColor;

vec2 vsmMoments(float depth);

void main()
{
	if (writeColor)
	{
		outColor.rg = vsmMoments(gl_FragCoord.z);
		outColor.b = exp(gl_FragCoord.z);
		outColor.a = gl_FragCoord.z;
	}
	else
	{
		outColor = vec4(1.0);
	}
}