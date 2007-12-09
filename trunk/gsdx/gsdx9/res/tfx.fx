struct VS_INPUT
{
    float4 p : POSITION; 
	float4 c : COLOR0;
	float4 f : COLOR1;
    float2 t : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 p : POSITION;
	float4 c : COLOR0;
	float4 t : TEXCOORD0;
};

float4 vs_params[3];

#define VertexScale vs_params[0]
#define VertexOffset vs_params[1]
#define TextureScale vs_params[2].xy

VS_OUTPUT vs_main(VS_INPUT input)
{
	VS_OUTPUT output;
	
	output.p = input.p * VertexScale - VertexOffset;
	//
	output.p.z = log2(1.0f + input.p.z) / 32;
	
	output.c = input.c;
	
	output.t.xy = input.t * TextureScale;
	output.t.z = input.f.a;
	output.t.w = input.p.w < 0 ? 1 : input.p.w; // FIXME: <= takes small but not 0 numbers as 0
	
	return output;
}

sampler Texture : register(s0);
sampler1D Palette : register(s1);

float4 ps_params[5];

#define FogColor	ps_params[0]
#define ClampMin	ps_params[1].xy
#define ClampMax	ps_params[1].zw
#define TA0			ps_params[2].x
#define TA1			ps_params[2].y
#define AREF		ps_params[2].z
#define WH			ps_params[3].xy
#define rWrH		ps_params[3].zw
#define rWZ			ps_params[4].xy
#define ZrH			ps_params[4].zw

struct PS_INPUT
{
	float4 p : POSITION;
	float4 c : COLOR0;
	float4 t : TEXCOORD0;
};

#define EPSILON (0.001/256)

#ifndef FST
#define FST 0
#define CLAMP 0
#define BPP 0
#define AEM 0
#define TFX 0
#define TCC 1
#define ATE 0
#define ATST 1
#define FOG 0
#define CLR1 0
#define RT 0
#endif

float4 ps_main(PS_INPUT input) : COLOR
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
		t = tex2D(Texture, tc);
		
		if(RT == 1) t.a *= 0.5;
	}
	else if(BPP == 1) // 24
	{
		t = tex2D(Texture, tc);
		
		t.a = AEM == 0 || any(t.rgb) ? TA0 : 0;
	}
	else if(BPP == 2) // 16
	{
		t = tex2D(Texture, tc);
		
		t.a = t.a >= 0.5 ? TA1 : AEM == 0 || any(t.rgb) ? TA0 : 0; // a bit incompatible with up-scaling because the 1 bit alpha is interpolated
	}
	else if(BPP == 3) // 8HP ln
	{
		tc -= 0.5 * rWrH; // ?

		float4 f = float4(
			tex2D(Texture, tc).a,
			tex2D(Texture, tc + rWZ).a,
			tex2D(Texture, tc + ZrH).a,
			tex2D(Texture, tc + rWrH).a) * 0.5 - EPSILON;
		
		float4 t00 = tex1D(Palette, f.x);
		float4 t01 = tex1D(Palette, f.y);
		float4 t10 = tex1D(Palette, f.z);
		float4 t11 = tex1D(Palette, f.w);
		
		float2 dd = frac(tc * WH); 
		
		t = lerp(lerp(t00, t01, dd.x), lerp(t10, t11, dd.x), dd.y);
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
	
	c.a *= 2;
	
	return c;
}
