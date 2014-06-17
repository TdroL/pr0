#version 330

layout(location = 0) out vec4 outColor;

in vec2 uv;

uniform mat4 invP;

uniform float near;
uniform float far;

uniform sampler2D texNormal;
uniform sampler2D texColor;
uniform sampler2D texDS;

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

	outColor.rgb = ambient + diffuse + specular;
	outColor.a = 1.0;
}