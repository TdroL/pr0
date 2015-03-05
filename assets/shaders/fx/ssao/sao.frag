#version 330

layout(location = 0) out vec4 outColor;

in vec2 uv;

uniform mat4 P;
uniform mat4 invP;

uniform sampler2D texNormal;
uniform sampler2D texColor;
uniform sampler2D texDepth;
uniform sampler2D texNoise;

uniform vec2 filterRadius;

const int kernelSize = 16;
uniform vec3 kernel[kernelSize];
uniform vec2 noiseScale;

int samplesCount = 8;
float intensity = 0.15;
float contrast = 1.0;
float epsilon = 0.001;
float bias = 0.00065;
float radius = 0.125;

vec3 normalDecode(vec2 enc);
vec3 normalFaceReconstruct(vec3 position);
vec3 positionReconstruct(float z, vec2 uv);
vec2 positionZPack(float depth, float maxZ);

float random(vec2 co)
{
	return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

vec2 getSampleOffset(int i, float randomRotation)
{
	float alpha = float(i + 0.5) / samplesCount;
	float angle = (alpha + randomRotation) * (2.0 * 3.14159265);
	vec2 offset = vec2(cos(angle), sin(angle));

	return offset * alpha;
}

vec4 getPositionDepth(vec2 coords)
{
	float depth = texture(texDepth, coords).r * 2.0 - 1.0;
	return vec4(positionReconstruct(depth, coords), depth);
}

float sampleAO(int i, in vec3 position, in vec3 normal, float randomRotation, float diskRadius)
{
	vec2 offset = getSampleOffset(i, randomRotation);

	vec2 sampleUv = uv + offset * diskRadius;

	vec3 Q = getPositionDepth(sampleUv).xyz;
	vec3 v = Q - position;

	float v_v = dot(v, v);
	float v_n = dot(v, normal);

	float f = pow(max(0.0, radius * radius - v_v), 3); // fallof

	return f * max(0.0, (v_n + position.z * bias) / (v_v + epsilon));
}

void main()
{
	vec4 positionDepth = getPositionDepth(uv);
	vec3 position = positionDepth.xyz;
	float depth = positionDepth.w;

	vec2 packedZ = positionZPack(-position.z, 200);

	/**/
	vec2 encodedNormal = texture(texNormal, uv).xy;
	vec3 normal = normalDecode(encodedNormal);
	/*/
	vec3 normal = normalFaceReconstruct(position);
	/**/


	/** /
	// source: http://john-chapman-graphics.blogspot.co.uk/2013/01/ssao-tutorial.html

	float occlusion = 0.0;
	vec3 noise = texture(texNoise, uv * noiseScale).xyz * 2.0 - 1.0;
	vec3 tangent = normalize(noise - normal * dot(noise, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 tbn = mat3(tangent, bitangent, normal);

	float sampleRadius = 0.5;

	for (int i = 0; i < kernelSize; ++i)
	{
		float kernelScale = mix(0.1, 1.0, float(i * i) / float(kernelSize * kernelSize));

		vec3 sample = tbn * kernel[i] * kernelScale * sampleRadius + position;

		vec4 offset = P * vec4(sample, 1.0);
		offset /= offset.w;
		offset.xy = offset.xy * 0.5 + 0.5;

		float sampleDepth = texture(texDepth, offset.xy).x * 2.0 - 1.0;
		vec3 samplePosition = positionReconstruct(sampleDepth, offset.xy);

		float rangeCheck= abs(position.z - samplePosition.z) < sampleRadius ? 1.0 : 0.0;
		occlusion += (samplePosition.z > sample.z ? 1.0 : 0.0) * rangeCheck;
	}

	outColor = vec4(occlusion / float(kernelSize));
	/*/
	// Alchemy AO/SAO

	float occlusion;

	/**/
	float randomRotation = random(uv);
	/*/
	ivec2 depthSize = textureSize(texDepth, 0);
	ivec2 scc = ivec2(depthSize * uv);
	float randomRotation = float((3 * scc.x ^ scc.y + scc.x * scc.y) * 10.0);
	/**/

	float diskRadius = abs(0.5 * P[1].y * radius / position.z);

	for (int i = 0; i < samplesCount; i++)
	{
		occlusion += sampleAO(i, position, normal, randomRotation, diskRadius);
	}

	occlusion *= 5.0 / (pow(radius, 6) * samplesCount);
	occlusion = max(0.0, pow(1.0 - occlusion, contrast));
	// occlusion = pow(max(0.0, 1.0 - intensity / samplesCount * occlusion), contrast);

	outColor = vec4(occlusion, packedZ, 1.0);
	/**/
}