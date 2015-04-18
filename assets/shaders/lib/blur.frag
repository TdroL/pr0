#version 330

vec4 blurGaussian7(sampler2D tex, vec2 uv, vec2 scale)
{
	const float weight[4] = float[](0.3125, 0.234375, 0.09375, 0.015625);

	vec4 color = vec4(0.0);
	vec2 texelSize = scale / textureSize(tex, 0);

	color += texture(tex, uv - texelSize * 3.0) * weight[3];
	color += texture(tex, uv - texelSize * 2.0) * weight[2];
	color += texture(tex, uv - texelSize * 1.0) * weight[1];
	color += texture(tex, uv + texelSize * 0.0) * weight[0];
	color += texture(tex, uv + texelSize * 1.0) * weight[1];
	color += texture(tex, uv + texelSize * 2.0) * weight[2];
	color += texture(tex, uv + texelSize * 3.0) * weight[3];

	return color;
}

vec4 blurGaussian7(sampler2DArray tex, vec2 uv, int layer, vec2 scale)
{
	const float weight[4] = float[](0.3125, 0.234375, 0.09375, 0.015625);

	vec4 color = vec4(0.0);
	vec2 texelSize = scale / textureSize(tex, 0).xy;

	color += texture(tex, vec3(uv - texelSize * 3.0, layer)) * weight[3];
	color += texture(tex, vec3(uv - texelSize * 2.0, layer)) * weight[2];
	color += texture(tex, vec3(uv - texelSize * 1.0, layer)) * weight[1];
	color += texture(tex, vec3(uv + texelSize * 0.0, layer)) * weight[0];
	color += texture(tex, vec3(uv + texelSize * 1.0, layer)) * weight[1];
	color += texture(tex, vec3(uv + texelSize * 2.0, layer)) * weight[2];
	color += texture(tex, vec3(uv + texelSize * 3.0, layer)) * weight[3];

	return color;
}

// http://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/
vec4 blurGaussian7Opt(sampler2D tex, vec2 uv, vec2 scale)
{
	const float offset[3] = float[](0.0, 1.3846153846, 3.2307692308);
	const float weight[3] = float[](0.2270270270, 0.3162162162, 0.0702702703);

	vec4 color = vec4(0.0);
	vec2 texelSize = scale / textureSize(tex, 0);

	color += texture(tex, uv + texelSize * offset[2]) * weight[2];
	color += texture(tex, uv + texelSize * offset[1]) * weight[1];
	color += texture(tex, uv + texelSize * offset[0]) * weight[0];
	color += texture(tex, uv - texelSize * offset[1]) * weight[1];
	color += texture(tex, uv - texelSize * offset[2]) * weight[2];

	return color;
}

vec4 blurGaussian7Opt(sampler2DArray tex, vec2 uv, int layer, vec2 scale)
{
	const float offset[3] = float[](0.0, 1.3846153846, 3.2307692308);
	const float weight[3] = float[](0.2270270270, 0.3162162162, 0.0702702703);

	vec4 color = vec4(0.0);
	vec2 texelSize = scale / textureSize(tex, 0).xy;

	color += texture(tex, vec3(uv + texelSize * offset[2], layer)) * weight[2];
	color += texture(tex, vec3(uv + texelSize * offset[1], layer)) * weight[1];
	color += texture(tex, vec3(uv + texelSize * offset[0], layer)) * weight[0];
	color += texture(tex, vec3(uv - texelSize * offset[1], layer)) * weight[1];
	color += texture(tex, vec3(uv - texelSize * offset[2], layer)) * weight[2];

	return color;
}

vec4 blurBox(sampler2D tex, vec2 uv, vec2 scale, int size)
{
	vec2 texelSize = scale / textureSize(tex, 0);
	vec4 color = vec4(0.0);

	float midpoint = float(size - 1) / 2.0;

	for (int i = 0; i < size; i++)
	{
		vec2 offset = scale * vec2(float(i) - midpoint);
		color += texture(tex, uv + offset * texelSize);
	}

	return color / size;
}