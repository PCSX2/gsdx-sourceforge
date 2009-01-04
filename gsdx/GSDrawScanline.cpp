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

GSDrawScanline::GSDrawScanline(GSState* state, int id)
	: m_state(state)
	, m_id(id)
{
	memset(&m_env, 0, sizeof(m_env));

	Init();
}

GSDrawScanline::~GSDrawScanline()
{
}

// IDrawScanline

void GSDrawScanline::BeginDraw(const GSRasterizerData* data, Functions* f)
{
	GSDrawingEnvironment& env = m_state->m_env;
	GSDrawingContext* context = m_state->m_context;

	const GSScanlineParam* p = (const GSScanlineParam*)data->param;

	m_env.sel = p->sel;

	m_env.vm = p->vm;
	m_env.fbr = p->fbo->row;
	m_env.zbr = p->zbo->row;
	m_env.fbc = p->fbo->col;
	m_env.zbc = p->zbo->col;
	m_env.fzbc = p->fzbo->col;
	m_env.fzbr = p->fzbo->row;
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
	else if(m_env.sel.fpsm == 2)
	{
		GSVector4i rb = m_env.fm & 0x00f800f8;
		GSVector4i ga = m_env.fm & 0x8000f800;

		m_env.fm = (ga >> 16) | (rb >> 9) | (ga >> 6) | (rb >> 3) | GSVector4i::xffff0000();
	}

	if(m_env.sel.zpsm == 1)
	{
		m_env.zm |= GSVector4i::xff000000();
	}
	else if(m_env.sel.zpsm == 2)
	{
		m_env.zm |= GSVector4i::xffff0000();
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

	f->sl = m_ds.Lookup(m_env.sel);

	//

	if(data->primclass == GS_SPRITE_CLASS)
	{
		f->sr = (DrawSolidRectPtr)&GSDrawScanline::DrawSolidRect;

		if(m_env.sel.iip 
		|| m_env.sel.tfx != TFX_NONE
		|| m_env.sel.abe != 255 
		|| m_env.sel.ztst > 1 
		|| m_env.sel.atst > 1
		|| m_env.sel.date)
		{
			f->sr = NULL;
		}
	}
}

void GSDrawScanline::EndDraw(const GSRasterizerStats& stats)
{
	m_ds.UpdateStats(stats, m_state->m_perfmon.GetFrame());
}

void GSDrawScanline::SetupPrim(GS_PRIM_CLASS primclass, const GSVertexSW* vertices, const GSVertexSW& dscan)
{
	if(primclass == GS_POINT_CLASS)
	{
		GSVector4i c = GSVector4i(vertices[0].c);

		c = c.upl16(c.zwxy());

		m_env.rb = c.xxxx();
		m_env.ga = c.zzzz();

		return;
	}

	if(m_env.sel.ztst)
	{
		GSVector4 z = dscan.p.zzzz();

		for(int i = 0; i < 4; i++)
		{
			m_env.d[i].z = z * s_ps0123[i];
		}

		m_env.d4.z = z * 4.0f;
	}

	if(m_env.sel.fge)
	{
		GSVector4 f = dscan.p.wwww();

		for(int i = 0; i < 4; i++)
		{
			m_env.d[i].f = GSVector4i(f * s_ps0123[i]).xxzzl().xxzzh();
		}

		m_env.d4.f = GSVector4i(f * 4.0f).xxzzl().xxzzh();
	}

	if(m_env.sel.tfx != TFX_NONE)
	{
		GSVector4 t = dscan.t;

		GSVector4 ds = t.xxxx();
		GSVector4 dt = t.yyyy();
		GSVector4 dq = t.zzzz();

		for(int i = 0; i < 4; i++)
		{
			GSVector4 ps0123 = s_ps0123[i];

			m_env.d[i].s = ds * ps0123;
			m_env.d[i].t = dt * ps0123;
			m_env.d[i].q = dq * ps0123;
		}

		m_env.d4.stq = t * 4.0f;
	}

	if(m_env.sel.iip)
	{
		GSVector4 c = dscan.c;

		GSVector4 dr = c.xxxx();
		GSVector4 dg = c.yyyy();
		GSVector4 db = c.zzzz();
		GSVector4 da = c.wwww();

		for(int i = 0; i < 4; i++)
		{
			GSVector4 ps0123 = s_ps0123[i];

			GSVector4i rg = GSVector4i(dr * ps0123).ps32(GSVector4i(dg * ps0123));
			GSVector4i ba = GSVector4i(db * ps0123).ps32(GSVector4i(da * ps0123));

			m_env.d[i].rb = rg.upl16(ba);
			m_env.d[i].ga = rg.uph16(ba);
		}

		m_env.d4.c = GSVector4i(c * 4.0f).xzyw().ps32();
	}
	else
	{
		GSVector4i c = GSVector4i(vertices[0].c);

		c = c.upl16(c.zwxy());

		m_env.rb = c.xxxx();
		m_env.ga = c.zzzz();
	}
}

void GSDrawScanline::DrawSolidRect(const GSVector4i& r, const GSVertexSW& v)
{
/*
static FILE* s_fp = NULL;
if(!s_fp) s_fp = fopen("c:\\log2.txt", "w");
__int64 start = __rdtsc();
int size = (r.z - r.x) * (r.w - r.y);
*/
	ASSERT(r.y >= 0);
	ASSERT(r.w >= 0);

	DWORD m = m_env.fm.u32[0];

	if(m_env.sel.fpsm == 1)
	{
		m |= 0xff000000;
	}

	if(m != 0xffffffff)
	{
		DWORD c = (GSVector4i(v.c) >> 7).rgba32();

		if(m_state->m_context->FBA.FBA)
		{
			c |= 0x80000000;
		}
		
		if(m_env.sel.fpsm != 2)
		{
			if(m == 0)
			{
				DrawSolidRectT<DWORD, false>(m_env.fbr, m_env.fbc[0], r, c, m);
			}
			else
			{
				DrawSolidRectT<DWORD, true>(m_env.fbr, m_env.fbc[0], r, c, m);
			}
		}
		else
		{
			c = ((c & 0xf8) >> 3) | ((c & 0xf800) >> 6) | ((c & 0xf80000) >> 9) | ((c & 0x80000000) >> 16);

			if(m == 0)
			{
				DrawSolidRectT<WORD, false>(m_env.fbr, m_env.fbc[0], r, c, m);
			}
			else
			{
				DrawSolidRectT<WORD, true>(m_env.fbr, m_env.fbc[0], r, c, m);
			}
		}
	}

	m = m_env.zm.u32[0];

	if(m_env.sel.zpsm == 1)
	{
		m |= 0xff000000;
	}

	if(m != 0xffffffff)
	{
		DWORD z = (DWORD)(float)v.p.z;

		if(m_env.sel.zpsm != 2)
		{
			if(m == 0)
			{
				DrawSolidRectT<DWORD, false>(m_env.zbr, m_env.zbc[0], r, z, m);
			}
			else
			{
				DrawSolidRectT<DWORD, true>(m_env.zbr, m_env.zbc[0], r, z, m);
			}
		}
		else
		{
			if(m == 0)
			{
				DrawSolidRectT<WORD, false>(m_env.zbr, m_env.zbc[0], r, z, m);
			}
			else
			{
				DrawSolidRectT<WORD, true>(m_env.zbr, m_env.zbc[0], r, z, m);
			}
		}
	}
/*
__int64 stop = __rdtsc();
fprintf(s_fp, "%I64d => %I64d = %I64d (%d,%d - %d,%d) %d\n", start, stop, stop - start, r.x, r.y, r.z, r.w, size);
*/
}

template<class T, bool masked> 
void GSDrawScanline::DrawSolidRectT(const GSVector4i* row, int* col, const GSVector4i& r, DWORD c, DWORD m)
{
	if(m == 0xffffffff) return;

	GSVector4i color((int)c);
	GSVector4i mask((int)m);

	if(sizeof(T) == sizeof(WORD))
	{
		color = color.xxzzl().xxzzh();
		mask = mask.xxzzl().xxzzh();
	}

	color = color.andnot(mask);

	GSVector4i bm(8 * 4 / sizeof(T) - 1, 8 - 1);
	GSVector4i br = (r + bm).andnot(bm.xyxy());

	FillRect<T, masked>(row, col, GSVector4i(r.x, r.y, r.z, br.y), color, mask);
	FillRect<T, masked>(row, col, GSVector4i(r.x, br.w, r.z, r.w), color, mask);

	if(r.x < br.x || br.z < r.z)
	{
		FillRect<T, masked>(row, col, GSVector4i(r.x, br.y, br.x, br.w), color, mask);
		FillRect<T, masked>(row, col, GSVector4i(br.z, br.y, r.z, br.w), color, mask);
	}

	FillBlock<T, masked>(row, col, br, color, mask);
}

template<class T, bool masked> 
void GSDrawScanline::FillRect(const GSVector4i* row, int* col, const GSVector4i& r, const GSVector4i& c, const GSVector4i& m)
{
	if(r.x >= r.z) return;

	for(int y = r.y; y < r.w; y++)
	{
		DWORD base = row[y].x;

		for(int x = r.x; x < r.z; x++)
		{
			T* p = &((T*)m_env.vm)[base + col[x]];

			*p = (T)(!masked ? c.u32[0] : (c.u32[0] | (*p & m.u32[0])));
		}
	}
}

template<class T, bool masked> 
void GSDrawScanline::FillBlock(const GSVector4i* row, int* col, const GSVector4i& r, const GSVector4i& c, const GSVector4i& m)
{
	if(r.x >= r.z) return;

	for(int y = r.y; y < r.w; y += 8)
	{
		DWORD base = row[y].x;

		for(int x = r.x; x < r.z; x += 8 * 4 / sizeof(T))
		{
			GSVector4i* p = (GSVector4i*)&((T*)m_env.vm)[base + col[x]];

			for(int i = 0; i < 16; i += 4)
			{
				p[i + 0] = !masked ? c : (c | (p[i + 0] & m));
				p[i + 1] = !masked ? c : (c | (p[i + 1] & m));
				p[i + 2] = !masked ? c : (c | (p[i + 2] & m));
				p[i + 3] = !masked ? c : (c | (p[i + 3] & m));
			}
		}
	}
}

GSVector4i GSDrawScanline::Wrap(const GSVector4i& t)
{
	GSVector4i clamp = t.sat_i16(m_env.t.min, m_env.t.max);
	GSVector4i repeat = (t & m_env.t.min) | m_env.t.max;

	return clamp.blend8(repeat, m_env.t.mask);
}

void GSDrawScanline::SampleTexture(DWORD ztst, DWORD fst, DWORD ltf, DWORD tlu, const GSVector4i& test, const GSVector4& s, const GSVector4& t, const GSVector4& q, GSVector4i* c)
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

		if(tlu)
		{
			c00 = addr00.gather32_32((const BYTE*)tex, clut);
		}
		else
		{
			c00 = addr00.gather32_32((const DWORD*)tex);
		}

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

bool GSDrawScanline::TestZ(DWORD zpsm, DWORD ztst, const GSVector4i& zs, const GSVector4i& zd, GSVector4i& test)
{
	if(ztst > 1)
	{
		GSVector4i o = GSVector4i::x80000000(zs);

		GSVector4i zso = zs - o;
		GSVector4i zdo;

		switch(zpsm)
		{
		case 0: zdo = zd - o; break;
		case 1: zdo = (zd & GSVector4i::x00ffffff(zs)) - o; break;
		case 2: zdo = (zd & GSVector4i::x0000ffff(zs)) - o; break;
		}

		switch(ztst)
		{
		case ZTST_GEQUAL: test |= zso < zdo; break;
		case ZTST_GREATER: test |= zso <= zdo; break;
		default: __assume(0);
		}

		if(test.alltrue())
		{
			return false;
		}
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
		case ATST_NEVER: t = GSVector4i::xffffffff(); break;
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
			fm |= t & GSVector4i::xff000000();
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
	if(date)
	{
		switch(fpsm)
		{
		case 0:
			test |= (d ^ m_env.datm).sra32(31);
			if(test.alltrue()) return false;
		case 1:
			break;
		case 2:
			test |= ((d << 16) ^ m_env.datm).sra32(31);
			if(test.alltrue()) return false;
		case 3:
			break;
		default:
			__assume(0);
		}
	}

	return true;
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
	case 1:
		c = addr.gather64_32<0>(vm32);
		break;
	case 2:
		c = addr.gather64_32<0>(vm16);
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
	case 1: 
		z = addr.gather64_32<2>(vm32);
		break;
	case 2:
		z = addr.gather64_32<2>(vm16);
		break;
	case 3:
		z = GSVector4i::zero();
		break;
	}

	return z;
}

void GSDrawScanline::WriteFrameX(int fpsm, int rfb, GSVector4i* c, const GSVector4i& fd, const GSVector4i& fm, const GSVector4i& fza, int fzm)
{
	DWORD* RESTRICT vm32 = (DWORD*)m_env.vm;
	WORD* RESTRICT vm16 = (WORD*)m_env.vm;

	c[0] &= m_env.colclamp;
	c[1] &= m_env.colclamp;

	GSVector4i fs = c[0].upl16(c[1]).pu16(c[0].uph16(c[1]));

	if(fpsm != 1)
	{
		fs |= m_env.fba;
	}

	if(fpsm == 2)
	{
		GSVector4i rb = fs & 0x00f800f8;
		GSVector4i ga = fs & 0x8000f800;

		fs = (ga >> 16) | (rb >> 9) | (ga >> 6) | (rb >> 3);
	}

	if(rfb)
	{
		fs = fs.blend(fd, fm);

		if(fpsm < 2)
		{
			if(fzm & 0x000a) GSVector4i::storel(&vm32[fza.u32[0] + 0], fs); 
			if(fzm & 0x00a0) GSVector4i::storeh(&vm32[fza.u32[0] + 4], fs); 

			return;
		}
	}

	switch(fpsm)
	{
	case 0: 
		if(fzm & 0x0002) WritePixel32(vm32, fza.u32[0] + 0, fs.extract32<0>()); 
		if(fzm & 0x0008) WritePixel32(vm32, fza.u32[0] + 1, fs.extract32<1>());
		if(fzm & 0x0020) WritePixel32(vm32, fza.u32[0] + 4, fs.extract32<2>()); 
		if(fzm & 0x0080) WritePixel32(vm32, fza.u32[0] + 5, fs.extract32<3>());
		break;
	case 1: 
		if(fzm & 0x0002) WritePixel24(vm32, fza.u32[0] + 0, fs.extract32<0>()); 
		if(fzm & 0x0008) WritePixel24(vm32, fza.u32[0] + 1, fs.extract32<1>()); 
		if(fzm & 0x0020) WritePixel24(vm32, fza.u32[0] + 4, fs.extract32<2>()); 
		if(fzm & 0x0080) WritePixel24(vm32, fza.u32[0] + 5, fs.extract32<3>()); 
		break;
	case 2: 
		if(fzm & 0x0002) WritePixel16(vm16, fza.u32[0] + 0, fs.extract16<0 * 2>()); 
		if(fzm & 0x0008) WritePixel16(vm16, fza.u32[0] + 2, fs.extract16<1 * 2>()); 
		if(fzm & 0x0020) WritePixel16(vm16, fza.u32[0] + 8, fs.extract16<2 * 2>()); 
		if(fzm & 0x0080) WritePixel16(vm16, fza.u32[0] + 10, fs.extract16<3 * 2>()); 
		break;
	}
}

void GSDrawScanline::WriteZBufX(int zpsm, int ztst, const GSVector4i& z, const GSVector4i& zd, const GSVector4i& zm, const GSVector4i& fza, int fzm)
{
	if(ztst == 0) return;

	DWORD* RESTRICT vm32 = (DWORD*)m_env.vm;
	WORD* RESTRICT vm16 = (WORD*)m_env.vm;

	GSVector4i zs = z;

	if(ztst > 1)
	{
		zs = zs.blend8(zd, zm);

		if(zpsm < 2)
		{
			if(fzm & 0x0a00) GSVector4i::storel(&vm32[fza.u32[2] + 0], zs); 
			if(fzm & 0xa000) GSVector4i::storeh(&vm32[fza.u32[2] + 4], zs); 

			return;
		}
	}

	switch(zpsm)
	{
	case 0: 
		if(fzm & 0x0200) WritePixel32(vm32, fza.u32[2] + 0, zs.extract32<0>()); 
		if(fzm & 0x0800) WritePixel32(vm32, fza.u32[2] + 1, zs.extract32<1>()); 
		if(fzm & 0x2000) WritePixel32(vm32, fza.u32[2] + 4, zs.extract32<2>()); 
		if(fzm & 0x8000) WritePixel32(vm32, fza.u32[2] + 5, zs.extract32<3>()); 
		break;
	case 1: 
		if(fzm & 0x0200) WritePixel24(vm32, fza.u32[2] + 0, zs.extract32<0>()); 
		if(fzm & 0x0800) WritePixel24(vm32, fza.u32[2] + 1, zs.extract32<1>()); 
		if(fzm & 0x2000) WritePixel24(vm32, fza.u32[2] + 4, zs.extract32<2>()); 
		if(fzm & 0x8000) WritePixel24(vm32, fza.u32[2] + 5, zs.extract32<3>()); 
		break;
	case 2: 
		if(fzm & 0x0200) WritePixel16(vm16, fza.u32[2] + 0, zs.extract16<0 * 2>()); 
		if(fzm & 0x0800) WritePixel16(vm16, fza.u32[2] + 2, zs.extract16<1 * 2>()); 
		if(fzm & 0x2000) WritePixel16(vm16, fza.u32[2] + 8, zs.extract16<2 * 2>()); 
		if(fzm & 0x8000) WritePixel16(vm16, fza.u32[2] + 10, zs.extract16<3 * 2>()); 
		break;
	}
}

//

void GSDrawScanline::Init()
{
	// w00t :P

	#define InitDS_IIP(iFPSM, iZPSM, iZTST, iIIP) \
		m_ds.f[iFPSM][iZPSM][iZTST][iIIP] = (DrawScanlinePtr)&GSDrawScanline::DrawScanline<iFPSM, iZPSM, iZTST, iIIP>; \

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

	#define InitDS_Sel(sel) \
		m_ds.SetAt(sel, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineEx<##sel##>); \

	#ifdef FAST_DRAWSCANLINE

	// bios

	InitDS_Sel(0x1fe04850); //   7.36%
	InitDS_Sel(0x1fe28870); //  24.11%
	InitDS_Sel(0x1fe38050); //  19.32%
	InitDS_Sel(0x1fe38060); //   7.28%
	InitDS_Sel(0x1fe38064); //  15.07%
	InitDS_Sel(0x1fe39050); // 104.01%
	InitDS_Sel(0x1fe39054); //   7.04%
	InitDS_Sel(0x48428050); //   7.93%
	InitDS_Sel(0x48428060); //   6.00%
	InitDS_Sel(0x48804050); //  10.05%
	InitDS_Sel(0x48804860); //  21.21%
	InitDS_Sel(0x49004050); //  10.94%
	InitDS_Sel(0x49028060); //   5.88%
	InitDS_Sel(0x4902904c); //   5.25%
	InitDS_Sel(0x49038050); //  12.30%
	InitDS_Sel(0x49039050); //  10.95%
	InitDS_Sel(0x4b02804c); //   5.60%
	InitDS_Sel(0x4c40404c); //   5.58%
	InitDS_Sel(0x4c804050); //  10.27%
	InitDS_Sel(0x4d019050); //  93.35%
	InitDS_Sel(0x4d038864); // 124.19%

	// ffx

	InitDS_Sel(0x1100404d); //  30.17%
	InitDS_Sel(0x11004055); //  34.43%
	InitDS_Sel(0x11020865); //  17.61%
	InitDS_Sel(0x1fe68075); //  21.18%
	InitDS_Sel(0x1fe68155); //  16.25%
	InitDS_Sel(0x1fe68875); //   6.49%
	InitDS_Sel(0x1fe68975); //  15.53%
	InitDS_Sel(0x1fe69075); //  11.47%
	InitDS_Sel(0x1fe78075); //  21.17%
	InitDS_Sel(0x1fe78155); //  14.43%
	InitDS_Sel(0x1fe84075); //  11.45%
	InitDS_Sel(0x1fee8875); //  42.51%
	InitDS_Sel(0x1fee8975); //   8.37%
	InitDS_Sel(0x48404965); //   5.56%
	InitDS_Sel(0x48804055); //  41.48%
	InitDS_Sel(0x48804855); //  36.61%
	InitDS_Sel(0x48804865); //  62.03%
	InitDS_Sel(0x4883004d); //  18.65%
	InitDS_Sel(0x48868865); //  25.19%
	InitDS_Sel(0x48868965); //  10.31%
	InitDS_Sel(0x4887814d); //  11.23%
	InitDS_Sel(0x48878165); //  26.34%
	InitDS_Sel(0x48879065); //  34.92%
	InitDS_Sel(0x488e8965); //   5.30%
	InitDS_Sel(0x488f89f5); //  32.85%
	InitDS_Sel(0x49004065); //  20.86%
	InitDS_Sel(0x49004865); //   5.15%
	InitDS_Sel(0x49004875); //  50.37%
	InitDS_Sel(0x49004965); //  11.36%
	InitDS_Sel(0x49068165); //  10.53%
	InitDS_Sel(0x49068865); //  37.21%
	InitDS_Sel(0x49068965); //  15.39%
	InitDS_Sel(0x4907814d); //  12.24%
	InitDS_Sel(0x49078165); //   7.42%
	InitDS_Sel(0x4c819055); //  15.71%
	InitDS_Sel(0x4c839065); //   9.33%
	InitDS_Sel(0x4c83914d); //  18.24%
	InitDS_Sel(0x4d068075); //  10.04%
	InitDS_Sel(0x4d078075); //  10.02%
	InitDS_Sel(0x54204055); //  13.26%
	InitDS_Sel(0x55204055); //  28.55%

	// ffx-2

	InitDS_Sel(0x1fe0404e); //   6.66%
	InitDS_Sel(0x1fe30069); //  15.98%
	InitDS_Sel(0x4880404d); //   9.16%
	InitDS_Sel(0x4880484d); //   7.76%
	InitDS_Sel(0x4881814d); //  28.01%
	InitDS_Sel(0x4881884d); //   5.59%
	InitDS_Sel(0x4881904d); //  13.02%
	InitDS_Sel(0x48820965); //   6.06%
	InitDS_Sel(0x4883904d); //  18.26%
	InitDS_Sel(0x4885004d); //  13.69%
	InitDS_Sel(0x4885904d); //  13.68%
	InitDS_Sel(0x48878865); //   6.43%
	InitDS_Sel(0x49004059); //  13.20%
	InitDS_Sel(0x49004859); //  20.52%
	InitDS_Sel(0x49004869); //   8.54%
	InitDS_Sel(0x4900494d); //   5.05%
	InitDS_Sel(0x49059155); //  12.96%
	InitDS_Sel(0x49078965); //  10.10%
	InitDS_Sel(0x49079165); //   9.52%
	InitDS_Sel(0x490e8165); //  16.46%
	InitDS_Sel(0x4c850055); //  17.31%
	InitDS_Sel(0x4d05884d); //   5.74%
	InitDS_Sel(0x1fe1004d); //  15.06%
	InitDS_Sel(0x1fe5884d); //  10.50%
	InitDS_Sel(0x4881004d); //  13.37%
	InitDS_Sel(0x488781f5); //   7.04%
	InitDS_Sel(0x49004055); //  12.90%
	InitDS_Sel(0x49068065); //   5.26%
	InitDS_Sel(0x4907804d); //  39.35%
	InitDS_Sel(0x49078865); //  19.40%

	// ffxii

	InitDS_Sel(0x1fe3804c); //   5.57%
	InitDS_Sel(0x1fe3904c); //   8.74%
	InitDS_Sel(0x1fe6804c); //   8.34%
	InitDS_Sel(0x1fe68064); //   6.52%
	InitDS_Sel(0x1fe6884c); //  17.15%
	InitDS_Sel(0x1fee8864); //  56.66%
	InitDS_Sel(0x1fee8964); //  12.42%
	InitDS_Sel(0x48404064); //   5.95%
	InitDS_Sel(0x4847004c); //  19.50%
	InitDS_Sel(0x48828064); //  76.92%
	InitDS_Sel(0x48828864); //   6.81%
	InitDS_Sel(0x4883004c); //  19.63%
	InitDS_Sel(0x4883084c); //  13.67%
	InitDS_Sel(0x4883804c); //  21.19%
	InitDS_Sel(0x4883884c); //  29.32%
	InitDS_Sel(0x4883904c); //  70.94%
	InitDS_Sel(0x48868064); //   6.14%
	InitDS_Sel(0x4886884c); //  13.80%
	InitDS_Sel(0x4887084c); // 307.95%
	InitDS_Sel(0x4887804c); //   5.54%
	InitDS_Sel(0x48878064); //   5.43%
	InitDS_Sel(0x48879064); //  21.30%
	InitDS_Sel(0x488e8b64); //  17.25%
	InitDS_Sel(0x48904064); //   5.34%
	InitDS_Sel(0x48c0404c); //  10.58%
	InitDS_Sel(0x48c0484c); //  11.69%
	InitDS_Sel(0x48c7804c); //  22.92%
	InitDS_Sel(0x48c7884c); //   6.32%
	InitDS_Sel(0x4903804c); //  14.11%
	InitDS_Sel(0x4907804c); //  92.69%
	InitDS_Sel(0x49078064); //   7.59%
	InitDS_Sel(0x4907884c); //  10.83%
	InitDS_Sel(0x49278064); //  98.56%
	InitDS_Sel(0x5fe0404c); //  22.56%
	InitDS_Sel(0x5fe04064); //  31.46%
	InitDS_Sel(0x4886804c); //   5.08%
	InitDS_Sel(0x4887904c); //   6.91%
	InitDS_Sel(0x488e8864); //   6.02%

	// kingdom hearts

	InitDS_Sel(0x1fe1804c); //   7.77%
	InitDS_Sel(0x1fe3004d); //  21.62%
	InitDS_Sel(0x1fe3804d); //  17.61%
	InitDS_Sel(0x1fe3904d); //  15.93%
	InitDS_Sel(0x4840404c); //  15.07%
	InitDS_Sel(0x48478174); //  41.16%
	InitDS_Sel(0x4880404c); //   8.17%
	InitDS_Sel(0x4881004c); //  11.20%
	InitDS_Sel(0x48818054); //  14.25%
	InitDS_Sel(0x48819054); //  15.30%
	InitDS_Sel(0x48829164); //   5.41%
	InitDS_Sel(0x48830064); // 157.08%
	InitDS_Sel(0x48858054); //  11.76%
	InitDS_Sel(0x4886814c); //   6.36%
	InitDS_Sel(0x48868154); //   6.83%
	InitDS_Sel(0x4886904c); //  21.61%
	InitDS_Sel(0x488e8064); //   5.23%
	InitDS_Sel(0x488e8964); //  17.77%
	InitDS_Sel(0x4900404c); //   7.52%
	InitDS_Sel(0x49004054); //  13.01%
	InitDS_Sel(0x49068874); //  18.89%
	InitDS_Sel(0x4907814c); //  13.77%
	InitDS_Sel(0x490e8974); //  48.61%
	InitDS_Sel(0x48830054); //  22.39%
	InitDS_Sel(0x48830874); //  10.81%
	InitDS_Sel(0x49078174); //  10.79%

	// kingdom hearts 2

	InitDS_Sel(0x1fee9164); //   5.14%
	InitDS_Sel(0x48804060); //  14.51%
	InitDS_Sel(0x48810054); //  31.18%
	InitDS_Sel(0x4883814c); //   5.12%
	InitDS_Sel(0x48858854); //   9.46%
	InitDS_Sel(0x4887884d); //   9.38%
	InitDS_Sel(0x48878854); //  53.51%
	InitDS_Sel(0x488a8964); //  14.66%
	InitDS_Sel(0x49004874); //  12.01%
	InitDS_Sel(0x49078054); //  28.91%
	InitDS_Sel(0x490e8964); //  10.43%
	InitDS_Sel(0x4c83004d); //  61.82%
	InitDS_Sel(0x4d03004d); //  21.62%

	// persona 3

	InitDS_Sel(0x484e8068); //  26.83%
	InitDS_Sel(0x48804868); //  21.74%
	InitDS_Sel(0x4881884c); //  19.25%
	InitDS_Sel(0x48858068); //  27.71%
	InitDS_Sel(0x48858868); //  16.62%
	InitDS_Sel(0x48878b68); //  12.58%
	InitDS_Sel(0x4907904c); //  12.22%
	InitDS_Sel(0x490e8068); //   9.27%
	InitDS_Sel(0x4a43004c); //  17.50%
	InitDS_Sel(0x4b07904c); //  55.71%
	InitDS_Sel(0x4b07934c); //  10.74%
	InitDS_Sel(0x4d47804c); //  99.37%
	InitDS_Sel(0x4d47834c); //  57.10%
	InitDS_Sel(0x4d478b4c); //  17.94%
	InitDS_Sel(0x4d47934c); //  26.77%

	// persona 4

	InitDS_Sel(0x1fe04858); //   5.96%
	InitDS_Sel(0x4840484c); //  13.30%
	InitDS_Sel(0x48804058); //  99.10%
	InitDS_Sel(0x48804068); //  20.25%
	InitDS_Sel(0x48804b68); //   5.22%
	InitDS_Sel(0x4881834c); //  19.38%
	InitDS_Sel(0x4881934c); //  17.88%
	InitDS_Sel(0x48828868); //  10.94%
	InitDS_Sel(0x48828b68); //   8.89%
	InitDS_Sel(0x48859368); //   7.38%
	InitDS_Sel(0x48868f68); //  26.82%
	InitDS_Sel(0x48869068); //  17.27%
	InitDS_Sel(0x4887884c); //   5.23%
	InitDS_Sel(0x48879068); //  30.31%
	InitDS_Sel(0x48879168); //   6.16%
	InitDS_Sel(0x48879368); //   5.66%
	InitDS_Sel(0x4900484c); //  10.85%
	InitDS_Sel(0x4903004c); //  24.17%
	InitDS_Sel(0x49068068); //  10.00%
	InitDS_Sel(0x49068868); //  16.15%
	InitDS_Sel(0x49078068); //  56.68%
	InitDS_Sel(0x49079068); //  55.22%
	InitDS_Sel(0x490e8868); //  19.40%
	InitDS_Sel(0x4a47804c); //  67.17%
	InitDS_Sel(0x4a47904c); //  24.27%
	InitDS_Sel(0x4a80404c); //  47.00%
	InitDS_Sel(0x4a87804c); //  26.55%
	InitDS_Sel(0x4a878068); //  30.30%
	InitDS_Sel(0x4a878868); //  22.16%
	InitDS_Sel(0x4a879068); //  11.09%
	InitDS_Sel(0x4b00404c); //  25.00%
	InitDS_Sel(0x4b004868); //  10.46%
	InitDS_Sel(0x4b07804c); //  23.27%
	InitDS_Sel(0x4b07884c); //  23.97%
	InitDS_Sel(0x4d0e8868); //   6.84%
	InitDS_Sel(0x5fe04858); //  19.04%

	// sfex3

	InitDS_Sel(0x1fe1004e); //   6.65%
	InitDS_Sel(0x1fe3004e); //  12.59%
	InitDS_Sel(0x1fe6b068); //  15.24%
	InitDS_Sel(0x1fe6b868); //   5.18%
	InitDS_Sel(0x41268068); //   7.42%
	InitDS_Sel(0x41269068); //   7.07%
	InitDS_Sel(0x48859058); //  10.06%
	InitDS_Sel(0x4886b068); //  15.43%
	InitDS_Sel(0x4886b868); //  32.83%
	InitDS_Sel(0x4886b968); //   5.69%
	InitDS_Sel(0x49079078); //   5.22%
	InitDS_Sel(0x4c804058); //   7.25%

	// gt4

	InitDS_Sel(0x1fe1904c); //  16.10%
	InitDS_Sel(0x1fe1904d); //   7.80%
	InitDS_Sel(0x1fe5904d); //   9.49%
	InitDS_Sel(0x1fe7804d); //  11.90%
	InitDS_Sel(0x1fe7904d); //  11.44%
	InitDS_Sel(0x1fee8064); //   5.26%
	InitDS_Sel(0x1fee9064); //  13.01%
	InitDS_Sel(0x488181d4); //  18.94%
	InitDS_Sel(0x488191d4); //  20.50%
	InitDS_Sel(0x4887804d); //   7.01%
	InitDS_Sel(0x4887904d); //  10.49%
	InitDS_Sel(0x48884064); //  48.69%
	InitDS_Sel(0x48884864); //  69.60%
	InitDS_Sel(0x488e804c); //   5.53%
	InitDS_Sel(0x488e8065); //  30.74%
	InitDS_Sel(0x488e8865); //  39.25%
	InitDS_Sel(0x488e8f64); //   7.44%
	InitDS_Sel(0x488e9764); //  34.11%
	InitDS_Sel(0x4b03804c); //  27.01%
	InitDS_Sel(0x4b1a8864); //   8.64%
	InitDS_Sel(0x4c81904c); //  27.70%
	InitDS_Sel(0x4c81904d); //  10.41%
	InitDS_Sel(0x4c83804d); //  14.36%
	InitDS_Sel(0x4c83904d); //  13.05%
	InitDS_Sel(0x4d05804c); //  22.18%
	InitDS_Sel(0x5520404c); //  19.08%
	InitDS_Sel(0x5fe1904c); //   7.73%
	InitDS_Sel(0x5fe3804c); //   7.28%
	InitDS_Sel(0x5fe5804c); //   9.38%
	InitDS_Sel(0x5fee8864); //  11.11%
	InitDS_Sel(0x1fe04054); //   5.18%
	InitDS_Sel(0x5fe2884e); //   5.23%

	// katamary damacy

	InitDS_Sel(0x488181cc); //   7.16%
	InitDS_Sel(0x48858064); //  50.67%
	InitDS_Sel(0x48859064); //  16.13%
	InitDS_Sel(0x488e89e4); //   9.52%
	InitDS_Sel(0x488e91d4); //  17.20%
	InitDS_Sel(0x48904054); //   9.73%

	// grandia 3

	InitDS_Sel(0x41268060); //  21.76%
	InitDS_Sel(0x48869060); //  12.07%
	InitDS_Sel(0x48869360); //   6.46%
	InitDS_Sel(0x48869760); //  32.54%
	InitDS_Sel(0x48878060); //  29.61%
	InitDS_Sel(0x48879760); //  26.66%
	InitDS_Sel(0x488a8060); //  16.62%
	InitDS_Sel(0x488a8860); //  19.44%
	InitDS_Sel(0x488e8360); //  21.74%
	InitDS_Sel(0x488e8860); //  49.04%
	InitDS_Sel(0x488e8b60); //  42.75%
	InitDS_Sel(0x488e8f60); //  14.06%
	InitDS_Sel(0x488e9060); //   9.02%
	InitDS_Sel(0x488e9360); //   5.11%
	InitDS_Sel(0x488f8860); //  44.99%
	InitDS_Sel(0x4906804c); //  41.10%
	InitDS_Sel(0x49078060); //   7.74%
	InitDS_Sel(0x49078860); //  22.69%
	InitDS_Sel(0x4c81804c); //  57.62%
	InitDS_Sel(0x4c839060); //  67.22%
	InitDS_Sel(0x50368060); //  10.65%
	InitDS_Sel(0x488e8060); //   5.70%

	// rumble roses

	InitDS_Sel(0x1fe78064); //  26.77%
	InitDS_Sel(0x1fe79064); //   9.97%
	InitDS_Sel(0x48838164); //  13.17%
	InitDS_Sel(0x4887b864); //  35.77%
	InitDS_Sel(0x4c830064); //  39.43%
	InitDS_Sel(0x4c8e8864); //   5.79%

	// dmc

	InitDS_Sel(0x1fe78158); //   3.30%
	InitDS_Sel(0x1fea8968); //   2.26%
	InitDS_Sel(0x1fee8068); //   0.45%
	InitDS_Sel(0x1fee8168); //   0.00%
	InitDS_Sel(0x4423904c); //   8.75%
	InitDS_Sel(0x4427904c); //  28.36%
	InitDS_Sel(0x4520404c); //   8.75%
	InitDS_Sel(0x45204078); //   2.93%
	InitDS_Sel(0x48830068); //  10.63%
	InitDS_Sel(0x48859078); //   4.66%
	InitDS_Sel(0x488e8168); //   0.01%
	InitDS_Sel(0x49068168); //   0.21%
	InitDS_Sel(0x4c43804c); //  19.18%
	InitDS_Sel(0x4c87914c); //  45.13%
	InitDS_Sel(0x4c8a8968); //   0.33%
	InitDS_Sel(0x4d07904c); //   1.68%
	InitDS_Sel(0x54204078); //   3.00%
	InitDS_Sel(0x5fe30868); //   0.44%

	// xenosaga 2

	InitDS_Sel(0x1fe68864); //  14.31%
	InitDS_Sel(0x1fee804c); //  13.85%
	InitDS_Sel(0x48804074); //  14.06%
	InitDS_Sel(0x48878164); //  75.85%
	InitDS_Sel(0x49078164); //  54.70%
	InitDS_Sel(0x49079064); //  28.67%
	InitDS_Sel(0x4c404054); //  13.72%
	InitDS_Sel(0x4c804054); //  15.00%
	InitDS_Sel(0x4c839054); //  21.34%
	InitDS_Sel(0x4d004054); //  13.62%
	InitDS_Sel(0x4d069064); //   5.22%
	InitDS_Sel(0x51229064); //   7.80%

	// nfs mw

	InitDS_Sel(0x1fe68068); //  13.79%
	InitDS_Sel(0x1fe6806a); //   9.59%
	InitDS_Sel(0x1fe68868); //  92.05%
	InitDS_Sel(0x1fe6886a); //   6.20%
	InitDS_Sel(0x1fe68964); //  29.40%
	InitDS_Sel(0x1fe78068); //  22.75%
	InitDS_Sel(0x4805904c); //  13.74%
	InitDS_Sel(0x4927904c); //  19.12%
	InitDS_Sel(0x4b004064); //  14.63%
	InitDS_Sel(0x4b004068); //  23.06%
	InitDS_Sel(0x4b004864); //  20.42%
	InitDS_Sel(0x4b028064); //  15.98%
	InitDS_Sel(0x4b028068); //  22.27%
	InitDS_Sel(0x4b028864); //  19.58%
	InitDS_Sel(0x4b028868); //  22.32%
	InitDS_Sel(0x4b038864); //  15.46%
	InitDS_Sel(0x4c83804e); //   5.42%
	InitDS_Sel(0x5420404c); //   9.61%
	InitDS_Sel(0x5fe19064); //  21.51%

	// berserk

	// castlevania

	InitDS_Sel(0x1fe1004c); //  11.56%
	InitDS_Sel(0x1fe5904c); //  14.91%
	InitDS_Sel(0x1fe78868); //  25.25%
	InitDS_Sel(0x4881004e); //  21.92%
	InitDS_Sel(0x48878868); // 112.75%
	InitDS_Sel(0x488c8968); //   5.71%
	InitDS_Sel(0x4d00407a); //  22.67%

	// okami

	InitDS_Sel(0x1fe18058); //  14.39%
	InitDS_Sel(0x45218058); //  35.05%
	InitDS_Sel(0x4881804c); //  15.42%
	InitDS_Sel(0x48839158); //  25.68%
	InitDS_Sel(0x48868168); //  33.36%
	InitDS_Sel(0x48878058); //  39.16%
	InitDS_Sel(0x48878158); //   5.61%
	InitDS_Sel(0x48878168); // 291.53%
	InitDS_Sel(0x48879058); //   9.03%
	InitDS_Sel(0x488e8968); //  48.19%
	InitDS_Sel(0x49078168); //  30.95%
	InitDS_Sel(0x4a83804c); //  21.71%
	InitDS_Sel(0x4c43904c); //  98.78%
	InitDS_Sel(0x5127904c); //   5.66%
	InitDS_Sel(0x5fe59068); //  83.06%

	// bully

	InitDS_Sel(0x110e8864); //  60.50%
	InitDS_Sel(0x110e8964); //  59.73%
	InitDS_Sel(0x1fe04077); //  11.64%
	InitDS_Sel(0x1fe04864); //   7.71%
	InitDS_Sel(0x48804864); //  11.88%
	InitDS_Sel(0x48878b4c); //  11.57%
	InitDS_Sel(0x4901004c); //  20.73%
	InitDS_Sel(0x4d068364); //  16.32%
	InitDS_Sel(0x4d068864); //  22.99%
	InitDS_Sel(0x4d068b64); //  18.38%
	InitDS_Sel(0x4d07804c); //  12.98%
	InitDS_Sel(0x5480404d); //  15.97%
	InitDS_Sel(0x5501904e); //  27.90%
	InitDS_Sel(0x4c20404c); //  12.05%
	InitDS_Sel(0x4d068064); //   9.92%

	// culdcept

	InitDS_Sel(0x1fe04866); //  12.73%
	InitDS_Sel(0x1fe191e6); //   6.19%
	InitDS_Sel(0x1fe2a1e6); //  13.34%
	InitDS_Sel(0x1fe2a9e6); //  19.49%
	InitDS_Sel(0x1fe391e6); //  26.82%
	InitDS_Sel(0x1fe3a1e6); //  14.27%
	InitDS_Sel(0x1fe59066); //  19.01%
	InitDS_Sel(0x1fe991e6); //  19.86%
	InitDS_Sel(0x488089e6); //   5.01%
	InitDS_Sel(0x488181e6); //   5.99%
	InitDS_Sel(0x488291e6); //   7.19%
	InitDS_Sel(0x4d02a1e6); //  10.61%

	// suikoden 5

	InitDS_Sel(0x00428868); //   9.89%
	InitDS_Sel(0x40428868); //  18.61%
	InitDS_Sel(0x4846804c); //  26.43%
	InitDS_Sel(0x48819368); //  22.82%
	InitDS_Sel(0x48828368); //   5.10%
	InitDS_Sel(0x48829368); //  14.96%
	InitDS_Sel(0x48859068); //  21.58%
	InitDS_Sel(0x488a8b68); //  10.48%
	InitDS_Sel(0x49028868); //  14.05%
	InitDS_Sel(0x4d068868); //  32.13%

	// dq8

	InitDS_Sel(0x1103b04c); //   6.34%
	InitDS_Sel(0x1fe0484c); //  11.25%
	InitDS_Sel(0x1fee8164); //  14.23%
	InitDS_Sel(0x4883914c); //  11.56%
	InitDS_Sel(0x48859054); //   9.41%
	InitDS_Sel(0x488e8164); //  17.36%
	InitDS_Sel(0x48c3804c); //  41.24%
	InitDS_Sel(0x490a8164); //  12.94%
	InitDS_Sel(0x490a8964); //   5.17%
	InitDS_Sel(0x490e904c); //  28.19%
	InitDS_Sel(0x490f904c); //  21.14%
	InitDS_Sel(0x4c83914c); //   8.19%
	InitDS_Sel(0x5fe3904e); //   7.26%

	// resident evil 4

	InitDS_Sel(0x1fe04057); //   8.21%
	InitDS_Sel(0x1fe18064); //  13.95%
	InitDS_Sel(0x4887814c); //  15.55%
	InitDS_Sel(0x4903904c); //  20.28%
	InitDS_Sel(0x4b068064); //   6.98%
	InitDS_Sel(0x4d07814c); //  10.56%
	InitDS_Sel(0x5483904c); //   5.85%
	InitDS_Sel(0x5fe68864); //   6.81%

	// tomoyo after 

	InitDS_Sel(0x1fe04058); //   5.59%
	InitDS_Sel(0x1fe38059); //  24.74%
	InitDS_Sel(0x1fe39059); //  24.19%
	InitDS_Sel(0x48478068); //   9.16%
	InitDS_Sel(0x48818068); //  34.42%
	InitDS_Sel(0x49004068); //  19.67%
	InitDS_Sel(0x49058068); //  17.71%
	InitDS_Sel(0x4a858068); //  17.72%

	// .hack redemption

	InitDS_Sel(0x1fe1914c); //   5.09%
	InitDS_Sel(0x4123004c); //  19.45%
	InitDS_Sel(0x48868364); //  14.72%
	InitDS_Sel(0x48469064); //  12.96%
	InitDS_Sel(0x48869364); //  21.30%
	InitDS_Sel(0x488e9064); //  16.46%
	InitDS_Sel(0x488e9364); //   5.74%
	InitDS_Sel(0x49004864); //   7.13%
	InitDS_Sel(0x4c41804c); //  14.43%
	InitDS_Sel(0x4d00404c); //   9.80%
	InitDS_Sel(0x5fe1004c); //  12.16%

	// wild arms 5

	InitDS_Sel(0x4845804c); //  14.90%
	InitDS_Sel(0x4845904c); //  13.24%
	InitDS_Sel(0x48804854); //  13.40%
	InitDS_Sel(0x4885884c); //  11.46%
	InitDS_Sel(0x488e8764); //  21.61%
	InitDS_Sel(0x48c68864); //   7.86%
	InitDS_Sel(0x5fe39054); //  21.06%

	// shadow of the colossus

	InitDS_Sel(0x48868b64); //  29.96%
	InitDS_Sel(0x48938064); //  52.63%
	InitDS_Sel(0x48939064); //  19.79%
	InitDS_Sel(0x49004064); //  92.76%
	InitDS_Sel(0x490e8864); // 134.30%
	InitDS_Sel(0x4d004064); // 254.80%
	InitDS_Sel(0x1fe0404c); //   5.78%
	InitDS_Sel(0x48878364); //  81.37%
	InitDS_Sel(0x48879364); //  29.65%
	InitDS_Sel(0x488f8864); //   8.60%
	InitDS_Sel(0x4c030064); //  27.75%

	// tales of redemption

	InitDS_Sel(0x48404054); //  10.38%
	InitDS_Sel(0x48478054); //  13.88%
	InitDS_Sel(0x48878b64); //  12.96%
	InitDS_Sel(0x488b9054); //   7.70%
	InitDS_Sel(0x4c838064); //  13.79%
	InitDS_Sel(0x4c838854); //  13.95%

	// digital devil saga

	InitDS_Sel(0x1fe39070); //   6.52%
	InitDS_Sel(0x40204050); //   5.58%
	InitDS_Sel(0x48404050); //   5.81%
	InitDS_Sel(0x48868870); //   6.60%
	InitDS_Sel(0x48878150); //   9.22%
	InitDS_Sel(0x48879060); //  11.85%
	InitDS_Sel(0x48879360); //   8.67%
	InitDS_Sel(0x48884870); //  21.76%
	InitDS_Sel(0x488e8870); //   8.94%
	InitDS_Sel(0x4890404c); //   5.89%
	InitDS_Sel(0x48904070); //  34.36%
	InitDS_Sel(0x49078360); //   8.78%
	InitDS_Sel(0x49079360); //   5.20%
	InitDS_Sel(0x490e8860); //   5.57%
	InitDS_Sel(0x4a878060); //   7.39%

	// dbzbt2

	InitDS_Sel(0x48868164); //  15.63%
	InitDS_Sel(0x48878074); //   7.87%
	InitDS_Sel(0x4906884c); //  19.06%
	InitDS_Sel(0x49079054); //   5.73%
	InitDS_Sel(0x543081e4); //  11.49%
	InitDS_Sel(0x4c904064); //  10.28%

	// dbzbt3

	InitDS_Sel(0x48478064); //  23.95%
	InitDS_Sel(0x48804054); //  21.55%
	InitDS_Sel(0x4881904c); //  15.10%
	InitDS_Sel(0x488391cc); //  12.58%
	InitDS_Sel(0x4885904c); //  35.18%
	InitDS_Sel(0x4885904e); //  15.45%
	InitDS_Sel(0x48859074); //  14.82%
	InitDS_Sel(0x48868864); //  77.06%
	InitDS_Sel(0x48868964); //  21.51%
	InitDS_Sel(0x48878054); //  22.08%
	InitDS_Sel(0x48879054); //  66.77%
	InitDS_Sel(0x489081e4); //   8.28%
	InitDS_Sel(0x48968864); //  13.81%
	InitDS_Sel(0x4905904c); //  26.55%
	InitDS_Sel(0x49068064); //  19.22%
	InitDS_Sel(0x49068864); //  60.78%
	InitDS_Sel(0x49078864); //  12.20%
	InitDS_Sel(0x4910404c); //  21.21%
	InitDS_Sel(0x4917804c); //   5.99%
	InitDS_Sel(0x4a40404c); //   9.32%
	InitDS_Sel(0x4a83004c); //  18.31%
	InitDS_Sel(0x4c45904e); //  31.06%
	InitDS_Sel(0x4c469064); //  17.98%
	InitDS_Sel(0x4c80404c); //  10.48%
	InitDS_Sel(0x4c80404e); //  14.32%
	InitDS_Sel(0x4c83004c); //  17.82%
	InitDS_Sel(0x4c8391cc); //  25.17%
	InitDS_Sel(0x4c869064); //  19.54%
	InitDS_Sel(0x4d03004c); //  33.10%
	InitDS_Sel(0x5fe1904e); //  18.47%

	// disgaea 2

	InitDS_Sel(0x1fe04064); //   6.74%
	InitDS_Sel(0x1fe69074); //   8.31%
	InitDS_Sel(0x48804064); //   9.18%
	InitDS_Sel(0x48820864); //  15.24%
	InitDS_Sel(0x48869064); //   9.41%
	InitDS_Sel(0x48869164); //   5.06%
	InitDS_Sel(0x48878964); //  42.60%
	InitDS_Sel(0x48879164); //   6.60%

	// gradius 5

	InitDS_Sel(0x1fee8868); //  40.55%
	InitDS_Sel(0x48868968); //   5.40%
	InitDS_Sel(0x48878968); //   7.07%
	InitDS_Sel(0x5fe04058); //   6.57%
	InitDS_Sel(0x5fe3814c); //  25.91%
	InitDS_Sel(0x5fe68068); //  30.24%
	InitDS_Sel(0x5fe68968); //   9.87%
	InitDS_Sel(0x5fee8868); //  42.47%
	InitDS_Sel(0x5fee8968); //  11.96%
	InitDS_Sel(0x5ffe8868); //   5.85%

	// tales of abyss

	InitDS_Sel(0x1fe39368); //   7.18%
	InitDS_Sel(0x4121004c); //  26.89%
	InitDS_Sel(0x4121084c); //  14.62%
	InitDS_Sel(0x4880484c); //  19.77%
	InitDS_Sel(0x4885804c); //  14.20%
	InitDS_Sel(0x48868068); //  14.98%
	InitDS_Sel(0x48868868); //  21.78%
	InitDS_Sel(0x4886934c); //   6.07%
	InitDS_Sel(0x4887834c); //  13.72%
	InitDS_Sel(0x4887934c); //   6.34%
	InitDS_Sel(0x488c8868); //   6.70%
	InitDS_Sel(0x488c8b68); //  11.36%
	InitDS_Sel(0x488e8068); //  35.41%
	InitDS_Sel(0x488e8368); //  13.38%
	InitDS_Sel(0x488e8868); //  76.29%
	InitDS_Sel(0x488e8b68); //   5.87%
	InitDS_Sel(0x48cf89e8); //  55.96%
	InitDS_Sel(0x4903834c); //  19.08%
	InitDS_Sel(0x490c8b68); //   7.74%
	InitDS_Sel(0x490e8b68); //   7.26%
	InitDS_Sel(0x490f89e8); //  58.70%
	InitDS_Sel(0x4a83904c); //  26.75%
	InitDS_Sel(0x4d03914c); //  16.34%
	InitDS_Sel(0x5fe3904c); //   7.09%
	InitDS_Sel(0x5fe59078); //  21.02%

	// Gundam Seed Destiny OMNI VS ZAFT II PLUS 

	InitDS_Sel(0x1fe19075); //  18.00%
	InitDS_Sel(0x1fee8b75); //  20.52%
	InitDS_Sel(0x48818075); //  31.08%
	InitDS_Sel(0x48819075); //  21.08%
	InitDS_Sel(0x4885804d); //  17.79%
	InitDS_Sel(0x48868875); //   8.89%
	InitDS_Sel(0x48868b75); //  16.39%
	InitDS_Sel(0x48878375); //  13.91%
	InitDS_Sel(0x48878875); //  11.87%
	InitDS_Sel(0x48878b75); //  25.83%
	InitDS_Sel(0x488e8075); //  18.58%
	InitDS_Sel(0x488e8375); //  30.75%
	InitDS_Sel(0x488e8875); //  51.11%
	InitDS_Sel(0x488e8b75); //  31.87%
	InitDS_Sel(0x488f8075); //  16.32%
	InitDS_Sel(0x488f8875); //  33.68%
	InitDS_Sel(0x488f8b75); //  15.37%
	InitDS_Sel(0x49068075); //  33.13%
	InitDS_Sel(0x4906884d); //   6.62%
	InitDS_Sel(0x490e8375); //  30.00%
	InitDS_Sel(0x490e8875); //  47.29%
	InitDS_Sel(0x490e8b75); //  82.46%
	InitDS_Sel(0x490f8075); //  25.85%
	InitDS_Sel(0x490f8b75); //  31.42%
	InitDS_Sel(0x490f9075); //  15.09%

	// nba 2k8

	InitDS_Sel(0x1fe04856); //  14.62%
	InitDS_Sel(0x1fe28956); //  27.38%
	InitDS_Sel(0x1fe38966); //  27.24%
	InitDS_Sel(0x1fe39156); //  24.67%
	InitDS_Sel(0x1fe79056); //  25.00%
	InitDS_Sel(0x4883804e); //  25.73%
	InitDS_Sel(0x48838166); //   5.95%
	InitDS_Sel(0x48868166); //  13.72%
	InitDS_Sel(0x48868866); //  27.19%
	InitDS_Sel(0x48868966); //  16.70%
	InitDS_Sel(0x48879066); //   8.64%
	InitDS_Sel(0x48879166); //  20.80%
	InitDS_Sel(0x49028966); //   6.49%
	InitDS_Sel(0x5fe48866); //  10.30%
	InitDS_Sel(0x5fe68866); //  15.19%
	InitDS_Sel(0x5fe79066); //  28.78%
	InitDS_Sel(0x1fe68866); //   6.12%
	InitDS_Sel(0x48868066); //   8.67%
	InitDS_Sel(0x49068066); //  22.19%
	InitDS_Sel(0x49068866); //   6.66%
	InitDS_Sel(0x49068966); //  11.88%
	InitDS_Sel(0x49068976); //  12.74%

	// onimusha 3

	InitDS_Sel(0x1fe18068); //   5.81%
	InitDS_Sel(0x1fe3904e); //   5.28%
	InitDS_Sel(0x1fee0868); //  42.87%
	InitDS_Sel(0x1fee8968); //  10.05%
	InitDS_Sel(0x48839168); //   7.30%
	InitDS_Sel(0x48878068); //  25.82%
	InitDS_Sel(0x48878368); //  11.30%
	InitDS_Sel(0x48c28368); //   6.98%
	InitDS_Sel(0x4903884c); //  38.57%
	InitDS_Sel(0x4c878168); //   9.48%
	InitDS_Sel(0x4d004068); //  31.77%
	InitDS_Sel(0x4d03804c); //   8.38%
	InitDS_Sel(0x4d05884c); //   8.39%
	InitDS_Sel(0x5125904c); //   5.30%
	InitDS_Sel(0x5425904c); //   6.52%
	InitDS_Sel(0x5fe04078); //  32.17%
	InitDS_Sel(0x5fe5904c); //   5.61%
	InitDS_Sel(0x5fe78368); //   6.84%
	InitDS_Sel(0x5fe7904c); //   8.82%
	InitDS_Sel(0x5fe79368); //  11.11%
	InitDS_Sel(0x49204068); //   6.26%
	InitDS_Sel(0x4c804068); //   5.45%

	// resident evil code veronica

	InitDS_Sel(0x1fe39068); //  19.35%
	InitDS_Sel(0x1fe78168); //  26.62%
	InitDS_Sel(0x4c819058); //  23.05%

	// armored core 3

	InitDS_Sel(0x1fe04074); //   9.18%
	InitDS_Sel(0x1fe84074); //   5.53%
	InitDS_Sel(0x1fee0874); //  48.94%
	InitDS_Sel(0x48404854); //   5.10%
	InitDS_Sel(0x48850054); //   9.46%
	InitDS_Sel(0x48878874); //  12.18%
	InitDS_Sel(0x488791d4); //   5.25%
	InitDS_Sel(0x488e8074); //  18.21%
	InitDS_Sel(0x49059054); //   9.87%
	InitDS_Sel(0x490e8074); //  53.97%
	InitDS_Sel(0x4c4e8074); //   7.40%
	InitDS_Sel(0x4c4e8874); //   9.56%
	InitDS_Sel(0x4d0e8074); //   6.18%

	// aerial planet

	InitDS_Sel(0x4820404c); //  14.64%
	InitDS_Sel(0x48478164); //  10.58%
	InitDS_Sel(0x4847914c); //   6.89%
	InitDS_Sel(0x4886894c); //  21.89%
	InitDS_Sel(0x4887914c); //   7.83%
	InitDS_Sel(0x488e814c); //  15.48%
	InitDS_Sel(0x488e894c); //  20.22%
	InitDS_Sel(0x488f8164); //  16.26%
	InitDS_Sel(0x4c868074); //  41.43%
	InitDS_Sel(0x4c868874); //   8.71%
	InitDS_Sel(0x4c868934); //  17.36%
	InitDS_Sel(0x4c8e8074); //  12.51%
	InitDS_Sel(0x4c8e8874); //  13.21%
	InitDS_Sel(0x4cc0404c); //  13.77%
	InitDS_Sel(0x4d068074); //   8.22%

	// one piece grand battle 3

	InitDS_Sel(0x1fe1904e); //  10.06%
	InitDS_Sel(0x48839054); //  22.82%
	InitDS_Sel(0x48868174); //   5.85%
	InitDS_Sel(0x49068174); //   5.54%
	InitDS_Sel(0x49068964); //   7.75%
	InitDS_Sel(0x49068974); //   5.18%
	InitDS_Sel(0x49078974); //  12.39%
	InitDS_Sel(0x49079174); //   7.20%
	InitDS_Sel(0x4ac0404c); //   9.04%
	InitDS_Sel(0x4c41904c); //  12.25%
	InitDS_Sel(0x4c4190cc); //   8.15%
	InitDS_Sel(0x5321914c); //   8.06%

	// one piece grand adventure

	InitDS_Sel(0x4421814c); //   7.91%
	InitDS_Sel(0x4843b04c); //  19.34%
	InitDS_Sel(0x4881984c); //  16.30%
	InitDS_Sel(0x48849164); //  16.24%
	InitDS_Sel(0x48869154); //   5.56%
	InitDS_Sel(0x48879154); //   6.81%
	InitDS_Sel(0x5fe7804c); //  15.70%

	// shadow hearts

	InitDS_Sel(0x1fe3004c); //  23.52%
	InitDS_Sel(0x4881814c); //   5.40%
	InitDS_Sel(0x4881904e); //   5.52%
	InitDS_Sel(0x48819168); //  14.09%
	InitDS_Sel(0x48830058); //  13.41%
	InitDS_Sel(0x48839058); //  13.17%
	InitDS_Sel(0x48868078); //   9.80%
	InitDS_Sel(0x48868778); //  10.54%
	InitDS_Sel(0x48868f78); //   5.33%
	InitDS_Sel(0x49004858); //   7.59%
	InitDS_Sel(0x49030058); //  44.51%
	InitDS_Sel(0x49039058); //  12.79%
	InitDS_Sel(0x4c870878); //   7.17%

	// the punisher

	InitDS_Sel(0x48420864); //   8.61%
	InitDS_Sel(0x48468864); //   8.94%
	InitDS_Sel(0x4880474c); //  26.47%
	InitDS_Sel(0x48868764); //   5.28%
	InitDS_Sel(0x48868f64); //  53.49%
	InitDS_Sel(0x4886bf64); //  16.79%
	InitDS_Sel(0x4906904c); //  12.19%
	InitDS_Sel(0x4d068f64); //  13.01%
	InitDS_Sel(0x5fe0404e); //   5.82%
	InitDS_Sel(0x5fe3974c); //  26.82%
	InitDS_Sel(0x5fe68f64); //  79.62%

	// guitar hero

	InitDS_Sel(0x1503204e); //  19.41%
	InitDS_Sel(0x1fe3906a); //  10.58%
	InitDS_Sel(0x1fe6887a); //   9.57%
	InitDS_Sel(0x48804d4e); //  18.77%
	InitDS_Sel(0x48804d7a); //   7.59%
	InitDS_Sel(0x4886804e); //  20.45%
	InitDS_Sel(0x4886854e); //   7.64%
	InitDS_Sel(0x4886887a); //  43.47%
	InitDS_Sel(0x48868d5a); //   6.72%
	InitDS_Sel(0x48868d6a); //   7.60%
	InitDS_Sel(0x48868d7a); //   8.22%
	InitDS_Sel(0x4887804e); //  22.36%
	InitDS_Sel(0x4887854e); //   8.45%
	InitDS_Sel(0x4887857a); //  26.38%
	InitDS_Sel(0x48878d7a); //  29.82%
	InitDS_Sel(0x4887904e); //  20.65%
	InitDS_Sel(0x4887917a); //  24.36%
	InitDS_Sel(0x4887954e); //  13.80%
	InitDS_Sel(0x4887957a); //  24.41%
	InitDS_Sel(0x488a917a); //  23.02%
	InitDS_Sel(0x488e887a); //  10.25%
	InitDS_Sel(0x488e8d7a); //  61.93%
	InitDS_Sel(0x4900487a); //  16.70%
	InitDS_Sel(0x4906806a); //  21.86%
	InitDS_Sel(0x4906886a); //   5.62%
	InitDS_Sel(0x4d03204e); //  64.14%
	InitDS_Sel(0x4d06a06a); //  19.34%
	InitDS_Sel(0x4d06a86a); //  11.29%
	InitDS_Sel(0x4d07806a); //   7.00%

	// ico

	InitDS_Sel(0x1fe04060); //   6.61%
	InitDS_Sel(0x1fe28060); //  15.02%
	InitDS_Sel(0x1fe380cc); //  15.50%
	InitDS_Sel(0x1fe3a04c); //  44.78%
	InitDS_Sel(0x1fe68860); //  46.02%
	InitDS_Sel(0x48859060); //  11.74%
	InitDS_Sel(0x48868060); //   7.60%
	InitDS_Sel(0x48868360); //   9.54%
	InitDS_Sel(0x48868860); //  11.90%
	InitDS_Sel(0x48868b60); //  88.70%
	InitDS_Sel(0x4893814c); //  39.42%
	InitDS_Sel(0x49004060); //   6.10%
	InitDS_Sel(0x49068860); //   7.74%
	InitDS_Sel(0x49068b60); //  15.40%
	InitDS_Sel(0x4c468b60); //  39.43%
	InitDS_Sel(0x4c478860); //  10.81%
	InitDS_Sel(0x4c83804c); //  18.81%
	InitDS_Sel(0x4c83904c); //  18.90%
	InitDS_Sel(0x4d004060); // 105.33%
	InitDS_Sel(0x4d028060); //  14.56%
	InitDS_Sel(0x4d03904c); //  19.21%
	InitDS_Sel(0x4d068360); //  16.48%
	InitDS_Sel(0x4d068860); //  18.17%
	InitDS_Sel(0x4d068b60); // 219.11%
	InitDS_Sel(0x4d078060); //   5.63%
	InitDS_Sel(0x4d078360); //   8.35%

	// kuon

	InitDS_Sel(0x1fee0865); //  12.82%
	InitDS_Sel(0x4847004d); //  22.09%
	InitDS_Sel(0x4847084d); //   6.13%
	InitDS_Sel(0x48860865); //  19.79%
	InitDS_Sel(0x48868365); //  14.13%
	InitDS_Sel(0x48868b65); //  15.02%
	InitDS_Sel(0x4887004d); //  20.99%
	InitDS_Sel(0x48870065); //  28.27%
	InitDS_Sel(0x48878b65); //  26.37%
	InitDS_Sel(0x488e0865); //  40.97%
	InitDS_Sel(0x488e0b65); //  23.33%
	InitDS_Sel(0x488e8b65); //  13.20%
	InitDS_Sel(0x4907004d); //  19.95%
	InitDS_Sel(0x4907084d); //   7.77%
	InitDS_Sel(0x4907884d); //  11.31%
	InitDS_Sel(0x4c429065); //  23.69%
	InitDS_Sel(0x4d068b65); //   8.78%

	// hxh

	InitDS_Sel(0x1fe04876); //   6.18%
	InitDS_Sel(0x1fe79076); //  12.32%
	InitDS_Sel(0x1fee8876); //  43.00%
	InitDS_Sel(0x1fee8976); //  10.43%
	InitDS_Sel(0x48838176); //   6.08%
	InitDS_Sel(0x48839176); //   5.87%
	InitDS_Sel(0x48878176); //   5.74%

	// grandia extreme

	InitDS_Sel(0x1fe3884c); //  27.75%
	InitDS_Sel(0x1fe3934c); //  19.29%
	InitDS_Sel(0x45269070); //   5.21%
	InitDS_Sel(0x452e9070); //   6.70%
	InitDS_Sel(0x48868070); //  13.30%
	InitDS_Sel(0x48869070); //  23.46%
	InitDS_Sel(0x48878370); //  24.55%
	InitDS_Sel(0x48879070); //  23.58%
	InitDS_Sel(0x48879370); //  13.95%
	InitDS_Sel(0x4888404c); //  12.23%
	InitDS_Sel(0x48884050); //  14.23%
	InitDS_Sel(0x488e8b70); //  33.31%
	InitDS_Sel(0x488e9370); //  14.62%
	InitDS_Sel(0x4c81934c); //  61.93%

	// enthusa

	InitDS_Sel(0x1fe04854); //  23.09%
	InitDS_Sel(0x1fe60064); //   6.78%
	InitDS_Sel(0x1fee0864); //  13.58%
	InitDS_Sel(0x48860f64); //  14.83%
	InitDS_Sel(0x488e0f64); //  13.48%
	InitDS_Sel(0x4a46884c); //   6.65%
	InitDS_Sel(0x4b020864); //  13.03%
	InitDS_Sel(0x4b060864); //  15.89%
	InitDS_Sel(0x4b068864); //  13.96%
	InitDS_Sel(0x4d40404c); //  11.94%

	// ys 1/2 eternal story

	InitDS_Sel(0x4907004c); //   7.77%
	InitDS_Sel(0x4c8791cc); //  10.50%

	// bloody roar

	InitDS_Sel(0x1fe84868); //   6.32%
	InitDS_Sel(0x1fee8b68); //   6.47%
	InitDS_Sel(0x48810068); //  23.23%
	InitDS_Sel(0x48818368); //   9.85%
	InitDS_Sel(0x48848068); //  58.71%
	InitDS_Sel(0x488e9368); //  13.27%
	InitDS_Sel(0x49004868); //  13.50%
	InitDS_Sel(0x49018368); //  12.68%
	InitDS_Sel(0x49019368); //  12.45%
	InitDS_Sel(0x4b068068); //  19.94%
	InitDS_Sel(0x4b078068); //  13.35%
	InitDS_Sel(0x4c469068); //   8.70%
	InitDS_Sel(0x4c8e8868); //   8.89%

	// ferrari f355 challenge

	InitDS_Sel(0x48804b64); //  17.19%
	InitDS_Sel(0x48858168); //  34.57%
	InitDS_Sel(0x489e8b64); //   7.69%
	InitDS_Sel(0x5fe04068); //  14.18%
	InitDS_Sel(0x5fe04868); //   8.42%
	InitDS_Sel(0x5fe60064); //  15.16%
	InitDS_Sel(0x5fee0064); //  10.10%
	InitDS_Sel(0x5fee0864); //  27.25%
	InitDS_Sel(0x5feeb864); //   6.29%
	InitDS_Sel(0x5ff60064); //  17.90%
	InitDS_Sel(0x5ffe0064); //  20.20%
	InitDS_Sel(0x5ffe0864); //  24.20%

	// king of fighters xi

	InitDS_Sel(0x488589e0); //  84.55%
	InitDS_Sel(0x488591e0); //  43.87%
	InitDS_Sel(0x74819050); //   9.68%

	// mana khemia

	InitDS_Sel(0x488e8369); //  96.66%
	InitDS_Sel(0x49078b69); //  21.54%
	InitDS_Sel(0x490f8069); //  35.98%
	InitDS_Sel(0x490f8369); //  17.82%

	// ar tonelico 2

	InitDS_Sel(0x484f8369); //   7.83%
	InitDS_Sel(0x48804059); //  16.56%
	InitDS_Sel(0x48859059); //  12.16%
	InitDS_Sel(0x488e8069); //  23.62%
	InitDS_Sel(0x488e9069); //  38.70%
	InitDS_Sel(0x488e9369); // 114.72%
	InitDS_Sel(0x488f8069); //   5.62%
	InitDS_Sel(0x488f8369); //  22.95%
	InitDS_Sel(0x488f9069); //  35.98%
	InitDS_Sel(0x4905904d); //   6.16%
	InitDS_Sel(0x4907904d); //   5.39%
	InitDS_Sel(0x49079059); //  98.11%

	// rouge galaxy

	InitDS_Sel(0x484e8164); //  53.59%
	InitDS_Sel(0x48858154); //  12.97%
	InitDS_Sel(0x490e8164); //  14.44%
	InitDS_Sel(0x5ff0404c); //   7.34%

	// mobile suit gundam seed battle assault 3

	InitDS_Sel(0x488390cc); //  21.55%
	InitDS_Sel(0x488781cc); //   8.13%
	InitDS_Sel(0x488791cc); //   5.77%
	InitDS_Sel(0x490781cc); //  10.40%
	InitDS_Sel(0x490e9164); //   6.89%
	InitDS_Sel(0x4c81004d); //  11.28%
	InitDS_Sel(0x5fee8074); //   9.50%
	InitDS_Sel(0x5fee8874); //  32.63%
	InitDS_Sel(0x5fef8074); // 133.09%
	InitDS_Sel(0x5fef8874); //  64.27%

	// hajime no ippo all stars

	InitDS_Sel(0x48848368); //   7.59%
	InitDS_Sel(0x48848868); //  14.98%
	InitDS_Sel(0x48848b68); //   5.46%
	InitDS_Sel(0x48858368); //   8.55%
	InitDS_Sel(0x48868b68); //  20.92%
	InitDS_Sel(0x488e9068); //  10.44%
	InitDS_Sel(0x49028368); //   5.22%
	InitDS_Sel(0x4b068868); //   9.10%
	InitDS_Sel(0x4b0e8868); //  19.19%

	// virtual tennis 2

	InitDS_Sel(0x1540404d); //   6.50%
	InitDS_Sel(0x48859065); //  12.77%
	InitDS_Sel(0x48868075); //  19.46%
	InitDS_Sel(0x4c818055); //  14.22%
	InitDS_Sel(0x4c8781f5); //  12.24%

	// crash wrath of cortex

	InitDS_Sel(0x1fe1804d); //   7.61%
	InitDS_Sel(0x1fe20864); //  16.10%
	InitDS_Sel(0x48818364); //   8.35%
	InitDS_Sel(0x48828764); //   6.50%
	InitDS_Sel(0x48828f64); //  13.99%
	InitDS_Sel(0x48838364); //  14.12%
	InitDS_Sel(0x49028f64); //  19.37%
	InitDS_Sel(0x49030064); // 179.29%
	InitDS_Sel(0x49038f64); //  13.57%
	InitDS_Sel(0x4a838364); //   7.88%
	InitDS_Sel(0x4d05834c); //   9.20%

	// sbam 2

	InitDS_Sel(0x1fe04068); //   7.71%
	InitDS_Sel(0x1fe59068); //  10.44%
	InitDS_Sel(0x1fe591e8); //  12.80%
	InitDS_Sel(0x488591e8); //  13.43%
	InitDS_Sel(0x55204068); //  12.19%

	// remember 11

	InitDS_Sel(0x48819068); //  23.39%
	InitDS_Sel(0x48839068); //  51.74%
	InitDS_Sel(0x4c818068); //  28.92%
	InitDS_Sel(0x4c839068); //  33.82%

	// prince of tennis

	InitDS_Sel(0x4885894c); //   9.05%
	InitDS_Sel(0x48859164); //  16.37%
	InitDS_Sel(0x488d8164); //  17.59%
	InitDS_Sel(0x488d81cc); //   5.09%
	InitDS_Sel(0x488d8964); //  22.85%
	InitDS_Sel(0x488d9064); //  21.53%
	InitDS_Sel(0x488d9164); //  39.14%
	InitDS_Sel(0x48958164); //  10.99%
	InitDS_Sel(0x489d814c); //   8.12%
	InitDS_Sel(0x4d458064); //  14.21%

	// ar tonelico

	InitDS_Sel(0x48804069); //  14.91%
	InitDS_Sel(0x4881804d); //  17.27%
	InitDS_Sel(0x48819059); //  19.32%
	InitDS_Sel(0x48868369); //  16.49%
	InitDS_Sel(0x48869369); //  18.98%
	InitDS_Sel(0x48878069); //  29.82%
	InitDS_Sel(0x48878369); //  12.77%
	InitDS_Sel(0x48879069); //  29.43%
	InitDS_Sel(0x48879369); //   9.79%
	InitDS_Sel(0x488f9369); //  78.06%
	InitDS_Sel(0x49038069); // 225.23%
	InitDS_Sel(0x490d804d); //   5.50%
	InitDS_Sel(0x490f804d); //  43.68%

	// dbz sagas

	// tourist trophy

	InitDS_Sel(0x1fe7904c); //   6.40%
	InitDS_Sel(0x1fe84064); //  15.82%
	InitDS_Sel(0x1fe84864); //  16.96%
	InitDS_Sel(0x488a8064); //  20.47%
	InitDS_Sel(0x488e9065); //  19.88%
	InitDS_Sel(0x5fe84064); //   8.14%
	InitDS_Sel(0x5fee9064); //   5.91%

	// svr2k8

	InitDS_Sel(0x1fe79066); //  16.07%
	InitDS_Sel(0x4880494c); //  19.04%
	InitDS_Sel(0x48804b4c); //   8.73%
	InitDS_Sel(0x488a0064); //   5.01%
	InitDS_Sel(0x488a0864); //  13.95%
	InitDS_Sel(0x490e8064); //  24.40%
	InitDS_Sel(0x490e8364); //  28.17%
	InitDS_Sel(0x4c839064); // 136.71%
	InitDS_Sel(0x55384874); //   5.80%

	// tokyo bus guide

	// 12riven

	// xenosaga

	// mgs3s1

	// god of war

	// aura for laura

	InitDS_Sel(0x1fe04070); // 269.55%
	InitDS_Sel(0x1fe19050); //   8.51%
	InitDS_Sel(0x1fe38070); //  26.29%
	InitDS_Sel(0x1fe68070); //  18.16%
	InitDS_Sel(0x1fe6904c); //   9.38%
	InitDS_Sel(0x1fe78050); //  13.57%
	InitDS_Sel(0x1fe78070); // 102.99%
	InitDS_Sel(0x1fe79070); //  53.21%
	InitDS_Sel(0x1fefb04c); //  16.50%
	InitDS_Sel(0x4523804c); // 115.55%
	InitDS_Sel(0x48839050); //  14.89%
	InitDS_Sel(0x49058050); // 321.36%
	InitDS_Sel(0x4c818050); //  46.00%
	InitDS_Sel(0x55204050); //  24.81%

	#endif
}

template<DWORD fpsm, DWORD zpsm, DWORD ztst, DWORD iip>
void GSDrawScanline::DrawScanline(int top, int left, int right, const GSVertexSW& v)
{
	int skip = left & 3;

	left -= skip;

	int steps = right - left;

	GSVector4i test = s_test[skip] | s_test[GSVector4i::min_i16(4, steps) + 4];

	//

	GSVector4i fza_base;
	GSVector4i* fza_offset;

	GSVector4 z, s, t, q;
	GSVector4i f, rb, ga;

	// fza

	fza_base = m_env.fzbr[top];
	fza_offset = &m_env.fzbc[left >> 2];

	// v.p

	GSVector4 vp = v.p;

	z = vp.zzzz() + m_env.d[skip].z;
	f = GSVector4i(vp).zzzzh().zzzz().add16(m_env.d[skip].f);

	// v.t

	GSVector4 vt = v.t;

	s = vt.xxxx() + m_env.d[skip].s; 
	t = vt.yyyy() + m_env.d[skip].t;
	q = vt.zzzz() + m_env.d[skip].q;

	// v.c

	if(iip)
	{
		GSVector4i vc = GSVector4i(v.c);

		vc = vc.upl16(vc.zwxy());

		rb = vc.xxxx().add16(m_env.d[skip].rb);
		ga = vc.zzzz().add16(m_env.d[skip].ga);
	}
	else
	{
		rb = m_env.rb;
		ga = m_env.ga;
	}

	//

	while(1)
	{
		do
		{
			GSVector4i fza = fza_base + *fza_offset;
			
			GSVector4i zs = (GSVector4i(z * 0.5f) << 1) | (GSVector4i(z) & GSVector4i::x00000001());
			GSVector4i zd = GSVector4i::zero();

			if(ztst > 1)
			{
				zd = ReadZBufX(zpsm, fza);

				if(!TestZ(zpsm, ztst, zs, zd, test))
				{
					continue;
				}
			}

			GSVector4i c[6];

			if(m_env.sel.tfx != TFX_NONE)
			{
				SampleTexture(ztst, m_env.sel.fst, m_env.sel.ltf, m_env.sel.tlu, test, s, t, q, c);
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

			GSVector4i fd = GSVector4i::zero();

			if(m_env.sel.rfb)
			{
				fd = ReadFrameX(fpsm, fza);

				if(!TestDestAlpha(fpsm, m_env.sel.date, fd, test))
				{
					continue;
				}
			}

			fm |= test;
			zm |= test;

			int fzm = ~(fm == GSVector4i::xffffffff()).ps32(zm == GSVector4i::xffffffff()).mask();

			WriteZBufX(zpsm, ztst, zs, zd, zm, fza, fzm);

			if(m_env.sel.abe != 255)
			{
				GSVector4i mask = GSVector4i::x00ff();

				switch(fpsm)
				{
				case 0:
					c[2] = fd & mask;
					c[3] = (fd >> 8) & mask;
					break;
				case 1:
					c[2] = fd & mask;
					c[3] = (fd >> 8) & mask;
					c[3] = c[3].mix16(GSVector4i(0x00800000));
					break;
				case 2:
					c[2] = ((fd & 0x7c00) << 9) | ((fd & 0x001f) << 3);
					c[3] = ((fd & 0x8000) << 8) | ((fd & 0x03e0) >> 2);
					break;
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
					mask = (c[1] << 8).sra32(31);

					rb = c[0].blend8(rb, mask);
					ga = c[1].blend8(ga, mask);
				}

				c[0] = rb;
				c[1] = ga.mix16(c[1]);
			}

			WriteFrameX(fpsm, m_env.sel.rfb, c, fd, fm, fza, fzm);
		}
		while(0);

		if(steps <= 4) break;

		steps -= 4;

		test = s_test[GSVector4i::min_i16(4, steps) + 4];

		fza_offset++;

		z += m_env.d4.z;
		f = f.add16(m_env.d4.f);

		GSVector4 stq = m_env.d4.stq;

		s += stq.xxxx();
		t += stq.yyyy();
		q += stq.zzzz();

		if(iip)
		{
			GSVector4i c = m_env.d4.c;

			rb = rb.add16(c.xxxx());
			ga = ga.add16(c.yyyy());
		}
	}
}

template<DWORD sel>
void GSDrawScanline::DrawScanlineEx(int top, int left, int right, const GSVertexSW& v)
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

	//

	int skip = left & 3;

	left -= skip;

	int steps = right - left;

	GSVector4i test = s_test[skip] | s_test[GSVector4i::min_i16(4, steps) + 4];

	//

	GSVector4i fza_base;
	GSVector4i* fza_offset;

	GSVector4 z, s, t, q;
	GSVector4i f, rb, ga;

	// fza

	fza_base = m_env.fzbr[top];
	fza_offset = &m_env.fzbc[left >> 2];

	// v.p

	GSVector4 vp = v.p;

	z = vp.zzzz() + m_env.d[skip].z;
	f = GSVector4i(vp).zzzzh().zzzz().add16(m_env.d[skip].f);

	// v.t

	GSVector4 vt = v.t;

	s = vt.xxxx() + m_env.d[skip].s; 
	t = vt.yyyy() + m_env.d[skip].t;
	q = vt.zzzz() + m_env.d[skip].q;

	// v.c

	if(iip)
	{
		GSVector4i vc = GSVector4i(v.c);

		vc = vc.upl16(vc.zwxy());

		rb = vc.xxxx().add16(m_env.d[skip].rb);
		ga = vc.zzzz().add16(m_env.d[skip].ga);
	}
	else
	{
		rb = m_env.rb;
		ga = m_env.ga;
	}

	//

	while(1)
	{
		do
		{
			GSVector4i fza = fza_base + *fza_offset;
			
			GSVector4i zs = (GSVector4i(z * 0.5f) << 1) | (GSVector4i(z) & GSVector4i::x00000001());
			GSVector4i zd = GSVector4i::zero();

			if(ztst > 1)
			{
				zd = ReadZBufX(zpsm, fza);

				if(!TestZ(zpsm, ztst, zs, zd, test))
				{
					continue;
				}
			}

			GSVector4i c[6];

			if(tfx != TFX_NONE)
			{
				SampleTexture(ztst, fst, ltf, tlu, test, s, t, q, c);
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

			GSVector4i fd = GSVector4i::zero();

			if(rfb)
			{
				fd = ReadFrameX(fpsm, fza);

				if(!TestDestAlpha(fpsm, date, fd, test))
				{
					continue;
				}
			}

			fm |= test;
			zm |= test;

			int fzm = ~(fm == GSVector4i::xffffffff()).ps32(zm == GSVector4i::xffffffff()).mask();

			WriteZBufX(zpsm, ztst, zs, zd, zm, fza, fzm);

			if(abe != 255)
			{
				GSVector4i mask = GSVector4i::x00ff();

				switch(fpsm)
				{
				case 0:
				case 1:
					c[2] = fd & mask;
					c[3] = (fd >> 8) & mask;
					break;
				case 2:
					c[2] = ((fd & 0x7c00) << 9) | ((fd & 0x001f) << 3);
					c[3] = ((fd & 0x8000) << 8) | ((fd & 0x03e0) >> 2);
					break;
				}

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
					mask = (c[1] << 8).sra32(31);

					rb = c[0].blend8(rb, mask);
					ga = c[1].blend8(ga, mask);
				}

				c[0] = rb;
				c[1] = ga.mix16(c[1]);
			}

			WriteFrameX(fpsm, rfb, c, fd, fm, fza, fzm);
		}
		while(0);

		if(steps <= 4) break;

		steps -= 4;

		test = s_test[GSVector4i::min_i16(4, steps) + 4];

		fza_offset++;

		z += m_env.d4.z;
		f = f.add16(m_env.d4.f);

		GSVector4 stq = m_env.d4.stq;

		s += stq.xxxx();
		t += stq.yyyy();
		q += stq.zzzz();

		if(iip)
		{
			GSVector4i c = m_env.d4.c;

			rb = rb.add16(c.xxxx());
			ga = ga.add16(c.yyyy());
		}
	}
}

const GSVector4 GSDrawScanline::s_ps0123[4] = 
{
	GSVector4(0.0f, 1.0f, 2.0f, 3.0f),
	GSVector4(-1.0f, 0.0f, 1.0f, 2.0f),
	GSVector4(-2.0f, -1.0f, 0.0f, 1.0f),
	GSVector4(-3.0f, -2.0f, -1.0f, 0.0f),
};

const GSVector4i GSDrawScanline::s_test[9] = 
{
	GSVector4i::zero(),
	GSVector4i(0xffffffff, 0, 0, 0),
	GSVector4i(0xffffffff, 0xffffffff, 0, 0),
	GSVector4i(0xffffffff, 0xffffffff, 0xffffffff, 0),
	GSVector4i(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff),
	GSVector4i(0, 0xffffffff, 0xffffffff, 0xffffffff),
	GSVector4i(0, 0, 0xffffffff, 0xffffffff),
	GSVector4i(0, 0, 0, 0xffffffff),
	GSVector4i::zero(),
};
