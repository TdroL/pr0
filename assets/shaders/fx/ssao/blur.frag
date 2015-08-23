#version 330 core

layout(location = 0) out vec4 outColor;

in vec2 uv;

uniform sampler2D texSource;
uniform vec2 scale;

float unpack2(vec2 source);

void main()
{
#if !defined(RADIUS)
	#define RADIUS 10
#endif

	int radius = RADIUS;
	float gaussianKernel[RADIUS + 1];

#if RADIUS == 3
	gaussianKernel[0] = 0.153170;
	gaussianKernel[1] = 0.144893;
	gaussianKernel[2] = 0.122649;
	gaussianKernel[3] = 0.092902;  // stddev = 2.0

	float stride = 2.0;
#elif RADIUS == 4
	gaussianKernel[0] = 0.153170;
	gaussianKernel[1] = 0.144893;
	gaussianKernel[2] = 0.122649;
	gaussianKernel[3] = 0.092902;
	gaussianKernel[4] = 0.062970;  // stddev = 2.0

	float stride = 2.0;
#elif RADIUS == 6
	gaussianKernel[0] = 0.111220;
	gaussianKernel[1] = 0.107798;
	gaussianKernel[2] = 0.098151;
	gaussianKernel[3] = 0.083953;
	gaussianKernel[4] = 0.067458;
	gaussianKernel[5] = 0.050920;
	gaussianKernel[6] = 0.036108;

	float stride = 2.0;
#elif RADIUS == 10
	gaussianKernel[0]  = 0.580947;
	gaussianKernel[1]  = 0.523832;
	gaussianKernel[2]  = 0.468721;
	gaussianKernel[3]  = 0.415692;
	gaussianKernel[4]  = 0.364828;
	gaussianKernel[5]  = 0.316227;
	gaussianKernel[6]  = 0.270142;
	gaussianKernel[7]  = 0.226274;
	gaussianKernel[8]  = 0.185202;
	gaussianKernel[9]  = 0.146969;
	gaussianKernel[10] = 0.111803;

	float stride = 1.0;
#endif

	float edgeSharpness = 1.0;

	ivec2 coord = ivec2(gl_FragCoord.xy);

	vec4 origin = texture(texSource, uv);
	vec2 axis = scale / textureSize(texSource, 0);

	float z = unpack2(origin.gb);

	float occlussion = origin.r;
	float weight = gaussianKernel[0];

	float sumOcclussion = occlussion * weight;
	float sumWeight = weight;

	for (int r = -radius; r <= radius; r++)
	{
		if (r == 0) continue;

		vec4 tap = texture(texSource, uv + axis * r * stride);
		float tapZ = unpack2(tap.gb);

		float tapOcclusion = tap.r;
		float tapWeight = (0.3 + gaussianKernel[abs(r)]) * max(0.0, 1.0 - (edgeSharpness * 2000.0) * abs(z - tapZ));

		sumOcclussion += tapOcclusion * tapWeight;
		sumWeight += tapWeight;
	}

	outColor.r = sumOcclussion / sumWeight;
	outColor.gba = origin.gba;
}