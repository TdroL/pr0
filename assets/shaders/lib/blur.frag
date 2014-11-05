#version 330

vec4 blurGaussian7(sampler2D tex, vec2 uv, vec2 scale)
{
	vec4 color = vec4(0.0);
	vec2 texelSize = scale / textureSize(tex, 0);

	color += texture(tex, uv + texelSize * -3.0) * 0.015625;
	color += texture(tex, uv + texelSize * -2.0) * 0.09375;
	color += texture(tex, uv + texelSize * -1.0) * 0.234375;
	color += texture(tex, uv + texelSize *  0.0) * 0.3125;
	color += texture(tex, uv + texelSize *  1.0) * 0.234375;
	color += texture(tex, uv + texelSize *  2.0) * 0.09375;
	color += texture(tex, uv + texelSize *  3.0) * 0.015625;

	return color;
}

vec4 blurBox4(sampler2D tex, vec2 uv, vec2 scale)
{
	vec2 texelSize = scale / textureSize(tex, 0);
	vec4 color = vec4(0.0);

	color += texture(tex, uv + texelSize * -1.5);
	color += texture(tex, uv + texelSize * -0.5);
	color += texture(tex, uv + texelSize *  0.5);
	color += texture(tex, uv + texelSize *  1.5);

	return color / 4.0;
}