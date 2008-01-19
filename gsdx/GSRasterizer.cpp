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

	m_slenv = (ScanlineEnv*)_aligned_malloc(sizeof(ScanlineEnv), 16);

	InvalidateTextureCache();

	// w00t :P

	#define InitDS_ABE(iFPSM, iZPSM, iZTST, iABE) \
		m_ds[iFPSM][iZPSM][iZTST][iABE] = &GSRasterizer::DrawScanline<iFPSM, iZPSM, iZTST, iABE>; \

	#define InitDS_ZTST(iFPSM, iZPSM, iZTST) \
		InitDS_ABE(iFPSM, iZPSM, iZTST, 0) \
		InitDS_ABE(iFPSM, iZPSM, iZTST, 1) \
		InitDS_ABE(iFPSM, iZPSM, iZTST, 2) \

	#define InitDS_ZPSM(iFPSM, iZPSM) \
		InitDS_ZTST(iFPSM, iZPSM, 0) \
		InitDS_ZTST(iFPSM, iZPSM, 1) \
		InitDS_ZTST(iFPSM, iZPSM, 2) \

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
	memset(m_page, ~0, sizeof(m_page));
}

// int iFPSM;
// int iZPSM;
// int iZTST;
int bZRW;
int iTFX, bTCC, bFST, bLTF;
int bFGE;
int iATST, iAFAIL;
int bRFB;
int bDATE;
// int iABE;
int iALPHA_A, iALPHA_B, iALPHA_C, iALPHA_D;

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
	int iZPSM = GSLocalMemory::EncodeZPSM(context->ZBUF.PSM);
	int iZTST = context->TEST.ZTE && context->TEST.ZTST > 0 ? context->TEST.ZTST - 1 : 0;
	bZRW = (context->DepthRead() || context->DepthWrite()) ? 1 : 0;
	iTFX = PRIM->TME ? (int)context->TEX0.TFX : 4;
	bTCC = 0;
	bFST = 0;
	bLTF = 0;
	bFGE = PRIM->FGE ? 1 : 0;
	iATST = context->TEST.ATE ? context->TEST.ATST : 1;
	iAFAIL = context->TEST.AFAIL;
	bRFB = m_state->PRIM->ABE || m_state->m_env.PABE.PABE || m_state->m_context->FRAME.FBMSK || m_state->m_context->TEST.ATE && m_state->m_context->TEST.ATST != 1 && m_state->m_context->TEST.AFAIL == 3;
	bDATE = context->TEST.DATE;
	int iABE = env.PABE.PABE ? 2 : PRIM->ABE ? 1 : 0; // AA1 && prim == line
	iALPHA_A = 0;
	iALPHA_B = 0;
	iALPHA_C = 0;
	iALPHA_D = 0;

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
		iALPHA_A = context->ALPHA.A;
		iALPHA_B = context->ALPHA.B;
		iALPHA_C = context->ALPHA.C;
		iALPHA_D = context->ALPHA.D;

		if(iALPHA_A == iALPHA_B) iALPHA_C = 0;
	}

	m_dsf = m_ds[iFPSM][iZPSM][iZTST][iABE];

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
		(this->*m_dsf)(p.y, p.x, p.x + 1, *v);
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

	SetupScanlineDelta(dscan);

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

				(this->*m_dsf)(top, left, right, scan);
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

	SetupScanlineDelta(dscan);

	for(; top < bottom; top++)
	{
		(this->*m_dsf)(top, left, right, edge);

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

void GSRasterizer::SetupScanlineDelta(const Vertex& dv)
{
	__m128 dz = _mm_shuffle_ps(dv.p, dv.p, _MM_SHUFFLE(2, 2, 2, 2));

	m_slenv->dz0123 = _mm_mul_ps(dz, _mm_set_ps(3, 2, 1, 0));	
	m_slenv->dz = _mm_mul_ps(dz, _mm_set1_ps(4));

	m_slenv->dt1 = dv.t;
	m_slenv->dt2 = dv.t * Scalar(2);
	m_slenv->dt3 = dv.t * Scalar(3);

	__m128 ds = dv.t * Scalar(4);
	__m128 dt = ds;
	__m128 df = ds;
	__m128 dq = ds;

	_MM_TRANSPOSE4_PS(ds, dt, df, dq);

	m_slenv->ds = ds;
	m_slenv->dt = dt;
	m_slenv->df = df;
	m_slenv->dq = dq;

	m_slenv->dc1 = dv.c;
	m_slenv->dc2 = dv.c * Scalar(2);
	m_slenv->dc3 = dv.c * Scalar(3);

	__m128 dr = dv.c * Scalar(4);
	__m128 dg = dr;
	__m128 db = dr;
	__m128 da = dr;

	_MM_TRANSPOSE4_PS(dr, dg, db, da);

	m_slenv->dr = dr;
	m_slenv->dg = dg;
	m_slenv->db = db;
	m_slenv->da = da;
}

template<int iFPSM, int iZPSM, int iZTST, int iABE>
void GSRasterizer::DrawScanline(int top, int left, int right, const Vertex& v)	
{
	int fpsm = GSLocalMemory::DecodeFPSM(iFPSM);
	int zpsm = GSLocalMemory::DecodeZPSM(iZPSM);

	__m128i fa_base = m_fbco->addr[top];
	int* fa_offset = &m_state->m_context->ftbl->rowOffset[top & 7][left];

	__m128i za_base = m_zbco->addr[top];
	int* za_offset = &m_state->m_context->ztbl->rowOffset[top & 7][left];

	__m128 z = _mm_add_ps(_mm_shuffle_ps(v.p, v.p, _MM_SHUFFLE(2, 2, 2, 2)), m_slenv->dz0123);
	__m128 dz = m_slenv->dz;

	__m128 s = v.t;
	__m128 t = _mm_add_ps(v.t, m_slenv->dt1);
	__m128 f = _mm_add_ps(v.t, m_slenv->dt2);
	__m128 q = _mm_add_ps(v.t, m_slenv->dt3);

	_MM_TRANSPOSE4_PS(s, t, f, q);

	__m128 ds = m_slenv->ds;
	__m128 dt = m_slenv->dt;
	__m128 df = m_slenv->df;
	__m128 dq = m_slenv->dq;

	__m128 r = v.c;
	__m128 g = _mm_add_ps(v.c, m_slenv->dc1);
	__m128 b = _mm_add_ps(v.c, m_slenv->dc2);
	__m128 a = _mm_add_ps(v.c, m_slenv->dc3);

	_MM_TRANSPOSE4_PS(r, g, b, a);

	__m128 dr = m_slenv->dr;
	__m128 dg = m_slenv->dg;
	__m128 db = m_slenv->db;
	__m128 da = m_slenv->da;

	__m128 aref = m_slenv->aref;
	__m128i datm = m_slenv->datm; 

#if _M_SSE >= 0x301
	__m128i upmask = _mm_set_epi32(0x80808003, 0x80808002, 0x80808001, 0x80808000);
#endif

	__m128 c[12];

	c[8] = _mm_setzero_ps();
	c[9] = _mm_setzero_ps();
	c[10] = _mm_setzero_ps();
	c[11] = m_slenv->afix;

	for(int j = 0, steps = right - left; j < steps; j += 4, 
		z = _mm_add_ps(z, dz),
		s = _mm_add_ps(s, ds),
		t = _mm_add_ps(t, dt),
		f = _mm_add_ps(f, df),
		q = _mm_add_ps(q, dq),
		r = _mm_add_ps(r, dr),
		g = _mm_add_ps(g, dg),
		b = _mm_add_ps(b, db),
		a = _mm_add_ps(a, da)
		)
	{
		__m128i fa = _mm_add_epi32(fa_base, _mm_loadu_si128((__m128i*)&fa_offset[j]));
		__m128i za = _mm_add_epi32(za_base, _mm_loadu_si128((__m128i*)&za_offset[j]));
		
		__m128i fm =  m_slenv->fm;
		__m128i zm =  m_slenv->zm;
		__m128i test = _mm_setzero_si128();
		
		__m128i zi = _mm_slli_epi32(_mm_cvttps_epi32(_mm_mul_ps(z, _mm_set1_ps(0.5f))), 1);

		if(iZTST)
		{
			DWORD zd0 = m_state->m_mem.readPixelX(zpsm, za.m128i_u32[0]);
			DWORD zd1 = m_state->m_mem.readPixelX(zpsm, za.m128i_u32[1]);
			DWORD zd2 = m_state->m_mem.readPixelX(zpsm, za.m128i_u32[2]);
			DWORD zd3 = m_state->m_mem.readPixelX(zpsm, za.m128i_u32[3]);

			__m128i zs = _mm_sub_epi32(zi, _mm_set1_epi32(0x80000000));
			__m128i zd = _mm_sub_epi32(_mm_set_epi32(zd3, zd2, zd1, zd0), _mm_set1_epi32(0x80000000));

			switch(iZTST)
			{
			case 1: test = _mm_cmplt_epi32(zs, zd); break; // ge
			case 2: test = _mm_cmpgt_epi32(zd, zs); break; // g
			default: __assume(0);
			}

			if(_mm_movemask_epi8(test) == 0xffff)
			{
				continue;
			}
		}

		if(iTFX < 4)
		{
			__m128 u = s;
			__m128 v = t;

			if(!bFST)
			{
				__m128 w = _mm_rcp_ps(q);

				u = _mm_mul_ps(u, w);
				v = _mm_mul_ps(v, w);
			}

			if(bLTF)
			{
				u = _mm_sub_ps(u, _mm_set1_ps(0.5f));
				v = _mm_sub_ps(v, _mm_set1_ps(0.5f));

				// hmmm
				__m128i ui = _mm_cvttps_epi32(u);//Vector(u).floor());
				__m128i vi = _mm_cvttps_epi32(v);//Vector(v).floor());
				__m128 uf = _mm_sub_ps(u, Vector(u).floor());//_mm_cvtepi32_ps(ui));
				__m128 vf = _mm_sub_ps(v, Vector(v).floor());//_mm_cvtepi32_ps(vi));
				__m128i uv = _mm_packs_epi32(ui, vi);
				__m128i uv0 = Wrap(uv);
				__m128i uv1 = Wrap(_mm_add_epi16(uv, _mm_set1_epi32(0x00010001)));

				for(int i = 0, k = min(steps - j, 4); i < k; i++)
				{
					if(iZTST && test.m128i_u32[i])
					{
						continue;
					}

					FetchTexel(uv0.m128i_u16[i], uv0.m128i_u16[i + 4]);
					FetchTexel(uv1.m128i_u16[i], uv0.m128i_u16[i + 4]);
					FetchTexel(uv0.m128i_u16[i], uv1.m128i_u16[i + 4]);
					FetchTexel(uv1.m128i_u16[i], uv1.m128i_u16[i + 4]);

					#if _M_SSE >= 0x301

					__m128 c00 = _mm_cvtepi32_ps(_mm_shuffle_epi8(_mm_cvtsi32_si128(ReadTexelNoFetch(uv0.m128i_u16[i], uv0.m128i_u16[i + 4])), upmask));
					__m128 c01 = _mm_cvtepi32_ps(_mm_shuffle_epi8(_mm_cvtsi32_si128(ReadTexelNoFetch(uv1.m128i_u16[i], uv0.m128i_u16[i + 4])), upmask));
					__m128 c10 = _mm_cvtepi32_ps(_mm_shuffle_epi8(_mm_cvtsi32_si128(ReadTexelNoFetch(uv0.m128i_u16[i], uv1.m128i_u16[i + 4])), upmask));
					__m128 c11 = _mm_cvtepi32_ps(_mm_shuffle_epi8(_mm_cvtsi32_si128(ReadTexelNoFetch(uv1.m128i_u16[i], uv1.m128i_u16[i + 4])), upmask));

					#else

					__m128 c00 = Unpack(ReadTexelNoFetch(uv0.m128i_u16[i], uv0.m128i_u16[i + 4]));
					__m128 c01 = Unpack(ReadTexelNoFetch(uv1.m128i_u16[i], uv0.m128i_u16[i + 4]));
					__m128 c10 = Unpack(ReadTexelNoFetch(uv0.m128i_u16[i], uv1.m128i_u16[i + 4]));
					__m128 c11 = Unpack(ReadTexelNoFetch(uv1.m128i_u16[i], uv1.m128i_u16[i + 4]));

					#endif

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

				__m128i c00;

				for(int i = 0, k = min(steps - j, 4); i < k; i++)
				{
					if(iZTST && test.m128i_u32[i])
					{
						continue;
					}

					c00.m128i_u32[i] = ReadTexel(uv0.m128i_u16[i], uv0.m128i_u16[i + 4]);
				}

				__m128i mask = _mm_set1_epi32(0xff);

				c[0] = _mm_cvtepi32_ps(_mm_and_si128(c00, mask));
				c[1] = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(c00, 8), mask));
				c[2] = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(c00, 16), mask));
				c[3] = _mm_cvtepi32_ps(_mm_srli_epi32(c00, 24));
			}
		}

		switch(iTFX)
		{
		case 0: c[3] = bTCC ? Saturate(Modulate(c[3], a)) : a; break;
		case 1: break;
		case 2: c[3] = bTCC ? Saturate(_mm_add_ps(c[3], a)) : a; break;
		case 3: if(!bTCC) c[3] = a; break;
		case 4: c[3] = a; break; 
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
				fm = _mm_or_si128(fm, ti);
				zm = _mm_or_si128(zm, ti);
				test = _mm_or_si128(test, ti);
				if(_mm_movemask_epi8(test) == 0xffff) continue;
				break;
			case 1:
				zm = _mm_or_si128(zm, ti);
				break;
			case 2:
				fm = _mm_or_si128(fm, ti);
				break;
			case 3: 
				fm = _mm_or_si128(fm, _mm_and_si128(ti, _mm_set1_epi32(0xff000000)));
				zm = _mm_or_si128(zm, ti);
				break;
			default: 
				__assume(0);
			}
		}

		switch(iTFX)
		{
		case 0:
			c[0] = Saturate(Modulate(c[0], r));
			c[1] = Saturate(Modulate(c[1], g));
			c[2] = Saturate(Modulate(c[2], b));
			break;
		case 1:
			break;
		case 2:
			c[0] = Saturate(_mm_add_ps(Modulate(c[0], r), a));
			c[1] = Saturate(_mm_add_ps(Modulate(c[1], g), a));
			c[2] = Saturate(_mm_add_ps(Modulate(c[2], b), a));
			break;
		case 3:
			c[0] = Saturate(_mm_add_ps(Modulate(c[0], r), a));
			c[1] = Saturate(_mm_add_ps(Modulate(c[1], g), a));
			c[2] = Saturate(_mm_add_ps(Modulate(c[2], b), a));
			break;
		case 4:
			c[0] = r;
			c[1] = g;
			c[2] = b;
			break;
		default:
			__assume(0);
		}

		if(bFGE)
		{
			c[0] = Blend(m_slenv->f.r, c[0], f);
			c[1] = Blend(m_slenv->f.g, c[1], f);
			c[2] = Blend(m_slenv->f.b, c[2], f);
		}

		__m128i d = _mm_setzero_si128();

		if(bRFB)
		{
			for(int i = 0, k = min(steps - j, 4); i < k; i++)
			{
				if(test.m128i_u32[i])
				{
					continue;
				}

				d.m128i_u32[i] = m_state->m_mem.readFrameX(fpsm, fa.m128i_u32[i]);
			}
		}

		if(bDATE)
		{
			test = _mm_or_si128(test, _mm_srai_epi32(_mm_xor_si128(d, datm), 31));

			if(_mm_movemask_epi8(test) == 0xffff)
			{
				continue;
			}
		}

		fm = _mm_or_si128(fm, test);
		zm = _mm_or_si128(zm, test);

		if(iABE)
		{
			__m128i mask = _mm_set1_epi32(0xff);

			c[4] = _mm_cvtepi32_ps(_mm_and_si128(d, mask));
			c[5] = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(d, 8), mask));
			c[6] = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(d, 16), mask));
			c[7] = _mm_cvtepi32_ps(_mm_srli_epi32(d, 24));

			__m128 r = _mm_add_ps(Modulate(_mm_sub_ps(c[iALPHA_A*4 + 0], c[iALPHA_B*4 + 0]), c[iALPHA_C*4 + 3]), c[iALPHA_D*4 + 0]);
			__m128 g = _mm_add_ps(Modulate(_mm_sub_ps(c[iALPHA_A*4 + 1], c[iALPHA_B*4 + 1]), c[iALPHA_C*4 + 3]), c[iALPHA_D*4 + 1]);
			__m128 b = _mm_add_ps(Modulate(_mm_sub_ps(c[iALPHA_A*4 + 2], c[iALPHA_B*4 + 2]), c[iALPHA_C*4 + 3]), c[iALPHA_D*4 + 2]);

			if(iABE == 2)
			{
				__m128 mask = _mm_cmpge_ps(c[3], _mm_set1_ps(128));

				#if _M_SSE >= 0x400

				c[0] = _mm_blendv_ps(c[0], r, mask);
				c[1] = _mm_blendv_ps(c[1], g, mask);
				c[2] = _mm_blendv_ps(c[2], b, mask);

				#else

				c[0] = _mm_or_ps(_mm_andnot_ps(mask, c[0]), _mm_and_ps(mask, r));
				c[1] = _mm_or_ps(_mm_andnot_ps(mask, c[1]), _mm_and_ps(mask, g));
				c[2] = _mm_or_ps(_mm_andnot_ps(mask, c[2]), _mm_and_ps(mask, b));

				#endif
			}
			else
			{
				c[0] = r;
				c[1] = g;
				c[2] = b;
			}
		}

		_MM_TRANSPOSE4_PS(c[0], c[1], c[2], c[3]);

		__m128i s01 = _mm_packs_epi32(_mm_cvtps_epi32(c[0]), _mm_cvtps_epi32(c[1]));
		__m128i s23 = _mm_packs_epi32(_mm_cvtps_epi32(c[2]), _mm_cvtps_epi32(c[3]));
		__m128i s = _mm_packus_epi16(s01, s23);

		s = _mm_or_si128(_mm_andnot_si128(fm, s), _mm_and_si128(fm, d));

		for(int i = 0, k = min(steps - j, 4); i < k; i++)
		{
			if(fm.m128i_u32[i] != 0xffffffff)
			{
				m_state->m_mem.writeFrameX(fpsm, fa.m128i_u32[i], s.m128i_u32[i]);
			}

			if(zm.m128i_u32[i] != 0xffffffff)
			{
				m_state->m_mem.writePixelX(zpsm, za.m128i_u32[i], zi.m128i_u32[i]);
			}
		}
	}
}

