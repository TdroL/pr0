#version 440 core

layout(location = 0) out float outColor;

in vec2 uv;

uniform sampler2D texDepth;
uniform vec3 clipInfo;

void main()
{
	float depth = 1.0 - texelFetch(texDepth, ivec2(gl_FragCoord.xy), 0).r;

	outColor = clipInfo[0] / (clipInfo[1] * depth + clipInfo[2]);
}