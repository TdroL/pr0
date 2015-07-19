#version 330 core

layout(location = 0) out vec4 outColor;

in vec2 uv;

uniform sampler2D texSource;
uniform int layer;

void main()
{
	vec2 texelSize = 1.0 / textureSize(texSource, 0).xy;
	vec2 coord = uv;

	outColor = texture(texSource, coord);

	if (layer == 0)
	{
		const float offset[2] = float[] (1.3846153846, 3.2307692308);
		const float weight[3] = float[] (0.2270270270, 0.3162162162, 0.0702702703);

		outColor.xy *= weight[0];

		outColor.xy += texture(texSource, coord + vec2(0.0, texelSize.y * offset[0])).xy * weight[1];
		outColor.xy += texture(texSource, coord - vec2(0.0, texelSize.y * offset[0])).xy * weight[1];
		outColor.xy += texture(texSource, coord + vec2(0.0, texelSize.y * offset[1])).xy * weight[2];
		outColor.xy += texture(texSource, coord - vec2(0.0, texelSize.y * offset[1])).xy * weight[2];
	}
	else if (layer == 1)
	{
		const float offset[2] = float[] (1.0, 1.75);
		const float weight[3] = float[] (0.4, 0.25, 0.05);

		outColor.xy *= weight[0];

		outColor.xy += texture(texSource, coord + vec2(0.0, texelSize.y * offset[0])).xy * weight[1];
		outColor.xy += texture(texSource, coord - vec2(0.0, texelSize.y * offset[0])).xy * weight[1];
		outColor.xy += texture(texSource, coord + vec2(0.0, texelSize.y * offset[1])).xy * weight[2];
		outColor.xy += texture(texSource, coord - vec2(0.0, texelSize.y * offset[1])).xy * weight[2];
	}
	else if (layer == 2)
	{
		const float offset[1] = float[] (1.0);
		const float weight[2] = float[] (0.8, 0.1);

		outColor.xy *= weight[0];

		outColor.xy += texture(texSource, coord + vec2(0.0, texelSize.y * offset[0])).xy * weight[1];
		outColor.xy += texture(texSource, coord - vec2(0.0, texelSize.y * offset[0])).xy * weight[1];
	}
	else if (layer == 3)
	{
		// do not blur
	}
}