#version 330 core

layout(location = 0) out vec4 outColor;

in vec2 uv;

uniform sampler2D texSource;
uniform sampler2D texZ;

void main()
{
	/**/
	outColor.rgb = texture(texSource, uv).rrr;
	/*/
	outColor.rgb = texture(texSource, uv).rgb;
	/**/

	int level = 0;
	outColor.rgb = texelFetch(texZ, ivec2(uv * textureSize(texZ, level)), level).rrr;
	outColor.rgb = texelFetch(texSource, ivec2(gl_FragCoord.xy), 0).rgb;
	outColor.a = 1.0;
}