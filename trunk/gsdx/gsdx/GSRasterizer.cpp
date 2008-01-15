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

#include "StdAfx.h"
#include "GSRasterizer.h"

GSRasterizer::GSRasterizer(GSState* state)
	: m_state(state)
	, m_fbco(NULL)
	, m_zbco(NULL)
{
	m_cache = (DWORD*)_aligned_malloc(1024 * 1024 * sizeof(m_cache[0]), 16);
	m_pagehash = 0;

	m_sl = (Scanline*)_aligned_malloc(sizeof(Scanline) * 1024, 16);
	m_slenv = (ScanlineEnv*)_aligned_malloc(sizeof(ScanlineEnv), 16);

	InvalidateTextureCache();

	// w00t :P

	#define InitPS_ZRW(iTFX, bFST, bFGE, bZRW) \
		m_ps1[iTFX][bFST][bFGE][bZRW] = &GSRasterizer::PrepareScanline1<iTFX, bFST, bFGE, bZRW>; \
		m_ps4[iTFX][bFST][bFGE][bZRW] = &GSRasterizer::PrepareScanline4<iTFX, bFST, bFGE, bZRW>; \

	#define InitPS_FGE(iTFX, bFST, bFGE) \
		InitPS_ZRW(iTFX, bFST, bFGE, 0) \
		InitPS_ZRW(iTFX, bFST, bFGE, 1) \

	#define InitPS_FST(iTFX, bFST) \
		InitPS_FGE(iTFX, bFST, 0) \
		InitPS_FGE(iTFX, bFST, 1) \

	#define InitPS_TFX(iTFX) \
		InitPS_FST(iTFX, 0) \
		InitPS_FST(iTFX, 1) \

	#define InitPS() \
		InitPS_TFX(0) \
		InitPS_TFX(1) \
		InitPS_TFX(2) \
		InitPS_TFX(3) \
		InitPS_FGE(4, 0, 0) \
		InitPS_FGE(4, 0, 1) \

	InitPS();

	#define InitDS_TAF(iZTST, iZPSM, bTAF) \
		m_ds[iZTST][iZPSM][bTAF] = &GSRasterizer::DrawScanline<iZTST, iZPSM, bTAF>; \

	#define InitDS_ZPSM(iZTST, iZPSM) \
		InitDS_TAF(iZTST, iZPSM, 0) \
		InitDS_TAF(iZTST, iZPSM, 1) \

	#define InitDS_ZTST(iZTST) \
		InitDS_ZPSM(iZTST, 0) \
		InitDS_ZPSM(iZTST, 1) \
		InitDS_ZPSM(iZTST, 2) \
		InitDS_ZPSM(iZTST, 3) \

	#define InitDS() \
		InitDS_ZTST(0) \
		InitDS_ZTST(1) \
		InitDS_ZTST(2) \

	InitDS();

	#define InitDST_FST(iTFX, bTCC, bLTF, bFST) \
		m_dst[iTFX][bTCC][bLTF][bFST] = &GSRasterizer::DrawScanlineT<iTFX, bTCC, bLTF, bFST>; \

	#define InitDST_LTF(iTFX, bTCC, bLTF) \
		InitDST_FST(iTFX, bTCC, bLTF, 0) \
		InitDST_FST(iTFX, bTCC, bLTF, 1) \

	#define InitDST_TCC(iTFX, bTCC) \
		InitDST_LTF(iTFX, bTCC, 0) \
		InitDST_LTF(iTFX, bTCC, 1) \

	#define InitDST_TFX(iTFX) \
		InitDST_TCC(iTFX, 0) \
		InitDST_TCC(iTFX, 1) \

	#define InitDST() \
		InitDST_TFX(0) \
		InitDST_TFX(1) \
		InitDST_TFX(2) \
		InitDST_TFX(3) \
		InitDST_LTF(4, 0, 0) \

	InitDST();

	#define InitDSOM_ABE(iFPSM, iZPSM, bRFB, bDATE, iABE) \
		m_dsom[iFPSM][iZPSM][bRFB][bDATE][iABE] = &GSRasterizer::DrawScanlineOM<iFPSM, iZPSM, bRFB, bDATE, iABE>; \

	#define InitDSOM_DATE(iFPSM, iZPSM, bRFB, bDATE) \
		InitDSOM_ABE(iFPSM, iZPSM, bRFB, bDATE, 0) \
		InitDSOM_ABE(iFPSM, iZPSM, bRFB, bDATE, 1) \
		InitDSOM_ABE(iFPSM, iZPSM, bRFB, bDATE, 2) \

	#define InitDSOM_RFB(iFPSM, iZPSM, bRFB) \
		InitDSOM_DATE(iFPSM, iZPSM, bRFB, 0) \
		InitDSOM_DATE(iFPSM, iZPSM, bRFB, 1) \

	#define InitDSOM_ZPSM(iFPSM, iZPSM) \
		InitDSOM_RFB(iFPSM, iZPSM, 0) \
		InitDSOM_RFB(iFPSM, iZPSM, 1) \

	#define InitDSOM_FPSM(iFPSM) \
		InitDSOM_ZPSM(iFPSM, 0) \
		InitDSOM_ZPSM(iFPSM, 1) \
		InitDSOM_ZPSM(iFPSM, 2) \
		InitDSOM_ZPSM(iFPSM, 3) \

	#define InitDSOM() \
		InitDSOM_FPSM(0) \
		InitDSOM_FPSM(1) \
		InitDSOM_FPSM(2) \
		InitDSOM_FPSM(3) \
		InitDSOM_FPSM(4) \
		InitDSOM_FPSM(5) \
		InitDSOM_FPSM(6) \
		InitDSOM_FPSM(7) \
		InitDSOM_FPSM(8) \
		InitDSOM_FPSM(9) \

	InitDSOM();
}

GSRasterizer::~GSRasterizer()
{
	_aligned_free(m_cache);
	_aligned_free(m_sl);
	_aligned_free(m_slenv);

	for(int i = 0, j = m_comap.GetSize(); i < j; i++)
	{
		_aligned_free(m_comap.GetValueAt(i));
	}

	m_comap.RemoveAll();
}

void GSRasterizer::InvalidateTextureCache() 
{
	memset(m_page, ~0, sizeof(m_page));
}

void GSRasterizer::BeginDraw()
{
	GSDrawingEnvironment& env = m_state->m_env;
	GSDrawingContext* context = m_state->m_context;
	GIFRegPRIM* PRIM = m_state->PRIM;

	m_scissor.left = max(context->SCISSOR.SCAX0, 0);
	m_scissor.top = max(context->SCISSOR.SCAY0, 0);
	m_scissor.right = min(context->SCISSOR.SCAX1 + 1, context->FRAME.FBW * 64);
	m_scissor.bottom = min(context->SCISSOR.SCAY1 + 1, 4096);

	int iFPSM = GSLocalMemory::EncodeFPSM(context->FRAME.PSM);
	int iTFX = PRIM->TME ? (int)context->TEX0.TFX : 4;
	int bTCC = 0;
	int bFST = 0;
	int bLTF = 0;
	int bFGE = PRIM->FGE ? 1 : 0;
	int iZPSM = GSLocalMemory::EncodeZPSM(context->ZBUF.PSM);
	int bZRW = (context->DepthRead() || context->DepthWrite()) ? 1 : 0;
	int iZTST = context->TEST.ZTE && context->TEST.ZTST > 0 ? context->TEST.ZTST - 1 : 0;
	int iATST = context->TEST.ATE ? context->TEST.ATST : 1;
	int iAFAIL = context->TEST.AFAIL;
	int bTAF = PRIM->TME || context->TEST.ATE && context->TEST.ATST != 1 || PRIM->FGE;
	int bRFB = m_state->PRIM->ABE || m_state->m_env.PABE.PABE || m_state->m_context->FRAME.FBMSK || m_state->m_context->TEST.ATE && m_state->m_context->TEST.ATST != 1 && m_state->m_context->TEST.AFAIL == 3;
	int bDATE = context->TEST.DATE;
	int iABE = env.PABE.PABE ? 2 : PRIM->ABE ? 1 : 0; // AA1 && prim == line
	int iALPHA_A = 0;
	int iALPHA_B = 0;
	int iALPHA_C = 0;
	int iALPHA_D = 0;

	if(iTFX != 4)
	{
		bTCC = context->TEX0.TCC ? 1 : 0;
		bFST = PRIM->FST ? 1 : 0;
		bLTF = (context->TEX1.MMAG & 1) | (context->TEX1.MMIN & 1); // TODO: if(context->TEX1.LCM == 0)

		if(context->TEX1.LCM)
		{
			bLTF = context->TEX1.K <= 0 && (context->TEX1.MMAG & 1) || context->TEX1.K > 0 && (context->TEX1.MMIN & 1);
		}
	}

	if(iABE)
	{
		iALPHA_A = min(context->ALPHA.A, 2);
		iALPHA_B = min(context->ALPHA.B, 2);
		iALPHA_C = min(context->ALPHA.C, 2);
		iALPHA_D = min(context->ALPHA.D, 2);

		if(iALPHA_A == iALPHA_B) iALPHA_C = 0;
	}

	m_ps1f = m_ps1[iTFX][bFST][bFGE][bZRW];
	m_ps4f = m_ps4[iTFX][bFST][bFGE][bZRW];
	m_dsf = m_ds[iZTST][iZPSM][bTAF];
	m_dstf = m_dst[iTFX][bTCC][bLTF][bFST];
	m_dsomf = m_dsom[iFPSM][iZPSM][bRFB][bDATE][iABE];

	m_slenv->fm = _mm_set1_epi32(context->FRAME.FBMSK);
	m_slenv->zm = _mm_set1_epi32(context->ZBUF.ZMSK ? 0xffffffff : 0);
	m_slenv->f.r = _mm_set1_ps((float)(int)env.FOGCOL.FCR);
	m_slenv->f.g = _mm_set1_ps((float)(int)env.FOGCOL.FCG);
	m_slenv->f.b = _mm_set1_ps((float)(int)env.FOGCOL.FCB);
	m_slenv->afix = _mm_set1_ps((float)(int)context->ALPHA.FIX);
	m_slenv->aref = _mm_set1_ps((float)(int)context->TEST.AREF);
	m_slenv->datm = _mm_set1_epi32(context->TEST.DATM ? 0x80000000 : 0);

	if(PRIM->TME)
	{
		SetupTexture();
	}

	SetupColumnOffset();

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
}

void GSRasterizer::DrawPoint(Vertex* v)
{
	// TODO: prestep

	CPoint p = *v;

	if(m_scissor.PtInRect(p))
	{
		Vertex scan = *v;
		Vertex dscan;

		(this->*m_dsf)(p.y, p.x, p.x + 1, scan, dscan);
	}
}

void GSRasterizer::DrawLine(Vertex* v)
{
	Vertex dv = v[1] - v[0];

	Vector dp = dv.p;

	dp.x.abs();
	dp.y.abs();

	int dx = (int)dp.x;
	int dy = (int)dp.y;

	if(dx == 0 && dy == 0) return;

	int i = dx > dy ? 0 : 1;

	Scalar dd = dp.v[i];

	Vertex edge = v[0];
	Vertex dedge = dv / dd;

	// TODO: prestep + clip with the scissor

	int steps = (int)dd;

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

	Scalar temp = v01.p.y / v02.p.y;
	Scalar longest = temp * v02.p.x - v01.p.x;

	int ledge, redge;
	if(Scalar(0) < longest) {ledge = 0; redge = 1; if(longest < Scalar(1)) longest = Scalar(1);}
	else if(longest < Scalar(0)) {ledge = 1; redge = 0; if(Scalar(-1) < longest) longest = Scalar(-1);}
	else return;

	Vertex edge[2] = {v[0], v[0]};

	Vertex dedge[2];
	dedge[0].p.y = dedge[1].p.y = Scalar(1);
	if(Scalar(0) < v01.p.y) dedge[ledge] = v01 / v01.p.y;
	if(Scalar(0) < v02.p.y) dedge[redge] = v02 / v02.p.y;

	Vertex scan;

	Vertex dscan = (v02 * temp - v01) / longest;
	dscan.p.y = 0;

	for(int i = 0; i < 2; i++, v++)
	{ 
		int top = edge[0].p.y.ceil_i();
		int bottom = v[1].p.y.ceil_i();

		if(top < m_scissor.top) top = min(m_scissor.top, bottom);
		if(bottom > m_scissor.bottom) bottom = m_scissor.bottom;

		if(edge[0].p.y < Scalar(top)) // for(int j = 0; j < 2; j++) edge[j] += dedge[j] * ((float)top - edge[0].p.y);
		{
			Scalar dy = Scalar(top) - edge[0].p.y;
			edge[0] += dedge[0] * dy;
			edge[1].p.x += dedge[1].p.x * dy;
			edge[0].p.y = edge[1].p.y = Scalar(top);
		}

		ASSERT(top >= bottom || (int)((edge[1].p.y - edge[0].p.y) * 10) == 0);

		for(; top < bottom; top++)
		{
			int left = edge[0].p.x.ceil_i();
			int right = edge[1].p.x.ceil_i();

			if(left < m_scissor.left) left = m_scissor.left;
			if(right > m_scissor.right) right = m_scissor.right;

			if(right > left)
			{
				scan = edge[0];

				if(edge[0].p.x < Scalar(left))
				{
					scan += dscan * (Scalar(left) - edge[0].p.x);
					scan.p.x = Scalar(left);
				}

				(this->*m_dsf)(top, left, right, scan, dscan);
			}

			// for(int j = 0; j < 2; j++) edge[j] += dedge[j];
			edge[0] += dedge[0];
			edge[1].p += dedge[1].p;
		}

		if(v[1].p.y < v[2].p.y)
		{
			edge[ledge] = v[1];
			dedge[ledge] = (v[2] - v[1]) / (v[2].p.y - v[1].p.y);
			edge[ledge] += dedge[ledge] * (edge[ledge].p.y.ceil_s() - edge[ledge].p.y);
		}
	}
}

void GSRasterizer::DrawSprite(Vertex* v)
{
	if(v[2].p.y < v[0].p.y) {Vertex::Exchange(&v[0], &v[2]); Vertex::Exchange(&v[1], &v[3]);}
	if(v[1].p.x < v[0].p.x) {Vertex::Exchange(&v[0], &v[1]); Vertex::Exchange(&v[2], &v[3]);}

	if(v[0].p.x == v[1].p.x || v[0].p.y == v[2].p.y) return;

	Vertex v01 = v[1] - v[0];
	Vertex v02 = v[2] - v[0];

	Vertex edge = v[0];
	Vertex dedge = v02 / v02.p.y;
	Vertex scan;
	Vertex dscan = v01 / v01.p.x;

	int top = v[0].p.y.ceil_i();
	int bottom = v[2].p.y.ceil_i();

	if(top < m_scissor.top) top = min(m_scissor.top, bottom);
	if(bottom > m_scissor.bottom) bottom = m_scissor.bottom;

	if(v[0].p.y < Scalar(top)) edge += dedge * (Scalar(top) - v[0].p.y);

	int left = v[0].p.x.ceil_i();
	int right = v[1].p.x.ceil_i();

	if(left < m_scissor.left) left = m_scissor.left;
	if(right > m_scissor.right) right = m_scissor.right;

	if(left >= right || top >= bottom) return;

	if(v[0].p.x < Scalar(left)) edge += dscan * (Scalar(left) - v[0].p.x);

	if(DrawSolidRect(left, top, right, bottom, edge))
		return;

	for(; top < bottom; top++)
	{
		scan = edge;

		(this->*m_dsf)(top, left, right, scan, dscan);

		edge += dedge;
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
		m_state->m_mem.FillRect(r, v.GetZ(), zpsm, zbp, bw);
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

void GSRasterizer::SetupTexture()
{
	GSDrawingEnvironment& env = m_state->m_env;
	GSDrawingContext* context = m_state->m_context;

	m_state->m_mem.SetupCLUT32(context->TEX0, env.TEXA);

	m_pagehash = context->TEX0.ai32[0];

	short tw = (short)(1 << context->TEX0.TW);
	short th = (short)(1 << context->TEX0.TH);

	switch(context->CLAMP.WMS)
	{
	case 0: 
		m_slenv->t.and.m128i_u16[0] = tw - 1;
		m_slenv->t.or.m128i_u16[0] = 0;
		m_slenv->t.mask.m128i_u32[0] = 0xffffffff; 
		break;
	case 1: 
		m_slenv->t.min.m128i_u16[0] = 0;
		m_slenv->t.max.m128i_u16[0] = tw - 1;
		m_slenv->t.mask.m128i_u32[0] = 0; 
		break;
	case 2: 
		m_slenv->t.min.m128i_u16[0] = context->CLAMP.MINU;
		m_slenv->t.max.m128i_u16[0] = context->CLAMP.MAXU;
		m_slenv->t.mask.m128i_u32[0] = 0; 
		break;
	case 3: 
		m_slenv->t.and.m128i_u16[0] = context->CLAMP.MINU;
		m_slenv->t.or.m128i_u16[0] = context->CLAMP.MAXU;
		m_slenv->t.mask.m128i_u32[0] = 0xffffffff; 
		break;
	default: 
		__assume(0);
	}

	switch(context->CLAMP.WMT)
	{
	case 0: 
		m_slenv->t.and.m128i_u16[4] = th - 1;
		m_slenv->t.or.m128i_u16[4] = 0;
		m_slenv->t.mask.m128i_u32[2] = 0xffffffff; 
		break;
	case 1: 
		m_slenv->t.min.m128i_u16[4] = 0;
		m_slenv->t.max.m128i_u16[4] = th - 1;
		m_slenv->t.mask.m128i_u32[2] = 0; 
		break;
	case 2: 
		m_slenv->t.min.m128i_u16[4] = context->CLAMP.MINV;
		m_slenv->t.max.m128i_u16[4] = context->CLAMP.MAXV;
		m_slenv->t.mask.m128i_u32[2] = 0; 
		break;
	case 3: 
		m_slenv->t.and.m128i_u16[4] = context->CLAMP.MINV;
		m_slenv->t.or.m128i_u16[4] = context->CLAMP.MAXV;
		m_slenv->t.mask.m128i_u32[2] = 0xffffffff; 
		break;
	default: 
		__assume(0);
	}

	m_slenv->t.and = _mm_shufflelo_epi16(m_slenv->t.and, _MM_SHUFFLE(0, 0, 0, 0));
	m_slenv->t.and = _mm_shufflehi_epi16(m_slenv->t.and, _MM_SHUFFLE(0, 0, 0, 0));
	m_slenv->t.or = _mm_shufflelo_epi16(m_slenv->t.or, _MM_SHUFFLE(0, 0, 0, 0));
	m_slenv->t.or = _mm_shufflehi_epi16(m_slenv->t.or, _MM_SHUFFLE(0, 0, 0, 0));
	m_slenv->t.mask = _mm_shuffle_epi32(m_slenv->t.mask, _MM_SHUFFLE(2, 2, 0, 0));
}

void GSRasterizer::FetchTexture(int x, int y)
{
	const int xs = 64;
	const int ys = 32;

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

template<int iTFX, int bFST, int bFGE, int bZRW>
void GSRasterizer::PrepareScanline1(int x, int y, int steps, const Vertex& v, const Vertex& dv)
{
	Scanline* sl = m_sl;

	sl->fa.m128i_u32[0] = m_fbco->addr[y].m128i_u32[0] + m_state->m_context->ftbl->rowOffset[y & 7][x];
	sl->fm = m_slenv->fm;
	sl->zm = m_slenv->zm;
	sl->t = _mm_setzero_si128();

	if(bZRW)
	{
		__m128 r0 = v.p.xyzq;

		sl->za.m128i_u32[0] = m_zbco->addr[y].m128i_u32[0] + m_state->m_context->ztbl->rowOffset[y & 7][x];
		// sl->z.m128i_u32[0] = (DWORD)r0.m128_f32[2];
		sl->z = _mm_slli_epi32(_mm_cvttps_epi32(_mm_mul_ps(_mm_shuffle_ps(r0, r0, _MM_SHUFFLE(2, 2, 2, 2)), _mm_set1_ps(0.5f))), 1);
	}

	if(iTFX != 4 || bFGE)
	{
		if(iTFX != 4)
		{
			__m128 r0 = v.t.xyzq;

			if(!bFST)
			{
				sl->q = _mm_shuffle_ps(r0, r0, _MM_SHUFFLE(3, 3, 3, 3));
			}

			sl->u = _mm_shuffle_ps(r0, r0, _MM_SHUFFLE(0, 0, 0, 0));
			sl->v = _mm_shuffle_ps(r0, r0, _MM_SHUFFLE(1, 1, 1, 1));
		}

		if(bFGE)
		{
			__m128 r0 = v.t.xyzq;

			sl->f = _mm_shuffle_ps(r0, r0, _MM_SHUFFLE(2, 2, 2, 2));
		}
	}

	if(iTFX != 1)
	{
		__m128 r0 = v.c.xyzq;

		sl->r[0] = _mm_shuffle_ps(r0, r0, _MM_SHUFFLE(0, 0, 0, 0));
		sl->g[0] = _mm_shuffle_ps(r0, r0, _MM_SHUFFLE(1, 1, 1, 1));
		sl->b[0] = _mm_shuffle_ps(r0, r0, _MM_SHUFFLE(2, 2, 2, 2));
		sl->a[0] = _mm_shuffle_ps(r0, r0, _MM_SHUFFLE(3, 3, 3, 3));
	}
}

template<int iTFX, int bFST, int bFGE, int bZRW>
void GSRasterizer::PrepareScanline4(int x, int y, int steps, const Vertex& v, const Vertex& dv)
{
	Scanline* sl = m_sl;

	__m128 z, dz;
	__m128 t0, t1, t2, t3, dt = dv.t;
	__m128 c0, c1, c2, c3, dc = dv.c;

	if(bZRW)
	{
		z = _mm_shuffle_ps(v.p.xyzq, v.p.xyzq, _MM_SHUFFLE(2, 2, 2, 2));
		dz = _mm_shuffle_ps(dv.p.xyzq, dv.p.xyzq, _MM_SHUFFLE(2, 2, 2, 2));
		
		z = _mm_add_ps(z, _mm_mul_ps(dz, _mm_set_ps(3, 2, 1, 0)));
		dz = _mm_mul_ps(dz, _mm_set1_ps(4));
	}

	if(iTFX != 4 || bFGE)
	{
		__m128 dt2 = _mm_add_ps(dt, dt);
		__m128 dt3 = _mm_add_ps(dt2, dt);

		t0 = v.t.xyzq;
		t1 = _mm_add_ps(v.t.xyzq, dt);
		t2 = _mm_add_ps(v.t.xyzq, dt2);
		t3 = _mm_add_ps(v.t.xyzq, dt3);

		dt = _mm_add_ps(dt2, dt2);
	}

	if(iTFX != 1)
	{
		__m128 dc2 = _mm_add_ps(dc, dc);
		__m128 dc3 = _mm_add_ps(dc2, dc);

		c0 = v.c.xyzq;
		c1 = _mm_add_ps(v.c.xyzq, dc);
		c2 = _mm_add_ps(v.c.xyzq, dc2);
		c3 = _mm_add_ps(v.c.xyzq, dc3);

		dc = _mm_add_ps(dc2, dc2);
	}

	__m128i fa_base = m_fbco->addr[y];
	int* fa_offset = &m_state->m_context->ftbl->rowOffset[y & 7][x];

	__m128i za_base;
	int* za_offset;

	if(bZRW)
	{
		za_base = m_zbco->addr[y];
		za_offset = &m_state->m_context->ztbl->rowOffset[y & 7][x];
	}

	for(int i = 0; i < steps; i += 4, sl++)
	{
		sl->fa = _mm_add_epi32(fa_base, _mm_loadu_si128((__m128i*)&fa_offset[i]));
		sl->fm = m_slenv->fm;
		sl->zm = m_slenv->zm;
		sl->t = _mm_setzero_si128();

		if(bZRW)
		{
			sl->za = _mm_add_epi32(za_base, _mm_loadu_si128((__m128i*)&za_offset[i]));
			sl->z = _mm_slli_epi32(_mm_cvttps_epi32(_mm_mul_ps(z, _mm_set1_ps(0.5f))), 1);

			z = _mm_add_ps(z, dz);
		}

		if(iTFX != 4 || bFGE)
		{
			__m128 r0 = t0;
			__m128 r1 = t1;
			__m128 r2 = t2;
			__m128 r3 = t3;

			_MM_TRANSPOSE4_PS(r0, r1, r2, r3); 

			if(iTFX != 4)
			{
				if(!bFST)
				{
					sl->q = r3;
				}

				sl->u = r0;
				sl->v = r1;
			}

			if(bFGE)
			{
				sl->f = r2;
			}

			t0 = _mm_add_ps(t0, dt);
			t1 = _mm_add_ps(t1, dt);
			t2 = _mm_add_ps(t2, dt);
			t3 = _mm_add_ps(t3, dt);
		}

		if(iTFX != 1)
		{
			__m128 r0 = c0;
			__m128 r1 = c1;
			__m128 r2 = c2;
			__m128 r3 = c3;

			_MM_TRANSPOSE4_PS(r0, r1, r2, r3); 

			sl->r[0] = r0; 
			sl->g[0] = r1; 
			sl->b[0] = r2; 
			sl->a[0] = r3;

			c0 = _mm_add_ps(c0, dc);
			c1 = _mm_add_ps(c1, dc);
			c2 = _mm_add_ps(c2, dc);
			c3 = _mm_add_ps(c3, dc);
		}
	}
}

template<int iZTST, int iZPSM, int bTAF>
void GSRasterizer::DrawScanline(int top, int left, int right, Vertex& v, const Vertex& dv)	
{
	int steps = right - left;

	if(steps > 1)
	{
		(this->*m_ps4f)(left, top, steps, v, dv);

		if(iZTST)
		{
			int psm = GSLocalMemory::DecodeZPSM(iZPSM);

			Scanline* sl = m_sl;

			for(int i = 0; i < steps; i += 4, sl++)
			{
				DWORD zd0 = m_state->m_mem.readPixelX(psm, sl->za.m128i_u32[0]);
				DWORD zd1 = m_state->m_mem.readPixelX(psm, sl->za.m128i_u32[1]);
				DWORD zd2 = m_state->m_mem.readPixelX(psm, sl->za.m128i_u32[2]);
				DWORD zd3 = m_state->m_mem.readPixelX(psm, sl->za.m128i_u32[3]);

				__m128i zs = _mm_sub_epi32(sl->z, _mm_set1_epi32(0x80000000));
				__m128i zd = _mm_sub_epi32(_mm_set_epi32(zd3, zd2, zd1, zd0), _mm_set1_epi32(0x80000000));

				switch(iZTST)
				{
				case 1: sl->t = _mm_cmplt_epi32(zs, zd); break; // ge
				case 2: sl->t = _mm_cmpgt_epi32(zd, zs); break; // g
				default: __assume(0);
				}
			}
		}
	}
	else
	{
		(this->*m_ps1f)(left, top, steps, v, dv);

		if(iZTST)
		{
			int psm = GSLocalMemory::DecodeZPSM(iZPSM);

			Scanline* sl = m_sl;

			DWORD zs = sl->z.m128i_u32[0];
			DWORD zd = m_state->m_mem.readPixelX(psm, sl->za.m128i_u32[0]);

			switch(iZTST)
			{
			case 1: if(zs < zd) sl->t.m128i_u32[0] = 0xffffffff; break; // ge
			case 2: if(zs <= zd) sl->t.m128i_u32[0] = 0xffffffff; break; // g
			default: __assume(0);
			}
		}
	}

	if(bTAF)
	{
		(this->*m_dstf)(steps);
	}

	(this->*m_dsomf)(steps);
}

template<int iTFX, bool bTCC, int bLTF, int bFST>
void GSRasterizer::DrawScanlineT(int steps)
{
	int iATST = m_state->m_context->TEST.ATE ? m_state->m_context->TEST.ATST : 1;
	int iAFAIL = m_state->m_context->TEST.AFAIL;
	int bZTST = m_state->m_context->TEST.ZTE && m_state->m_context->TEST.ZTST != 1 ? 1 : 0;
	int bFGE = m_state->PRIM->FGE;

	Scanline* sl = m_sl;

	__m128 c[4];

	c[0] = _mm_setzero_ps();
	c[1] = _mm_setzero_ps();
	c[2] = _mm_setzero_ps();
	c[3] = _mm_setzero_ps();

	__m128 aref = m_slenv->aref;

	for(int j = 0; j < steps; j += 4, sl++)
	{
		if(bZTST && _mm_movemask_epi8(sl->t) == 0xffff)
		{
			continue;
		}

		if(iTFX < 4)
		{
			__m128 u = sl->u;
			__m128 v = sl->v;

			if(!bFST)
			{
				__m128 w = _mm_rcp_ps(sl->q);

				u = _mm_mul_ps(u, w);
				v = _mm_mul_ps(v, w);
			}
/*
			if(bLTF)
			{
				__m128i uif = _mm_cvtps_epi32(_mm_mul_ps(u, _mm_set1_ps(65535.5f)));
				__m128i vif = _mm_cvtps_epi32(_mm_mul_ps(v, _mm_set1_ps(65535.5f)));

				__m128i uv = _mm_packs_epi32(_mm_srai_epi32(uif, 16), _mm_srai_epi32(vif, 16));

				__m128i uv0 = Wrap(uv);
				__m128i uv1 = Wrap(_mm_add_epi16(uv, _mm_set1_epi32(0x00010001)));

				uif = _mm_srli_epi16(uif, 9);
				vif = _mm_srli_epi16(vif, 9);

				uif = _mm_or_si128(_mm_slli_epi32(uif, 16), _mm_sub_epi32(_mm_set1_epi32(0x00000080), uif));
				vif = _mm_or_si128(_mm_slli_epi32(vif, 16), _mm_sub_epi32(_mm_set1_epi32(0x00000080), vif));

				for(int i = 0, k = min(steps - j, 4); i < k; i++)
				{
					if(bZTST && sl->t.m128i_u32[i])
					{
						continue;
					}

					FetchTexel(uv0.m128i_u16[i], uv0.m128i_u16[i + 4]);
					FetchTexel(uv1.m128i_u16[i], uv0.m128i_u16[i + 4]);
					FetchTexel(uv0.m128i_u16[i], uv1.m128i_u16[i + 4]);
					FetchTexel(uv1.m128i_u16[i], uv1.m128i_u16[i + 4]);

					__m128i s;

					s.m128i_u32[0] = ReadTexelNoFetch(uv0.m128i_u16[i], uv0.m128i_u16[i + 4]);
					s.m128i_u32[1] = ReadTexelNoFetch(uv1.m128i_u16[i], uv0.m128i_u16[i + 4]);
					s.m128i_u32[2] = ReadTexelNoFetch(uv0.m128i_u16[i], uv1.m128i_u16[i + 4]);
					s.m128i_u32[3] = ReadTexelNoFetch(uv1.m128i_u16[i], uv1.m128i_u16[i + 4]);

					__m128i zero = _mm_setzero_si128();

					__m128i s0 = _mm_unpacklo_epi8(s, zero);
					__m128i s1 = _mm_unpackhi_epi8(s, zero);

					__m128i s2 = _mm_unpacklo_epi16(s0, s1);
					__m128i s3 = _mm_unpackhi_epi16(s0, s1);

					s0 = _mm_madd_epi16(s2, _mm_set1_epi32(vif.m128i_u32[i]));
					s1 = _mm_madd_epi16(s3, _mm_set1_epi32(vif.m128i_u32[i]));

					s0 = _mm_unpacklo_epi16(_mm_packs_epi32(s0, s0), _mm_packs_epi32(s1, s1));

					s0 = _mm_srli_epi32(_mm_madd_epi16(s0, _mm_set1_epi32(uif.m128i_u32[i])), 14);

					c[i] = _mm_cvtepi32_ps(s0);
				}

				_MM_TRANSPOSE4_PS(c[0], c[1], c[2], c[3]); 
			}
*/
			if(bLTF)
			{
				u = _mm_sub_ps(u, _mm_set1_ps(0.5f));
				v = _mm_sub_ps(v, _mm_set1_ps(0.5f));

				__m128i ui = _mm_cvttps_epi32(u);
				__m128i vi = _mm_cvttps_epi32(v);
				__m128 uf = _mm_sub_ps(u, _mm_cvtepi32_ps(ui));
				__m128 vf = _mm_sub_ps(v, _mm_cvtepi32_ps(vi));
				__m128i uv = _mm_packs_epi32(ui, vi);
				__m128i uv0 = Wrap(uv);
				__m128i uv1 = Wrap(_mm_add_epi16(uv, _mm_set1_epi32(0x00010001)));

				for(int i = 0, k = min(steps - j, 4); i < k; i++)
				{
					if(bZTST && sl->t.m128i_u32[i])
					{
						continue;
					}

					FetchTexel(uv0.m128i_u16[i], uv0.m128i_u16[i + 4]);
					FetchTexel(uv1.m128i_u16[i], uv0.m128i_u16[i + 4]);
					FetchTexel(uv0.m128i_u16[i], uv1.m128i_u16[i + 4]);
					FetchTexel(uv1.m128i_u16[i], uv1.m128i_u16[i + 4]);

					__m128 c00 = Unpack(ReadTexelNoFetch(uv0.m128i_u16[i], uv0.m128i_u16[i + 4]));
					__m128 c01 = Unpack(ReadTexelNoFetch(uv1.m128i_u16[i], uv0.m128i_u16[i + 4]));
					__m128 c10 = Unpack(ReadTexelNoFetch(uv0.m128i_u16[i], uv1.m128i_u16[i + 4]));
					__m128 c11 = Unpack(ReadTexelNoFetch(uv1.m128i_u16[i], uv1.m128i_u16[i + 4]));

					c00 = Blend(c00, c01, _mm_set1_ps(uf.m128_f32[i]));
					c10 = Blend(c10, c11, _mm_set1_ps(uf.m128_f32[i]));
					c00 = Blend(c00, c10, _mm_set1_ps(vf.m128_f32[i]));

					c[i] = c00;
				}

				_MM_TRANSPOSE4_PS(c[0], c[1], c[2], c[3]); 
			}

			else
			{
				__m128i ui = _mm_cvttps_epi32(u);
				__m128i vi = _mm_cvttps_epi32(v);
				__m128i uv = _mm_packs_epi32(ui, vi);
				__m128i uv0 = Wrap(uv);

				__m128i s;

				for(int i = 0, k = min(steps - j, 4); i < k; i++)
				{
					if(bZTST && sl->t.m128i_u32[i])
					{
						continue;
					}

					s.m128i_u32[i] = ReadTexel(uv0.m128i_u16[i], uv0.m128i_u16[i + 4]);
				}

				__m128i mask = _mm_set1_epi32(0xff);

				c[0] = _mm_cvtepi32_ps(_mm_and_si128(s, mask));
				c[1] = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(s, 8), mask));
				c[2] = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(s, 16), mask));
				c[3] = _mm_cvtepi32_ps(_mm_srli_epi32(s, 24));
			}
		}

		switch(iTFX)
		{
		case 0: c[3] = bTCC ? Saturate(Modulate(c[3], sl->a[0])) : sl->a[0]; break;
		case 1: break;
		case 2: c[3] = bTCC ? Saturate(_mm_add_ps(c[3], sl->a[0])) : sl->a[0]; break;
		case 3: if(bTCC) c[3] = sl->a[0]; break;
		case 4: c[3] = sl->a[0]; break; 
		default: __assume(0);
		}

		if(iATST != 1)
		{
			__m128 t;

			switch(iATST)
			{
			case 0: t = _mm_cmpeq_ps(_mm_setzero_ps(), _mm_setzero_ps()); break;
			case 1: t = _mm_setzero_ps(); break;
			case 2: t = _mm_cmpge_ps(c[3], aref); break;
			case 3: t = _mm_cmpgt_ps(c[3], aref); break;
			case 4: t = _mm_cmpneq_ps(c[3], aref); break;
			case 5: t = _mm_cmplt_ps(c[3], aref); break;
			case 6: t = _mm_cmple_ps(c[3], aref); break;
			case 7: t = _mm_cmpeq_ps(c[3], aref); break;
			default: __assume(0);
			}

			__m128i ti = *(__m128i*)&t;

			switch(iAFAIL)
			{
			case 0:
				sl->fm = _mm_or_si128(sl->fm, ti);
				sl->zm = _mm_or_si128(sl->zm, ti);
				sl->t = _mm_or_si128(sl->t, ti);
				if(_mm_movemask_epi8(sl->t) == 0xffff) continue;
				break;
			case 1:
				sl->zm = _mm_or_si128(sl->zm, ti);
				break;
			case 2:
				sl->fm = _mm_or_si128(sl->fm, ti);
				break;
			case 3: 
				sl->fm = _mm_or_si128(sl->fm, _mm_and_si128(ti, _mm_set1_epi32(0xff000000)));
				sl->zm = _mm_or_si128(sl->zm, ti);
				break;
			default: 
				__assume(0);
			}
		}

		switch(iTFX)
		{
		case 0:
			c[0] = Saturate(Modulate(c[0], sl->r[0]));
			c[1] = Saturate(Modulate(c[1], sl->g[0]));
			c[2] = Saturate(Modulate(c[2], sl->b[0]));
			break;
		case 1:
			break;
		case 2:
			c[0] = Saturate(_mm_add_ps(Modulate(c[0], sl->r[0]), sl->a[0]));
			c[1] = Saturate(_mm_add_ps(Modulate(c[1], sl->g[0]), sl->a[0]));
			c[2] = Saturate(_mm_add_ps(Modulate(c[2], sl->b[0]), sl->a[0]));
			break;
		case 3:
			c[0] = Saturate(_mm_add_ps(Modulate(c[0], sl->r[0]), sl->a[0]));
			c[1] = Saturate(_mm_add_ps(Modulate(c[1], sl->g[0]), sl->a[0]));
			c[2] = Saturate(_mm_add_ps(Modulate(c[2], sl->b[0]), sl->a[0]));
			break;
		case 4:
			c[0] = sl->r[0];
			c[1] = sl->g[0];
			c[2] = sl->b[0];
			break;
		default:
			__assume(0);
		}

		if(bFGE)
		{
			c[0] = Blend(m_slenv->f.r, c[0], sl->f);
			c[1] = Blend(m_slenv->f.g, c[1], sl->f);
			c[2] = Blend(m_slenv->f.b, c[2], sl->f);
		}

		sl->r[0] = c[0];
		sl->g[0] = c[1];
		sl->b[0] = c[2];
		sl->a[0] = c[3];
	}
}

template<int iFPSM, int iZPSM, int bRFB, int bDATE, int iABE>
void GSRasterizer::DrawScanlineOM(int steps)
{
	int iALPHA_A = min(m_state->m_context->ALPHA.A, 2);
	int iALPHA_B = min(m_state->m_context->ALPHA.B, 2);
	int iALPHA_C = min(m_state->m_context->ALPHA.C, 2);
	int iALPHA_D = min(m_state->m_context->ALPHA.D, 2);

	int fpsm = GSLocalMemory::DecodeFPSM(iFPSM);
	int zpsm = GSLocalMemory::DecodeZPSM(iZPSM);

	__m128i datm = m_slenv->datm; 

	Scanline* sl = m_sl;

	for(int j = 0; j < steps; j += 4, sl++)
	{
		__m128i t = sl->t;

		if(_mm_movemask_epi8(t) == 0xffff)
		{
			continue;
		}

		__m128i d = _mm_setzero_si128();

		if(bRFB)
		{
			for(int i = 0, k = min(steps - j, 4); i < k; i++)
			{
				if(t.m128i_u32[i])
				{
					continue;
				}

				d.m128i_u32[i] = m_state->m_mem.readFrameX(fpsm, sl->fa.m128i_u32[i]);
			}
		}

		__m128i fm = sl->fm;
		__m128i zm = sl->zm;

		if(bDATE)
		{
			t = _mm_or_si128(t, _mm_srai_epi32(_mm_xor_si128(d, datm), 31));

			if(_mm_movemask_epi8(t) == 0xffff)
			{
				continue;
			}
		}

		fm = _mm_or_si128(fm, t);
		zm = _mm_or_si128(zm, t);

		__m128 r, g, b, a;

		if(iABE)
		{
			__m128i mask = _mm_set1_epi32(0xff);

			sl->r[1] = _mm_cvtepi32_ps(_mm_and_si128(d, mask));
			sl->g[1] = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(d, 8), mask));
			sl->b[1] = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(d, 16), mask));
			sl->a[1] = _mm_cvtepi32_ps(_mm_srli_epi32(d, 24));

			sl->r[2] = _mm_setzero_ps();
			sl->g[2] = _mm_setzero_ps();
			sl->b[2] = _mm_setzero_ps();
			sl->a[2] = m_slenv->afix;

			r = _mm_add_ps(Modulate(_mm_sub_ps(sl->r[iALPHA_A], sl->r[iALPHA_B]), sl->a[iALPHA_C]), sl->r[iALPHA_D]);
			g = _mm_add_ps(Modulate(_mm_sub_ps(sl->g[iALPHA_A], sl->g[iALPHA_B]), sl->a[iALPHA_C]), sl->g[iALPHA_D]);
			b = _mm_add_ps(Modulate(_mm_sub_ps(sl->b[iALPHA_A], sl->b[iALPHA_B]), sl->a[iALPHA_C]), sl->b[iALPHA_D]);
			a = sl->a[0];

			if(iABE == 2)
			{
				__m128 mask = _mm_cmpge_ps(a, _mm_set1_ps(128));

				#if _M_SSE >= 0x400

				r = _mm_blendv_ps(sl->r[0], r, mask);
				g = _mm_blendv_ps(sl->g[0], g, mask);
				b = _mm_blendv_ps(sl->b[0], b, mask);

				#else

				r = _mm_or_ps(_mm_andnot_ps(mask, sl->r[0]), _mm_and_ps(mask, r));
				g = _mm_or_ps(_mm_andnot_ps(mask, sl->g[0]), _mm_and_ps(mask, g));
				b = _mm_or_ps(_mm_andnot_ps(mask, sl->b[0]), _mm_and_ps(mask, b));

				#endif
			}
		}
		else
		{
			r = sl->r[0];
			g = sl->g[0];
			b = sl->b[0];
			a = sl->a[0];
		}

		_MM_TRANSPOSE4_PS(r, g, b, a);

		__m128i s01 = _mm_packs_epi32(_mm_cvtps_epi32(r), _mm_cvtps_epi32(g));
		__m128i s23 = _mm_packs_epi32(_mm_cvtps_epi32(b), _mm_cvtps_epi32(a));
		__m128i s = _mm_packus_epi16(s01, s23);

		s = _mm_or_si128(_mm_andnot_si128(fm, s), _mm_and_si128(fm, d));

		for(int i = 0, k = min(steps - j, 4); i < k; i++)
		{
			if(fm.m128i_u32[i] != 0xffffffff)
			{
				m_state->m_mem.writeFrameX(fpsm, sl->fa.m128i_u32[i], s.m128i_u32[i]);
			}

			if(zm.m128i_u32[i] != 0xffffffff)
			{
				m_state->m_mem.writePixelX(zpsm, sl->za.m128i_u32[i], sl->z.m128i_u32[i]);
			}
		}
	}
}
