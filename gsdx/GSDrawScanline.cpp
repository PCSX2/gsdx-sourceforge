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
#include "GSDrawScanline.h"
#include "GSTextureCacheSW.h"

GSDrawScanline::GSDrawScanline(GSState* state)
	: m_state(state)
	, m_fbo(NULL)
	, m_zbo(NULL)
{
	Init();
}

GSDrawScanline::~GSDrawScanline()
{
	FreeOffsets();
}

// IDrawScanline

bool GSDrawScanline::SetupDraw(const GSRasterizerData* data)
{
	GSDrawingEnvironment& env = m_state->m_env;
	GSDrawingContext* context = m_state->m_context;

	const GSScanlineParam* p = (const GSScanlineParam*)data->param;

	m_env.sel = p->sel;

	SetupOffset(m_fbo, context->FRAME.Block(), context->FRAME.FBW, context->FRAME.PSM);
	SetupOffset(m_zbo, context->ZBUF.Block(), context->FRAME.FBW, context->ZBUF.PSM);

	m_env.vm = p->vm;
	m_env.fbr = m_fbo->row;
	m_env.zbr = m_zbo->row;
	m_env.fbc = m_fbo->col;
	m_env.zbc = m_zbo->col;
	m_env.fm = GSVector4i(p->fm);
	m_env.zm = GSVector4i(p->zm);
	m_env.datm = GSVector4i(context->TEST.DATM ? 0x80000000 : 0);
	m_env.colclamp = GSVector4i(env.COLCLAMP.CLAMP ? 0xffffffff : 0x00ff00ff);
	m_env.fba = GSVector4i(context->FBA.FBA ? 0x80000000 : 0);
	m_env.aref = GSVector4i((int)context->TEST.AREF);
	m_env.afix = GSVector4i((int)context->ALPHA.FIX << 16);
	m_env.afix2 = m_env.afix.yywwl().yywwh().sll16(7);
	m_env.frb = GSVector4i((int)env.FOGCOL.ai32[0] & 0x00ff00ff);
	m_env.fga = GSVector4i((int)(env.FOGCOL.ai32[0] >> 8) & 0x00ff00ff);

	if(m_env.sel.fpsm == 1)
	{
		m_env.fm |= GSVector4i::xff000000();
	}

	if(m_env.sel.atst == ATST_LESS)
	{
		m_env.sel.atst = ATST_LEQUAL;

		m_env.aref -= GSVector4i::x00000001();
	}
	else if(m_env.sel.atst == ATST_GREATER)
	{
		m_env.sel.atst = ATST_GEQUAL;

		m_env.aref += GSVector4i::x00000001();
	}

	if(m_env.sel.tfx != TFX_NONE)
	{
		m_env.tex = p->tex;
		m_env.clut = p->clut;
		m_env.tw = p->tw;

		short tw = (short)(1 << context->TEX0.TW);
		short th = (short)(1 << context->TEX0.TH);

		switch(context->CLAMP.WMS)
		{
		case CLAMP_REPEAT: 
			m_env.t.min.u16[0] = tw - 1;
			m_env.t.max.u16[0] = 0;
			m_env.t.mask.u32[0] = 0xffffffff; 
			break;
		case CLAMP_CLAMP: 
			m_env.t.min.u16[0] = 0;
			m_env.t.max.u16[0] = tw - 1;
			m_env.t.mask.u32[0] = 0; 
			break;
		case CLAMP_REGION_REPEAT: 
			m_env.t.min.u16[0] = context->CLAMP.MINU;
			m_env.t.max.u16[0] = context->CLAMP.MAXU;
			m_env.t.mask.u32[0] = 0; 
			break;
		case CLAMP_REGION_CLAMP: 
			m_env.t.min.u16[0] = context->CLAMP.MINU;
			m_env.t.max.u16[0] = context->CLAMP.MAXU;
			m_env.t.mask.u32[0] = 0xffffffff; 
			break;
		default: 
			__assume(0);
		}

		switch(context->CLAMP.WMT)
		{
		case CLAMP_REPEAT: 
			m_env.t.min.u16[4] = th - 1;
			m_env.t.max.u16[4] = 0;
			m_env.t.mask.u32[2] = 0xffffffff; 
			break;
		case CLAMP_CLAMP: 
			m_env.t.min.u16[4] = 0;
			m_env.t.max.u16[4] = th - 1;
			m_env.t.mask.u32[2] = 0; 
			break;
		case CLAMP_REGION_REPEAT: 
			m_env.t.min.u16[4] = context->CLAMP.MINV;
			m_env.t.max.u16[4] = context->CLAMP.MAXV;
			m_env.t.mask.u32[2] = 0; 
			break;
		case CLAMP_REGION_CLAMP: 
			m_env.t.min.u16[4] = context->CLAMP.MINV;
			m_env.t.max.u16[4] = context->CLAMP.MAXV;
			m_env.t.mask.u32[2] = 0xffffffff; 
			break;
		default: 
			__assume(0);
		}

		m_env.t.min = m_env.t.min.xxxxl().xxxxh();
		m_env.t.max = m_env.t.max.xxxxl().xxxxh();
		m_env.t.mask = m_env.t.mask.xxzz();
	}

	//

	m_dsf = m_ds[m_env.sel.fpsm][m_env.sel.zpsm][m_env.sel.ztst][m_env.sel.iip];

	CRBMap<DWORD, DrawScanlinePtr>::CPair* pair = m_dsmap2.Lookup(m_env.sel);

	if(pair)
	{
		m_dsf = pair->m_value;
	}
	else
	{
		static int found = 0;

		pair = m_dsmap.Lookup(m_env.sel);

		if(pair && pair->m_value)
		{
			m_dsf = pair->m_value;

			m_dsmap2.SetAt(pair->m_key, pair->m_value);

			found++;

		}
		else if(!pair)
		{
			_tprintf(
				_T("*** [%d,%d] ")
				_T("psm %d/%d ztst %d iip %d ")
				_T("tfx %d tcc %d fst %d ltf %d tlu %d ")
				_T("atst %d afail %d fge %d date %d ")
				_T("abe %02x pabe %d rfb %d ")
				_T("\n"), 
				m_dsmap.GetCount(), found,
				m_env.sel.fpsm, m_env.sel.zpsm, m_env.sel.ztst, m_env.sel.iip,
				m_env.sel.tfx, m_env.sel.tcc, m_env.sel.fst, m_env.sel.ltf, m_env.sel.tlu,
				m_env.sel.atst, m_env.sel.afail, m_env.sel.fge, m_env.sel.date,
				m_env.sel.abe, m_env.sel.pabe, m_env.sel.rfb);

			m_dsmap.SetAt(m_env.sel, NULL);

			if(FILE* fp = _tfopen(_T("c:\\1.txt"), _T("w")))
			{
				POSITION pos = m_dsmap.GetHeadPosition();

				while(pos) 
				{
					pair = m_dsmap.GetNext(pos);

					if(!pair->m_value)
					{
						_ftprintf(fp, _T("m_dsmap.SetAt(0x%08x, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x%08x>);\n"), pair->m_key, pair->m_key);
					}
				}

				fclose(fp);
			}
		}
	}

	if(data->primclass == GS_SPRITE_CLASS)
	{
		GIFRegPRIM* PRIM = m_state->PRIM;

		bool solid = true;

		if(PRIM->IIP || PRIM->TME || PRIM->ABE || PRIM->FGE
		|| context->TEST.ZTE && context->TEST.ZTST != ZTST_ALWAYS 
		|| context->TEST.ATE && context->TEST.ATST != ATST_ALWAYS
		|| context->TEST.DATE
		|| env.DTHE.DTHE
		|| context->FRAME.FBMSK)
		{
			solid = false;
		}
/*
		// TODO

		bool solid2 = true;

		if(m_env.sel.iip 
		|| m_env.sel.tfx != TFX_NONE
		|| m_env.sel.abe != 255 
		|| m_env.sel.fge
		|| m_env.sel.ztst > 1 
		|| m_env.sel.atst > 1
		|| m_env.sel.date
		|| p->fm != 0 && p->fm != 0xffffffff // TODO: implement masked FillRect
		)
		{
			solid2 = false;
		}

		if(solid != solid2)
		{
			int pixels = 0;

			for(int i = 0; i < data->count; i += 2)
			{
				const GSVector4& p0 = data->vertices[i + 0].p;
				const GSVector4& p1 = data->vertices[i + 1].p;

				GSVector4 v = (p1 - p0).abs();

				pixels += (int)v.x * (int)v.y;
			}

			printf("[%I64d] %d != %d, %08x, %d\n", m_state->m_perfmon.GetFrame(), solid, solid2, p->fm, pixels);
		}
*/
		return solid;
	}

	return false;
}

void GSDrawScanline::SetupPrim(GS_PRIM_CLASS primclass, const GSVertexSW* vertices, const GSVertexSW& dscan)
{
	GSVector4 ps0123 = GSVector4::ps0123();
	
	GSVector4 dp = dscan.p;
	GSVector4 dt = dscan.t;
	GSVector4 dc = dscan.c;

	if(m_env.sel.ztst)
	{
		GSVector4 dz = dp.zzzz();

		m_env.dz = dz * ps0123;
		m_env.dz4 = dz * 4.0;
	}

	if(m_env.sel.fge)
	{
		GSVector4 df = dp.wwww();
		GSVector4i dfi = GSVector4i(df * ps0123).ps32(GSVector4i(df * 4.0f));

		m_env.df = dfi.upl16(dfi);
		m_env.df4 = dfi.uph16(dfi);
	}

	if(m_env.sel.tfx != TFX_NONE)
	{
		m_env.dt = dt;
		m_env.dt4 = dt * 4.0;
	}

	if(m_env.sel.iip)
	{
		GSVector4i drg = GSVector4i(dc.xxxx() * ps0123).ps32(GSVector4i(dc.yyyy() * ps0123));
		GSVector4i dba = GSVector4i(dc.zzzz() * ps0123).ps32(GSVector4i(dc.wwww() * ps0123));
		GSVector4i dc4 = GSVector4i(dc * 4.0f).ps32().xzywl();

		m_env.drb = drg.upl16(dba);
		m_env.dga = drg.uph16(dba);
		m_env.dc4 = dc4;
	}
}

void GSDrawScanline::DrawScanline(int top, int left, int right, const GSVertexSW& v)
{
	(this->*m_dsf)(top, left, right, v);
}

void GSDrawScanline::DrawSolidRect(const GSVector4i& r, const GSVertexSW& v)
{
	ASSERT(r.y >= 0);
	ASSERT(r.w >= 0);

	GSDrawingContext* context = m_state->m_context;

	DWORD fbp = context->FRAME.Block();
	DWORD fpsm = context->FRAME.PSM;
	DWORD zbp = context->ZBUF.Block();
	DWORD zpsm = context->ZBUF.PSM;
	DWORD bw = context->FRAME.FBW;

	if(m_env.fm.u32[0] != 0xffffffff) // TODO
	{
		DWORD c = (GSVector4i(v.c) >> 7).rgba32();

		if(context->FBA.FBA)
		{
			c |= 0x80000000;
		}
		
		if(fpsm == PSM_PSMCT16 || fpsm == PSM_PSMCT16S)
		{
			c = ((c & 0xf8) >> 3) | ((c & 0xf800) >> 6) | ((c & 0xf80000) >> 9) | ((c & 0x80000000) >> 16);
		}

		m_state->m_mem.FillRect(r, c, fpsm, fbp, bw);
	}

	if(m_env.zm.u32[0] != 0xffffffff)
	{
		m_state->m_mem.FillRect(r, (DWORD)(float)v.p.z, zpsm, zbp, bw);
	}
}

IDrawScanline::DrawScanlinePtr GSDrawScanline::GetDrawScanlinePtr()
{
	return m_dsf;
}

void GSDrawScanline::SetupOffset(Offset*& o, DWORD bp, DWORD bw, DWORD psm)
{
	if(bw == 0) {ASSERT(0); return;}

	DWORD hash = bp | (bw << 14) | (psm << 20);

	if(!o || o->hash != hash)
	{
		CRBMap<DWORD, Offset*>::CPair* pair = m_omap.Lookup(hash);

		if(pair)
		{
			o = pair->m_value;
		}
		else
		{
			o = (Offset*)_aligned_malloc(sizeof(Offset), 16);

			o->hash = hash;

			GSLocalMemory::pixelAddress pa = GSLocalMemory::m_psm[psm].pa;

			for(int i = 0, j = 1024; i < j; i++)
			{
				o->row[i] = GSVector4i((int)pa(0, i, bp, bw));
			}

			int* p = (int*)_aligned_malloc(sizeof(int) * (2048 + 3) * 4, 16);

			for(int i = 0; i < 4; i++)
			{
				o->col[i] = &p[2048 * i + ((4 - (i & 3)) & 3)];

				memcpy(o->col[i], GSLocalMemory::m_psm[psm].rowOffset[0], sizeof(int) * 2048);
			}

			m_omap.SetAt(hash, o);
		}
	}
}

void GSDrawScanline::FreeOffsets()
{
	POSITION pos = m_omap.GetHeadPosition();

	while(pos)
	{
		Offset* o = m_omap.GetNextValue(pos);

		for(int i = 0; i < countof(o->col); i++)
		{
			_aligned_free(o->col);
		}

		_aligned_free(o);
	}

	m_omap.RemoveAll();
}

GSVector4i GSDrawScanline::Wrap(const GSVector4i& t)
{
	GSVector4i clamp = t.sat_i16(m_env.t.min, m_env.t.max);
	GSVector4i repeat = (t & m_env.t.min) | m_env.t.max;

	return clamp.blend8(repeat, m_env.t.mask);
}

void GSDrawScanline::SampleTexture(int pixels, DWORD ztst, DWORD fst, DWORD ltf, DWORD tlu, const GSVector4i& test, const GSVector4& s, const GSVector4& t, const GSVector4& q, GSVector4i* c)
{
	const void* RESTRICT tex = m_env.tex;
	const DWORD* RESTRICT clut = m_env.clut;
	const DWORD tw = m_env.tw;

	GSVector4 u = s;
	GSVector4 v = t;

	if(!fst)
	{
		GSVector4 w = q.rcp();

		u *= w;
		v *= w;

		if(ltf)
		{
			u -= 0x8000;
			v -= 0x8000;
		}
	}

	GSVector4i ui = GSVector4i(u); 
	GSVector4i vi = GSVector4i(v);

	GSVector4i uv = ui.sra32(16).ps32(vi.sra32(16));

	GSVector4i c00, c01, c10, c11;

	if(ltf)
	{
		GSVector4i uf = ui.xxzzl().xxzzh().srl16(1);
		GSVector4i vf = vi.xxzzl().xxzzh().srl16(1);

		GSVector4i uv0 = Wrap(uv);
		GSVector4i uv1 = Wrap(uv.add16(GSVector4i::x0001()));

		GSVector4i y0 = uv0.uph16() << tw;
		GSVector4i y1 = uv1.uph16() << tw;
		GSVector4i x0 = uv0.upl16();
		GSVector4i x1 = uv1.upl16();

		GSVector4i addr00 = y0 + x0;
		GSVector4i addr01 = y0 + x1;
		GSVector4i addr10 = y1 + x0;
		GSVector4i addr11 = y1 + x1;

		#if _M_SSE >= 0x401

		if(tlu)
		{
			c00 = addr00.gather32_32((const BYTE*)tex, clut);
			c01 = addr01.gather32_32((const BYTE*)tex, clut);
			c10 = addr10.gather32_32((const BYTE*)tex, clut);
			c11 = addr11.gather32_32((const BYTE*)tex, clut);
		}
		else
		{
			c00 = addr00.gather32_32((const DWORD*)tex);
			c01 = addr01.gather32_32((const DWORD*)tex);
			c10 = addr10.gather32_32((const DWORD*)tex);
			c11 = addr11.gather32_32((const DWORD*)tex);
		}

		#else

		int i = 0;

		if(tlu)
		{
			do
			{
				if(ztst > 1 && test.u32[i])
				{
					continue;
				}

				c00.u32[i] = clut[((const BYTE*)tex)[addr00.u32[i]]];
				c01.u32[i] = clut[((const BYTE*)tex)[addr01.u32[i]]];
				c10.u32[i] = clut[((const BYTE*)tex)[addr10.u32[i]]];
				c11.u32[i] = clut[((const BYTE*)tex)[addr11.u32[i]]];
			}
			while(++i < pixels);
		}
		else
		{
			do
			{
				if(ztst > 1 && test.u32[i])
				{
					continue;
				}

				c00.u32[i] = ((const DWORD*)tex)[addr00.u32[i]];
				c01.u32[i] = ((const DWORD*)tex)[addr01.u32[i]];
				c10.u32[i] = ((const DWORD*)tex)[addr10.u32[i]];
				c11.u32[i] = ((const DWORD*)tex)[addr11.u32[i]];
			}
			while(++i < pixels);
		}

		#endif

		GSVector4i mask = GSVector4i::x00ff();

		GSVector4i rb00 = c00 & mask;
		GSVector4i rb01 = c01 & mask;
		GSVector4i rb10 = c10 & mask;
		GSVector4i rb11 = c11 & mask;

		GSVector4i ga00 = (c00 >> 8) & mask;
		GSVector4i ga01 = (c01 >> 8) & mask;
		GSVector4i ga10 = (c10 >> 8) & mask;
		GSVector4i ga11 = (c11 >> 8) & mask;

		rb00 = rb00.lerp16<0>(rb01, uf);
		rb10 = rb10.lerp16<0>(rb11, uf);
		rb00 = rb00.lerp16<0>(rb10, vf);

		ga00 = ga00.lerp16<0>(ga01, uf);
		ga10 = ga10.lerp16<0>(ga11, uf);
		ga00 = ga00.lerp16<0>(ga10, vf);

		c[0] = rb00;
		c[1] = ga00;
	}
	else
	{
		GSVector4i uv0 = Wrap(uv);

		GSVector4i addr00 = (uv0.uph16() << tw) + uv0.upl16();

		#if _M_SSE >= 0x401

		if(tlu)
		{
			c00 = addr00.gather32_32((const BYTE*)tex, clut);
		}
		else
		{
			c00 = addr00.gather32_32((const DWORD*)tex);
		}

		#else

		int i = 0;

		if(tlu)
		{
			do
			{
				if(ztst > 1 && test.u32[i])
				{
					continue;
				}

				c00.u32[i] = clut[((const BYTE*)tex)[addr00.u32[i]]];
			}
			while(++i < pixels);
		}
		else
		{
			do
			{
				if(ztst > 1 && test.u32[i])
				{
					continue;
				}

				c00.u32[i] = ((const DWORD*)tex)[addr00.u32[i]];
			}
			while(++i < pixels);
		}

		#endif

		GSVector4i mask = GSVector4i::x00ff();

		c[0] = c00 & mask;
		c[1] = (c00 >> 8) & mask;
	}
}

void GSDrawScanline::ColorTFX(DWORD tfx, const GSVector4i& rbf, const GSVector4i& gaf, GSVector4i& rbt, GSVector4i& gat)
{
	GSVector4i af;

	switch(tfx)
	{
	case TFX_MODULATE:
		rbt = rbt.modulate16<1>(rbf).clamp8();
		break;
	case TFX_DECAL:
		break;
	case TFX_HIGHLIGHT:
	case TFX_HIGHLIGHT2:
		af = gaf.yywwl().yywwh().srl16(7);
		rbt = rbt.modulate16<1>(rbf).add16(af).clamp8();
		gat = gat.modulate16<1>(gaf).add16(af).clamp8().mix16(gat);
		break;
	case TFX_NONE:
		rbt = rbf.srl16(7);
		break;
	default:
		__assume(0);
	}
}

void GSDrawScanline::AlphaTFX(DWORD tfx, DWORD tcc, const GSVector4i& gaf, GSVector4i& gat)
{
	switch(tfx)
	{
	case TFX_MODULATE:
		gat = gat.modulate16<1>(gaf).clamp8(); // mul16hrs rounds and breaks fogging in resident evil 4 (only modulate16<0> uses mul16hrs, but watch out)
		if(!tcc) gat = gat.mix16(gaf.srl16(7));
		break;
	case TFX_DECAL: 
		break;
	case TFX_HIGHLIGHT: 
		gat = gat.mix16(!tcc ? gaf.srl16(7) : gat.addus8(gaf.srl16(7)));
		break;
	case TFX_HIGHLIGHT2:
		if(!tcc) gat = gat.mix16(gaf.srl16(7));
		break;
	case TFX_NONE: 
		gat = gaf.srl16(7);
		break; 
	default: 
		__assume(0);
	}
}

void GSDrawScanline::Fog(DWORD fge, const GSVector4i& f, GSVector4i& rb, GSVector4i& ga)
{
	if(fge)
	{
		rb = m_env.frb.lerp16<0>(rb, f);
		ga = m_env.fga.lerp16<0>(ga, f).mix16(ga);
	}
}

bool GSDrawScanline::TestZ(DWORD zpsm, DWORD ztst, const GSVector4i& zs, const GSVector4i& za, GSVector4i& test)
{
	if(ztst > 1)
	{
		GSVector4i zd = ReadZBufX(zpsm, za);

		GSVector4i o = GSVector4i::x80000000(zs);

		GSVector4i zso = zs - o;
		GSVector4i zdo = zd - o;

		switch(ztst)
		{
		case ZTST_GEQUAL: test = zso < zdo; break;
		case ZTST_GREATER: test = zso <= zdo; break;
		default: __assume(0);
		}

		if(test.alltrue())
		{
			return false;
		}
	}
	else
	{
		test = GSVector4i::zero();
	}

	return true;
}

bool GSDrawScanline::TestAlpha(DWORD atst, DWORD afail, const GSVector4i& ga, GSVector4i& fm, GSVector4i& zm, GSVector4i& test)
{
	if(atst != ATST_ALWAYS)
	{
		GSVector4i t;

		switch(atst)
		{
		case ATST_NEVER: t = GSVector4i::invzero(); break;
		case ATST_ALWAYS: t = GSVector4i::zero(); break;
		case ATST_LESS: 
		case ATST_LEQUAL: t = (ga >> 16) > m_env.aref; break;
		case ATST_EQUAL: t = (ga >> 16) != m_env.aref; break;
		case ATST_GEQUAL: 
		case ATST_GREATER: t = (ga >> 16) < m_env.aref; break;
		case ATST_NOTEQUAL: t = (ga >> 16) == m_env.aref; break;
		default: __assume(0);
		}

		switch(afail)
		{
		case AFAIL_KEEP:
			fm |= t;
			zm |= t;
			test |= t;
			if(test.alltrue()) return false;
			break;
		case AFAIL_FB_ONLY:
			zm |= t;
			break;
		case AFAIL_ZB_ONLY:
			fm |= t;
			break;
		case AFAIL_RGB_ONLY: 
			fm |= t & GSVector4i::xff000000(t);
			zm |= t;
			break;
		default: 
			__assume(0);
		}
	}

	return true;
}

bool GSDrawScanline::TestDestAlpha(DWORD fpsm, DWORD date, const GSVector4i& d, GSVector4i& test)
{
	if(fpsm != 1 && date)
	{
		test |= (d ^ m_env.datm).sra32(31);

		if(test.alltrue())
		{
			return false;
		}
	}

	return true;
}

DWORD GSDrawScanline::ReadPixel32(DWORD* RESTRICT vm, DWORD addr)
{
	return vm[addr];
}

DWORD GSDrawScanline::ReadPixel24(DWORD* RESTRICT vm, DWORD addr)
{
	return vm[addr] & 0x00ffffff;
}

DWORD GSDrawScanline::ReadPixel16(WORD* RESTRICT vm, DWORD addr)
{
	return (DWORD)vm[addr];
}

void GSDrawScanline::WritePixel32(DWORD* RESTRICT vm, DWORD addr, DWORD c) 
{
	vm[addr] = c;
}

void GSDrawScanline::WritePixel24(DWORD* RESTRICT vm, DWORD addr, DWORD c) 
{
	vm[addr] = (vm[addr] & 0xff000000) | (c & 0x00ffffff);
}

void GSDrawScanline::WritePixel16(WORD* RESTRICT vm, DWORD addr, DWORD c) 
{
	vm[addr] = (WORD)c;
}

GSVector4i GSDrawScanline::ReadFrameX(int psm, const GSVector4i& addr) const
{
	DWORD* RESTRICT vm32 = (DWORD*)m_env.vm;
	WORD* RESTRICT vm16 = (WORD*)m_env.vm;

	GSVector4i c, r, g, b, a;

	switch(psm)
	{
	case 0:
		#if _M_SSE >= 0x401
		c = addr.gather32_32(vm32);
		#else
		c = GSVector4i(
			ReadPixel32(vm32, addr.u32[0]),
			ReadPixel32(vm32, addr.u32[1]),
			ReadPixel32(vm32, addr.u32[2]),
			ReadPixel32(vm32, addr.u32[3]));
		#endif
		break;
	case 1:
		#if _M_SSE >= 0x401
		c = addr.gather32_32(vm32);
		#else
		c = GSVector4i(
			ReadPixel32(vm32, addr.u32[0]),
			ReadPixel32(vm32, addr.u32[1]),
			ReadPixel32(vm32, addr.u32[2]),
			ReadPixel32(vm32, addr.u32[3]));
		#endif
		c = (c & GSVector4i::x00ffffff(addr)) | GSVector4i::x80000000(addr);
		break;
	case 2:
		#if _M_SSE >= 0x401
		c = addr.gather32_32(vm16);
		#else
		c = GSVector4i(
			ReadPixel16(vm16, addr.u32[0]),
			ReadPixel16(vm16, addr.u32[1]),
			ReadPixel16(vm16, addr.u32[2]),
			ReadPixel16(vm16, addr.u32[3]));
		#endif
		c = ((c & 0x8000) << 16) | ((c & 0x7c00) << 9) | ((c & 0x03e0) << 6) | ((c & 0x001f) << 3); 
		break;
	case 3:
		c = GSVector4i::zero();
		break;
	}
	
	return c;
}

GSVector4i GSDrawScanline::ReadZBufX(int psm, const GSVector4i& addr) const
{
	DWORD* RESTRICT vm32 = (DWORD*)m_env.vm;
	WORD* RESTRICT vm16 = (WORD*)m_env.vm;

	GSVector4i z;

	switch(psm)
	{
	case 0: 
		#if _M_SSE >= 0x401
		z = addr.gather32_32(vm32);
		#else
		z = GSVector4i(
			ReadPixel32(vm32, addr.u32[0]),
			ReadPixel32(vm32, addr.u32[1]),
			ReadPixel32(vm32, addr.u32[2]),
			ReadPixel32(vm32, addr.u32[3]));
		#endif
		break;
	case 1: 
		#if _M_SSE >= 0x401
		z = addr.gather32_32(vm32);
		#else
		z = GSVector4i(
			ReadPixel32(vm32, addr.u32[0]),
			ReadPixel32(vm32, addr.u32[1]),
			ReadPixel32(vm32, addr.u32[2]),
			ReadPixel32(vm32, addr.u32[3]));
		#endif
		z = z & GSVector4i::x00ffffff(addr);
		break;
	case 2: 
		#if _M_SSE >= 0x401
		z = addr.gather32_32(vm16);
		#else
		z = GSVector4i(
			ReadPixel16(vm16, addr.u32[0]),
			ReadPixel16(vm16, addr.u32[1]),
			ReadPixel16(vm16, addr.u32[2]),
			ReadPixel16(vm16, addr.u32[3]));
		#endif
		break;
	case 3:
		z = GSVector4i::zero();
		break;
	}

	return z;
}

void GSDrawScanline::WriteFrameAndZBufX(
	int fpsm, const GSVector4i& fa, const GSVector4i& fm, const GSVector4i& f, 
	int zpsm, const GSVector4i& za, const GSVector4i& zm, const GSVector4i& z, 
	int pixels)
{
	// FIXME: compiler problem or not enough xmm regs in x86 mode to store the address regs (fa, za)

	DWORD* RESTRICT vm32 = (DWORD*)m_env.vm;
	WORD* RESTRICT vm16 = (WORD*)m_env.vm;

	GSVector4i c = f;

	if(fpsm == 2)
	{
		GSVector4i rb = c & 0x00f800f8;
		GSVector4i ga = c & 0x8000f800;

		c = (ga >> 16) | (rb >> 9) | (ga >> 6) | (rb >> 3);
	}

	#if _M_SSE >= 0x401

	if(fm.extract32<0>() != 0xffffffff) 
	{
		switch(fpsm)
		{
		case 0: WritePixel32(vm32, fa.u32[0], c.extract32<0>()); break;
		case 1: WritePixel24(vm32, fa.u32[0], c.extract32<0>()); break;
		case 2: WritePixel16(vm16, fa.u32[0], c.extract16<0 * 2>()); break;
		}
	}

	if(zm.extract32<0>() != 0xffffffff) 
	{
		switch(zpsm)
		{
		case 0: WritePixel32(vm32, za.u32[0], z.extract32<0>()); break;
		case 1: WritePixel24(vm32, za.u32[0], z.extract32<0>()); break;
		case 2: WritePixel16(vm16, za.u32[0], z.extract16<0 * 2>()); break;
		}
	}

	if(pixels <= 1) return;

	if(fm.extract32<1>() != 0xffffffff) 
	{
		switch(fpsm)
		{
		case 0: WritePixel32(vm32, fa.u32[1], c.extract32<1>()); break;
		case 1: WritePixel24(vm32, fa.u32[1], c.extract32<1>()); break;
		case 2: WritePixel16(vm16, fa.u32[1], c.extract16<1 * 2>()); break;
		}
	}

	if(zm.extract32<1>() != 0xffffffff) 
	{
		switch(zpsm)
		{
		case 0: WritePixel32(vm32, za.u32[1], z.extract32<1>()); break;
		case 1: WritePixel24(vm32, za.u32[1], z.extract32<1>()); break;
		case 2: WritePixel16(vm16, za.u32[1], z.extract16<1 * 2>()); break;
		}
	}

	if(pixels <= 2) return;

	if(fm.extract32<2>() != 0xffffffff) 
	{
		switch(fpsm)
		{
		case 0: WritePixel32(vm32, fa.u32[2], c.extract32<2>()); break;
		case 1: WritePixel24(vm32, fa.u32[2], c.extract32<2>()); break;
		case 2: WritePixel16(vm16, fa.u32[2], c.extract16<2 * 2>()); break;
		}
	}

	if(zm.extract32<2>() != 0xffffffff) 
	{
		switch(zpsm)
		{
		case 0: WritePixel32(vm32, za.u32[2], z.extract32<2>()); break;
		case 1: WritePixel24(vm32, za.u32[2], z.extract32<2>()); break;
		case 2: WritePixel16(vm16, za.u32[2], z.extract16<2 * 2>()); break;
		}
	}

	if(pixels <= 3) return;

	if(fm.extract32<3>() != 0xffffffff) 
	{
		switch(fpsm)
		{
		case 0: WritePixel32(vm32, fa.u32[3], c.extract32<3>()); break;
		case 1: WritePixel24(vm32, fa.u32[3], c.extract32<3>()); break;
		case 2: WritePixel16(vm16, fa.u32[3], c.extract16<3 * 2>()); break;
		}
	}

	if(zm.extract32<3>() != 0xffffffff) 
	{
		switch(zpsm)
		{
		case 0: WritePixel32(vm32, za.u32[3], z.extract32<3>()); break;
		case 1: WritePixel24(vm32, za.u32[3], z.extract32<3>()); break;
		case 2: WritePixel16(vm16, za.u32[3], z.extract16<3 * 2>()); break;
		}
	}

	#else

	int i = 0;

	do
	{
		if(fm.u32[i] != 0xffffffff)
		{
			switch(fpsm)
			{
			case 0: WritePixel32(vm32, fa.u32[i], c.u32[i]);  break;
			case 1: WritePixel24(vm32, fa.u32[i], c.u32[i]);  break;
			case 2: WritePixel16(vm16, fa.u32[i], c.u16[i * 2]);  break;
			}
		}

		if(zm.u32[i] != 0xffffffff) 
		{
			switch(zpsm)
			{
			case 0: WritePixel32(vm32, za.u32[i], z.u32[i]);  break;
			case 1: WritePixel24(vm32, za.u32[i], z.u32[i]);  break;
			case 2: WritePixel16(vm16, za.u32[i], z.u16[i * 2]);  break;
			}
		}
	}
	while(++i < pixels);

	#endif
}

//

void GSDrawScanline::Init()
{
	// w00t :P

	#define InitDS_IIP(iFPSM, iZPSM, iZTST, iIIP) \
		m_ds[iFPSM][iZPSM][iZTST][iIIP] = (DrawScanlinePtr)&GSDrawScanline::DrawScanlineT<iFPSM, iZPSM, iZTST, iIIP>; \

	#define InitDS_ZTST(iFPSM, iZPSM, iZTST) \
		InitDS_IIP(iFPSM, iZPSM, iZTST, 0) \
		InitDS_IIP(iFPSM, iZPSM, iZTST, 1) \

	#define InitDS(iFPSM, iZPSM) \
		InitDS_ZTST(iFPSM, iZPSM, 0) \
		InitDS_ZTST(iFPSM, iZPSM, 1) \
		InitDS_ZTST(iFPSM, iZPSM, 2) \
		InitDS_ZTST(iFPSM, iZPSM, 3) \

	InitDS(0, 0);
	InitDS(0, 1);
	InitDS(0, 2);
	InitDS(0, 3);
	InitDS(1, 0);
	InitDS(1, 1);
	InitDS(1, 2);
	InitDS(1, 3);
	InitDS(2, 0);
	InitDS(2, 1);
	InitDS(2, 2);
	InitDS(2, 3);
	InitDS(3, 0);
	InitDS(3, 1);
	InitDS(3, 2);

	#ifdef FAST_DRAWSCANLINE

	// bios

	m_dsmap.SetAt(0x1fe00860, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe00860>);
	m_dsmap.SetAt(0x1fe04070, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04070>);
	m_dsmap.SetAt(0x1fe04850, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04850>);
	m_dsmap.SetAt(0x1fe04860, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04860>);
	m_dsmap.SetAt(0x1fe20060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe20060>);
	m_dsmap.SetAt(0x1fe20860, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe20860>);
	m_dsmap.SetAt(0x1fe28864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe28864>);
	m_dsmap.SetAt(0x1fe28870, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe28870>);
	m_dsmap.SetAt(0x1fe38050, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe38050>);
	m_dsmap.SetAt(0x1fe38060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe38060>);
	m_dsmap.SetAt(0x1fe38064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe38064>);
	m_dsmap.SetAt(0x1fe38070, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe38070>);
	m_dsmap.SetAt(0x1fe39050, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe39050>);
	m_dsmap.SetAt(0x48428050, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48428050>);
	m_dsmap.SetAt(0x48428060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48428060>);
	m_dsmap.SetAt(0x48804860, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804860>);
	m_dsmap.SetAt(0x48838050, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48838050>);
	m_dsmap.SetAt(0x48838064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48838064>);
	m_dsmap.SetAt(0x48839050, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48839050>);
	m_dsmap.SetAt(0x49004050, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004050>);
	m_dsmap.SetAt(0x4902884c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4902884c>);
	m_dsmap.SetAt(0x4902904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4902904c>);
	m_dsmap.SetAt(0x49038050, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49038050>);
	m_dsmap.SetAt(0x49039050, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49039050>);
	m_dsmap.SetAt(0x4b02804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b02804c>);
	m_dsmap.SetAt(0x4c40404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c40404c>);
	m_dsmap.SetAt(0x4c419050, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c419050>);
	m_dsmap.SetAt(0x4c804050, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c804050>);
	m_dsmap.SetAt(0x4c804850, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c804850>);
	m_dsmap.SetAt(0x4d019050, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d019050>);
	m_dsmap.SetAt(0x4d028864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d028864>);
	m_dsmap.SetAt(0x4d038864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d038864>);

	// ffx

	m_dsmap.SetAt(0x1fe04055, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04055>);
	m_dsmap.SetAt(0x1fe11056, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe11056>);
	m_dsmap.SetAt(0x1fe68175, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe68175>);
	m_dsmap.SetAt(0x1fe68975, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe68975>);
	m_dsmap.SetAt(0x1fe69175, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe69175>);
	m_dsmap.SetAt(0x1fe84175, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe84175>);
	m_dsmap.SetAt(0x1fe84975, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe84975>);
	m_dsmap.SetAt(0x1fee8175, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee8175>);
	m_dsmap.SetAt(0x1fee8975, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee8975>);
	m_dsmap.SetAt(0x1fee9175, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee9175>);
	m_dsmap.SetAt(0x48404965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48404965>);
	m_dsmap.SetAt(0x4847814d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4847814d>);
	m_dsmap.SetAt(0x48478165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48478165>);
	m_dsmap.SetAt(0x48479165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48479165>);
	m_dsmap.SetAt(0x4880404d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4880404d>);
	m_dsmap.SetAt(0x48804055, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804055>);
	m_dsmap.SetAt(0x48804155, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804155>);
	m_dsmap.SetAt(0x488041cd, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488041cd>);
	m_dsmap.SetAt(0x4880484d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4880484d>);
	m_dsmap.SetAt(0x48804855, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804855>);
	m_dsmap.SetAt(0x48804965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804965>);
	m_dsmap.SetAt(0x488049cd, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488049cd>);
	m_dsmap.SetAt(0x48810055, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48810055>);
	m_dsmap.SetAt(0x48820965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48820965>);
	m_dsmap.SetAt(0x48830075, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48830075>);
	m_dsmap.SetAt(0x4883014d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883014d>);
	m_dsmap.SetAt(0x48830875, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48830875>);
	m_dsmap.SetAt(0x48830965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48830965>);
	m_dsmap.SetAt(0x488589cd, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488589cd>);
	m_dsmap.SetAt(0x48868165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868165>);
	m_dsmap.SetAt(0x48868965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868965>);
	m_dsmap.SetAt(0x4887814d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887814d>);
	m_dsmap.SetAt(0x48878165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878165>);
	m_dsmap.SetAt(0x488781cd, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488781cd>);
	m_dsmap.SetAt(0x488781e5, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488781e5>);
	m_dsmap.SetAt(0x488789cd, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488789cd>);
	m_dsmap.SetAt(0x4887914d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887914d>);
	m_dsmap.SetAt(0x488791cd, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488791cd>);
	m_dsmap.SetAt(0x488791d5, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488791d5>);
	m_dsmap.SetAt(0x488791e5, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488791e5>);
	m_dsmap.SetAt(0x48884965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48884965>);
	m_dsmap.SetAt(0x488e8165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8165>);
	m_dsmap.SetAt(0x488e8965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8965>);
	m_dsmap.SetAt(0x488e9165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e9165>);
	m_dsmap.SetAt(0x488f81f5, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488f81f5>);
	m_dsmap.SetAt(0x488f89f5, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488f89f5>);
	m_dsmap.SetAt(0x48c04865, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48c04865>);
	m_dsmap.SetAt(0x49004165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004165>);
	m_dsmap.SetAt(0x490041cd, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490041cd>);
	m_dsmap.SetAt(0x49004865, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004865>);
	m_dsmap.SetAt(0x49004965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004965>);
	m_dsmap.SetAt(0x490049cd, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490049cd>);
	m_dsmap.SetAt(0x49068165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068165>);
	m_dsmap.SetAt(0x49068965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068965>);
	m_dsmap.SetAt(0x49069165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49069165>);
	m_dsmap.SetAt(0x4907814d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4907814d>);
	m_dsmap.SetAt(0x49078165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078165>);
	m_dsmap.SetAt(0x49078965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078965>);
	m_dsmap.SetAt(0x49079165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49079165>);
	m_dsmap.SetAt(0x490f89f5, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490f89f5>);
	m_dsmap.SetAt(0x4c811055, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c811055>);
	m_dsmap.SetAt(0x4c831065, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c831065>);
	m_dsmap.SetAt(0x4d0481e5, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d0481e5>);
	m_dsmap.SetAt(0x51004155, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x51004155>);
	m_dsmap.SetAt(0x51004165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x51004165>);
	m_dsmap.SetAt(0x51004965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x51004965>);
	m_dsmap.SetAt(0x5101004e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5101004e>);
	m_dsmap.SetAt(0x51020965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x51020965>);
	m_dsmap.SetAt(0x54204055, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x54204055>);
	m_dsmap.SetAt(0x55204055, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x55204055>);
	m_dsmap.SetAt(0x1fe78075, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe78075>);
	m_dsmap.SetAt(0x1fe78155, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe78155>);
	m_dsmap.SetAt(0x484049cd, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x484049cd>);
	m_dsmap.SetAt(0x48804875, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804875>);
	m_dsmap.SetAt(0x49004075, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004075>);
	m_dsmap.SetAt(0x49004875, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004875>);
	m_dsmap.SetAt(0x4907914d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4907914d>);
	m_dsmap.SetAt(0x4c804075, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c804075>);
	m_dsmap.SetAt(0x4c804875, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c804875>);
	m_dsmap.SetAt(0x4d078075, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d078075>);
	m_dsmap.SetAt(0x5100414d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5100414d>);

	// ffxii

	m_dsmap.SetAt(0x1fe0404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe0404c>);
	m_dsmap.SetAt(0x1fe04054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04054>);
	m_dsmap.SetAt(0x1fe04057, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04057>);
	m_dsmap.SetAt(0x1fe3804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3804c>);
	m_dsmap.SetAt(0x1fe3904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3904c>);
	m_dsmap.SetAt(0x1fe6804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe6804c>);
	m_dsmap.SetAt(0x1fe68164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe68164>);
	m_dsmap.SetAt(0x1fe6884c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe6884c>);
	m_dsmap.SetAt(0x1fe68964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe68964>);
	m_dsmap.SetAt(0x1fe84164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe84164>);
	m_dsmap.SetAt(0x1fe84964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe84964>);
	m_dsmap.SetAt(0x1fec8064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fec8064>);
	m_dsmap.SetAt(0x1fec8864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fec8864>);
	m_dsmap.SetAt(0x1fee8164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee8164>);
	m_dsmap.SetAt(0x1fee8864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee8864>);
	m_dsmap.SetAt(0x1fee8964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee8964>);
	m_dsmap.SetAt(0x48404064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48404064>);
	m_dsmap.SetAt(0x4847004c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4847004c>);
	m_dsmap.SetAt(0x4880404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4880404c>);
	m_dsmap.SetAt(0x48804064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804064>);
	m_dsmap.SetAt(0x4880484c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4880484c>);
	m_dsmap.SetAt(0x48828064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48828064>);
	m_dsmap.SetAt(0x48828864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48828864>);
	m_dsmap.SetAt(0x4883004c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883004c>);
	m_dsmap.SetAt(0x4883084c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883084c>);
	m_dsmap.SetAt(0x4883804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883804c>);
	m_dsmap.SetAt(0x4883884c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883884c>);
	m_dsmap.SetAt(0x4883904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883904c>);
	m_dsmap.SetAt(0x4885804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4885804c>);
	m_dsmap.SetAt(0x4885904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4885904c>);
	m_dsmap.SetAt(0x4886804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4886804c>);
	m_dsmap.SetAt(0x48868064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868064>);
	m_dsmap.SetAt(0x48868364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868364>);
	m_dsmap.SetAt(0x4886884c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4886884c>);
	m_dsmap.SetAt(0x48868864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868864>);
	m_dsmap.SetAt(0x48868b64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868b64>);
	m_dsmap.SetAt(0x48869064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48869064>);
	m_dsmap.SetAt(0x4887004c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887004c>);
	m_dsmap.SetAt(0x4887084c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887084c>);
	m_dsmap.SetAt(0x4887804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887804c>);
	m_dsmap.SetAt(0x48878064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878064>);
	m_dsmap.SetAt(0x48878364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878364>);
	m_dsmap.SetAt(0x4887884c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887884c>);
	m_dsmap.SetAt(0x4887904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887904c>);
	m_dsmap.SetAt(0x48879064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879064>);
	m_dsmap.SetAt(0x48884064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48884064>);
	m_dsmap.SetAt(0x488c8364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488c8364>);
	m_dsmap.SetAt(0x488c8b64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488c8b64>);
	m_dsmap.SetAt(0x488e8064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8064>);
	m_dsmap.SetAt(0x488e8364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8364>);
	m_dsmap.SetAt(0x488e8864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8864>);
	m_dsmap.SetAt(0x488e8b64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8b64>);
	m_dsmap.SetAt(0x48904064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48904064>);
	m_dsmap.SetAt(0x48c0404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48c0404c>);
	m_dsmap.SetAt(0x48c6804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48c6804c>);
	m_dsmap.SetAt(0x48c6904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48c6904c>);
	m_dsmap.SetAt(0x48c7804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48c7804c>);
	m_dsmap.SetAt(0x48c7884c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48c7884c>);
	m_dsmap.SetAt(0x49004064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004064>);
	m_dsmap.SetAt(0x4900484c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4900484c>);
	m_dsmap.SetAt(0x49004864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004864>);
	m_dsmap.SetAt(0x4903804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4903804c>);
	m_dsmap.SetAt(0x4905804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4905804c>);
	m_dsmap.SetAt(0x49068064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068064>);
	m_dsmap.SetAt(0x49068364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068364>);
	m_dsmap.SetAt(0x4906884c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4906884c>);
	m_dsmap.SetAt(0x49068864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068864>);
	m_dsmap.SetAt(0x49068b64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068b64>);
	m_dsmap.SetAt(0x4907804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4907804c>);
	m_dsmap.SetAt(0x49078064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078064>);
	m_dsmap.SetAt(0x49078364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078364>);
	m_dsmap.SetAt(0x4907884c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4907884c>);
	m_dsmap.SetAt(0x490e8b64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8b64>);
	m_dsmap.SetAt(0x49268064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49268064>);
	m_dsmap.SetAt(0x49278064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49278064>);
	m_dsmap.SetAt(0x4b00404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b00404c>);
	m_dsmap.SetAt(0x5fe0404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe0404c>);
	m_dsmap.SetAt(0x5fe04064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe04064>);
	m_dsmap.SetAt(0x5ff0404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5ff0404c>);

	// ffx-2

	m_dsmap.SetAt(0x1fe0404d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe0404d>);
	m_dsmap.SetAt(0x1fe0414d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe0414d>);
	m_dsmap.SetAt(0x1fe1004e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe1004e>);
	m_dsmap.SetAt(0x1fe30069, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe30069>);
	m_dsmap.SetAt(0x1fee8165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee8165>);
	m_dsmap.SetAt(0x1fee8965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee8965>);
	m_dsmap.SetAt(0x4840414d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4840414d>);
	m_dsmap.SetAt(0x48404155, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48404155>);
	m_dsmap.SetAt(0x48468165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48468165>);
	m_dsmap.SetAt(0x48468965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48468965>);
	m_dsmap.SetAt(0x48478965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48478965>);
	m_dsmap.SetAt(0x488041e5, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488041e5>);
	m_dsmap.SetAt(0x4880494d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4880494d>);
	m_dsmap.SetAt(0x488049f5, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488049f5>);
	m_dsmap.SetAt(0x4881004d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4881004d>);
	m_dsmap.SetAt(0x4881894d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4881894d>);
	m_dsmap.SetAt(0x48820165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48820165>);
	m_dsmap.SetAt(0x48820875, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48820875>);
	m_dsmap.SetAt(0x48830155, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48830155>);
	m_dsmap.SetAt(0x4883904d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883904d>);
	m_dsmap.SetAt(0x4885004d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4885004d>);
	m_dsmap.SetAt(0x4885814d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4885814d>);
	m_dsmap.SetAt(0x488581cd, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488581cd>);
	m_dsmap.SetAt(0x488581e5, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488581e5>);
	m_dsmap.SetAt(0x488591cd, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488591cd>);
	m_dsmap.SetAt(0x488591d5, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488591d5>);
	m_dsmap.SetAt(0x48869165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48869165>);
	m_dsmap.SetAt(0x48878074, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878074>);
	m_dsmap.SetAt(0x488781f5, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488781f5>);
	m_dsmap.SetAt(0x48878874, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878874>);
	m_dsmap.SetAt(0x48878965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878965>);
	m_dsmap.SetAt(0x488789f5, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488789f5>);
	m_dsmap.SetAt(0x48879165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879165>);
	m_dsmap.SetAt(0x49004059, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004059>);
	m_dsmap.SetAt(0x49004155, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004155>);
	m_dsmap.SetAt(0x49004859, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004859>);
	m_dsmap.SetAt(0x49004869, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004869>);
	m_dsmap.SetAt(0x4900494d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4900494d>);
	m_dsmap.SetAt(0x4901004d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4901004d>);
	m_dsmap.SetAt(0x490581cd, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490581cd>);
	m_dsmap.SetAt(0x49059155, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49059155>);
	m_dsmap.SetAt(0x490591cd, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490591cd>);
	m_dsmap.SetAt(0x4906814d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4906814d>);
	m_dsmap.SetAt(0x490781cd, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490781cd>);
	m_dsmap.SetAt(0x490791cd, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490791cd>);
	m_dsmap.SetAt(0x490e8165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8165>);
	m_dsmap.SetAt(0x490e8965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8965>);
	m_dsmap.SetAt(0x4c80404d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c80404d>);
	m_dsmap.SetAt(0x4c804055, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c804055>);
	m_dsmap.SetAt(0x4c81004d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c81004d>);
	m_dsmap.SetAt(0x4c850055, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c850055>);
	m_dsmap.SetAt(0x4d020965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d020965>);
	m_dsmap.SetAt(0x4d068965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d068965>);
	m_dsmap.SetAt(0x51068165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x51068165>);
	m_dsmap.SetAt(0x51068965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x51068965>);
	m_dsmap.SetAt(0x510a8165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x510a8165>);
	m_dsmap.SetAt(0x510a8965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x510a8965>);
	m_dsmap.SetAt(0x5520404d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5520404d>);
	m_dsmap.SetAt(0x5fe0414d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe0414d>);
	m_dsmap.SetAt(0x5fe0494d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe0494d>);
	m_dsmap.SetAt(0x5fe1004d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe1004d>);
	m_dsmap.SetAt(0x5fe5884d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe5884d>);
	m_dsmap.SetAt(0x5fe5894d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe5894d>);
	m_dsmap.SetAt(0x5fe5914d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe5914d>);

	// kingdom hearts

	m_dsmap.SetAt(0x1fe0484c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe0484c>);
	m_dsmap.SetAt(0x1fe1804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe1804c>);
	m_dsmap.SetAt(0x1fe3004d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3004d>);
	m_dsmap.SetAt(0x1fe30054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe30054>);
	m_dsmap.SetAt(0x1fe3804d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3804d>);
	m_dsmap.SetAt(0x1fe3904d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3904d>);
	m_dsmap.SetAt(0x1fe39054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe39054>);
	m_dsmap.SetAt(0x1fee9164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee9164>);
	m_dsmap.SetAt(0x4840414c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4840414c>);
	m_dsmap.SetAt(0x4846814c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4846814c>);
	m_dsmap.SetAt(0x48468174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48468174>);
	m_dsmap.SetAt(0x48468974, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48468974>);
	m_dsmap.SetAt(0x4847814c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4847814c>);
	m_dsmap.SetAt(0x48478174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48478174>);
	m_dsmap.SetAt(0x48478974, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48478974>);
	m_dsmap.SetAt(0x48804054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804054>);
	m_dsmap.SetAt(0x4880414c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4880414c>);
	m_dsmap.SetAt(0x4880494c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4880494c>);
	m_dsmap.SetAt(0x48804974, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804974>);
	m_dsmap.SetAt(0x4881004c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4881004c>);
	m_dsmap.SetAt(0x48818054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48818054>);
	m_dsmap.SetAt(0x48819054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48819054>);
	m_dsmap.SetAt(0x48829164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48829164>);
	m_dsmap.SetAt(0x48830054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48830054>);
	m_dsmap.SetAt(0x48830164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48830164>);
	m_dsmap.SetAt(0x48858054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48858054>);
	m_dsmap.SetAt(0x48858854, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48858854>);
	m_dsmap.SetAt(0x48859054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48859054>);
	m_dsmap.SetAt(0x4886814c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4886814c>);
	m_dsmap.SetAt(0x48868154, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868154>);
	m_dsmap.SetAt(0x48868174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868174>);
	m_dsmap.SetAt(0x48868974, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868974>);
	m_dsmap.SetAt(0x48878054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878054>);
	m_dsmap.SetAt(0x4887814c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887814c>);
	m_dsmap.SetAt(0x48878154, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878154>);
	m_dsmap.SetAt(0x48878174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878174>);
	m_dsmap.SetAt(0x48878854, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878854>);
	m_dsmap.SetAt(0x4887894c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887894c>);
	m_dsmap.SetAt(0x48879054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879054>);
	m_dsmap.SetAt(0x4887914c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887914c>);
	m_dsmap.SetAt(0x488a8168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488a8168>);
	m_dsmap.SetAt(0x488e80e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e80e4>);
	m_dsmap.SetAt(0x488e8164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8164>);
	m_dsmap.SetAt(0x488e8168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8168>);
	m_dsmap.SetAt(0x488e88e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e88e4>);
	m_dsmap.SetAt(0x488e8964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8964>);
	m_dsmap.SetAt(0x488e90e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e90e4>);
	m_dsmap.SetAt(0x488e9164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e9164>);
	m_dsmap.SetAt(0x488e9168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e9168>);
	m_dsmap.SetAt(0x48c68174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48c68174>);
	m_dsmap.SetAt(0x48c68974, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48c68974>);
	m_dsmap.SetAt(0x49004054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004054>);
	m_dsmap.SetAt(0x4900414c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4900414c>);
	m_dsmap.SetAt(0x49058054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49058054>);
	m_dsmap.SetAt(0x49059054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49059054>);
	m_dsmap.SetAt(0x49068174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068174>);
	m_dsmap.SetAt(0x4906894c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4906894c>);
	m_dsmap.SetAt(0x49068974, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068974>);
	m_dsmap.SetAt(0x49078054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078054>);
	m_dsmap.SetAt(0x4907814c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4907814c>);
	m_dsmap.SetAt(0x49078174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078174>);
	m_dsmap.SetAt(0x4907894c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4907894c>);
	m_dsmap.SetAt(0x49078974, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078974>);
	m_dsmap.SetAt(0x49079174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49079174>);
	m_dsmap.SetAt(0x490e814c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e814c>);
	m_dsmap.SetAt(0x490e894c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e894c>);
	m_dsmap.SetAt(0x490e8974, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8974>);
	m_dsmap.SetAt(0x5020404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5020404c>);
	m_dsmap.SetAt(0x5fe04058, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe04058>);
	m_dsmap.SetAt(0x5fe04168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe04168>);
	m_dsmap.SetAt(0x5fe3804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe3804c>);
	m_dsmap.SetAt(0x5fe3904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe3904c>);
	m_dsmap.SetAt(0x5fea8168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fea8168>);

	// kingdom hearts 2

	m_dsmap.SetAt(0x1fe04050, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04050>);
	m_dsmap.SetAt(0x1fe31054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe31054>);
	m_dsmap.SetAt(0x1fe3914d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3914d>);
	m_dsmap.SetAt(0x40478174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x40478174>);
	m_dsmap.SetAt(0x40c78174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x40c78174>);
	m_dsmap.SetAt(0x48478054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48478054>);
	m_dsmap.SetAt(0x48478854, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48478854>);
	m_dsmap.SetAt(0x48804160, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804160>);
	m_dsmap.SetAt(0x48810054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48810054>);
	m_dsmap.SetAt(0x4883814c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883814c>);
	m_dsmap.SetAt(0x4883914c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883914c>);
	m_dsmap.SetAt(0x4885884d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4885884d>);
	m_dsmap.SetAt(0x4887804d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887804d>);
	m_dsmap.SetAt(0x4887884d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887884d>);
	m_dsmap.SetAt(0x48879154, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879154>);
	m_dsmap.SetAt(0x488a8964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488a8964>);
	m_dsmap.SetAt(0x49004974, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004974>);
	m_dsmap.SetAt(0x4907804d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4907804d>);
	m_dsmap.SetAt(0x4907884d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4907884d>);
	m_dsmap.SetAt(0x49078854, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078854>);
	m_dsmap.SetAt(0x4907904d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4907904d>);
	m_dsmap.SetAt(0x49079054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49079054>);
	m_dsmap.SetAt(0x490e8164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8164>);
	m_dsmap.SetAt(0x490e8964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8964>);
	m_dsmap.SetAt(0x4ac78174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4ac78174>);
	m_dsmap.SetAt(0x4c83004d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c83004d>);
	m_dsmap.SetAt(0x4d03004d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d03004d>);
	m_dsmap.SetAt(0x50c68174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x50c68174>);
	m_dsmap.SetAt(0x54478174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x54478174>);
	m_dsmap.SetAt(0x5fe68174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe68174>);
	m_dsmap.SetAt(0x5fe78174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe78174>);
	m_dsmap.SetAt(0x48830074, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48830074>);
	m_dsmap.SetAt(0x48830874, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48830874>);
	m_dsmap.SetAt(0x4887904d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887904d>);
	m_dsmap.SetAt(0x48879074, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879074>);
	m_dsmap.SetAt(0x4888404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4888404c>);
	m_dsmap.SetAt(0x488a8164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488a8164>);
	m_dsmap.SetAt(0x49038174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49038174>);
	m_dsmap.SetAt(0x49084174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49084174>);
	m_dsmap.SetAt(0x4c084174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c084174>);
	m_dsmap.SetAt(0x50884174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x50884174>);
	m_dsmap.SetAt(0x5fe04174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe04174>);
	m_dsmap.SetAt(0x5fe2b174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe2b174>);
	m_dsmap.SetAt(0x5fe3b174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe3b174>);

	// persona 3

	m_dsmap.SetAt(0x1fe04058, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04058>);
	m_dsmap.SetAt(0x1fe0405b, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe0405b>);
	m_dsmap.SetAt(0x1fe04068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04068>);
	m_dsmap.SetAt(0x1fe04368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04368>);
	m_dsmap.SetAt(0x1fe82368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe82368>);
	m_dsmap.SetAt(0x1fe84068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe84068>);
	m_dsmap.SetAt(0x1fe84368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe84368>);
	m_dsmap.SetAt(0x1fe84868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe84868>);
	m_dsmap.SetAt(0x4840404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4840404c>);
	m_dsmap.SetAt(0x4840484c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4840484c>);
	m_dsmap.SetAt(0x484e8068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x484e8068>);
	m_dsmap.SetAt(0x48804068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804068>);
	m_dsmap.SetAt(0x48804368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804368>);
	m_dsmap.SetAt(0x48804868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804868>);
	m_dsmap.SetAt(0x4881834c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4881834c>);
	m_dsmap.SetAt(0x4881884c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4881884c>);
	m_dsmap.SetAt(0x4881934c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4881934c>);
	m_dsmap.SetAt(0x48820368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48820368>);
	m_dsmap.SetAt(0x48848b68, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48848b68>);
	m_dsmap.SetAt(0x48858368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48858368>);
	m_dsmap.SetAt(0x48858b68, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48858b68>);
	m_dsmap.SetAt(0x48859368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48859368>);
	m_dsmap.SetAt(0x48868068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868068>);
	m_dsmap.SetAt(0x48868368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868368>);
	m_dsmap.SetAt(0x48868b68, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868b68>);
	m_dsmap.SetAt(0x48878068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878068>);
	m_dsmap.SetAt(0x4887834c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887834c>);
	m_dsmap.SetAt(0x48878368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878368>);
	m_dsmap.SetAt(0x48878b4c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878b4c>);
	m_dsmap.SetAt(0x48878b68, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878b68>);
	m_dsmap.SetAt(0x4887934c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887934c>);
	m_dsmap.SetAt(0x48879368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879368>);
	m_dsmap.SetAt(0x48884068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48884068>);
	m_dsmap.SetAt(0x48884368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48884368>);
	m_dsmap.SetAt(0x48884868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48884868>);
	m_dsmap.SetAt(0x48892368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48892368>);
	m_dsmap.SetAt(0x488b2368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488b2368>);
	m_dsmap.SetAt(0x488e8068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8068>);
	m_dsmap.SetAt(0x488e8368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8368>);
	m_dsmap.SetAt(0x488e8868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8868>);
	m_dsmap.SetAt(0x488e8b68, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8b68>);
	m_dsmap.SetAt(0x488e8f68, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8f68>);
	m_dsmap.SetAt(0x488e9368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e9368>);
	m_dsmap.SetAt(0x488f8868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488f8868>);
	m_dsmap.SetAt(0x488f8b68, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488f8b68>);
	m_dsmap.SetAt(0x4900404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4900404c>);
	m_dsmap.SetAt(0x49068068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068068>);
	m_dsmap.SetAt(0x49068868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068868>);
	m_dsmap.SetAt(0x49078068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078068>);
	m_dsmap.SetAt(0x49078868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078868>);
	m_dsmap.SetAt(0x4907904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4907904c>);
	m_dsmap.SetAt(0x49079068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49079068>);
	m_dsmap.SetAt(0x490e8068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8068>);
	m_dsmap.SetAt(0x490e8868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8868>);
	m_dsmap.SetAt(0x490f8068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490f8068>);
	m_dsmap.SetAt(0x4a43004c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a43004c>);
	m_dsmap.SetAt(0x4a83004c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a83004c>);
	m_dsmap.SetAt(0x4a87934c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a87934c>);
	m_dsmap.SetAt(0x4b03004c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b03004c>);
	m_dsmap.SetAt(0x4b07934c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b07934c>);
	m_dsmap.SetAt(0x4c8689e8, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c8689e8>);
	m_dsmap.SetAt(0x4d0e8068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d0e8068>);
	m_dsmap.SetAt(0x4d0e8868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d0e8868>);
	m_dsmap.SetAt(0x4d47834c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d47834c>);
	m_dsmap.SetAt(0x4d478b4c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d478b4c>);
	m_dsmap.SetAt(0x4d47934c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d47934c>);

	// persona 4

	m_dsmap.SetAt(0x1fe04858, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04858>);
	m_dsmap.SetAt(0x1fe04b4c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04b4c>);
	m_dsmap.SetAt(0x48804058, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804058>);
	m_dsmap.SetAt(0x48804b68, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804b68>);
	m_dsmap.SetAt(0x48828368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48828368>);
	m_dsmap.SetAt(0x48828b68, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48828b68>);
	m_dsmap.SetAt(0x48830168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48830168>);
	m_dsmap.SetAt(0x48868868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868868>);
	m_dsmap.SetAt(0x48868f68, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868f68>);
	m_dsmap.SetAt(0x48869368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48869368>);
	m_dsmap.SetAt(0x48879168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879168>);
	m_dsmap.SetAt(0x48884b68, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48884b68>);
	m_dsmap.SetAt(0x488f8368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488f8368>);
	m_dsmap.SetAt(0x49004068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004068>);
	m_dsmap.SetAt(0x49004868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004868>);
	m_dsmap.SetAt(0x4903004c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4903004c>);
	m_dsmap.SetAt(0x49084068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49084068>);
	m_dsmap.SetAt(0x49084868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49084868>);
	m_dsmap.SetAt(0x4a47804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a47804c>);
	m_dsmap.SetAt(0x4a47904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a47904c>);
	m_dsmap.SetAt(0x4a80404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a80404c>);
	m_dsmap.SetAt(0x4a87834c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a87834c>);
	m_dsmap.SetAt(0x4a878368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a878368>);
	m_dsmap.SetAt(0x4a878b68, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a878b68>);
	m_dsmap.SetAt(0x4a879068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a879068>);
	m_dsmap.SetAt(0x4b004968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b004968>);
	m_dsmap.SetAt(0x4b07804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b07804c>);
	m_dsmap.SetAt(0x4b07884c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b07884c>);
	m_dsmap.SetAt(0x4d080068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d080068>);
	m_dsmap.SetAt(0x5fe04068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe04068>);
	m_dsmap.SetAt(0x5fe04858, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe04858>);
	m_dsmap.SetAt(0x5ff0484c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5ff0484c>);

	// sfex3

	m_dsmap.SetAt(0x1fe04868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04868>);
	m_dsmap.SetAt(0x1fe04878, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04878>);
	m_dsmap.SetAt(0x1fe3004e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3004e>);
	m_dsmap.SetAt(0x1fe39178, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe39178>);
	m_dsmap.SetAt(0x1fe6b168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe6b168>);
	m_dsmap.SetAt(0x1fe6b968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe6b968>);
	m_dsmap.SetAt(0x41268068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x41268068>);
	m_dsmap.SetAt(0x41268868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x41268868>);
	m_dsmap.SetAt(0x41269068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x41269068>);
	m_dsmap.SetAt(0x485041cc, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x485041cc>);
	m_dsmap.SetAt(0x485049cc, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x485049cc>);
	m_dsmap.SetAt(0x4856b1cc, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4856b1cc>);
	m_dsmap.SetAt(0x4856b9cc, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4856b9cc>);
	m_dsmap.SetAt(0x48804158, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804158>);
	m_dsmap.SetAt(0x48804178, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804178>);
	m_dsmap.SetAt(0x48804978, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804978>);
	m_dsmap.SetAt(0x4885814c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4885814c>);
	m_dsmap.SetAt(0x48858158, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48858158>);
	m_dsmap.SetAt(0x48859078, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48859078>);
	m_dsmap.SetAt(0x4885914c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4885914c>);
	m_dsmap.SetAt(0x48859158, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48859158>);
	m_dsmap.SetAt(0x48868168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868168>);
	m_dsmap.SetAt(0x48868178, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868178>);
	m_dsmap.SetAt(0x48868968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868968>);
	m_dsmap.SetAt(0x48868978, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868978>);
	m_dsmap.SetAt(0x4886b168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4886b168>);
	m_dsmap.SetAt(0x4886b178, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4886b178>);
	m_dsmap.SetAt(0x4886b968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4886b968>);
	m_dsmap.SetAt(0x4886b978, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4886b978>);
	m_dsmap.SetAt(0x48878158, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878158>);
	m_dsmap.SetAt(0x48879158, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879158>);
	m_dsmap.SetAt(0x49004178, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004178>);
	m_dsmap.SetAt(0x490041f8, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490041f8>);
	m_dsmap.SetAt(0x49004878, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004878>);
	m_dsmap.SetAt(0x49004978, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004978>);
	m_dsmap.SetAt(0x490049f8, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490049f8>);
	m_dsmap.SetAt(0x49068078, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068078>);
	m_dsmap.SetAt(0x49068178, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068178>);
	m_dsmap.SetAt(0x49068978, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068978>);
	m_dsmap.SetAt(0x49078078, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078078>);
	m_dsmap.SetAt(0x49078178, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078178>);
	m_dsmap.SetAt(0x490781f8, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490781f8>);
	m_dsmap.SetAt(0x490789f8, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490789f8>);
	m_dsmap.SetAt(0x49079178, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49079178>);
	m_dsmap.SetAt(0x4c80404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c80404c>);
	m_dsmap.SetAt(0x4c804158, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c804158>);
	m_dsmap.SetAt(0x4c80484c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c80484c>);
	m_dsmap.SetAt(0x4c85814c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c85814c>);
	m_dsmap.SetAt(0x4c85914c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c85914c>);
	m_dsmap.SetAt(0x4c859158, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c859158>);
	m_dsmap.SetAt(0x4c868068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c868068>);
	m_dsmap.SetAt(0x4c868868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c868868>);
	m_dsmap.SetAt(0x4d068168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d068168>);
	m_dsmap.SetAt(0x4d06b168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d06b168>);
	m_dsmap.SetAt(0x4d06b968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d06b968>);
	m_dsmap.SetAt(0x4d404058, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d404058>);

	// gt4

	m_dsmap.SetAt(0x1fe0404e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe0404e>);
	m_dsmap.SetAt(0x1fe04064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04064>);
	m_dsmap.SetAt(0x1fe1904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe1904c>);
	m_dsmap.SetAt(0x1fe1904d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe1904d>);
	m_dsmap.SetAt(0x1fe3904e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3904e>);
	m_dsmap.SetAt(0x1fe7804d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe7804d>);
	m_dsmap.SetAt(0x1fe7904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe7904c>);
	m_dsmap.SetAt(0x1fe7904d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe7904d>);
	m_dsmap.SetAt(0x1fec8164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fec8164>);
	m_dsmap.SetAt(0x1fec8964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fec8964>);
	m_dsmap.SetAt(0x1fec9164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fec9164>);
	m_dsmap.SetAt(0x1fecb964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fecb964>);
	m_dsmap.SetAt(0x1fee8064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee8064>);
	m_dsmap.SetAt(0x1feeb964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1feeb964>);
	m_dsmap.SetAt(0x488181d4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488181d4>);
	m_dsmap.SetAt(0x488191d4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488191d4>);
	m_dsmap.SetAt(0x4883804d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883804d>);
	m_dsmap.SetAt(0x4885804d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4885804d>);
	m_dsmap.SetAt(0x4885904d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4885904d>);
	m_dsmap.SetAt(0x4886884d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4886884d>);
	m_dsmap.SetAt(0x48878065, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878065>);
	m_dsmap.SetAt(0x48878865, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878865>);
	m_dsmap.SetAt(0x488a8064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488a8064>);
	m_dsmap.SetAt(0x488a8864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488a8864>);
	m_dsmap.SetAt(0x488c8064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488c8064>);
	m_dsmap.SetAt(0x488c8864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488c8864>);
	m_dsmap.SetAt(0x488e804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e804c>);
	m_dsmap.SetAt(0x488e8065, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8065>);
	m_dsmap.SetAt(0x488e8764, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8764>);
	m_dsmap.SetAt(0x488e8865, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8865>);
	m_dsmap.SetAt(0x488e8f64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8f64>);
	m_dsmap.SetAt(0x488e9764, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e9764>);
	m_dsmap.SetAt(0x488eb064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488eb064>);
	m_dsmap.SetAt(0x488eb864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488eb864>);
	m_dsmap.SetAt(0x490e8064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8064>);
	m_dsmap.SetAt(0x490e8864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8864>);
	m_dsmap.SetAt(0x4b03804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b03804c>);
	m_dsmap.SetAt(0x4b0a8864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b0a8864>);
	m_dsmap.SetAt(0x4b0e8064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b0e8064>);
	m_dsmap.SetAt(0x4b0e8864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b0e8864>);
	m_dsmap.SetAt(0x4b1a8064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b1a8064>);
	m_dsmap.SetAt(0x4b1a8864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b1a8864>);
	m_dsmap.SetAt(0x4c80484d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c80484d>);
	m_dsmap.SetAt(0x4c81804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c81804c>);
	m_dsmap.SetAt(0x4c81904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c81904c>);
	m_dsmap.SetAt(0x4c81904d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c81904d>);
	m_dsmap.SetAt(0x4c83804d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c83804d>);
	m_dsmap.SetAt(0x4c83904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c83904c>);
	m_dsmap.SetAt(0x4c83904d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c83904d>);
	m_dsmap.SetAt(0x4c87904d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c87904d>);
	m_dsmap.SetAt(0x4d05804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d05804c>);
	m_dsmap.SetAt(0x5520404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5520404c>);
	m_dsmap.SetAt(0x5520410c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5520410c>);
	m_dsmap.SetAt(0x5fe0404d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe0404d>);
	m_dsmap.SetAt(0x5fe1904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe1904c>);
	m_dsmap.SetAt(0x5fe1914c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe1914c>);
	m_dsmap.SetAt(0x5fe2884c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe2884c>);
	m_dsmap.SetAt(0x5fe2884e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe2884e>);
	m_dsmap.SetAt(0x5fe3904e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe3904e>);
	m_dsmap.SetAt(0x5fe5804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe5804c>);
	m_dsmap.SetAt(0x5fe5904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe5904c>);
	m_dsmap.SetAt(0x5fe5904d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe5904d>);
	m_dsmap.SetAt(0x5fee8064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fee8064>);
	m_dsmap.SetAt(0x5fee8864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fee8864>);
	m_dsmap.SetAt(0x5fee9064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fee9064>);

	// katamary damacy

	m_dsmap.SetAt(0x1fe04056, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04056>);
	m_dsmap.SetAt(0x488041d4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488041d4>);
	m_dsmap.SetAt(0x488041e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488041e4>);
	m_dsmap.SetAt(0x48804864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804864>);
	m_dsmap.SetAt(0x4881004e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4881004e>);
	m_dsmap.SetAt(0x4881804d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4881804d>);
	m_dsmap.SetAt(0x488181cc, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488181cc>);
	m_dsmap.SetAt(0x488181e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488181e4>);
	m_dsmap.SetAt(0x4881904d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4881904d>);
	m_dsmap.SetAt(0x488301cd, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488301cd>);
	m_dsmap.SetAt(0x488301e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488301e4>);
	m_dsmap.SetAt(0x488581d4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488581d4>);
	m_dsmap.SetAt(0x488581e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488581e4>);
	m_dsmap.SetAt(0x488591d4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488591d4>);
	m_dsmap.SetAt(0x488591e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488591e4>);
	m_dsmap.SetAt(0x488681e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488681e4>);
	m_dsmap.SetAt(0x488691e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488691e4>);
	m_dsmap.SetAt(0x48884864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48884864>);
	m_dsmap.SetAt(0x488e81e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e81e4>);
	m_dsmap.SetAt(0x488e89e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e89e4>);
	m_dsmap.SetAt(0x488e9064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e9064>);
	m_dsmap.SetAt(0x488e91d4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e91d4>);
	m_dsmap.SetAt(0x488e91e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e91e4>);
	m_dsmap.SetAt(0x488f8064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488f8064>);
	m_dsmap.SetAt(0x488f81e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488f81e4>);
	m_dsmap.SetAt(0x488f9064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488f9064>);
	m_dsmap.SetAt(0x48904054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48904054>);
	m_dsmap.SetAt(0x489e91cc, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x489e91cc>);
	m_dsmap.SetAt(0x489f91cc, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x489f91cc>);
	m_dsmap.SetAt(0x490581e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490581e4>);
	m_dsmap.SetAt(0x490591e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490591e4>);
	m_dsmap.SetAt(0x490681cc, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490681cc>);
	m_dsmap.SetAt(0x490681e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490681e4>);
	m_dsmap.SetAt(0x490689cc, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490689cc>);
	m_dsmap.SetAt(0x490689e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490689e4>);
	m_dsmap.SetAt(0x490691cc, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490691cc>);
	m_dsmap.SetAt(0x490691e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490691e4>);
	m_dsmap.SetAt(0x490781cc, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490781cc>);
	m_dsmap.SetAt(0x490781e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490781e4>);
	m_dsmap.SetAt(0x490791cc, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490791cc>);
	m_dsmap.SetAt(0x490791e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490791e4>);
	m_dsmap.SetAt(0x490a09e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490a09e4>);
	m_dsmap.SetAt(0x490e81cc, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e81cc>);
	m_dsmap.SetAt(0x490e81e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e81e4>);
	m_dsmap.SetAt(0x490e89e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e89e4>);
	m_dsmap.SetAt(0x490e91e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e91e4>);
	m_dsmap.SetAt(0x50258154, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x50258154>);
	m_dsmap.SetAt(0x502691e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x502691e4>);
	m_dsmap.SetAt(0x502791e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x502791e4>);
	m_dsmap.SetAt(0x5fe04054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe04054>);

	// grandia 3

	m_dsmap.SetAt(0x41268060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x41268060>);
	m_dsmap.SetAt(0x484e8860, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x484e8860>);
	m_dsmap.SetAt(0x48868360, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868360>);
	m_dsmap.SetAt(0x48868760, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868760>);
	m_dsmap.SetAt(0x48868870, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868870>);
	m_dsmap.SetAt(0x48868b60, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868b60>);
	m_dsmap.SetAt(0x48868f60, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868f60>);
	m_dsmap.SetAt(0x48869760, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48869760>);
	m_dsmap.SetAt(0x48878760, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878760>);
	m_dsmap.SetAt(0x48879760, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879760>);
	m_dsmap.SetAt(0x488e8360, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8360>);
	m_dsmap.SetAt(0x488e8870, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8870>);
	m_dsmap.SetAt(0x488e8b60, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8b60>);
	m_dsmap.SetAt(0x488e8f60, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8f60>);
	m_dsmap.SetAt(0x488f8f60, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488f8f60>);
	m_dsmap.SetAt(0x48984070, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48984070>);
	m_dsmap.SetAt(0x49004060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004060>);
	m_dsmap.SetAt(0x49004860, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004860>);
	m_dsmap.SetAt(0x4906804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4906804c>);
	m_dsmap.SetAt(0x49068860, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068860>);
	m_dsmap.SetAt(0x49078060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078060>);
	m_dsmap.SetAt(0x49078070, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078070>);
	m_dsmap.SetAt(0x49078860, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078860>);
	m_dsmap.SetAt(0x49078870, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078870>);
	m_dsmap.SetAt(0x49079060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49079060>);
	m_dsmap.SetAt(0x490e8860, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8860>);
	m_dsmap.SetAt(0x4c839060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c839060>);
	m_dsmap.SetAt(0x50368060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x50368060>);
	m_dsmap.SetAt(0x1fe0405a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe0405a>);
	m_dsmap.SetAt(0x48868070, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868070>);
	m_dsmap.SetAt(0x48869070, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48869070>);
	m_dsmap.SetAt(0x48869360, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48869360>);
	m_dsmap.SetAt(0x48884060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48884060>);
	m_dsmap.SetAt(0x48884760, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48884760>);
	m_dsmap.SetAt(0x488a8060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488a8060>);
	m_dsmap.SetAt(0x488e8760, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8760>);
	m_dsmap.SetAt(0x488e9360, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e9360>);
	m_dsmap.SetAt(0x490e8060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8060>);
	m_dsmap.SetAt(0x4c83964c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c83964c>);
	m_dsmap.SetAt(0x4c839660, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c839660>);

	// rumble roses

	m_dsmap.SetAt(0x1fe78164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe78164>);
	m_dsmap.SetAt(0x1fe79164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe79164>);
	m_dsmap.SetAt(0x48804164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804164>);
	m_dsmap.SetAt(0x48804964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804964>);
	m_dsmap.SetAt(0x48838164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48838164>);
	m_dsmap.SetAt(0x48868964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868964>);
	m_dsmap.SetAt(0x4886b964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4886b964>);
	m_dsmap.SetAt(0x48878164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878164>);
	m_dsmap.SetAt(0x48878964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878964>);
	m_dsmap.SetAt(0x4887b164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887b164>);
	m_dsmap.SetAt(0x4887b964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887b964>);
	m_dsmap.SetAt(0x4887bf64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887bf64>);
	m_dsmap.SetAt(0x488a0064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488a0064>);
	m_dsmap.SetAt(0x488e0b64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e0b64>);
	m_dsmap.SetAt(0x490e804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e804c>);
	m_dsmap.SetAt(0x4c48484c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c48484c>);
	m_dsmap.SetAt(0x4c830164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c830164>);
	m_dsmap.SetAt(0x4c8a0064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c8a0064>);
	m_dsmap.SetAt(0x4c8e0b64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c8e0b64>);
	m_dsmap.SetAt(0x4c8e8864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c8e8864>);
	m_dsmap.SetAt(0x4c8e8964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c8e8964>);
	m_dsmap.SetAt(0x4c8e8b64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c8e8b64>);
	m_dsmap.SetAt(0x55384874, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x55384874>);

	// dmc (fixme)

	m_dsmap.SetAt(0x1fe0424c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe0424c>);
	m_dsmap.SetAt(0x1fe39058, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe39058>);
	m_dsmap.SetAt(0x1fe59068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe59068>);
	m_dsmap.SetAt(0x1fe68968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe68968>);
	m_dsmap.SetAt(0x1fe78068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe78068>);
	m_dsmap.SetAt(0x1fe7814c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe7814c>);
	m_dsmap.SetAt(0x1fe78158, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe78158>);
	m_dsmap.SetAt(0x1fea8968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fea8968>);
	m_dsmap.SetAt(0x1fee8168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee8168>);
	m_dsmap.SetAt(0x1fee8968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee8968>);
	m_dsmap.SetAt(0x1fee8978, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee8978>);
	m_dsmap.SetAt(0x45204078, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x45204078>);
	m_dsmap.SetAt(0x4520424c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4520424c>);
	m_dsmap.SetAt(0x48804078, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804078>);
	m_dsmap.SetAt(0x48810068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48810068>);
	m_dsmap.SetAt(0x48830068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48830068>);
	m_dsmap.SetAt(0x48878058, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878058>);
	m_dsmap.SetAt(0x48878168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878168>);
	m_dsmap.SetAt(0x488e8968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8968>);
	m_dsmap.SetAt(0x49068168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068168>);
	m_dsmap.SetAt(0x49078168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078168>);
	m_dsmap.SetAt(0x490e8168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8168>);
	m_dsmap.SetAt(0x490e8968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8968>);
	m_dsmap.SetAt(0x4c43804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c43804c>);
	m_dsmap.SetAt(0x4c839068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c839068>);
	m_dsmap.SetAt(0x4d068968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d068968>);
	m_dsmap.SetAt(0x4d0e8968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d0e8968>);
	m_dsmap.SetAt(0x54204078, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x54204078>);
	m_dsmap.SetAt(0x5420424c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5420424c>);
	m_dsmap.SetAt(0x5fe30868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe30868>);

	// xenosaga 2

	m_dsmap.SetAt(0x1fe04067, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04067>);
	m_dsmap.SetAt(0x1fe18054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe18054>);
	m_dsmap.SetAt(0x1fe58174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe58174>);
	m_dsmap.SetAt(0x1fe68064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe68064>);
	m_dsmap.SetAt(0x1fe68864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe68864>);
	m_dsmap.SetAt(0x1fe78174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe78174>);
	m_dsmap.SetAt(0x1fe8404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe8404c>);
	m_dsmap.SetAt(0x1fe84064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe84064>);
	m_dsmap.SetAt(0x1fe84864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe84864>);
	m_dsmap.SetAt(0x1fea8864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fea8864>);
	m_dsmap.SetAt(0x1fea8964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fea8964>);
	m_dsmap.SetAt(0x1fee804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee804c>);
	m_dsmap.SetAt(0x1fee884c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee884c>);
	m_dsmap.SetAt(0x1fee9064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee9064>);
	m_dsmap.SetAt(0x48468864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48468864>);
	m_dsmap.SetAt(0x48804074, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804074>);
	m_dsmap.SetAt(0x48839054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48839054>);
	m_dsmap.SetAt(0x48868164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868164>);
	m_dsmap.SetAt(0x488688e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488688e4>);
	m_dsmap.SetAt(0x488a88e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488a88e4>);
	m_dsmap.SetAt(0x4901004c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4901004c>);
	m_dsmap.SetAt(0x4905904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4905904c>);
	m_dsmap.SetAt(0x49068164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068164>);
	m_dsmap.SetAt(0x49069064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49069064>);
	m_dsmap.SetAt(0x49078164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078164>);
	m_dsmap.SetAt(0x49079164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49079164>);
	m_dsmap.SetAt(0x4946824c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4946824c>);
	m_dsmap.SetAt(0x4947824c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4947824c>);
	m_dsmap.SetAt(0x4a478164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a478164>);
	m_dsmap.SetAt(0x4a878164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a878164>);
	m_dsmap.SetAt(0x4b078164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b078164>);
	m_dsmap.SetAt(0x4c404054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c404054>);
	m_dsmap.SetAt(0x4c804054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c804054>);
	m_dsmap.SetAt(0x4c839054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c839054>);
	m_dsmap.SetAt(0x4d004054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d004054>);
	m_dsmap.SetAt(0x4d004064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d004064>);
	m_dsmap.SetAt(0x4d03804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d03804c>);
	m_dsmap.SetAt(0x4d039054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d039054>);
	m_dsmap.SetAt(0x4d068064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d068064>);
	m_dsmap.SetAt(0x4d068864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d068864>);
	m_dsmap.SetAt(0x4d069064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d069064>);
	m_dsmap.SetAt(0x51229064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x51229064>);

	// nfs mw

	m_dsmap.SetAt(0x1fe68168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe68168>);
	m_dsmap.SetAt(0x1fe6816a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe6816a>);
	m_dsmap.SetAt(0x1fe6894e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe6894e>);
	m_dsmap.SetAt(0x1fe6896a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe6896a>);
	m_dsmap.SetAt(0x1fe78168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe78168>);
	m_dsmap.SetAt(0x4805904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4805904c>);
	m_dsmap.SetAt(0x4882814e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4882814e>);
	m_dsmap.SetAt(0x4883814e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883814e>);
	m_dsmap.SetAt(0x48838168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48838168>);
	m_dsmap.SetAt(0x48838968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48838968>);
	m_dsmap.SetAt(0x4886816a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4886816a>);
	m_dsmap.SetAt(0x4886896a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4886896a>);
	m_dsmap.SetAt(0x48869164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48869164>);
	m_dsmap.SetAt(0x48878968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878968>);
	m_dsmap.SetAt(0x4906816a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4906816a>);
	m_dsmap.SetAt(0x49068964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068964>);
	m_dsmap.SetAt(0x49068968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068968>);
	m_dsmap.SetAt(0x4906896a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4906896a>);
	m_dsmap.SetAt(0x4907816a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4907816a>);
	m_dsmap.SetAt(0x49078964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078964>);
	m_dsmap.SetAt(0x49078968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078968>);
	m_dsmap.SetAt(0x4927904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4927904c>);
	m_dsmap.SetAt(0x4a838164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a838164>);
	m_dsmap.SetAt(0x4a838964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a838964>);
	m_dsmap.SetAt(0x4a83904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a83904c>);
	m_dsmap.SetAt(0x4b004064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b004064>);
	m_dsmap.SetAt(0x4b004068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b004068>);
	m_dsmap.SetAt(0x4b004864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b004864>);
	m_dsmap.SetAt(0x4b004868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b004868>);
	m_dsmap.SetAt(0x4b028064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b028064>);
	m_dsmap.SetAt(0x4b028068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b028068>);
	m_dsmap.SetAt(0x4b028864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b028864>);
	m_dsmap.SetAt(0x4b028868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b028868>);
	m_dsmap.SetAt(0x4b038164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b038164>);
	m_dsmap.SetAt(0x4b038964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b038964>);
	m_dsmap.SetAt(0x4c83004c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c83004c>);
	m_dsmap.SetAt(0x4c83804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c83804c>);
	m_dsmap.SetAt(0x4c83804e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c83804e>);
	m_dsmap.SetAt(0x4d03904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d03904c>);
	m_dsmap.SetAt(0x4d03914c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d03914c>);
	m_dsmap.SetAt(0x5127904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5127904c>);
	m_dsmap.SetAt(0x5420404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5420404c>);
	m_dsmap.SetAt(0x5420404d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5420404d>);
	m_dsmap.SetAt(0x5420404e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5420404e>);
	m_dsmap.SetAt(0x5fe1904e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe1904e>);
	m_dsmap.SetAt(0x5fe19064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe19064>);
	m_dsmap.SetAt(0x5fe78064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe78064>);

	// berserk

	m_dsmap.SetAt(0x48804165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804165>);
	m_dsmap.SetAt(0x4883104c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883104c>);
	m_dsmap.SetAt(0x4887804f, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887804f>);
	m_dsmap.SetAt(0x49004874, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004874>);
	m_dsmap.SetAt(0x49078864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078864>);
	m_dsmap.SetAt(0x4c8eb964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c8eb964>);
	m_dsmap.SetAt(0x4c8fb164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c8fb164>);
	m_dsmap.SetAt(0x4c8fb964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c8fb964>);

	// castlevania

	m_dsmap.SetAt(0x1fe1004c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe1004c>);
	m_dsmap.SetAt(0x1fe3104e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3104e>);
	m_dsmap.SetAt(0x1fe5904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe5904c>);
	m_dsmap.SetAt(0x1fe78868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe78868>);
	m_dsmap.SetAt(0x48878868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878868>);
	m_dsmap.SetAt(0x488c8968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488c8968>);
	m_dsmap.SetAt(0x488eb168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488eb168>);
	m_dsmap.SetAt(0x488eb968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488eb968>);
	m_dsmap.SetAt(0x488f8068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488f8068>);
	m_dsmap.SetAt(0x490e9068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e9068>);
	m_dsmap.SetAt(0x4d00407a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d00407a>);

	// okami

	m_dsmap.SetAt(0x1fe18058, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe18058>);
	m_dsmap.SetAt(0x45218058, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x45218058>);
	m_dsmap.SetAt(0x4881804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4881804c>);
	m_dsmap.SetAt(0x48838868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48838868>);
	m_dsmap.SetAt(0x48839158, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48839158>);
	m_dsmap.SetAt(0x48879058, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879058>);
	m_dsmap.SetAt(0x48879068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879068>);
	m_dsmap.SetAt(0x48884168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48884168>);
	m_dsmap.SetAt(0x48884968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48884968>);
	m_dsmap.SetAt(0x488f8168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488f8168>);
	m_dsmap.SetAt(0x488f8968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488f8968>);
	m_dsmap.SetAt(0x4a83804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a83804c>);
	m_dsmap.SetAt(0x4c43904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c43904c>);
	m_dsmap.SetAt(0x4c83104c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c83104c>);
	m_dsmap.SetAt(0x4d03104c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d03104c>);
	m_dsmap.SetAt(0x5fe59068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe59068>);
	m_dsmap.SetAt(0x5fe7104c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe7104c>);
	m_dsmap.SetAt(0x5fe7904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe7904c>);

	// bully

	m_dsmap.SetAt(0x1fe04077, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04077>);
	m_dsmap.SetAt(0x1fe04864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04864>);
	m_dsmap.SetAt(0x488e8a64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8a64>);
	m_dsmap.SetAt(0x49404054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49404054>);
	m_dsmap.SetAt(0x494e8a64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x494e8a64>);
	m_dsmap.SetAt(0x4c20404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c20404c>);
	m_dsmap.SetAt(0x4d068364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d068364>);
	m_dsmap.SetAt(0x4d068b64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d068b64>);
	m_dsmap.SetAt(0x4d07834c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d07834c>);
	m_dsmap.SetAt(0x4d078364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d078364>);
	m_dsmap.SetAt(0x4d078b64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d078b64>);
	m_dsmap.SetAt(0x510e8164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x510e8164>);
	m_dsmap.SetAt(0x510e8964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x510e8964>);
	m_dsmap.SetAt(0x5480404d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5480404d>);
	m_dsmap.SetAt(0x5501904e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5501904e>);
	m_dsmap.SetAt(0x5fe3104c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe3104c>);

	// culdcept

	m_dsmap.SetAt(0x1fe041e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe041e6>);
	m_dsmap.SetAt(0x1fe049e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe049e6>);
	m_dsmap.SetAt(0x1fe181e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe181e6>);
	m_dsmap.SetAt(0x1fe191e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe191e6>);
	m_dsmap.SetAt(0x1fe2a1e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe2a1e6>);
	m_dsmap.SetAt(0x1fe2a9e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe2a9e6>);
	m_dsmap.SetAt(0x1fe3104c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3104c>);
	m_dsmap.SetAt(0x1fe31066, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe31066>);
	m_dsmap.SetAt(0x1fe381e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe381e6>);
	m_dsmap.SetAt(0x1fe391e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe391e6>);
	m_dsmap.SetAt(0x1fe3a1e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3a1e6>);
	m_dsmap.SetAt(0x1fe3a9e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3a9e6>);
	m_dsmap.SetAt(0x1fe581e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe581e6>);
	m_dsmap.SetAt(0x1fe591e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe591e6>);
	m_dsmap.SetAt(0x1fe781e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe781e6>);
	m_dsmap.SetAt(0x1fe791e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe791e6>);
	m_dsmap.SetAt(0x1fe991e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe991e6>);
	m_dsmap.SetAt(0x1feaa1e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1feaa1e6>);
	m_dsmap.SetAt(0x1feaa9e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1feaa9e6>);
	m_dsmap.SetAt(0x1feba1e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1feba1e6>);
	m_dsmap.SetAt(0x1fef81e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fef81e6>);
	m_dsmap.SetAt(0x488049e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488049e6>);
	m_dsmap.SetAt(0x488089e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488089e6>);
	m_dsmap.SetAt(0x488181e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488181e6>);
	m_dsmap.SetAt(0x488191e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488191e6>);
	m_dsmap.SetAt(0x488281e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488281e6>);
	m_dsmap.SetAt(0x488291e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488291e6>);
	m_dsmap.SetAt(0x488381e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488381e6>);
	m_dsmap.SetAt(0x488391e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488391e6>);
	m_dsmap.SetAt(0x488581e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488581e6>);
	m_dsmap.SetAt(0x490049e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490049e6>);
	m_dsmap.SetAt(0x4d0049e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d0049e6>);
	m_dsmap.SetAt(0x4d02a1e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d02a1e6>);
	m_dsmap.SetAt(0x4d02a9e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d02a9e6>);
	m_dsmap.SetAt(0x4d0381e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d0381e6>);
	m_dsmap.SetAt(0x4d0391e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d0391e6>);
	m_dsmap.SetAt(0x5422a1e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5422a1e6>);

	// suikoden 5

	m_dsmap.SetAt(0x1fe3934c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3934c>);
	m_dsmap.SetAt(0x40428068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x40428068>);
	m_dsmap.SetAt(0x40428868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x40428868>);
	m_dsmap.SetAt(0x404a8068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x404a8068>);
	m_dsmap.SetAt(0x404a8868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x404a8868>);
	m_dsmap.SetAt(0x4846834c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4846834c>);
	m_dsmap.SetAt(0x4847834c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4847834c>);
	m_dsmap.SetAt(0x48829368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48829368>);
	m_dsmap.SetAt(0x4883834c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883834c>);
	m_dsmap.SetAt(0x48838368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48838368>);
	m_dsmap.SetAt(0x4883934c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883934c>);
	m_dsmap.SetAt(0x488a8368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488a8368>);
	m_dsmap.SetAt(0x488a8b68, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488a8b68>);
	m_dsmap.SetAt(0x488a9368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488a9368>);
	m_dsmap.SetAt(0x49028068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49028068>);
	m_dsmap.SetAt(0x49028868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49028868>);
	m_dsmap.SetAt(0x4906834c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4906834c>);
	m_dsmap.SetAt(0x4907834c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4907834c>);
	m_dsmap.SetAt(0x490a8068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490a8068>);
	m_dsmap.SetAt(0x490a8868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490a8868>);
	m_dsmap.SetAt(0x4d068068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d068068>);
	m_dsmap.SetAt(0x4d068868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d068868>);

	// dq8

	m_dsmap.SetAt(0x1fe3914c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3914c>);
	m_dsmap.SetAt(0x48404164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48404164>);
	m_dsmap.SetAt(0x488b0164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488b0164>);
	m_dsmap.SetAt(0x488b0964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488b0964>);
	m_dsmap.SetAt(0x48c3804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48c3804c>);
	m_dsmap.SetAt(0x49004164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004164>);
	m_dsmap.SetAt(0x4c83914c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c83914c>);
	m_dsmap.SetAt(0x5103804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5103804c>);
	m_dsmap.SetAt(0x5103b04c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5103b04c>);
	m_dsmap.SetAt(0x1fe7804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe7804c>);
	m_dsmap.SetAt(0x48884164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48884164>);
	m_dsmap.SetAt(0x48884964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48884964>);
	m_dsmap.SetAt(0x490e914c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e914c>);
	m_dsmap.SetAt(0x490f914c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490f914c>);

	// resident evil 4

	m_dsmap.SetAt(0x1fe18064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe18064>);
	m_dsmap.SetAt(0x4886894c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4886894c>);
	m_dsmap.SetAt(0x49004b64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004b64>);
	m_dsmap.SetAt(0x4903904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4903904c>);
	m_dsmap.SetAt(0x4b068164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b068164>);
	m_dsmap.SetAt(0x4c804164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c804164>);
	m_dsmap.SetAt(0x4c828964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c828964>);
	m_dsmap.SetAt(0x4c879164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c879164>);
	m_dsmap.SetAt(0x4d07814c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d07814c>);
	m_dsmap.SetAt(0x5120404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5120404c>);
	m_dsmap.SetAt(0x5483904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5483904c>);

	// tomoyo after 

	m_dsmap.SetAt(0x1fe38059, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe38059>);
	m_dsmap.SetAt(0x1fe39059, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe39059>);
	m_dsmap.SetAt(0x48404868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48404868>);
	m_dsmap.SetAt(0x48478068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48478068>);
	m_dsmap.SetAt(0x48818068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48818068>);
	m_dsmap.SetAt(0x48858068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48858068>);
	m_dsmap.SetAt(0x49058068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49058068>);
	m_dsmap.SetAt(0x4a858068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a858068>);

	// .hack redemption

	m_dsmap.SetAt(0x1fe04b64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04b64>);
	m_dsmap.SetAt(0x1fe1804d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe1804d>);
	m_dsmap.SetAt(0x1fe1914c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe1914c>);
	m_dsmap.SetAt(0x4123004c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4123004c>);
	m_dsmap.SetAt(0x48404074, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48404074>);
	m_dsmap.SetAt(0x48468064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48468064>);
	m_dsmap.SetAt(0x48469064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48469064>);
	m_dsmap.SetAt(0x48478064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48478064>);
	m_dsmap.SetAt(0x48478864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48478864>);
	m_dsmap.SetAt(0x48830064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48830064>);
	m_dsmap.SetAt(0x48848064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48848064>);
	m_dsmap.SetAt(0x48858064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48858064>);
	m_dsmap.SetAt(0x48859064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48859064>);
	m_dsmap.SetAt(0x48869364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48869364>);
	m_dsmap.SetAt(0x48878b64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878b64>);
	m_dsmap.SetAt(0x48879364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879364>);
	m_dsmap.SetAt(0x488a8b64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488a8b64>);
	m_dsmap.SetAt(0x488e9364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e9364>);
	m_dsmap.SetAt(0x488f8364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488f8364>);
	m_dsmap.SetAt(0x488f9364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488f9364>);
	m_dsmap.SetAt(0x49004074, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004074>);
	m_dsmap.SetAt(0x49079064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49079064>);
	m_dsmap.SetAt(0x4c41804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c41804c>);
	m_dsmap.SetAt(0x4c41904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c41904c>);
	m_dsmap.SetAt(0x4d00404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d00404c>);
	m_dsmap.SetAt(0x5fe1004c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe1004c>);

	// wild arms 5

	m_dsmap.SetAt(0x1fe19050, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe19050>);
	m_dsmap.SetAt(0x1fef8064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fef8064>);
	m_dsmap.SetAt(0x4845804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4845804c>);
	m_dsmap.SetAt(0x4845904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4845904c>);
	m_dsmap.SetAt(0x48804854, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804854>);
	m_dsmap.SetAt(0x4885884c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4885884c>);
	m_dsmap.SetAt(0x48c68864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48c68864>);
	m_dsmap.SetAt(0x4b068864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b068864>);
	m_dsmap.SetAt(0x4d078064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d078064>);
	m_dsmap.SetAt(0x4d078164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d078164>);
	m_dsmap.SetAt(0x5fe19054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe19054>);
	m_dsmap.SetAt(0x5fe39054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe39054>);

	// shadow of the colossus

	m_dsmap.SetAt(0x1fe04164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04164>);
	m_dsmap.SetAt(0x1fe1004d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe1004d>);
	m_dsmap.SetAt(0x484e8864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x484e8864>);
	m_dsmap.SetAt(0x48838864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48838864>);
	m_dsmap.SetAt(0x48859364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48859364>);
	m_dsmap.SetAt(0x48868b24, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868b24>);
	m_dsmap.SetAt(0x488b0864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488b0864>);
	m_dsmap.SetAt(0x488e8264, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8264>);
	m_dsmap.SetAt(0x488f8864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488f8864>);
	m_dsmap.SetAt(0x48938064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48938064>);
	m_dsmap.SetAt(0x48939064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48939064>);
	m_dsmap.SetAt(0x490c8364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490c8364>);
	m_dsmap.SetAt(0x490e8364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8364>);
	m_dsmap.SetAt(0x490f8064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490f8064>);
	m_dsmap.SetAt(0x490f8864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490f8864>);
	m_dsmap.SetAt(0x688b8064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x688b8064>);
	m_dsmap.SetAt(0x688b8864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x688b8864>);
	m_dsmap.SetAt(0x688b9064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x688b9064>);

	// tales of redemption

	m_dsmap.SetAt(0x4827824c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4827824c>);
	m_dsmap.SetAt(0x48804254, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804254>);
	m_dsmap.SetAt(0x48804b4c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804b4c>);
	m_dsmap.SetAt(0x48804b74, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804b74>);
	m_dsmap.SetAt(0x48868074, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868074>);
	m_dsmap.SetAt(0x48878374, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878374>);
	m_dsmap.SetAt(0x48878864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878864>);
	m_dsmap.SetAt(0x48879164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879164>);
	m_dsmap.SetAt(0x48879374, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879374>);
	m_dsmap.SetAt(0x488b9054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488b9054>);
	m_dsmap.SetAt(0x4907824c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4907824c>);
	m_dsmap.SetAt(0x49078264, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078264>);
	m_dsmap.SetAt(0x4c838064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c838064>);
	m_dsmap.SetAt(0x4c838364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c838364>);
	m_dsmap.SetAt(0x4c838854, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c838854>);
	m_dsmap.SetAt(0x4c838b64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c838b64>);

	// digital devil saga

	m_dsmap.SetAt(0x1fe04053, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04053>);
	m_dsmap.SetAt(0x1fe39070, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe39070>);
	m_dsmap.SetAt(0x40204250, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x40204250>);
	m_dsmap.SetAt(0x48404050, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48404050>);
	m_dsmap.SetAt(0x48404870, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48404870>);
	m_dsmap.SetAt(0x48468070, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48468070>);
	m_dsmap.SetAt(0x484e8070, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x484e8070>);
	m_dsmap.SetAt(0x484e8870, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x484e8870>);
	m_dsmap.SetAt(0x484e9070, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x484e9070>);
	m_dsmap.SetAt(0x48804050, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804050>);
	m_dsmap.SetAt(0x48804060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804060>);
	m_dsmap.SetAt(0x48804150, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804150>);
	m_dsmap.SetAt(0x48804360, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804360>);
	m_dsmap.SetAt(0x48804b60, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804b60>);
	m_dsmap.SetAt(0x48804b70, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804b70>);
	m_dsmap.SetAt(0x48804f50, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804f50>);
	m_dsmap.SetAt(0x48858360, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48858360>);
	m_dsmap.SetAt(0x48868b70, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868b70>);
	m_dsmap.SetAt(0x48878060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878060>);
	m_dsmap.SetAt(0x48878150, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878150>);
	m_dsmap.SetAt(0x48878360, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878360>);
	m_dsmap.SetAt(0x48878750, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878750>);
	m_dsmap.SetAt(0x48879360, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879360>);
	m_dsmap.SetAt(0x48879750, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879750>);
	m_dsmap.SetAt(0x48884360, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48884360>);
	m_dsmap.SetAt(0x48884870, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48884870>);
	m_dsmap.SetAt(0x48884b60, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48884b60>);
	m_dsmap.SetAt(0x48884b70, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48884b70>);
	m_dsmap.SetAt(0x488a8b70, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488a8b70>);
	m_dsmap.SetAt(0x488e8060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8060>);
	m_dsmap.SetAt(0x488e8370, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8370>);
	m_dsmap.SetAt(0x488e8860, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8860>);
	m_dsmap.SetAt(0x488e8b70, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8b70>);
	m_dsmap.SetAt(0x488f8360, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488f8360>);
	m_dsmap.SetAt(0x4890404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4890404c>);
	m_dsmap.SetAt(0x48904270, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48904270>);
	m_dsmap.SetAt(0x4890484c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4890484c>);
	m_dsmap.SetAt(0x48904a70, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48904a70>);
	m_dsmap.SetAt(0x4897904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4897904c>);
	m_dsmap.SetAt(0x4898404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4898404c>);
	m_dsmap.SetAt(0x49004070, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004070>);
	m_dsmap.SetAt(0x49004870, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004870>);
	m_dsmap.SetAt(0x49004b60, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004b60>);
	m_dsmap.SetAt(0x49004f50, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004f50>);
	m_dsmap.SetAt(0x49028060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49028060>);
	m_dsmap.SetAt(0x49028860, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49028860>);
	m_dsmap.SetAt(0x49048060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49048060>);
	m_dsmap.SetAt(0x49048860, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49048860>);
	m_dsmap.SetAt(0x49068060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068060>);
	m_dsmap.SetAt(0x49068070, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068070>);
	m_dsmap.SetAt(0x49068870, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068870>);
	m_dsmap.SetAt(0x49078360, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078360>);
	m_dsmap.SetAt(0x49078750, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078750>);
	m_dsmap.SetAt(0x49079360, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49079360>);
	m_dsmap.SetAt(0x4908404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4908404c>);
	m_dsmap.SetAt(0x49084070, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49084070>);
	m_dsmap.SetAt(0x49084870, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49084870>);
	m_dsmap.SetAt(0x490e8070, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8070>);
	m_dsmap.SetAt(0x490e8870, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8870>);
	m_dsmap.SetAt(0x4910404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4910404c>);
	m_dsmap.SetAt(0x4a878060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a878060>);
	m_dsmap.SetAt(0x4a879060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a879060>);
	m_dsmap.SetAt(0x51284870, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x51284870>);
	m_dsmap.SetAt(0x51284b60, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x51284b60>);

	// dbzbt2

	m_dsmap.SetAt(0x1fe3004c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3004c>);
	m_dsmap.SetAt(0x4881904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4881904c>);
	m_dsmap.SetAt(0x488391cc, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488391cc>);
	m_dsmap.SetAt(0x4885904e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4885904e>);
	m_dsmap.SetAt(0x48859074, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48859074>);
	m_dsmap.SetAt(0x488781cc, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488781cc>);
	m_dsmap.SetAt(0x48878674, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878674>);
	m_dsmap.SetAt(0x4887874c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887874c>);
	m_dsmap.SetAt(0x488791cc, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488791cc>);
	m_dsmap.SetAt(0x4887970c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887970c>);
	m_dsmap.SetAt(0x4887974c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887974c>);
	m_dsmap.SetAt(0x48968064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48968064>);
	m_dsmap.SetAt(0x48968864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48968864>);
	m_dsmap.SetAt(0x4897804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4897804c>);
	m_dsmap.SetAt(0x4a40404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a40404c>);
	m_dsmap.SetAt(0x4a83104c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a83104c>);
	m_dsmap.SetAt(0x4b069064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b069064>);
	m_dsmap.SetAt(0x4b07904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b07904c>);
	m_dsmap.SetAt(0x4c45904e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c45904e>);
	m_dsmap.SetAt(0x4c469064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c469064>);
	m_dsmap.SetAt(0x4c80404e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c80404e>);
	m_dsmap.SetAt(0x4c81004c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c81004c>);
	m_dsmap.SetAt(0x4c8391cc, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c8391cc>);
	m_dsmap.SetAt(0x4c869064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c869064>);
	m_dsmap.SetAt(0x4c904064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c904064>);
	m_dsmap.SetAt(0x4d03004c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d03004c>);
	m_dsmap.SetAt(0x543081e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x543081e4>);
	m_dsmap.SetAt(0x4a87804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a87804c>);
	m_dsmap.SetAt(0x4a87904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a87904c>);
	m_dsmap.SetAt(0x543089e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x543089e4>);

	// dbzbt3

	m_dsmap.SetAt(0x4881104c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4881104c>);
	m_dsmap.SetAt(0x489081e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x489081e4>);
	m_dsmap.SetAt(0x4917804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4917804c>);
	m_dsmap.SetAt(0x49268864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49268864>);
	m_dsmap.SetAt(0x4a879164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a879164>);

	// dbz sagas

	// disgaea 2

	m_dsmap.SetAt(0x1fe04364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04364>);
	m_dsmap.SetAt(0x1fe68174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe68174>);
	m_dsmap.SetAt(0x1fe69174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe69174>);
	m_dsmap.SetAt(0x1fe79174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe79174>);
	m_dsmap.SetAt(0x48468364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48468364>);
	m_dsmap.SetAt(0x48468b64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48468b64>);
	m_dsmap.SetAt(0x48478364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48478364>);
	m_dsmap.SetAt(0x48478b64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48478b64>);
	m_dsmap.SetAt(0x48820b64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48820b64>);
	m_dsmap.SetAt(0x49078b64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078b64>);
	m_dsmap.SetAt(0x4c469164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c469164>);

	// resident evil code veronica

	// Gundam Seed Destiny OMNI VS ZAFT II PLUS 

	// grandia extreme

	// the punisher

	// ico

	// kuon

	// virtual tennis 2

	// one piece grand battle 3

	// one piece grand adventure

	// enthusai

	// prince of tennis

	// crash wrath of cortex

	// hxh

	// ys 1/2 eternal story

	// sbam 2

	// armored core 3

	// bloody roar

	// guitar hero

	// aerial planet

	// gradius 5

	// ferrari f355 challenge

	// onimusha 3

	// king of fighters xi

	// nba 2k8

	// mana khemia

	// shadow hearts

	// tourist trophy

	// ar tonelico

	// ar tonelico 2

	// svr2k8

	// remember 11

	// tales of abyss

	// tokyo bus guide

	// 12riven

	// xenosaga

	// mgs3s1

	// rouge galaxy

	// god of war

	#endif

	POSITION pos = m_dsmap.GetHeadPosition();
	
	while(pos)
	{
		CRBMap<DWORD, DrawScanlinePtr>::CPair* pair = m_dsmap.GetNext(pos);

		GSScanlineSelector sel;

		sel.dw = pair->m_key;

		if(sel.atst == ATST_LESS)
		{
			sel.atst = ATST_LEQUAL;
			
			if(m_dsmap.Lookup(sel))
			{
				printf("*** %08x\n", sel.dw);
			}
		}
		else if(sel.atst == ATST_GREATER)
		{
			sel.atst = ATST_GEQUAL;
			
			if(m_dsmap.Lookup(sel))
			{
				printf("*** %08x\n", sel.dw);
			}
		}
	}
}

template<DWORD fpsm, DWORD zpsm, DWORD ztst, DWORD iip>
void GSDrawScanline::DrawScanlineT(int top, int left, int right, const GSVertexSW& v)	
{
	const GSVector4 ps0123 = GSVector4::ps0123();

	GSVector4i fa_base;
	GSVector4i* fa_offset;
	
	GSVector4i za_base;
	GSVector4i* za_offset;

	GSVector4 z, s, t, q;
	GSVector4i f, rb, ga;

	// fa

	fa_base = m_env.fbr[top];
	fa_offset = (GSVector4i*)&m_env.fbc[left & 3][left];

	// za

	za_base = m_env.zbr[top];
	za_offset = (GSVector4i*)&m_env.zbc[left & 3][left];

	// v.p

	GSVector4 vp = v.p;

	z = vp.zzzz() + m_env.dz;
	f = GSVector4i(vp).zzzzh().zzzz().add16(m_env.df);

	// v.t

	GSVector4 vt = v.t;
	GSVector4 dt = m_env.dt;
	
	s = vt.xxxx() + dt.xxxx() * ps0123; 
	t = vt.yyyy() + dt.yyyy() * ps0123; 
	q = vt.zzzz() + dt.zzzz() * ps0123; 
	
	// v.c

	GSVector4i vc = GSVector4i(v.c);

	vc = vc.upl16(vc.zwxy());

	rb = vc.xxxx();
	ga = vc.zzzz();

	if(iip)
	{
		rb = rb.add16(m_env.drb);
		ga = ga.add16(m_env.dga);
	}

	//

	int steps = right - left;

	while(1)
	{
		do
		{
			GSVector4i test = GSVector4i::zero();

			GSVector4i za = za_base + *za_offset;
			
			GSVector4i zs = (GSVector4i(z * 0.5f) << 1) | (GSVector4i(z) & GSVector4i::x00000001(za));

			if(!TestZ(zpsm, ztst, zs, za, test))
			{
				continue;
			}

			int pixels = GSVector4i::min_i16(steps, 4);

			GSVector4i c[6];

			if(m_env.sel.tfx != TFX_NONE)
			{
				SampleTexture(pixels, ztst, m_env.sel.fst, m_env.sel.ltf, m_env.sel.tlu, test, s, t, q, c);
			}

			AlphaTFX(m_env.sel.tfx, m_env.sel.tcc, ga, c[1]);

			GSVector4i fm = m_env.fm;
			GSVector4i zm = m_env.zm;

			if(!TestAlpha(m_env.sel.atst, m_env.sel.afail, c[1], fm, zm, test))
			{
				continue;
			}

			ColorTFX(m_env.sel.tfx, rb, ga, c[0], c[1]);

			Fog(m_env.sel.fge, f, c[0], c[1]);

			GSVector4i fa = fa_base + *fa_offset;

			GSVector4i fd = GSVector4i::zero();

			if(m_env.sel.rfb)
			{
				fd = ReadFrameX(fpsm == 1 ? 0 : fpsm, fa);

				if(!TestDestAlpha(fpsm, m_env.sel.date, fd, test))
				{
					continue;
				}
			}

			fm |= test;
			zm |= test;

			if(m_env.sel.abe != 255)
			{
				GSVector4i mask = GSVector4i::x00ff();

				c[2] = fd & mask;
				c[3] = (fd >> 8) & mask;

				if(fpsm == 1)
				{
					c[3] = c[3].mix16(GSVector4i(0x00800000));
				}

				c[4] = GSVector4::zero();
				c[5] = m_env.afix;

				DWORD abea = m_env.sel.abea;
				DWORD abeb = m_env.sel.abeb;
				DWORD abec = m_env.sel.abec;
				DWORD abed = m_env.sel.abed;

				GSVector4i a = c[abec * 2 + 1].yywwl().yywwh().sll16(7);

				GSVector4i rb = GSVector4i::lerp16<1>(c[abea * 2 + 0], c[abeb * 2 + 0], a, c[abed * 2 + 0]);
				GSVector4i ga = GSVector4i::lerp16<1>(c[abea * 2 + 1], c[abeb * 2 + 1], a, c[abed * 2 + 1]);

				if(m_env.sel.pabe)
				{
					GSVector4i mask = (c[1] << 8).sra32(31);

					rb = c[0].blend8(rb, mask);
					ga = c[1].blend8(ga, mask);
				}

				c[0] = rb;
				c[1] = ga.mix16(c[1]);
			}

			c[0] &= m_env.colclamp;
			c[1] &= m_env.colclamp;

			GSVector4i fs = c[0].upl16(c[1]).pu16(c[0].uph16(c[1]));

			if(fpsm != 1)
			{
				fs |= m_env.fba;
			}

			if(m_env.sel.rfb)
			{
				fs = fs.blend(fd, fm);
			}

			WriteFrameAndZBufX(fpsm, fa, fm, fs, ztst > 0 ? zpsm : 3, za, zm, zs, pixels);
		}
		while(0);

		if(steps <= 4) break;

		steps -= 4;

		fa_offset++;
		za_offset++;

		z += m_env.dz4;
		f = f.add16(m_env.df4);

		GSVector4 dt4 = m_env.dt4;

		s += dt4.xxxx();
		t += dt4.yyyy();
		q += dt4.zzzz();

		if(iip)
		{
			GSVector4i dc4 = m_env.dc4;

			rb = rb.add16(dc4.xxxx());
			ga = ga.add16(dc4.yyyy());
		}
	}
}

template<DWORD sel>
void GSDrawScanline::DrawScanlineExT(int top, int left, int right, const GSVertexSW& v)
{
	const DWORD fpsm = (sel >> 0) & 3;
	const DWORD zpsm = (sel >> 2) & 3;
	const DWORD ztst = (sel >> 4) & 3;
	const DWORD atst = (sel >> 6) & 7;
	const DWORD afail = (sel >> 9) & 3;
	const DWORD iip = (sel >> 11) & 1;
	const DWORD tfx = (sel >> 12) & 7;
	const DWORD tcc = (sel >> 15) & 1;
	const DWORD fst = (sel >> 16) & 1;
	const DWORD ltf = (sel >> 17) & 1;
	const DWORD tlu = (sel >> 18) & 1;
	const DWORD fge = (sel >> 19) & 1;
	const DWORD date = (sel >> 20) & 1;
	const DWORD abe = (sel >> 21) & 255;
	const DWORD abea = (sel >> 21) & 3;
	const DWORD abeb = (sel >> 23) & 3;
	const DWORD abec = (sel >> 25) & 3;
	const DWORD abed = (sel >> 27) & 3;
	const DWORD pabe = (sel >> 29) & 1;
	const DWORD rfb = (sel >> 30) & 1;

	const GSVector4 ps0123 = GSVector4::ps0123();

	GSVector4i fa_base;
	GSVector4i* fa_offset;
	
	GSVector4i za_base;
	GSVector4i* za_offset;

	GSVector4 z, s, t, q;
	GSVector4i f, rb, ga;

	// fa

	fa_base = m_env.fbr[top];
	fa_offset = (GSVector4i*)&m_env.fbc[left & 3][left];

	// za

	za_base = m_env.zbr[top];
	za_offset = (GSVector4i*)&m_env.zbc[left & 3][left];

	// v.p

	GSVector4 vp = v.p;

	z = vp.zzzz() + m_env.dz;
	f = GSVector4i(vp).zzzzh().zzzz().add16(m_env.df);

	// v.t

	GSVector4 vt = v.t;
	GSVector4 dt = m_env.dt;

	s = vt.xxxx() + dt.xxxx() * ps0123; 
	t = vt.yyyy() + dt.yyyy() * ps0123; 
	q = vt.zzzz() + dt.zzzz() * ps0123; 

	// v.c

	GSVector4i vc = GSVector4i(v.c);

	vc = vc.upl16(vc.zwxy());

	rb = vc.xxxx();
	ga = vc.zzzz();

	if(iip)
	{
		rb = rb.add16(m_env.drb);
		ga = ga.add16(m_env.dga);
	}

	//

	int steps = right - left;

	while(1)
	{
		do
		{
			GSVector4i test = GSVector4i::zero();

			GSVector4i za = za_base + *za_offset;
			
			GSVector4i zs = (GSVector4i(z * 0.5f) << 1) | (GSVector4i(z) & GSVector4i::x00000001(za));

			if(!TestZ(zpsm, ztst, zs, za, test))
			{
				continue;
			}

			int pixels = GSVector4i::min_i16(steps, 4);

			GSVector4i c[6];

			if(tfx != TFX_NONE)
			{
				SampleTexture(pixels, ztst, fst, ltf, tlu, test, s, t, q, c);
			}
			
			AlphaTFX(tfx, tcc, ga, c[1]);

			GSVector4i fm = m_env.fm;
			GSVector4i zm = m_env.zm;

			if(!TestAlpha(atst, afail, c[1], fm, zm, test))
			{
				continue;
			}

			ColorTFX(tfx, rb, ga, c[0], c[1]);

			Fog(fge, f, c[0], c[1]);

			GSVector4i fa = fa_base + *fa_offset;

			GSVector4i fd = GSVector4i::zero();

			if(rfb)
			{
				fd = ReadFrameX(fpsm == 1 ? 0 : fpsm, fa);

				if(!TestDestAlpha(fpsm, date, fd, test))
				{
					continue;
				}
			}

			if(ztst > 1 || atst != ATST_ALWAYS || date)
			{
				fm |= test;
				zm |= test;
			}

			if(abe != 255)
			{
				GSVector4i mask = GSVector4i::x00ff();

				c[2] = fd & mask;
				c[3] = (fd >> 8) & mask;

				c[4] = GSVector4::zero();
				c[5] = GSVector4::zero();

				GSVector4i rb, ga;

				if(abea != abeb)
				{
					rb = c[abea * 2 + 0];
					ga = c[abea * 2 + 1];

					if(abeb != 2)
					{
						rb = rb.sub16(c[abeb * 2 + 0]);
						ga = ga.sub16(c[abeb * 2 + 1]);
					}

					if(!(fpsm == 1 && abec == 1))
					{
						GSVector4i a = abec < 2 ? c[abec * 2 + 1].yywwl().yywwh().sll16(7) : m_env.afix2;

						rb = rb.modulate16<1>(a);
						ga = ga.modulate16<1>(a);
					}

					if(abed < 2)
					{
						rb = rb.add16(c[abed * 2 + 0]);
						ga = ga.add16(c[abed * 2 + 1]);
					}
				}
				else
				{
					rb = c[abed * 2 + 0];
					ga = c[abed * 2 + 1];
				}

				if(pabe)
				{
					GSVector4i mask = (c[1] << 8).sra32(31);

					rb = c[0].blend8(rb, mask);
					ga = c[1].blend8(ga, mask);
				}

				c[0] = rb;
				c[1] = ga.mix16(c[1]);
			}

			c[0] &= m_env.colclamp;
			c[1] &= m_env.colclamp;

			GSVector4i fs = c[0].upl16(c[1]).pu16(c[0].uph16(c[1]));

			if(fpsm != 1)
			{
				fs |= m_env.fba;
			}

			if(rfb)
			{
				fs = fs.blend(fd, fm);
			}

			WriteFrameAndZBufX(fpsm, fa, fm, fs, ztst > 0 ? zpsm : 3, za, zm, zs, pixels);
		}
		while(0);

		if(steps <= 4) break;

		steps -= 4;

		fa_offset++;
		za_offset++;

		z += m_env.dz4;
		f = f.add16(m_env.df4);

		GSVector4 dt4 = m_env.dt4;

		s += dt4.xxxx();
		t += dt4.yyyy();
		q += dt4.zzzz();

		if(iip)
		{
			GSVector4i dc4 = m_env.dc4;

			rb = rb.add16(dc4.xxxx());
			ga = ga.add16(dc4.yyyy());
		}
	}
}
