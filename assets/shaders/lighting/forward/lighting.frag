#version 330 core

layout(location = 0) out vec4 outColor;

uniform vec4 matDiffuse;
uniform float matShininess;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

uniform samplerBuffer lightD;
uniform samplerBuffer lightP;
uniform int lightDCount;
uniform int lightPCount;

in vec4 position;
in vec3 normal;

int lightDOffset = 3;

void main()
{
	vec4 albedo = pow(matDiffuse, vec4(2.2));
	float shininess = matShininess;

	vec3 n = normalize(normal);
	vec3 v = normalize(-position.xyz);

	for (int i = 0; i < lightDCount; i++)
	{
		vec4 lAmbient = texelFetch(lightD, lightDOffset * i + 0).rgba;
		vec4 lColor = texelFetch(lightD, lightDOffset * i + 1).rgba;
		vec4 lDirectionIntensity = texelFetch(lightD, lightDOffset * i + 2).rgba;

		vec3 lDirection = lDirectionIntensity.xyz;
		float lIntensity = lDirectionIntensity.a;

		vec3 l = lDirection;
		vec3 h = normalize(l + v);

		float n_l = max(0.0, dot(n, l));

		float n_h = dot(n, h);
		float alpha = acos(n_h);
		float exponent = alpha / shininess;
		float gauss = sign(n_l) * exp(-(exponent * exponent));

		vec4 ambient = lAmbient;
		vec4 diffuse = vec4(lColor.rgb * n_l, lColor.a);
		vec4 specular = vec4(lColor.rgb * gauss, lColor.a);

		outColor += albedo * (ambient + diffuse + specular);
	}

	outColor = pow(outColor, vec4(1.0/2.2));
}