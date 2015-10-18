#version 440 core

layout(location = 0) out float outColor;

in vec2 uv;

uniform sampler2D texZ;
uniform int prevLevel;

void main()
{
	/**/
	ivec2 coord = ivec2(gl_FragCoord.xy);

	coord = coord * 2 + ivec2(coord.y & 1 ^ 1, coord.x & 1 ^ 1);

	ivec2 lim = textureSize(texZ, prevLevel) - ivec2(1);

	coord = clamp(coord, ivec2(0), lim);

	outColor = texelFetch(texZ, coord, prevLevel).r;
	/*/
	outColor = texture(texZ, uv * pow(2.0, prevLevel + 1.0) - 1.0).r;
	/**/
}