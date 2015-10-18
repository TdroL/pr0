#version 440 core

vec2 normalEncode(vec3 n)
{
	/**/
	float scale = 1.0 / 1.7777; // 9 / 16
	return (n.xy / (1.0 - n.z)) * scale * 0.5 + 0.5;
	/*/
	float p = sqrt(-n.z * 8.0 + 8.0);
	return vec2(n.xy / p + 0.5);
	/**/
}

vec3 normalDecode(vec2 enc)
{
	/**/
	float scale = 1.7777; // 16 / 9
	vec3 nn = vec3(enc.xy * 2.0 * scale - scale, 1.0);
	float g = 2.0 / dot(nn, nn);
	return vec3(g * nn.xy, 1.0 - g);
	/*/
	vec2 fenc = enc * 4.0 - 2.0;
	float f = dot(fenc, fenc);
	float g = sqrt(1.0 - f / 4.0);
	return vec3(fenc * g, -1.0 + f / 2.0);
	/**/
}