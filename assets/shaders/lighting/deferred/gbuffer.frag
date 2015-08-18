#version 330 core

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outNormal;
// layout(location = 2) out float outZ;
layout(location = 2) out vec3 outZ;

in vec4 position;
in vec3 normal;

uniform float matShininess;
uniform vec4 matDiffuse;
uniform vec3 color;

vec2 normalEncode(vec3 n);

void main()
{
	vec3 n = normalize(normal);

	outColor = vec4(pow(matDiffuse.rgb, vec3(2.2)), matShininess);
	outNormal = normalEncode(n);
	// outZ = position.z;
	outZ = position.zyx;

	// outColor = vec4(position.xyz, 1.0);
	// outColor = vec4(1.0);
}