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

#pragma once

#include "GSRenderer.h"
#include "GSTextureCache.h"
#include "GSVertexHW.h"

template<class Device>
class GSRendererHW : public GSRendererT<Device, GSVertexHW>
{
protected:
	GSTextureCache<Device>* m_tc; // derived class creates this
	int m_width;
	int m_height;
	int m_skip;

	void VertexKick(bool skip)
	{
		GSVertexHW& v = m_vl.AddTail();

		v.p.x = (float)m_v.XYZ.X;
		v.p.y = (float)m_v.XYZ.Y;
		v.p.z = (float)m_v.XYZ.Z;

		v.c0 = m_v.RGBAQ.ai32[0];
		v.c1 = m_v.FOG.ai32[1];

		if(PRIM->TME)
		{
			if(PRIM->FST)
			{
				v.p.w = 1.0f;
				v.t.x = (float)(int)m_v.UV.U;
				v.t.y = (float)(int)m_v.UV.V;
			}
			else
			{
				v.p.w = m_v.RGBAQ.Q;
				v.t.x = m_v.ST.S;
				v.t.y = m_v.ST.T;
			}
		}
		else
		{
			v.p.w = 1.0f;
			v.t.x = 0;
			v.t.y = 0;
		}

		__super::VertexKick(skip);
	}

	void MinMaxXY(GSVector4& mm)
	{
		#if _M_IX86_FP >= 2 || defined(_M_AMD64)
				
		__m128 min = _mm_set1_ps(+1e10);
		__m128 max = _mm_set1_ps(-1e10);

		int i = 0;

		for(int count = m_count - 5; i < count; i += 6) // 6 regs for loading, 2 regs for min/max
		{
			min = _mm_min_ps(m_vertices[i+0].m128[0], min);
			max = _mm_max_ps(m_vertices[i+0].m128[0], max);
			min = _mm_min_ps(m_vertices[i+1].m128[0], min);
			max = _mm_max_ps(m_vertices[i+1].m128[0], max);
			min = _mm_min_ps(m_vertices[i+2].m128[0], min);
			max = _mm_max_ps(m_vertices[i+2].m128[0], max);
			min = _mm_min_ps(m_vertices[i+3].m128[0], min);
			max = _mm_max_ps(m_vertices[i+3].m128[0], max);
			min = _mm_min_ps(m_vertices[i+4].m128[0], min);
			max = _mm_max_ps(m_vertices[i+4].m128[0], max);
			min = _mm_min_ps(m_vertices[i+5].m128[0], min);
			max = _mm_max_ps(m_vertices[i+5].m128[0], max);
		}

		for(; i < m_count; i++)
		{
			min = _mm_min_ps(m_vertices[i+0].m128[0], min);
			max = _mm_max_ps(m_vertices[i+0].m128[0], max);
		}

		mm.x = min.m128_f32[0];
		mm.y = min.m128_f32[1];
		mm.z = max.m128_f32[0];
		mm.w = max.m128_f32[1];

		#else	

		mm.x = mm.y = +1e10;
		mm.z = mm.w = -1e10;

		for(int i = 0, j = m_count; i < j; i++)
		{
			float x = m_vertices[i].p.x;

			if(x < mm.x) mm.x = x;
			if(x > mm.z) mm.z = x;
			
			float y = m_vertices[i].p.y;

			if(y < mm.y) mm.y = y;
			if(y > mm.w) mm.w = y;
		}

		#endif
	}

	void MinMaxUV(GSVector4& mm)
	{
		if(PRIM->FST)
		{
			#if defined(_M_AMD64) || _M_IX86_FP >= 2

			__m128 min = _mm_set1_ps(+1e10);
			__m128 max = _mm_set1_ps(-1e10);

			int i = 0;

			for(int count = m_count - 5; i < count; i += 6) // 6 regs for loading, 2 regs for min/max
			{
				min = _mm_min_ps(m_vertices[i+0].m128[1], min);
				max = _mm_max_ps(m_vertices[i+0].m128[1], max);
				min = _mm_min_ps(m_vertices[i+1].m128[1], min);
				max = _mm_max_ps(m_vertices[i+1].m128[1], max);
				min = _mm_min_ps(m_vertices[i+2].m128[1], min);
				max = _mm_max_ps(m_vertices[i+2].m128[1], max);
				min = _mm_min_ps(m_vertices[i+3].m128[1], min);
				max = _mm_max_ps(m_vertices[i+3].m128[1], max);
				min = _mm_min_ps(m_vertices[i+4].m128[1], min);
				max = _mm_max_ps(m_vertices[i+4].m128[1], max);
				min = _mm_min_ps(m_vertices[i+5].m128[1], min);
				max = _mm_max_ps(m_vertices[i+5].m128[1], max);
			}

			for(; i < m_count; i++)
			{
				min = _mm_min_ps(m_vertices[i+0].m128[1], min);
				max = _mm_max_ps(m_vertices[i+0].m128[1], max);
			}

			mm.x = min.m128_f32[0];
			mm.y = min.m128_f32[1];
			mm.z = max.m128_f32[0];
			mm.w = max.m128_f32[1];

			#else

			for(int i = 0, j = m_count; i < j; i++)
			{
				float x = m_vertices[i].t.x;

				if(x < mm.x) mm.x = x;
				if(x > mm.z) mm.z = x;
				
				float y = m_vertices[i].t.y;

				if(y < mm.y) mm.y = y;
				if(y > mm.w) mm.w = y;
			}

			#endif

			mm.x *= 1.0f / (16 << m_context->TEX0.TW);
			mm.y *= 1.0f / (16 << m_context->TEX0.TH);
			mm.z *= 1.0f / (16 << m_context->TEX0.TW);
			mm.w *= 1.0f / (16 << m_context->TEX0.TH);
		}
		else
		{
			// TODO: sse

			mm.x = mm.y = +1e10;
			mm.z = mm.w = -1e10;

			for(int i = 0, j = m_count; i < j; i++)
			{
				float w = 1.0f / m_vertices[i].p.w;

				float x = m_vertices[i].t.x * w;

				if(x < mm.x) mm.x = x;
				if(x > mm.z) mm.z = x;
				
				float y = m_vertices[i].t.y * w;

				if(y < mm.y) mm.y = y;
				if(y > mm.w) mm.w = y;
			}
		}
	}

	void MinMaxUV(int w, int h, CRect& r)
	{
		r.SetRect(0, 0, w, h);

		GSVector4 mm(0, 0, 1, 1);

		if((m_context->CLAMP.WMS < 3 || m_context->CLAMP.WMT < 3) && m_count < 100)
		{
			MinMaxUV(mm);
		}

		CSize bs = GSLocalMemory::m_psm[m_context->TEX0.PSM].bs;
		CSize bsm(bs.cx - 1, bs.cy - 1);

		if(m_context->CLAMP.WMS != 3)
		{
			if(m_context->CLAMP.WMS == 0)
			{
				float fmin = floor(mm.x);
				float fmax = floor(mm.z);

				if(fmin != fmax) {mm.x = 0; mm.z = 1.0f;}
				else {mm.x -= fmin; mm.z -= fmax;}

				// FIXME: 
				if(mm.x == 0 && mm.z != 1.0f) mm.z = 1.0f;
			}
			else if(m_context->CLAMP.WMS == 1)
			{
				if(mm.x < 0) mm.x = 0;
				else if(mm.x > 1.0f) mm.x = 1.0f;
				if(mm.z < 0) mm.z = 0;
				else if(mm.z > 1.0f) mm.z = 1.0f;
				if(mm.x > mm.z) mm.x = mm.z;
			}
			else if(m_context->CLAMP.WMS == 2)
			{
				float minu = 1.0f * m_context->CLAMP.MINU / w;
				float maxu = 1.0f * m_context->CLAMP.MAXU / w;
				if(mm.x < minu) mm.x = minu;
				else if(mm.x > maxu) mm.x = maxu;
				if(mm.z < minu) mm.z = minu;
				else if(mm.z > maxu) mm.z = maxu;
				if(mm.x > mm.z) mm.x = mm.z;
			}

			r.left = (int)(mm.x * w);
			r.right = (int)(mm.z * w);
		}
		else
		{
			#ifdef SW_REGION_REPEAT
			r.left = 0;
			r.right = w;
			#else
			r.left = (int)(m_context->CLAMP.MAXU);
			r.right = (int)(r.left + (m_context->CLAMP.MINU + 1));
			#endif
		}

		r.left = max(r.left & ~bsm.cx, 0);
		r.right = min((r.right + bsm.cx + 1) & ~bsm.cx, w);

		if(m_context->CLAMP.WMT != 3)
		{
			if(m_context->CLAMP.WMT == 0)
			{
				float fmin = floor(mm.y);
				float fmax = floor(mm.w);

				if(fmin != fmax) {mm.y = 0; mm.w = 1.0f;}
				else {mm.y -= fmin; mm.w -= fmax;}

				// FIXME: 
				if(mm.y == 0 && mm.w != 1.0f) mm.w = 1.0f;
			}
			else if(m_context->CLAMP.WMT == 1)
			{
				if(mm.y < 0) mm.y = 0;
				else if(mm.y > 1.0f) mm.y = 1.0f;
				if(mm.w < 0) mm.w = 0;
				else if(mm.w > 1.0f) mm.w = 1.0f;
				if(mm.y > mm.w) mm.y = mm.w;
			}
			else if(m_context->CLAMP.WMT == 2)
			{
				float minv = 1.0f * m_context->CLAMP.MINV / h;
				float maxv = 1.0f * m_context->CLAMP.MAXV / h;
				if(mm.y < minv) mm.y = minv;
				else if(mm.y > maxv) mm.y = maxv;
				if(mm.w < minv) mm.w = minv;
				else if(mm.w > maxv) mm.w = maxv;
				if(mm.y > mm.w) mm.y = mm.w;
			}

			r.top = (int)(mm.y * h);
			r.bottom = (int)(mm.w * h);
		}
		else
		{
			#ifdef SW_REGION_REPEAT
			r.top = 0;
			r.bottom = h;
			#else
			r.top = (int)(m_context->CLAMP.MAXV);
			r.bottom = (int)(r.top + (m_context->CLAMP.MINV + 1));
			#endif
		}

		r.top = max(r.top & ~bsm.cy, 0);
		r.bottom = min((r.bottom + bsm.cy + 1) & ~bsm.cy, h);
	}

	void VSync(int field)
	{
		__super::VSync(field);

		m_tc->IncAge();

		m_skip = 0;

		// s_dump = m_perfmon.GetFrame() >= 5002;
	}

	void ResetDevice() 
	{
		m_tc->RemoveAll();
	}

	bool GetOutput(int i, Texture& t, GSVector2& s)
	{
		GIFRegTEX0 TEX0;

		TEX0.TBP0 = DISPFB[i]->Block();
		TEX0.TBW = DISPFB[i]->FBW;
		TEX0.PSM = DISPFB[i]->PSM;

		if(GSTextureCache<Device>::GSRenderTarget* rt = m_tc->GetRenderTarget(TEX0, m_width, m_height, true))
		{
			t = rt->m_texture;
			s = rt->m_scale;
/*
			if(s_dump)
			{
				CString str;
				str.Format(_T("c:\\temp2\\_%05d_f%I64d_fr%d_%05x_%d.bmp"), s_n++, m_perfmon.GetFrame(), i, (int)TEX0.TBP0, (int)TEX0.PSM);
				if(s_save) rt->m_texture.Save(str);
			}
*/
			return true;
		}

		return false;
	}

	void InvalidateVideoMem(const GIFRegBITBLTBUF& BITBLTBUF, CRect r)
	{
		TRACE(_T("[%d] InvalidateVideoMem %d,%d - %d,%d %05x (%d)\n"), (int)m_perfmon.GetFrame(), r.left, r.top, r.right, r.bottom, (int)BITBLTBUF.DBP, (int)BITBLTBUF.DPSM);

		m_tc->InvalidateVideoMem(BITBLTBUF, &r);
	}

	void InvalidateLocalMem(const GIFRegBITBLTBUF& BITBLTBUF, CRect r)
	{
		TRACE(_T("[%d] InvalidateLocalMem %d,%d - %d,%d %05x (%d)\n"), (int)m_perfmon.GetFrame(), r.left, r.top, r.right, r.bottom, (int)BITBLTBUF.SBP, (int)BITBLTBUF.SPSM);

		m_tc->InvalidateLocalMem(BITBLTBUF, &r);
	}

public:
	GSRendererHW(BYTE* base, bool mt, void (*irq)(), int nloophack, int interlace, int aspectratio, int filter, bool vsync)
		: GSRendererT<Device, GSVertexHW>(base, mt, irq, nloophack, interlace, aspectratio, filter, vsync)
		, m_width(1024)
		, m_height(1024)
		, m_skip(0)
	{
	}

	virtual ~GSRendererHW()
	{
		delete m_tc;
	}
};
