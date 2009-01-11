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
	GSLocalMemory::Offset* m_fbo;
	GSLocalMemory::Offset* m_zbo;
	GSLocalMemory::Offset4* m_fzbo;

	__declspec(align(16)) struct VertexTrace
	{
		GSVector4 tmin, tmax;
		GSVector4 cmin, cmax;

		union
		{
			DWORD value; 
			struct {DWORD s:1, t:1, q:1, _pad:1, r:1, g:1, b:1, a:1;};
			struct {DWORD stq:4, rgba:4;};
		} eq;

		void Reset()
		{
			tmin = GSVector4(FLT_MAX);
			tmax = GSVector4(-FLT_MAX);
			cmin = GSVector4(FLT_MAX);
			cmax = GSVector4::zero();
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

		// if((m_perfmon.GetFrame() & 255) == 0) m_rl.PrintStats();
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
			float q;

			if(PRIM->FST)
			{
				v.t = GSVector4(GSVector4i((int)m_v.UV.U, (int)m_v.UV.V, 0, 0) << (16 - 4));
				q = 1.0f;
			}
			else
			{
				v.t = GSVector4(m_v.ST.S, m_v.ST.T, 0.0f, 0.0f);
				v.t *= GSVector4(0x10000 << m_context->TEX0.TW, 0x10000 << m_context->TEX0.TH);
				q = m_v.RGBAQ.Q;
			}

			v.t = v.t.xyxy(GSVector4::load(q));
		}

		m_vl.AddTail() = v;

		__super::VertexKick(skip);
	}

	template<int primclass>
	void DrawingKick(GSVertexSW* v, int& count)
	{
		GSVector4 p0, p1;

		switch(primclass)
		{
		case GS_POINT_CLASS:
			p0 = v[0].p;
			p1 = v[0].p;
			break;
		case GS_LINE_CLASS:
			p0 = v[0].p.maxv(v[1].p);
			p1 = v[0].p.minv(v[1].p);
			break;
		case GS_TRIANGLE_CLASS:
			p0 = v[0].p.maxv(v[1].p).maxv(v[2].p);
			p1 = v[0].p.minv(v[1].p).minv(v[2].p);
			break;
		case GS_SPRITE_CLASS:
			p0 = v[0].p.maxv(v[1].p);
			p1 = v[0].p.minv(v[1].p);
			break;
		}

		GSVector4 scissor = m_context->scissor.ex;

		GSVector4 v0 = p0 < scissor;
		GSVector4 v1 = p1 > scissor.zwxy();

		if((v0 | v1).mask() & 3)
		{
			count = 0;
			return;
		}

		switch(primclass)
		{
		case GS_POINT_CLASS:
			m_vtrace.cmin = m_vtrace.cmin.minv(v[0].c);
			m_vtrace.cmax = m_vtrace.cmax.maxv(v[0].c);
			m_vtrace.tmin = m_vtrace.tmin.minv(v[0].t);
			m_vtrace.tmax = m_vtrace.tmax.maxv(v[0].t);
			break;
		case GS_LINE_CLASS:
			if(PRIM->IIP == 0) {v[0].c = v[1].c;}
			m_vtrace.cmin = m_vtrace.cmin.minv(v[0].c).minv(v[1].c);
			m_vtrace.cmax = m_vtrace.cmax.maxv(v[0].c).maxv(v[1].c);
			m_vtrace.tmin = m_vtrace.tmin.minv(v[0].t).minv(v[1].t);
			m_vtrace.tmax = m_vtrace.tmax.maxv(v[0].t).maxv(v[1].t);
			break;
		case GS_TRIANGLE_CLASS:
			if(PRIM->IIP == 0) {v[0].c = v[2].c; v[1].c = v[2].c;}
			m_vtrace.cmin = m_vtrace.cmin.minv(v[0].c).minv(v[1].c).minv(v[2].c);
			m_vtrace.cmax = m_vtrace.cmax.maxv(v[0].c).maxv(v[1].c).maxv(v[2].c);
			m_vtrace.tmin = m_vtrace.tmin.minv(v[0].t).minv(v[1].t).minv(v[2].t);
			m_vtrace.tmax = m_vtrace.tmax.maxv(v[0].t).maxv(v[1].t).maxv(v[2].t);
			break;
		case GS_SPRITE_CLASS:
			m_vtrace.cmin = m_vtrace.cmin.minv(v[1].c);
			m_vtrace.cmax = m_vtrace.cmax.maxv(v[1].c);
			m_vtrace.tmin = m_vtrace.tmin.minv(v[0].t).minv(v[1].t);
			m_vtrace.tmax = m_vtrace.tmax.maxv(v[0].t).maxv(v[1].t);
			break;
		}
	}

	GSVector4i GetScissor()
	{
		GSVector4i v = GSVector4i(m_context->scissor.in);

		// TODO: find a game that overflows and check which one is the right behaviour

		v.z = min(v.z, (int)m_context->FRAME.FBW * 64);

		return v;
	}

	bool TryAlphaTest(DWORD& fm, DWORD& zm)
	{
		const GSDrawingEnvironment& env = m_env;
		const GSDrawingContext* context = m_context;

		bool pass = true;

		if(context->TEST.ATST == ATST_NEVER)
		{
			pass = false;
		}
		else if(context->TEST.ATST != ATST_ALWAYS)
		{
			GSVector4i a = GSVector4i(m_vtrace.cmin.wwww(m_vtrace.cmax)) >> 7;

			int amin, amax;

			if(PRIM->TME && (context->TEX0.TCC || context->TEX0.TFX == TFX_DECAL))
			{
				DWORD bpp = GSLocalMemory::m_psm[context->TEX0.PSM].trbpp;
				DWORD cbpp = GSLocalMemory::m_psm[context->TEX0.CPSM].trbpp;
				DWORD pal = GSLocalMemory::m_psm[context->TEX0.PSM].pal;

				if(bpp == 32)
				{
					return false;
				}
				else if(bpp == 24)
				{
					amin = env.TEXA.AEM ? 0 : env.TEXA.TA0;
					amax = env.TEXA.TA0;
				}
				else if(bpp == 16)
				{
					amin = env.TEXA.AEM ? 0 : min(env.TEXA.TA0, env.TEXA.TA1);
					amax = max(env.TEXA.TA0, env.TEXA.TA1);
				}
				else
				{
					m_mem.m_clut.GetAlphaMinMax32(amin, amax);
				}

				switch(context->TEX0.TFX)
				{
				case TFX_MODULATE:
					amin = (amin * a.x) >> 7;
					amax = (amax * a.z) >> 7;
					if(amin > 255) amin = 255;
					if(amax > 255) amax = 255;
					break;
				case TFX_DECAL:
					break;
				case TFX_HIGHLIGHT:
					amin = amin + a.x;
					amax = amax + a.z;
					if(amin > 255) amin = 255;
					if(amax > 255) amax = 255;
					break;
				case TFX_HIGHLIGHT2:
					break;
				default:
					__assume(0);
				}
			}
			else
			{
				amin = a.x;
				amax = a.z;
			}

			int aref = context->TEST.AREF;

			switch(context->TEST.ATST)
			{
			case ATST_NEVER: 
				pass = false; 
				break;
			case ATST_ALWAYS: 
				pass = true; 
				break;
			case ATST_LESS: 
				if(amax < aref) pass = true;
				else if(amin >= aref) pass = false;
				else return false;
				break;
			case ATST_LEQUAL: 
				if(amax <= aref) pass = true;
				else if(amin > aref) pass = false;
				else return false;
				break;
			case ATST_EQUAL: 
				if(amin == aref && amax == aref) pass = true;
				else if(amin > aref || amax < aref) pass = false;
				else return false;
				break;
			case ATST_GEQUAL: 
				if(amin >= aref) pass = true;
				else if(amax < aref) pass = false;
				else return false;
				break;
			case ATST_GREATER: 
				if(amin > aref) pass = true;
				else if(amax <= aref) pass = false;
				else return false;
				break;
			case ATST_NOTEQUAL: 
				if(amin == aref && amax == aref) pass = false;
				else if(amin > aref || amax < aref) pass = true;
				else return false;
				break;
			default: 
				__assume(0);
			}
		}

		if(!pass)
		{
			switch(context->TEST.AFAIL)
			{
			case AFAIL_KEEP: fm = zm = 0xffffffff; break;
			case AFAIL_FB_ONLY: zm = 0xffffffff; break;
			case AFAIL_ZB_ONLY: fm = 0xffffffff; break;
			case AFAIL_RGB_ONLY: fm |= 0xff000000; zm = 0xffffffff; break;
			default: __assume(0);
			}
		}

		return true;
	}

	void Draw()
	{
		const GSDrawingEnvironment& env = m_env;
		const GSDrawingContext* context = m_context;
		const GS_PRIM prim = (GS_PRIM)PRIM->PRIM;
		const GS_PRIM_CLASS primclass = GSUtil::GetPrimClass(prim);

		//

		m_vtrace.eq.value = ((m_vtrace.tmin == m_vtrace.tmax).mask() | ((m_vtrace.cmin == m_vtrace.cmax).mask() << 4));

		//

		if(PRIM->TME)
		{
			m_mem.m_clut.Read32(context->TEX0, env.TEXA);
		}

		//

		GSScanlineParam p;

		p.vm = m_mem.m_vm32;

		m_fbo = m_mem.GetOffset(context->FRAME.Block(), context->FRAME.FBW, context->FRAME.PSM, m_fbo);
		m_zbo = m_mem.GetOffset(context->ZBUF.Block(), context->FRAME.FBW, context->ZBUF.PSM, m_zbo);
		m_fzbo = m_mem.GetOffset4(context->FRAME, context->ZBUF, m_fzbo);

		p.fbo = m_fbo;
		p.zbo = m_zbo;
		p.fzbo = m_fzbo;

		p.sel.dw = 0;

		p.sel.fpsm = 3;
		p.sel.zpsm = 3;
		p.sel.atst = ATST_ALWAYS;
		p.sel.tfx = TFX_NONE;
		p.sel.abe = 255;

		//

		p.fm = context->FRAME.FBMSK;
		p.zm = context->ZBUF.ZMSK || context->TEST.ZTE == 0 ? 0xffffffff : 0;

		if(context->TEST.ZTE && context->TEST.ZTST == ZTST_NEVER)
		{
			p.fm = 0xffffffff;
			p.zm = 0xffffffff;
		}

		if(context->TEST.ATE)
		{
			if(!TryAlphaTest(p.fm, p.zm))
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
					if(m_vtrace.eq.rgba == 15 && (m_vtrace.cmin == GSVector4(128.0f * 128.0f)).alltrue())
					{
						// modulate does not do anything when vertex color is 0x80

						p.sel.tfx = TFX_DECAL;
					}
				}

				if(p.sel.tfx == TFX_DECAL)
				{
					p.sel.tcc = 1;
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

					GSVertexSW* v = m_vertices;

					GSVector4 half(0x8000, 0x8000, 0, 0);

					for(int i = 0, j = m_count; i < j; i++)
					{
						v[i].t -= half;
					}
				}

				CRect r;
				
				int w = 1 << context->TEX0.TW;
				int h = 1 << context->TEX0.TH;

				MinMaxUV(w, h, r, p.sel.fst);

				const GSTextureCacheSW::GSTexture* t = m_tc->Lookup(context->TEX0, env.TEXA, &r);

				if(!t) {ASSERT(0); return;}

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
			|| p.sel.abea == 1 || p.sel.abeb == 1 || p.sel.abec == 1 || p.sel.abed == 1
			|| p.sel.atst != ATST_ALWAYS && p.sel.afail == AFAIL_RGB_ONLY 
			|| p.sel.fpsm == 0 && p.fm != 0 && p.fm != 0xffffffff
			|| p.sel.fpsm == 1 && (p.fm & 0x00ffffff) != 0 && (p.fm & 0x00ffffff) != 0x00ffffff
			|| p.sel.fpsm == 2 && (p.fm & 0x80f8f8f8) != 0 && (p.fm & 0x80f8f8f8) != 0x80f8f8f8)
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

		m_rl.Draw(&data);

		GSRasterizerStats stats;

		m_rl.GetStats(stats);
	
		m_perfmon.Put(GSPerfMon::Draw, 1);
		m_perfmon.Put(GSPerfMon::Prim, stats.prims);
		m_perfmon.Put(GSPerfMon::Fillrate, stats.pixels);
/*
__int64 diff = __rdtsc() - start;
s_total += diff;
if(PRIM->TME)
if(stats.pixels >= 50000 || diff >= 1000000)
fprintf(s_fp, "[%I64d, %d, %d, %d, %d] %08x, diff = %I64d /prim = %I64d /pixel = %I64d\n", 
		frame, PRIM->PRIM, stats.prims, stats.pixels, s_n, p.sel, diff, diff / stats.prims, stats.pixels > 0 ? diff / stats.pixels : 0);
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
			str.Format(_T("c:\\temp1\\_%05d_f%I64d_rt1_%05x_%d.bmp"), s_n++, m_perfmon.GetFrame(), m_context->FRAME.Block(), m_context->FRAME.PSM);
			if(s_save) {m_mem.SaveBMP(str, m_context->FRAME.Block(), m_context->FRAME.FBW, m_context->FRAME.PSM, GetFrameSize(1).cx, 512);}//GetFrameSize(1).cy);
			str.Format(_T("c:\\temp1\\_%05d_f%I64d_rz1_%05x_%d.bmp"), s_n-1, m_perfmon.GetFrame(), m_context->ZBUF.Block(), m_context->ZBUF.PSM);
			if(s_savez) {m_mem.SaveBMP(str, m_context->ZBUF.Block(), m_context->FRAME.FBW, m_context->ZBUF.PSM, GetFrameSize(1).cx, 512);}
		}
	}

	void InvalidateVideoMem(const GIFRegBITBLTBUF& BITBLTBUF, CRect r)
	{
		m_tc->InvalidateVideoMem(BITBLTBUF, r);
	}

	void MinMaxUV(int w, int h, CRect& r, bool fst)
	{
		const GSDrawingContext* context = m_context;

		int wms = context->CLAMP.WMS;
		int wmt = context->CLAMP.WMT;

		int minu = (int)context->CLAMP.MINU;
		int minv = (int)context->CLAMP.MINV;
		int maxu = (int)context->CLAMP.MAXU;
		int maxv = (int)context->CLAMP.MAXV;

		GSVector4i vr(0, 0, w, h);

		switch(wms)
		{
		case CLAMP_REPEAT:
			break;
		case CLAMP_CLAMP:
			break;
		case CLAMP_REGION_CLAMP:
			if(vr.x < minu) vr.x = minu;
			if(vr.z > maxu + 1) vr.z = maxu + 1;
			break;
		case CLAMP_REGION_REPEAT:
			vr.x = maxu; 
			vr.z = vr.x + (minu + 1);
			break;
		default: 
			__assume(0);
		}

		switch(wmt)
		{
		case CLAMP_REPEAT:
			break;
		case CLAMP_CLAMP:
			break;
		case CLAMP_REGION_CLAMP:
			if(vr.y < minv) vr.y = minv;
			if(vr.w > maxv + 1) vr.w = maxv + 1;
			break;
		case CLAMP_REGION_REPEAT:
			vr.y = maxv; 
			vr.w = vr.y + (minv + 1);
			break;
		default:
			__assume(0);
		}

		if(fst)
		{
			GSVector4i v = GSVector4i(m_vtrace.tmin.xyxy(m_vtrace.tmax) / (m_vtrace.tmin.zzzz() * 0x10000));

			switch(wms)
			{
			case CLAMP_REPEAT: // TODO
				break;
			case CLAMP_CLAMP:
			case CLAMP_REGION_CLAMP:
				if(vr.x < v.x) vr.x = v.x;
				if(vr.z > v.z + 1) vr.z = v.z + 1;
				break;
			case CLAMP_REGION_REPEAT: // TODO
				break;
			default:
				__assume(0);
			}

			switch(wmt)
			{
			case CLAMP_REPEAT: // TODO
				break;
			case CLAMP_CLAMP:
			case CLAMP_REGION_CLAMP:
				if(vr.y < v.y) vr.y = v.y;
				if(vr.w > v.w + 1) vr.w = v.w + 1;
				break;
			case CLAMP_REGION_REPEAT: // TODO
				break;
			default:
				__assume(0);
			}
		}

		r = vr;

		r &= CRect(0, 0, w, h);
	}

public:
	GSRendererSW(BYTE* base, bool mt, void (*irq)(), int nloophack, const GSRendererSettings& rs, int threads)
		: GSRendererT(base, mt, irq, nloophack, rs)
		, m_fbo(NULL)
		, m_zbo(NULL)
		, m_fzbo(NULL)
	{
		m_rl.Create<GSDrawScanline>(this, threads);

		m_tc = new GSTextureCacheSW(this);

		m_fpDrawingKickHandlers[GS_POINTLIST] = (DrawingKickHandler)&GSRendererSW::DrawingKick<GS_POINT_CLASS>;
		m_fpDrawingKickHandlers[GS_LINELIST] = (DrawingKickHandler)&GSRendererSW::DrawingKick<GS_LINE_CLASS>;
		m_fpDrawingKickHandlers[GS_LINESTRIP] = (DrawingKickHandler)&GSRendererSW::DrawingKick<GS_LINE_CLASS>;
		m_fpDrawingKickHandlers[GS_TRIANGLELIST] = (DrawingKickHandler)&GSRendererSW::DrawingKick<GS_TRIANGLE_CLASS>;
		m_fpDrawingKickHandlers[GS_TRIANGLESTRIP] = (DrawingKickHandler)&GSRendererSW::DrawingKick<GS_TRIANGLE_CLASS>;
		m_fpDrawingKickHandlers[GS_TRIANGLEFAN] = (DrawingKickHandler)&GSRendererSW::DrawingKick<GS_TRIANGLE_CLASS>;
		m_fpDrawingKickHandlers[GS_SPRITE] = (DrawingKickHandler)&GSRendererSW::DrawingKick<GS_SPRITE_CLASS>;
	}

	virtual ~GSRendererSW()
	{
		delete m_tc;
	}
};
