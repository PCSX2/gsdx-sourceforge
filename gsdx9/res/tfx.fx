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

#ifndef LOGZ
#define LOGZ 1
#endif

VS_OUTPUT vs_main(VS_INPUT input)
{
	VS_OUTPUT output;

	output.p = input.p * VertexScale - VertexOffset;

	if(LOGZ == 1)
	{
		output.p.z = log2(1.0f + input.p.z) / 32;
	}

	output.c = input.c;

	output.t.xy = input.t * TextureScale;
	output.t.z = input.f.a;
	output.t.w = input.p.w < 0 ? 1 : input.p.w; // FIXME: <= takes small but not 0 numbers as 0

	return output;
}

sampler Texture : register(s0);
sampler1D Palette : register(s1);
sampler1D UMSKFIX : register(s2);
sampler1D VMSKFIX : register(s3);

float4 ps_params[6];

#define FogColor	ps_params[0]
#define MINU		ps_params[1].x
#define MAXU		ps_params[1].y
#define MINV		ps_params[1].z
#define MAXV		ps_params[1].w
#define UMSK		ps_params[2].x
#define UFIX		ps_params[2].y
#define VMSK		ps_params[2].z
#define VFIX		ps_params[2].w
#define TA0			ps_params[3].x
#define TA1			ps_params[3].y
#define AREF		ps_params[3].z
#define WH			ps_params[4].xy
#define rWrH		ps_params[4].zw
#define rWZ			ps_params[5].xy
#define ZrH			ps_params[5].zw

struct PS_INPUT
{
	float4 c : COLOR0;
	float4 t : TEXCOORD0;
};

#define EPSILON (0.001/256)

#ifndef FST
#define FST 0
#define WMS 3
#define WMT 3
#define BPP 0
#define AEM 0
#define TFX 0
#define TCC 1
#define ATE 0
#define ATST 0
#define FOG 0
#define CLR1 0
#define RT 0
#endif

float repeatu(float tc)
{
	return WMS == 3 ? tex1D(UMSKFIX, tc) : tc;
}

float repeatv(float tc)
{
	return WMT == 3 ? tex1D(VMSKFIX, tc) : tc;
}

float4 sample(float2 tc)
{
	float4 t;
	
	if(WMS == 3 || WMT == 3)
	{
		float4 tc01;
		
		tc01.x = repeatu(tc.x); 
		tc01.y = repeatv(tc.y);
		tc01.z = repeatu(tc.x + rWrH.x); 
		tc01.w = repeatv(tc.y + rWrH.y);

		float4 t00 = tex2D(Texture, tc01.xy);
		float4 t01 = tex2D(Texture, tc01.zy);
		float4 t10 = tex2D(Texture, tc01.xw);
		float4 t11 = tex2D(Texture, tc01.zw);
	
		float2 dd = frac(tc * WH); 

		t = lerp(lerp(t00, t01, dd.x), lerp(t10, t11, dd.x), dd.y);
	}
	else
	{
		t = tex2D(Texture, tc);
	}
	
	return t;
}

float4 sample8hp(float2 tc)
{
	// tc -= 0.5 * rWrH; // ?

	float4 t;
	
	float4 tc01;
	
	tc01.x = repeatu(tc.x); 
	tc01.y = repeatv(tc.y);
	tc01.z = repeatu(tc.x + rWrH.x); 
	tc01.w = repeatv(tc.y + rWrH.y);

	t.x = tex2D(Texture, tc01.xy).a;
	t.y = tex2D(Texture, tc01.zy).a;
	t.z = tex2D(Texture, tc01.xw).a;
	t.w = tex2D(Texture, tc01.zw).a;

	float4 t00 = tex1D(Palette, t.x);
	float4 t01 = tex1D(Palette, t.y);
	float4 t10 = tex1D(Palette, t.z);
	float4 t11 = tex1D(Palette, t.w);

	float2 dd = frac(tc * WH); 

	return lerp(lerp(t00, t01, dd.x), lerp(t10, t11, dd.x), dd.y);
}

float4 ps_main(PS_INPUT input) : COLOR
{
	float2 tc = input.t.xy;

	if(FST == 0)
	{
		tc /= input.t.w;
	}

	if(WMS == 2)
	{
		tc.x = clamp(tc.x, MINU, MAXU);
	}
	
	if(WMT == 2)
	{
		tc.y = clamp(tc.y, MINV, MAXV);
	}

	float4 t;

	if(BPP == 0) // 32
	{
		t = sample(tc);

		if(RT == 1) t.a *= 0.5;
	}
	else if(BPP == 1) // 24
	{
		t = sample(tc);

		t.a = AEM == 0 || any(t.rgb) ? TA0 : 0;
	}
	else if(BPP == 2) // 16
	{
		t = sample(tc);

		t.a = t.a >= 0.5 ? TA1 : AEM == 0 || any(t.rgb) ? TA0 : 0; // a bit incompatible with up-scaling because the 1 bit alpha is interpolated
	}
	else if(BPP == 3) // 8HP ln
	{
		t = sample8hp(tc);
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
