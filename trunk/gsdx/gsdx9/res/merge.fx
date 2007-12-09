
sampler RAO1 : register(s0);
sampler RAO2 : register(s1);

float4 Params1 : register(c0);

#define BGColor	(Params1)
#define Alpha	(Params1.a)

float4 Params2 : register(c1);

#define EN1		(Params2[0])
#define EN2		(Params2[1])
#define MMOD	(Params2[2] >= 0)
#define SLBG	(Params2[3] >= 0)

struct PS_INPUT
{
	float2 t0 : TEXCOORD0;
	float2 t1 : TEXCOORD1;
};

float4 ps_main(PS_INPUT input) : COLOR
{
	float4 c0 = EN1 * tex2D(RAO1, input.t0);
	float4 c1 = SLBG ? BGColor : EN2 * tex2D(RAO2, input.t1);
	float a = EN1 * (MMOD ? Alpha : min(c0.a * 2, 1));
	return lerp(c1, c0, a).bgra;
}

