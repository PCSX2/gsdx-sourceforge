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
#include "GSVertexSW.h"
#include "GSRasterizer.h"

extern const GSVector4 g_pos_scale;

template <class Device>
class GSRendererSW : public GSRendererT<Device, GSVertexSW>
{
	typedef GSVertexSW Vertex;

protected:
	long* m_sync;
	long m_threads;
	GSRasterizer* m_rst;
	CAtlList<GSRasterizerMT*> m_rmt;
	Texture m_texture[2];
	DWORD m_fbp;

	void ResetDevice() 
	{
		m_texture[0] = Texture();
		m_texture[1] = Texture();
	}

	bool GetOutput(int i, Texture& t)
	{
		CRect r(0, 0, DISPFB[i]->FBW * 64, GetFrameRect(i).bottom);

		// TODO: round up bottom

		if(m_texture[i].GetWidth() != r.Width() || m_texture[i].GetHeight() != r.Height())
		{
			m_texture[i] = Texture();
		}

		if(!m_texture[i] && !m_dev.CreateTexture(m_texture[i], r.Width(), r.Height())) 
		{
			return false;
		}

		GIFRegTEX0 TEX0;

		TEX0.TBP0 = DISPFB[i]->Block();
		TEX0.TBW = DISPFB[i]->FBW;
		TEX0.PSM = DISPFB[i]->PSM;

		GIFRegCLAMP CLAMP;

		CLAMP.WMS = CLAMP.WMT = 1;

		// TODO
		static BYTE* buff = (BYTE*)_aligned_malloc(1024 * 1024 * 4, 16);
		static int pitch = 1024 * 4;

		m_mem.ReadTexture(r, buff, pitch, TEX0, m_env.TEXA, CLAMP);

		m_texture[i].Update(r, buff, pitch);

		t = m_texture[i];

		if(s_dump)
		{
			CString str;
			str.Format(_T("c:\\temp1\\_%05d_f%I64d_fr%d_%05x_%d.bmp"), s_n++, m_perfmon.GetFrame(), i, (int)TEX0.TBP0, (int)TEX0.PSM);
			if(s_save) t.Save(str);
		}

		return true;
	}

	void VertexKick(bool skip)
	{
		GSVertexSW& v = m_vl.AddTail();

		int x = (int)m_v.XYZ.X - (int)m_context->XYOFFSET.OFX;
		int y = (int)m_v.XYZ.Y - (int)m_context->XYOFFSET.OFY;

		GSVector4i p(x, y, 0, (int)m_v.FOG.F);

		v.p = GSVector4(p) * g_pos_scale;
		v.p.z = (float)min(m_v.XYZ.Z, 0xffffff00); // max value which can survive the DWORD=>float=>DWORD conversion

		v.c = (DWORD)m_v.RGBAQ.ai32[0];

		if(PRIM->TME)
		{
			if(PRIM->FST)
			{
				v.t.x = (float)(int)m_v.UV.U;
				v.t.y = (float)(int)m_v.UV.V;
				v.t *= 1.0f / 16;
				v.t.z = 1.0f;
			}
			else
			{
				v.t.x = m_v.ST.S;
				v.t.y = m_v.ST.T;
				v.t *= GSVector4((float)(1 << m_context->TEX0.TW), (float)(1 << m_context->TEX0.TH));
				v.t.z = m_v.RGBAQ.Q;
			}
		}

		__super::VertexKick(skip);
	}

	void DrawingKick(bool skip)
	{
		Vertex* v = &m_vertices[m_count];
		int nv = 0;

		switch(PRIM->PRIM)
		{
		case GS_POINTLIST:
			m_vl.RemoveAt(0, v[0]);
			nv = 1;
			break;
		case GS_LINELIST:
			m_vl.RemoveAt(0, v[0]);
			m_vl.RemoveAt(0, v[1]);
			nv = 2;
			break;
		case GS_LINESTRIP:
			m_vl.RemoveAt(0, v[0]);
			m_vl.GetAt(0, v[1]);
			nv = 2;
			break;
		case GS_TRIANGLELIST:
			m_vl.RemoveAt(0, v[0]);
			m_vl.RemoveAt(0, v[1]);
			m_vl.RemoveAt(0, v[2]);
			nv = 3;
			break;
		case GS_TRIANGLESTRIP:
			m_vl.RemoveAt(0, v[0]);
			m_vl.GetAt(0, v[1]);
			m_vl.GetAt(1, v[2]);
			nv = 3;
			break;
		case GS_TRIANGLEFAN:
			m_vl.GetAt(0, v[0]);
			m_vl.RemoveAt(1, v[1]);
			m_vl.GetAt(1, v[2]);
			nv = 3;
			break;
		case GS_SPRITE:
			m_vl.RemoveAt(0, v[0]);
			m_vl.RemoveAt(0, v[1]);
			nv = 4;
			v[0].p.z = v[1].p.z;
			v[0].p.w = v[1].p.w;
			v[0].t.z = v[1].t.z;
			v[2] = v[1];
			v[3] = v[1];
			v[1].p.y = v[0].p.y;
			v[1].t.y = v[0].t.y;
			v[2].p.x = v[0].p.x;
			v[2].t.x = v[0].t.x;
			break;
		default:
			ASSERT(0);
			return;
		}

		if(skip)
		{
			return;
		}

		float sx0 = (float)(int)m_context->SCISSOR.SCAX0;
		float sy0 = (float)(int)m_context->SCISSOR.SCAY0;
		float sx1 = (float)(int)m_context->SCISSOR.SCAX1;
		float sy1 = (float)(int)m_context->SCISSOR.SCAY1;

		switch(nv)
		{
		case 1:
			if(v[0].p.x < sx0
			|| v[0].p.x > sx1
			|| v[0].p.y < sy0
			|| v[0].p.y > sy1)
				return;
			break;
		case 2:
			if(v[0].p.x < sx0 && v[1].p.x < sx0
			|| v[0].p.x > sx1 && v[1].p.x > sx1
			|| v[0].p.y < sy0 && v[1].p.y < sy0
			|| v[0].p.y > sy1 && v[1].p.y > sy1)
				return;
			break;
		case 3:
			if(v[0].p.x < sx0 && v[1].p.x < sx0 && v[2].p.x < sx0
			|| v[0].p.x > sx1 && v[1].p.x > sx1 && v[2].p.x > sx1
			|| v[0].p.y < sy0 && v[1].p.y < sy0 && v[2].p.y < sy0
			|| v[0].p.y > sy1 && v[1].p.y > sy1 && v[2].p.y > sy1)
				return;
			break;
		case 4:
			if(v[0].p.x < sx0 && v[3].p.x < sx0
			|| v[0].p.x > sx1 && v[3].p.x > sx1
			|| v[0].p.y < sy0 && v[3].p.y < sy0
			|| v[0].p.y > sy1 && v[3].p.y > sy1)
				return;
			break;
		default:
			__assume(0);
		}

		if(PRIM->IIP == 0 || PRIM->PRIM == GS_SPRITE)
		{
			GSVector4 c = v[nv - 1].c;

			for(int i = 0; i < nv - 1; i++) 
			{
				v[i].c = c;
			}
		}

		m_count += nv;
	}

	void Draw()
	{
		if(m_fbp != m_context->FRAME.Block())
		{
			//InvalidateTextureCache();

			m_fbp = m_context->FRAME.Block();
		}

		if(s_dump)
		{
			CString str;
			str.Format(_T("c:\\temp1\\_%05d_f%I64d_tex_%05x_%d.bmp"), s_n++, m_perfmon.GetFrame(), (int)m_context->TEX0.TBP0, (int)m_context->TEX0.PSM);
			if(PRIM->TME) if(s_save) {m_mem.SetupCLUT32(m_context->TEX0, m_env.TEXA); m_mem.SaveBMP(str, m_context->TEX0.TBP0, m_context->TEX0.TBW, m_context->TEX0.PSM, 1 << m_context->TEX0.TW, 1 << m_context->TEX0.TH);}
			str.Format(_T("c:\\temp1\\_%05d_f%I64d_rt0_%05x_%d.bmp"), s_n++, m_perfmon.GetFrame(), m_context->FRAME.Block(), m_context->FRAME.PSM);
			if(s_save) {m_mem.SaveBMP(str, m_context->FRAME.Block(), m_context->FRAME.FBW, m_context->FRAME.PSM, GetFrameSize(1).cx, 512);}//GetFrameSize(1).cy);
			str.Format(_T("c:\\temp1\\_%05d_f%I64d_rz0_%05x_%d.bmp"), s_n-1, m_perfmon.GetFrame(), m_context->ZBUF.Block(), m_context->ZBUF.PSM);
			if(s_savez) {m_mem.SaveBMP(str, m_context->ZBUF.Block(), m_context->FRAME.FBW, m_context->ZBUF.PSM, GetFrameSize(1).cx, 512);}
		}

		if(PRIM->TME)
		{
			m_mem.SetupCLUT32(m_context->TEX0, m_env.TEXA);
		}

		*m_sync = 0;

		POSITION pos = m_rmt.GetHeadPosition();

		while(pos)
		{
			GSRasterizerMT* r = m_rmt.GetNext(pos);

			r->BeginDraw(m_vertices, m_count);
		}

		// 1st thread is this thread

		int prims = m_rst->Draw(m_vertices, m_count);

		// wait for the other threads to finish

		while(*m_sync)
		{
			_mm_pause();
		}

		m_perfmon.Put(GSPerfMon::Prim, prims);
		m_perfmon.Put(GSPerfMon::Draw, 1);

		if(s_dump)
		{
			CString str;
			str.Format(_T("c:\\temp1\\_%05d_f%I64d_rt1_%05x_%d.bmp"), s_n++, m_perfmon.GetFrame(), m_context->FRAME.Block(), m_context->FRAME.PSM);
			if(s_save) {m_mem.SaveBMP(str, m_context->FRAME.Block(), m_context->FRAME.FBW, m_context->FRAME.PSM, GetFrameSize(1).cx, 512);}//GetFrameSize(1).cy);
			str.Format(_T("c:\\temp1\\_%05d_f%I64d_rz1_%05x_%d.bmp"), s_n-1, m_perfmon.GetFrame(), m_context->ZBUF.Block(), m_context->ZBUF.PSM);
			if(s_savez) {m_mem.SaveBMP(str, m_context->ZBUF.Block(), m_context->FRAME.FBW, m_context->ZBUF.PSM, GetFrameSize(1).cx, 512);}
		}
	}

	void InvalidateVideoMem(const GIFRegBITBLTBUF& BITBLTBUF, CRect r)
	{
		//InvalidateTextureCache();
	}

	void InvalidateTextureCache()
	{
		m_rst->InvalidateTextureCache();

		POSITION pos = m_rmt.GetHeadPosition();

		while(pos)
		{
			GSRasterizerMT* r = m_rmt.GetNext(pos);

			r->InvalidateTextureCache();
		}
	}

public:
	GSRendererSW(BYTE* base, bool mt, void (*irq)(), int nloophack, const GSRendererSettings& rs)
		: GSRendererT(base, mt, irq, nloophack, rs)
	{
		m_sync = (long*)_aligned_malloc(sizeof(*m_sync), 128); // get a whole cache line
		m_threads = AfxGetApp()->GetProfileInt(_T("Settings"), _T("swthreads"), 1);

		m_rst = new GSRasterizer(this, 0, m_threads);

		for(int i = 1; i < m_threads; i++) 
		{
			GSRasterizerMT* r = new GSRasterizerMT(this, i, m_threads, m_sync);

			m_rmt.AddTail(r);
		}

		m_fbp = (DWORD)~0;
	}

	virtual ~GSRendererSW()
	{
		delete m_rst;

		while(!m_rmt.IsEmpty()) 
		{
			delete m_rmt.RemoveHead();
		}
	}
};
