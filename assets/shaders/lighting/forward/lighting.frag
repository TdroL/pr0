#version 440 core

// #pragma optionNV(fastmath on)
// #pragma optionNV(fastprecision on)
// #pragma optionNV(ifcvt none)
// #pragma optionNV(inline all)
// #pragma optionNV(unroll all)
#pragma optionNV(strict on)

#pragma rn: include(fx/csm/csm.glsl)

layout(location = 0) out vec4 outColor;

// layout(std140, binding = 0) uniform MatricesData
// {
// 	mat4 M;
// 	mat4 V;
// 	mat4 P;

// 	mat4 invM;
// 	mat4 invV;
// 	mat4 invP;
// };

layout(std140, binding = 1) uniform CSMData
{
	CSM csmData;
};
uniform sampler2DArray texCSMDepths;

struct PointLight
{
	vec3 color;
	float intensity;
	vec3 position;
	float radius;
	float cutoff;
};

struct DirectionalLight
{
	vec3 ambient;
	float intensity;
	vec3 color;
	int diffuseOnly;
	vec3 direction;
	int shadowCaster;
};

layout(std430, binding = 0) buffer PointLightData
{
	PointLight pointLightData[];
};

layout(std430, binding = 1) buffer DirectionalLightData
{
	DirectionalLight directionalLightData[];
};

uniform vec4 matDiffuse;
uniform float matRoughness;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat4 invV;

uniform vec2 fbScale;

uniform sampler2D texAO;

in vec4 position;
in vec4 positionV;
in vec3 normal;

int lightDOffset = 3;
int lightPOffset = 3;

float G1V(float n_v, float k)
{
	return 1.0 / (n_v * (1.0 - k) + k);
}

float fresnel(float n_l, float f0, float power)
{
	float facing = 1.0 - n_l;
	return max(f0 + (1.0 - f0) * pow(facing, power), 0.0);
}

float lightDiffuseLambert(vec3 l, vec3 v, vec3 n, float roughness)
{
	float n_l = dot(n, l);

	return clamp(n_l, 0.0, 1.0);
}

float lightDiffuseOrenNayar(vec3 l, vec3 v, vec3 n, float roughness)
{
	float sigma2 = roughness * roughness;

	float n_v = dot(n, v);
	float n_l = dot(n, l);
	float l_v = dot(l, v);
	float theta_r = acos(n_v);
	float theta_i = acos(n_l);

	float alpha = max(theta_r, theta_i);
	float beta = min(theta_r, theta_i);
	float gamma = dot(normalize(v - n * n_v), normalize(l - n * n_l));

	float A = 1.0 - 0.5 * sigma2 / (sigma2 + 0.33);
	float B = 0.45 * sigma2 / (sigma2 + 0.09);

	return max(0.0, n_l) * (A + B * max(0.0, gamma) * sin(alpha) * tan(beta));
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
	// float l_h5 = pow(1.0 - l_h, 5.0);
	// f = f0 + (1.0 - f0) * l_h5;
	f = fresnel(l_h, f0, 5.0);

	// G
	float k = alpha / 2.0;
	g = G1V(n_l, k) * G1V(n_v, k);

	return (d * f * g) * n_l;
}

float directionalLightVisibility(vec4 position, float cosTheta, struct CSM csm, sampler2DArray texDepths)
{
	if (cosTheta > 0.0)
	{
		return csmVisibility(position, cosTheta, csm, texDepths);
	}

	return 0.0;
}

float pointLightVisibility(float d, float cosTheta, float cutoff, float r)
{
	if (cosTheta > 0.0)
	{
		// https://imdoingitwrong.wordpress.com/tag/attenuation/
		d = max(0.0, d - r);
		d = min(d, cutoff);
		r = max(0.001, r);

		float denom = d / cutoff;
		denom = (d / ((1.0 - denom * denom) * r) + 1.0);

		return cosTheta / (denom * denom);
	}

	return 0.0;
}

vec3 smootherstep(vec3 edge0, vec3 edge1, vec3 x)
{
	// Scale, and clamp x to 0..1 range
	x = clamp((x - edge0)/(edge1 - edge0), vec3(0.0), vec3(1.0));
	// Evaluate polynomial
	return x * x * x * (x * (x * 6.0 - 15.0) + 10.0);
}

vec3 calcLighting(vec3 n, vec3 v, float roughness)
{
	vec2 ssuv = gl_FragCoord.xy * fbScale;
	float ao = texture(texAO, ssuv).r;

	vec3 color = vec3(0.0);

	for (int i = 0; i < directionalLightData.length(); i++)
	{
		vec3 lAmbient = directionalLightData[i].ambient;
		vec3 lColor = directionalLightData[i].color;
		vec3 lDirection = directionalLightData[i].direction;
		float lIntensity = directionalLightData[i].intensity;
		bool lDffuseOnly = bool(directionalLightData[i].diffuseOnly);
		bool lShadowCaster = bool(directionalLightData[i].shadowCaster);

		vec3 l = lDirection.xyz;

		float n_l = max(0.0, dot(n, l));

		float visibility = 1.0;
		if (lShadowCaster)
		{
			visibility = directionalLightVisibility(position, n_l, csmData, texCSMDepths);
		}

		if (visibility > 0.0)
		{
			float diffuse = n_l;
			float specular = 0.0;

			if ( ! lDffuseOnly)
			{
				diffuse = 1.0 - fresnel(n_l, 0.82, 1.0);
				specular = lightSpecular(n, v, l, roughness, 0.82);
			}

			color.rgb += lColor.rgb * lIntensity * (diffuse + specular) * visibility;
		}

		color.rgb += lAmbient.rgb * lIntensity * ao;
	}

	for (int i = 0; i < pointLightData.length(); i++)
	{
		vec3 lColor = pointLightData[i].color;
		vec3 lPosition = pointLightData[i].position;
		float lIntensity = pointLightData[i].intensity;
		float lRadius = pointLightData[i].radius;
		float lCutoff = pointLightData[i].cutoff;

		float d = length(lPosition.xyz - position.xyz);
		vec3 l = normalize(lPosition.xyz - position.xyz);
		float n_l = max(0.0, dot(n, l));

		float visibility = pointLightVisibility(d, n_l, lCutoff, lRadius);

		if (visibility > 0.0)
		{
			float diffuse = 1.0 - fresnel(n_l, 0.82, 1.0);
			float specular = lightSpecular(n, v, l, roughness, 0.82);

			color.rgb += lColor.rgb * lIntensity * (diffuse + specular) * visibility;
		}
	}

	return color;
}

void main()
{
	vec4 diffuseAlbedo = pow(matDiffuse, vec4(2.2));
	float roughness = matRoughness;

	vec3 n = normalize(normal);
	vec3 v = normalize(-position.xyz);

	outColor.rgb = outColor.rgb * diffuseAlbedo.rgb;
	outColor.a = 1.0;

	int layer = 0;
	vec4 shadowCoord = csmData.layers[layer].MVP * (position + vec4(normal, 0.0) * 0.00001); // - vec4(normal, 0.0) * 0.0004
	shadowCoord.w = float(layer);

	float visibility = float(texture(texCSMDepths, shadowCoord.xyw).r > shadowCoord.z - 0.0008);

	vec3 lDirection = directionalLightData[0].direction;
	vec3 l = lDirection.xyz;

	float n_l = max(0.0, dot(n, l));

	outColor.rgb = vec3(min(visibility, n_l));
	// outColor.rgb = vec3(n_l);
	outColor.rgb = normal;
	outColor.rgb = position.xyz;
	outColor.rgb = position.xyz + normal * 0.0001;
	outColor.rgb = positionV.xyz;

	// visibility = directionalLightVisibility(position, n_l, csmData, texCSMDepths);

	outColor.rgb = vec3(visibility);

	// outColor.rgb = vec3(1.0) * lightDiffuseOrenNayar(l, v, n, roughness) * visibility;
}