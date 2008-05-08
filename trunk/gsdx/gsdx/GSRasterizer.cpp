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
// TODO: sse is a waste for 1 pixel
// TODO: +/-1 aref when atst is gt/lt => remove extra eq test
// TODO: sprite doesn't need z/f interpolation, q could be eliminated by premultiplying s/t

#include "StdAfx.h"
#include "GSRasterizer.h"

GSRasterizer::GSRasterizer(GSState* state)
	: m_state(state)
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
	}

	m_sel.atst = context->TEST.ATE ? context->TEST.ATST : 1;
	m_sel.afail = context->TEST.AFAIL;
	m_sel.fge = PRIM->FGE;
	m_sel.rfb = m_state->PRIM->ABE || m_state->m_env.PABE.PABE || m_state->m_context->FRAME.FBMSK || m_state->m_context->TEST.ATE && m_state->m_context->TEST.ATST != 1 && m_state->m_context->TEST.AFAIL == 3;
	m_sel.date = context->TEST.DATE;
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

	slenv->fm = _mm_set1_epi32(context->FRAME.FBMSK);
	slenv->zm = _mm_set1_epi32(context->ZBUF.ZMSK ? 0xffffffff : 0);
	slenv->datm = _mm_set1_epi32(context->TEST.DATM ? 0x80000000 : 0);
	slenv->colclamp = _mm_set1_epi32(env.COLCLAMP.CLAMP ? 0xffffffff : 0x00ff00ff);
	slenv->fba = _mm_set1_epi32(context->FBA.FBA ? 0x80000000 : 0);
	slenv->aref = _mm_set1_epi32((int)context->TEST.AREF + (m_sel.atst == 2 ? -1 : m_sel.atst == 6 ? +1 : 0));
	slenv->afix = GSVector4((float)(int)context->ALPHA.FIX);
	slenv->f.r = GSVector4((float)(int)env.FOGCOL.FCR);
	slenv->f.g = GSVector4((float)(int)env.FOGCOL.FCG);
	slenv->f.b = GSVector4((float)(int)env.FOGCOL.FCB);

	if(PRIM->TME)
	{
		if(m_pagehash != context->TEX0.ai32[0])
		{
			// memset(m_page, ~0, sizeof(m_page));

			m_pagehash = context->TEX0.ai32[0];
		}

		short tw = (short)(1 << context->TEX0.TW);
		short th = (short)(1 << context->TEX0.TH);

		switch(context->CLAMP.WMS)
		{
		case 0: 
			slenv->t.min.m128i_u16[0] = tw - 1;
			slenv->t.max.m128i_u16[0] = 0;
			slenv->t.mask.m128i_u32[0] = 0xffffffff; 
			break;
		case 1: 
			slenv->t.min.m128i_u16[0] = 0;
			slenv->t.max.m128i_u16[0] = tw - 1;
			slenv->t.mask.m128i_u32[0] = 0; 
			break;
		case 2: 
			slenv->t.min.m128i_u16[0] = context->CLAMP.MINU;
			slenv->t.max.m128i_u16[0] = context->CLAMP.MAXU;
			slenv->t.mask.m128i_u32[0] = 0; 
			break;
		case 3: 
			slenv->t.min.m128i_u16[0] = context->CLAMP.MINU;
			slenv->t.max.m128i_u16[0] = context->CLAMP.MAXU;
			slenv->t.mask.m128i_u32[0] = 0xffffffff; 
			break;
		default: 
			__assume(0);
		}

		switch(context->CLAMP.WMT)
		{
		case 0: 
			slenv->t.min.m128i_u16[4] = th - 1;
			slenv->t.max.m128i_u16[4] = 0;
			slenv->t.mask.m128i_u32[2] = 0xffffffff; 
			break;
		case 1: 
			slenv->t.min.m128i_u16[4] = 0;
			slenv->t.max.m128i_u16[4] = th - 1;
			slenv->t.mask.m128i_u32[2] = 0; 
			break;
		case 2: 
			slenv->t.min.m128i_u16[4] = context->CLAMP.MINV;
			slenv->t.max.m128i_u16[4] = context->CLAMP.MAXV;
			slenv->t.mask.m128i_u32[2] = 0; 
			break;
		case 3: 
			slenv->t.min.m128i_u16[4] = context->CLAMP.MINV;
			slenv->t.max.m128i_u16[4] = context->CLAMP.MAXV;
			slenv->t.mask.m128i_u32[2] = 0xffffffff; 
			break;
		default: 
			__assume(0);
		}

		slenv->t.min = _mm_shufflelo_epi16(slenv->t.min, _MM_SHUFFLE(0, 0, 0, 0));
		slenv->t.min = _mm_shufflehi_epi16(slenv->t.min, _MM_SHUFFLE(0, 0, 0, 0));
		slenv->t.max = _mm_shufflelo_epi16(slenv->t.max, _MM_SHUFFLE(0, 0, 0, 0));
		slenv->t.max = _mm_shufflehi_epi16(slenv->t.max, _MM_SHUFFLE(0, 0, 0, 0));
		slenv->t.mask = _mm_shuffle_epi32(slenv->t.mask, _MM_SHUFFLE(2, 2, 0, 0));
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
	// TODO: prestep

	__m128i p = v->p;

	int x = p.m128i_i32[0];
	int y = p.m128i_i32[1];

	if(m_scissor.left <= x && x < m_scissor.right && m_scissor.top <= y && y < m_scissor.bottom)
	{
		(this->*m_dsf)(y, x, x + 1, *v);
	}
}

void GSRasterizer::DrawLine(Vertex* v)
{
	Vertex dv = v[1] - v[0];
	Vector dp = dv.p.abs();

	__m128i dpi = dp;

	int dx = dpi.m128i_i32[0];
	int dy = dpi.m128i_i32[1];

	if(dx == 0 && dy == 0) return;

	int i = dx > dy ? 0 : 1;

	Vertex edge = v[0];
	Vertex dedge = dv / dp.v[i];

	// TODO: prestep + clip with the scissor

	int steps = dpi.m128i_i32[i];

	while(steps-- > 0)
	{
		DrawPoint(&edge);

		edge += dedge;
	}
}

void GSRasterizer::DrawTriangle(Vertex* v)
{
	if(v[1].p.y < v[0].p.y) {Vertex::Exchange(&v[0], &v[1]);}
	if(v[2].p.y < v[0].p.y) {Vertex::Exchange(&v[0], &v[2]);}
	if(v[2].p.y < v[1].p.y) {Vertex::Exchange(&v[1], &v[2]);}

	if(!(v[0].p.y < v[2].p.y)) return;

	Vertex v01 = v[1] - v[0];
	Vertex v02 = v[2] - v[0];

	float temp = v01.p.y / v02.p.y;
	float longest = temp * v02.p.x - v01.p.x;

	int ledge, redge;
	if(longest > 0) {ledge = 0; redge = 1; if(longest < 1) longest = 1;}
	else if(longest < 0) {ledge = 1; redge = 0; if(longest > -1) longest = -1;}
	else return;

	Vertex edge[2] = {v[0], v[0]};
	
	Vertex dedge[2];
	dedge[0].p.y = dedge[1].p.y = 1;
	if(v01.p.y > 0) dedge[ledge] = v01 / v01.p.yyyy();
	if(v02.p.y > 0) dedge[redge] = v02 / v02.p.yyyy();

	Vertex scan;
	Vertex dscan = (v02 * temp - v01) / longest;

	dscan.p.y = 0;

	SetupScanline<true, true, true>(dscan);

	for(int i = 0; i < 2; i++, v++)
	{
		__m128i tb = Vector(_mm_unpacklo_ps(edge[0].p, v[1].p)).ceil();

		int top = tb.m128i_i32[2];
		int bottom = tb.m128i_i32[3];

		if(top < m_scissor.top) top = min(m_scissor.top, bottom);
		if(bottom > m_scissor.bottom) bottom = m_scissor.bottom;

		if(edge[0].p.y < (float)top) // for(int j = 0; j < 2; j++) edge[j] += dedge[j] * ((float)top - edge[0].p.y);
		{
			float dy = (float)top - edge[0].p.y;
			edge[0] += dedge[0] * dy;
			edge[1].p.x += dedge[1].p.x * dy;
			edge[0].p.y = edge[1].p.y = (float)top; // ?
		}

		ASSERT(top >= bottom || (int)((edge[1].p.y - edge[0].p.y) * 10) == 0);

		for(; top < bottom; top++)
		{
			__m128i lr = Vector(_mm_unpacklo_ps(edge[0].p, edge[1].p)).ceil();

			int left = lr.m128i_i32[0];
			int right = lr.m128i_i32[1];

			if(left < m_scissor.left) left = m_scissor.left;
			if(right > m_scissor.right) right = m_scissor.right;

			if(right > left)
			{
				scan = edge[0];

				if(edge[0].p.x < (float)left)
				{
					scan += dscan * ((float)left - edge[0].p.x);
					scan.p.x = (float)left; // ?
				}

				(this->*m_dsf)(top, left, right, scan);
			}

			// for(int j = 0; j < 2; j++) edge[j] += dedge[j];
			edge[0] += dedge[0];
			edge[1].p += dedge[1].p;
		}

		if(v[1].p.y < v[2].p.y)
		{
			edge[ledge] = v[1];
			dedge[ledge] = (v[2] - v[1]) / (v[2].p - v[1].p).yyyy();
			edge[ledge] += dedge[ledge] * (edge[ledge].p.ceil() - edge[ledge].p).yyyy();
		}
	}
}

void GSRasterizer::DrawSprite(Vertex* v)
{
	if(v[2].p.y < v[0].p.y) {Vertex::Exchange(&v[0], &v[2]); Vertex::Exchange(&v[1], &v[3]);}
	if(v[1].p.x < v[0].p.x) {Vertex::Exchange(&v[0], &v[1]); Vertex::Exchange(&v[2], &v[3]);}

	__m128d tb = _mm_castps_pd(_mm_unpacklo_ps(v[0].p, v[2].p));
	__m128d lr = _mm_castps_pd(_mm_unpacklo_ps(v[0].p, v[1].p));
	__m128i tblr = Vector(_mm_castpd_ps(_mm_shuffle_pd(tb, lr, _MM_SHUFFLE2(0, 1)))).ceil();

	int top = tblr.m128i_i32[0];
	int bottom = tblr.m128i_i32[1];

	if(top < m_scissor.top) top = min(m_scissor.top, bottom);
	if(bottom > m_scissor.bottom) bottom = m_scissor.bottom;
	if(top >= bottom) return;

	int left = tblr.m128i_i32[2];
	int right = tblr.m128i_i32[3];

	if(left < m_scissor.left) left = m_scissor.left;
	if(right > m_scissor.right) right = m_scissor.right;
	if(left >= right) return;

	Vertex scan = v[0];

	if(DrawSolidRect(left, top, right, bottom, scan))
	{
		return;
	}

	Vertex dedge, dscan;

	dedge.p = Vector::zero();
	dscan.p = Vector::zero();

	if(m_sel.tfx < 4)
	{
		dedge.t = (v[2].t - v[0].t) / (v[2].p - v[0].p).yyyy();
		dscan.t = (v[1].t - v[0].t) / (v[1].p - v[0].p).xxxx();

		if(scan.p.y < (float)top) scan.t += dedge.t * ((float)top - scan.p.y);
		if(scan.p.x < (float)left) scan.t += dscan.t * ((float)left - scan.p.x);

		SetupScanline<true, true, false>(dscan);

		for(; top < bottom; top++, scan.t += dedge.t)
		{
			(this->*m_dsf)(top, left, right, scan);
		}
	}
	else
	{
		SetupScanline<true, false, false>(dscan);

		for(; top < bottom; top++)
		{
			(this->*m_dsf)(top, left, right, scan);
		}
	}
}

bool GSRasterizer::DrawSolidRect(int left, int top, int right, int bottom, const Vertex& v)
{
	if(left >= right || top >= bottom || !m_solidrect)
	{
		return false;
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

	DWORD c = v.c;

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
				fbco->addr[i] = _mm_set1_epi32(context->ftbl->pa(0, i, context->FRAME.Block(), context->FRAME.FBW));
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
				zbco->addr[i] = _mm_set1_epi32(context->ztbl->pa(0, i, context->ZBUF.Block(), context->FRAME.FBW));
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
		Vector dp = dv.p;

		slenv->dz0123 = dp.zzzz() * Vector(0, 1, 2, 3);	
		slenv->df0123 = dp.wwww() * Vector(0, 1, 2, 3); 

		Vector dp4 = dp * 4.0f;

		slenv->dz = dp4.zzzz();
		slenv->df = dp4.wwww();
	}

	if(tex)
	{
		Vector dt = dv.t;

		slenv->ds0123 = dt.xxxx() * Vector(0, 1, 2, 3); 
		slenv->dt0123 = dt.yyyy() * Vector(0, 1, 2, 3); 
		slenv->dq0123 = dt.zzzz() * Vector(0, 1, 2, 3); 

		Vector dt4 = dt * 4.0f;

		slenv->ds = dt4.xxxx();
		slenv->dt = dt4.yyyy();
		slenv->dq = dt4.zzzz();
	}

	if(col)
	{
		Vector dc = dv.c;

		slenv->dr0123 = dc.xxxx() * Vector(0, 1, 2, 3); 
		slenv->dg0123 = dc.yyyy() * Vector(0, 1, 2, 3); 
		slenv->db0123 = dc.zzzz() * Vector(0, 1, 2, 3); 
		slenv->da0123 = dc.wwww() * Vector(0, 1, 2, 3); 

		Vector dc4 = dc * 4.0f;

		slenv->dr = dc4.xxxx();
		slenv->dg = dc4.yyyy();
		slenv->db = dc4.zzzz();
		slenv->da = dc4.wwww();
	}
}

template<int iFPSM, int iZPSM, int ztst, int iip>
void GSRasterizer::DrawScanline(int top, int left, int right, const Vertex& v)	
{
	ScanlineEnvironment* slenv = m_slenv;

	int fpsm = GSLocalMemory::DecodeFPSM(iFPSM);
	int zpsm = GSLocalMemory::DecodeZPSM(iZPSM);

	__m128i fa_base = m_fbco->addr[top];
	__m128i* fa_offset = (__m128i*)&m_state->m_context->ftbl->rowOffset[top & 7][left];

	__m128i za_base = m_zbco->addr[top];
	__m128i* za_offset = (__m128i*)&m_state->m_context->ztbl->rowOffset[top & 7][left];

	Vector vp = v.p;
	Vector z = vp.zzzz(); z += slenv->dz0123;
	Vector f = vp.wwww(); f += slenv->df0123;

	Vector vt = v.t;
	Vector s = vt.xxxx(); s += slenv->ds0123;
	Vector t = vt.yyyy(); t += slenv->dt0123;
	Vector q = vt.zzzz(); q += slenv->dq0123;

	Vector vc = v.c;
	Vector r = vc.xxxx(); if(iip) r += slenv->dr0123;
	Vector g = vc.yyyy(); if(iip) g += slenv->dg0123;
	Vector b = vc.zzzz(); if(iip) b += slenv->db0123;
	Vector a = vc.wwww(); if(iip) a += slenv->da0123;

	for(int steps = right - left; steps > 0; steps -= 4)
	{
		do
		{

		int pixels = min(steps, 4);

		__m128i fa = _mm_add_epi32(fa_base, _mm_loadu_si128(fa_offset));
		__m128i za = _mm_add_epi32(za_base, _mm_loadu_si128(za_offset));
		
		__m128i fm =  slenv->fm;
		__m128i zm =  slenv->zm;
		__m128i test = _mm_setzero_si128();

		__m128i zi = _mm_or_si128(_mm_slli_epi32(z * 0.5f, 1), _mm_and_si128(z, _mm_set1_epi32(1)));

		if(ztst > 1)
		{
			__m128i zd0123;

			// TODO: benchmark this, it may not be faster, but compiles to only 3 instructions per line for a 32 bit z buffer (extract, mov, insert)

			#if 0 // _M_SSE >= 0x400

			zd0123 = _mm_insert_epi32(zd0123, m_state->m_mem.readPixelX(zpsm, _mm_extract_epi32(za, 0)), 0);
			zd0123 = _mm_insert_epi32(zd0123, m_state->m_mem.readPixelX(zpsm, _mm_extract_epi32(za, 1)), 1);
			zd0123 = _mm_insert_epi32(zd0123, m_state->m_mem.readPixelX(zpsm, _mm_extract_epi32(za, 2)), 2);
			zd0123 = _mm_insert_epi32(zd0123, m_state->m_mem.readPixelX(zpsm, _mm_extract_epi32(za, 3)), 3);

			#else

			for(int i = 0; i < pixels; i++)
			{
				zd0123.m128i_u32[i] = m_state->m_mem.readPixelX(zpsm, za.m128i_u32[i]);
			}

			#endif

			__m128i zs = _mm_sub_epi32(zi, _mm_set1_epi32(0x80000000));
			__m128i zd = _mm_sub_epi32(zd0123, _mm_set1_epi32(0x80000000));

			switch(ztst)
			{
			case 2: test = _mm_cmplt_epi32(zs, zd); break; // ge
			case 3: test = _mm_or_si128(_mm_cmplt_epi32(zs, zd), _mm_cmpeq_epi32(zs, zd)); break; // g
			default: __assume(0);
			}

			if(_mm_movemask_epi8(test) == 0xffff)
			{
				continue;
			}
		}

		Vector c[12];

		if(m_sel.tfx < 4)
		{
			Vector u = s;
			Vector v = t;

			if(!m_sel.fst)
			{
				Vector w = q.rcp();

				u *= w;
				v *= w;
			}

			if(m_sel.ltf)
			{
				u -= 0.5f;
				v -= 0.5f;

				Vector uf = u.floor();
				Vector vf = v.floor();
				
				Vector uff = u - uf;
				Vector vff = v - vf;

				__m128i uv = _mm_packs_epi32(uf, vf);
				__m128i uv0 = Wrap(uv);
				__m128i uv1 = Wrap(_mm_add_epi16(uv, _mm_set1_epi32(0x00010001)));

				for(int i = 0; i < pixels; i++)
				{
					if(ztst > 1 && test.m128i_u32[i])
					{
						continue;
					}

					FetchTexel(uv0.m128i_u16[i], uv0.m128i_u16[i + 4]);
					FetchTexel(uv1.m128i_u16[i], uv0.m128i_u16[i + 4]);
					FetchTexel(uv0.m128i_u16[i], uv1.m128i_u16[i + 4]);
					FetchTexel(uv1.m128i_u16[i], uv1.m128i_u16[i + 4]);

					Vector c00(ReadTexelNoFetch(uv0.m128i_u16[i], uv0.m128i_u16[i + 4]));
					Vector c01(ReadTexelNoFetch(uv1.m128i_u16[i], uv0.m128i_u16[i + 4]));
					Vector c10(ReadTexelNoFetch(uv0.m128i_u16[i], uv1.m128i_u16[i + 4]));
					Vector c11(ReadTexelNoFetch(uv1.m128i_u16[i], uv1.m128i_u16[i + 4]));

					c00 = c00.lerp(c01, uff.v[i]);
					c10 = c10.lerp(c11, uff.v[i]);
					c00 = c00.lerp(c10, vff.v[i]);

					c[i] = c00;
				}

				Vector::transpose(c[0], c[1], c[2], c[3]);
			}
			else
			{
				__m128i uv = _mm_packs_epi32(u, v);
				__m128i uv0 = Wrap(uv);

				__m128i c00;

				for(int i = 0; i < pixels; i++)
				{
					if(ztst > 1 && test.m128i_u32[i])
					{
						continue;
					}

					c00.m128i_u32[i] = ReadTexel(uv0.m128i_u16[i], uv0.m128i_u16[i + 4]);
				}

				Vector::expand(c00, c[0], c[1], c[2], c[3]);
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
			__m128i t;

			switch(m_sel.atst)
			{
			case 0: t = _mm_set1_epi32(0xffffffff); break; // never 
			case 1: t = _mm_setzero_si128(); break; // always
			case 2: case 3: t = _mm_cmpgt_epi32(c[3], slenv->aref); break; // l, le
			case 4: t = _mm_xor_si128(_mm_cmpeq_epi32(c[3], slenv->aref), _mm_set1_epi32(0xffffffff)); break; // e
			case 5: case 6: t = _mm_cmplt_epi32(c[3], slenv->aref); break; // ge, g
			case 7: t = _mm_cmpeq_epi32(c[3], slenv->aref); break; // ne 
			default: __assume(0);
			}

			switch(m_sel.afail)
			{
			case 0:
				fm = _mm_or_si128(fm, t);
				zm = _mm_or_si128(zm, t);
				test = _mm_or_si128(test, t);
				if(_mm_movemask_epi8(test) == 0xffff) continue;
				break;
			case 1:
				zm = _mm_or_si128(zm, t);
				break;
			case 2:
				fm = _mm_or_si128(fm, t);
				break;
			case 3: 
				fm = _mm_or_si128(fm, _mm_and_si128(t, _mm_set1_epi32(0xff000000)));
				zm = _mm_or_si128(zm, t);
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

		__m128i d = _mm_setzero_si128();

		if(m_sel.rfb)
		{
			for(int i = 0; i < pixels; i++)
			{
				if(test.m128i_u32[i])
				{
					continue;
				}

				d.m128i_u32[i] = m_state->m_mem.readFrameX(fpsm, fa.m128i_u32[i]);
			}
		}

		if(m_sel.date)
		{
			test = _mm_or_si128(test, _mm_srai_epi32(_mm_xor_si128(d, slenv->datm), 31));

			if(_mm_movemask_epi8(test) == 0xffff)
			{
				continue;
			}
		}

		fm = _mm_or_si128(fm, test);
		zm = _mm_or_si128(zm, test);

		if(m_sel.abe)
		{
			Vector::expand(d, c[4], c[5], c[6], c[7]);

			c[8] = Vector::zero();
			c[9] = Vector::zero();
			c[10] = Vector::zero();
			c[11] = slenv->afix;

			int abea = m_sel.abea;
			int abeb = m_sel.abeb;
			int abec = m_sel.abec;
			int abed = m_sel.abed;

			Vector r = (c[abea*4 + 0] - c[abeb*4 + 0]).mod2x(c[abec*4 + 3]) + c[abed*4 + 0];
			Vector g = (c[abea*4 + 1] - c[abeb*4 + 1]).mod2x(c[abec*4 + 3]) + c[abed*4 + 1];
			Vector b = (c[abea*4 + 2] - c[abeb*4 + 2]).mod2x(c[abec*4 + 3]) + c[abed*4 + 2];

			if(m_sel.abe == 2)
			{
				Vector mask = c[3] >= Vector(128.0f);

				c[0] = c[0].blend(r, mask);
				c[1] = c[1].blend(g, mask);
				c[2] = c[2].blend(b, mask);
			}
			else
			{
				c[0] = r;
				c[1] = g;
				c[2] = b;
			}
		}

		__m128i rb = _mm_packs_epi32(c[0], c[2]);
		__m128i ga = _mm_packs_epi32(c[1], c[3]);
		
		__m128i rg = _mm_and_si128(_mm_unpacklo_epi16(rb, ga), slenv->colclamp);
		__m128i ba = _mm_and_si128(_mm_unpackhi_epi16(rb, ga), slenv->colclamp);
		
		__m128i s = _mm_or_si128(_mm_packus_epi16(_mm_unpacklo_epi32(rg, ba), _mm_unpackhi_epi32(rg, ba)), slenv->fba);

		s = _mm_or_si128(_mm_andnot_si128(fm, s), _mm_and_si128(fm, d));

		for(int i = 0; i < pixels; i++)
		{
			if(fm.m128i_u32[i] != 0xffffffff)
			{
				m_state->m_mem.writeFrameX(fpsm, fa.m128i_u32[i], s.m128i_u32[i]);
			}

			if(ztst > 0 && zm.m128i_u32[i] != 0xffffffff)
			{
				m_state->m_mem.writePixelX(zpsm, za.m128i_u32[i], zi.m128i_u32[i]);
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

