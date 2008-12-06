/* 
 *	Copyright (C) 2007-2009 Gabest
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

GSRasterizer::GSRasterizer(IDrawScanline* ds, int id, int threads)
	: m_ds(ds)
	, m_id(id)
	, m_threads(threads)
	, m_pixels(0)
{
}

GSRasterizer::~GSRasterizer()
{
	delete m_ds;
}

void GSRasterizer::DrawPoint(Vertex* v, const GSVector4i& scissor)
{
	// TODO: round to closest for point, prestep for line

	GSVector4i p(v->p);

	if(scissor.x <= p.x && p.x < scissor.z && scissor.y <= p.y && p.y < scissor.w)
	{
		if((p.y % m_threads) == m_id) 
		{
			m_ds->DrawScanline(p.y, p.x, p.x + 1, *v);

			m_pixels++;
		}
	}
}

void GSRasterizer::DrawLine(Vertex* v, const GSVector4i& scissor)
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
		DrawPoint(&edge, scissor);

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

void GSRasterizer::DrawTriangle(Vertex* vertices, const GSVector4i& scissor)
{
	Vertex v[3];

	GSVector4 aabb = vertices[0].p.yyyy(vertices[1].p);
	GSVector4 bccb = vertices[1].p.yyyy(vertices[2].p).xzzx();

	int i = (aabb > bccb).mask() & 7;

	v[0] = vertices[s_abc[i][0]];
	v[1] = vertices[s_abc[i][1]];
	v[2] = vertices[s_abc[i][2]];

	aabb = v[0].p.yyyy(v[1].p);
	bccb = v[1].p.yyyy(v[2].p).xzzx();

	i = (aabb == bccb).mask() & 7;

	switch(i)
	{
	case 0: // a < b < c
		DrawTriangleTopBottom(v, scissor);
		break;
	case 1: // a == b < c
		DrawTriangleBottom(v, scissor);
		break;
	case 4: // a < b == c
		DrawTriangleTop(v, scissor);
		break;
	case 7: // a == b == c
		break;
	default:
		__assume(0);
	}
}

void GSRasterizer::DrawTriangleTop(Vertex* v, const GSVector4i& scissor)
{
	Vertex longest = v[2] - v[1];
	
	if((longest.p == GSVector4::zero()).mask() & 1)
	{
		return;
	}

	Vertex dscan = longest * longest.p.xxxx().rcp();

	int i = (longest.p > GSVector4::zero()).mask() & 1;

	Vertex& l = v[0];
	GSVector4 r = v[0].p;

	Vertex vl = v[2 - i] - l;
	GSVector4 vr = v[1 + i].p - r;

	Vertex dl = vl / vl.p.yyyy();
	GSVector4 dr = vr / vr.yyyy();

	GSVector4i tb(l.p.xyxy(v[2].p).ceil());

	int top = tb.y;
	int bottom = tb.w;

	if(top < scissor.y) top = scissor.y;
	if(bottom > scissor.w) bottom = scissor.w;
			
	if(top < bottom)
	{
		float py = (float)top - l.p.y;

		if(py > 0)
		{
			GSVector4 dy(py);

			l += dl * dy;
			r += dr * dy;
		}

		m_ds->SetupPrim(IDrawScanline::Triangle, v, dscan);

		DrawTriangleSection(top, bottom, l, dl, r, dr, dscan, scissor);
	}
}

void GSRasterizer::DrawTriangleBottom(Vertex* v, const GSVector4i& scissor)
{
	Vertex longest = v[1] - v[0];
	
	if((longest.p == GSVector4::zero()).mask() & 1)
	{
		return;
	}
	
	Vertex dscan = longest * longest.p.xxxx().rcp();

	int i = (longest.p > GSVector4::zero()).mask() & 1;

	Vertex& l = v[1 - i];
	GSVector4& r = v[i].p;

	Vertex vl = v[2] - l;
	GSVector4 vr = v[2].p - r;

	Vertex dl = vl / vl.p.yyyy();
	GSVector4 dr = vr / vr.yyyy();

	GSVector4i tb(l.p.xyxy(v[2].p).ceil());

	int top = tb.y;
	int bottom = tb.w;

	if(top < scissor.y) top = scissor.y;
	if(bottom > scissor.w) bottom = scissor.w;
	
	if(top < bottom)
	{
		float py = (float)top - l.p.y;

		if(py > 0)
		{
			GSVector4 dy(py);

			l += dl * dy;
			r += dr * dy;
		}
		
		m_ds->SetupPrim(IDrawScanline::Triangle, v, dscan);

		DrawTriangleSection(top, bottom, l, dl, r, dr, dscan, scissor);
	}
}

void GSRasterizer::DrawTriangleTopBottom(Vertex* v, const GSVector4i& scissor)
{
	Vertex v01, v02, v12;

	v01 = v[1] - v[0];
	v02 = v[2] - v[0];

	Vertex longest = v[0] + v02 * (v01.p / v02.p).yyyy() - v[1];

	if((longest.p == GSVector4::zero()).mask() & 1)
	{
		return;
	}

	Vertex dscan = longest * longest.p.xxxx().rcp();

	m_ds->SetupPrim(IDrawScanline::Triangle, v, dscan);

	Vertex& l = v[0];
	GSVector4 r = v[0].p;

	Vertex dl;
	GSVector4 dr;

	bool b = (longest.p > GSVector4::zero()).mask() & 1;
	
	if(b)
	{
		dl = v01 / v01.p.yyyy();
		dr = v02.p / v02.p.yyyy();
	}
	else
	{
		dl = v02 / v02.p.yyyy();
		dr = v01.p / v01.p.yyyy();
	}

	GSVector4i tb(v[0].p.yyyy(v[1].p).xzyy(v[2].p).ceil());

	int top = tb.x;
	int bottom = tb.y;

	if(top < scissor.y) top = scissor.y;
	if(bottom > scissor.w) bottom = scissor.w;

	float py = (float)top - l.p.y;

	if(py > 0)
	{
		GSVector4 dy(py);

		l += dl * dy;
		r += dr * dy;
	}

	if(top < bottom)
	{
		DrawTriangleSection(top, bottom, l, dl, r, dr, dscan, scissor);
	}

	if(b)
	{
		v12 = v[2] - v[1];

		l = v[1];

		dl = v12 / v12.p.yyyy();
	}
	else
	{
		v12.p = v[2].p - v[1].p;

		r = v[1].p;

		dr = v12.p / v12.p.yyyy();
	}

	top = tb.y;
	bottom = tb.z;

	if(top < scissor.y) top = scissor.y;
	if(bottom > scissor.w) bottom = scissor.w;

	if(top < bottom)
	{
		py = (float)top - l.p.y;

		if(py > 0) l += dl * py;

		py = (float)top - r.y;

		if(py > 0) r += dr * py;

		DrawTriangleSection(top, bottom, l, dl, r, dr, dscan, scissor);
	}
}

void GSRasterizer::DrawTriangleSection(int top, int bottom, Vertex& l, const Vertex& dl, GSVector4& r, const GSVector4& dr, const Vertex& dscan, const GSVector4i& scissor)
{
	ASSERT(top < bottom);

	IDrawScanline::DrawScanlinePtr dsf = m_ds->GetDrawScanlinePtr();

	while(1)
	{
		do
		{
			// rarely used (character shadows in ffx-2)

			/*

			int scanmsk = (int)m_state->m_env.SCANMSK.MSK - 2;

			if(scanmsk >= 0)
			{
				if(((top & 1) ^ scanmsk) == 0)
				{
					continue;
				}
			}

			*/

			if((top % m_threads) == m_id) 
			{
				GSVector4i lr(l.p.xyxy(r).ceil());

				int& left = lr.x;
				int& right = lr.z;

				if(left < scissor.x) left = scissor.x;
				if(right > scissor.z) right = scissor.z;

				int pixels = right - left;

				if(pixels > 0)
				{
					m_pixels += pixels;

					Vertex scan = l;

					float px = (float)left - l.p.x;

					if(px > 0) scan += dscan * px;

					(m_ds->*dsf)(top, left, right, scan);
				}
			}
		}
		while(0);

		if(++top >= bottom) break;

		l += dl;
		r += dr;
	}
}

void GSRasterizer::DrawSprite(Vertex* vertices, const GSVector4i& scissor, bool solid)
{
	Vertex v[2];

	GSVector4 mask = vertices[0].p > vertices[1].p;

	v[0].p = vertices[0].p.blend8(vertices[1].p, mask).xyzw(vertices[1].p);
	v[0].t = vertices[0].t.blend8(vertices[1].t, mask).xyzw(vertices[1].t);
	v[0].c = vertices[1].c;

	v[1].p = vertices[1].p.blend8(vertices[0].p, mask);
	v[1].t = vertices[1].t.blend8(vertices[0].t, mask);

	GSVector4i r(v[0].p.xyxy(v[1].p).ceil());

	int& top = r.y;
	int& bottom = r.w;

	int& left = r.x;
	int& right = r.z;

	#if _M_SSE >= 0x401

	r = r.sat_i32(scissor);
	
	if((r < r.zwzw()).mask() != 0x00ff) return;

	#else

	if(top < scissor.y) top = scissor.y;
	if(bottom > scissor.w) bottom = scissor.w;
	if(top >= bottom) return;

	if(left < scissor.x) left = scissor.x;
	if(right > scissor.z) right = scissor.z;
	if(left >= right) return;

	#endif

	Vertex scan = v[0];

	if(solid)
	{
		if(m_id == 0)
		{
			m_ds->FillRect(r, scan);
		}

		return;
	}

	GSVector4 zero = GSVector4::zero();

	Vertex dedge, dscan;

	dedge.p = zero;
	dscan.p = zero;

	Vertex dv = v[1] - v[0];

	dedge.t = (dv.t / dv.p.yyyy()).xyxy(zero).wyww();
	dscan.t = (dv.t / dv.p.xxxx()).xyxy(zero).xwww();

	if(scan.p.y < (float)top) scan.t += dedge.t * ((float)top - scan.p.y);
	if(scan.p.x < (float)left) scan.t += dscan.t * ((float)left - scan.p.x);

	m_ds->SetupPrim(IDrawScanline::Sprite, v, dscan);

	IDrawScanline::DrawScanlinePtr dsf = m_ds->GetDrawScanlinePtr();

	for(; top < bottom; top++, scan.t += dedge.t)
	{
		if((top % m_threads) == m_id) 
		{
			(m_ds->*dsf)(top, left, right, scan);

			m_pixels += right - left;
		}
	}
}

//

GSRasterizerMT::GSRasterizerMT(IDrawScanline* ds, int id, int threads, IDrawAsync* da, long* sync)
	: GSRasterizer(ds, id, threads)
	, m_da(da)
	, m_sync(sync)
	, m_exit(false)
	, m_ThreadId(0)
	, m_hThread(NULL)
{
	if(id > 0)
	{
		m_hThread = CreateThread(NULL, 0, StaticThreadProc, (LPVOID)this, 0, &m_ThreadId);
	}
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

int GSRasterizerMT::Draw(Vertex* vertices, int count, const void* texture)
{
	m_ds->SetupDraw(vertices, count, texture);

	int prims = 0;

	if(m_id == 0)
	{
		prims = m_da->DrawAsync(this);
	}
	else
	{
		InterlockedBitTestAndSet(m_sync, m_id);
	}

	return prims;
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
			m_da->DrawAsync(this);

			InterlockedBitTestAndReset(m_sync, m_id);
		}
		else
		{
			_mm_pause();
		}
	}

	return 0;
}
