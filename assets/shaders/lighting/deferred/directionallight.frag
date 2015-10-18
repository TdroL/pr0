#version 440 core

layout(location = 0) out vec4 outColor;

in vec2 uv;

uniform mat4 invP;
uniform mat4 invV;

uniform float zNear;
uniform float zFar;

uniform sampler2D texNormal;
uniform sampler2D texColor;
uniform sampler2D texZ;
uniform sampler2D texDepth;
uniform sampler2D texAO;
uniform sampler2D texShadowMoments;
uniform sampler2D texShadowDepth;
uniform sampler2DArray csmTexCascades;
uniform sampler2DArrayShadow csmTexDepths;

uniform mat4 shadowmapMVP;

#define CSM_MAX_CASCADES 4
uniform int csmSplits;
uniform float csmCascades[CSM_MAX_CASCADES];
uniform float csmRadiuses2[CSM_MAX_CASCADES];
uniform vec3 csmCenters[CSM_MAX_CASCADES];
uniform mat4 csmMVP[CSM_MAX_CASCADES];

uniform vec4 lightAmbient;
uniform vec4 lightColor;
uniform vec3 lightDirection;
uniform float lightIntensity;
uniform uint useColor;

float kernelSize = 0.0;
bool fixZValue = true;
bool showConstCascade = false;
bool useLinearSplits = false;
bool enableBlending = false;
bool enableVisualizeCascades = false;

vec3 normalDecode(vec2 enc);
vec3 positionReconstruct(float z, vec2 uv);
vec4 positionReconstruct2(sampler2D texDepth, vec2 uv, mat4 invP);
float vsmVisibility(float dist, vec2 moments);

float pcfVisibility(vec3 shadowCoord, int csmLayer, float kernelSize)
{
	vec2 texSize = textureSize(csmTexDepths, 0).xy;
	vec2 texSizeInv = 1.0 / texSize;
	float depthBias = texSizeInv.x;

	vec2 shadowCoord2 = floor(shadowCoord.xy);
	vec2 shadowCoord3 = fract(shadowCoord.xy);

	texSizeInv /= float(csmLayer + 1.0);

	float visibility = 0.0;
	float weight = 0.0;
	float origKernelSize = kernelSize;
	kernelSize = floor(kernelSize / 2.0);

	for (float i = -kernelSize; i <= kernelSize; i += 1.0) {
		for (float j = -kernelSize; j <= kernelSize; j += 1.0) {
			float lightDepth = shadowCoord.z - depthBias;
			vec4 testCoords = vec4(shadowCoord.xy + vec2(i, j) * texSizeInv, csmLayer, lightDepth);

			visibility += texture(csmTexDepths, testCoords);

			weight += 1.0;
		}
	}

	visibility /= max(weight, 1.0);

	return visibility;
}

int calcCascade(vec3 position)
{
	if (useLinearSplits)
	{
		for (int i = 0; i < csmSplits; i++)
		{
			if (position.z > csmCascades[i])
			{
				return i;
			}
		}
	}
	else
	{
		for (int i = 0; i < csmSplits - 1; i++)
		{
			vec3 len = csmCenters[i] - position;
			float dist2 = dot(len, len);

			if (dist2 <= csmRadiuses2[i])
			{
				return i;
			}
		}

		if (position.z > csmCascades[csmSplits - 1])
		{
			return csmSplits - 1;
		}
	}

	return -1;
}

float calcVisibility(vec3 position)
{
	int csmLayer = calcCascade(position);

	if (csmLayer == -1) {
		return 1.0;
	}

	vec4 shadowCoord = csmMVP[csmLayer] * invV * vec4(position, 1.0) * 0.5 + 0.5;

	float visibility = pcfVisibility(shadowCoord.xyz, csmLayer, kernelSize);

	if (enableBlending && csmLayer < csmSplits - 1)
	{
		float blendScale = 16.0;

		float dl = abs((position.z - csmCascades[csmLayer]) / (csmCascades[csmLayer + 1] - csmCascades[csmLayer])) * blendScale;

		if (dl < 1.0)
		{
			shadowCoord = csmMVP[csmLayer + 1] * invV * vec4(position, 1.0) * 0.5 + 0.5;

			float nextVisibility = pcfVisibility(shadowCoord.xyz, csmLayer + 1, kernelSize);

			visibility = min(visibility, mix(visibility, nextVisibility, 1.0 - dl));
		}
	}

	return visibility;
}

float cookTorranceDiffuseGGX(vec3 n, vec3 h, float alpha2)
{
	float pi = 3.141592;
	float n_h = dot(n, h);

	float chi = sign(max(0.0, n_h));
	float n_h2 = n_h * n_h;

	float den = n_h2 * alpha2 + (1.0 - n_h2);

	return chi * alpha2 / (pi * den * den);
}

vec3 cookTorranceFresnelSchlick(vec3 v, vec3 h, vec3 f0)
{
	float v_h = dot(v, h);
	return f0 + (1.0 - f0) * pow(1.0 - v_h, 5.0);
}

float cookTorranceGeometry(vec3 v, vec3 h, vec3 n, float alpha2)
{
	float v_h = max(0.0, dot(v, h));
	float v_n = max(0.0, dot(v, n));
	float v_h2 = v_h * v_h;

	float chi = sign(max(0.0, v_h / v_n));

	float tan2 = (1.0 - v_h) / v_h;
	return chi * 2.0 / (1.0 + sqrt(1.0 + alpha2 * tan2));

	return 0.0;
}

vec3 cookTorrance(vec3 n, vec3 l, vec3 v, float alpha2, vec3 albedo, vec3 ior, float metallic)
{
	vec3 h = normalize(v + l);

	float l_n = dot(l, n);
	float v_n = dot(v, n);

	vec3 f0 = abs((1.0 - ior) / (1.0 + ior));
	f0 = f0 * f0;
	f0 = mix(f0, albedo, metallic);

	float D = cookTorranceDiffuseGGX(n, h, alpha2);
	vec3 F = cookTorranceFresnelSchlick(v, h, f0);
	float G = cookTorranceGeometry(v, h, n, alpha2);
	return (D * F * G) / (4.0 * l_n * v_n);
}

void main()
{
	vec2 encodedNormal = texture(texNormal, uv).xy;
	vec4 diffuseShininess = texture(texColor, uv);
	float depth = 1.0 - texture(texDepth, uv).x;

	vec3 albedo = diffuseShininess.rgb;
	float shininess = diffuseShininess.a;

	vec3 normal = normalDecode(encodedNormal);
	// vec3 position = positionReconstruct(depth, uv);
	vec3 position = positionReconstruct2(texDepth, uv, invP).xyz;

	if (fixZValue)
	{
		position.z = texture(texZ, uv).x; // fix z precision
	}

	if (useColor == 0u)
	{
		// albedo = vec3(0.5);
	}

	float visibility = 1.0; //calcVisibility(position);

	if (showConstCascade)
	{
		visibility = 1.0;
	}

	float occlusion = texture(texAO, uv).r;

	vec3 n = normalize(normal);
	vec3 l = normalize(lightDirection);
	vec3 v = normalize(-position);
	vec3 h = normalize(l + v);

	float n_l = dot(n, l);
	float n_h = dot(n, h);
	float n_v = dot(n, v);
	float h_v = dot(h, v);

	float alpha = acos(n_h);
	float theta = max(n_l, 0.0);

	float exponent = alpha / shininess;
	float gauss = sign(theta) * exp(-(exponent * exponent));

	vec3 lightAmbientG = pow(lightAmbient.rgb, vec3(2.2));
	vec3 lightColorG = pow(lightColor.rgb, vec3(2.2));

	float roughness = shininess;
	float roughness2 = roughness * roughness;

	float fresnel = pow(1.0 + n_v, roughness2);
	float beckmann = exp(-tan(alpha) / roughness2) / (3.141592 * roughness2 * pow(cos(alpha), 4.0));
	// float geometric = min(1.0, min((2.0 * n_h * n_v) / h_v, (2.0 * n_h * n_l) / h_v));
	float ggx = (roughness2 * sign(max(0.0, n_h))) / (3.141592 * n_l * n_l * pow(roughness2 + (1.0 - n_l * n_l) / (n_l * n_l), 2.0));
	float geometric = min(1.0, min((2.0 * dot(n, h) * dot(n, v)) / dot(v, h), (2.0 * dot(n, h) * dot(n, l)) / dot(v, h)));

	vec3 ambient  = lightAmbientG * albedo * occlusion;
	vec3 diffuse  = lightColorG * albedo * theta;
	vec3 specular = lightColorG * gauss;

	outColor.rgb = ambient + (diffuse + specular) * visibility;

	vec3 cascadeColors[6] = vec3[6](
		vec3(1.0, 0.0, 0.0),
		vec3(0.0, 1.0, 0.0),
		vec3(0.0, 0.0, 1.0),
		vec3(0.0, 1.0, 1.0),
		vec3(1.0, 0.0, 1.0),
		vec3(1.0, 1.0, 0.0)
	);

	int cascade = calcCascade(position);

	if (enableVisualizeCascades && cascade != -1)
	{
		outColor.rgb *= cascadeColors[cascade % 6];
	}

	if (showConstCascade)
	{
		mat4 fV = mat4(
			-0.948358,  0.076933, -0.307732, 0.0,
			 0.000000,  0.970142, -0.920042, 0.0,
			 0.317203,  0.230011,  2.757908, 0.0,
			 1.394276, -1.409597,  2.757908, 1.0
		);
		mat4 fP = mat4(
			0.063737, 0.000000,  0.000000, 0.0,
			0.000000, 0.063737,  0.000000, 0.0,
			0.000000, 0.000000, -0.063737, 0.0,
			0.000000, 0.000000,  0.000000, 1.0
		);

		vec4 fPos = fP * fV * invV * vec4(position, 1.0);

		if (fPos.x >= -0.125 && fPos.x <= 0.125 && fPos.y >= -0.125 && fPos.y <= 0.125)
		{
			outColor.rgb = fPos.xyz * 8.0;
		}
	}


	outColor.rgb = pow(outColor.rgb, vec3(1.0 / 2.2));
	outColor.rgb = vec3(h_v);
	outColor.rgb = vec3(n_h);
	outColor.rgb = vec3(n_v);
	outColor.rgb = vec3(n_l);
	outColor.rgb = vec3(n);
	outColor.rgb = vec3(exp(-(exponent * exponent)));
	// outColor.rgb = vec3(position);

	// outColor.rgb = vec3(theta);
	outColor.rgb = vec3(geometric);
	outColor.rgb = vec3(beckmann);
	outColor.rgb = albedo * (vec3(ggx) + theta);
	outColor.rgb = pow(vec3(diffuseShininess), vec3(1.0 / 2.2));

	outColor.rgb = vec3(occlusion);
	// outColor.rgb = cookTorrance(n, l, v, roughness2, albedo, vec3(1.35), 1.0);
	// outColor.rgb = pow(cookTorrance(n, l, v, roughness2, albedo, vec3(1.35), 0.1) * albedo, vec3(1.0/2.2));
	// outColor.rgb = vec3(fresnel);
	// outColor.rgb = vec3(theta + (fresnel * beckmann * geometric) / (4.0 * n_v * n_l));

	// specular = lightColorG * (fresnel * beckmann * geometric) / (4.0 * n_v * n_l);

	// outColor.rgb = ambient + (diffuse + specular) * visibility;
	outColor.a = 1.0;
}