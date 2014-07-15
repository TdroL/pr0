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
uniform sampler2D shadowMoments;
uniform sampler2D shadowDepth;

uniform mat4 shadowmapMVP;

uniform vec3 lightDirection;
uniform vec4 lightColor;

vec3 decodeNormal(vec2 enc)
{
	float scale = 1.7777;
	vec3 nn = vec3(enc.xy * 2.0 * scale - scale, 1.0);
	float g = 2.0 / dot(nn, nn);
	vec3 n = vec3(g * nn.xy, g - 1.0);

	return n;
}

vec3 reconstructPosition(float z)
{
	vec4 position = invP * vec4(uv * 2.0 - 1.0, z, 1.0);
	return (position.xyz / position.w);
}

float chebyshevUpperBound(float dist, vec2 moments)
{
	dist = max(dist, moments.x);

	// The fragment is either in shadow or penumbra. We now use chebyshev's upperBound to check
	// How likely this pixel is to be lit (p_max)
	float variance = moments.y - (moments.x * moments.x);
	variance = max(variance, 0.0002);

	float d = dist - moments.x;
	float pMax = variance / (variance + d * d);

	// Reduce light bleeding
	pMax = smoothstep(0.75, 1.0, pMax);

	return pMax;
}

void main()
{
	vec2 encodedNormal = texture(texNormal, uv).xy;
	vec4 diffuseShininess = texture(texColor, uv);
	float depth = texture(texDepth, uv).x * 2.0 - 1.0;

	vec3 albedo = diffuseShininess.rgb;
	float shininess = diffuseShininess.a;

	vec3 normal = decodeNormal(encodedNormal);
	vec3 position = reconstructPosition(depth);

	vec4 shadowCoord = shadowmapMVP * invV * vec4(position, 1.0) * 0.5 + 0.5;
	shadowCoord /= shadowCoord.w;

	vec3 n = normalize(normal);
	vec3 l = normalize(lightDirection);
	vec3 v = normalize(-position.xyz);
	vec3 h = normalize(l + v);

	float n_l = dot(n, l);
	float n_h = dot(n, h);

	float theta = max(n_l, 0.0);

	float exponent = acos(n_h) / (shininess);
	float gauss = (theta != 0.0) ? exp(-(exponent * exponent) * inversesqrt(theta)) : 0.0;

	vec3 ambient  = lightColor.rgb / 32.0;
	vec3 diffuse  = lightColor.rgb * albedo * theta;
	vec3 specular = lightColor.rgb * gauss;

	float visibility = 1.0;

	vec2 moments = texture(shadowMoments, shadowCoord.xy).rg;
	visibility = chebyshevUpperBound(min(shadowCoord.z, 1.0), moments);

	outColor.rgb = ambient + (diffuse + specular) * visibility;
	// outColor.rgb = albedo * vec3(theta);
	outColor.r = albedo.r * (theta + 0.2) / (1.0 + 0.2);
	outColor.g = albedo.g * (theta + 0.05) / (1.0 + 0.05);
	outColor.b = albedo.b * (theta + 0.0) / (1.0 + 0.0);
	outColor.a = 1.0;
}