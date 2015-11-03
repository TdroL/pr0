#version 440 core

// #define CSM_USE_IDENTITY_KERNEL
#define CSM_FORCE_KERNEL_SIZE 1

#if (defined(CSM_KERNEL_SIZE_1) && ! defined(CSM_FORCE_KERNEL_SIZE)) || CSM_FORCE_KERNEL_SIZE == 1
	#define CSM_KERNEL_SIZE_1
	#undef CSM_KERNEL_SIZE_3
	#undef CSM_KERNEL_SIZE_5
	#undef CSM_KERNEL_SIZE_7
	#undef CSM_KERNEL_SIZE_9
#elif (defined(CSM_KERNEL_SIZE_3) && ! defined(CSM_FORCE_KERNEL_SIZE)) || CSM_FORCE_KERNEL_SIZE == 3
	#define CSM_KERNEL_SIZE_3
	#undef CSM_KERNEL_SIZE_1
	#undef CSM_KERNEL_SIZE_5
	#undef CSM_KERNEL_SIZE_7
	#undef CSM_KERNEL_SIZE_9
#elif (defined(CSM_KERNEL_SIZE_5) && ! defined(CSM_FORCE_KERNEL_SIZE)) || CSM_FORCE_KERNEL_SIZE == 5
	#define CSM_KERNEL_SIZE_5
	#undef CSM_KERNEL_SIZE_1
	#undef CSM_KERNEL_SIZE_3
	#undef CSM_KERNEL_SIZE_7
	#undef CSM_KERNEL_SIZE_9
#elif (defined(CSM_KERNEL_SIZE_7) && ! defined(CSM_FORCE_KERNEL_SIZE)) || CSM_FORCE_KERNEL_SIZE == 7
	#define CSM_KERNEL_SIZE_7
	#undef CSM_KERNEL_SIZE_1
	#undef CSM_KERNEL_SIZE_3
	#undef CSM_KERNEL_SIZE_5
	#undef CSM_KERNEL_SIZE_9
#elif (defined(CSM_KERNEL_SIZE_9) && ! defined(CSM_FORCE_KERNEL_SIZE)) || CSM_FORCE_KERNEL_SIZE == 9
	#define CSM_KERNEL_SIZE_9
	#undef CSM_KERNEL_SIZE_1
	#undef CSM_KERNEL_SIZE_3
	#undef CSM_KERNEL_SIZE_5
	#undef CSM_KERNEL_SIZE_7
#else
	#define CSM_KERNEL_SIZE_1
	#define CSM_KERNEL_SIZE_3
	#define CSM_KERNEL_SIZE_5
	#define CSM_KERNEL_SIZE_7
	#define CSM_KERNEL_SIZE_9
#endif

const int maxCascades = 5;

struct CSM
{
	mat4 MVP[maxCascades];
	vec3 centers[maxCascades];
	float radiuses2[maxCascades];
	uint kernelSize;
	uint splits;
	bool blendCascades;
};

float bilerp(vec4 values, vec2 uv) // bilinear interpolation
{
	vec2 mix1 = mix(values.wx, values.zy, uv.xx);
	float mix2 = mix(mix1[0], mix1[1], uv.y);

	return mix2;
}

#ifdef CSM_KERNEL_SIZE_1
	#define SAMPLE_SIZE 1
	#define HALF_SIZE ((SAMPLE_SIZE + 1) / 2)
	#define KERNEL_SIZE (SAMPLE_SIZE * SAMPLE_SIZE)
	#define COMP_SIZE (HALF_SIZE * HALF_SIZE)

	void csmReadDepthComponents1(vec3 uvl, out vec4 comp[COMP_SIZE], sampler2DArray texDepths)
	{
		for (int i = 0; i < HALF_SIZE; i++)
		{
			for (int j = 0; j < HALF_SIZE; j++)
			{
				int idx = i * HALF_SIZE + j;
				ivec2 offset = ivec2(j, i) * 2 - HALF_SIZE;

				comp[idx] = textureGatherOffset(texDepths, uvl, offset, 0);
			}
		}
	}

	vec4 csmExtractComp1(vec4 comp[COMP_SIZE], int x, int y)
	{
		if ((x % 2) == 0 && (y % 2) == 0)
		{
			int idx = x / 2 + y / 2 * HALF_SIZE;
			return comp[idx].xyzw;
		}

		if ((x % 2) == 0 && (y % 2) == 1)
		{
			int idx = x / 2 + (y - 1) / 2 * HALF_SIZE;
			return vec4(comp[idx + HALF_SIZE].wz, comp[idx].yx);
		}

		if ((x % 2) == 1 && (y % 2) == 0)
		{
			int idx = (x - 1) / 2 + y / 2 * HALF_SIZE;
			return vec4(comp[idx].y, comp[idx + 1].xw, comp[idx].z);
		}

		int idx = (x - 1) / 2 + (y - 1) / 2 * HALF_SIZE;
		return vec4(comp[idx + HALF_SIZE].z, comp[idx + HALF_SIZE + 1].w, comp[idx + 1].x, comp[idx].y);
	}

	void csmBuildDepthArray1(vec4 comp[COMP_SIZE], vec2 uv, float sz, out float depths[KERNEL_SIZE])
	{
		for (int y = 0; y < SAMPLE_SIZE; y++)
		{
			for (int x = 0; x < SAMPLE_SIZE; x++)
			{
				int idx = x + y * SAMPLE_SIZE;
				depths[idx] = bilerp(step(vec4(sz), csmExtractComp1(comp, x, y)), uv);
			}
		}
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

		csmReadDepthComponents1(roundedShadowCoord, comp, texDepths);
		csmBuildDepthArray1(comp, fractShadowCoord, shadowCoord.z, depths);

		float acc = 0.0;
		#ifndef CSM_USE_IDENTITY_KERNEL
		float accDiv = 0.0;
		#else
		float accDiv = float(depths.length());
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

#ifdef CSM_KERNEL_SIZE_3
	#define SAMPLE_SIZE 3
	#define HALF_SIZE ((SAMPLE_SIZE + 1) / 2)
	#define KERNEL_SIZE (SAMPLE_SIZE * SAMPLE_SIZE)
	#define COMP_SIZE (HALF_SIZE * HALF_SIZE)

	void csmReadDepthComponents3(vec3 uvl, out vec4 comp[COMP_SIZE], sampler2DArray texDepths)
	{
		for (int i = 0; i < HALF_SIZE; i++)
		{
			for (int j = 0; j < HALF_SIZE; j++)
			{
				int idx = i * HALF_SIZE + j;
				ivec2 offset = ivec2(j, i) * 2 - HALF_SIZE;

				comp[idx] = textureGatherOffset(texDepths, uvl, offset, 0);
			}
		}
	}

	vec4 csmExtractComp3(vec4 comp[COMP_SIZE], int x, int y)
	{
		if ((x % 2) == 0 && (y % 2) == 0)
		{
			int idx = x / 2 + y / 2 * HALF_SIZE;
			return comp[idx].xyzw;
		}

		if ((x % 2) == 0 && (y % 2) == 1)
		{
			int idx = x / 2 + (y - 1) / 2 * HALF_SIZE;
			return vec4(comp[idx + HALF_SIZE].wz, comp[idx].yx);
		}

		if ((x % 2) == 1 && (y % 2) == 0)
		{
			int idx = (x - 1) / 2 + y / 2 * HALF_SIZE;
			return vec4(comp[idx].y, comp[idx + 1].xw, comp[idx].z);
		}

		int idx = (x - 1) / 2 + (y - 1) / 2 * HALF_SIZE;
		return vec4(comp[idx + HALF_SIZE].z, comp[idx + HALF_SIZE + 1].w, comp[idx + 1].x, comp[idx].y);
	}

	void csmBuildDepthArray3(vec4 comp[COMP_SIZE], vec2 uv, float sz, out float depths[KERNEL_SIZE])
	{
		for (int y = 0; y < SAMPLE_SIZE; y++)
		{
			for (int x = 0; x < SAMPLE_SIZE; x++)
			{
				int idx = x + y * SAMPLE_SIZE;
				depths[idx] = bilerp(step(vec4(sz), csmExtractComp3(comp, x, y)), uv);
			}
		}
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

		csmReadDepthComponents3(roundedShadowCoord, comp, texDepths);
		csmBuildDepthArray3(comp, fractShadowCoord, shadowCoord.z, depths);

		float acc = 0.0;
		#ifndef CSM_USE_IDENTITY_KERNEL
		float accDiv = 0.0;
		#else
		float accDiv = float(depths.length());
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

#ifdef CSM_KERNEL_SIZE_5
	#define SAMPLE_SIZE 5
	#define HALF_SIZE ((SAMPLE_SIZE + 1) / 2)
	#define KERNEL_SIZE (SAMPLE_SIZE * SAMPLE_SIZE)
	#define COMP_SIZE (HALF_SIZE * HALF_SIZE)

	void csmReadDepthComponents5(vec3 uvl, out vec4 comp[COMP_SIZE], sampler2DArray texDepths)
	{
		for (int i = 0; i < HALF_SIZE; i++)
		{
			for (int j = 0; j < HALF_SIZE; j++)
			{
				int idx = i * HALF_SIZE + j;
				ivec2 offset = ivec2(j, i) * 2 - HALF_SIZE;

				comp[idx] = textureGatherOffset(texDepths, uvl, offset, 0);
			}
		}
	}

	vec4 csmExtractComp5(vec4 comp[COMP_SIZE], int x, int y)
	{
		if ((x % 2) == 0 && (y % 2) == 0)
		{
			int idx = x / 2 + y / 2 * HALF_SIZE;
			return comp[idx].xyzw;
		}

		if ((x % 2) == 0 && (y % 2) == 1)
		{
			int idx = x / 2 + (y - 1) / 2 * HALF_SIZE;
			return vec4(comp[idx + HALF_SIZE].wz, comp[idx].yx);
		}

		if ((x % 2) == 1 && (y % 2) == 0)
		{
			int idx = (x - 1) / 2 + y / 2 * HALF_SIZE;
			return vec4(comp[idx].y, comp[idx + 1].xw, comp[idx].z);
		}

		int idx = (x - 1) / 2 + (y - 1) / 2 * HALF_SIZE;
		return vec4(comp[idx + HALF_SIZE].z, comp[idx + HALF_SIZE + 1].w, comp[idx + 1].x, comp[idx].y);
	}

	void csmBuildDepthArray5(vec4 comp[COMP_SIZE], vec2 uv, float sz, out float depths[KERNEL_SIZE])
	{
		for (int y = 0; y < SAMPLE_SIZE; y++)
		{
			for (int x = 0; x < SAMPLE_SIZE; x++)
			{
				int idx = x + y * SAMPLE_SIZE;
				depths[idx] = bilerp(step(vec4(sz), csmExtractComp5(comp, x, y)), uv);
			}
		}
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

		csmReadDepthComponents5(roundedShadowCoord, comp, texDepths);
		csmBuildDepthArray5(comp, fractShadowCoord, shadowCoord.z, depths);

		float acc = 0.0;
		#ifndef CSM_USE_IDENTITY_KERNEL
		float accDiv = 0.0;
		#else
		float accDiv = float(depths.length());
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

#ifdef CSM_KERNEL_SIZE_7
	#define SAMPLE_SIZE 7
	#define HALF_SIZE ((SAMPLE_SIZE + 1) / 2)
	#define KERNEL_SIZE (SAMPLE_SIZE * SAMPLE_SIZE)
	#define COMP_SIZE (HALF_SIZE * HALF_SIZE)

	void csmReadDepthComponents7(vec3 uvl, out vec4 comp[COMP_SIZE], sampler2DArray texDepths)
	{
		for (int i = 0; i < HALF_SIZE; i++)
		{
			for (int j = 0; j < HALF_SIZE; j++)
			{
				int idx = i * HALF_SIZE + j;
				ivec2 offset = ivec2(j, i) * 2 - HALF_SIZE;

				comp[idx] = textureGatherOffset(texDepths, uvl, offset, 0);
			}
		}
	}

	vec4 csmExtractComp7(vec4 comp[COMP_SIZE], int x, int y)
	{
		if ((x % 2) == 0 && (y % 2) == 0)
		{
			int idx = x / 2 + y / 2 * HALF_SIZE;
			return comp[idx].xyzw;
		}

		if ((x % 2) == 0 && (y % 2) == 1)
		{
			int idx = x / 2 + (y - 1) / 2 * HALF_SIZE;
			return vec4(comp[idx + HALF_SIZE].wz, comp[idx].yx);
		}

		if ((x % 2) == 1 && (y % 2) == 0)
		{
			int idx = (x - 1) / 2 + y / 2 * HALF_SIZE;
			return vec4(comp[idx].y, comp[idx + 1].xw, comp[idx].z);
		}

		int idx = (x - 1) / 2 + (y - 1) / 2 * HALF_SIZE;
		return vec4(comp[idx + HALF_SIZE].z, comp[idx + HALF_SIZE + 1].w, comp[idx + 1].x, comp[idx].y);
	}

	void csmBuildDepthArray7(vec4 comp[COMP_SIZE], vec2 uv, float sz, out float depths[KERNEL_SIZE])
	{
		for (int y = 0; y < SAMPLE_SIZE; y++)
		{
			for (int x = 0; x < SAMPLE_SIZE; x++)
			{
				int idx = x + y * SAMPLE_SIZE;
				depths[idx] = bilerp(step(vec4(sz), csmExtractComp7(comp, x, y)), uv);
			}
		}
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

		csmReadDepthComponents7(roundedShadowCoord, comp, texDepths);
		csmBuildDepthArray7(comp, fractShadowCoord, shadowCoord.z, depths);

		float acc = 0.0;
		#ifndef CSM_USE_IDENTITY_KERNEL
		float accDiv = 0.0;
		#else
		float accDiv = float(depths.length());
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

#ifdef CSM_KERNEL_SIZE_9
	#define SAMPLE_SIZE 9
	#define HALF_SIZE ((SAMPLE_SIZE + 1) / 2)
	#define KERNEL_SIZE (SAMPLE_SIZE * SAMPLE_SIZE)
	#define COMP_SIZE (HALF_SIZE * HALF_SIZE)

	void csmReadDepthComponents9(vec3 uvl, out vec4 comp[COMP_SIZE], sampler2DArray texDepths)
	{
		for (int i = 0; i < HALF_SIZE; i++)
		{
			for (int j = 0; j < HALF_SIZE; j++)
			{
				int idx = i * HALF_SIZE + j;
				ivec2 offset = ivec2(j, i) * 2 - HALF_SIZE;

				comp[idx] = textureGatherOffset(texDepths, uvl, offset, 0);
			}
		}
	}

	vec4 csmExtractComp9(vec4 comp[COMP_SIZE], int x, int y)
	{
		if ((x % 2) == 0 && (y % 2) == 0)
		{
			int idx = x / 2 + y / 2 * HALF_SIZE;
			return comp[idx].xyzw;
		}

		if ((x % 2) == 0 && (y % 2) == 1)
		{
			int idx = x / 2 + (y - 1) / 2 * HALF_SIZE;
			return vec4(comp[idx + HALF_SIZE].wz, comp[idx].yx);
		}

		if ((x % 2) == 1 && (y % 2) == 0)
		{
			int idx = (x - 1) / 2 + y / 2 * HALF_SIZE;
			return vec4(comp[idx].y, comp[idx + 1].xw, comp[idx].z);
		}

		int idx = (x - 1) / 2 + (y - 1) / 2 * HALF_SIZE;
		return vec4(comp[idx + HALF_SIZE].z, comp[idx + HALF_SIZE + 1].w, comp[idx + 1].x, comp[idx].y);
	}

	void csmBuildDepthArray9(vec4 comp[COMP_SIZE], vec2 uv, float sz, out float depths[KERNEL_SIZE])
	{
		for (int y = 0; y < SAMPLE_SIZE; y++)
		{
			for (int x = 0; x < SAMPLE_SIZE; x++)
			{
				int idx = x + y * SAMPLE_SIZE;
				depths[idx] = bilerp(step(vec4(sz), csmExtractComp9(comp, x, y)), uv);
			}
		}
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

		csmReadDepthComponents9(roundedShadowCoord, comp, texDepths);
		csmBuildDepthArray9(comp, fractShadowCoord, shadowCoord.z, depths);

		float acc = 0.0;
		#ifndef CSM_USE_IDENTITY_KERNEL
		float accDiv = 0.0;
		#else
		float accDiv = float(depths.length());
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
		#ifdef CSM_KERNEL_SIZE_9
		case 9:
			return csmVisibilityPCF9(shadowCoord, csm, texDepths);
		#endif
		#ifdef CSM_KERNEL_SIZE_7
		case 7:
			return csmVisibilityPCF7(shadowCoord, csm, texDepths);
		#endif
		#ifdef CSM_KERNEL_SIZE_5
		case 5:
			return csmVisibilityPCF5(shadowCoord, csm, texDepths);
		#endif
		#ifdef CSM_KERNEL_SIZE_3
		case 3:
			return csmVisibilityPCF3(shadowCoord, csm, texDepths);
		#endif
		#ifdef CSM_KERNEL_SIZE_1
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

	float zBias = -clamp(0.001 * tan(acos(cosTheta)), 0.0, 0.01);

	if (csm.blendCascades)
	{
		float radiusPadding = 0.9;
		float radiusPadding2 = radiusPadding * radiusPadding;

		for (int i = 0; i < csm.splits; i++)
		{
			vec4 shadowCoord = csm.MVP[i] * position;
			shadowCoord.z -= zBias;
			shadowCoord.w = i;

			vec3 camCenterPosDiff = csm.centers[i] - position.xyz;
			float camDistance2 = dot(camCenterPosDiff, camCenterPosDiff);

			if (csm.radiuses2[i] > camDistance2)
			{
				visibility = csmVisibilityPCF(shadowCoord, csm, texDepths);

				if (csm.radiuses2[i] * radiusPadding2 < camDistance2)
				{
					float visibility2 = 1.0;

					if (i + 1 < csm.splits)
					{
						vec4 shadowCoord2 = csm.MVP[i + 1] * position;
						shadowCoord2.z -= zBias;
						shadowCoord2.w = i + 1;

						visibility2 = csmVisibilityPCF(shadowCoord2, csm, texDepths);
					}

					float blend = smoothstep(csm.radiuses2[i] * radiusPadding2, csm.radiuses2[i], camDistance2);

					visibility = mix(visibility, visibility2, blend);
				}

				break;
			}
		}
	}
	else
	{
		for (int i = 0; i < csm.splits; i++)
		{
			vec4 shadowCoord = csm.MVP[i] * position;
			shadowCoord.z -= zBias;
			shadowCoord.w = i;

			vec3 camCenterPosDiff = csm.centers[i] - position.xyz;
			float camDistance2 = dot(camCenterPosDiff, camCenterPosDiff);

			if (csm.radiuses2[i] > camDistance2)
			{
				visibility = csmVisibilityPCF(shadowCoord, csm, texDepths);
				break;
			}
		}
	}

	return pow(visibility, 2.2);
	// return visibility * visibility;
	// return visibility;
}