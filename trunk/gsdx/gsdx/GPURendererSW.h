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

#include "GPURenderer.h"
#include "GPUTextureCacheSW.h"
#include "GPUDrawScanline.h"

template <class Device>
class GPURendererSW : public GPURenderer<Device, GSVertexSW>
{
	typedef GSVertexSW Vertex;

protected:
	long m_threads;
	GPUTextureCacheSW* m_tc;
	GPUDrawScanline* m_ds;
	GSRasterizer* m_rst;
	Texture m_texture;

	void Reset() 
	{
		m_tc->Invalidate(CRect(0, 0, 1024, 512));

		__super::Reset();
	}

	void ResetDevice() 
	{
		m_texture = Texture();
	}

	bool GetOutput(Texture& t)
	{
		CRect r = m_env.GetDisplayRect();

		if(m_texture.GetWidth() != r.Width() || m_texture.GetHeight() != r.Height())
		{
			m_texture = Texture();
		}

		if(!m_texture && !m_dev.CreateTexture(m_texture, r.Width(), r.Height())) 
		{
			return false;
		}

		// TODO
		static DWORD* buff = (DWORD*)_aligned_malloc(1024 * 512 * 4, 16);
		static int pitch = 1024 * 4;

		if(m_env.STATUS.ISRGB24)
		{
			DWORD* dst = buff;

			for(int i = r.top; i < r.bottom; i++, dst += 1024)
			{
				m_mem.Expand24(&m_mem.m_vm16[(i << 10) + r.left], dst, r.Width());
			}
		}
		else
		{
			DWORD* dst = buff;

			for(int i = r.top; i < r.bottom; i++, dst += 1024)
			{
				m_mem.Expand16(&m_mem.m_vm16[(i << 10) + r.left], dst, r.Width());
			}
		}

		r.OffsetRect(-r.TopLeft());

		m_texture.Update(r, buff, pitch);

		t = m_texture;

		return true;
	}

	void VertexKick()
	{
		Vertex& v = m_vl.AddTail();

		// TODO: x/y + off.x/y should wrap around at +/-1024

		int x = m_v.XY.X + m_env.DROFF.X;
		int y = m_v.XY.Y + m_env.DROFF.Y;

		int s = m_v.UV.X;
		int t = m_v.UV.Y;

		GSVector4 pt(x, y, s, t);

		v.p = pt.xyxy(GSVector4::zero());
		v.t = pt.zwzw(GSVector4::zero()) + GSVector4(0.5f);
		v.c = GSVector4((DWORD)m_v.RGB.ai32);

		__super::VertexKick();
	}

	void DrawingKickTriangle(Vertex* v, int& count)
	{
		// TODO
	}

	void DrawingKickLine(Vertex* v, int& count)
	{
		// TODO
	}

	void DrawingKickSprite(Vertex* v, int& count)
	{
		// TODO
	}

	void Draw()
	{
		const void* texture = NULL;

		if(m_env.PRIM.TME)
		{
			texture = m_tc->Lookup(m_env.STATUS);

			if(!texture) {ASSERT(0); return;}
		}

		//

		m_ds->SetOptions(m_filter, m_dither);

		m_ds->SetupDraw(m_vertices, m_count, texture);

		//

		GSVector4i scissor;

		scissor.x = m_env.DRAREATL.X;
		scissor.y = m_env.DRAREATL.Y;
		scissor.z = min(m_env.DRAREABR.X + 1, 1024);
		scissor.w = min(m_env.DRAREABR.Y + 1, 512);

		//

		int prims = 0;

		switch(m_env.PRIM.TYPE)
		{
		case GPU_POLYGON:
			ASSERT(!(m_count % 3));
			prims = m_count / 3;
			for(int i = 0, j = m_count; i < j; i += 3) m_rst->DrawTriangle(&m_vertices[i], scissor);
			break;
		case GPU_LINE:
			ASSERT(!(m_count & 1));
			prims = m_count / 2;
			for(int i = 0, j = m_count; i < j; i += 2) m_rst->DrawLine(&m_vertices[i], scissor);
			break;
		case GPU_SPRITE:
			ASSERT(!(m_count & 1));
			prims = m_count / 2;
			for(int i = 0, j = m_count; i < j; i += 2) m_rst->DrawSprite(&m_vertices[i], scissor, false);
			break;
		default:
			__assume(0);
		}

		m_perfmon.Put(GSPerfMon::Prim, prims);
		m_perfmon.Put(GSPerfMon::Draw, 1);

		{
			int pixels = m_rst->GetPixels();
			
			m_perfmon.Put(GSPerfMon::Fillrate, pixels); 
		}

		// TODO

		{
			GSVector4 tl(+1e10f);
			GSVector4 br(-1e10f);

			for(int i = 0, j = m_count; i < j; i++)
			{
				GSVector4 p = m_vertices[i].p;

				tl = tl.minv(p);
				br = br.maxv(p);
			}

			CRect r;

			r.left = max(scissor.x, min(scissor.z, (int)tl.x));
			r.top = max(scissor.y, min(scissor.w, (int)tl.y));
			r.right = max(scissor.x, min(scissor.z, (int)br.x));
			r.bottom = max(scissor.y, min(scissor.w, (int)br.y));

			Invalidate(r);
		}
	}

	void Invalidate(const CRect& r)
	{
		__super::Invalidate(r);

		m_tc->Invalidate(r);
	}

public:
	GPURendererSW(const GPURendererSettings& rs)
		: GPURenderer(rs)
	{
		m_threads = 1;

		m_tc = new GPUTextureCacheSW(this);
		m_ds = new GPUDrawScanline(this, m_filter, m_dither);
		m_rst = new GSRasterizer(m_ds, 0, m_threads);

		m_fpDrawingKickHandlers[GPU_POLYGON] = (DrawingKickHandler)&GPURendererSW::DrawingKickTriangle;
		m_fpDrawingKickHandlers[GPU_LINE] = (DrawingKickHandler)&GPURendererSW::DrawingKickLine;
		m_fpDrawingKickHandlers[GPU_SPRITE] = (DrawingKickHandler)&GPURendererSW::DrawingKickSprite;
	}

	virtual ~GPURendererSW()
	{
		delete m_tc;
		delete m_ds;
		delete m_rst;
	}
};
