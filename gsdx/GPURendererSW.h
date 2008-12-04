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
#include "GPUDrawScanline.h"

template <class Device>
class GPURendererSW : public GPURenderer<Device, GSVertexSW>, public IDrawAsync
{
	typedef GSVertexSW Vertex;

protected:
	long* m_sync;
	long m_threads;
	GSRasterizer* m_rst;
	CAtlList<GSRasterizerMT*> m_rmt;
	Texture m_texture;

	void ResetDevice() 
	{
		m_texture = Texture();
	}

	bool GetOutput(Texture& t)
	{
		CRect r = m_env.GetDisplayRect();

		r.left <<= m_scale.cx;
		r.top <<= m_scale.cy;
		r.right <<= m_scale.cx;
		r.bottom <<= m_scale.cy;

		// TODO
		static DWORD* buff = (DWORD*)_aligned_malloc(m_mem.GetWidth() * m_mem.GetHeight() * sizeof(DWORD), 16);

		m_mem.ReadFrame32(r, buff, !!m_env.STATUS.ISRGB24);

		r.OffsetRect(-r.TopLeft());

		if(m_texture.GetWidth() != r.Width() || m_texture.GetHeight() != r.Height())
		{
			m_texture = Texture();
		}

		if(!m_texture && !m_dev.CreateTexture(m_texture, r.Width(), r.Height())) 
		{
			return false;
		}

		m_texture.Update(r, buff, m_mem.GetWidth() * sizeof(DWORD));

		t = m_texture;

		return true;
	}

	void VertexKick()
	{
		Vertex& v = m_vl.AddTail();

		// TODO: x/y + off.x/y should wrap around at +/-1024

		int x = (int)(m_v.XY.X + m_env.DROFF.X) << m_scale.cx;
		int y = (int)(m_v.XY.Y + m_env.DROFF.Y) << m_scale.cy;

		int s = m_v.UV.X;
		int t = m_v.UV.Y;

		GSVector4 pt(x, y, s, t);

		v.p = pt.xyxy(GSVector4::zero());
		v.t = (pt.zwzw(GSVector4::zero()) + GSVector4(0.125f)) * 256.0f;
		v.c = GSVector4((DWORD)m_v.RGB.ai32) * 128.0f;

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

	GSVector4i GetScissor()
	{
		GSVector4i v;

		v.x = (int)m_env.DRAREATL.X << m_scale.cx;
		v.y = (int)m_env.DRAREATL.Y << m_scale.cy;
		v.z = min((int)(m_env.DRAREABR.X + 1) << m_scale.cx, m_mem.GetWidth());
		v.w = min((int)(m_env.DRAREABR.Y + 1) << m_scale.cy, m_mem.GetHeight());

		return v;
	}

	void Draw()
	{
		const void* texture = NULL;

		if(m_env.PRIM.TME)
		{
			texture = m_mem.GetTexture(m_env.STATUS.TP, m_env.STATUS.TX, m_env.STATUS.TY);

			if(!texture) {ASSERT(0); return;}
		}

		//

		GPUDrawScanline* ds = (GPUDrawScanline*)m_rst->GetDrawScanline();

		ds->SetOptions(m_filter, m_dither);
		ds->SetupDraw(m_vertices, m_count, texture);

		//

		*m_sync = 0;

		POSITION pos = m_rmt.GetHeadPosition();

		while(pos)
		{
			GSRasterizerMT* r = m_rmt.GetNext(pos);

			GPUDrawScanline* ds = (GPUDrawScanline*)r->GetDrawScanline();

			ds->SetOptions(m_filter, m_dither);
			ds->SetupDraw(m_vertices, m_count, texture);

			r->Draw();
		}

		// 1st thread is this thread

		int prims = DrawAsync(m_rst);

		// wait for the other threads to finish

		while(*m_sync)
		{
			_mm_pause();
		}
		
		m_perfmon.Put(GSPerfMon::Prim, prims);
		m_perfmon.Put(GSPerfMon::Draw, 1);
		
		{
			int pixels = m_rst->GetPixels();
			
			POSITION pos = m_rmt.GetHeadPosition();

			while(pos)
			{
				pixels += m_rmt.GetNext(pos)->GetPixels();
			}

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

			GSVector4i scissor = GetScissor();

			CRect r;

			r.left = max(scissor.x, min(scissor.z, (int)tl.x)) >> m_scale.cx;
			r.top = max(scissor.y, min(scissor.w, (int)tl.y)) >> m_scale.cy;
			r.right = max(scissor.x, min(scissor.z, (int)br.x)) >> m_scale.cx;
			r.bottom = max(scissor.y, min(scissor.w, (int)br.y)) >> m_scale.cy;

			Invalidate(r);
		}
	}

	int DrawAsync(GSRasterizer* r)
	{
		GSVector4i scissor = GetScissor();

		int prims = 0;

		switch(m_env.PRIM.TYPE)
		{
		case GPU_POLYGON:
			ASSERT(!(m_count % 3));
			prims = m_count / 3;
			for(int i = 0, j = m_count; i < j; i += 3) r->DrawTriangle(&m_vertices[i], scissor);
			break;
		case GPU_LINE:
			ASSERT(!(m_count & 1));
			prims = m_count / 2;
			for(int i = 0, j = m_count; i < j; i += 2) r->DrawLine(&m_vertices[i], scissor);
			break;
		case GPU_SPRITE:
			ASSERT(!(m_count & 1));
			prims = m_count / 2;
			for(int i = 0, j = m_count; i < j; i += 2) r->DrawSprite(&m_vertices[i], scissor, false);
			break;
		default:
			__assume(0);
		}

		return prims;
	}

public:
	GPURendererSW(const GPURendererSettings& rs)
		: GPURenderer(rs)
	{
		m_sync = (long*)_aligned_malloc(sizeof(*m_sync), 128); // get a whole cache line
		m_threads = AfxGetApp()->GetProfileInt(_T("GPUSettings"), _T("swthreads"), 1);

		m_rst = new GSRasterizer(new GPUDrawScanline(this, m_filter, m_dither), 0, m_threads);

		for(int i = 1; i < m_threads; i++) 
		{
			m_rmt.AddTail(new GSRasterizerMT(new GPUDrawScanline(this, m_filter, m_dither), i, m_threads, this, m_sync));
		}

		m_fpDrawingKickHandlers[GPU_POLYGON] = (DrawingKickHandler)&GPURendererSW::DrawingKickTriangle;
		m_fpDrawingKickHandlers[GPU_LINE] = (DrawingKickHandler)&GPURendererSW::DrawingKickLine;
		m_fpDrawingKickHandlers[GPU_SPRITE] = (DrawingKickHandler)&GPURendererSW::DrawingKickSprite;
	}

	virtual ~GPURendererSW()
	{
		delete m_rst;

		while(!m_rmt.IsEmpty()) 
		{
			delete m_rmt.RemoveHead();
		}
	}
};
