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

// TODO: avx (256 bit regs, 8 pixels, 3-4 op instructions), DrawScanline ~50-70% of total time
// TODO: sse is a waste for 1 pixel (not that bad, sse register utilization is 90-95%)
// TODO: sprite doesn't need z/f interpolation, q could be eliminated by premultiplying s/t
// TODO: eliminate small triangles faster, usually 50% of the triangles do not output any pixel because they are so tiny
// current fillrate is about 20M tp/s, 1.2G needed, that means we can already hit 1 fps in the worst case :P
// TODO: DrawScanline => CUDA impl., input: array of [scan, dscan, index], kernel function: draw pixel at [scan + dscan * index]

#include "StdAfx.h"
#include "GSRasterizer.h"

GSRasterizer::GSRasterizer(GSState* state, int id, int threads)
	: m_state(state)
	, m_id(id)
	, m_threads(threads)
	, m_fbco(NULL)
	, m_zbco(NULL)
{
	m_cache = (DWORD*)_aligned_malloc(1024 * 1024 * sizeof(m_cache[0]), 16);
	m_pagehash = 0;
	m_pagedirty = true;

	m_slenv = (ScanlineEnvironment*)_aligned_malloc(sizeof(ScanlineEnvironment), 16);

	InvalidateTextureCache();

	// w00t :P

	#define InitDS_IIP(iFPSM, iZPSM, iZTST, iIIP) \
		m_ds[iFPSM][iZPSM][iZTST][iIIP] = &GSRasterizer::DrawScanline<iFPSM, iZPSM, iZTST, iIIP>; \

	#define InitDS_ZTST(iFPSM, iZPSM, iZTST) \
		InitDS_IIP(iFPSM, iZPSM, iZTST, 0) \
		InitDS_IIP(iFPSM, iZPSM, iZTST, 1) \

	#define InitDS_ZPSM(iFPSM, iZPSM) \
		InitDS_ZTST(iFPSM, iZPSM, 0) \
		InitDS_ZTST(iFPSM, iZPSM, 1) \
		InitDS_ZTST(iFPSM, iZPSM, 2) \
		InitDS_ZTST(iFPSM, iZPSM, 3) \

	#define InitDS_FPSM(iFPSM) \
		InitDS_ZPSM(iFPSM, 0) \
		InitDS_ZPSM(iFPSM, 1) \
		InitDS_ZPSM(iFPSM, 2) \
		InitDS_ZPSM(iFPSM, 3) \

	#define InitDS() \
		InitDS_FPSM(0) \
		InitDS_FPSM(1) \
		InitDS_FPSM(2) \
		InitDS_FPSM(3) \
		InitDS_FPSM(4) \
		InitDS_FPSM(5) \
		InitDS_FPSM(6) \
		InitDS_FPSM(7) \

	InitDS();

	InitEx();
}

GSRasterizer::~GSRasterizer()
{
	_aligned_free(m_cache);
	_aligned_free(m_slenv);

	for(int i = 0, j = m_comap.GetSize(); i < j; i++)
	{
		_aligned_free(m_comap.GetValueAt(i));
	}

	m_comap.RemoveAll();
}

void GSRasterizer::InvalidateTextureCache() 
{
	if(m_pagedirty)
	{
		memset(m_page, ~0, sizeof(m_page));
	}

	m_pagedirty = false;
}

int GSRasterizer::Draw(Vertex* vertices, int count)
{
	GSDrawingEnvironment& env = m_state->m_env;
	GSDrawingContext* context = m_state->m_context;
	GIFRegPRIM* PRIM = m_state->PRIM;

	// sanity check

	if(context->TEST.ZTE && context->TEST.ZTST == 0)
	{
		return 0;
	}

	// m_scissor

	m_scissor.left = max(context->SCISSOR.SCAX0, 0);
	m_scissor.top = max(context->SCISSOR.SCAY0, 0);
	m_scissor.right = min(context->SCISSOR.SCAX1 + 1, context->FRAME.FBW * 64);
	m_scissor.bottom = min(context->SCISSOR.SCAY1 + 1, 4096);

	// m_sel

	m_sel.dw = 0;

	m_sel.fpsm = GSLocalMemory::EncodeFPSM(context->FRAME.PSM);
	m_sel.zpsm = GSLocalMemory::EncodeZPSM(context->ZBUF.PSM);
	m_sel.ztst = context->TEST.ZTE && context->TEST.ZTST > 1 ? context->TEST.ZTST : context->ZBUF.ZMSK ? 0 : 1;
	m_sel.iip = PRIM->PRIM == GS_POINTLIST || PRIM->PRIM == GS_SPRITE ? 0 : PRIM->IIP;
	m_sel.tfx = PRIM->TME ? context->TEX0.TFX : 4;

	if(m_sel.tfx != 4)
	{
		m_sel.tcc = context->TEX0.TCC;
		m_sel.fst = PRIM->FST;
		m_sel.ltf = context->TEX1.LCM 
			? (context->TEX1.K <= 0 && (context->TEX1.MMAG & 1) || context->TEX1.K > 0 && (context->TEX1.MMIN & 1)) 
			: ((context->TEX1.MMAG & 1) | (context->TEX1.MMIN & 1));

		if(m_sel.fst == 0 && PRIM->PRIM == GS_SPRITE)
		{
			// skip per pixel division if q is constant

			m_sel.fst = 1;

			for(int i = 0; i < count; i++)
			{
				vertices[i].t /= vertices[i].t.zzzz();
			}
		}
	}

	m_sel.atst = context->TEST.ATE ? context->TEST.ATST : 1;
	m_sel.afail = context->TEST.AFAIL;
	m_sel.fge = PRIM->FGE;
	m_sel.rfb = 
		PRIM->ABE || env.PABE.PABE || 
		context->FRAME.FBMSK != 0 && context->FRAME.FBMSK != 0xffffffff || 
		context->TEST.ATE && context->TEST.ATST != 1 && context->TEST.AFAIL == 3;
	m_sel.date = context->FRAME.PSM != PSM_PSMCT24 ? context->TEST.DATE : 0;
	m_sel.abe = env.PABE.PABE ? 2 : PRIM->ABE ? 1 : 0;
	m_sel.abea = m_sel.abe ? context->ALPHA.A : 0;
	m_sel.abeb = m_sel.abe ? context->ALPHA.B : 0;
	m_sel.abec = m_sel.abe ? context->ALPHA.C : 0;
	m_sel.abed = m_sel.abe ? context->ALPHA.D : 0;

	m_dsf = m_ds[m_sel.fpsm][m_sel.zpsm][m_sel.ztst][m_sel.iip];

	CAtlMap<DWORD, DrawScanlinePtr>::CPair* pair = m_dsmap2.Lookup(m_sel);

	if(pair)
	{
		m_dsf = pair->m_value;
	}
	else
	{
		pair = m_dsmap.Lookup(m_sel);

		if(pair && pair->m_value)
		{
			m_dsf = pair->m_value;

			m_dsmap2[pair->m_key] = pair->m_value;
		}
		else if(!pair)
		{
			_tprintf(_T("*** [%d] fpsm %d zpsm %d ztst %d tfx %d tcc %d fst %d ltf %d atst %d afail %d fge %d rfb %d date %d abe %d\n"), 
				m_dsmap.GetCount(), 
				m_sel.fpsm, m_sel.zpsm, m_sel.ztst, 
				m_sel.tfx, m_sel.tcc, m_sel.fst, m_sel.ltf, 
				m_sel.atst, m_sel.afail, m_sel.fge, m_sel.rfb, m_sel.date, m_sel.abe);

			m_dsmap[m_sel] = NULL;

			if(FILE* fp = _tfopen(_T("c:\\1.txt"), _T("w")))
			{
				POSITION pos = m_dsmap.GetStartPosition();

				while(pos) 
				{
					pair = m_dsmap.GetNext(pos);

					if(!pair->m_value)
					{
						_ftprintf(fp, _T("m_dsmap[0x%08x] = &GSRasterizer::DrawScanlineEx<0x%08x>;\n"), pair->m_key, pair->m_key);
					}
				}

				fclose(fp);
			}
		}
	}

	// m_slenv

	ScanlineEnvironment* slenv = m_slenv;

	slenv->fo = m_state->m_context->ftbl->rowOffset;
	slenv->zo = m_state->m_context->ztbl->rowOffset;
	slenv->fm = GSVector4i(context->FRAME.FBMSK);
	slenv->zm = GSVector4i(context->ZBUF.ZMSK ? 0xffffffff : 0);
	slenv->datm = GSVector4i(context->TEST.DATM ? 0x80000000 : 0);
	slenv->colclamp = GSVector4i(env.COLCLAMP.CLAMP ? 0xffffffff : 0x00ff00ff);
	slenv->fba = GSVector4i(context->FBA.FBA ? 0x80000000 : 0);
	slenv->aref = GSVector4i((int)context->TEST.AREF + (m_sel.atst == 2 ? -1 : m_sel.atst == 6 ? +1 : 0));
	slenv->afix = GSVector4((float)(int)context->ALPHA.FIX);
	slenv->f.r = GSVector4((float)(int)env.FOGCOL.FCR);
	slenv->f.g = GSVector4((float)(int)env.FOGCOL.FCG);
	slenv->f.b = GSVector4((float)(int)env.FOGCOL.FCB);

	if(PRIM->TME)
	{
		DWORD pagehash = context->TEX0.ai32[0];// ^ context->TEX0.ai32[1] ^ env.TEXA.ai32[0] ^ env.TEXA.ai32[1] ^ env.TEXCLUT.ai32[0];

		if(m_pagehash != pagehash)
		{
//			InvalidateTextureCache();

			m_pagehash = pagehash;
		}

		short tw = (short)(1 << context->TEX0.TW);
		short th = (short)(1 << context->TEX0.TH);

		switch(context->CLAMP.WMS)
		{
		case 0: 
			slenv->t.min.u16[0] = tw - 1;
			slenv->t.max.u16[0] = 0;
			slenv->t.mask.u32[0] = 0xffffffff; 
			break;
		case 1: 
			slenv->t.min.u16[0] = 0;
			slenv->t.max.u16[0] = tw - 1;
			slenv->t.mask.u32[0] = 0; 
			break;
		case 2: 
			slenv->t.min.u16[0] = context->CLAMP.MINU;
			slenv->t.max.u16[0] = context->CLAMP.MAXU;
			slenv->t.mask.u32[0] = 0; 
			break;
		case 3: 
			slenv->t.min.u16[0] = context->CLAMP.MINU;
			slenv->t.max.u16[0] = context->CLAMP.MAXU;
			slenv->t.mask.u32[0] = 0xffffffff; 
			break;
		default: 
			__assume(0);
		}

		switch(context->CLAMP.WMT)
		{
		case 0: 
			slenv->t.min.u16[4] = th - 1;
			slenv->t.max.u16[4] = 0;
			slenv->t.mask.u32[2] = 0xffffffff; 
			break;
		case 1: 
			slenv->t.min.u16[4] = 0;
			slenv->t.max.u16[4] = th - 1;
			slenv->t.mask.u32[2] = 0; 
			break;
		case 2: 
			slenv->t.min.u16[4] = context->CLAMP.MINV;
			slenv->t.max.u16[4] = context->CLAMP.MAXV;
			slenv->t.mask.u32[2] = 0; 
			break;
		case 3: 
			slenv->t.min.u16[4] = context->CLAMP.MINV;
			slenv->t.max.u16[4] = context->CLAMP.MAXV;
			slenv->t.mask.u32[2] = 0xffffffff; 
			break;
		default: 
			__assume(0);
		}

		slenv->t.min = slenv->t.min.xxxxl().xxxxh();
		slenv->t.max = slenv->t.max.xxxxl().xxxxh();
		slenv->t.mask = slenv->t.mask.xxzz();
	}

	//

	SetupColumnOffset();

	//

	m_solidrect = true;

	if(m_state->PRIM->IIP || m_state->PRIM->TME 
	|| m_state->PRIM->ABE || m_state->PRIM->FGE
	|| context->TEST.ZTE && context->TEST.ZTST != 1 
	|| context->TEST.ATE && context->TEST.ATST != 1
	|| context->TEST.DATE
	|| env.DTHE.DTHE
	|| context->FRAME.FBMSK)
	{
		m_solidrect = false;
	}

	//

	switch(PRIM->PRIM)
	{
	case GS_POINTLIST:
		for(int i = 0; i < count; i++, vertices++) DrawPoint(vertices);
		break;
	case GS_LINELIST: 
	case GS_LINESTRIP: 
		ASSERT(!(count & 1));
		count = count / 2;
		for(int i = 0; i < count; i++, vertices += 2) DrawLine(vertices);
		break;
	case GS_TRIANGLELIST: 
	case GS_TRIANGLESTRIP: 
	case GS_TRIANGLEFAN:
		ASSERT(!(count % 3));
		count = count / 3;
		for(int i = 0; i < count; i++, vertices += 3) DrawTriangle(vertices);
		break;
	case GS_SPRITE:
		ASSERT(!(count & 3));
		count = count / 4;
		for(int i = 0; i < count; i++, vertices += 4) DrawSprite(vertices);
		break;
	default:
		__assume(0);
	}

	return count;
}

void GSRasterizer::DrawPoint(Vertex* v)
{
	// TODO: round to closest for point, prestep for line

	GSVector4i p(v->p);

	if(m_scissor.left <= p.x && p.x < m_scissor.right && m_scissor.top <= p.y && p.y < m_scissor.bottom)
	{
		if((p.y % m_threads) == m_id) 
		{
			(this->*m_dsf)(p.y, p.x, p.x + 1, *v);
		}
	}
}

void GSRasterizer::DrawLine(Vertex* v)
{
	Vertex dv = v[1] - v[0];

	GSVector4 dp = dv.p.abs();
	GSVector4i dpi(dp);

	if(dpi.x == 0 && dpi.y == 0) return;

	int i = dpi.x > dpi.y ? 0 : 1;

	Vertex edge = v[0];
	Vertex dedge = dv / dp.v[i];

	// TODO: prestep + clip with the scissor

	int steps = dpi.v[i];

	while(steps-- > 0)
	{
		DrawPoint(&edge);

		edge += dedge;
	}
}

static const int s_abc[8][4] = 
{
	{0, 1, 2, 0},
	{1, 0, 2, 0},
	{0, 0, 0, 0},
	{1, 2, 0, 0},
	{0, 2, 1, 0},
	{0, 0, 0, 0},
	{2, 0, 1, 0},
	{2, 1, 0, 0},
};

void GSRasterizer::DrawTriangle(Vertex* vertices)
{
	Vertex v[4];

	GSVector4 aabb = vertices[0].p.yyyy(vertices[1].p);
	GSVector4 bccb = vertices[1].p.yyyy(vertices[2].p).xzzx();

	int i = _mm_movemask_ps(bccb < aabb) & 7;

	v[0] = vertices[s_abc[i][0]];
	v[1] = vertices[s_abc[i][1]];
	v[3] = vertices[s_abc[i][2]];

	if(v[0].p.y >= v[3].p.y) return;

	Vertex v01, v03, v12, v13; 

	v03 = v[3] - v[0];
	v01.p = v[1].p - v[0].p;

	v[2] = v[0] + v03 * (v01.p / v03.p).yyyy();

	v12.p = v[2].p - v[1].p;

	if(v12.p.x == 0) return;

	v12.c = v[2].c - v[1].c;
	v12.t = v[2].t - v[1].t;

	Vertex dscan = v12 / v12.p.xxxx();

	SetupScanline<true, true, true>(dscan);

	Vertex dl, dr;

	if(v12.p.x < 0) 
	{
		dl = v03 / v03.p.yyyy();
		dr.p = v01.p / v01.p.yyyy();

		GSVector4 p0 = v[0].p;

		DrawTriangleSection(v[0], dl, p0, dr.p, v[1].p, dscan);

		v13.p = v[3].p - v[1].p;

		dr.p = v13.p / v13.p.yyyy();

		DrawTriangleSection(v[2], dl, v[1].p, dr.p, v[3].p, dscan);
	}
	else
	{
		v01.t = v[1].t - v[0].t;
		v01.c = v[1].c - v[0].c;

		dl = v01 / v01.p.yyyy();
		dr.p = v03.p / v03.p.yyyy();

		GSVector4 p0 = v[0].p;

		DrawTriangleSection(v[0], dl, p0, dr.p, v[1].p, dscan);
		
		v13 = v[3] - v[1];

		dl = v13 / v13.p.yyyy();

		DrawTriangleSection(v[1], dl, v[2].p, dr.p, v[3].p, dscan);
	}
}

void GSRasterizer::DrawTriangleSection(Vertex& l, const Vertex& dl, GSVector4& r, const GSVector4& dr, const GSVector4& b, const Vertex& dscan)
{
	GSVector4i tb(l.p.upl(b).ceil());

	int top = tb.z;
	int bottom = tb.w;

	if(top < m_scissor.top) top = min(m_scissor.top, bottom);
	if(bottom > m_scissor.bottom) bottom = m_scissor.bottom;
	
	if(top < bottom)
	{
		float py = (float)top - l.p.y;

		if(py > 0)
		{
			GSVector4 dy(py);

			l += dl * dy;
			r += dr * dy;
		}

		for(; top < bottom; top++)
		{
			if((top % m_threads) == m_id) 
			{
				GSVector4i lr(l.p.upl(r).ceil());

				int left = lr.x;
				int right = lr.y;

				if(left < m_scissor.left) left = m_scissor.left;
				if(right > m_scissor.right) right = m_scissor.right;

				if(right > left)
				{
					Vertex scan = l;

					float px = (float)left - l.p.x;

					if(px > 0)
					{
						scan += dscan * px;
					}

					(this->*m_dsf)(top, left, right, scan);
				}
			}

			l += dl;
			r += dr;
		}
	}
}

void GSRasterizer::DrawSprite(Vertex* vertices)
{
	Vertex v[4];
	
	int a = 0, b = 1, c = 2, d = 3;

	if(vertices[c].p.y < vertices[a].p.y) {int i = a; a = c; c = i; i = b; b = d; d = i;}
	if(vertices[b].p.x < vertices[a].p.x) {int i = a; a = b; b = i; i = c; c = d; c = i;}

	v[0] = vertices[a];
	v[1] = vertices[b];
	v[2] = vertices[c];
	v[3] = vertices[d];

	__m128d tb = _mm_castps_pd(v[0].p.upl(v[2].p));
	__m128d lr = _mm_castps_pd(v[0].p.upl(v[1].p));
	
	GSVector4i tblr(GSVector4(_mm_castpd_ps(_mm_shuffle_pd(tb, lr, _MM_SHUFFLE2(0, 1)))).ceil());

	int top = tblr.x;
	int bottom = tblr.y;

	if(top < m_scissor.top) top = m_scissor.top;
	if(bottom > m_scissor.bottom) bottom = m_scissor.bottom;
	if(top >= bottom) return;

	int left = tblr.z;
	int right = tblr.w;

	if(left < m_scissor.left) left = m_scissor.left;
	if(right > m_scissor.right) right = m_scissor.right;
	if(left >= right) return;

	Vertex scan = v[0];

	if(DrawSolidRect(left, top, right, bottom, scan))
	{
		return;
	}

	Vertex dedge, dscan;

	dedge.p = GSVector4::zero();
	dscan.p = GSVector4::zero();

	if(m_sel.tfx < 4)
	{
		dedge.t = (v[2].t - v[0].t) / (v[2].p - v[0].p).yyyy();
		dscan.t = (v[1].t - v[0].t) / (v[1].p - v[0].p).xxxx();

		if(scan.p.y < (float)top) scan.t += dedge.t * ((float)top - scan.p.y);
		if(scan.p.x < (float)left) scan.t += dscan.t * ((float)left - scan.p.x);

		SetupScanline<true, true, false>(dscan);

		for(; top < bottom; top++, scan.t += dedge.t)
		{
			if((top % m_threads) == m_id) 
			{
				(this->*m_dsf)(top, left, right, scan);
			}
		}
	}
	else
	{
		SetupScanline<true, false, false>(dscan);

		for(; top < bottom; top++)
		{
			if((top % m_threads) == m_id) 
			{
				(this->*m_dsf)(top, left, right, scan);
			}
		}
	}
}

bool GSRasterizer::DrawSolidRect(int left, int top, int right, int bottom, const Vertex& v)
{
	if(left >= right || top >= bottom || !m_solidrect)
	{
		return false;
	}

	if(m_id != 0)
	{
		return true;
	}

	ASSERT(top >= 0);
	ASSERT(bottom >= 0);

	CRect r(left, top, right, bottom);

	GSDrawingContext* context = m_state->m_context;

	DWORD fbp = context->FRAME.Block();
	DWORD fpsm = context->FRAME.PSM;
	DWORD zbp = context->ZBUF.Block();
	DWORD zpsm = context->ZBUF.PSM;
	DWORD bw = context->FRAME.FBW;

	if(!context->ZBUF.ZMSK)
	{
		m_state->m_mem.FillRect(r, (DWORD)(float)v.p.z, zpsm, zbp, bw);
	}

	DWORD c = v.c.rgba32();

	if(context->FBA.FBA)
	{
		c |= 0x80000000;
	}
	
	if(fpsm == PSM_PSMCT16 || fpsm == PSM_PSMCT16S)
	{
		c = ((c & 0xf8) >> 3) | ((c & 0xf800) >> 6) | ((c & 0xf80000) >> 9) | ((c & 0x80000000) >> 16);
	}

	m_state->m_mem.FillRect(r, c, fpsm, fbp, bw);

	return true;
}

void GSRasterizer::FetchTexture(int x, int y)
{
	const int xs = 1 << TEXTURE_CACHE_WIDTH;
	const int ys = 1 << TEXTURE_CACHE_HEIGHT;

	x &= ~(xs - 1);
	y &= ~(ys - 1);

	CRect r(x, y, x + xs, y + ys);

	DWORD* dst = &m_cache[y * 1024 + x];

	(m_state->m_mem.*m_state->m_context->ttbl->ust)(r, (BYTE*)dst, 1024 * 4, m_state->m_context->TEX0, m_state->m_env.TEXA);

	m_state->m_perfmon.Put(GSPerfMon::Unswizzle, r.Width() * r.Height() * 4);
}

void GSRasterizer::SetupColumnOffset()
{
	GSDrawingContext* context = m_state->m_context;

	if(context->FRAME.FBW == 0) return;

	// fb

	DWORD hash = context->FRAME.FBP | (context->FRAME.FBW << 9) | (context->FRAME.PSM << 15);

	if(!m_fbco || m_fbco->hash != hash)
	{
		ColumnOffset* fbco = m_comap.Lookup(hash);

		if(!fbco)
		{
			fbco = (ColumnOffset*)_aligned_malloc(sizeof(ColumnOffset), 16);

			fbco->hash = hash;

			for(int i = 0, j = 1024; i < j; i++)
			{
				fbco->addr[i] = GSVector4i((int)context->ftbl->pa(0, i, context->FRAME.Block(), context->FRAME.FBW));
			}

			m_comap.Add(hash, fbco);
		}

		m_fbco = fbco;
	}

	// zb

	hash = context->ZBUF.ZBP | (context->FRAME.FBW << 9) | (context->ZBUF.PSM << 15);

	if(!m_zbco || m_zbco->hash != hash)
	{
		ColumnOffset* zbco = m_comap.Lookup(hash);

		if(!zbco)
		{
			zbco = (ColumnOffset*)_aligned_malloc(sizeof(ColumnOffset), 16);

			zbco->hash = hash;

			for(int i = 0, j = 1024; i < j; i++)
			{
				zbco->addr[i] = GSVector4i((int)context->ztbl->pa(0, i, context->ZBUF.Block(), context->FRAME.FBW));
			}

			m_comap.Add(hash, zbco);
		}

		m_zbco = zbco;
	}
}

template<bool pos, bool tex, bool col> 
void GSRasterizer::SetupScanline(const Vertex& dv)
{
	ScanlineEnvironment* slenv = m_slenv;

	if(pos)
	{
		GSVector4 dp = dv.p;

		slenv->dz0123 = dp.zzzz() * GSVector4(0, 1, 2, 3);	
		slenv->df0123 = dp.wwww() * GSVector4(0, 1, 2, 3); 

		GSVector4 dp4 = dp * 4.0f;

		slenv->dz = dp4.zzzz();
		slenv->df = dp4.wwww();
	}

	if(tex)
	{
		GSVector4 dt = dv.t;

		slenv->ds0123 = dt.xxxx() * GSVector4(0, 1, 2, 3); 
		slenv->dt0123 = dt.yyyy() * GSVector4(0, 1, 2, 3); 
		slenv->dq0123 = dt.zzzz() * GSVector4(0, 1, 2, 3); 

		GSVector4 dt4 = dt * 4.0f;

		slenv->ds = dt4.xxxx();
		slenv->dt = dt4.yyyy();
		slenv->dq = dt4.zzzz();
	}

	if(col)
	{
		GSVector4 dc = dv.c;

		slenv->dr0123 = dc.xxxx() * GSVector4(0, 1, 2, 3); 
		slenv->dg0123 = dc.yyyy() * GSVector4(0, 1, 2, 3); 
		slenv->db0123 = dc.zzzz() * GSVector4(0, 1, 2, 3); 
		slenv->da0123 = dc.wwww() * GSVector4(0, 1, 2, 3); 

		GSVector4 dc4 = dc * 4.0f;

		slenv->dr = dc4.xxxx();
		slenv->dg = dc4.yyyy();
		slenv->db = dc4.zzzz();
		slenv->da = dc4.wwww();
	}
}

template<int iFPSM, int iZPSM, int ztst, int iip>
void GSRasterizer::DrawScanline(int top, int left, int right, const Vertex& v)	
{
/*
extern UINT64 g_slp1;
extern UINT64 g_slp2;
extern UINT64 g_slp3;
extern UINT64 g_slp4;
{
int steps = right - left;
for(; steps >= 4; steps -= 4) g_slp4++;
if(steps == 1) g_slp1++;
else if(steps == 2) g_slp2++;
else if(steps == 3) g_slp3++;
}
*/
	ScanlineEnvironment* slenv = m_slenv;

	int fpsm = GSLocalMemory::DecodeFPSM(iFPSM);
	int zpsm = GSLocalMemory::DecodeZPSM(iZPSM);

	GSVector4i fa_base = m_fbco->addr[top];
	GSVector4i* fa_offset = (GSVector4i*)&slenv->fo[top & 7][left];

	GSVector4i za_base = m_zbco->addr[top];
	GSVector4i* za_offset = (GSVector4i*)&slenv->zo[top & 7][left];

	GSVector4 vp = v.p;
	GSVector4 z = vp.zzzz(); z += slenv->dz0123;
	GSVector4 f = vp.wwww(); f += slenv->df0123;

	GSVector4 vt = v.t;
	GSVector4 s = vt.xxxx(); s += slenv->ds0123;
	GSVector4 t = vt.yyyy(); t += slenv->dt0123;
	GSVector4 q = vt.zzzz(); q += slenv->dq0123;

	GSVector4 vc = v.c;
	GSVector4 r = vc.xxxx(); if(iip) r += slenv->dr0123;
	GSVector4 g = vc.yyyy(); if(iip) g += slenv->dg0123;
	GSVector4 b = vc.zzzz(); if(iip) b += slenv->db0123;
	GSVector4 a = vc.wwww(); if(iip) a += slenv->da0123;

	for(int steps = right - left; steps > 0; steps -= 4)
	{
		do
		{

		int pixels = min(steps, 4);

		GSVector4i fa = fa_base + GSVector4i::loadu(fa_offset);
		GSVector4i za = za_base + GSVector4i::loadu(za_offset);
		
		GSVector4i fm = slenv->fm;
		GSVector4i zm = slenv->zm;
		GSVector4i test = GSVector4i::zero();

		GSVector4i zi = (GSVector4i(z * 0.5f) << 1) | (GSVector4i(z) & 1);

		if(ztst > 1)
		{
			GSVector4i zd0123;

			for(int i = 0; i < pixels; i++)
			{
				zd0123.u32[i] = m_state->m_mem.readPixelX(zpsm, za.u32[i]);
			}

			GSVector4i zs = zi - 0x80000000;
			GSVector4i zd = zd0123 - 0x80000000;

			switch(ztst)
			{
			case 2: test = zs < zd; break; // ge
			case 3: test = zs <= zd; break; // g
			default: __assume(0);
			}

			if(_mm_movemask_epi8(test) == 0xffff)
			{
				continue;
			}
		}

		GSVector4 c[12];

		if(m_sel.tfx < 4)
		{
			GSVector4 u = s;
			GSVector4 v = t;

			if(!m_sel.fst)
			{
				GSVector4 w = q.rcp();

				u *= w;
				v *= w;
			}

			if(m_sel.ltf)
			{
				u -= 0.5f;
				v -= 0.5f;

				GSVector4 uf = u.floor();
				GSVector4 vf = v.floor();
				
				GSVector4 uff = u - uf;
				GSVector4 vff = v - vf;

				GSVector4i uv = GSVector4i(uf).ps32(GSVector4i(vf));

				GSVector4i uv0 = Wrap(uv);
				GSVector4i uv1 = Wrap(uv + 0x00010001);

				for(int i = 0; i < pixels; i++)
				{
					if(ztst > 1 && test.u32[i])
					{
						continue;
					}

					FetchTexel(uv0.u16[i], uv0.u16[i + 4]);
					FetchTexel(uv1.u16[i], uv0.u16[i + 4]);
					FetchTexel(uv0.u16[i], uv1.u16[i + 4]);
					FetchTexel(uv1.u16[i], uv1.u16[i + 4]);

					GSVector4 c00(ReadTexelNoFetch(uv0.u16[i], uv0.u16[i + 4]));
					GSVector4 c01(ReadTexelNoFetch(uv1.u16[i], uv0.u16[i + 4]));
					GSVector4 c10(ReadTexelNoFetch(uv0.u16[i], uv1.u16[i + 4]));
					GSVector4 c11(ReadTexelNoFetch(uv1.u16[i], uv1.u16[i + 4]));

					c00 = c00.lerp(c01, uff.v[i]);
					c10 = c10.lerp(c11, uff.v[i]);
					c00 = c00.lerp(c10, vff.v[i]);

					c[i] = c00;
				}

				GSVector4::transpose(c[0], c[1], c[2], c[3]);
			}
			else
			{
				GSVector4i uv = Wrap(GSVector4i(u).ps32(GSVector4i(v)));

				GSVector4i c00;

				for(int i = 0; i < pixels; i++)
				{
					if(ztst > 1 && test.u32[i])
					{
						continue;
					}

					c00.u32[i] = ReadTexel(uv.u16[i], uv.u16[i + 4]);
				}

				GSVector4::expand(c00, c[0], c[1], c[2], c[3]);
			}
		}

		switch(m_sel.tfx)
		{
		case 0: c[3] = m_sel.tcc ? c[3].mod2x(a).sat() : a; break;
		case 1: break;
		case 2: c[3] = m_sel.tcc ? (c[3] + a).sat() : a; break;
		case 3: if(!m_sel.tcc) c[3] = a; break;
		case 4: c[3] = a; break; 
		default: __assume(0);
		}

		if(m_sel.atst != 1)
		{
			GSVector4i t;

			switch(m_sel.atst)
			{
			case 0: t = GSVector4i::invzero(); break; // never 
			case 1: t = GSVector4i::zero(); break; // always
			case 2: case 3: t = GSVector4i(c[3]) > slenv->aref; break; // l, le
			case 4: t = GSVector4i(c[3]) != slenv->aref; break; // e
			case 5: case 6: t = GSVector4i(c[3]) < slenv->aref; break; // ge, g
			case 7: t = GSVector4i(c[3]) == slenv->aref; break; // ne 
			default: __assume(0);
			}

			switch(m_sel.afail)
			{
			case 0:
				fm |= t;
				zm |= t;
				test |= t;
				if(_mm_movemask_epi8(test) == 0xffff) continue;
				break;
			case 1:
				zm |= t;
				break;
			case 2:
				fm |= t;
				break;
			case 3: 
				fm |= t & 0xff000000;
				zm |= t;
				break;
			default: 
				__assume(0);
			}
		}

		switch(m_sel.tfx)
		{
		case 0:
			c[0] = c[0].mod2x(r);
			c[1] = c[1].mod2x(g);
			c[2] = c[2].mod2x(b);
			c[0] = c[0].sat();
			c[1] = c[1].sat();
			c[2] = c[2].sat();
			break;
		case 1:
			break;
		case 2:
		case 3:
			c[0] = c[0].mod2x(r) + a;
			c[1] = c[1].mod2x(g) + a;
			c[2] = c[2].mod2x(b) + a;
			c[0] = c[0].sat();
			c[1] = c[1].sat();
			c[2] = c[2].sat();
			break;
		case 4:
			c[0] = r;
			c[1] = g;
			c[2] = b;
			break;
		default:
			__assume(0);
		}

		if(m_sel.fge)
		{
			c[0] = slenv->f.r.lerp(c[0], f);
			c[1] = slenv->f.g.lerp(c[1], f);
			c[2] = slenv->f.b.lerp(c[2], f);
		}

		GSVector4i d = GSVector4i::zero();

		if(m_sel.rfb)
		{
			d = m_state->m_mem.readFrameX(fpsm, fa);

			if(m_sel.date)
			{
				test |= (d ^ slenv->datm).sra32(31);

				if(_mm_movemask_epi8(test) == 0xffff)
				{
					continue;
				}
			}
		}

		fm |= test;
		zm |= test;

		if(m_sel.abe)
		{
			GSVector4::expand(d, c[4], c[5], c[6], c[7]);

			c[8] = GSVector4::zero();
			c[9] = GSVector4::zero();
			c[10] = GSVector4::zero();
			c[11] = slenv->afix;

			int abea = m_sel.abea;
			int abeb = m_sel.abeb;
			int abec = m_sel.abec;
			int abed = m_sel.abed;

			GSVector4 r = (c[abea*4 + 0] - c[abeb*4 + 0]).mod2x(c[abec*4 + 3]) + c[abed*4 + 0];
			GSVector4 g = (c[abea*4 + 1] - c[abeb*4 + 1]).mod2x(c[abec*4 + 3]) + c[abed*4 + 1];
			GSVector4 b = (c[abea*4 + 2] - c[abeb*4 + 2]).mod2x(c[abec*4 + 3]) + c[abed*4 + 2];

			if(m_sel.abe == 2)
			{
				GSVector4 mask = c[3] >= GSVector4(128.0f);

				c[0] = c[0].blend8(r, mask);
				c[1] = c[1].blend8(g, mask);
				c[2] = c[2].blend8(b, mask);
			}
			else
			{
				c[0] = r;
				c[1] = g;
				c[2] = b;
			}
		}

		GSVector4i rb = GSVector4i(c[0]).ps32(GSVector4i(c[2]));
		GSVector4i ga = GSVector4i(c[1]).ps32(GSVector4i(c[3]));
		
		GSVector4i rg = rb.upl16(ga) & slenv->colclamp;
		GSVector4i ba = rb.uph16(ga) & slenv->colclamp;
		
		GSVector4i s = rg.upl32(ba).pu16(rg.uph32(ba)) | slenv->fba;

		if(m_sel.rfb)
		{
			s = s.blend(d, fm);
		}

		m_state->m_mem.writeFrameX(fpsm, fa, s, fm, pixels);

		for(int i = 0; ztst > 0 && i < pixels; i++)
		{
			if(zm.u32[i] != 0xffffffff)
			{
				m_state->m_mem.writePixelX(zpsm, za.u32[i], zi.u32[i]);
			}
		}

		}
		while(0);

		fa_offset++;
		za_offset++;
		z += slenv->dz;
		f += slenv->df;
		s += slenv->ds;
		t += slenv->dt;
		q += slenv->dq;
		if(iip) r += slenv->dr;
		if(iip) g += slenv->dg;
		if(iip) b += slenv->db;
		if(iip) a += slenv->da;
	}
}

//

GSRasterizerMT::GSRasterizerMT(GSState* state, int id, int threads, long* sync)
	: GSRasterizer(state, id, threads)
	, m_vertices(NULL)
	, m_count(0)
	, m_sync(sync)
	, m_exit(false)
	, m_ThreadId(0)
	, m_hThread(NULL)
{
	m_hThread = CreateThread(NULL, 0, StaticThreadProc, (LPVOID)this, 0, &m_ThreadId);
}

GSRasterizerMT::~GSRasterizerMT()
{
	if(m_hThread != NULL)
	{
		m_exit = true;

		if(WaitForSingleObject(m_hThread, 5000) != WAIT_OBJECT_0)
		{
			TerminateThread(m_hThread, 1);
		}

		CloseHandle(m_hThread);
	}
}

void GSRasterizerMT::BeginDraw(Vertex* vertices, int count)
{
	m_vertices = vertices;
	m_count = count;

	InterlockedBitTestAndSet(m_sync, m_id);
}

DWORD WINAPI GSRasterizerMT::StaticThreadProc(LPVOID lpParam)
{
	return ((GSRasterizerMT*)lpParam)->ThreadProc();
}

DWORD GSRasterizerMT::ThreadProc()
{
	// _mm_setcsr(MXCSR);

	while(!m_exit)
	{
		if(*m_sync & (1 << m_id))
		{
			Draw(m_vertices, m_count);

			InterlockedBitTestAndReset(m_sync, m_id);
		}
		else
		{
			_mm_pause();
		}
	}

	return 0;
}

