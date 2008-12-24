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

#pragma once

#include "GSRenderer.h"
#include "GSTextureCacheSW.h"
#include "GSDrawScanline.h"

extern const GSVector4 g_pos_scale;

template <class Device>
class GSRendererSW : public GSRendererT<Device, GSVertexSW>
{
protected:
	GSRasterizerList m_rl;
	GSTextureCacheSW* m_tc;
	Texture m_texture[2];
	bool m_reset;

	__declspec(align(16)) struct VertexTrace
	{
		GSVertexSW v; 

		union
		{
			DWORD value; 
			struct {DWORD s:1, t:1, q:1, _pad:1, r:1, g:1, b:1, a:1;};
			struct {DWORD stq:4, rgba:4;};
		} eq;

		bool first;

		void Reset()
		{
			first = true;
			eq.value = 0xffffffff;
		}

		VertexTrace() {Reset();}

	} m_vtrace;

	void Reset() 
	{
		// TODO: GSreset can come from the main thread too => crash
		// m_tc->RemoveAll();

		m_reset = true;

		m_vtrace.Reset();

		__super::Reset();
	}

	void VSync(int field)
	{
		__super::VSync(field);

		m_tc->IncAge();

		if(m_reset)
		{
			m_tc->RemoveAll();

			m_reset = false;
		}
	}

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
		GSVertexSW v;

		GSVector4i p((int)m_v.XYZ.X, (int)m_v.XYZ.Y, 0, (int)m_v.FOG.F);
		GSVector4i o((int)m_context->XYOFFSET.OFX, (int)m_context->XYOFFSET.OFY);

		v.p = GSVector4(p - o) * g_pos_scale;
		v.p.z = (float)min(m_v.XYZ.Z, 0xffffff00); // max value which can survive the DWORD=>float=>DWORD conversion

		v.c = GSVector4((DWORD)m_v.RGBAQ.ai32[0]) * 128.0f;

		if(PRIM->TME)
		{
			if(PRIM->FST)
			{
				v.t = GSVector4(GSVector4i((int)m_v.UV.U, (int)m_v.UV.V) << (16 - 4));
				v.t.z = 1.0f;
			}
			else
			{
				v.t = GSVector4(m_v.ST.S, m_v.ST.T, 0.0f, 0.0f);
				v.t *= GSVector4((float)(0x10000 << m_context->TEX0.TW), (float)(0x10000 << m_context->TEX0.TH));
				v.t.z = m_v.RGBAQ.Q;
			}
		}

		if(m_vtrace.first)
		{
			m_vtrace.v.t = v.t;
			m_vtrace.v.c = v.c;
			m_vtrace.first = false;
		}
		else
		{
			m_vtrace.eq.value &= (m_vtrace.v.t == v.t).mask() | ((m_vtrace.v.c == v.c).mask() << 4); // v.p not needed
		}

		m_vl.AddTail() = v;

		__super::VertexKick(skip);
	}

	__forceinline int ScissorTest(const GSVector4& p0, const GSVector4& p1)
	{
		GSVector4 scissor = m_context->scissor.ex;

		GSVector4 v0 = p0 < scissor;
		GSVector4 v1 = p1 > scissor.zwxy();

		return (v0 | v1).mask() & 3;
	}

	void DrawingKickPoint(GSVertexSW* v, int& count)
	{
		GSVector4 p0 = v[0].p;
		GSVector4 p1 = v[0].p;

		if(ScissorTest(p0, p1))
		{
			count = 0;
			return;
		}
	}
	
	void DrawingKickLine(GSVertexSW* v, int& count)
	{
		GSVector4 p0 = v[0].p.maxv(v[1].p);
		GSVector4 p1 = v[0].p.minv(v[1].p);

		if(ScissorTest(p0, p1))
		{
			count = 0;
			return;
		}

		if(PRIM->IIP == 0)
		{
			v[0].c = v[1].c;
		}
	}

	void DrawingKickTriangle(GSVertexSW* v, int& count)
	{
		GSVector4 p0 = v[0].p.maxv(v[1].p).maxv(v[2].p);
		GSVector4 p1 = v[0].p.minv(v[1].p).minv(v[2].p);

		if(ScissorTest(p0, p1))
		{
			count = 0;
			return;
		}

		if(PRIM->IIP == 0)
		{
			v[0].c = v[2].c;
			v[1].c = v[2].c;
		}
	}

	void DrawingKickSprite(GSVertexSW* v, int& count)
	{
		GSVector4 p0 = v[0].p.maxv(v[1].p);
		GSVector4 p1 = v[0].p.minv(v[1].p);

		if(ScissorTest(p0, p1))
		{
			count = 0;
			return;
		}
	}

	GSVector4i GetScissor()
	{
		GSVector4i v = GSVector4i(m_context->scissor.in);

		// TODO: find a game that overflows and check which one is the right behaviour

		v.z = min(v.z, (int)m_context->FRAME.FBW * 64);

		return v;
	}

	void Draw()
	{
		const GSDrawingEnvironment& env = m_env;
		const GSDrawingContext* context = m_context;
		const GS_PRIM prim = (GS_PRIM)PRIM->PRIM;
		const GS_PRIM_CLASS primclass = GSUtil::GetPrimClass(prim);

		//

		GSScanlineParam p;

		p.vm = m_mem.m_vm32;

		p.sel.dw = 0;

		p.sel.fpsm = 3;
		p.sel.zpsm = 3;
		p.sel.atst = ATST_ALWAYS;
		p.sel.tfx = TFX_NONE;
		p.sel.abe = 255;

		//

		p.fm = context->FRAME.FBMSK;
		p.zm = context->ZBUF.ZMSK || context->TEST.ZTE == 0 ? 0xffffffff : 0;

		// 

		if(context->TEST.ZTE && context->TEST.ZTST == ZTST_NEVER)
		{
			p.fm = 0xffffffff;
			p.zm = 0xffffffff;
		}

		if(context->TEST.ATE)
		{
			bool pass = context->TEST.ATST != ATST_NEVER;

			if((!PRIM->TME || !context->TEX0.TCC && context->TEX0.TFX != TFX_DECAL) && m_vtrace.eq.a)
			{
				// surprisingly large number of games leave alpha test on when alpha is constant

				DWORD a = (DWORD)((int)m_vtrace.v.c.a >> 7);
				DWORD aref = context->TEST.AREF;

				switch(context->TEST.ATST)
				{
				case ATST_NEVER: pass = false; break;
				case ATST_ALWAYS: pass = true; break;
				case ATST_LESS: pass = a < aref; break;
				case ATST_LEQUAL: pass = a <= aref; break;
				case ATST_EQUAL: pass = a == aref; break;
				case ATST_GEQUAL: pass = a >= aref; break;
				case ATST_GREATER: pass = a > aref; break;
				case ATST_NOTEQUAL: pass = a != aref; break;
				default: __assume(0);
				}
			}

			if(!pass)
			{
				switch(context->TEST.AFAIL)
				{
				case AFAIL_KEEP: p.fm = p.zm = 0xffffffff; break;
				case AFAIL_FB_ONLY: p.zm = 0xffffffff; break;
				case AFAIL_ZB_ONLY: p.fm = 0xffffffff; break;
				case AFAIL_RGB_ONLY: p.fm |= 0xff000000; p.zm = 0xffffffff; break;
				default: __assume(0);
				}

				// "don't care" values

				p.sel.atst = ATST_ALWAYS;
				p.sel.afail = 0;
			}
			else
			{
				p.sel.atst = context->TEST.ATST;
				p.sel.afail = context->TEST.AFAIL;
			}
		}

		bool fwrite = p.fm != 0xffffffff;
		bool ftest = p.sel.atst != ATST_ALWAYS || context->TEST.DATE && context->FRAME.PSM != PSM_PSMCT24;

		if(fwrite || ftest)
		{
			p.sel.fpsm = GSUtil::EncodePSM(context->FRAME.PSM);

			if((primclass == GS_LINE_CLASS || primclass == GS_TRIANGLE_CLASS) && m_vtrace.eq.rgba != 15)
			{
				p.sel.iip = PRIM->IIP;
			}

			if(PRIM->TME)
			{
				p.sel.tfx = context->TEX0.TFX;
				p.sel.tcc = context->TEX0.TCC;
				p.sel.fst = PRIM->FST;
				p.sel.ltf = context->TEX1.IsLinear();
				p.sel.tlu = GSLocalMemory::m_psm[context->TEX0.PSM].pal > 0;

				if(p.sel.iip == 0 && p.sel.tfx == TFX_MODULATE && p.sel.tcc)
				{
					if(m_vtrace.eq.rgba == 15 && (m_vtrace.v.c == GSVector4(128.0f * 128.0f)).alltrue())
					{
						// modulate does not do anything when vertex color is 0x80

						p.sel.tfx = TFX_DECAL;
					}
				}

				if(p.sel.fst == 0)
				{
					// skip per pixel division if q is constant

					GSVertexSW* v = m_vertices;

					if(m_vtrace.eq.q)
					{
						p.sel.fst = 1;

						GSVector4 w = v[0].t.zzzz().rcpnr();

						for(int i = 0, j = m_count; i < j; i++)
						{
							v[i].t *= w;
						}
					}
					else if(prim == GS_SPRITE)
					{
						p.sel.fst = 1;

						for(int i = 0, j = m_count; i < j; i += 2)
						{
							GSVector4 w = v[i + 1].t.zzzz().rcpnr();

							v[i + 0].t *= w;
							v[i + 1].t *= w;
						}
					}
				}

				if(p.sel.fst && p.sel.ltf)
				{
					// if q is constant we can do the half pel shift for bilinear sampling on the vertices

					GSVector4 half((float)0x8000, (float)0x8000, 0.0f, 0.0f);

					for(int i = 0; i < m_count; i++)
					{
						m_vertices[i].t -= half;
					}
				}

				int w = 1 << context->TEX0.TW;
				int h = 1 << context->TEX0.TH;

				int wms = context->CLAMP.WMS;
				int wmt = context->CLAMP.WMT;

				int minu = (int)context->CLAMP.MINU;
				int minv = (int)context->CLAMP.MINV;
				int maxu = (int)context->CLAMP.MAXU;
				int maxv = (int)context->CLAMP.MAXV;

				CRect r = CRect(0, 0, w, h);

				switch(wms)
				{
				case CLAMP_REPEAT: // TODO
					break;
				case CLAMP_CLAMP: // TODO
					break;
				case CLAMP_REGION_REPEAT:
					if(r.left < minu) r.left = minu;
					if(r.right > maxu + 1) r.right = maxu + 1;
					break;
				case CLAMP_REGION_CLAMP:
					r.left = maxu; 
					r.right = r.left + (minu + 1);
					break;
				default: 
					__assume(0);
				}

				switch(wmt)
				{
				case CLAMP_REPEAT: // TODO
					break;
				case CLAMP_CLAMP: // TODO
					break;
				case CLAMP_REGION_REPEAT:
					if(r.top < minv) r.top = minv;
					if(r.bottom > maxv + 1) r.bottom = maxv + 1;
					break;
				case CLAMP_REGION_CLAMP:
					r.top = maxv; 
					r.bottom = r.top + (minv + 1);
					break;
				default:
					__assume(0);
				}

				r &= CRect(0, 0, w, h);

				const GSTextureCacheSW::GSTexture* t = m_tc->Lookup(context->TEX0, env.TEXA, &r);

				if(!t) {ASSERT(0); return;}

				m_mem.m_clut.Read32(context->TEX0, env.TEXA);

				p.tex = t->m_buff;
				p.clut = m_mem.m_clut;
				p.tw = t->m_tw;
			}

			p.sel.fge = PRIM->FGE;

			if(context->FRAME.PSM != PSM_PSMCT24)
			{
				p.sel.date = context->TEST.DATE;
			}

			if(PRIM->ABE)
			{
				if(!context->ALPHA.IsOpaque())
				{
					p.sel.abe = context->ALPHA.ai32[0];
					p.sel.pabe = env.PABE.PABE;
				}
				else
				{
					// printf("opaque\n");
				}

				if(PRIM->AA1)
				{
					// TODO: automatic alpha blending (ABE=1, A=0 B=1 C=0 D=1)
				}
			}

			if(p.sel.date 
			|| p.sel.abe != 255 
			|| p.sel.atst != ATST_ALWAYS && p.sel.afail == AFAIL_RGB_ONLY 
			|| p.fm != 0 && p.fm != 0xffffffff)
			{
				p.sel.rfb = 1;
			}
		}

		bool zwrite = p.zm != 0xffffffff;
		bool ztest = context->TEST.ZTE && context->TEST.ZTST > 1;

		if(zwrite || ztest)
		{
			p.sel.zpsm = GSUtil::EncodePSM(context->ZBUF.PSM);
			p.sel.ztst = ztest ? context->TEST.ZTST : 1;
		}

		m_vtrace.Reset();

		if((p.fm & p.zm) == 0xffffffff)
		{
			return;
		}

		if(s_dump)
		{
/*
			TRACE(_T("\n"));

			TRACE(_T("PRIM = %d, ZMSK = %d, ZTE = %d, ZTST = %d, ATE = %d, ATST = %d, AFAIL = %d, AREF = %02x\n"), 
				PRIM->PRIM, context->ZBUF.ZMSK, 
				context->TEST.ZTE, context->TEST.ZTST,
				context->TEST.ATE, context->TEST.ATST, context->TEST.AFAIL, context->TEST.AREF);

			for(int i = 0; i < m_count; i++)
			{
				TRACE(_T("[%d] %3.0f %3.0f %3.0f %3.0f\n"), i, m_vertices[i].p.x, m_vertices[i].p.y, m_vertices[i].p.z, m_vertices[i].c.w / 128);
			}
*/
			CString str;
			str.Format(_T("c:\\temp1\\_%05d_f%I64d_tex_%05x_%d.bmp"), s_n++, m_perfmon.GetFrame(), (int)m_context->TEX0.TBP0, (int)m_context->TEX0.PSM);
			if(PRIM->TME) if(s_save) {m_mem.SaveBMP(str, m_context->TEX0.TBP0, m_context->TEX0.TBW, m_context->TEX0.PSM, 1 << m_context->TEX0.TW, 1 << m_context->TEX0.TH);}
			str.Format(_T("c:\\temp1\\_%05d_f%I64d_rt0_%05x_%d.bmp"), s_n++, m_perfmon.GetFrame(), m_context->FRAME.Block(), m_context->FRAME.PSM);
			if(s_save) {m_mem.SaveBMP(str, m_context->FRAME.Block(), m_context->FRAME.FBW, m_context->FRAME.PSM, GetFrameSize(1).cx, 512);}//GetFrameSize(1).cy);
			str.Format(_T("c:\\temp1\\_%05d_f%I64d_rz0_%05x_%d.bmp"), s_n-1, m_perfmon.GetFrame(), m_context->ZBUF.Block(), m_context->ZBUF.PSM);
			if(s_savez) {m_mem.SaveBMP(str, m_context->ZBUF.Block(), m_context->FRAME.FBW, m_context->ZBUF.PSM, GetFrameSize(1).cx, 512);}
		}

		//
/*
static FILE* s_fp = NULL;
static UINT64 s_frame = 0;
static __int64 s_total = 0;
if(!s_fp) s_fp = fopen("c:\\log.txt", "w");
UINT64 frame = m_perfmon.GetFrame();
if(s_frame != frame) 
{
	fprintf(s_fp, "=> %I64d\n", s_total); 
	s_frame = frame; 
	s_total = 0;
}
__int64 start = __rdtsc();
*/
		GSRasterizerData data;

		data.scissor = GetScissor();
		data.primclass = primclass;
		data.vertices = m_vertices;
		data.count = m_count;
		data.param = &p;

		int prims = m_rl.Draw(&data);
		
		m_perfmon.Put(GSPerfMon::Prim, prims);
		m_perfmon.Put(GSPerfMon::Draw, 1);
		
		int pixels = m_rl.GetPixels();

		m_perfmon.Put(GSPerfMon::Fillrate, pixels);

/*
__int64 diff = __rdtsc() - start;
s_total += diff;
//if(pixels > 50000)
fprintf(s_fp, "[%I64d, %d, %d, %d] %08x, diff = %I64d /prim = %I64d /pixel = %I64d \n", frame, PRIM->PRIM, prims, pixels, p.sel, diff, diff / prims, pixels > 0 ? diff / pixels : 0);
*/
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

			GSVector4i scissor = data.scissor;

			CRect r;

			r.left = max(scissor.x, min(scissor.z, (int)tl.x));
			r.top = max(scissor.y, min(scissor.w, (int)tl.y));
			r.right = max(scissor.x, min(scissor.z, (int)br.x));
			r.bottom = max(scissor.y, min(scissor.w, (int)br.y));

			GIFRegBITBLTBUF BITBLTBUF;

			BITBLTBUF.DBW = context->FRAME.FBW;

			if(p.fm != 0xffffffff)
			{
				BITBLTBUF.DBP = context->FRAME.Block();
				BITBLTBUF.DPSM = context->FRAME.PSM;

				m_tc->InvalidateVideoMem(BITBLTBUF, r);
			}

			if(p.zm != 0xffffffff)
			{
				BITBLTBUF.DBP = context->ZBUF.Block();
				BITBLTBUF.DPSM = context->ZBUF.PSM;

				m_tc->InvalidateVideoMem(BITBLTBUF, r);
			}
		}

		if(s_dump)
		{
			CString str;
			str.Format(_T("c:\\temp1\\_%05d_f%I64d_rt1_%05x_%d_%d.bmp"), s_n++, m_perfmon.GetFrame(), m_context->FRAME.Block(), m_context->FRAME.PSM);
			if(s_save) {m_mem.SaveBMP(str, m_context->FRAME.Block(), m_context->FRAME.FBW, m_context->FRAME.PSM, GetFrameSize(1).cx, 512);}//GetFrameSize(1).cy);
			str.Format(_T("c:\\temp1\\_%05d_f%I64d_rz1_%05x_%d.bmp"), s_n-1, m_perfmon.GetFrame(), m_context->ZBUF.Block(), m_context->ZBUF.PSM);
			if(s_savez) {m_mem.SaveBMP(str, m_context->ZBUF.Block(), m_context->FRAME.FBW, m_context->ZBUF.PSM, GetFrameSize(1).cx, 512);}
		}
	}

	void InvalidateVideoMem(const GIFRegBITBLTBUF& BITBLTBUF, CRect r)
	{
		m_tc->InvalidateVideoMem(BITBLTBUF, r);
	}

public:
	GSRendererSW(BYTE* base, bool mt, void (*irq)(), int nloophack, const GSRendererSettings& rs, int threads)
		: GSRendererT(base, mt, irq, nloophack, rs)
	{
		m_rl.Create<GSDrawScanline>(this, threads);

		m_tc = new GSTextureCacheSW(this);

		m_fpDrawingKickHandlers[GS_POINTLIST] = (DrawingKickHandler)&GSRendererSW::DrawingKickPoint;
		m_fpDrawingKickHandlers[GS_LINELIST] = (DrawingKickHandler)&GSRendererSW::DrawingKickLine;
		m_fpDrawingKickHandlers[GS_LINESTRIP] = (DrawingKickHandler)&GSRendererSW::DrawingKickLine;
		m_fpDrawingKickHandlers[GS_TRIANGLELIST] = (DrawingKickHandler)&GSRendererSW::DrawingKickTriangle;
		m_fpDrawingKickHandlers[GS_TRIANGLESTRIP] = (DrawingKickHandler)&GSRendererSW::DrawingKickTriangle;
		m_fpDrawingKickHandlers[GS_TRIANGLEFAN] = (DrawingKickHandler)&GSRendererSW::DrawingKickTriangle;
		m_fpDrawingKickHandlers[GS_SPRITE] = (DrawingKickHandler)&GSRendererSW::DrawingKickSprite;
	}

	virtual ~GSRendererSW()
	{
		delete m_tc;
	}

	GSRasterizer* GetRasterizer()
	{
		return m_rl.GetHead();
	}
};
