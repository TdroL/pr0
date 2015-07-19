#version 330 core

layout(location = 0) out vec4 outColor;

in vec2 uv;

uniform mat4 invP;
uniform mat4 invV;

uniform float near;
uniform float far;

uniform sampler2D texNormal;
uniform sampler2D texColor;
uniform sampler2D texDepth;
uniform sampler2D texAO;
uniform sampler2D texShadowMoments;
uniform sampler2D texShadowDepth;
uniform sampler2DArray csmTexCascades;
uniform sampler2DArrayShadow csmTexDepths;

uniform mat4 shadowmapMVP;

#define CSM_MAX_CASCADES 4
uniform int csmCascades;
uniform float csmSplits[CSM_MAX_CASCADES];
uniform mat4 csmMVP[CSM_MAX_CASCADES];

uniform vec3 lightDirection;
uniform vec4 lightColor;
uniform float lightIntensity;
uniform uint useColor;

bool enableBlending = false;
bool enableVisualizeCascades = true;

vec3 normalDecode(vec2 enc);
vec3 positionReconstruct(float z, vec2 uv);
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
			// float lightDepth = shadowCoord.z - depthBias * (0.25 * float(csmLayer + 1.0)) * max(abs(i), abs(j));

			float lightDepth = shadowCoord.z - depthBias;
			visibility += texture(csmTexDepths, vec4(shadowCoord.xy + vec2(i, j) * texSizeInv, csmLayer, lightDepth));

			weight += 1.0;
		}
	}

	visibility /= weight;

	// visibility = 1.0 - pow(1.0 - visibility, 2.0);

	// visibility = pow(visibility, kernelSize / 2.0);

	return visibility;
}

float optPcfVisibility(vec3 shadowCoord, int csmLayer, float kernelSize)
{
	vec2 texSize = textureSize(csmTexDepths, 0).xy;
	vec2 texSizeInv = 1.0 / texSize;

	float depthBias = texSizeInv.x;
	float lightDepth = shadowCoord.z + depthBias;

	vec2 uv = shadowCoord.xy * texSize;

	vec2 baseUv = floor(uv + 0.5);
	float s = fract(uv.x);
	float t = fract(uv.y);

	baseUv -= 0.5;
	baseUv *= texSizeInv;

	// kernelSize = 5
	float uw0 = (4.0 - 3.0 * s);
	float uw1 = 7.0;
	float uw2 = (1.0 + 3.0 * s);

	float u0 = (3.0 - 2.0 * s) / uw0 - 2.0;
	float u1 = (3.0 + s) / uw1;
	float u2 = s / uw2 + 2.0;

	float vw0 = (4.0 - 3.0 * t);
	float vw1 = 7.0;
	float vw2 = (1.0 + 3.0 * t);

	float v0 = (3.0 - 2.0 * t) / vw0 - 2.0;
	float v1 = (3.0 + t) / vw1;
	float v2 = t / vw2 + 2.0;

	vec2 texSizeInvScaled = texSizeInv / float(csmLayer + 1.0);

	float visibility = 1.0;
	visibility += uw0 * vw0 * texture(csmTexDepths, vec4(baseUv + vec2(u0, v0) * texSizeInvScaled, csmLayer, lightDepth));
	visibility += uw1 * vw0 * texture(csmTexDepths, vec4(baseUv + vec2(u1, v0) * texSizeInvScaled, csmLayer, lightDepth));
	visibility += uw2 * vw0 * texture(csmTexDepths, vec4(baseUv + vec2(u2, v0) * texSizeInvScaled, csmLayer, lightDepth));

	visibility += uw0 * vw1 * texture(csmTexDepths, vec4(baseUv + vec2(u0, v1) * texSizeInvScaled, csmLayer, lightDepth));
	visibility += uw1 * vw1 * texture(csmTexDepths, vec4(baseUv + vec2(u1, v1) * texSizeInvScaled, csmLayer, lightDepth));
	visibility += uw2 * vw1 * texture(csmTexDepths, vec4(baseUv + vec2(u2, v1) * texSizeInvScaled, csmLayer, lightDepth));

	visibility += uw0 * vw2 * texture(csmTexDepths, vec4(baseUv + vec2(u0, v2) * texSizeInvScaled, csmLayer, lightDepth));
	visibility += uw1 * vw2 * texture(csmTexDepths, vec4(baseUv + vec2(u1, v2) * texSizeInvScaled, csmLayer, lightDepth));
	visibility += uw2 * vw2 * texture(csmTexDepths, vec4(baseUv + vec2(u2, v2) * texSizeInvScaled, csmLayer, lightDepth));

	visibility = visibility * 1.0f / 144;

	return visibility;
}

int calcCascade(float z)
{
	for (int i = 0; i < csmCascades; i++)
	{
		if (csmSplits[i] <= z)
		{
			return i;
		}
	}

	return -1;
}

float calcVisibility(vec3 position)
{
	int csmLayer = calcCascade(position.z);

	if (csmLayer == -1) {
		return 1.0;
	}

	vec4 shadowCoord = csmMVP[csmLayer] * vec4(position, 1.0) * 0.5 + 0.5;
	shadowCoord /= shadowCoord.w;

	// vec4 moments = texture(csmTexCascades, vec3(shadowCoord.xy, csmLayer));
	// float visibility = vsmVisibility(min(shadowCoord.z, 1.0), moments.xy);

	float kernelSize = 1.0;
	float visibility = pcfVisibility(shadowCoord.xyz, csmLayer, kernelSize);
	// float visibility = optPcfVisibility(shadowCoord.xyz, csmLayer, 5.0);

	if (enableBlending && csmLayer < csmCascades - 1)
	{
		float blendScale = 16.0;

		float dl = abs((position.z - csmSplits[csmLayer]) / (csmSplits[csmLayer + 1] - csmSplits[csmLayer])) * blendScale;

		if (dl < 1.0)
		{
			shadowCoord = csmMVP[csmLayer + 1] * vec4(position, 1.0) * 0.5 + 0.5;
			shadowCoord /= shadowCoord.w;

			// moments = texture(csmTexCascades, vec3(shadowCoord.xy, csmLayer + 1.0));
			// float nextVisibility = vsmVisibility(min(shadowCoord.z, 1.0), moments.xy);
			float nextVisibility = pcfVisibility(shadowCoord.xyz, csmLayer + 1, kernelSize);

			visibility = min(visibility, mix(visibility, nextVisibility, 1.0 - dl));
		}
	}

	return visibility;
}

void main()
{
	vec2 encodedNormal = texture(texNormal, uv).xy;
	vec4 diffuseShininess = texture(texColor, uv);
	float depth = texture(texDepth, uv).x * 2.0 - 1.0;

	vec3 albedo = diffuseShininess.rgb;
	float shininess = diffuseShininess.a;

	vec3 normal = normalDecode(encodedNormal);
	vec3 position = positionReconstruct(depth, uv);

	if (useColor == 0u) {
		albedo = vec3(0.5);
	}

	float visibility = calcVisibility(position);
	float occlusion = texture(texAO, uv).r;

	// albedo *= occlusion;

	vec3 n = normalize(normal);
	vec3 l = normalize(lightDirection);
	vec3 v = normalize(-position.xyz);
	vec3 h = normalize(l + v);

	float n_l = dot(n, l);
	float n_h = dot(n, h);
	float n_v = dot(n, v);

	float theta = max(n_l, 0.0);

	float exponent = acos(n_h) / shininess;
	float gauss = sign(theta) * exp(-(exponent * exponent));

	vec3 lightColorG = pow(lightColor.rgb, vec3(2.2)) * lightIntensity;

	vec3 ambient  = lightColorG * albedo * occlusion / 16.0;
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
	int cascade = calcCascade(position.z);

	if (enableVisualizeCascades && cascade != -1)
	{
		outColor.rgb *= cascadeColors[cascade % 6];
	}

	outColor.rgb = pow(outColor.rgb, vec3(1.0 / 2.2));
	outColor.a = 1.0;
}