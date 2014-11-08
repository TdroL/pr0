#version 330

layout(location = 0) out vec4 outColor;

in vec2 uv;

uniform sampler2D texDepth;
uniform sampler2D texNormal;
uniform sampler2D texSource;
uniform vec2 scale;
uniform mat4 invP;

vec3 normalDecode(vec2 enc);
vec3 positionReconstruct(float z, vec2 uv);

void main()
{
	vec2 texelSize = scale / textureSize(texSource, 0);

	float depth = texture(texDepth, uv).x * 2.0 - 1.0;
	vec3 position = positionReconstruct(depth, uv);
	vec3 normal = normalDecode(texture(texNormal, uv).xy);

	outColor = texture(texSource, uv);
	float divisor = 1.0;

	for (int i = -2; i < 0; i++)
	{
		vec2 coords = uv + texelSize * float(i);

		vec4 occlussion = texture(texSource, coords);

		float neighbourDepth = texture(texDepth, coords).x * 2.0 - 1.0;
		vec3 neighbourPosition = positionReconstruct(neighbourDepth, coords);
		vec3 neighbourNormal = normalDecode(texture(texNormal, coords).xy);

		float weight = clamp(1.5 - abs(neighbourPosition.z - position.z) * 1.0 - (1.0 - abs(dot(neighbourNormal, normal))), 0.0, 1.0);

		// weight = 1.0;

		divisor += weight;
		outColor += occlussion * weight;
	}

	for (int i = 1; i <= 2; i++)
	{
		vec2 coords = uv + texelSize * float(i);

		vec4 occlussion = texture(texSource, coords);

		float neighbourDepth = texture(texDepth, coords).x * 2.0 - 1.0;
		vec3 neighbourPosition = positionReconstruct(neighbourDepth, coords);
		vec3 neighbourNormal = normalDecode(texture(texNormal, coords).xy);

		float weight = clamp(1.5 - abs(neighbourPosition.z - position.z) * 1.0 - (1.0 - abs(dot(neighbourNormal, normal))), 0.0, 1.0);

		// weight = 1.0;

		divisor += weight;
		outColor += occlussion * weight;
	}

	outColor /= divisor;

	// outColor = texture(texSource, uv);
	outColor.a = 1.0;
}