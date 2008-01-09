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

template <class Device, class Vertex>
class GSRendererSW : public GSRendererT<Device, Vertex>
{
	typedef typename Vertex::Scalar Scalar;
	typedef typename Vertex::Vector Vector;

protected:
	DWORD m_faddr_x0, m_faddr;
	DWORD m_zaddr_x0, m_zaddr;
	int* m_faddr_ro;
	int* m_zaddr_ro;
	int m_fx, m_fy;

	typedef void (GSRendererSW<Device, Vertex>::*DrawScanlinePtr)(int top, int left, int right, Vertex& v, const Vertex& dv);
	DrawScanlinePtr m_ds[2][4][4], m_pDrawScanline;

	typedef bool (GSRendererSW<Device, Vertex>::*DrawVertexPtr)(const Vertex& v);
	DrawVertexPtr m_dv[3][8][4][4], m_pDrawVertex;

	typedef void (GSRendererSW<Device, Vertex>::*DrawVertexTFXPtr)(Vector& Cf, const Vertex& v);
	DrawVertexTFXPtr m_dvtfx[4][2][2][4], m_pDrawVertexTFX;

	struct uv_wrap_t {union {struct {short min[8], max[8];}; struct {short and[8], or[8];};}; unsigned short mask[8];}* m_uv;

	CRect m_scissor;
	BYTE m_clip[65536];
	BYTE m_mask[65536];
	BYTE* m_clamp;

	Texture m_texture[2];

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
			v[0].p.q = v[1].p.q;
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

		Scalar sx0((int)m_context->SCISSOR.SCAX0);
		Scalar sy0((int)m_context->SCISSOR.SCAY0);
		Scalar sx1((int)m_context->SCISSOR.SCAX1);
		Scalar sy1((int)m_context->SCISSOR.SCAY1);

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
			Vector c = v[nv - 1].c;

			for(int i = 0; i < nv - 1; i++) 
			{
				v[i].c = c;
			}
		}

		m_count += nv;
	}

	void Draw()
	{
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

		int bZRW = (m_context->DepthRead() || m_context->DepthWrite()) ? 1 : 0;
		int iZTST = !m_context->TEST.ZTE ? 1 : m_context->TEST.ZTST;
		int iZPSM = 0;

		switch(m_context->ZBUF.PSM)
		{
		case PSM_PSMZ32: iZPSM = 0; break;
		case PSM_PSMZ24: iZPSM = 1; break;
		case PSM_PSMZ16: iZPSM = 2; break;
		case PSM_PSMZ16S: iZPSM = 3; break;
		default: ASSERT(0); iZPSM = 0; break;
		}

		m_pDrawScanline = m_ds[bZRW][iZTST][iZPSM];

		int iABE = m_env.PABE.PABE ? 2 : PRIM->ABE || PRIM->AA1 && (PRIM->PRIM == 1 || PRIM->PRIM == 2) ? 1 : 0;
		int iATST = !m_context->TEST.ATE ? 1 : m_context->TEST.ATST;
		int iAFAIL = m_context->TEST.AFAIL;
		int iFPSM = 0;

		switch(m_context->FRAME.PSM)
		{
		case PSM_PSMCT32: iFPSM = 0; break;
		case PSM_PSMCT24: iFPSM = 1; break;
		case PSM_PSMCT16: iFPSM = 2; break;
		case PSM_PSMCT16S: iFPSM = 3; break;
		default: ASSERT(0); iFPSM = 0; break;
		}

		m_pDrawVertex = m_dv[iABE][iATST][iAFAIL][iFPSM];

		if(PRIM->TME)
		{
			int iLOD = (m_context->TEX1.MMAG & 1) + (m_context->TEX1.MMIN & 1);
			int bLCM = m_context->TEX1.LCM ? 1 : 0;
			int bTCC = m_context->TEX0.TCC ? 1 : 0;
			int iTFX = m_context->TEX0.TFX;

			if(PRIM->FST)
			{
				iLOD = 3;
				bLCM = m_context->TEX1.K <= 0 && (m_context->TEX1.MMAG & 1) || m_context->TEX1.K > 0 && (m_context->TEX1.MMIN & 1);
			}

			if(m_filter == 0)
			{
				if(iLOD == 3) bLCM = 0;
				else iLOD = 0;
			}

			m_pDrawVertexTFX = m_dvtfx[iLOD][bLCM][bTCC][iTFX];

			SetupTexture();
		}
		
		m_scissor.SetRect(
			max(m_context->SCISSOR.SCAX0, 0),
			max(m_context->SCISSOR.SCAY0, 0),
			min(m_context->SCISSOR.SCAX1 + 1, m_context->FRAME.FBW * 64),
			min(m_context->SCISSOR.SCAY1 + 1, 4096));

		m_clamp = (m_env.COLCLAMP.CLAMP ? m_clip : m_mask) + 32768;

		int prims = 0;

		Vertex* vertices = m_vertices;

		switch(PRIM->PRIM)
		{
		case GS_POINTLIST:
			prims = m_count;
			for(int i = 0; i < prims; i++, vertices++) DrawPoint(vertices);
			break;
		case GS_LINELIST: 
		case GS_LINESTRIP: 
			ASSERT(!(m_count & 1));
			prims = m_count / 2;
			for(int i = 0; i < prims; i++, vertices += 2) DrawLine(vertices);
			break;
		case GS_TRIANGLELIST: 
		case GS_TRIANGLESTRIP: 
		case GS_TRIANGLEFAN:
			ASSERT(!(m_count % 3));
			prims = m_count / 3;
			for(int i = 0; i < prims; i++, vertices += 3) DrawTriangle(vertices);
			break;
		case GS_SPRITE:
			ASSERT(!(m_count & 3));
			prims = m_count / 4;
			for(int i = 0; i < prims; i++, vertices += 4) DrawSprite(vertices);
			break;
		default:
			__assume(0);
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

	void DrawPoint(Vertex* v)
	{
		CPoint p = *v;

		if(m_scissor.PtInRect(p))
		{
			Vertex scan = *v;
			Vertex dscan;

			(this->*m_pDrawScanline)(p.y, p.x, p.x + 1, scan, dscan);
		}
	}

	void DrawLine(Vertex* v)
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

	void DrawTriangle(Vertex* v)
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

					(this->*m_pDrawScanline)(top, left, right, scan, dscan);
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

	void DrawSprite(Vertex* v)
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

		if(DrawFilledRect(left, top, right, bottom, edge))
			return;

		for(; top < bottom; top++)
		{
			scan = edge;

			(this->*m_pDrawScanline)(top, left, right, scan, dscan);

			edge += dedge;
		}
	}

	bool DrawFilledRect(int left, int top, int right, int bottom, const Vertex& v)
	{
		if(left >= right || top >= bottom)
			return false;

		ASSERT(top >= 0);
		ASSERT(bottom >= 0);

		if(PRIM->IIP && PRIM->PRIM != GS_SPRITE
		|| m_context->TEST.ZTE && m_context->TEST.ZTST != 1
		|| m_context->TEST.ATE && m_context->TEST.ATST != 1
		|| m_context->TEST.DATE
		|| PRIM->TME
		|| PRIM->ABE
		|| PRIM->FGE
		|| m_env.DTHE.DTHE
		|| m_context->FRAME.FBMSK)
			return false;

		DWORD FBP = m_context->FRAME.Block(), FBW = m_context->FRAME.FBW;
		DWORD ZBP = m_context->ZBUF.Block();

		if(!m_context->ZBUF.ZMSK)
		{
			m_mem.FillRect(CRect(left, top, right, bottom), v.GetZ(), m_context->ZBUF.PSM, ZBP, FBW);
		}

		__declspec(align(16)) union {struct {short Rf, Gf, Bf, Af;}; UINT64 Cui64;};
		Cui64 = v.c;

		Rf = m_clamp[Rf];
		Gf = m_clamp[Gf];
		Bf = m_clamp[Bf];
		Af |= m_context->FBA.FBA << 7;

		DWORD Cdw;
		
		if(m_context->FRAME.PSM == PSM_PSMCT16 || m_context->FRAME.PSM == PSM_PSMCT16S)
		{
			Cdw = ((DWORD)(Rf&0xf8) >> 3)
				| ((DWORD)(Gf&0xf8) << 2) 
				| ((DWORD)(Bf&0xf8) << 7) 
				| ((DWORD)(Af&0x80) << 8);
		}
		else
		{
			#if _M_SSE >= 0x200
			
			__m128i r0 = _mm_load_si128((__m128i*)&Cui64);

			Cdw = (DWORD)_mm_cvtsi128_si32(_mm_packus_epi16(r0, r0));

			#else
			
			Cdw = ((DWORD)(Rf&0xff) << 0)
				| ((DWORD)(Gf&0xff) << 8) 
				| ((DWORD)(Bf&0xff) << 16) 
				| ((DWORD)(Af&0xff) << 24);
			
			#endif
		}

		m_mem.FillRect(CRect(left, top, right, bottom), Cdw, m_context->FRAME.PSM, FBP, FBW);

		return true;
	}

	template<int bZRW, int iZTST, int iZPSM>
	void DrawScanline(int top, int left, int right, Vertex& v, const Vertex& dv)
	{
		m_faddr_x0 = (m_context->ftbl->pa)(0, top, m_context->FRAME.Block(), m_context->FRAME.FBW);
		m_faddr_ro = m_context->ftbl->rowOffset[top & 7];

		m_fx = left;
		m_fy = top;

		if(bZRW)
		{
			int ZPSM;

			switch(iZPSM)
			{
			case 0: ZPSM = PSM_PSMZ32; break;
			case 1: ZPSM = PSM_PSMZ24; break;
			case 2: ZPSM = PSM_PSMZ16; break;
			case 3: ZPSM = PSM_PSMZ16S; break;
			default: ZPSM = PSM_PSMZ32; break;
			}

			m_zaddr_x0 = m_mem.pixelAddressX(ZPSM, 0, top, m_context->ZBUF.Block(), m_context->FRAME.FBW);
			m_zaddr_ro = m_mem.m_psm[ZPSM].rowOffset[top & 7];

			for(; left < right; left++, v.c += dv.c, v.p += dv.p, v.t += dv.t, m_fx++)
			{
				m_faddr = m_faddr_x0 + m_faddr_ro[left];
				m_zaddr = m_zaddr_x0 + m_zaddr_ro[left];

				DWORD z = v.GetZ();
				
				switch(iZTST)
				{
				case 0: continue;
				case 1: break;
				case 2: if(z < m_mem.readPixelX(ZPSM, m_zaddr)) continue; break;
				case 3: if(z <= m_mem.readPixelX(ZPSM, m_zaddr)) continue; break;
				default: __assume(0);
				}

				if((this->*m_pDrawVertex)(v))
				{
					m_mem.writePixelX(ZPSM, m_zaddr, z);
				}
			}
		}
		else
		{
			for(; left < right; left++, v.c += dv.c, v.t += dv.t, m_fx++)
			{
				m_faddr = m_faddr_x0 + m_faddr_ro[left];

				(this->*m_pDrawVertex)(v);
			}
		}
	}

	template <int iABE, int iATST, int iAFAIL, int iFPSM> 
	bool DrawVertex(const Vertex& v)
	{
		int FPSM;

		switch(iFPSM)
		{
		case 0: FPSM = PSM_PSMCT32; break;
		case 1: FPSM = PSM_PSMCT24; break;
		case 2: FPSM = PSM_PSMCT16; break;
		case 3: FPSM = PSM_PSMCT16S; break;
		default: FPSM = PSM_PSMCT32; break;
		}

		union
		{
			struct {Vector Cf, Cd, Ca;};
			struct {Vector Cfda[3];};
		};

		Cf = v.c;

		if(PRIM->TME)
		{
			(this->*m_pDrawVertexTFX)(Cf, v);
		}

		bool Apass = true;

		int Af = (int)Cf.a;

		switch(iATST)
		{
		case 0: Apass = false; break;
		case 1: Apass = true; break;
		case 2: Apass = Af < (int)m_context->TEST.AREF; break;
		case 3: Apass = Af <= (int)m_context->TEST.AREF; break;
		case 4: Apass = Af == (int)m_context->TEST.AREF; break;
		case 5: Apass = Af >= (int)m_context->TEST.AREF; break;
		case 6: Apass = Af > (int)m_context->TEST.AREF; break;
		case 7: Apass = Af != (int)m_context->TEST.AREF; break;
		default: __assume(0);
		}

		BOOL ZMSK = m_context->ZBUF.ZMSK;
		DWORD FBMSK = m_context->FRAME.FBMSK;

		if(!Apass)
		{
			switch(iAFAIL)
			{
			case 0: return false;
			case 1: ZMSK = 1; break; // RGBA
			case 2: FBMSK = 0xffffffff; break; // Z
			case 3: FBMSK = 0xff000000; ZMSK = 1; break; // RGB
			default: __assume(0);
			}
		}

		DWORD Cddw;

		// FIXME: for AA1 the value of Af should be calculated from the pixel coverage...

		bool ABE = iABE == 1 || iABE == 2 && (int)Cf.a >= 0x80;

		if(FBMSK || ABE || m_context->TEST.DATE)
		{
			GIFRegTEXA TEXA;
			/*
			TEXA.AEM = 0;
			TEXA.TA0 = 0;
			TEXA.TA1 = 0x80;
			*/
			TEXA.ai32[0] = 0;
			TEXA.ai32[1] = 0x80;

			Cddw = m_mem.readTexelX(FPSM, m_faddr, TEXA);
		}

		if(m_context->TEST.DATE)
		{
			if((Cddw >> 31) ^ m_context->TEST.DATM) 
			{
				return false;
			}
		}

		if(PRIM->FGE)
		{
			Scalar a = Cf.a;
			Vector Cfog((DWORD)m_env.FOGCOL.ai32[0]);
			Cf = Cfog + (Cf - Cfog) * v.t.z;
			Cf.a = a;
		}

		if(ABE)
		{
			Cd = Cddw;

			Ca = Vector(Scalar(0));
			Ca.a = Scalar((int)m_context->ALPHA.FIX);

			Scalar a = Cf.a;
			Cf = ((Cfda[m_context->ALPHA.A] - Cfda[m_context->ALPHA.B]) * Cfda[m_context->ALPHA.C].a >> 7) + Cfda[m_context->ALPHA.D];
			Cf.a = a;
		}

		DWORD Cfdw; 

		if(m_env.COLCLAMP.CLAMP && !m_env.DTHE.DTHE)
		{
			Cfdw = Cf;
		}
		else
		{
			__declspec(align(16)) union {struct {short Rf, Gf, Bf, Af;}; UINT64 Cui64;};
			
			Cui64 = Cf;

			if(m_env.DTHE.DTHE)
			{
				short DMxy = (signed char)((*((WORD*)&m_env.DIMX.i64 + (m_fy&3)) >> ((m_fx&3)<<2)) << 5) >> 5;
				
				Rf = (short)(Rf + DMxy);
				Gf = (short)(Gf + DMxy);
				Bf = (short)(Bf + DMxy);
			}

			Rf = m_clamp[Rf];
			Gf = m_clamp[Gf];
			Bf = m_clamp[Bf];
			Af |= m_context->FBA.FBA << 7;

			#if _M_SSE >= 0x200
			
			__m128i r0 = _mm_load_si128((__m128i*)&Cui64);

			Cfdw = (DWORD)_mm_cvtsi128_si32(_mm_packus_epi16(r0, r0));

			#else
			
			Cfdw = ((DWORD)(Rf & 0xff) << 0)
				| ((DWORD)(Gf & 0xff) << 8) 
				| ((DWORD)(Bf & 0xff) << 16) 
				| ((DWORD)(Af & 0xff) << 24);
			
			#endif
		}

		if(FBMSK != 0)
		{
			Cfdw = (Cfdw & ~FBMSK) | (Cddw & FBMSK);
		}

		m_mem.writeFrameX(FPSM, m_faddr, Cfdw);

		return ZMSK == 0;
	}

	template <int iLOD, bool bLCM, bool bTCC, int iTFX>
	void DrawVertexTFX(Vector& Cf, const Vertex& v)
	{
		Vector t = v.t;

		bool bilinear = iLOD == 2; 

		if(iLOD == 3)
		{
			bilinear = !!bLCM;
		}
		else
		{
			t.q.rcp();
			t *= t.q;

			if(iLOD == 1)
			{
				float lod = (float)(int)m_context->TEX1.K;
				float one_over_log2 = 3.3219281f;
				if(!bLCM) lod += log(fabs((float)t.q)) * one_over_log2 * (1 << m_context->TEX1.L);
				bilinear = lod <= 0 && (m_context->TEX1.MMAG & 1) || lod > 0 && (m_context->TEX1.MMIN & 1);
			}
		}

		if(bilinear) t -= Scalar(0.5f);

		__declspec(align(16)) short ituv[8];
		
		t.lnuv(ituv);

		#if _M_SSE >= 0x200

		__m128i uv = _mm_load_si128((__m128i*)ituv);
		__m128i mask = _mm_load_si128((__m128i*)m_uv->mask);
		__m128i region = _mm_or_si128(_mm_and_si128(uv, *(__m128i*)m_uv->and), *(__m128i*)m_uv->or);
		__m128i clamp = _mm_min_epi16(_mm_max_epi16(uv, *(__m128i*)m_uv->min), *(__m128i*)m_uv->max);
		_mm_store_si128((__m128i*)ituv, _mm_or_si128(_mm_and_si128(region, mask), _mm_andnot_si128(mask, clamp)));

		#else

		for(int i = 0; i < 4; i++)
		{
			short region = (ituv[i] & m_uv->and[i]) | m_uv->or[i];
			short clamp = ituv[i] < m_uv->min[i] ? m_uv->min[i] : ituv[i] > m_uv->max[i] ? m_uv->max[i] : ituv[i];
			ituv[i] = (region & m_uv->mask[i]) | (clamp & ~m_uv->mask[i]);
		}

		#endif

		Vector Ct[4];

		if(bilinear)
		{
			Ct[0] = ReadTexel(ituv[0], ituv[2]);
			Ct[1] = ReadTexel(ituv[1], ituv[2]);
			Ct[2] = ReadTexel(ituv[0], ituv[3]);
			Ct[3] = ReadTexel(ituv[1], ituv[3]);

			Vector ft = t - t.floor();

			Ct[0] = Ct[0] + (Ct[1] - Ct[0]) * ft.x;
			Ct[2] = Ct[2] + (Ct[3] - Ct[2]) * ft.x;
			Ct[0] = Ct[0] + (Ct[2] - Ct[0]) * ft.y;
		}
		else 
		{
			Ct[0] = ReadTexel(ituv[0], ituv[2]);
		}

		Scalar a = Cf.a;

		switch(iTFX)
		{
		case 0:
			Cf = (Cf * Ct[0] >> 7);
			if(!bTCC) Cf.a = a;
			break;
		case 1:
			Cf = Ct[0];
			break;
		case 2:
			Cf = (Cf * Ct[0] >> 7) + Cf.a;
			Cf.a = !bTCC ? a : (Ct[0].a + a);
			break;
		case 3:
			Cf = (Cf * Ct[0] >> 7) + Cf.a;
			Cf.a = !bTCC ? a : Ct[0].a;
			break;
		default: 
			__assume(0);
		}

		Cf.sat();
	}

	void SetupTexture()
	{
		m_mem.SetupCLUT32(m_context->TEX0, m_env.TEXA);

		//

		int tw = 1 << m_context->TEX0.TW;
		int th = 1 << m_context->TEX0.TH;

		switch(m_context->CLAMP.WMS)
		{
		case 0: m_uv->and[0] = (short)(tw-1); m_uv->or[0] = 0; m_uv->mask[0] = 0xffff; break;
		case 1: m_uv->min[0] = 0; m_uv->max[0] = (short)(tw-1); m_uv->mask[0] = 0; break;
		case 2: m_uv->min[0] = (short)m_context->CLAMP.MINU; m_uv->max[0] = (short)m_context->CLAMP.MAXU; m_uv->mask[0] = 0; break;
		case 3: m_uv->and[0] = (short)m_context->CLAMP.MINU; m_uv->or[0] = (short)m_context->CLAMP.MAXU; m_uv->mask[0] = 0xffff; break;
		default: __assume(0);
		}

		m_uv->and[1] = m_uv->and[0];
		m_uv->or[1] = m_uv->or[0];
		m_uv->min[1] = m_uv->min[0];
		m_uv->max[1] = m_uv->max[0];
		m_uv->mask[1] = m_uv->mask[0];

		switch(m_context->CLAMP.WMT)
		{
		case 0: m_uv->and[2] = (short)(th-1); m_uv->or[2] = 0; m_uv->mask[2] = 0xffff; break;
		case 1: m_uv->min[2] = 0; m_uv->max[2] = (short)(th-1); m_uv->mask[2] = 0; break;
		case 2: m_uv->min[2] = (short)m_context->CLAMP.MINV; m_uv->max[2] = (short)m_context->CLAMP.MAXV; m_uv->mask[2] = 0; break;
		case 3: m_uv->and[2] = (short)m_context->CLAMP.MINV; m_uv->or[2] = (short)m_context->CLAMP.MAXV; m_uv->mask[2] = 0xffff; break;
		default: __assume(0);
		}

		m_uv->and[3] = m_uv->and[2];
		m_uv->or[3] = m_uv->or[2];
		m_uv->min[3] = m_uv->min[2];
		m_uv->max[3] = m_uv->max[2];
		m_uv->mask[3] = m_uv->mask[2];
	}

	DWORD* m_cache;
	DWORD m_cache_page[(1024 / 32) * (1024 / 64)];

	void InvalidateTextureCache() 
	{
		memset(m_cache_page, ~0, sizeof(m_cache_page));
	}

	void ReadTexture(int x, int y)
	{
		const int xs = 64;
		const int ys = 32;

		x &= ~(xs - 1);
		y &= ~(ys - 1);

		CRect r(x, y, x + xs, y + ys);

		DWORD* dst = &m_cache[y * 1024 + x];

		(m_mem.*m_context->ttbl->ust)(r, (BYTE*)dst, 1024 * 4, m_context->TEX0, m_env.TEXA);

		m_perfmon.Put(GSPerfMon::Unswizzle, r.Width() * r.Height() * 4);
	}

	__forceinline DWORD ReadTexel(int x, int y)
	{
		// return (m_mem.*m_context->ttbl->rt)(x, y, m_context->TEX0, m_env.TEXA);

		// - page size depends on psm, this is just the smallest for 32 bit
		// - seems to be just enough to avoid recursive rendering errors
		// - to be fully correct we need to fetch ttbl->pgs sized pages

		DWORD* page = &m_cache_page[(y & ~31) + (x >> 6)];

		if(*page != m_context->TEX0.ai32[0])
		{
			*page = m_context->TEX0.ai32[0];

			ReadTexture(x, y);
		}

		return m_cache[y * 1024 + x];
	}

public:
	GSRendererSW(BYTE* base, bool mt, void (*irq)(), int nloophack, int interlace, int aspectratio, int filter, bool vsync)
		: GSRendererT(base, mt, irq, nloophack, interlace, aspectratio, filter, vsync)
	{
		int i = SHRT_MIN;
		int j = 0;
		for(; i < 0; i++, j++) {m_clip[j] = 0; m_mask[j] = (BYTE)i;}
		for(; i < 256; i++, j++) {m_clip[j] = (BYTE)i; m_mask[j] = (BYTE)i;}
		for(; i <= SHRT_MAX; i++, j++) {m_clip[j] = 255; m_mask[j] = (BYTE)i;}

		m_cache = (DWORD*)_aligned_malloc(1024 * 1024 * sizeof(m_cache[0]), 16);
		
		InvalidateTextureCache();

		m_uv = (uv_wrap_t*)_aligned_malloc(sizeof(uv_wrap_t), 16);

		// w00t :P

		#define InitZPSM(bZRW, iZTST, iZPSM) \
			m_ds[bZRW][iZTST][iZPSM] = &GSRendererSW<Device, Vertex>::DrawScanline<bZRW, iZTST, iZPSM>; \

		#define InitZTST(bZRW, iZTST) \
			InitZPSM(bZRW, iZTST, 0) \
			InitZPSM(bZRW, iZTST, 1) \
			InitZPSM(bZRW, iZTST, 2) \
			InitZPSM(bZRW, iZTST, 3) \

		#define InitZRW(bZRW) \
			InitZTST(bZRW, 0) \
			InitZTST(bZRW, 1) \
			InitZTST(bZRW, 2) \
			InitZTST(bZRW, 3) \

		#define InitDS() \
			InitZRW(0) \
			InitZRW(1) \

		InitDS();

		#define InitFPSM(iABE, iATST, iAFAIL, iFPSM) \
			m_dv[iABE][iATST][iAFAIL][iFPSM] = &GSRendererSW<Device, Vertex>::DrawVertex<iABE, iATST, iAFAIL, iFPSM>; \

		#define InitAFAIL(iABE, iATST, iAFAIL) \
			InitFPSM(iABE, iATST, iAFAIL, 0) \
			InitFPSM(iABE, iATST, iAFAIL, 1) \
			InitFPSM(iABE, iATST, iAFAIL, 2) \
			InitFPSM(iABE, iATST, iAFAIL, 3) \

		#define InitATST(iABE, iATST) \
			InitAFAIL(iABE, iATST, 0) \
			InitAFAIL(iABE, iATST, 1) \
			InitAFAIL(iABE, iATST, 2) \
			InitAFAIL(iABE, iATST, 3) \

		#define InitABE(iABE) \
			InitATST(iABE, 0) \
			InitATST(iABE, 1) \
			InitATST(iABE, 2) \
			InitATST(iABE, 3) \
			InitATST(iABE, 4) \
			InitATST(iABE, 5) \
			InitATST(iABE, 6) \
			InitATST(iABE, 7) \

		#define InitDV() \
			InitABE(0) \
			InitABE(1) \
			InitABE(2) \

		InitDV();

		#define InitTFX(iLOD, bLCM, bTCC, iTFX) \
			m_dvtfx[iLOD][bLCM][bTCC][iTFX] = &GSRendererSW<Device, Vertex>::DrawVertexTFX<iLOD, bLCM, bTCC, iTFX>; \

		#define InitTCC(iLOD, bLCM, bTCC) \
			InitTFX(iLOD, bLCM, bTCC, 0) \
			InitTFX(iLOD, bLCM, bTCC, 1) \
			InitTFX(iLOD, bLCM, bTCC, 2) \
			InitTFX(iLOD, bLCM, bTCC, 3) \

		#define InitLCM(iLOD, bLCM) \
			InitTCC(iLOD, bLCM, false) \
			InitTCC(iLOD, bLCM, true) \

		#define InitLOD(iLOD) \
			InitLCM(iLOD, false) \
			InitLCM(iLOD, true) \

		#define InitDVTFX() \
			InitLOD(0) \
			InitLOD(1) \
			InitLOD(2) \
			InitLOD(3) \


		InitDVTFX();
	}

	virtual ~GSRendererSW()
	{
		_aligned_free(m_cache);
		_aligned_free(m_uv);
	}
};

#include "GSVertexSW.h"

template<class Device>
class GSRendererSWFP : public GSRendererSW<Device, GSVertexSWFP>
{
public:
	GSRendererSWFP(BYTE* base, bool mt, void (*irq)(), int nloophack, int interlace, int aspectratio, int filter, bool vsync)
		: GSRendererSW<Device, GSVertexSWFP>(base, mt, irq, nloophack, interlace, aspectratio, filter, vsync)
	{
	}

protected:
	void VertexKick(bool skip)
	{
		GSVertexSWFP& v = m_vl.AddTail();

		v.p.x = (int)m_v.XYZ.X - (int)m_context->XYOFFSET.OFX;
		v.p.y = (int)m_v.XYZ.Y - (int)m_context->XYOFFSET.OFY;
		v.p *= GSVertexSWFP::Scalar(1.0f / 16);
		v.p.z = (float)m_v.XYZ.Z;

		v.c = (DWORD)m_v.RGBAQ.ai32[0];

		if(PRIM->TME)
		{
			if(PRIM->FST)
			{
				v.t.x = (float)(int)m_v.UV.U;
				v.t.y = (float)(int)m_v.UV.V;
				v.t *= GSVertexSWFP::Scalar(1.0f / 16);
				v.t.q = 1.0f;
			}
			else
			{
				v.t.x = m_v.ST.S * (1 << m_context->TEX0.TW);
				v.t.y = m_v.ST.T * (1 << m_context->TEX0.TH);
				v.t.q = m_v.RGBAQ.Q;
			}
		}

		if(PRIM->FGE)
		{
			v.t.z = (float)m_v.FOG.F * (1.0f / 255);
		}

		__super::VertexKick(skip);
	}
};
