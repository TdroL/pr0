#version 330

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outNormal;

in vec4 position;
in vec3 normal;

uniform float matShininess;
uniform vec4 matDiffuse;

vec2 encodeNormal(vec3 n)
{
	float scale = 1.0/1.7777;
	vec2 enc = n.xy / (n.z+1);
	enc = enc * scale * 0.5 + 0.5;
	return enc;
}

void main()
{
	vec3 n = normalize(normal);

	outColor = vec4(matDiffuse.rgb, matShininess);
	outNormal = encodeNormal(n);
}