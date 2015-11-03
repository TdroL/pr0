#version 440 core

// Normal Distribution Function (NDF) or D(h)
// GGX (Trowbridge-Reitz)
float distributionGGX(float h_n, float alpha)
{
	float alpha2 = alpha * alpha;
	float denom = (h_n * alpha2 - h_n) * h_n + 1.0;

	return (alpha2 / (3.14159 * denom * denom));
}

float distributionGGXDisney(float h_n, float alpha)
{
	float alpha2 = alpha * alpha;
	float denom = h_n * h_n * (alpha2 - 1.0) + 1.0;

	return (alpha2 / (3.14159 * denom));
}

float distributionGGX1886(float h_n, float alpha)
{
	float denom = h_n * h_n * (alpha - 1.0) + 1.0;

	return (alpha / (3.14159 * denom));
}

// Fresnel term F(l, h)
// Fnone(l, h) = F(0°)
float fresnelSchlick(float f0, float l_h)
{
	return f0 + (1.0 - f0) * pow(1.0 - l_h, 5.0);
}

vec3 fresnelSchlick3(vec3 f0, float l_h)
{
	return f0 + (1.0 - f0) * pow(1.0 - l_h, 5.0);
}

// Visibility term G(l, v, h)
float visibilitySchlick(float n_v, float n_l, float alpha)
{
	float k = alpha * 0.5;

	float denomL = (n_l * (1.0 - k) + k);
	float denomV = (n_v * (1.0 - k) + k);

	return (0.25 / (denomL * denomV));
}

float visibilitySchlickOpt(float l_h, float alpha)
{
	float k = alpha * 0.5;
	float denom = 1.0 / (l_h * (1.0 - k) + k);

	vis = 1.0 / (denom * denom);
}

float visibilitySmithGGX(float n_v, float n_l, float alpha)
{
	float vis1 = n_l + sqrt(alpha + (1.0 - alpha) * n_l * n_l);
	float vis2 = n_v + sqrt(alpha + (1.0 - alpha) * n_v * n_v);

	return (1.0 / max(vis1 * vis2, 0.15));
}

float a1vf(float g)
{
	return (0.25 * g + 0.75);
}

float a004(float g, float n_v)
{
	float t = min(0.475 * g, exp2(-9.28 * n_v));
	return (t + 0.0275) * g + 0.015;
}

float a0r(float g, float n_v)
{
	return ((a004(g, n_v) - a1vf(g) * 0.04) / 0.96);
}

vec3 environmentBRDF(float g, float n_v, vec3 f0)
{
	vec4 t = vec4(1.0 / 0.96, 0.475, (0.0275 - 0.25 * 0.04) / 0.96, 0.25);
	t *= vec4(g);
	t += vec4(0.0, 0.0, (0.015 - 0.75 * 0.04) / 0.96, 0.75);

	float a0 = t.x * min(t.y, exp2(-9.28 * n_v)) + t.z;
	float a1 = t.w;

	return clamp(a0 + f0 * (a1 - a0), 0.0, 1.0);
}

vec3 environmentBRDFApprox(float roughness, float n_v, vec3 f0)
{
	const vec4 c0 = vec4(-1.0, -0.0275, -0.572, 0.022);
	const vec4 c1 = vec4(1.0, 0.0425, 1.04, -0.04);

	vec4 r = roughness * c0 + c1;
	float a004 = min(r.x * r.x, exp2(-9.28 * n_v)) * r.x + r.y;
	vec2 AB = vec2(-1.04, 1.04) * a004 + r.zw;

	return f0 * AB.x + AB.y;

}