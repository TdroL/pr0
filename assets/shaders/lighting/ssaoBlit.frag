#version 330

layout(location = 0) out vec4 outColor;

in vec2 uv;

uniform sampler2D texSource;
uniform sampler2D texColor;

void main()
{
	vec3 albedo = texture(texColor, uv).rgb;
	outColor.rgb = pow(albedo, vec3(2.2));
	outColor.a = texture(texSource, uv).r;
}