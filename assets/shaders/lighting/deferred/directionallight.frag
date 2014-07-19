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

vec3 decodeNormal(vec2 enc);
vec3 reconstructPosition(float z, vec2 uv);
float chebyshevUpperBound(float dist, vec2 moments);

void main()
{
	vec2 encodedNormal = texture(texNormal, uv).xy;
	vec4 diffuseShininess = texture(texColor, uv);
	float depth = texture(texDepth, uv).x * 2.0 - 1.0;

	vec3 albedo = diffuseShininess.rgb;
	float shininess = diffuseShininess.a;

	vec3 normal = decodeNormal(encodedNormal);
	vec3 position = reconstructPosition(depth, uv);

	vec4 shadowCoord = shadowmapMVP * invV * vec4(position, 1.0) * 0.5 + 0.5;
	shadowCoord /= shadowCoord.w;

	vec3 n = normalize(normal);
	vec3 l = normalize(lightDirection);
	vec3 v = normalize(-position.xyz);
	vec3 h = normalize(l + v);

	float theta = max(dot(n, l), 0.0);

	float exponent = acos(dot(h, n)) / (shininess);
	float gauss = (theta != 0.0) ? exp(-(exponent * exponent) * inversesqrt(theta)) : 0.0;

	vec3 ambient  = lightColor.rgb / 32.0;
	vec3 diffuse  = lightColor.rgb * albedo * theta;
	vec3 specular = lightColor.rgb * gauss;

	float visibility = 1.0;

	vec2 moments = texture(shadowMoments, shadowCoord.xy).rg;
	visibility = chebyshevUpperBound(min(shadowCoord.z, 1.0), moments);

	outColor.rgb = ambient + (diffuse + specular) * visibility;
	outColor.a = 1.0;
}