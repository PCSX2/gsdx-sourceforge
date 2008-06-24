/* 
 *	Copyright (C) 2007 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "stdafx.h"
#include "GSRendererHW10.h"
#include "GSTextureCache10.h"
#include "GSCrc.h"
#include "resource.h"

GSRendererHW10::GSRendererHW10(BYTE* base, bool mt, void (*irq)(), int nloophack, const GSRendererSettings& rs)
	: GSRendererHW<GSDevice10, GSVertexHW10>(base, mt, irq, nloophack, rs, true)
{
	m_tc = new GSTextureCache10(this);
}

bool GSRendererHW10::Create(LPCTSTR title)
{
	if(!__super::Create(title))
		return false;

	if(!m_tfx.Create(&m_dev))
		return false;

	//

	D3D10_DEPTH_STENCIL_DESC dsd;

	memset(&dsd, 0, sizeof(dsd));

	dsd.DepthEnable = false;
	dsd.StencilEnable = true;
	dsd.StencilReadMask = 1;
	dsd.StencilWriteMask = 1;
	dsd.FrontFace.StencilFunc = D3D10_COMPARISON_ALWAYS;
	dsd.FrontFace.StencilPassOp = D3D10_STENCIL_OP_REPLACE;
	dsd.FrontFace.StencilFailOp = D3D10_STENCIL_OP_KEEP;
	dsd.FrontFace.StencilDepthFailOp = D3D10_STENCIL_OP_KEEP;
	dsd.BackFace.StencilFunc = D3D10_COMPARISON_ALWAYS;
	dsd.BackFace.StencilPassOp = D3D10_STENCIL_OP_REPLACE;
	dsd.BackFace.StencilFailOp = D3D10_STENCIL_OP_KEEP;
	dsd.BackFace.StencilDepthFailOp = D3D10_STENCIL_OP_KEEP;

	m_dev->CreateDepthStencilState(&dsd, &m_date.dss);

	D3D10_BLEND_DESC bd;

	memset(&bd, 0, sizeof(bd));

	m_dev->CreateBlendState(&bd, &m_date.bs);

	//

	return true;
}

void GSRendererHW10::VertexKick(bool skip)
{
	GSVertexHW10& v = m_vl.AddTail();

#if 0 //_M_SSE >= 0x200

	// TODO: make m_v aligned

	v.m128i[0] = m_v.m128i[0];
	v.m128i[1] = m_v.m128i[1];

	if(PRIM->TME)
	{
		if(PRIM->FST)
		{
			v.ST.S = (float)(int)m_v.UV.U;
			v.ST.T = (float)(int)m_v.UV.V;
			v.RGBAQ.Q = 1.0;
		}
	}

#else

	v.RGBAQ = m_v.RGBAQ;
	v.FOG = m_v.FOG;
	v.XYZ = m_v.XYZ;

	if(PRIM->TME)
	{
		if(PRIM->FST)
		{
			v.ST.S = (float)(int)m_v.UV.U;
			v.ST.T = (float)(int)m_v.UV.V;
			v.RGBAQ.Q = 1.0;
		}
		else
		{
			v.ST = m_v.ST;
		}
	}

#endif

	__super::VertexKick(skip);
}

void GSRendererHW10::DrawingKick(bool skip)
{
	GSVertexHW10* v = &m_vertices[m_count];
	int nv = 0;

	switch(PRIM->PRIM)
	{
	case GS_POINTLIST:
		m_vl.RemoveAt(0, v[0]);
		nv = 1;
		break;
	case GS_LINELIST:
		m_vl.RemoveAt(0, v[0]);
		m_vl.RemoveAt(0, v[1]);
		nv = 2;
		break;
	case GS_LINESTRIP:
		m_vl.RemoveAt(0, v[0]);
		m_vl.GetAt(0, v[1]);
		nv = 2;
		break;
	case GS_TRIANGLELIST:
		m_vl.RemoveAt(0, v[0]);
		m_vl.RemoveAt(0, v[1]);
		m_vl.RemoveAt(0, v[2]);
		nv = 3;
		break;
	case GS_TRIANGLESTRIP:
		m_vl.RemoveAt(0, v[0]);
		m_vl.GetAt(0, v[1]);
		m_vl.GetAt(1, v[2]);
		nv = 3;
		break;
	case GS_TRIANGLEFAN:
		m_vl.GetAt(0, v[0]);
		m_vl.RemoveAt(1, v[1]);
		m_vl.GetAt(1, v[2]);
		nv = 3;
		break;
	case GS_SPRITE:
		m_vl.RemoveAt(0, v[0]);
		m_vl.RemoveAt(0, v[1]);
		nv = 2;
		break;
	default:
		m_vl.RemoveAll();
		ASSERT(0);
		return;
	}

	if(skip)
	{
		return;
	}

	DWORD sx0 = m_context->scissor.x0;
	DWORD sy0 = m_context->scissor.y0;
	DWORD sx1 = m_context->scissor.x1;
	DWORD sy1 = m_context->scissor.y1;

	switch(nv)
	{
	case 1:
		if(v[0].p.x < sx0
		|| v[0].p.x > sx1
		|| v[0].p.y < sy0
		|| v[0].p.y > sy1)
			return;
		break;
	case 2:
		if(v[0].p.x < sx0 && v[1].p.x < sx0
		|| v[0].p.x > sx1 && v[1].p.x > sx1
		|| v[0].p.y < sy0 && v[1].p.y < sy0
		|| v[0].p.y > sy1 && v[1].p.y > sy1)
			return;
		break;
	case 3:
		if(v[0].p.x < sx0 && v[1].p.x < sx0 && v[2].p.x < sx0
		|| v[0].p.x > sx1 && v[1].p.x > sx1 && v[2].p.x > sx1
		|| v[0].p.y < sy0 && v[1].p.y < sy0 && v[2].p.y < sy0
		|| v[0].p.y > sy1 && v[1].p.y > sy1 && v[2].p.y > sy1)
			return;
		break;
	default:
		__assume(0);
	}

	m_count += nv;

	// costs a few fps, but fixes RR's shadows (or anything which paints overlapping shapes with date)
/*
	if(m_context->TEST.DATE)
	{
		Flush();
	}

	if(m_env.COLCLAMP.CLAMP == 0)
	{
		Flush();
	}
*/
}

void GSRendererHW10::Draw()
{
	if(IsBadFrame(m_skip))
	{
		return;
	}

	//

	GIFRegTEX0 TEX0;

	// rt

	TEX0.TBP0 = m_context->FRAME.Block();
	TEX0.TBW = m_context->FRAME.FBW;
	TEX0.PSM = m_context->FRAME.PSM;

	GSTextureCache<GSDevice10>::GSRenderTarget* rt = m_tc->GetRenderTarget(TEX0, m_width, m_height);

	// ds

	TEX0.TBP0 = m_context->ZBUF.Block();
	TEX0.TBW = m_context->FRAME.FBW;
	TEX0.PSM = m_context->ZBUF.PSM;

	GSTextureCache<GSDevice10>::GSDepthStencil* ds = m_tc->GetDepthStencil(TEX0, m_width, m_height);

	// tex

	GSTextureCache<GSDevice10>::GSTexture* tex = NULL;

	if(PRIM->TME)
	{
		tex = m_tc->GetTexture();

		if(!tex) return;
	}

if(s_dump)
{
	CString str;
	str.Format(_T("c:\\temp2\\_%05d_f%I64d_tex_%05x_%d_%d%d_%02x_%02x_%02x_%02x.dds"), 
		s_n++, m_perfmon.GetFrame(), (int)m_context->TEX0.TBP0, (int)m_context->TEX0.PSM,
		(int)m_context->CLAMP.WMS, (int)m_context->CLAMP.WMT, 
		(int)m_context->CLAMP.MINU, (int)m_context->CLAMP.MAXU, 
		(int)m_context->CLAMP.MINV, (int)m_context->CLAMP.MAXV);
	if(PRIM->TME) if(s_save) tex->m_texture.Save(str, true);
	str.Format(_T("c:\\temp2\\_%05d_f%I64d_rt0_%05x_%d.bmp"), s_n++, m_perfmon.GetFrame(), m_context->FRAME.Block(), m_context->FRAME.PSM);
	if(s_save) rt->m_texture.Save(str);
	str.Format(_T("c:\\temp2\\_%05d_f%I64d_rz0_%05x_%d.bmp"), s_n-1, m_perfmon.GetFrame(), m_context->ZBUF.Block(), m_context->ZBUF.PSM);
	if(s_savez) m_dev.SaveToFileD32S8X24(ds->m_texture, str); // TODO
}

	//

	int prim = PRIM->PRIM;
	int prims = 0;

	if(!OverrideInput(prim, rt->m_texture, ds->m_texture, tex ? &tex->m_texture : NULL))
	{
		return;
	}

	D3D10_PRIMITIVE_TOPOLOGY topology;

	switch(prim)
	{
	case GS_POINTLIST:
		topology = D3D10_PRIMITIVE_TOPOLOGY_POINTLIST;
		prims = m_count;
		break;
	case GS_LINELIST: 
	case GS_LINESTRIP:
	case GS_SPRITE:
		topology = D3D10_PRIMITIVE_TOPOLOGY_LINELIST;
		prims = m_count / 2;
		break;
	case GS_TRIANGLELIST: 
	case GS_TRIANGLESTRIP: 
	case GS_TRIANGLEFAN: 
		topology = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		prims = m_count / 3;
		break;
	default:
		__assume(0);
	}

	m_perfmon.Put(GSPerfMon::Prim, prims);
	m_perfmon.Put(GSPerfMon::Draw, 1);

	// date

	SetupDATE(rt->m_texture, ds->m_texture);

	//

	m_dev.BeginScene();

	// om

	GSTextureFX10::OMDepthStencilSelector om_dssel;

	om_dssel.zte = m_context->TEST.ZTE;
	om_dssel.ztst = m_context->TEST.ZTST;
	om_dssel.zwe = !m_context->ZBUF.ZMSK;
	om_dssel.date = m_context->FRAME.PSM != PSM_PSMCT24 ? m_context->TEST.DATE : 0;

	GSTextureFX10::OMBlendSelector om_bsel;

	om_bsel.abe = PRIM->ABE || (PRIM->PRIM == 1 || PRIM->PRIM == 2) && PRIM->AA1;
	om_bsel.a = m_context->ALPHA.A;
	om_bsel.b = m_context->ALPHA.B;
	om_bsel.c = m_context->ALPHA.C;
	om_bsel.d = m_context->ALPHA.D;
	om_bsel.wr = (m_context->FRAME.FBMSK & 0x000000ff) != 0x000000ff;
	om_bsel.wg = (m_context->FRAME.FBMSK & 0x0000ff00) != 0x0000ff00;
	om_bsel.wb = (m_context->FRAME.FBMSK & 0x00ff0000) != 0x00ff0000;
	om_bsel.wa = (m_context->FRAME.FBMSK & 0xff000000) != 0xff000000;

	float bf = (float)(int)m_context->ALPHA.FIX / 0x80;

	// vs

	GSTextureFX10::VSSelector vs_sel;

	vs_sel.bppz = 0;
	vs_sel.tme = PRIM->TME;
	vs_sel.fst = PRIM->FST;
	vs_sel.prim = prim;

	if(om_dssel.zte && om_dssel.ztst > 0 && om_dssel.zwe)
	{
		if(m_context->ZBUF.PSM == PSM_PSMZ24)
		{
			if(WrapZ(0xffffff))
			{
				vs_sel.bppz = 1;
				om_dssel.ztst = 1;
			}
		}
		else if(m_context->ZBUF.PSM == PSM_PSMZ16 || m_context->ZBUF.PSM == PSM_PSMZ16S)
		{
			if(WrapZ(0xffff))
			{
				vs_sel.bppz = 2;
				om_dssel.ztst = 1;
			}
		}
	}

	GSTextureFX10::VSConstantBuffer vs_cb;

	float sx = 2.0f * rt->m_texture.m_scale.x / (rt->m_texture.GetWidth() * 16);
	float sy = 2.0f * rt->m_texture.m_scale.y / (rt->m_texture.GetHeight() * 16);
	float ox = (float)(int)m_context->XYOFFSET.OFX;
	float oy = (float)(int)m_context->XYOFFSET.OFY;

	vs_cb.VertexScale = GSVector4(sx, -sy, 1.0f / UINT_MAX, 0.0f);
	vs_cb.VertexOffset = GSVector4(ox * sx + 1, -(oy * sy + 1), 0.0f, -1.0f);
	vs_cb.TextureScale = GSVector2(1.0f, 1.0f);

	if(PRIM->TME && PRIM->FST)
	{
		vs_cb.TextureScale.x = 1.0f / (16 << m_context->TEX0.TW);
		vs_cb.TextureScale.y = 1.0f / (16 << m_context->TEX0.TH);
	}

	// gs

	GSTextureFX10::GSSelector gs_sel;

	gs_sel.iip = PRIM->IIP;
	gs_sel.prim = GetPrimClass(prim);

	// ps

	GSTextureFX10::PSSelector ps_sel;

	ps_sel.fst = PRIM->FST;
	ps_sel.wms = m_context->CLAMP.WMS;
	ps_sel.wmt = m_context->CLAMP.WMT;
	ps_sel.bpp = 0;
	ps_sel.aem = m_env.TEXA.AEM;
	ps_sel.tfx = m_context->TEX0.TFX;
	ps_sel.tcc = m_context->TEX0.TCC;
	ps_sel.ate = m_context->TEST.ATE;
	ps_sel.atst = m_context->TEST.ATST;
	ps_sel.fog = PRIM->FGE;
	ps_sel.clr1 = om_bsel.abe && om_bsel.a == 1 && om_bsel.b == 2 && om_bsel.d == 1;
	ps_sel.fba = m_context->FBA.FBA;
	ps_sel.aout = m_context->FRAME.PSM == PSM_PSMCT16 || m_context->FRAME.PSM == PSM_PSMCT16S || (m_context->FRAME.FBMSK & 0xff000000) == 0x7f000000 ? 1 : 0;

	GSTextureFX10::PSSamplerSelector ps_ssel;

	ps_ssel.min = m_filter == 2 ? (m_context->TEX1.MMIN & 1) : m_filter;
	ps_ssel.mag = m_filter == 2 ? (m_context->TEX1.MMAG & 1) : m_filter;
	ps_ssel.tau = 0;
	ps_ssel.tav = 0;

	GSTextureFX10::PSConstantBuffer ps_cb;

	ps_cb.FogColor = GSVector4(m_env.FOGCOL.FCR, m_env.FOGCOL.FCG, m_env.FOGCOL.FCB, 0) / 255.0f;
	ps_cb.TA0 = (float)(int)m_env.TEXA.TA0 / 255;
	ps_cb.TA1 = (float)(int)m_env.TEXA.TA1 / 255;
	ps_cb.AREF = (float)(int)m_context->TEST.AREF / 255;

	if(m_context->TEST.ATST == 2 || m_context->TEST.ATST == 5)
	{
		ps_cb.AREF -= 0.9f/256;
	}
	else if(m_context->TEST.ATST == 3 || m_context->TEST.ATST == 6)
	{
		ps_cb.AREF += 0.9f/256;
	}

	if(tex)
	{
		ps_sel.bpp = tex->m_bpp2;

		switch(m_context->CLAMP.WMS)
		{
		case 0: 
			ps_ssel.tau = 1; 
			break;
		case 1: 
			ps_ssel.tau = 0; 
			break;
		case 2: 
			ps_cb.MINU = ((float)(int)m_context->CLAMP.MINU + 0.5f) / (1 << m_context->TEX0.TW);
			ps_cb.MAXU = ((float)(int)m_context->CLAMP.MAXU) / (1 << m_context->TEX0.TW);
			ps_ssel.tau = 0; 
			break;
		case 3: 
			ps_cb.UMSK = m_context->CLAMP.MINU;
			ps_cb.UFIX = m_context->CLAMP.MAXU;
			ps_ssel.tau = 1; 
			break;
		default: 
			__assume(0);
		}

		switch(m_context->CLAMP.WMT)
		{
		case 0: 
			ps_ssel.tav = 1; 
			break;
		case 1: 
			ps_ssel.tav = 0; 
			break;
		case 2: 
			ps_cb.MINV = ((float)(int)m_context->CLAMP.MINV + 0.5f) / (1 << m_context->TEX0.TH);
			ps_cb.MAXV = ((float)(int)m_context->CLAMP.MAXV) / (1 << m_context->TEX0.TH);
			ps_ssel.tav = 0; 
			break;
		case 3: 
			ps_cb.VMSK = m_context->CLAMP.MINV;
			ps_cb.VFIX = m_context->CLAMP.MAXV;
			ps_ssel.tav = 1; 
			break;
		default: 
			__assume(0);
		}

		float w = (float)tex->m_texture.GetWidth();
		float h = (float)tex->m_texture.GetHeight();

		ps_cb.WH = GSVector2(w, h);
		ps_cb.rWrH = GSVector2(1.0f / w, 1.0f / h);
	}
	else
	{
		ps_sel.tfx = 4;
	}

	// rs

	int w = rt->m_texture.GetWidth();
	int h = rt->m_texture.GetHeight();

	CRect scissor(
		(int)(rt->m_texture.m_scale.x * (m_context->SCISSOR.SCAX0)),
		(int)(rt->m_texture.m_scale.y * (m_context->SCISSOR.SCAY0)), 
		(int)(rt->m_texture.m_scale.x * (m_context->SCISSOR.SCAX1 + 1)),
		(int)(rt->m_texture.m_scale.y * (m_context->SCISSOR.SCAY1 + 1)));

	scissor &= CRect(0, 0, w, h);

	//

	m_tfx.SetupOM(om_dssel, om_bsel, bf, rt->m_texture, ds->m_texture);
	m_tfx.SetupIA(m_vertices, m_count, topology);
	m_tfx.SetupVS(vs_sel, &vs_cb);
	m_tfx.SetupGS(gs_sel);
	m_tfx.SetupPS(ps_sel, &ps_cb, ps_ssel, 
		tex ? (ID3D10ShaderResourceView*)tex->m_texture : NULL, 
		tex ? (ID3D10ShaderResourceView*)tex->m_palette : NULL);
	m_tfx.SetupRS(w, h, scissor);

	// draw

	if(m_context->TEST.DoFirstPass())
	{
		m_tfx.Draw();
	}

	if(m_context->TEST.DoSecondPass())
	{
		ASSERT(!m_env.PABE.PABE);

		static const DWORD iatst[] = {1, 0, 5, 6, 7, 2, 3, 4};

		ps_sel.atst = iatst[ps_sel.atst];

		m_tfx.UpdatePS(ps_sel, &ps_cb, ps_ssel);

		bool z = om_dssel.zwe;
		bool r = om_bsel.wr;
		bool g = om_bsel.wg;
		bool b = om_bsel.wb;
		bool a = om_bsel.wa;

		switch(m_context->TEST.AFAIL)
		{
		case 0: z = r = g = b = a = false; break; // none
		case 1: z = false; break; // rgba
		case 2: r = g = b = a = false; break; // z
		case 3: z = a = false; break; // rgb
		default: __assume(0);
		}

		if(z || r || g || b || a)
		{
			om_dssel.zwe = z;
			om_bsel.wr = r;
			om_bsel.wg = g;
			om_bsel.wb = b;
			om_bsel.wa = a;

			m_tfx.UpdateOM(om_dssel, om_bsel, bf);

			m_tfx.Draw();
		}
	}

	m_dev.EndScene();

	OverrideOutput();

if(s_dump)
{
	CString str;
	str.Format(_T("c:\\temp2\\_%05d_f%I64d_rt1_%05x_%d.bmp"), s_n++, m_perfmon.GetFrame(), m_context->FRAME.Block(), m_context->FRAME.PSM);
	if(s_save) rt->m_texture.Save(str);
	str.Format(_T("c:\\temp2\\_%05d_f%I64d_rz1_%05x_%d.bmp"), s_n-1, m_perfmon.GetFrame(), m_context->ZBUF.Block(), m_context->ZBUF.PSM);
	if(s_savez) m_dev.SaveToFileD32S8X24(ds->m_texture, str); // TODO
}

}

bool GSRendererHW10::WrapZ(DWORD maxz)
{
	// should only run once if z values are in the z buffer range

	for(int i = 0, j = m_count; i < j; i++)
	{
		if(m_vertices[i].p.z <= maxz)
		{
			return false;
		}
	}

	return true;
}

void GSRendererHW10::SetupDATE(Texture& rt, Texture& ds)
{
	if(!m_context->TEST.DATE) return; // || (::GetAsyncKeyState(VK_CONTROL) & 0x8000)

	// sfex3 (after the capcom logo), vf4 (first menu fading in), ffxii shadows, rumble roses shadows

	GSVector4 mm;

	// TODO

	mm = GSVector4(-1, -1, 1, 1);
/*
	MinMaxXY(mm);

	int w = rt.GetWidth();
	int h = rt.GetHeight();

	float sx = 2.0f * rt.m_scale.x / (w * 16);
	float sy = 2.0f * rt.m_scale.y / (h * 16);	
	float ox = (float)(int)m_context->XYOFFSET.OFX;
	float oy = (float)(int)m_context->XYOFFSET.OFY;

	mm.x = (mm.x - ox) * sx - 1;
	mm.y = (mm.y - oy) * sy - 1;
	mm.z = (mm.z - ox) * sx - 1;
	mm.w = (mm.w - oy) * sy - 1;

	if(mm.x < -1) mm.x = -1;
	if(mm.y < -1) mm.y = -1;
	if(mm.z > +1) mm.z = +1;
	if(mm.w > +1) mm.w = +1;
*/
	GSVector4 uv = (mm + 1.0f) / 2.0f;

	//

	m_dev.BeginScene();

	// om

	GSTexture10 tmp;

	m_dev.CreateRenderTarget(tmp, rt.GetWidth(), rt.GetHeight());

	m_dev.OMSetRenderTargets(tmp, ds);
	m_dev.OMSetDepthStencilState(m_date.dss, 1);
	m_dev.OMSetBlendState(m_date.bs, 0);

	m_dev->ClearDepthStencilView(ds, D3D10_CLEAR_STENCIL, 0, 0);

	// ia

	GSVertexPT1 vertices[] =
	{
		{GSVector4(mm.x, -mm.y), GSVector2(uv.x, uv.y)},
		{GSVector4(mm.z, -mm.y), GSVector2(uv.z, uv.y)},
		{GSVector4(mm.x, -mm.w), GSVector2(uv.x, uv.w)},
		{GSVector4(mm.z, -mm.w), GSVector2(uv.z, uv.w)},
	};

	D3D10_BOX box = {0, 0, 0, sizeof(vertices), 1, 1};
	m_dev->UpdateSubresource(m_dev.m_convert.vb, 0, &box, vertices, 0, 0);

	m_dev.IASetVertexBuffer(m_dev.m_convert.vb, sizeof(vertices[0]));
	m_dev.IASetInputLayout(m_dev.m_convert.il);
	m_dev.IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// vs

	m_dev.VSSetShader(m_dev.m_convert.vs, NULL);

	// gs

	m_dev.GSSetShader(NULL);

	// ps

	m_dev.PSSetShaderResources(rt, NULL);
	m_dev.PSSetShader(m_dev.m_convert.ps[m_context->TEST.DATM ? 2 : 3], NULL);
	m_dev.PSSetSamplerState(m_dev.m_convert.pt);

	// rs

	m_dev.RSSet(tmp.GetWidth(), tmp.GetHeight());

	// set

	m_dev.DrawPrimitive(countof(vertices));

	//

	m_dev.EndScene();

	m_dev.Recycle(tmp);
}
