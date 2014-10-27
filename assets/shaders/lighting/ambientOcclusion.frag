#version 330

layout(location = 0) out vec4 outColor;

in vec2 uv;

uniform sampler2D texNormal;
uniform sampler2D texColor;
uniform sampler2D texDepth;

uniform vec2 filterRadius;

vec3 normalDecode(vec2 enc);
vec3 positionReconstruct(float z, vec2 uv);

const int kernelSize = 16;
const vec3 kernel[] = vec3[](
	vec3(0.018444309930676236, -0.01585005544113947, 0.0035282220461234784),
	vec3(-0.044859687514876526, -0.005329715771205972, 0.06754857691196124),
	vec3(-0.0038485382642773243, -0.004352382511300488, 0.006534074231161479),
	vec3(0.0010136433019542106, 0.0004961919512010594, 0.0005817016462038329),
	vec3(-0.025152742377179668, -0.05998497009165362, 0.04029527086701834),
	vec3(-0.06875659583728748, -0.12951151462025925, 0.06530055915124229),
	vec3(0.017005501626420415, 0.013011041545852291, 0.019846007800684528),
	vec3(-0.05919260350079405, 0.10470023204419025, 0.1924894319468266),
	vec3(-0.06732759383610883, 0.07482083967667019, 0.02326412618284116),
	vec3(-0.0518393678395067, -0.016803243237516682, 0.011920281758925242),
	vec3(0.20929591361233052, 0.13771200293820515, 0.31006891588600477),
	vec3(0.15053303277076746, -0.32055664841652703, 0.18643959962456713),
	vec3(0.027539842058443648, -0.009418488501194046, 0.018386735310452953),
	vec3(0.1630311084052609, 0.43138127280715416, 0.19406387601576933),
	vec3(0.5080596917794714, -0.3024327057183682, 0.05018614941213072),
	vec3(-0.008830867290950629, -0.005458797594230522, 0.00478483981207492)
);

const vec3 noise[] = vec3[](
	vec3(-0.5091595031050833, 0.569949743404252, 0),
	vec3(-0.9334178387270256, 0.46812988691341206, 0),
	vec3(0.7443704983134103, -0.5304336725534602, 0),
	vec3(0.18991184120127058, 0.4739378254385538, 0),
	vec3(-0.19564512262720934, 0.282984489368191, 0),
	vec3(0.39704377307792726, -0.032296657046295474, 0),
	vec3(0.2109529589874517, 0.8203207873347875, 0),
	vec3(-0.4339176236370228, -0.14218355673988503, 0),
	vec3(-0.4252415940980443, -0.3578796429390838, 0),
	vec3(0.9788023674200077, 0.5645592576867826, 0),
	vec3(-0.6017791727488269, 0.5231415058355584, 0),
	vec3(0.6820190218027284, 0.4465458825390256, 0),
	vec3(0.9127707644885943, 0.7651720829773034, 0),
	vec3(0.5177679834688313, -0.39447883060778133, 0),
	vec3(0.24225559901573068, 0.12895757148691844, 0),
	vec3(-0.912995057844951, 0.5202625901236555, 0)
);

void main()
{
	vec2 encodedNormal = texture(texNormal, uv).xy;
	float depth = texture(texDepth, uv).x * 2.0 - 1.0;
	vec3 albedo = texture(texColor, uv).rgb;

	vec3 normal = normalDecode(encodedNormal);
	vec3 position = positionReconstruct(depth, uv);

	float ambientOcclusion = 0;
	float distanceThreshold = 0.5;

	vec3 rvec = texture(uTexRandom, vTexcoord * uNoiseScale).xyz * 2.0 - 1.0;
	vec3 tangent = normalize(rvec - normal * dot(rvec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 tbn = mat3(tangent, bitangent, normal);

	for (int i = 0; i < kernelSize; ++i)
	{
		/*
		// sample at an offset specified by the current Poisson-Disk sample and scale it by a radius (has to be in Texture-Space)
		vec2 sampleUV = uv + (poissonDisk[i] * filterRadius);
		float sampleDepth = texture(texDepth, sampleUV).r * 2.0 - 1.0;
		vec3 samplePos = positionReconstruct(sampleDepth, sampleUV);
		vec3 sampleDir = normalize(samplePos - position);

		// angle between SURFACE-NORMAL and SAMPLE-DIRECTION (vector from SURFACE-POSITION to SAMPLE-POSITION)
		float NdotS = max(dot(normal, sampleDir), 0);
		// distance between SURFACE-POSITION and SAMPLE-POSITION
		float VPdistSP = distance(position, samplePos);

		// a = distance function
		float a = 1.0 - smoothstep(distanceThreshold, distanceThreshold * 2, VPdistSP);
		// b = dot-Product
		float b = NdotS;

		ambientOcclusion += (a * b);
		*/
	}

	outColor.rgb = vec3(0.0);
	outColor.a = ambientOcclusion / kernelSize;
	// outColor.a = 0.0;
}