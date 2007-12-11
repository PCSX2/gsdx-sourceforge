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
#include "GSRendererHW.h"
#include "resource.h"

int s_n = 0;
bool s_dump = false;
bool s_save = false;

GSRendererHW::GSRendererHW(BYTE* base, bool mt, void (*irq)(), bool nloophack)
	: GSRendererT(base, mt, irq, nloophack)
	, m_tc(this)
	, m_width(1024)
	, m_height(1024)
	, m_skip(0)
{
	if(!AfxGetApp()->GetProfileInt(_T("Settings"), _T("nativeres"), FALSE))
	{
		m_width = AfxGetApp()->GetProfileInt(_T("Settings"), _T("resx"), 1024);
		m_height = AfxGetApp()->GetProfileInt(_T("Settings"), _T("resy"), 1024);
	}
}

bool GSRendererHW::Create(LPCTSTR title)
{
	if(!__super::Create(title))
		return false;

	if(!m_tfx.Create(&m_dev))
		return false;

	return true;
}

void GSRendererHW::VertexKick(bool skip)
{
	GSVertexHW& v = m_vl.AddTail();

	v.x = (float)m_v.XYZ.X;
	v.y = (float)m_v.XYZ.Y;
	v.z = (float)m_v.XYZ.Z;

	v.c = m_v.RGBAQ.ai32[0];

	v.f = m_v.FOG.ai32[1];

	if(PRIM->TME)
	{
		if(PRIM->FST)
		{
			v.w = 1.0f;
			v.u = (float)(int)m_v.UV.U;
			v.v = (float)(int)m_v.UV.V;
		}
		else
		{
			v.w = m_v.RGBAQ.Q;
			v.u = m_v.ST.S;
			v.v = m_v.ST.T;
		}
	}
	else
	{
		v.w = 1.0f;
		v.u = 0;
		v.v = 0;
	}

	__super::VertexKick(skip);
}

void GSRendererHW::DrawingKick(bool skip)
{
	GSVertexHW* v = &m_vertices[m_count];
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
		// ASSERT(v[0].z == v[1].z);
		v[0].z = v[1].z;
		v[0].w = v[1].w;
		v[0].f = v[1].f;
		v[2] = v[1];
		v[3] = v[1];
		v[1].y = v[0].y;
		v[1].v = v[0].v;
		v[2].x = v[0].x;
		v[2].u = v[0].u;
		v[4] = v[1];
		v[5] = v[2];
		nv = 6;
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

	float sx0 = m_context->scissor.x0;
	float sy0 = m_context->scissor.y0;
	float sx1 = m_context->scissor.x1;
	float sy1 = m_context->scissor.y1;

	switch(nv)
	{
	case 1:
		if(v[0].x < sx0
		|| v[0].x > sx1
		|| v[0].y < sy0
		|| v[0].y > sy1)
			return;
		break;
	case 2:
		if(v[0].x < sx0 && v[1].x < sx0
		|| v[0].x > sx1 && v[1].x > sx1
		|| v[0].y < sy0 && v[1].y < sy0
		|| v[0].y > sy1 && v[1].y > sy1)
			return;
		break;
	case 3:
		if(v[0].x < sx0 && v[1].x < sx0 && v[2].x < sx0
		|| v[0].x > sx1 && v[1].x > sx1 && v[2].x > sx1
		|| v[0].y < sy0 && v[1].y < sy0 && v[2].y < sy0
		|| v[0].y > sy1 && v[1].y > sy1 && v[2].y > sy1)
			return;
		break;
	case 6:
		if(v[0].x < sx0 && v[3].x < sx0
		|| v[0].x > sx1 && v[3].x > sx1
		|| v[0].y < sy0 && v[3].y < sy0
		|| v[0].y > sy1 && v[3].y > sy1)
			return;
		break;
	default:
		__assume(0);
	}

	if(!PRIM->IIP)
	{
		v[0].c = v[nv - 1].c;

		if(PRIM->PRIM == 6)
		{
			v[3].c = v[5].c;
		}
	}

	m_count += nv;
}

void GSRendererHW::Draw()
{
	if(DetectBadFrame(m_crc, m_skip))
	{
		return;
	}

	//

	GIFRegTEX0 TEX0;

	// rt

	TEX0.TBP0 = m_context->FRAME.Block();
	TEX0.TBW = m_context->FRAME.FBW;
	TEX0.PSM = m_context->FRAME.PSM;

	GSTextureCache::GSRenderTarget* rt = m_tc.GetRenderTarget(TEX0, m_width, m_height);

	// ds

	TEX0.TBP0 = m_context->ZBUF.Block();
	TEX0.TBW = m_context->FRAME.FBW;
	TEX0.PSM = m_context->ZBUF.PSM;

	GSTextureCache::GSDepthStencil* ds = m_tc.GetDepthStencil(TEX0, m_width, m_height);

	// tex

	GSTextureCache::GSTexture* tex = NULL;

	if(PRIM->TME)
	{
		tex = m_tc.GetTexture();

		if(!tex) return;
	}

if(s_dump)
{
	CString str;
	str.Format(_T("c:\\temp2\\_%05d_f%I64d_tex_%05x_%d.bmp"), s_n++, m_perfmon.GetFrame(), (int)m_context->TEX0.TBP0, (int)m_context->TEX0.PSM);
	if(PRIM->TME) if(s_save) ::D3DXSaveTextureToFile(str, D3DXIFF_BMP, tex->m_texture, NULL);
	str.Format(_T("c:\\temp2\\_%05d_f%I64d_rt0_%05x_%d.bmp"), s_n++, m_perfmon.GetFrame(), m_context->FRAME.Block(), m_context->FRAME.PSM);
	if(s_save) ::D3DXSaveTextureToFile(str, D3DXIFF_BMP, rt->m_texture, NULL);
}

	int prim = PRIM->PRIM;
	int prims = 0;

	if(!OverrideInput(prim, tex))
	{
		return;
	}

	D3DPRIMITIVETYPE topology;

	switch(prim)
	{
	case GS_POINTLIST:
		topology = D3DPT_POINTLIST;
		prims = m_count;
		break;
	case GS_LINELIST: 
	case GS_LINESTRIP:
		topology = D3DPT_LINELIST;
		prims = m_count / 2;
		break;
	case GS_TRIANGLELIST: 
	case GS_TRIANGLESTRIP: 
	case GS_TRIANGLEFAN: 
	case GS_SPRITE:
		topology = D3DPT_TRIANGLELIST;
		prims = m_count / 3;
		break;
	default:
		__assume(0);
	}

	m_perfmon.Put(GSPerfMon::Prim, prims);
	m_perfmon.Put(GSPerfMon::Draw, 1);

	// date

	SetupDATE(rt, ds);

	//

	HRESULT hr;

	hr = m_dev->BeginScene();

	hr = m_dev->SetRenderState(D3DRS_SHADEMODE, PRIM->IIP ? D3DSHADE_GOURAUD : D3DSHADE_FLAT);

	// om

	GSTextureFX::OMDepthStencilSelector om_dssel;

	om_dssel.zte = m_context->TEST.ZTE;
	om_dssel.ztst = m_context->TEST.ZTST;
	om_dssel.zwe = !m_context->ZBUF.ZMSK;
	om_dssel.date = m_context->TEST.DATE;

	GSTextureFX::OMBlendSelector om_bsel;

	om_bsel.abe = PRIM->ABE || (PRIM->PRIM == 1 || PRIM->PRIM == 2) && PRIM->AA1;
	om_bsel.a = m_context->ALPHA.A;
	om_bsel.b = m_context->ALPHA.B;
	om_bsel.c = m_context->ALPHA.C;
	om_bsel.d = m_context->ALPHA.D;
	om_bsel.wr = (m_context->FRAME.FBMSK & 0x000000ff) != 0x000000ff;
	om_bsel.wg = (m_context->FRAME.FBMSK & 0x0000ff00) != 0x0000ff00;
	om_bsel.wb = (m_context->FRAME.FBMSK & 0x00ff0000) != 0x00ff0000;
	om_bsel.wa = (m_context->FRAME.FBMSK & 0xff000000) != 0xff000000;

	BYTE bf = m_context->ALPHA.FIX;

	m_tfx.SetupOM(om_dssel, om_bsel, bf, rt->m_texture, ds->m_texture);

	// vs

	GSTextureFX::VSConstantBuffer vs_cb;

	float sx = 2.0f * rt->m_scale.x / (rt->m_texture.m_desc.Width * 16);
	float sy = 2.0f * rt->m_scale.y / (rt->m_texture.m_desc.Height * 16);
	float ox = (float)(int)m_context->XYOFFSET.OFX;
	float oy = (float)(int)m_context->XYOFFSET.OFY;

	vs_cb.VertexScale = D3DXVECTOR4(sx, -sy, 1.0f / UINT_MAX, 0);
	vs_cb.VertexOffset = D3DXVECTOR4(ox * sx + 1, -(oy * sy + 1), 0, -1);
	vs_cb.TextureScale = D3DXVECTOR2(1.0f, 1.0f);

	if(PRIM->TME && PRIM->FST)
	{
		vs_cb.TextureScale.x = 1.0f / (16 << m_context->TEX0.TW);
		vs_cb.TextureScale.y = 1.0f / (16 << m_context->TEX0.TH);
	}

	m_tfx.SetupVS(&vs_cb);

	// ps

	GSTextureFX::PSSelector ps_sel;

	ps_sel.fst = PRIM->FST;
	ps_sel.clamp = 0;
	ps_sel.bpp = 0;
	ps_sel.aem = m_env.TEXA.AEM;
	ps_sel.tfx = m_context->TEX0.TFX;
	ps_sel.tcc = m_context->TEX0.TCC;
	ps_sel.ate = m_context->TEST.ATE;
	ps_sel.atst = m_context->TEST.ATST;
	ps_sel.fog = PRIM->FGE;
	ps_sel.clr1 = om_bsel.abe && om_bsel.a == 1 && om_bsel.b == 2 && om_bsel.d == 1;
	ps_sel.rt = tex && tex->m_rendered;

	GSTextureFX::PSSamplerSelector ps_ssel;

	ps_ssel.min = m_filter == 2 ? (m_context->TEX1.MMIN & 1) : m_filter;
	ps_ssel.mag = m_filter == 2 ? (m_context->TEX1.MMAG & 1) : m_filter;
	ps_ssel.tau = 0;
	ps_ssel.tav = 0;

	GSTextureFX::PSConstantBuffer ps_cb;

	ps_cb.FogColor = D3DXVECTOR4((float)(int)m_env.FOGCOL.FCB / 255, (float)(int)m_env.FOGCOL.FCG / 255, (float)(int)m_env.FOGCOL.FCR / 255, 0);
	ps_cb.ClampMin = D3DXVECTOR2(-4096, -4096);
	ps_cb.ClampMax = D3DXVECTOR2(+4096, +4096);
	ps_cb.TA0 = (float)(int)m_env.TEXA.TA0 / 255;
	ps_cb.TA1 = (float)(int)m_env.TEXA.TA1 / 255;
	ps_cb.AREF = (float)(int)m_context->TEST.AREF / 255;

#ifdef PS_ALPHA_TEST

	if(m_context->TEST.ATST == 2 || m_context->TEST.ATST == 5)
	{
		ps_cb.AREF -= 0.9f/256;
	}
	else if(m_context->TEST.ATST == 3 || m_context->TEST.ATST == 6)
	{
		ps_cb.AREF += 0.9f/256;
	}

#endif

	if(tex)
	{
		ps_sel.bpp = tex->m_bpp2;

		switch(m_context->CLAMP.WMS)
		{
		case 0: case 3: ps_ssel.tau = 1; break;
		case 1: case 2: ps_ssel.tau = 0; break;
		default: __assume(0);
		}

		switch(m_context->CLAMP.WMT)
		{
		case 0: case 3: ps_ssel.tav = 1; break;
		case 1: case 2: ps_ssel.tav = 0; break;
		default: __assume(0);
		}

		if(m_context->CLAMP.WMS == 2)
		{
			ps_cb.ClampMin.x = (float)(int)m_context->CLAMP.MINU / (1 << m_context->TEX0.TW);
			ps_cb.ClampMax.x = (float)(int)m_context->CLAMP.MAXU / (1 << m_context->TEX0.TW);
			ps_sel.clamp = 1;
		}

		if(m_context->CLAMP.WMT == 2)
		{
			ps_cb.ClampMin.y = (float)(int)m_context->CLAMP.MINV / (1 << m_context->TEX0.TH);
			ps_cb.ClampMax.y = (float)(int)m_context->CLAMP.MAXV / (1 << m_context->TEX0.TH);
			ps_sel.clamp = 1;
		}

		float w = (float)(int)tex->m_texture.m_desc.Width;
		float h = (float)(int)tex->m_texture.m_desc.Height;

		ps_cb.WH = D3DXVECTOR2(w, h);
		ps_cb.rWrH = D3DXVECTOR2(1.0f / w, 1.0f / h);
		ps_cb.rWZ = D3DXVECTOR2(1.0f / w, 0);
		ps_cb.ZrH = D3DXVECTOR2(0, 1.0f / h);

		m_tfx.SetupPS(ps_sel, &ps_cb, ps_ssel, tex->m_texture, tex->m_palette);
	}
	else
	{
		ps_sel.tfx = 4;

		m_tfx.SetupPS(ps_sel, &ps_cb, ps_ssel, NULL, NULL);
	}

	// rs

	UINT w = rt->m_texture.m_desc.Width;
	UINT h = rt->m_texture.m_desc.Height;

	CRect scissor(
		(int)(rt->m_scale.x * (m_context->SCISSOR.SCAX0)),
		(int)(rt->m_scale.y * (m_context->SCISSOR.SCAY0)), 
		(int)(rt->m_scale.x * (m_context->SCISSOR.SCAX1 + 1)),
		(int)(rt->m_scale.y * (m_context->SCISSOR.SCAY1 + 1)));

	scissor &= CRect(0, 0, w, h);

	m_tfx.SetupRS(scissor);

	// draw

	if(!m_context->TEST.ATE || m_context->TEST.ATST != 0)
	{
		m_dev->DrawPrimitiveUP(topology, prims, m_vertices, sizeof(m_vertices[0]));
	}

	if(m_context->TEST.ATE && m_context->TEST.ATST != 1 && m_context->TEST.AFAIL)
	{
		ASSERT(!m_env.PABE.PABE);

		static const DWORD iatst[] = {1, 0, 5, 6, 7, 2, 3, 4};

		ps_sel.atst = iatst[ps_sel.atst];

		m_tfx.UpdatePS(ps_sel, ps_ssel);

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

			m_dev->DrawPrimitiveUP(topology, prims, m_vertices, sizeof(m_vertices[0]));
		}
	}










/*





	SetupVertexShader(rt);

	SetupTexture(tex);

	SetupAlphaBlend();

	SetupColorMask();

	SetupZBuffer();

	SetupAlphaTest();

	SetupScissor(rt->m_scale);

	SetupFrameBufferAlpha();

	if(!m_context->TEST.ATE || m_context->TEST.ATST != 0)
	{
		hr = m_dev->DrawPrimitiveUP(prim, nPrims, m_pVertices, sizeof(GSVertexHW));
	}

	if(m_context->TEST.ATE && m_context->TEST.ATST != 1 && m_context->TEST.AFAIL)
	{
		ASSERT(!m_env.PABE.PABE);

		static const DWORD iafunc[] = {D3DCMP_ALWAYS, D3DCMP_NEVER, D3DCMP_GREATEREQUAL, D3DCMP_GREATER, D3DCMP_NOTEQUAL, D3DCMP_LESS, D3DCMP_LESSEQUAL, D3DCMP_EQUAL};

		hr = m_dev->SetRenderState(D3DRS_ALPHAFUNC, iafunc[m_context->TEST.ATST]);
		hr = m_dev->SetRenderState(D3DRS_ALPHAREF, (DWORD)SCALE_ALPHA(m_context->TEST.AREF));

		DWORD mask = 0;
		bool zwrite = false;

		hr = m_dev->GetRenderState(D3DRS_COLORWRITEENABLE, &mask);

		switch(m_context->TEST.AFAIL)
		{
		case 0: mask = 0; break; // keep
		case 1: break; // fbuf
		case 2: mask = 0; zwrite = !m_context->ZBUF.ZMSK; break; // zbuf
		case 3: mask &= ~D3DCOLORWRITEENABLE_ALPHA; break; // fbuf w/o alpha
		default: __assume(0);
		}

		hr = m_dev->SetRenderState(D3DRS_ZWRITEENABLE, zwrite);
		hr = m_dev->SetRenderState(D3DRS_COLORWRITEENABLE, mask);

		if(mask || zwrite)
		{
			hr = m_dev->DrawPrimitiveUP(prim, nPrims, m_pVertices, sizeof(GSVertexHW));
		}
	}
*/
	hr = m_dev->EndScene();

	// UpdateFrameBufferAlpha(rt);

	hr = m_dev->SetRenderState(D3DRS_STENCILENABLE, FALSE);

if(s_dump)
{
	CString str;
	str.Format(_T("c:\\temp2\\_%05d_f%I64d_rt1_%05x_%d.bmp"), s_n++, m_perfmon.GetFrame(), m_context->FRAME.Block(), m_context->FRAME.PSM);
	if(s_save) ::D3DXSaveTextureToFile(str, D3DXIFF_BMP, rt->m_texture, NULL);
}

}

void GSRendererHW::Flip()
{
	FlipInfo src[2];

	for(int i = 0; i < countof(src); i++)
	{
		if(!IsEnabled(i))
		{
			continue;
		}

		GIFRegTEX0 TEX0;

		TEX0.TBP0 = DISPFB[i]->Block();
		TEX0.TBW = DISPFB[i]->FBW;
		TEX0.PSM = DISPFB[i]->PSM;

		if(GSTextureCache::GSRenderTarget* rt = m_tc.GetRenderTarget(TEX0, m_width, m_height, true))
		{
			src[i].t = rt->m_texture;
			src[i].s = rt->m_scale;

if(s_dump)
{
	CString str;
	str.Format(_T("c:\\temp2\\_%05d_f%I64d_fr%d_%05x_%d.bmp"), s_n++, m_perfmon.GetFrame(), i, (int)TEX0.TBP0, (int)TEX0.PSM);
	if(s_save) ::D3DXSaveTextureToFile(str, D3DXIFF_BMP, rt->m_texture, NULL);
}
		}

// s_dump = m_perfmon.GetFrame() >= 540;

	}

	FinishFlip(src);

	m_tc.IncAge();

	m_skip = 0;
}

void GSRendererHW::InvalidateTexture(const GIFRegBITBLTBUF& BITBLTBUF, CRect r)
{
	// TRACE(_T("[%d] InvalidateTexture %d,%d - %d,%d %05x\n"), (int)m_perfmon.GetFrame(), r.left, r.top, r.right, r.bottom, (int)BITBLTBUF.DBP);

	m_tc.InvalidateTexture(BITBLTBUF, &r);
}

void GSRendererHW::InvalidateLocalMem(const GIFRegBITBLTBUF& BITBLTBUF, CRect r)
{
	// TRACE(_T("[%d] InvalidateLocalMem %d,%d - %d,%d %05x\n"), (int)m_perfmon.GetFrame(), r.left, r.top, r.right, r.bottom, (int)BITBLTBUF.SBP);

	m_tc.InvalidateLocalMem(BITBLTBUF, &r);
}

void GSRendererHW::MinMaxUV(int w, int h, CRect& r)
{
	r.SetRect(0, 0, w, h);

	if(m_count > 100) 
	{
		return;
	}

	if(m_context->CLAMP.WMS < 3 || m_context->CLAMP.WMT < 3)
	{
		uvmm_t uv;

		uv.umin = uv.vmin = 0;
		uv.umax = uv.vmax = 1;

		if(PRIM->FST)
		{
			UVMinMax(m_count, (vertex_t*)m_vertices, &uv);

			uv.umin *= 1.0f / (16 << m_context->TEX0.TW);
			uv.umax *= 1.0f / (16 << m_context->TEX0.TW);
			uv.vmin *= 1.0f / (16 << m_context->TEX0.TH);
			uv.vmax *= 1.0f / (16 << m_context->TEX0.TH);
		}
		else
		{
			// FIXME

			if(m_count > 0)// && m_count < 100)
			{
				uv.umin = uv.vmin = +1e10;
				uv.umax = uv.vmax = -1e10;

				for(int i = 0, j = m_count; i < j; i++)
				{
					float w = 1.0f / m_vertices[i].w;
					float u = m_vertices[i].u * w;
					if(uv.umax < u) uv.umax = u;
					if(uv.umin > u) uv.umin = u;
					float v = m_vertices[i].v * w;
					if(uv.vmax < v) uv.vmax = v;
					if(uv.vmin > v) uv.vmin = v;
				}
			}
		}

		CSize bs = GSLocalMemory::m_psm[m_context->TEX0.PSM].bs;

		CSize bsm(bs.cx-1, bs.cy-1);

		if(m_context->CLAMP.WMS < 3)
		{
			if(m_context->CLAMP.WMS == 0)
			{
				float fmin = floor(uv.umin);
				float fmax = floor(uv.umax);

				if(fmin != fmax) {uv.umin = 0; uv.umax = 1.0f;}
				else {uv.umin -= fmin; uv.umax -= fmax;}

				// FIXME: 
				if(uv.umin == 0 && uv.umax != 1.0f) uv.umax = 1.0f;
			}
			else if(m_context->CLAMP.WMS == 1)
			{
				if(uv.umin < 0) uv.umin = 0;
				else if(uv.umin > 1.0f) uv.umin = 1.0f;
				if(uv.umax < 0) uv.umax = 0;
				else if(uv.umax > 1.0f) uv.umax = 1.0f;
				if(uv.umin > uv.umax) uv.umin = uv.umax;
			}
			else if(m_context->CLAMP.WMS == 2)
			{
				float minu = 1.0f * m_context->CLAMP.MINU / w;
				float maxu = 1.0f * m_context->CLAMP.MAXU / w;
				if(uv.umin < minu) uv.umin = minu;
				else if(uv.umin > maxu) uv.umin = maxu;
				if(uv.umax < minu) uv.umax = minu;
				else if(uv.umax > maxu) uv.umax = maxu;
				if(uv.umin > uv.umax) uv.umin = uv.umax;
			}

			r.left = max((int)(uv.umin * w) & ~bsm.cx, 0);
			r.right = min(((int)(uv.umax * w) + bsm.cx + 1) & ~bsm.cx, w);
		}

		if(m_context->CLAMP.WMT < 3)
		{
			if(m_context->CLAMP.WMT == 0)
			{
				float fmin = floor(uv.vmin);
				float fmax = floor(uv.vmax);

				if(fmin != fmax) {uv.vmin = 0; uv.vmax = 1.0f;}
				else {uv.vmin -= fmin; uv.vmax -= fmax;}

				// FIXME: 
				if(uv.vmin == 0 && uv.vmax != 1.0f) uv.vmax = 1.0f;
			}
			else if(m_context->CLAMP.WMT == 1)
			{
				if(uv.vmin < 0) uv.vmin = 0;
				else if(uv.vmin > 1.0f) uv.vmin = 1.0f;
				if(uv.vmax < 0) uv.vmax = 0;
				else if(uv.vmax > 1.0f) uv.vmax = 1.0f;
				if(uv.vmin > uv.vmax) uv.vmin = uv.vmax;
			}
			else if(m_context->CLAMP.WMT == 2)
			{
				float minv = 1.0f * m_context->CLAMP.MINV / h;
				float maxv = 1.0f * m_context->CLAMP.MAXV / h;
				if(uv.vmin < minv) uv.vmin = minv;
				else if(uv.vmin > maxv) uv.vmin = maxv;
				if(uv.vmax < minv) uv.vmax = minv;
				else if(uv.vmax > maxv) uv.vmax = maxv;
				if(uv.vmin > uv.vmax) uv.vmin = uv.vmax;
			}
			
			r.top = max((int)(uv.vmin * h) & ~bsm.cy, 0);
			r.bottom = min(((int)(uv.vmax * h) + bsm.cy + 1) & ~bsm.cy, h);
		}
	}

	//ASSERT(r.left <= r.right);
	//ASSERT(r.top <= r.bottom);
}
void GSRendererHW::SetupDATE(GSTextureCache::GSRenderTarget* rt, GSTextureCache::GSDepthStencil* ds)
{
	if(!m_context->TEST.DATE) return; // || (::GetAsyncKeyState(VK_CONTROL)&0x80000000)

	// sfex3 (after the capcom logo), vf4 (first menu fading in), ffxii shadows, rumble roses shadows

	float xmin = 0, xmax = 1;
	float ymin = 0, ymax = 1;

	// if(m_count < 1000) {

#if _M_IX86_FP >= 2 || defined(_M_AMD64)
		
	__m128 xymin = _mm_set1_ps(+1e10);
	__m128 xymax = _mm_set1_ps(-1e10);

	for(int i = 0, j = m_count; i < j; i++)
	{
		xymin = _mm_min_ps(m_vertices[i].m128[0], xymin);
		xymax = _mm_max_ps(m_vertices[i].m128[0], xymax);
	}

	xmin = xymin.m128_f32[0];
	ymin = xymin.m128_f32[1];
	xmax = xymax.m128_f32[0];
	ymax = xymax.m128_f32[1];

#else	

	xmin = ymin = +1e10;
	xmax = ymax = -1e10;

	for(int i = 0, j = m_count; i < j; i++)
	{
		float x = m_vertices[i].x;

		if(x < xmin) xmin = x;
		if(x > xmax) xmax = x;
		
		float y = m_vertices[i].y;

		if(y < ymin) ymin = y;
		if(y > ymax) ymax = y;
	}

#endif

	int w = rt->m_texture.m_desc.Width;
	int h = rt->m_texture.m_desc.Height;

	float sx = rt->m_scale.x / (w * 16);
	float sy = rt->m_scale.y / (h * 16);

	float ox = (float)(int)m_context->XYOFFSET.OFX;
	float oy = (float)(int)m_context->XYOFFSET.OFY;

	xmin = (xmin - ox) * sx;
	xmax = (xmax - ox) * sx;
	ymin = (ymin - oy) * sy;
	ymax = (ymax - oy) * sy;

	if(xmin < 0) xmin = 0;
	if(xmax > 1) xmax = 1;
	if(ymin < 0) ymin = 0;
	if(ymax > 1) ymax = 1;

	// }

	GSTexture2D tmp;

	m_dev.CreateRenderTarget(tmp, rt->m_texture.m_desc.Width, rt->m_texture.m_desc.Height);

	m_dev->SetRenderTarget(0, tmp);
	m_dev->SetDepthStencilSurface(ds->m_texture);

	m_dev->Clear(0, NULL, D3DCLEAR_STENCIL, 0, 0, 0);

	m_dev->SetTexture(0, rt->m_texture);
	
	m_dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	m_dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);

	m_dev->SetRenderState(D3DRS_ZENABLE, FALSE);
	m_dev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	m_dev->SetRenderState(D3DRS_STENCILENABLE, TRUE);
	m_dev->SetRenderState(D3DRS_STENCILREF, 1);
	m_dev->SetRenderState(D3DRS_STENCILMASK, 1);
	m_dev->SetRenderState(D3DRS_STENCILWRITEMASK, 1);	
	m_dev->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
	m_dev->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);
	m_dev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	m_dev->SetRenderState(D3DRS_ALPHAFUNC, m_context->TEST.DATM ? D3DCMP_EQUAL : D3DCMP_LESS);
	m_dev->SetRenderState(D3DRS_ALPHAREF, 0xff);
	m_dev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_dev->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
	m_dev->SetRenderState(D3DRS_COLORWRITEENABLE, 0);

	m_dev->SetVertexShader(NULL);
	m_dev->SetPixelShader(m_dev.m_ps_convert[0]);

	VertexPT1 vertices[] =
	{
		{xmin * w, ymin * h, 0.5f, 1.0f, xmin, ymin},
		{xmax * w, ymin * h, 0.5f, 1.0f, xmax, ymin},
		{xmin * w, ymax * h, 0.5f, 1.0f, xmin, ymax},
		{xmax * w, ymax * h, 0.5f, 1.0f, xmax, ymax},
	};

	m_dev->BeginScene();
	m_dev->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
	m_dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(vertices[0]));
	m_dev->EndScene();

	m_dev.Recycle(tmp);
}

bool GSRendererHW::OverrideInput(int& prim, GSTextureCache::GSTexture* tex)
{
	#pragma region ffxii pal video conversion

	if(m_crc == 0x78da0252 || m_crc == 0xc1274668 || m_crc == 0xdc2a467e || m_crc == 0xca284668)
	{
		static DWORD* video = NULL;
		static bool ok = false;

		if(prim == GS_POINTLIST && m_count >= 448*448 && m_count <= 448*512)
		{
			// incoming pixels are stored in columns, one column is 16x512, total res 448x512 or 448x454

			if(!video) video = new DWORD[512*512];

			for(int x = 0, i = 0, rows = m_count / 448; x < 448; x += 16)
			{
				DWORD* dst = &video[x];

				for(int y = 0; y < rows; y++, dst += 512)
				{
					for(int j = 0; j < 16; j++, i++)
					{
						dst[j] = m_vertices[i].c;
					}
				}
			}

			ok = true;

			return false;
		}
		else if(prim == GS_LINELIST && m_count == 512*2 && ok)
		{
			// normally, this step would copy the video onto screen with 512 texture mapped horizontal lines,
			// but we use the stored video data to create a new texture, and replace the lines with two triangles

			ok = false;

			m_dev.Recycle(tex->m_texture);
			m_dev.Recycle(tex->m_palette);

			m_dev.CreateTexture(tex->m_texture, 512, 512);

			D3DLOCKED_RECT lr;
			
			if(SUCCEEDED(tex->m_texture->LockRect(0, &lr, NULL, 0)))
			{
				BYTE* bits = (BYTE*)lr.pBits;

				for(int i = 0; i < 512; i++, bits += lr.Pitch)
				{
					memcpy(bits, &video[i*512], 448*4);
				}

				tex->m_texture->UnlockRect(0);
			}

			m_vertices[0] = m_vertices[0];
			m_vertices[1] = m_vertices[1];
			m_vertices[2] = m_vertices[m_count - 2];
			m_vertices[3] = m_vertices[1];
			m_vertices[4] = m_vertices[2];
			m_vertices[5] = m_vertices[m_count - 1];

			prim = GS_TRIANGLELIST;
			m_count = 6;

			return true;
		}
	}

	#pragma endregion

	return true;
}






/*



void GSRendererHW::SetupAlphaBlend()
{
	// ASSERT(!m_env.PABE.PABE); // bios

//static __int64 s_frame = 0;
//if(m_env.PABE.PABE && s_frame != m_perfmon.GetFrame()) {printf("PABE\n"); s_frame = m_perfmon.GetFrame();}

	HRESULT hr;

	bool ABE = m_pPRIM->ABE || (m_pPRIM->PRIM == 1 || m_pPRIM->PRIM == 2) && m_pPRIM->AA1; // FIXME

	hr = m_dev->SetRenderState(D3DRS_ALPHABLENDENABLE, ABE);

	if(!ABE) return;

	// (A:Cs/Cd/0 - B:Cs/Cd/0) * C:As/Ad/FIX + D:Cs/Cd/0

	BYTE FIX = SCALE_ALPHA(m_context->ALPHA.FIX);

	hr = m_dev->SetRenderState(D3DRS_BLENDFACTOR, (0x010101*FIX) | (FIX<<24));

	D3DBLENDOP op = D3DBLENDOP_ADD;
	D3DBLEND src = D3DBLEND_SRCALPHA, dst = D3DBLEND_INVSRCALPHA;

	static const struct {bool bogus; D3DBLENDOP op; D3DBLEND src, dst;} blendmap[3*3*3*3] =
	{
		{0, D3DBLENDOP_ADD, D3DBLEND_ONE, D3DBLEND_ZERO},						// 0000: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cs ==> Cs
		{0, D3DBLENDOP_ADD, D3DBLEND_ZERO, D3DBLEND_ONE},						// 0001: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cd ==> Cd
		{0, D3DBLENDOP_ADD, D3DBLEND_ZERO, D3DBLEND_ZERO},						// 0002: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + 0 ==> 0
		{0, D3DBLENDOP_ADD, D3DBLEND_ONE, D3DBLEND_ZERO},						// 0010: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cs ==> Cs
		{0, D3DBLENDOP_ADD, D3DBLEND_ZERO, D3DBLEND_ONE},						// 0011: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cd ==> Cd
		{0, D3DBLENDOP_ADD, D3DBLEND_ZERO, D3DBLEND_ZERO},						// 0012: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + 0 ==> 0
		{0, D3DBLENDOP_ADD, D3DBLEND_ONE, D3DBLEND_ZERO},						// 0020: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cs ==> Cs
		{0, D3DBLENDOP_ADD, D3DBLEND_ZERO, D3DBLEND_ONE},						// 0021: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cd ==> Cd
		{0, D3DBLENDOP_ADD, D3DBLEND_ZERO, D3DBLEND_ZERO},						// 0022: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + 0 ==> 0
		{1, D3DBLENDOP_SUBTRACT, D3DBLEND_SRCALPHA, D3DBLEND_SRCALPHA},			// * 0100: (Cs - Cd)*As + Cs ==> Cs*(As + 1) - Cd*As
		{0, D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA},			// 0101: (Cs - Cd)*As + Cd ==> Cs*As + Cd*(1 - As)
		{0, D3DBLENDOP_SUBTRACT, D3DBLEND_SRCALPHA, D3DBLEND_SRCALPHA},			// 0102: (Cs - Cd)*As + 0 ==> Cs*As - Cd*As
		{1, D3DBLENDOP_SUBTRACT, D3DBLEND_DESTALPHA, D3DBLEND_DESTALPHA},		// * 0110: (Cs - Cd)*Ad + Cs ==> Cs*(Ad + 1) - Cd*Ad
		{0, D3DBLENDOP_ADD, D3DBLEND_DESTALPHA, D3DBLEND_INVDESTALPHA},			// 0111: (Cs - Cd)*Ad + Cd ==> Cs*Ad + Cd*(1 - Ad)
		{0, D3DBLENDOP_ADD, D3DBLEND_DESTALPHA, D3DBLEND_DESTALPHA},			// 0112: (Cs - Cd)*Ad + 0 ==> Cs*Ad - Cd*Ad
		{1, D3DBLENDOP_SUBTRACT, D3DBLEND_BLENDFACTOR, D3DBLEND_BLENDFACTOR},	// * 0120: (Cs - Cd)*F + Cs ==> Cs*(F + 1) - Cd*F
		{0, D3DBLENDOP_ADD, D3DBLEND_BLENDFACTOR, D3DBLEND_INVBLENDFACTOR},		// 0121: (Cs - Cd)*F + Cd ==> Cs*F + Cd*(1 - F)
		{0, D3DBLENDOP_SUBTRACT, D3DBLEND_BLENDFACTOR, D3DBLEND_BLENDFACTOR},	// 0122: (Cs - Cd)*F + 0 ==> Cs*F - Cd*F
		{1, D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_ZERO},					// * 0200: (Cs - 0)*As + Cs ==> Cs*(As + 1)
		{0, D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_ONE},					// 0201: (Cs - 0)*As + Cd ==> Cs*As + Cd
		{0, D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_ZERO},					// 0202: (Cs - 0)*As + 0 ==> Cs*As
		{1, D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_ZERO},					// * 0210: (Cs - 0)*Ad + Cs ==> Cs*(As + 1)
		{0, D3DBLENDOP_ADD, D3DBLEND_DESTALPHA, D3DBLEND_ONE},					// 0211: (Cs - 0)*Ad + Cd ==> Cs*Ad + Cd
		{0, D3DBLENDOP_ADD, D3DBLEND_DESTALPHA, D3DBLEND_ZERO},					// 0212: (Cs - 0)*Ad + 0 ==> Cs*Ad
		{1, D3DBLENDOP_ADD, D3DBLEND_BLENDFACTOR, D3DBLEND_ZERO},				// * 0220: (Cs - 0)*F + Cs ==> Cs*(F + 1)
		{0, D3DBLENDOP_ADD, D3DBLEND_BLENDFACTOR, D3DBLEND_ONE},				// 0221: (Cs - 0)*F + Cd ==> Cs*F + Cd
		{0, D3DBLENDOP_ADD, D3DBLEND_BLENDFACTOR, D3DBLEND_ZERO},				// 0222: (Cs - 0)*F + 0 ==> Cs*F
		{0, D3DBLENDOP_ADD, D3DBLEND_INVSRCALPHA, D3DBLEND_SRCALPHA},			// 1000: (Cd - Cs)*As + Cs ==> Cd*As + Cs*(1 - As)
		{1, D3DBLENDOP_REVSUBTRACT, D3DBLEND_SRCALPHA, D3DBLEND_SRCALPHA},		// * 1001: (Cd - Cs)*As + Cd ==> Cd*(As + 1) - Cs*As
		{0, D3DBLENDOP_REVSUBTRACT, D3DBLEND_SRCALPHA, D3DBLEND_SRCALPHA},		// 1002: (Cd - Cs)*As + 0 ==> Cd*As - Cs*As
		{0, D3DBLENDOP_ADD, D3DBLEND_INVDESTALPHA, D3DBLEND_DESTALPHA},			// 1010: (Cd - Cs)*Ad + Cs ==> Cd*Ad + Cs*(1 - Ad)
		{1, D3DBLENDOP_REVSUBTRACT, D3DBLEND_DESTALPHA, D3DBLEND_DESTALPHA},	// * 1011: (Cd - Cs)*Ad + Cd ==> Cd*(Ad + 1) - Cs*Ad
		{0, D3DBLENDOP_REVSUBTRACT, D3DBLEND_DESTALPHA, D3DBLEND_DESTALPHA},	// 1012: (Cd - Cs)*Ad + 0 ==> Cd*Ad - Cs*Ad
		{0, D3DBLENDOP_ADD, D3DBLEND_INVBLENDFACTOR, D3DBLEND_BLENDFACTOR},		// 1020: (Cd - Cs)*F + Cs ==> Cd*F + Cs*(1 - F)
		{1, D3DBLENDOP_REVSUBTRACT, D3DBLEND_BLENDFACTOR, D3DBLEND_BLENDFACTOR},// * 1021: (Cd - Cs)*F + Cd ==> Cd*(F + 1) - Cs*F
		{0, D3DBLENDOP_REVSUBTRACT, D3DBLEND_BLENDFACTOR, D3DBLEND_BLENDFACTOR},// 1022: (Cd - Cs)*F + 0 ==> Cd*F - Cs*F
		{0, D3DBLENDOP_ADD, D3DBLEND_ONE, D3DBLEND_ZERO},						// 1100: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cs ==> Cs
		{0, D3DBLENDOP_ADD, D3DBLEND_ZERO, D3DBLEND_ONE},						// 1101: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cd ==> Cd
		{0, D3DBLENDOP_ADD, D3DBLEND_ZERO, D3DBLEND_ZERO},						// 1102: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + 0 ==> 0
		{0, D3DBLENDOP_ADD, D3DBLEND_ONE, D3DBLEND_ZERO},						// 1110: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cs ==> Cs
		{0, D3DBLENDOP_ADD, D3DBLEND_ZERO, D3DBLEND_ONE},						// 1111: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cd ==> Cd
		{0, D3DBLENDOP_ADD, D3DBLEND_ZERO, D3DBLEND_ZERO},						// 1112: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + 0 ==> 0
		{0, D3DBLENDOP_ADD, D3DBLEND_ONE, D3DBLEND_ZERO},						// 1120: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cs ==> Cs
		{0, D3DBLENDOP_ADD, D3DBLEND_ZERO, D3DBLEND_ONE},						// 1121: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cd ==> Cd
		{0, D3DBLENDOP_ADD, D3DBLEND_ZERO, D3DBLEND_ZERO},						// 1122: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + 0 ==> 0
		{0, D3DBLENDOP_ADD, D3DBLEND_ONE, D3DBLEND_SRCALPHA},					// 1200: (Cd - 0)*As + Cs ==> Cs + Cd*As
		{1, D3DBLENDOP_ADD, D3DBLEND_ZERO, D3DBLEND_SRCALPHA},					// * 1201: (Cd - 0)*As + Cd ==> Cd*(1 + As)  // ffxii main menu background glow effect
		{0, D3DBLENDOP_ADD, D3DBLEND_ZERO, D3DBLEND_SRCALPHA},					// 1202: (Cd - 0)*As + 0 ==> Cd*As
		{0, D3DBLENDOP_ADD, D3DBLEND_ONE, D3DBLEND_DESTALPHA},					// 1210: (Cd - 0)*Ad + Cs ==> Cs + Cd*Ad
		{1, D3DBLENDOP_ADD, D3DBLEND_ZERO, D3DBLEND_DESTALPHA},					// * 1211: (Cd - 0)*Ad + Cd ==> Cd*(1 + Ad)
		{0, D3DBLENDOP_ADD, D3DBLEND_ZERO, D3DBLEND_DESTALPHA},					// 1212: (Cd - 0)*Ad + 0 ==> Cd*Ad
		{0, D3DBLENDOP_ADD, D3DBLEND_ONE, D3DBLEND_BLENDFACTOR},				// 1220: (Cd - 0)*F + Cs ==> Cs + Cd*F
		{1, D3DBLENDOP_ADD, D3DBLEND_ZERO, D3DBLEND_BLENDFACTOR},				// * 1221: (Cd - 0)*F + Cd ==> Cd*(1 + F)
		{0, D3DBLENDOP_ADD, D3DBLEND_ZERO, D3DBLEND_BLENDFACTOR},				// 1222: (Cd - 0)*F + 0 ==> Cd*F
		{0, D3DBLENDOP_ADD, D3DBLEND_INVSRCALPHA, D3DBLEND_ZERO},				// 2000: (0 - Cs)*As + Cs ==> Cs*(1 - As)
		{0, D3DBLENDOP_REVSUBTRACT, D3DBLEND_SRCALPHA, D3DBLEND_ONE},			// 2001: (0 - Cs)*As + Cd ==> Cd - Cs*As
		{0, D3DBLENDOP_REVSUBTRACT, D3DBLEND_SRCALPHA, D3DBLEND_ZERO},			// 2002: (0 - Cs)*As + 0 ==> 0 - Cs*As
		{0, D3DBLENDOP_ADD, D3DBLEND_INVDESTALPHA, D3DBLEND_ZERO},				// 2010: (0 - Cs)*Ad + Cs ==> Cs*(1 - Ad)
		{0, D3DBLENDOP_REVSUBTRACT, D3DBLEND_DESTALPHA, D3DBLEND_ONE},			// 2011: (0 - Cs)*Ad + Cd ==> Cd - Cs*Ad
		{0, D3DBLENDOP_REVSUBTRACT, D3DBLEND_DESTALPHA, D3DBLEND_ZERO},			// 2012: (0 - Cs)*Ad + 0 ==> 0 - Cs*Ad
		{0, D3DBLENDOP_ADD, D3DBLEND_INVBLENDFACTOR, D3DBLEND_ZERO},			// 2020: (0 - Cs)*F + Cs ==> Cs*(1 - F)
		{0, D3DBLENDOP_REVSUBTRACT, D3DBLEND_BLENDFACTOR, D3DBLEND_ONE},		// 2021: (0 - Cs)*F + Cd ==> Cd - Cs*F
		{0, D3DBLENDOP_REVSUBTRACT, D3DBLEND_BLENDFACTOR, D3DBLEND_ZERO},		// 2022: (0 - Cs)*F + 0 ==> 0 - Cs*F
		{0, D3DBLENDOP_SUBTRACT, D3DBLEND_ONE, D3DBLEND_SRCALPHA},				// 2100: (0 - Cd)*As + Cs ==> Cs - Cd*As
		{0, D3DBLENDOP_ADD, D3DBLEND_ZERO, D3DBLEND_INVSRCALPHA},				// 2101: (0 - Cd)*As + Cd ==> Cd*(1 - As)
		{0, D3DBLENDOP_SUBTRACT, D3DBLEND_ZERO, D3DBLEND_SRCALPHA},				// 2102: (0 - Cd)*As + 0 ==> 0 - Cd*As
		{0, D3DBLENDOP_SUBTRACT, D3DBLEND_ONE, D3DBLEND_DESTALPHA},				// 2110: (0 - Cd)*Ad + Cs ==> Cs - Cd*Ad
		{0, D3DBLENDOP_ADD, D3DBLEND_ZERO, D3DBLEND_INVDESTALPHA},				// 2111: (0 - Cd)*Ad + Cd ==> Cd*(1 - Ad)
		{0, D3DBLENDOP_SUBTRACT, D3DBLEND_ONE, D3DBLEND_DESTALPHA},				// 2112: (0 - Cd)*Ad + 0 ==> 0 - Cd*Ad
		{0, D3DBLENDOP_SUBTRACT, D3DBLEND_ONE, D3DBLEND_BLENDFACTOR},			// 2120: (0 - Cd)*F + Cs ==> Cs - Cd*F
		{0, D3DBLENDOP_ADD, D3DBLEND_ZERO, D3DBLEND_INVBLENDFACTOR},			// 2121: (0 - Cd)*F + Cd ==> Cd*(1 - F)
		{0, D3DBLENDOP_SUBTRACT, D3DBLEND_ONE, D3DBLEND_BLENDFACTOR},			// 2122: (0 - Cd)*F + 0 ==> 0 - Cd*F
		{0, D3DBLENDOP_ADD, D3DBLEND_ONE, D3DBLEND_ZERO},						// 2200: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cs ==> Cs
		{0, D3DBLENDOP_ADD, D3DBLEND_ZERO, D3DBLEND_ONE},						// 2201: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cd ==> Cd
		{0, D3DBLENDOP_ADD, D3DBLEND_ZERO, D3DBLEND_ZERO},						// 2202: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + 0 ==> 0
		{0, D3DBLENDOP_ADD, D3DBLEND_ONE, D3DBLEND_ZERO},						// 2210: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cs ==> Cs
		{0, D3DBLENDOP_ADD, D3DBLEND_ZERO, D3DBLEND_ONE},						// 2211: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cd ==> Cd
		{0, D3DBLENDOP_ADD, D3DBLEND_ZERO, D3DBLEND_ZERO},						// 2212: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + 0 ==> 0
		{0, D3DBLENDOP_ADD, D3DBLEND_ONE, D3DBLEND_ZERO},						// 2220: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cs ==> Cs
		{0, D3DBLENDOP_ADD, D3DBLEND_ZERO, D3DBLEND_ONE},						// 2221: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cd ==> Cd
		{0, D3DBLENDOP_ADD, D3DBLEND_ZERO, D3DBLEND_ZERO},						// 2222: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + 0 ==> 0
	};

	// bogus: 0100, 0110, 0120, 0200, 0210, 0220, 1001, 1011, 1021, 1201, 1211, 1221

	int i = (((m_context->ALPHA.A&3)*3+(m_context->ALPHA.B&3))*3+(m_context->ALPHA.C&3))*3+(m_context->ALPHA.D&3);

	ASSERT(m_context->ALPHA.A != 3);
	ASSERT(m_context->ALPHA.B != 3);
	ASSERT(m_context->ALPHA.C != 3);
	ASSERT(m_context->ALPHA.D != 3);

	hr = m_dev->SetRenderState(D3DRS_BLENDOP, blendmap[i].op);
	hr = m_dev->SetRenderState(D3DRS_SRCBLEND, blendmap[i].src);
	hr = m_dev->SetRenderState(D3DRS_DESTBLEND, blendmap[i].dst);

	if(blendmap[i].bogus)
	{
		ASSERT(0);

		if(m_context->ALPHA.A == 0)
		{
			hr = m_dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		}
		else
		{
			hr = m_dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		}
	}
}


void GSRendererHW::SetupAlphaTest()
{
	HRESULT hr = m_dev->SetRenderState(D3DRS_ALPHATESTENABLE, m_context->TEST.ATE);

	if(m_context->TEST.ATE)
	{
		static const DWORD afunc[] = {D3DCMP_NEVER, D3DCMP_ALWAYS, D3DCMP_LESS, D3DCMP_LESSEQUAL, D3DCMP_EQUAL, D3DCMP_GREATEREQUAL, D3DCMP_GREATER, D3DCMP_NOTEQUAL};

		hr = m_dev->SetRenderState(D3DRS_ALPHAFUNC, afunc[m_context->TEST.ATST]);
		hr = m_dev->SetRenderState(D3DRS_ALPHAREF, (DWORD)SCALE_ALPHA(m_context->TEST.AREF));
	}
}

void GSRendererHW::SetupFrameBufferAlpha()
{
	HRESULT hr;
	
	if(m_context->FBA.FBA=0)
	{
//static __int64 s_frame = 0;
//if(s_frame != m_perfmon.GetFrame()) {printf("FBA\n"); s_frame = m_perfmon.GetFrame();}
		if(m_context->TEST.DATE)
		{
			ASSERT(0); // can't do both at the same time

			return;
		}

		if(m_context->TEST.ATE && m_context->TEST.ATST != 1 && m_context->TEST.AFAIL == 0)
		{
			// if not all pixels will reach the stencil test then clear it

//			hr = m_dev->Clear(0, NULL, D3DCLEAR_STENCIL, 0, 0, 0);
		}

		hr = m_dev->SetRenderState(D3DRS_STENCILENABLE, TRUE);
		hr = m_dev->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
		hr = m_dev->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);
		hr = m_dev->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_ZERO);
		hr = m_dev->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_ZERO);
		hr = m_dev->SetRenderState(D3DRS_STENCILREF, 2);
		hr = m_dev->SetRenderState(D3DRS_STENCILMASK, 2);
		hr = m_dev->SetRenderState(D3DRS_STENCILWRITEMASK, 2);	
	}
}

void GSRendererHW::UpdateFrameBufferAlpha(const GSTextureCache::GSRenderTarget* rt)
{
	HRESULT hr;
	
	if(m_context->FBA.FBA)
	{
		if(m_context->TEST.DATE)
		{
			ASSERT(0); // can't do both at the same time

			return;
		}

		hr = m_dev->SetTexture(0, NULL);
		hr = m_dev->SetRenderState(D3DRS_ZENABLE, FALSE);
		hr = m_dev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		hr = m_dev->SetRenderState(D3DRS_STENCILENABLE, TRUE);
		hr = m_dev->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
		hr = m_dev->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_ZERO);
		hr = m_dev->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_ZERO);
		hr = m_dev->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_ZERO);
		hr = m_dev->SetRenderState(D3DRS_STENCILREF, 2);
		hr = m_dev->SetRenderState(D3DRS_STENCILMASK, 2);
		hr = m_dev->SetRenderState(D3DRS_STENCILWRITEMASK, 2);	
		hr = m_dev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		hr = m_dev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		hr = m_dev->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		hr = m_dev->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA);

		hr = m_dev->SetVertexShader(NULL);
		hr = m_dev->SetPixelShader(m_tc.m_ps[2]);

		struct
		{
			float x, y, z, rhw;
		}
		pVertices[] =
		{
			{0, 0, 0.5f, 2.0f},
			{(float)rt->m_desc.Width, 0, 0.5f, 2.0f},
			{0, (float)rt->m_desc.Height, 0.5f, 2.0f},
			{(float)rt->m_desc.Width, (float)rt->m_desc.Height, 0.5f, 2.0f},
		};

		hr = m_dev->BeginScene();
		hr = m_dev->SetFVF(D3DFVF_XYZRHW);
		hr = m_dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pVertices, sizeof(pVertices[0]));
		hr = m_dev->EndScene();
	}

	return;

	if(m_context->FRAME.PSM == PSM_PSMCT16 || m_context->FRAME.PSM == PSM_PSMCT16S)
	{
		CComPtr<IDirect3DTexture9> texture;
		CComPtr<IDirect3DSurface9> surface;

		hr = m_tc.CreateRenderTarget(rt->m_desc.Width, rt->m_desc.Height, &texture, &surface);

		hr = m_dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		hr = m_dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		hr = m_dev->SetRenderState(D3DRS_ZENABLE, FALSE);
		hr = m_dev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		hr = m_dev->SetRenderState(D3DRS_STENCILENABLE, FALSE);
		hr = m_dev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		hr = m_dev->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		hr = m_dev->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA);

		hr = m_dev->SetVertexShader(NULL);

		struct
		{
			float x, y, z, rhw;
			float tu, tv;
		}
		pVertices[] =
		{
			{0, 0, 0.5f, 2.0f, 0, 0},
			{(float)rt->m_desc.Width, 0, 0.5f, 2.0f, 1, 0},
			{0, (float)rt->m_desc.Height, 0.5f, 2.0f, 0, 1},
			{(float)rt->m_desc.Width, (float)rt->m_desc.Height, 0.5f, 2.0f, 1, 1},
		};

		hr = m_dev->BeginScene();

		hr = m_dev->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);

		hr = m_dev->SetRenderTarget(0, surface);
		hr = m_dev->SetTexture(0, rt->m_texture);
		hr = m_dev->SetPixelShader(m_tc.m_ps[3]);

		hr = m_dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pVertices, sizeof(pVertices[0]));

		hr = m_dev->SetRenderTarget(0, rt->m_surface);
		hr = m_dev->SetTexture(0, texture);
		hr = m_dev->SetPixelShader(m_tc.m_ps[0]);

		hr = m_dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pVertices, sizeof(pVertices[0]));

		hr = m_dev->EndScene();

		m_tc.Recycle(surface);
	}
}

bool GSRendererHW::SetupHacks(D3DPRIMITIVETYPE& prim, int& count, GSTextureCache::GSTexture* tex)
{
	#pragma region ffxii pal video conversion

	if(m_crc == 0x78DA0252 || m_crc == 0xC1274668 || m_crc == 0xDC2A467E)
	{
		static DWORD* video = NULL;
		static bool ok = false;

		if(prim == D3DPT_POINTLIST && count >= 448*448 && count <= 448*512)
		{
			// incoming pixels are stored in columns, one column is 16x512, total res 448x512 or 448x454

			if(!video) video = new DWORD[512*512];

			int i = 0;

			int rows = count / 448;

			for(int x = 0; x < 448; x += 16)
			{
				DWORD* dst = &video[x];

				for(int y = 0; y < rows; y++, dst += 512)
				{
					for(int j = 0; j < 16; j++, i++)
					{
						dst[j] = m_pVertices[i].color;
					}
				}
			}

			ok = true;

			return false;
		}
		else if(prim == D3DPT_LINELIST && count == 512 && ok)
		{
			// normally, this step would copy the video onto screen with 512 texture mapped horizontal lines,
			// but we use the stored video data to create a new texture, and replace the lines with two triangles

			ok = false;

			m_tc.Recycle(tex->m_surface);

			tex->m_surface = NULL;
			tex->m_texture = NULL;

			if(SUCCEEDED(m_tc.CreateTexture(512, 512, D3DFMT_A8R8G8B8, &tex->m_texture, &tex->m_surface, &tex->m_desc)))
			{
				D3DLOCKED_RECT lr;
				
				if(SUCCEEDED(tex->m_surface->LockRect(&lr, NULL, 0)))
				{
					BYTE* bits = (BYTE*)lr.pBits;

					for(int i = 0; i < 512; i++, bits += lr.Pitch)
					{
						memcpy(bits, &video[i*512], 448*4);
					}

					tex->m_surface->UnlockRect();
				}
			}

			m_nVertices = 6;

			m_pVertices[0] = m_pVertices[0];
			m_pVertices[1] = m_pVertices[1];
			m_pVertices[2] = m_pVertices[m_nVertices - 2];
			m_pVertices[3] = m_pVertices[1];
			m_pVertices[4] = m_pVertices[2];
			m_pVertices[5] = m_pVertices[m_nVertices - 1];

			prim = D3DPT_TRIANGLELIST;
			count = 2;

			return true;
		}
	}

	#pragma endregion

	return true;
}
*/