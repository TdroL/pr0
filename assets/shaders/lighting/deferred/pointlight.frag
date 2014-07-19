#version 330

layout(location = 0) out vec4 outColor;

in vec2 uv;

uniform mat4 invP;

uniform float near;
uniform float far;

uniform sampler2D texNormal;
uniform sampler2D texColor;
uniform sampler2D texDS;

uniform vec4 lightColor;
uniform vec4 lightPosition;
uniform float lightLinearAttenuation;
uniform float lightQuadraticAttenuation;

vec3 decodeNormal(vec2 enc);
vec3 reconstructPosition(float z, vec2 uv);

void main()
{
	vec2 encodedNormal = texture(texNormal, uv).xy;
	vec4 diffuseShininess = texture(texColor, uv);
	float depth = texture(texDS, uv).x * 2.0 - 1.0;

	vec3 normal = decodeNormal(encodedNormal);
	vec3 position = reconstructPosition(depth, uv);

	vec3 n = normalize(normal);
	vec3 l = normalize(lightPosition.xyz - position.xyz);
	vec3 v = normalize(-position.xyz);
	vec3 h = normalize(l + v);

	float d = distance(lightPosition.xyz, position.xyz);
	float attenuation = 1.0 / (1.0 + d * lightLinearAttenuation + d * d * lightQuadraticAttenuation);

	float theta = max(dot(n, l), 0.0);

	float exponent = acos(dot(h, n)) / diffuseShininess.a;
	float gauss = (theta != 0.0) ? exp(-(exponent * exponent)) : 0.0;

	vec3 ambient  = lightColor.rgb * attenuation / 1024.0;
	vec3 diffuse  = lightColor.rgb * diffuseShininess.rgb * attenuation * theta;
	vec3 specular = lightColor.rgb * attenuation * gauss;

	outColor.rgb = ambient + diffuse + specular;
	outColor.a = 1.0;
}