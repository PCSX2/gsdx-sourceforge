struct VS_INPUT
{
    float4 p : POSITION; 
    float2 t0 : TEXCOORD0;
    float2 t1 : TEXCOORD1;
};

struct VS_OUTPUT
{
	float4 p : SV_Position;
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

Texture2D RA01;
Texture2D RA02;
SamplerState Sampler;

cbuffer cb1
{
	float4 BGColor;
	float Alpha;
	float EN1;
	float EN2;
	int MMOD;
	int SLBG;
};

struct PS_INPUT
{
	float4 p : SV_Position;
	float2 t0 : TEXCOORD0;
	float2 t1 : TEXCOORD1;
};

float4 ps_main(PS_INPUT input) : SV_Target0
{
	float4 c0 = EN1 * RA01.Sample(Sampler, input.t0);
	float4 c1 = SLBG ? BGColor : EN2 * RA02.Sample(Sampler, input.t1);
	float a = EN1 * (MMOD ? Alpha : min(c0.a * 2, 1));
	return lerp(c1, c0, a);
}
