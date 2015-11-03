#version 440 core

#pragma rn: include(lib/normal.glsl)

out vec4 outColor;

in vec2 uv;

uniform sampler2D texNormal;
uniform sampler2D texDebug;
uniform sampler2D texScreen;

// vec3 normalDecode(vec2 enc);

void main()
{
	// outColor.rgb = vec3(1.0);
	// outColor.rgb = texture(texNormal, uv).rgb;
	// outColor.rgb = normalDecode(texture(texNormal, uv).rg) * vec3(1.0, 1.0, -1.0);
	// outColor.rgb = texture(texDebug, uv).rgb * vec3(1.0, 1.0, -1.0);
	// outColor.rgb = abs(texture(texDebug, uv).rgb - normalDecode(texture(texNormal, uv).rg)) * 100.0;
	outColor.rgb = texture(texScreen, uv).rgb;
	outColor.a = 1.0;
}