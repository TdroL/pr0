#version 330

layout(location = 0) out vec4 outColor;

in vec2 uv;

uniform mat4 invP;
uniform mat4 invV;

uniform float near;
uniform float far;

uniform sampler2D texNormal;
uniform sampler2D texColor;
uniform sampler2D texDS;
uniform sampler2D smtexDS;

uniform mat4 shadowmapMVP;

uniform vec3 lightDirection;
uniform vec4 lightColor;

vec3 decodeNormal(vec2 enc)
{
	float scale = 1.7777;
	vec3 nn = vec3(enc.xy * 2.0 * scale - scale, 1.0);
	float g = 2.0 / dot(nn, nn);
	vec3 n = vec3(g * nn.xy, g - 1.0);

	return n;
}

vec3 reconstructPosition(float z)
{
	vec4 position = invP * vec4(uv * 2.0 - 1.0, z, 1.0);
	return (position.xyz / position.w);
}

void main()
{
	vec2 encodedNormal = texture(texNormal, uv).xy;
	vec4 diffuseShininess = texture(texColor, uv);
	float depth = texture(texDS, uv).x * 2.0 - 1.0;

	vec3 normal = decodeNormal(encodedNormal);
	vec3 position = reconstructPosition(depth);

	vec4 shadowCoord = shadowmapMVP * invV * vec4(position, 1.0) * 0.5 + 0.5;
	shadowCoord /= shadowCoord.w;

	vec3 n = normalize(normal);
	vec3 l = normalize(lightDirection);
	vec3 v = normalize(-position.xyz);
	vec3 h = normalize(l + v);

	float theta = max(dot(n, l), 0.0);

	float exponent = acos(dot(h, n)) / diffuseShininess.a;
	float gauss = (theta != 0.0) ? exp(-(exponent * exponent)) : 0.0;

	vec3 ambient  = lightColor.rgb / 32.0;
	vec3 diffuse  = lightColor.rgb * diffuseShininess.rgb * theta;
	vec3 specular = lightColor.rgb * gauss;

	float shadowZBias = clamp(0.0005 * tan((theta)), 0.0, 0.01);
	float visibility = 1.0;
	shadowCoord.z += shadowZBias;

	if (shadowCoord.z <= 1.0)
	{
		float shadow = 0.0;

		shadow += step(shadowCoord.z, textureOffset(smtexDS, shadowCoord.xy, ivec2(-1,  0)).z);
		shadow += step(shadowCoord.z, textureOffset(smtexDS, shadowCoord.xy, ivec2( 1,  0)).z);
		shadow += step(shadowCoord.z, textureOffset(smtexDS, shadowCoord.xy, ivec2( 0, -1)).z);
		shadow += step(shadowCoord.z, textureOffset(smtexDS, shadowCoord.xy, ivec2( 0, -1)).z);

		visibility = shadow / 4.0;
	}

	outColor.rgb = ambient + (diffuse + specular) * visibility;
	// outColor.rgb = texture(smtexDS, shadowCoord.xy).zzz - shadowCoord.zzz;
	outColor.a = 1.0;
}