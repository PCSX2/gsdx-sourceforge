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
#include "GSVertexHW.h"

template<class Device>
class GSRendererHW : public GSRendererT<Device, GSVertexHW>
{
protected:
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

public:
	GSRendererHW(BYTE* base, bool mt, void (*irq)(), int nloophack, int interlace, int aspectratio, int filter, bool vsync)
		: GSRendererT<Device, GSVertexHW>(base, mt, irq, nloophack, interlace, aspectratio, filter, vsync)
	{
	}
};
