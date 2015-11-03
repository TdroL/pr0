#version 440 core

#pragma rn: include(lib/normal.glsl)
#pragma rn: include(lib/position.glsl)

layout(location = 0) out vec4 outColor;

in vec2 uv;

uniform mat4 invP;

uniform sampler2D texNormal;
uniform sampler2D texColor;
uniform sampler2D texDepth;
uniform sampler2D shadowMoments;
uniform sampler2D shadowDepth;

uniform mat4 shadowmapMVP;

uniform vec4 lightColor;
uniform vec4 lightPosition;
uniform float lightLinearAttenuation;
uniform float lightQuadraticAttenuation;

// vec3 normalDecode(vec2 enc);
// vec3 positionReconstruct(float z, vec2 uv);

void main()
{
	vec2 encodedNormal = texture(texNormal, uv).xy;
	vec4 diffuseShininess = texture(texColor, uv);
	float depth = texture(texDepth, uv).x * 2.0 - 1.0;

	vec3 albedo = diffuseShininess.rgb;
	float shininess = diffuseShininess.a;

	vec3 normal = normalDecode(encodedNormal);
	vec3 position = positionReconstruct(depth, uv, invP);

	vec3 n = normalize(normal);
	vec3 l = normalize(lightPosition.xyz - position.xyz);
	vec3 v = normalize(-position.xyz);
	vec3 h = normalize(l + v);

	float n_l = dot(n, l);
	float n_h = dot(n, h);

	float theta = max(n_l, 0.0);

	float exponent = acos(n_h) / (shininess);
	float gauss = sign(theta) * exp(-(exponent * exponent));

	vec3 lightColorG = pow(lightColor.rgb, vec3(2.2));

	vec3 diffuse  = lightColorG * albedo * theta;
	vec3 specular = lightColorG * gauss;

	outColor.rgb = pow(diffuse + specular, vec3(1/2.2));
	outColor.rgb = vec3(0.0);
	outColor.a = 1.0;
}