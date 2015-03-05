#version 330

layout(location = 0) out vec4 outColor;

in vec2 uv;

uniform sampler2D texSource;
uniform sampler2D texDepth;
uniform vec2 scale;

vec4 blurBox(sampler2D tex, vec2 uv, vec2 scale, int size);
float depthUnpack(vec2 source);
vec3 positionReconstruct(float z, vec2 uv);

void main()
{
	// outColor = blurBox(texSource, uv, scale, 9);

	int radius = 4;
	float edgeSharpness = 1.0;

	float gaussian[5];
	gaussian[0] = 0.153170;
	gaussian[1] = 0.144893;
	gaussian[2] = 0.122649;
	gaussian[3] = 0.092902;
	gaussian[4] = 0.062970;

	vec3 origin = texture(texSource, uv).rgb;
	vec2 sourceScale = scale / textureSize(texSource, 0);

	float occlussion = origin.r;
	float depth = depthUnpack(origin.gb);

	float weight = gaussian[0];

	float sumOcclussion = occlussion * weight;
	float sumWeight = weight;

	for (int r = -radius; r <= radius; r++)
	{
		if (r == 0) continue;

		vec3 tap = texture(texSource, uv + sourceScale * r * 1.5).rgb;
		float tapOcclusion = tap.r;
		float tapDepth = depthUnpack(tap.gb);

		float tapWeight = (0.3 + gaussian[abs(r)]) * max(0.0, 1.0 - (edgeSharpness * 2000.0) * abs(tapDepth - depth));

		sumOcclussion += tapOcclusion * tapWeight;
		sumWeight += tapWeight;
	}

	// float depth2 = texture(texDepth, uv) * 2.0 - 1.0;

	outColor.r = sumOcclussion / sumWeight;
	outColor.gb = origin.gb;

	// outColor.r = 0.0;
	// outColor.r = depth2;
	// outColor.rgb = vec3(depth2);
	// outColor.rgb = vec3(depth);
	// outColor.gb = outColor.rr;
	// outColor.rgb = vec3(occlussion);

	// outColor.r = blurBox(texSource, uv, scale, 9).r;
	// outColor.rgb = vec3(abs(depth));
	// outColor.rgb = vec3(scale, 0.0);
	// outColor.rgb = vec3(1.0);
	outColor.a = 1.0;
}