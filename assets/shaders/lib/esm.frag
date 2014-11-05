#version 330

float esmLogConv(float x0,float X, float y0, float Y)
{
	return (X + log(x0 + (y0 * exp(Y - X))));
}

float esmLogBlur(sampler2D tex, vec2 uv, vec2 scale)
{
	float blur[7];
	float sample[7];

	blur[0] = 0.015625;
	blur[1] = 0.09375;
	blur[2] = 0.234375;
	blur[3] = 0.3125;
	blur[4] = 0.234375;
	blur[5] = 0.09375;
	blur[6] = 0.015625;

	vec2 texelSize = scale / textureSize(tex, 0);

	sample[0] = texture(tex, uv + texelSize * -3.0).b;
	sample[1] = texture(tex, uv + texelSize * -2.0).b;
	sample[2] = texture(tex, uv + texelSize * -1.0).b;
	sample[3] = texture(tex, uv + texelSize *  0.0).b;
	sample[4] = texture(tex, uv + texelSize *  1.0).b;
	sample[5] = texture(tex, uv + texelSize *  2.0).b;
	sample[6] = texture(tex, uv + texelSize *  3.0).b;

	float acc = esmLogConv(blur[0], sample[0], blur[1], sample[1]);

	for (int i = 2; i < 7; i++)
	{
		acc = esmLogConv(1.0, acc, blur[i], sample[i]);
	}

	return acc;
}

float esmVisibility(float dist, float depth)
{
	float c = 1.0;
	return exp(c*(dist - depth));
}