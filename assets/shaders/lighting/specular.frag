#version 330

in vec3 position;
in vec3 normal;

out vec4 outColor;

uniform mat4 V;

uniform vec4 matAmbient;
uniform vec4 matDiffuse;
uniform vec4 matSpecular;
uniform vec4 matEmission;
uniform float matShininess;

uniform vec4 lightAmbient;
uniform vec4 lightDiffuse;
uniform vec4 lightSpecular;
uniform vec4 lightPosition;
// uniform vec3 lightSpotDirection;
// uniform float lightSpotExponent;
// uniform float lightSpotCutoff;
// uniform float lightConstantAttenuation;
uniform float lightLinearAttenuation;
uniform float lightQuadraticAttenuation;

void main()
{
	vec4 lightPositionV = V * lightPosition;

	vec3 l = normalize(lightPositionV.xyz - position);
	vec3 n = normalize(normal);
	vec3 v = normalize(-position.xyz);
	vec3 h = normalize(l + v);

	float d = distance(lightPositionV.xyz, position);

	float attenuation = 1.0 / (1.0 + d * lightLinearAttenuation + d * d * lightQuadraticAttenuation);
	float theta = dot(n, l);
	theta = max(theta, 0.0);

	float exponent = acos(dot(h, n)) / matShininess;
	exponent = -exponent * exponent;
	float gauss = exp(exponent);
	gauss = theta != 0.0 ? gauss : 0.0;

	vec3 ambient = vec3(lightAmbient);
	vec3 diffuse = vec3(lightDiffuse * attenuation * theta);
	vec3 specular = vec3(lightSpecular * attenuation * gauss);

	outColor = vec4(ambient + diffuse + specular, 1.0);
}