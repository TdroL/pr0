#version 330 core

layout(location = 0) out vec4 outColor;

uniform vec4 matDiffuse;
uniform float matShininess;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

uniform vec2 fbScale;

uniform sampler2D texAO;
uniform samplerBuffer lightD;
uniform samplerBuffer lightP;
uniform int lightDCount;
uniform int lightPCount;

in vec4 position;
in vec3 normal;

int lightDOffset = 3;

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
	float l_h5 = pow(1.0 - l_h, 5.0);
	f = f0 + (1.0 - f0) * l_h5;

	// G
	float k = alpha / 2.0;
	g = G1V(n_l, k) * G1V(n_v, k);

	return n_l * d * f * g;
}

void main()
{
	vec2 ssuv = gl_FragCoord.xy * fbScale;
	vec4 albedo = pow(matDiffuse, vec4(1.0));
	float shininess = matShininess;

	vec3 n = normalize(normal);
	vec3 v = normalize(-position.xyz);

	for (int i = 0; i < lightDCount; i++)
	{
		vec4 lAmbient = texelFetch(lightD, lightDOffset * i + 0).rgba;
		vec4 lColor = pow(texelFetch(lightD, lightDOffset * i + 1).rgba, vec4(2.2));
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

		// outColor += albedo * (ambient + diffuse + specular);
		outColor += diffuse + lColor.rgba * lightSpecular(n, v, l, shininess, 0.82);
	}

	outColor = pow(albedo * outColor * texture(texAO, ssuv).r, vec4(1.0/2.2));
	outColor.a = 1.0;
}