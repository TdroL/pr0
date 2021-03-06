#version 440 core

#pragma rn: include(lib/normal.glsl)
#pragma rn: include(lib/util.glsl)

layout(location = 0) out vec4 outColor;

in vec2 uv;

uniform sampler2D texZ;
uniform sampler2D texNormal;

uniform vec4 projectionInfo;
uniform float pixelScale;
uniform float intensity;
uniform float radius;
uniform int zMipLevels;

int logMaxOffset = 3;
bool useNormalTexture = true;
int samplesCount = 9;
float spins = 7;
float epsilon = 0.01;
float bias = 0.012;
float maxZFar = 256.0;

vec3 reconstructCSPosition(vec2 coord, float z)
{
	return vec3(-(coord.xy * projectionInfo.xy + projectionInfo.zw) * z, z);
}

vec3 getPosition(ivec2 offsetCoord)
{
	return reconstructCSPosition(vec2(offsetCoord) + vec2(0.5), texelFetch(texZ, offsetCoord, 0).r);
}

vec3 getOffsetPosition(ivec2 coord, vec2 offsetVector, float offsetRadius)
{
	int mipLevel = clamp(findMSB(int(offsetRadius)) - logMaxOffset, 0, zMipLevels);
	ivec2 offsetCoord = ivec2(offsetRadius * offsetVector) + coord;
	ivec2 mipCoord = clamp(offsetCoord >> mipLevel, ivec2(0), textureSize(texZ, mipLevel) - ivec2(1));

	return reconstructCSPosition(vec2(offsetCoord) + vec2(0.5), texelFetch(texZ, mipCoord, mipLevel).r);
}

vec3 reconstructCSFaceNormal(vec3 position)
{
	return normalize(cross(dFdy(position), dFdx(position)));
}

void main()
{
	ivec2 coord = ivec2(gl_FragCoord.xy);

	vec3 position = getPosition(coord);

	float randomRotation = (30 * coord.x ^ coord.y + coord.x * coord.y) * 10;

	vec3 normal;
	if (useNormalTexture)
	{
		normal = normalDecode(texture(texNormal, uv).xy);
	}
	else
	{
		normal = reconstructCSFaceNormal(position);
	}

	float diskRadius = pixelScale * radius / position.z;

	float sum = 0.0;

	for (int i = 0; i < samplesCount; i++)
	{
		float alpha = float(i + 0.5) / samplesCount;
		float angle = alpha * (spins * 2.0 * 3.14159265) + randomRotation;

		float offsetRadius = alpha * diskRadius;
		vec2 offsetVector = vec2(cos(angle), sin(angle));

		vec3 Q = getOffsetPosition(coord, offsetVector, offsetRadius);

		vec3 v = Q - position;

		float v_v = dot(v, v);
		float v_n = dot(v, normal);

		float f = max(radius * radius - v_v, 0.0);

		sum += f * f * f * max((v_n - bias) / (epsilon + v_v), 0.0);
	}

	float o = max(1.0 - 5.0 * intensity / pow(radius, 6.0) * sum / samplesCount, 0.0);

	if (abs(dFdx(position.z)) < 0.02)
	{
		o -= dFdx(o) * ((coord.x & 1) - 0.5);
	}

	if (abs(dFdy(position.z)) < 0.02)
	{
		o -= dFdy(o) * ((coord.y & 1) - 0.5);
	}

	vec2 packedZ = pack2(clamp(position.z / maxZFar, 0.0, 1.0));

	outColor = vec4(o, packedZ, 1.0);
}
