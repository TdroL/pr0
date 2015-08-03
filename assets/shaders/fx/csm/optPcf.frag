#version 333 core

float optPcfVisibility(vec3 shadowCoord, int csmLayer, float kernelSize)
{
	vec2 texSize = textureSize(csmTexDepths, 0).xy;
	vec2 texSizeInv = 1.0 / texSize;

	float depthBias = texSizeInv.x;
	float lightDepth = shadowCoord.z + depthBias;

	vec2 uv = shadowCoord.xy * texSize;

	vec2 baseUv = floor(uv + 0.5);
	float s = fract(uv.x);
	float t = fract(uv.y);

	baseUv -= 0.5;
	baseUv *= texSizeInv;

	// kernelSize = 5
	float uw0 = (4.0 - 3.0 * s);
	float uw1 = 7.0;
	float uw2 = (1.0 + 3.0 * s);

	float u0 = (3.0 - 2.0 * s) / uw0 - 2.0;
	float u1 = (3.0 + s) / uw1;
	float u2 = s / uw2 + 2.0;

	float vw0 = (4.0 - 3.0 * t);
	float vw1 = 7.0;
	float vw2 = (1.0 + 3.0 * t);

	float v0 = (3.0 - 2.0 * t) / vw0 - 2.0;
	float v1 = (3.0 + t) / vw1;
	float v2 = t / vw2 + 2.0;

	vec2 texSizeInvScaled = texSizeInv / float(csmLayer + 1.0);

	float visibility = 1.0;
	visibility += uw0 * vw0 * texture(csmTexDepths, vec4(baseUv + vec2(u0, v0) * texSizeInvScaled, csmLayer, lightDepth));
	visibility += uw1 * vw0 * texture(csmTexDepths, vec4(baseUv + vec2(u1, v0) * texSizeInvScaled, csmLayer, lightDepth));
	visibility += uw2 * vw0 * texture(csmTexDepths, vec4(baseUv + vec2(u2, v0) * texSizeInvScaled, csmLayer, lightDepth));

	visibility += uw0 * vw1 * texture(csmTexDepths, vec4(baseUv + vec2(u0, v1) * texSizeInvScaled, csmLayer, lightDepth));
	visibility += uw1 * vw1 * texture(csmTexDepths, vec4(baseUv + vec2(u1, v1) * texSizeInvScaled, csmLayer, lightDepth));
	visibility += uw2 * vw1 * texture(csmTexDepths, vec4(baseUv + vec2(u2, v1) * texSizeInvScaled, csmLayer, lightDepth));

	visibility += uw0 * vw2 * texture(csmTexDepths, vec4(baseUv + vec2(u0, v2) * texSizeInvScaled, csmLayer, lightDepth));
	visibility += uw1 * vw2 * texture(csmTexDepths, vec4(baseUv + vec2(u1, v2) * texSizeInvScaled, csmLayer, lightDepth));
	visibility += uw2 * vw2 * texture(csmTexDepths, vec4(baseUv + vec2(u2, v2) * texSizeInvScaled, csmLayer, lightDepth));

	visibility = visibility * 1.0f / 144;

	return visibility;
}