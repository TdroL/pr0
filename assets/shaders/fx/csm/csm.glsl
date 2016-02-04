#version 440 core

/**
 * CSM_USE_IDENTITY_KERNEL
 * CSM_ENABLE_KERNEL_SIZE_1
 * CSM_ENABLE_KERNEL_SIZE_3
 * CSM_ENABLE_KERNEL_SIZE_5
 * CSM_ENABLE_KERNEL_SIZE_7
 * CSM_ENABLE_KERNEL_SIZE_9
 * CSM_FORCE_KERNEL_SIZE x
 * CSM_FORCE_MAX_CASCADES x
 * CSM_FORCE_CASCADES_BLENDING x
 */

// #define CSM_FORCE_KERNEL_SIZE 7

#if defined(CSM_FORCE_KERNEL_SIZE)
	#undef CSM_ENABLE_KERNEL_SIZE_1
	#undef CSM_ENABLE_KERNEL_SIZE_3
	#undef CSM_ENABLE_KERNEL_SIZE_5
	#undef CSM_ENABLE_KERNEL_SIZE_7
	#undef CSM_ENABLE_KERNEL_SIZE_9

	#if CSM_FORCE_KERNEL_SIZE == 1
		#define CSM_ENABLE_KERNEL_SIZE_1
	#elif CSM_FORCE_KERNEL_SIZE == 3
		#define CSM_ENABLE_KERNEL_SIZE_3
	#elif CSM_FORCE_KERNEL_SIZE == 5
		#define CSM_ENABLE_KERNEL_SIZE_5
	#elif CSM_FORCE_KERNEL_SIZE == 7
		#define CSM_ENABLE_KERNEL_SIZE_7
	#elif CSM_FORCE_KERNEL_SIZE == 9
		#define CSM_ENABLE_KERNEL_SIZE_9
	#else
		#define CSM_ENABLE_KERNEL_SIZE_1
		#define CSM_FORCE_KERNEL_SIZE 1
	#endif
#endif

#if ! (defined(CSM_ENABLE_KERNEL_SIZE_1) || defined(CSM_ENABLE_KERNEL_SIZE_3) || defined(CSM_ENABLE_KERNEL_SIZE_5) || defined(CSM_ENABLE_KERNEL_SIZE_7) || defined(CSM_ENABLE_KERNEL_SIZE_9))
	#define CSM_ENABLE_KERNEL_SIZE_1
	#define CSM_ENABLE_KERNEL_SIZE_3
	#define CSM_ENABLE_KERNEL_SIZE_5
	#define CSM_ENABLE_KERNEL_SIZE_7
	#define CSM_ENABLE_KERNEL_SIZE_9
#endif

// csmMaxCascades must be equal to fx::CSM::maxCascades
#if defined(CSM_FORCE_MAX_CASCADES)
const uint csmMaxCascades = CSM_FORCE_MAX_CASCADES;
#else
const uint csmMaxCascades = 5;
#endif

struct CSMLayer
{
	mat4 MVP;
	vec3 centers;
	float radiuses2;
};

struct CSM
{
	CSMLayer layers[csmMaxCascades];
	uint kernelSize;
	uint splits;
	bool blendCascades;
};

float csmBilerp(vec4 values, vec2 uv)
{
	// bilinear interpolation
	vec2 mix1 = mix(values.wx, values.zy, uv.xx);
	return mix(mix1[0], mix1[1], uv.y);
}

#define CSM_CONCAT2_IMPL(x, y) x ## y
#define CSM_CONCAT2(x, y) CSM_CONCAT2_IMPL(x, y)

#define csmReadDepthComponents_IMPL(uvl, texDepths, comp) \
{ \
	for (int i = 0; i < HALF_SIZE; i++) \
	{ \
		for (int j = 0; j < HALF_SIZE; j++) \
		{ \
			int idx = i * HALF_SIZE + j; \
			ivec2 offset = ivec2(j, i) * 2 - HALF_SIZE; \
			comp[idx] = textureGatherOffset(texDepths, uvl, offset, 0); \
		} \
	} \
}

#define csmExtractComp_IMPL(comp, x, y) \
{ \
	if ((x % 2) == 0 && (y % 2) == 0) \
	{ \
		int idx = x / 2 + y / 2 * HALF_SIZE; \
		return comp[idx].xyzw; \
	} \
	else if ((x % 2) == 0 && (y % 2) == 1) \
	{ \
		int idx = x / 2 + (y - 1) / 2 * HALF_SIZE; \
		return vec4(comp[idx + HALF_SIZE].wz, comp[idx].yx); \
	} \
	else if ((x % 2) == 1 && (y % 2) == 0) \
	{ \
		int idx = (x - 1) / 2 + y / 2 * HALF_SIZE; \
		return vec4(comp[idx].y, comp[idx + 1].xw, comp[idx].z); \
	} \
	else \
	{ \
		int idx = (x - 1) / 2 + (y - 1) / 2 * HALF_SIZE; \
		return vec4(comp[idx + HALF_SIZE].z, comp[idx + HALF_SIZE + 1].w, comp[idx + 1].x, comp[idx].y); \
	} \
}

#define csmBuildDepthArray_IMPL(comp, uv, sz, depths) \
{ \
	for (int y = 0; y < SAMPLE_SIZE; y++) \
	{ \
		for (int x = 0; x < SAMPLE_SIZE; x++) \
		{ \
			int idx = x + y * SAMPLE_SIZE; \
			depths[idx] = csmBilerp(step(vec4(sz), CSM_CONCAT2(csmExtractComp, SAMPLE_SIZE)(comp, x, y)), uv); \
		} \
	} \
}

#ifdef CSM_ENABLE_KERNEL_SIZE_1
	#undef SAMPLE_SIZE
	#undef HALF_SIZE
	#undef KERNEL_SIZE
	#undef COMP_SIZE

	#define SAMPLE_SIZE 1
	#define HALF_SIZE ((SAMPLE_SIZE + 1) / 2)
	#define KERNEL_SIZE (SAMPLE_SIZE * SAMPLE_SIZE)
	#define COMP_SIZE (HALF_SIZE * HALF_SIZE)

	void csmReadDepthComponents1(vec3 uvl, sampler2DArray texDepths, out vec4 comp[COMP_SIZE])
	{
		csmReadDepthComponents_IMPL(uvl, texDepths, comp);
	}

	vec4 csmExtractComp1(vec4 comp[COMP_SIZE], int x, int y)
	{
		csmExtractComp_IMPL(comp, x, y);
	}

	void csmBuildDepthArray1(vec4 comp[COMP_SIZE], vec2 uv, float sz, out float depths[KERNEL_SIZE])
	{
		csmBuildDepthArray_IMPL(comp, uv, sz, depths);
	}

	float csmVisibilityPCF1(vec4 shadowCoord, struct CSM csm, sampler2DArray texDepths)
	{
		vec2 texDepthsSize = textureSize(texDepths, 0).xy;
		vec2 invTexDepthsSize = 1.0 / texDepthsSize;
		vec2 fractShadowCoord = fract(shadowCoord.xy * texDepthsSize - 0.5);
		vec3 roundedShadowCoord = shadowCoord.xyw - vec3(fractShadowCoord * invTexDepthsSize, 0.0);

		vec4 comp[COMP_SIZE];
		float depths[KERNEL_SIZE];

		#ifndef CSM_USE_IDENTITY_KERNEL
			float kernel[KERNEL_SIZE] = float[KERNEL_SIZE](
				1.0
			);
		#endif

		csmReadDepthComponents1(roundedShadowCoord, texDepths, comp);
		csmBuildDepthArray1(comp, fractShadowCoord, shadowCoord.z, depths);

		float acc = 0.0;
		float accDiv = 0.0;

		#ifndef CSM_USE_IDENTITY_KERNEL
			accDiv = float(depths.length());
		#endif

		for (int i = 0; i < 1; i++)
		{
			#ifndef CSM_USE_IDENTITY_KERNEL
				accDiv += kernel[i];
				kernel[i] = depths[i] * kernel[i];
				acc += kernel[i];
			#else
				acc += depths[i];
			#endif
		}

		return acc / accDiv;
	}
#endif

#ifdef CSM_ENABLE_KERNEL_SIZE_3
	#undef SAMPLE_SIZE
	#undef HALF_SIZE
	#undef KERNEL_SIZE
	#undef COMP_SIZE

	#define SAMPLE_SIZE 3
	#define HALF_SIZE ((SAMPLE_SIZE + 1) / 2)
	#define KERNEL_SIZE (SAMPLE_SIZE * SAMPLE_SIZE)
	#define COMP_SIZE (HALF_SIZE * HALF_SIZE)

	void csmReadDepthComponents3(vec3 uvl, sampler2DArray texDepths, out vec4 comp[COMP_SIZE])
	{
		csmReadDepthComponents_IMPL(uvl, texDepths, comp);
	}

	vec4 csmExtractComp3(vec4 comp[COMP_SIZE], int x, int y)
	{
		vec4 result;
		csmExtractComp_IMPL(comp, x, y);
		return result;
	}

	void csmBuildDepthArray3(vec4 comp[COMP_SIZE], vec2 uv, float sz, out float depths[KERNEL_SIZE])
	{
		csmBuildDepthArray_IMPL(comp, uv, sz, depths);
	}

	float csmVisibilityPCF3(vec4 shadowCoord, struct CSM csm, sampler2DArray texDepths)
	{
		vec2 texDepthsSize = textureSize(texDepths, 0).xy;
		vec2 invTexDepthsSize = 1.0 / texDepthsSize;
		vec2 fractShadowCoord = fract(shadowCoord.xy * texDepthsSize - 0.5);
		vec3 roundedShadowCoord = shadowCoord.xyw - vec3(fractShadowCoord * invTexDepthsSize, 0.0);

		vec4 comp[COMP_SIZE];
		float depths[KERNEL_SIZE];

		#ifndef CSM_USE_IDENTITY_KERNEL
			float kernel[KERNEL_SIZE] = float[KERNEL_SIZE](
				0.5, 1.0, 0.5,
				1.0, 1.0, 1.0,
				0.5, 1.0, 0.5
			);
		#endif

		csmReadDepthComponents3(roundedShadowCoord, texDepths, comp);
		csmBuildDepthArray3(comp, fractShadowCoord, shadowCoord.z, depths);

		float acc = 0.0;
		float accDiv = 0.0;

		#ifndef CSM_USE_IDENTITY_KERNEL
			accDiv = float(depths.length());
		#endif

		for (int i = 0; i < KERNEL_SIZE; i++)
		{
			#ifndef CSM_USE_IDENTITY_KERNEL
				accDiv += kernel[i];
				kernel[i] = depths[i] * kernel[i];
				acc += kernel[i];
			#else
				acc += depths[i];
			#endif
		}

		return acc / accDiv;
	}
#endif

#ifdef CSM_ENABLE_KERNEL_SIZE_5
	#undef SAMPLE_SIZE
	#undef HALF_SIZE
	#undef KERNEL_SIZE
	#undef COMP_SIZE

	#define SAMPLE_SIZE 5
	#define HALF_SIZE ((SAMPLE_SIZE + 1) / 2)
	#define KERNEL_SIZE (SAMPLE_SIZE * SAMPLE_SIZE)
	#define COMP_SIZE (HALF_SIZE * HALF_SIZE)

	void csmReadDepthComponents5(vec3 uvl, sampler2DArray texDepths, out vec4 comp[COMP_SIZE])
	{
		csmReadDepthComponents_IMPL(uvl, texDepths, comp);
	}

	vec4 csmExtractComp5(vec4 comp[COMP_SIZE], int x, int y)
	{
		csmExtractComp_IMPL(comp, x, y);
	}

	void csmBuildDepthArray5(vec4 comp[COMP_SIZE], vec2 uv, float sz, out float depths[KERNEL_SIZE])
	{
		csmBuildDepthArray_IMPL(comp, uv, sz, depths);
	}

	float csmVisibilityPCF5(vec4 shadowCoord, struct CSM csm, sampler2DArray texDepths)
	{
		vec2 texDepthsSize = textureSize(texDepths, 0).xy;
		vec2 invTexDepthsSize = 1.0 / texDepthsSize;
		vec2 fractShadowCoord = fract(shadowCoord.xy * texDepthsSize - 0.5);
		vec3 roundedShadowCoord = shadowCoord.xyw - vec3(fractShadowCoord * invTexDepthsSize, 0.0);

		vec4 comp[COMP_SIZE];
		float depths[KERNEL_SIZE];

		#ifndef CSM_USE_IDENTITY_KERNEL
			float kernel[KERNEL_SIZE] = float[KERNEL_SIZE](
				/**/
				0.25, 0.50, 1.00, 0.50, 0.25,
				0.50, 1.00, 1.00, 1.00, 0.50,
				1.00, 1.00, 1.00, 1.00, 1.00,
				0.50, 1.00, 1.00, 1.00, 0.50,
				0.25, 0.50, 1.00, 0.50, 0.25
				/*/
				0.0, 0.5, 1.0, 0.5, 0.0,
				0.5, 1.0, 1.0, 1.0, 0.5,
				1.0, 1.0, 1.0, 1.0, 1.0,
				0.5, 1.0, 1.0, 1.0, 0.5,
				0.0, 0.5, 1.0, 0.5, 0.0
				/**/
			);
		#endif

		csmReadDepthComponents5(roundedShadowCoord, texDepths, comp);
		csmBuildDepthArray5(comp, fractShadowCoord, shadowCoord.z, depths);

		float acc = 0.0;
		float accDiv = 0.0;

		#ifdef CSM_USE_IDENTITY_KERNEL
			accDiv = float(depths.length());
		#endif

		for (int i = 0; i < KERNEL_SIZE; i++)
		{
			#ifndef CSM_USE_IDENTITY_KERNEL
				accDiv += kernel[i];
				kernel[i] = depths[i] * kernel[i];
				acc += kernel[i];
			#else
				acc += depths[i];
			#endif
		}

		return acc / accDiv;
	}
#endif

#ifdef CSM_ENABLE_KERNEL_SIZE_7
	#undef SAMPLE_SIZE
	#undef HALF_SIZE
	#undef KERNEL_SIZE
	#undef COMP_SIZE

	#define SAMPLE_SIZE 7
	#define HALF_SIZE ((SAMPLE_SIZE + 1) / 2)
	#define KERNEL_SIZE (SAMPLE_SIZE * SAMPLE_SIZE)
	#define COMP_SIZE (HALF_SIZE * HALF_SIZE)

	void csmReadDepthComponents7(vec3 uvl, sampler2DArray texDepths, out vec4 comp[COMP_SIZE])
	{
		csmReadDepthComponents_IMPL(uvl, texDepths, comp);
	}

	vec4 csmExtractComp7(vec4 comp[COMP_SIZE], int x, int y)
	{
		csmExtractComp_IMPL(comp, x, y);
	}

	void csmBuildDepthArray7(vec4 comp[COMP_SIZE], vec2 uv, float sz, out float depths[KERNEL_SIZE])
	{
		csmBuildDepthArray_IMPL(comp, uv, sz, depths);
	}

	float csmVisibilityPCF7(vec4 shadowCoord, struct CSM csm, sampler2DArray texDepths)
	{
		vec2 texDepthsSize = textureSize(texDepths, 0).xy;
		vec2 invTexDepthsSize = 1.0 / texDepthsSize;
		vec2 fractShadowCoord = fract(shadowCoord.xy * texDepthsSize - 0.5);
		vec3 roundedShadowCoord = shadowCoord.xyw - vec3(fractShadowCoord * invTexDepthsSize, 0.0);

		vec4 comp[COMP_SIZE];
		float depths[KERNEL_SIZE];

		#ifndef CSM_USE_IDENTITY_KERNEL
			float kernel[KERNEL_SIZE] = float[KERNEL_SIZE](
				/**/
				0.125, 0.250, 0.500, 1.000, 0.500, 0.250, 0.125,
				0.250, 0.750, 1.000, 1.000, 1.000, 0.750, 0.250,
				0.500, 1.000, 1.000, 1.000, 1.000, 1.000, 0.500,
				1.000, 1.000, 1.000, 1.000, 1.000, 1.000, 1.000,
				0.500, 1.000, 1.000, 1.000, 1.000, 1.000, 0.500,
				0.250, 0.750, 1.000, 1.000, 1.000, 0.750, 0.250,
				0.125, 0.250, 0.500, 1.000, 0.500, 0.250, 0.125
				/*/
				0.0, 0.0, 0.5, 1.0, 0.5, 0.0, 0.0,
				0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0,
				0.5, 1.0, 1.0, 1.0, 1.0, 1.0, 0.5,
				1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
				0.5, 1.0, 1.0, 1.0, 1.0, 1.0, 0.5,
				0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0,
				0.0, 0.0, 0.5, 1.0, 0.5, 0.0, 0.0
				/**/
			);
		#endif

		csmReadDepthComponents7(roundedShadowCoord, texDepths, comp);
		csmBuildDepthArray7(comp, fractShadowCoord, shadowCoord.z, depths);

		float acc = 0.0;
		float accDiv = 0.0;

		#ifdef CSM_USE_IDENTITY_KERNEL
			accDiv = float(depths.length());
		#endif

		for (int i = 0; i < KERNEL_SIZE; i++)
		{
			#ifndef CSM_USE_IDENTITY_KERNEL
				accDiv += kernel[i];
				kernel[i] = depths[i] * kernel[i];
				acc += kernel[i];
			#else
				acc += depths[i];
			#endif
		}

		return acc / accDiv;
	}
#endif

#ifdef CSM_ENABLE_KERNEL_SIZE_9
	#undef SAMPLE_SIZE
	#undef HALF_SIZE
	#undef KERNEL_SIZE
	#undef COMP_SIZE

	#define SAMPLE_SIZE 9
	#define HALF_SIZE ((SAMPLE_SIZE + 1) / 2)
	#define KERNEL_SIZE (SAMPLE_SIZE * SAMPLE_SIZE)
	#define COMP_SIZE (HALF_SIZE * HALF_SIZE)

	void csmReadDepthComponents9(vec3 uvl, sampler2DArray texDepths, out vec4 comp[COMP_SIZE])
	{
		csmReadDepthComponents_IMPL(uvl, texDepths, comp);
	}

	vec4 csmExtractComp9(vec4 comp[COMP_SIZE], int x, int y)
	{
		csmExtractComp_IMPL(comp, x, y);
	}

	void csmBuildDepthArray9(vec4 comp[COMP_SIZE], vec2 uv, float sz, out float depths[KERNEL_SIZE])
	{
		csmBuildDepthArray_IMPL(comp, uv, sz, depths);
	}

	float csmVisibilityPCF9(vec4 shadowCoord, struct CSM csm, sampler2DArray texDepths)
	{
		vec2 texDepthsSize = textureSize(texDepths, 0).xy;
		vec2 invTexDepthsSize = 1.0 / texDepthsSize;
		vec2 fractShadowCoord = fract(shadowCoord.xy * texDepthsSize - 0.5);
		vec3 roundedShadowCoord = shadowCoord.xyw - vec3(fractShadowCoord * invTexDepthsSize, 0.0);

		vec4 comp[COMP_SIZE];
		float depths[KERNEL_SIZE];

		#ifndef CSM_USE_IDENTITY_KERNEL
			float kernel[KERNEL_SIZE] = float[KERNEL_SIZE](
				/**/
				0.125, 0.250, 0.500, 0.750, 1.000, 0.750, 0.500, 0.250, 0.125,
				0.250, 0.500, 0.750, 1.000, 1.000, 1.000, 0.750, 0.500, 0.250,
				0.500, 0.750, 1.000, 1.000, 1.000, 1.000, 1.000, 0.750, 0.500,
				0.750, 1.000, 1.000, 1.000, 1.000, 1.000, 1.000, 1.000, 0.750,
				1.000, 1.000, 1.000, 1.000, 1.000, 1.000, 1.000, 1.000, 1.000,
				0.750, 1.000, 1.000, 1.000, 1.000, 1.000, 1.000, 1.000, 0.750,
				0.500, 0.750, 1.000, 1.000, 1.000, 1.000, 1.000, 0.750, 0.500,
				0.250, 0.500, 0.750, 1.000, 1.000, 1.000, 0.750, 0.500, 0.250,
				0.125, 0.250, 0.500, 0.750, 1.000, 0.750, 0.500, 0.250, 0.125
				/*/
				0.0, 0.0, 0.0, 0.5, 1.0, 0.5, 0.0, 0.0, 0.0,
				0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0,
				0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0,
				0.5, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.5,
				1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
				0.5, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.5,
				0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0,
				0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0,
				0.0, 0.0, 0.0, 0.5, 1.0, 0.5, 0.0, 0.0, 0.0
				/**/
			);
		#endif

		csmReadDepthComponents9(roundedShadowCoord, texDepths, comp);
		csmBuildDepthArray9(comp, fractShadowCoord, shadowCoord.z, depths);

		float acc = 0.0;
		float accDiv = 0.0;

		#ifdef CSM_USE_IDENTITY_KERNEL
			accDiv = float(depths.length());
		#endif

		for (int i = 0; i < KERNEL_SIZE; i++)
		{
			#ifndef CSM_USE_IDENTITY_KERNEL
				accDiv += kernel[i];
				kernel[i] = depths[i] * kernel[i];
				acc += kernel[i];
			#else
				acc += depths[i];
			#endif
		}

		return acc / accDiv;
	}
#endif

float csmVisibilityPCF(vec4 shadowCoord, struct CSM csm, sampler2DArray texDepths)
{
	uint kernelSize = csm.kernelSize;

	#if defined(CSM_FORCE_KERNEL_SIZE)
		kernelSize = CSM_FORCE_KERNEL_SIZE;
	#endif

	switch (kernelSize)
	{
		#ifdef CSM_ENABLE_KERNEL_SIZE_9
		case 9:
			return csmVisibilityPCF9(shadowCoord, csm, texDepths);
		#endif
		#ifdef CSM_ENABLE_KERNEL_SIZE_7
		case 7:
			return csmVisibilityPCF7(shadowCoord, csm, texDepths);
		#endif
		#ifdef CSM_ENABLE_KERNEL_SIZE_5
		case 5:
			return csmVisibilityPCF5(shadowCoord, csm, texDepths);
		#endif
		#ifdef CSM_ENABLE_KERNEL_SIZE_3
		case 3:
			return csmVisibilityPCF3(shadowCoord, csm, texDepths);
		#endif
		#ifdef CSM_ENABLE_KERNEL_SIZE_1
		case 1:
			return csmVisibilityPCF1(shadowCoord, csm, texDepths);
		#endif
		default:
			return 1.0;
	}
}

float csmVisibility(vec4 position, float cosTheta, struct CSM csm, sampler2DArray texDepths)
{
	float visibility = 1.0;

	float zBias = 0.0; // -clamp(0.0001 * tan(acos(cosTheta)), 0.0, 0.01);

	uint splits = min(csmMaxCascades, csm.splits);
	bool blendCascades = csm.blendCascades;

	#if defined(CSM_FORCE_CASCADES_BLENDING)
		blendCascades = CSM_FORCE_CASCADES_BLENDING;
	#endif

	if (blendCascades)
	{
		float radiusPadding = 0.8;
		float radiusPadding2 = radiusPadding * radiusPadding;

		for (uint i = 0; i < splits; i++)
		{
			vec4 shadowCoord = csm.layers[i].MVP * position;
			shadowCoord.z -= zBias;
			shadowCoord.w = i;

			vec3 camCenterPosDiff = csm.layers[i].centers.xyz - position.xyz;
			float camDistance2 = dot(camCenterPosDiff, camCenterPosDiff);

			if (csm.layers[i].radiuses2 > camDistance2)
			{
				visibility = csmVisibilityPCF(shadowCoord, csm, texDepths);

				if (csm.layers[i].radiuses2 * radiusPadding2 < camDistance2)
				{
					float visibility2 = 1.0;

					if (i + 1 < splits)
					{
						vec4 shadowCoord2 = csm.layers[i + 1].MVP * position;
						shadowCoord2.z -= zBias;
						shadowCoord2.w = i + 1;

						visibility2 = csmVisibilityPCF(shadowCoord2, csm, texDepths);
					}

					float blend = smoothstep(csm.layers[i].radiuses2 * radiusPadding2, csm.layers[i].radiuses2, camDistance2);

					visibility = mix(visibility, visibility2, blend);
				}

				break;
			}
		}
	}
	else
	{
		for (uint i = 0; i < splits; i++)
		{
			vec4 shadowCoord = csm.layers[i].MVP * position;
			shadowCoord.z -= zBias;
			shadowCoord.w = i;

			vec3 camCenterPosDiff = csm.layers[i].centers.xyz - position.xyz;
			float camDistance2 = dot(camCenterPosDiff, camCenterPosDiff);

			if (csm.layers[i].radiuses2 > camDistance2)
			{
				visibility = csmVisibilityPCF(shadowCoord, csm, texDepths);
				break;
			}
		}
	}

	// return pow(visibility, 2.2);
	return visibility * visibility;
	// return visibility;
}

#undef CSM_CONCAT2
#undef CSM_CONCAT2_IMPL
#undef CSM_ENABLE_KERNEL_SIZE_1
#undef CSM_ENABLE_KERNEL_SIZE_3
#undef CSM_ENABLE_KERNEL_SIZE_5
#undef CSM_ENABLE_KERNEL_SIZE_7
#undef CSM_ENABLE_KERNEL_SIZE_9
#undef csmBuildDepthArray_IMPL
#undef csmExtractComp_IMPL
#undef csmReadDepthComponents_IMPL
#undef HALF_SIZE
#undef COMP_SIZE
#undef KERNEL_SIZE
#undef SAMPLE_SIZE