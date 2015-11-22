#version 440 core

#define CSM_KERNEL_SIZE_7
#pragma rn: include(fx/csm/csm.glsl)

layout(location = 0) out vec4 outColor;

struct PointLight
{
	vec4 color;
	vec4 position;
	float intensity;
	float radius;
	float cutoff;
	float padding;
};

struct DirectionalLight
{
	vec4 ambient;
	vec4 color;
	vec3 direction;
	float intensity;
};

layout(std430, binding = 10) buffer PointLightData
{
	PointLight pointLightData[];
};

layout(std430, binding = 11) buffer DirectionalLightData
{
	DirectionalLight directionalLightData[];
};

uniform vec4 matDiffuse;
uniform float matShininess;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat4 invV;

uniform vec2 fbScale;

uniform sampler2D texAO;

uniform CSM csm;
uniform sampler2DArray csmTexDepths;

in vec4 position;
in vec4 positionV;
in vec3 normal;

int lightDOffset = 3;
int lightPOffset = 3;

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
		return csmVisibility(position, cosTheta, csm, csmTexDepths);
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
		float dPrime = d / (1.0 - denom * denom);

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

void main()
{
	vec2 ssuv = gl_FragCoord.xy * fbScale;
	vec4 diffuseAlbedo = pow(matDiffuse, vec4(2.2));
	float shininess = matShininess;

	vec3 n = normalize(normal);
	vec3 v = normalize(-position.xyz);
	float ao = texture(texAO, ssuv).r;

	for (int i = 0; i < directionalLightData.length(); i++)
	{
		vec4 lAmbient = directionalLightData[i].ambient;
		vec4 lColor = directionalLightData[i].color;
		vec3 lDirection = directionalLightData[i].direction;
		float lIntensity = directionalLightData[i].intensity;

		vec3 l = lDirection;

		float n_l = max(0.0, dot(n, l));

		float visibility = directionalLightVisibility(position, n_l, csm, csmTexDepths);

		if (visibility > 0.0)
		{
			float diffuse = 1.0 - fresnel(n_l, 0.82, 1.0);
			float specular = lightSpecular(n, v, l, shininess, 0.82);

			outColor.rgb += lColor.rgb * lIntensity * (diffuse + specular) * visibility;
		}

		outColor.rgb += lAmbient.rgb * ao;
	}

	for (int i = 0; i < pointLightData.length(); i++)
	{
		vec4 lColor = pointLightData[i].color;
		vec4 lPosition = pointLightData[i].position;
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
			float specular = lightSpecular(n, v, l, shininess, 0.82);

			outColor.rgb += lColor.rgb * lIntensity * (diffuse + specular) * visibility;
		}
	}

	outColor.rgb = pow(outColor.rgb * diffuseAlbedo.rgb, vec3(1.0/2.2));
	outColor.a = 1.0;
}