struct VS_INPUT
{
	float4 p : POSITION; 
	float2 t0 : TEXCOORD0;
	float2 t1 : TEXCOORD1;
};

struct VS_OUTPUT
{
	float4 p : POSITION;
	float2 t0 : TEXCOORD0;
	float2 t1 : TEXCOORD1;
};

VS_OUTPUT vs_main(VS_INPUT input)
{
	VS_OUTPUT output;

	output.p = input.p;
	output.t0 = input.t0;
	output.t1 = input.t1;

	return output;
}

sampler RAO1 : register(s0);
sampler RAO2 : register(s1);

float4 g_params[1];

#define BGColor	(g_params[0])

struct PS_INPUT
{
	float2 t0 : TEXCOORD0;
	float2 t1 : TEXCOORD1;
};

#ifndef EN1
#define EN1 1
#define EN2 0
#define SLBG 0
#define MMOD 1
#endif

float4 ps_main(PS_INPUT input) : COLOR
{
	float4 c0 = EN1 * tex2D(RAO1, input.t0);
	float4 c1 = SLBG ? BGColor : EN2 * tex2D(RAO2, input.t1);
	float a = EN1 * (MMOD ? BGColor.a : c0.a);
	return lerp(c1, c0, a).bgra;
}

