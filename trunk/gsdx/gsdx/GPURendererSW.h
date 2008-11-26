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
#include "GPUVertexSW.h"
#include "GPURasterizer.h"
#include "GPUTextureCacheSW.h"

template <class Device>
class GPURendererSW : public GPURenderer<Device, GPUVertexSW>
{
	typedef GPUVertexSW Vertex;

protected:
	long m_threads;
	GPUTextureCacheSW* m_tc;
	GPURasterizer* m_rst;
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

		// x/y + off.x/y should wrap around at +/-1024

		int x = m_v.XY.X + m_env.DROFF.X;
		int y = m_v.XY.Y + m_env.DROFF.Y;

		v.p = GSVector4(x, y, m_v.UV.X, m_v.UV.Y) + GSVector4(0.0f, 0.0f, 0.5f, 0.5f);
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

		int prims = m_rst->Draw(m_vertices, m_count, texture);

		// TODO
		{
			CRect r;
			
			r.left = m_env.DRAREATL.X;
			r.top = m_env.DRAREATL.Y;
			r.right = min(m_env.DRAREABR.X + 1, 1024);
			r.bottom = min(m_env.DRAREABR.Y + 1, 512);

			GSVector4 minv(+1e10f);
			GSVector4 maxv(-1e10f);

			for(int i = 0, j = m_count; i < j; i++)
			{
				GSVector4 p = m_vertices[i].p;

				minv = minv.minv(p);
				maxv = maxv.maxv(p);
			}

			GSVector4i v(minv.xyxy(maxv));

			r.left = max(r.left, min(r.right, v.x));
			r.top = max(r.top, min(r.bottom, v.y));
			r.right = min(r.right, max(r.left, v.z));
			r.bottom = min(r.bottom, max(r.top, v.w));

			Invalidate(r);
		}

		m_perfmon.Put(GSPerfMon::Prim, prims);
		m_perfmon.Put(GSPerfMon::Draw, 1);
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

		m_rst = new GPURasterizer(this, 0, m_threads);

		m_fpDrawingKickHandlers[GPU_POLYGON] = (DrawingKickHandler)&GPURendererSW::DrawingKickTriangle;
		m_fpDrawingKickHandlers[GPU_LINE] = (DrawingKickHandler)&GPURendererSW::DrawingKickLine;
		m_fpDrawingKickHandlers[GPU_SPRITE] = (DrawingKickHandler)&GPURendererSW::DrawingKickSprite;
	}

	virtual ~GPURendererSW()
	{
		delete m_tc;

		delete m_rst;
	}
};
