#version 330

in vec2 uv;

uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D texDS;

out vec4 outColor;

void main()
{
	// outColor.rgb = texture(tex0, uv).rgb;
	// outColor.rgb = texture(tex1, uv).rgb;
	outColor.rgb = texture(tex2, uv).rgb;
	// outColor.rgb = (texture(texDS, uv).rrr * 128.0) + (1.0 / 128.0);
	// outColor.rgb = texture(tex0, uv).zzz * 2.0;

	outColor.a = 1.0;

}