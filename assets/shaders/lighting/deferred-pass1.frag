#version 330

in vec3 position;
in vec3 normal;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;

uniform vec4 matAmbient;
uniform vec4 matDiffuse;
uniform vec4 matSpecular;
uniform vec4 matEmission;
uniform float matShininess;

void main()
{
	outPosition = vec4(position, 1.0);
	outNormal = vec4(normal, 1.0);
}