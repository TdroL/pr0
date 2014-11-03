#version 330

// source: http://john-chapman-graphics.blogspot.co.uk/2013/01/ssao-tutorial.html

layout(location = 0) out vec4 outColor;

in vec2 uv;

uniform mat4 P;

uniform sampler2D texNormal;
uniform sampler2D texColor;
uniform sampler2D texDepth;
uniform sampler2D texNoise;

uniform vec2 filterRadius;

const int kernelSize = 16;
uniform vec3 kernel[kernelSize];
uniform vec2 noiseScale;

vec3 normalDecode(vec2 enc);
vec3 positionReconstruct(float z, vec2 uv);

void main()
{
	vec2 encodedNormal = texture(texNormal, uv).xy;
	float depth = texture(texDepth, uv).x * 2.0 - 1.0;
	vec3 albedo = texture(texColor, uv).rgb;

	vec3 normal = normalDecode(encodedNormal);
	vec3 position = positionReconstruct(depth, uv);

	vec3 noise = texture(texNoise, uv * noiseScale).xyz * 2.0 - 1.0;
	vec3 tangent = normalize(noise - normal * dot(noise, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 tbn = mat3(tangent, bitangent, normal);

	float sampleRadius = 0.25;
	float occlusion = 0.0;

	for (int i = 0; i < kernelSize; ++i)
	{
		vec3 sample = tbn * kernel[i] * sampleRadius + position;

		vec4 offset = P * vec4(sample, 1.0);
		offset.xy = offset.xy / offset.w * 0.5 + 0.5;

		float sampleDepth = texture(texDepth, offset.xy).x * 2.0 - 1.0;
		vec3 samplePosition = positionReconstruct(sampleDepth, offset.xy);

		float rangeCheck= abs(position.z - samplePosition.z) < sampleRadius ? 1.0 : 0.0;
		occlusion += ((samplePosition.z > sample.z) ? 1.0 : 0.0) * rangeCheck;
	}

	outColor.rgb = vec3(0.0);
	outColor.a = occlusion / kernelSize;
}