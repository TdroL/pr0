#version 330

vec3 blurGaussian(sampler2D tex, vec2 uv, vec2 scale)
{
	vec3 color = vec3(0.0);

	color += texture(tex, uv + scale * vec2(-3.0, -3.0)).rgb * 0.015625;
	color += texture(tex, uv + scale * vec2(-2.0, -2.0)).rgb * 0.09375;
	color += texture(tex, uv + scale * vec2(-1.0, -1.0)).rgb * 0.234375;
	color += texture(tex, uv + scale * vec2( 0.0,  0.0)).rgb * 0.3125;
	color += texture(tex, uv + scale * vec2( 1.0,  1.0)).rgb * 0.234375;
	color += texture(tex, uv + scale * vec2( 2.0,  2.0)).rgb * 0.09375;
	color += texture(tex, uv + scale * vec2( 3.0,  3.0)).rgb * 0.015625;

	return color;
}