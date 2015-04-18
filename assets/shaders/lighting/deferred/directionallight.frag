#version 330

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
// uniform sampler2D texShadowDepth;
uniform sampler2DArray texCSM;

uniform mat4 shadowmapMVP;
uniform float csmCascades[3];
uniform mat4 csmMVP[4];

uniform vec3 lightDirection;
uniform vec4 lightColor;
uniform float lightIntensity;
uniform uint useColor;

vec3 normalDecode(vec2 enc);
vec3 positionReconstruct(float z, vec2 uv);
float vsmVisibility(float dist, vec2 moments);

float calcVisibility(vec3 position)
{
	int csmLayer = 3;

	if (csmCascades[0] < position.z)
	{
		csmLayer = 0;
	}
	else if (csmCascades[1] < position.z)
	{
		csmLayer = 1;
	}
	else if (csmCascades[2] < position.z)
	{
		csmLayer = 2;
	}

	vec4 shadowCoord = csmMVP[csmLayer] * vec4(position, 1.0) * 0.5 + 0.5;
	shadowCoord /= shadowCoord.w;

	vec4 moments = texture(texCSM, vec3(shadowCoord.xy, csmLayer));
	float visibility = vsmVisibility(min(shadowCoord.z, 1.0), moments.xy);

	if (csmLayer < 3)
	{
		float blendScale = 16.0;

		float dl = abs((position.z - csmCascades[csmLayer]) / (csmCascades[csmLayer + 1] - csmCascades[csmLayer])) * blendScale;

		if (dl < 1.0)
		{
			shadowCoord = csmMVP[csmLayer + 1] * vec4(position, 1.0) * 0.5 + 0.5;
			shadowCoord /= shadowCoord.w;

			moments = texture(texCSM, vec3(shadowCoord.xy, csmLayer + 1.0));
			float nextVisibility = vsmVisibility(min(shadowCoord.z, 1.0), moments.xy);

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

	// float visibility = 1.0;

	// vec2 moments = texture(texShadowMoments, shadowCoord.xy).rg;
	// visibility = vsmVisibility(min(shadowCoord.z, 1.0), moments);

	outColor.rgb = ambient + (diffuse + specular) * visibility;
	outColor.rgb = pow(outColor.rgb, vec3(1.0 / 2.2));
	outColor.a = 1.0;
}