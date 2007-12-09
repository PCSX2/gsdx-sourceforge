cbuffer cb0
{
	float4 VertexScale;
	float4 VertexOffset;
	float2 TextureScale;
};

struct VS_INPUT
{
    float4 p : POSITION; 
	float4 c : COLOR0;
	float4 f : COLOR1;
    float2 t : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 p : SV_Position;
	float4 c : COLOR0;
	float4 t : TEXCOORD0;
};

VS_OUTPUT vs_main(VS_INPUT input)
{
	VS_OUTPUT output;
	
	output.p = input.p * VertexScale - VertexOffset;
	
	output.c = input.c;
	
	output.t.xy = input.t.xy * TextureScale;
	output.t.z = input.f.a;
	output.t.w = input.p.w < 0 ? 1 : input.p.w; // FIXME: <= takes small but not 0 numbers as 0
	
	return output;
}

#ifndef IIP
#define IIP 0
#define PRIM 3
#endif
	
#if PRIM == 0

[maxvertexcount(1)]
void gs_main(point VS_OUTPUT input[1], inout PointStream<VS_OUTPUT> stream)
{
	stream.Append(input[0]);
}

#elif PRIM == 1

[maxvertexcount(2)]
void gs_main(line VS_OUTPUT input[2], inout LineStream<VS_OUTPUT> stream)
{
	#if IIP == 0
	input[0].c = input[1].c;
	input[0].t.z = input[1].t.z;
	#endif

	stream.Append(input[0]);
	stream.Append(input[1]);
}

#elif PRIM == 2

[maxvertexcount(3)]
void gs_main(triangle VS_OUTPUT input[3], inout TriangleStream<VS_OUTPUT> stream)
{
	#if IIP == 0
	input[0].c = input[2].c;
	input[0].t.z = input[2].t.z;
	input[1].c = input[2].c;
	input[1].t.z = input[2].t.z;
	#endif
	
	stream.Append(input[0]);
	stream.Append(input[1]);
	stream.Append(input[2]);
}

#elif PRIM == 3

[maxvertexcount(4)]
void gs_main(line VS_OUTPUT input[2], inout TriangleStream<VS_OUTPUT> stream)
{
	input[0].p.z = input[1].p.z;
	input[0].t.zw = input[1].t.zw;
	
	VS_OUTPUT lb = input[1];
	
	lb.p.x = input[0].p.x;
	lb.t.x = input[0].t.x;
	
	VS_OUTPUT rt = input[1];
	
	rt.p.y = input[0].p.y;
	rt.t.y = input[0].t.y;
	
	stream.Append(input[0]);
	stream.Append(lb);
	stream.Append(rt);
	stream.Append(input[1]);
}

#endif

Texture2D Texture;
Texture2D Palette;
SamplerState Sampler;

cbuffer cb1
{
	float4 FogColor;
	float2 ClampMin;
	float2 ClampMax;
	float TA0;
	float TA1;
	float AREF;
	float _pad;
	float2 WH;
	float2 rWrH;
	float2 rWZ;
	float2 ZrH;
};

struct PS_INPUT
{
	float4 p : SV_Position;
	float4 c : COLOR0;
	float4 t : TEXCOORD0;
};

struct PS_OUTPUT
{
	float4 c0 : SV_Target0;
	float4 c1 : SV_Target1;
};

#ifndef FST
#define FST 0
#define CLAMP 0
#define BPP 0
#define AEM 0
#define TFX 0
#define TCC 1
#define ATE 1
#define ATST 2
#define FOG 0
#define CLR1 0
#define FBA 0
#define AOUT 0
#endif

float4 Normalize16(float4 f)
{
	return f / float4(0x001f, 0x03e0, 0x7c00, 0x8000);
}

float4 Extract16(uint i)
{
	float4 f;

	f.r = i & 0x001f;
	f.g = i & 0x03e0;
	f.b = i & 0x7c00;
	f.a = i & 0x8000;
	
	return f;
}

PS_OUTPUT ps_main(PS_INPUT input)
{
	float2 tc = input.t.xy;

	if(FST == 0)
	{
		tc /= input.t.w;
	}

	if(CLAMP == 1)
	{
		tc = clamp(tc, ClampMin, ClampMax);
	}
	
	float4 t;
	
	if(BPP == 0) // 32
	{
		t = Texture.Sample(Sampler, tc);
	}
	else if(BPP == 1) // 24
	{
		t = Texture.Sample(Sampler, tc);
		
		t.a = AEM == 0 || any(t.rgb) ? TA0 : 0;
	}
	else if(BPP == 2) // 16
	{
		t = Texture.Sample(Sampler, tc);
		
		t.a = t.a >= 0.5 ? TA1 : AEM == 0 || any(t.rgb) ? TA0 : 0; // a bit incompatible with up-scaling because the 1 bit alpha is interpolated
	}
	else if(BPP == 3) // 16P
	{
		// tc -= 0.5 * rWrH; // ?
		
		uint4 i = float4(
			Texture.Sample(Sampler, tc).r,
			Texture.Sample(Sampler, tc + rWZ).r,
			Texture.Sample(Sampler, tc + ZrH).r,
			Texture.Sample(Sampler, tc + rWrH).r) * 65535;
			
		float4 t00 = Extract16(i.x);
		float4 t01 = Extract16(i.y);
		float4 t10 = Extract16(i.z);
		float4 t11 = Extract16(i.w);

		float2 dd = frac(tc * WH); 

		t = lerp(lerp(t00, t01, dd.x), lerp(t10, t11, dd.x), dd.y);
		
		t = Normalize16(t);
		
		t.a = t.a >= 0.5 ? TA1 : AEM == 0 || any(t.rgb) ? TA0 : 0; // a bit incompatible with up-scaling because the 1 bit alpha is interpolated
	}
	else if(BPP == 4) // 8HP / 32-bit palette
	{
		// tc -= 0.5 * rWrH; // ?
		
		float4 f = float4(
			Texture.Sample(Sampler, tc).a,
			Texture.Sample(Sampler, tc + rWZ).a,
			Texture.Sample(Sampler, tc + ZrH).a,
			Texture.Sample(Sampler, tc + rWrH).a);
			
		float4 t00 = Palette.Sample(Sampler, f.x);
		float4 t01 = Palette.Sample(Sampler, f.y);
		float4 t10 = Palette.Sample(Sampler, f.z);
		float4 t11 = Palette.Sample(Sampler, f.w);
		
		float2 dd = frac(tc * WH);
		
		t = lerp(lerp(t00, t01, dd.x), lerp(t10, t11, dd.x), dd.y);
	}
	else if(BPP == 5) // 8HP / 16-bit palette
	{
		// TODO: yuck, just pre-convert the palette to 32-bit
	}
	
	float4 c = input.c;
	
	if(TFX == 0)
	{
		if(TCC == 0) 
		{
			c.rgb = c.rgb * t.rgb * 2;
		}
		else
		{
			c = c * t * 2;
		}
	}
	else if(TFX == 1)
	{
		c = t;
	}
	else if(TFX == 2)
	{
		c.rgb = c.rgb * t.rgb * 2 + c.a;
		
		if(TCC == 1) 
		{
			c.a += t.a;
		}
	}
	else if(TFX == 3)
	{
		c.rgb = c.rgb * t.rgb * 2 + c.a;
		
		if(TCC == 1) 
		{
			c.a = t.a;
		}
	}

	c = saturate(c);
	
	if(ATE == 1)
	{
		if(ATST == 0)
		{
			discard;
		}
		else if(ATST == 2 || ATST == 3) // l, le
		{
			clip(AREF - c.a);
		}
		else if(ATST == 4) // e
		{
			clip(0.9f/256 - abs(c.a - AREF));
		}
		else if(ATST == 5 || ATST == 6) // ge, g
		{
			clip(c.a - AREF);
		}
		else if(ATST == 7) // ne
		{
			clip(abs(c.a - AREF) - 0.9f/256);
		}
	}

	if(FOG == 1)
	{
		c.rgb = lerp(FogColor.rgb, c.rgb, input.t.z);
	}
	
	if(CLR1 == 1) // needed for Cd * (As/Ad/F + 1) blending modes
	{
		c.rgb = 1; 
	}
	
	PS_OUTPUT output;
	
	output.c1 = c.a * 2; // used for alpha blending
	
	if(AOUT == 1) // 16 bit output
	{
		c.a = FBA == 1 ? 0.5 : step(0.5, c.a) * 0.5;
	}
	else if(FBA == 1)
	{
		if(c.a < 0.5) c.a += 0.5;
	}

	output.c0 = c;
	
	return output;
}
