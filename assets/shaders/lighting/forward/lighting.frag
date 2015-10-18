#version 440 core

layout(location = 0) out vec4 outColor;

uniform vec4 matDiffuse;
uniform float matShininess;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat4 invV;

uniform vec2 fbScale;

uniform sampler2D texAO;
uniform samplerBuffer lightD;
uniform samplerBuffer lightP;
uniform int lightDCount;
uniform int lightPCount;

#define CSM_MAX_CASCADES 5
uniform bool csmBlendCascades;
uniform int csmKernelSize;
uniform int csmSplits;
uniform float csmCascades[CSM_MAX_CASCADES];
uniform float csmRadiuses2[CSM_MAX_CASCADES];
uniform vec3 csmCenters[CSM_MAX_CASCADES];
uniform mat4 csmMVP[CSM_MAX_CASCADES];

uniform sampler2DArray csmTexDepths;

in vec4 position;
in vec4 positionV;
in vec3 normal;

int lightDOffset = 3;
int lightPOffset = 4;

float lightAmbient()
{
	return 0.0;
}

float lightDiffuse()
{
	return 0.0;
}

float G1V(float n_v, float k)
{
	return 1.0 / (n_v * (1.0 - k) + k);
}

float fresnel(float cosTheta, float f0, float power)
{
	float facing = 1.0 - cosTheta;
	return max(f0 + (1.0 - f0) * pow(facing, power), 0.0);
}

float lightSpecular(vec3 n, vec3 v, vec3 l, float roughness, float f0)
{
	float alpha = roughness * roughness;

	vec3 h = normalize(v + l);

	float n_l = max(0.0, dot(n, l));
	float n_v = max(0.0, dot(n, v));
	float n_h = max(0.0, dot(n, h));
	float l_h = max(0.0, dot(l, h));

	float f, d, g;

	// D
	float alpha2 = alpha * alpha;
	float pi = 3.141592;
	float denom = n_h * n_h * (alpha2 - 1.0) + 1.0;
	d = alpha2 / (pi * denom * denom);

	// F
	float l_h5 = pow(1.0 - l_h, 5.0);
	f = f0 + (1.0 - f0) * l_h5;

	// G
	float k = alpha / 2.0;
	g = G1V(n_l, k) * G1V(n_v, k);

	return (d * f * g) * n_l;
}

float bilerp(vec4 comp, vec2 uv, float sz)
{
	vec4 results = step(vec4(sz), comp);
	vec2 mix1 = mix(results.wx, results.zy, uv.xx);
	float mix2 = mix(mix1[0], mix1[1], uv.y);

	return mix2;
}

void readDepthComponents4(vec3 uvl, out vec4 comp[4])
{
	int s = 2;
	for (int i = 0; i < s; i++)
	{
		for (int j = 0; j < s; j++)
		{
			int idx = i * s + j;
			ivec2 offset = ivec2(j, i) * 2 - s;

			comp[idx] = textureGatherOffset(csmTexDepths, uvl, offset, 0);
		}
	}
}

void readDepthComponents9(vec3 uvl, out vec4 comp[9])
{
	int s = 3;
	for (int i = 0; i < s; i++)
	{
		for (int j = 0; j < s; j++)
		{
			int idx = i * s + j;
			ivec2 offset = ivec2(j, i) * 2 - s;

			comp[idx] = textureGatherOffset(csmTexDepths, uvl, offset, 0);
		}
	}
}

void readDepthComponents16(vec3 uvl, out vec4 comp[16])
{
	int s = 4;
	for (int i = 0; i < s; i++)
	{
		for (int j = 0; j < s; j++)
		{
			int idx = i * s + j;
			ivec2 offset = ivec2(j, i) * 2 - s;

			comp[idx] = textureGatherOffset(csmTexDepths, uvl, offset, 0);
		}
	}
}

void readDepthComponents25(vec3 uvl, out vec4 comp[25])
{
	int s = 5;
	for (int i = 0; i < s; i++)
	{
		for (int j = 0; j < s; j++)
		{
			int idx = i * s + j;
			ivec2 offset = ivec2(j, i) * 2 - s;

			comp[idx] = textureGatherOffset(csmTexDepths, uvl, offset, 0);
		}
	}
}

vec4 extractComp4(vec4 comp[4], int x, int y)
{
	int s = 2;
	if ((x % 2) == 0 && (y % 2) == 0)
	{
		int idx = x / 2 + y / 2 * s;
		return comp[idx].xyzw;
	}

	if ((x % 2) == 0 && (y % 2) == 1)
	{
		int idx = x / 2 + (y - 1) / 2 * s;
		return vec4(comp[idx + s].wz, comp[idx].yx);
	}

	if ((x % 2) == 1 && (y % 2) == 0)
	{
		int idx = (x - 1) / 2 + y / 2 * s;
		return vec4(comp[idx].y, comp[idx + 1].xw, comp[idx].z);
	}

	int idx = (x - 1) / 2 + (y - 1) / 2 * s;
	return vec4(comp[idx + s].z, comp[idx + s + 1].w, comp[idx + 1].x, comp[idx].y);
}

vec4 extractComp9(vec4 comp[9], int x, int y)
{
	int s = 3;
	if ((x % 2) == 0 && (y % 2) == 0)
	{
		int idx = x / 2 + y / 2 * s;
		return comp[idx].xyzw;
	}

	if ((x % 2) == 0 && (y % 2) == 1)
	{
		int idx = x / 2 + (y - 1) / 2 * s;
		return vec4(comp[idx + s].wz, comp[idx].yx);
	}

	if ((x % 2) == 1 && (y % 2) == 0)
	{
		int idx = (x - 1) / 2 + y / 2 * s;
		return vec4(comp[idx].y, comp[idx + 1].xw, comp[idx].z);
	}

	int idx = (x - 1) / 2 + (y - 1) / 2 * s;
	return vec4(comp[idx + s].z, comp[idx + s + 1].w, comp[idx + 1].x, comp[idx].y);
}

vec4 extractComp16(vec4 comp[16], int x, int y)
{
	int s = 4;
	if ((x % 2) == 0 && (y % 2) == 0)
	{
		int idx = x / 2 + y / 2 * s;
		return comp[idx].xyzw;
	}

	if ((x % 2) == 0 && (y % 2) == 1)
	{
		int idx = x / 2 + (y - 1) / 2 * s;
		return vec4(comp[idx + s].wz, comp[idx].yx);
	}

	if ((x % 2) == 1 && (y % 2) == 0)
	{
		int idx = (x - 1) / 2 + y / 2 * s;
		return vec4(comp[idx].y, comp[idx + 1].xw, comp[idx].z);
	}

	int idx = (x - 1) / 2 + (y - 1) / 2 * s;
	return vec4(comp[idx + s].z, comp[idx + s + 1].w, comp[idx + 1].x, comp[idx].y);
}

vec4 extractComp25(vec4 comp[25], int x, int y)
{
	int s = 5;
	if ((x % 2) == 0 && (y % 2) == 0)
	{
		int idx = x / 2 + y / 2 * s;
		return comp[idx].xyzw;
	}

	if ((x % 2) == 0 && (y % 2) == 1)
	{
		int idx = x / 2 + (y - 1) / 2 * s;
		return vec4(comp[idx + s].wz, comp[idx].yx);
	}

	if ((x % 2) == 1 && (y % 2) == 0)
	{
		int idx = (x - 1) / 2 + y / 2 * s;
		return vec4(comp[idx].y, comp[idx + 1].xw, comp[idx].z);
	}

	int idx = (x - 1) / 2 + (y - 1) / 2 * s;
	return vec4(comp[idx + s].z, comp[idx + s + 1].w, comp[idx + 1].x, comp[idx].y);
}

void buildDepthArray9(vec4 comp[4], vec2 uv, float sz, out float depths[9])
{
	int s = 3;
	for (int y = 0; y < s; y++)
	{
		for (int x = 0; x < s; x++)
		{
			int idx = x + y * s;
			depths[idx] = bilerp(extractComp4(comp, x, y), uv, sz);
		}
	}
}

void buildDepthArray25(vec4 comp[9], vec2 uv, float sz, out float depths[25])
{
	int s = 5;
	for (int y = 0; y < s; y++)
	{
		for (int x = 0; x < s; x++)
		{
			int idx = x + y * s;
			depths[idx] = bilerp(extractComp9(comp, x, y), uv, sz);
		}
	}
}

void buildDepthArray49(vec4 comp[16], vec2 uv, float sz, out float depths[49])
{
	int s = 7;
	for (int y = 0; y < s; y++)
	{
		for (int x = 0; x < s; x++)
		{
			int idx = x + y * s;
			depths[idx] = bilerp(extractComp16(comp, x, y), uv, sz);
		}
	}
}

void buildDepthArray81(vec4 comp[25], vec2 uv, float sz, out float depths[81])
{
	int s = 9;
	for (int y = 0; y < s; y++)
	{
		for (int x = 0; x < s; x++)
		{
			int idx = x + y * s;
			depths[idx] = bilerp(extractComp25(comp, x, y), uv, sz);
		}
	}
}

float visibilityPCF3(vec4 shadowCoord)
{
	vec3 csmTexDepthsSize = vec3(1.0 / (textureSize(csmTexDepths, 0).xy), 0.0);
	vec2 fractShadowCoord = fract(shadowCoord.xy * textureSize(csmTexDepths, 0).xy - 0.5);
	vec3 roundedShadowCoord = shadowCoord.xyw - vec3(fractShadowCoord * csmTexDepthsSize.xy, 0.0);

	vec4 comp4[4];
	float depths9[9];

	float kernel9[9] = float[9](
		0.5, 1.0, 0.5,
		1.0, 1.0, 1.0,
		0.5, 1.0, 0.5
	);

	readDepthComponents4(roundedShadowCoord, comp4);
	buildDepthArray9(comp4, fractShadowCoord, shadowCoord.z, depths9);

	float acc = 0.0;
	float accDiv = 0.0;

	for (int i = 0; i < 9; i++)
	{
		accDiv += kernel9[i];
		kernel9[i] = depths9[i] * kernel9[i];
		acc += kernel9[i];
	}

	return acc / accDiv;
}

float visibilityPCF5(vec4 shadowCoord)
{
	vec3 csmTexDepthsSize = vec3(1.0 / (textureSize(csmTexDepths, 0).xy), 0.0);
	vec2 fractShadowCoord = fract(shadowCoord.xy * textureSize(csmTexDepths, 0).xy - 0.5);
	vec3 roundedShadowCoord = shadowCoord.xyw - vec3(fractShadowCoord * csmTexDepthsSize.xy, 0.0);

	vec4 comp9[9];
	float depths25[25];

	float kernel25[25] = float[25](
		0.0, 0.5, 1.0, 0.5, 0.0,
		0.5, 1.0, 1.0, 1.0, 0.5,
		1.0, 1.0, 1.0, 1.0, 1.0,
		0.5, 1.0, 1.0, 1.0, 0.5,
		0.0, 0.5, 1.0, 0.5, 0.0
	);

	readDepthComponents9(roundedShadowCoord, comp9);
	buildDepthArray25(comp9, fractShadowCoord, shadowCoord.z, depths25);

	float acc = 0.0;
	float accDiv = 0.0;

	for (int i = 0; i < 25; i++)
	{
		accDiv += kernel25[i];
		kernel25[i] = depths25[i] * kernel25[i];
		acc += kernel25[i];
	}

	return acc / accDiv;
}

float visibilityPCF7(vec4 shadowCoord)
{
	vec3 csmTexDepthsSize = vec3(1.0 / (textureSize(csmTexDepths, 0).xy), 0.0);
	vec2 fractShadowCoord = fract(shadowCoord.xy * textureSize(csmTexDepths, 0).xy - 0.5);
	vec3 roundedShadowCoord = shadowCoord.xyw - vec3(fractShadowCoord * csmTexDepthsSize.xy, 0.0);

	vec4 comp16[16];
	float depths49[49];

	float kernel49[49] = float[49](
		0.00, 0.25, 0.50, 1.00, 0.50, 0.25, 0.00,
		0.25, 0.75, 1.00, 1.00, 1.00, 0.75, 0.25,
		0.50, 1.00, 1.00, 1.00, 1.00, 1.00, 0.50,
		1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00,
		0.50, 1.00, 1.00, 1.00, 1.00, 1.00, 0.50,
		0.25, 0.75, 1.00, 1.00, 1.00, 0.75, 0.25,
		0.00, 0.25, 0.50, 1.00, 0.50, 0.25, 0.00
	);

	readDepthComponents16(roundedShadowCoord, comp16);
	buildDepthArray49(comp16, fractShadowCoord, shadowCoord.z, depths49);

	float acc = 0.0;
	float accDiv = 0.0;

	for (int i = 0; i < 49; i++)
	{
		accDiv += kernel49[i];
		kernel49[i] = depths49[i] * kernel49[i];
		acc += kernel49[i];
	}

	return acc / accDiv;
}

float visibilityPCF9(vec4 shadowCoord)
{
	vec3 csmTexDepthsSize = vec3(1.0 / (textureSize(csmTexDepths, 0).xy), 0.0);
	vec2 fractShadowCoord = fract(shadowCoord.xy * textureSize(csmTexDepths, 0).xy - 0.5);
	vec3 roundedShadowCoord = shadowCoord.xyw - vec3(fractShadowCoord * csmTexDepthsSize.xy, 0.0);

	vec4 comp25[25];
	float depths81[81];

	float kernel81[81] = float[81](
		0.00, 0.25, 0.50, 0.75, 1.00, 0.75, 0.50, 0.25, 0.00,
		0.25, 0.50, 0.75, 1.00, 1.00, 1.00, 0.75, 0.50, 0.25,
		0.50, 0.75, 1.00, 1.00, 1.00, 1.00, 1.00, 0.75, 0.50,
		0.75, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 0.75,
		1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00,
		0.75, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 0.75,
		0.50, 0.75, 1.00, 1.00, 1.00, 1.00, 1.00, 0.75, 0.50,
		0.25, 0.50, 0.75, 1.00, 1.00, 1.00, 0.75, 0.50, 0.25,
		0.00, 0.25, 0.50, 0.75, 1.00, 0.75, 0.50, 0.25, 0.00
	);

	readDepthComponents25(roundedShadowCoord, comp25);
	buildDepthArray81(comp25, fractShadowCoord, shadowCoord.z, depths81);

	float acc = 0.0;
	float accDiv = 0.0;

	for (int i = 0; i < 81; i++)
	{
		accDiv += kernel81[i];
		kernel81[i] = depths81[i] * kernel81[i];
		acc += kernel81[i];
	}

	return acc / accDiv;
}

float visibilityPCF(vec4 shadowCoord)
{
	shadowCoord.z -= 0.01;

	switch (csmKernelSize)
	{
		case 9:
			return visibilityPCF9(shadowCoord);
		case 7:
			return visibilityPCF7(shadowCoord);
		case 5:
			return visibilityPCF5(shadowCoord);
		case 3:
		default:
			return visibilityPCF3(shadowCoord);
	}
}

float visibility(vec4 position)
{
	float radiusPadding = 0.9;
	float radiusPadding2 = radiusPadding * radiusPadding;

	if (csmBlendCascades)
	{
		for (int i = 0; i < csmSplits; i++)
		{
			vec4 shadowCoord = csmMVP[i] * invV * position;
			shadowCoord.xy = shadowCoord.xy * 0.5 + 0.5;
			shadowCoord.w = i;

			vec3 camCenterPosDiff = csmCenters[i] - position.xyz;
			float camDistance2 = dot(camCenterPosDiff, camCenterPosDiff);

			if (csmRadiuses2[i] > camDistance2)
			{
				float visibility = visibilityPCF(shadowCoord);

				if (csmBlendCascades && csmRadiuses2[i] * radiusPadding2 < camDistance2)
				{
					float visibility2 = 1.0;

					if (i + 1 < csmSplits)
					{
						vec4 shadowCoord2 = csmMVP[i + 1] * invV * position;
						shadowCoord2.xy = shadowCoord2.xy * 0.5 + 0.5;
						shadowCoord2.w = i + 1;

						visibility2 = visibilityPCF(shadowCoord2);
					}

					float blend = smoothstep(csmRadiuses2[i] * radiusPadding2, csmRadiuses2[i], camDistance2);

					visibility = mix(visibility, visibility2, blend);
				}

				return visibility;
			}
		}
	}
	else
	{
		for (int i = 0; i < csmSplits; i++)
		{
			vec4 shadowCoord = csmMVP[i] * invV * position;
			shadowCoord.xy = shadowCoord.xy * 0.5 + 0.5;
			shadowCoord.w = i;

			vec3 camCenterPosDiff = csmCenters[i] - position.xyz;
			float camDistance2 = dot(camCenterPosDiff, camCenterPosDiff);

			if (csmRadiuses2[i] > camDistance2)
			{
				return visibilityPCF(shadowCoord);
			}
		}
	}

	return 1.0;
}

vec3 smootherstep(vec3 edge0, vec3 edge1, vec3 x)
{
	// Scale, and clamp x to 0..1 range
	x = clamp((x - edge0)/(edge1 - edge0), vec3(0.0), vec3(1.0));
	// Evaluate polynomial
	return x * x * x * (x * (x * 6.0 - 15.0) + 10.0);
}

void main()
{
	vec2 ssuv = gl_FragCoord.xy * fbScale;
	vec4 diffuseAlbedo = pow(matDiffuse, vec4(2.2));
	float shininess = matShininess;

	vec3 n = normalize(normal);
	vec3 v = normalize(-position.xyz);

	float cosTheta = 0.0;

	for (int i = 0; i < lightDCount; i++)
	{
		vec4 lAmbient = texelFetch(lightD, lightDOffset * i + 0).rgba;
		vec4 lColor = pow(texelFetch(lightD, lightDOffset * i + 1).rgba, vec4(2.2));
		vec4 lDirectionIntensity = texelFetch(lightD, lightDOffset * i + 2).rgba;
		vec3 lDirection = lDirectionIntensity.xyz;
		float lIntensity = lDirectionIntensity.a;

		vec3 l = lDirection;

		float n_l = max(0.0, dot(n, l));
		cosTheta = n_l;

		float diffuse = 1.0 - fresnel(n_l, 0.82, 1.0);
		float specular = lightSpecular(n, v, l, shininess, 0.82);

		outColor.rgb += lColor.rgb * (diffuse + specular);
	}

	vec3 lightColor = diffuseAlbedo.rgb * outColor.rgb * texture(texAO, ssuv).r;

	outColor.rgb = lightColor * visibility(position);

	outColor.rgb = pow(outColor.rgb, vec3(1.0/2.2));
	outColor.a = 1.0;
}