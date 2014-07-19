#version 330

vec2 vsmMoments(float depth)
{
	vec2 moments = vec2(depth, depth * depth);

	// Adjusting moments (this is sort of bias per pixel) using partial derivative
	float dx = dFdx(depth);
	float dy = dFdy(depth);
	moments.y += 0.25 * (dx * dx + dy * dy);

	return moments;
}

float chebyshevUpperBound(float dist, vec2 moments)
{
	dist = max(dist, moments.x);

	// The fragment is either in shadow or penumbra. We now use chebyshev's upperBound to check
	// How likely this pixel is to be lit (p_max)
	float variance = moments.y - (moments.x * moments.x);
	variance = max(variance, 0.00002);

	float d = dist - moments.x;
	float pMax = variance / (variance + d * d);

	// Reduce light bleeding
	pMax = smoothstep(0.75, 1.0, pMax);

	return pMax;
}