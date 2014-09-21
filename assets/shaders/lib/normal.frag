#version 330

vec2 normalEncode(vec3 n)
{
	float scale = 1.7777;
	return (n.xy / (n.z+1)) / scale * 0.5 + 0.5;
}

vec3 normalDecode(vec2 enc)
{
	float scale = 1.7777;
	vec3 nn = vec3(enc.xy * 2.0 * scale - scale, 1.0);
	float g = 2.0 / dot(nn, nn);
	vec3 n = vec3(g * nn.xy, g - 1.0);

	return n;
}