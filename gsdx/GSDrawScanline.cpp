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
	Init();
}

GSDrawScanline::~GSDrawScanline()
{
	POSITION pos = m_dsmap_active.GetHeadPosition();

	while(pos)
	{
		delete m_dsmap_active.GetNextValue(pos);
	}

	m_dsmap_active.RemoveAll();
}

void GSDrawScanline::PrintStats()
{
	if(FILE* fp = fopen("c:\\1.txt", "w"))
	{
		POSITION pos = m_dsmap_active.GetHeadPosition();

		while(pos)
		{
			DWORD sel;
			ActiveDrawScanlinePtr* p;
			
			m_dsmap_active.GetNextAssoc(pos, sel, p);

			if(m_dsmap.Lookup(sel))
			{
				continue;
			}

			if(p->frames > 30)
			{
				int tpf = (int)((p->ticks / p->frames) * 10000 / (3000000000 / 60)); // 3 GHz, 60 fps

				if(tpf >= 200)
				{
					_ftprintf(fp, _T("m_dsmap.SetAt(0x%08x, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x%08x>); // %6.2f%%\n"), sel, sel, (float)tpf / 100);
				}
			}
		}

		fclose(fp);
	}

	{
		__int64 ttpf = 0;

		POSITION pos = m_dsmap_active.GetHeadPosition();

		while(pos)
		{
			ActiveDrawScanlinePtr* p = m_dsmap_active.GetNextValue(pos);
			
			ttpf += p->ticks / p->frames;
		}

		pos = m_dsmap_active.GetHeadPosition();

		while(pos)
		{
			DWORD sel;
			ActiveDrawScanlinePtr* p;

			m_dsmap_active.GetNextAssoc(pos, sel, p);

			if(p->frames > 0)
			{
				__int64 tpp = p->pixels > 0 ? p->ticks / p->pixels : 0;
				__int64 tpf = p->frames > 0 ? p->ticks / p->frames : 0;
				__int64 ppf = p->frames > 0 ? p->pixels / p->frames : 0;

				printf("[%08x]%c %6.2f%% | %5.2f%% | f %4I64d | p %10I64d | tpp %4I64d | tpf %8I64d | ppf %7I64d\n", 
					sel, !m_dsmap.Lookup(sel) ? '*' : ' ',
					(float)(tpf * 10000 / 50000000) / 100, 
					(float)(tpf * 10000 / ttpf) / 100, 
					p->frames, p->pixels, 
					tpp, tpf, ppf);
			}
		}
	}
}
// IDrawScanline

bool GSDrawScanline::BeginDraw(const GSRasterizerData* data)
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

	m_dsf = NULL;

	if(!m_dsmap_active.Lookup(m_env.sel, m_dsf))
	{
		CRBMap<DWORD, DrawScanlinePtr>::CPair* pair = m_dsmap.Lookup(m_env.sel);

		ActiveDrawScanlinePtr* p = new ActiveDrawScanlinePtr();

		memset(p, 0, sizeof(*p));

		p->frame = (UINT64)-1;

		p->dsf = pair ? pair->m_value : m_ds[m_env.sel.fpsm][m_env.sel.zpsm][m_env.sel.ztst][m_env.sel.iip];

		m_dsmap_active.SetAt(m_env.sel, p);

		m_dsf = p;
	}

	if(data->primclass == GS_SPRITE_CLASS)
	{
		bool solid = true;

		if(m_env.sel.iip 
		|| m_env.sel.tfx != TFX_NONE
		|| m_env.sel.abe != 255 
		|| m_env.sel.fge
		|| m_env.sel.ztst > 1 
		|| m_env.sel.atst > 1
		|| m_env.sel.date)
		{
			solid = false;
		}

		return solid;
	}

	return false;
}

void GSDrawScanline::EndDraw(const GSRasterizerStats& stats)
{
	UINT64 frame = m_state->m_perfmon.GetFrame();

	if(m_dsf->frame != frame)
	{
		m_dsf->frame = frame;
		m_dsf->frames++;
	}

	m_dsf->pixels += stats.pixels;
	m_dsf->ticks += stats.ticks;
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
	(this->*m_dsf->dsf)(top, left, right, v);
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

IDrawScanline::DrawScanlinePtr GSDrawScanline::GetDrawScanlinePtr()
{
	return m_dsf->dsf;
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

	m_dsmap.SetAt(0x1fe04850, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04850>); //   6.71%
	m_dsmap.SetAt(0x1fe28864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe28864>); //   4.49%
	m_dsmap.SetAt(0x1fe28870, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe28870>); //  21.34%
	m_dsmap.SetAt(0x1fe38050, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe38050>); //  17.74%
	m_dsmap.SetAt(0x1fe38060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe38060>); //   6.70%
	m_dsmap.SetAt(0x1fe38064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe38064>); //  20.15%
	m_dsmap.SetAt(0x1fe39050, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe39050>); // 103.71%
	m_dsmap.SetAt(0x48428050, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48428050>); //   7.52%
	m_dsmap.SetAt(0x48428060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48428060>); //   5.18%
	m_dsmap.SetAt(0x48804050, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804050>); //  10.09%
	m_dsmap.SetAt(0x48804860, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804860>); //  20.42%
	m_dsmap.SetAt(0x48839050, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48839050>); //   2.44%
	m_dsmap.SetAt(0x49004050, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004050>); //   8.89%
	m_dsmap.SetAt(0x49028060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49028060>); //   5.08%
	m_dsmap.SetAt(0x4902884c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4902884c>); //   2.41%
	m_dsmap.SetAt(0x4902904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4902904c>); //   7.00%
	m_dsmap.SetAt(0x49038050, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49038050>); //  21.08%
	m_dsmap.SetAt(0x49039050, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49039050>); //  11.79%
	m_dsmap.SetAt(0x4b02804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b02804c>); //   8.65%
	m_dsmap.SetAt(0x4c40404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c40404c>); //   2.47%
	m_dsmap.SetAt(0x4c419050, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c419050>); //  28.12%
	m_dsmap.SetAt(0x4c804050, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c804050>); //  10.93%
	m_dsmap.SetAt(0x4d019050, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d019050>); //  93.98%
	m_dsmap.SetAt(0x4d028864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d028864>); //  25.74%
	m_dsmap.SetAt(0x4d038864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d038864>); // 138.18%
	m_dsmap.SetAt(0x49029054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49029054>); //   3.71%
	m_dsmap.SetAt(0x4b028054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b028054>); //   3.79%
	m_dsmap.SetAt(0x4d00484c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d00484c>); //   5.70%
	m_dsmap.SetAt(0x4d02884c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d02884c>); // 108.92%

	// ffx

	m_dsmap.SetAt(0x1fe04055, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04055>); //   2.75%
	m_dsmap.SetAt(0x1fe11056, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe11056>); //  12.17%
	m_dsmap.SetAt(0x1fe3914d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3914d>); //  15.42%
	m_dsmap.SetAt(0x1fe68075, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe68075>); //  23.08%
	m_dsmap.SetAt(0x1fe68155, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe68155>); //  15.72%
	m_dsmap.SetAt(0x1fe68175, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe68175>); //   3.59%
	m_dsmap.SetAt(0x1fe68975, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe68975>); //   7.44%
	m_dsmap.SetAt(0x1fe69175, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe69175>); //  16.08%
	m_dsmap.SetAt(0x1fe78075, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe78075>); //  21.09%
	m_dsmap.SetAt(0x1fe78155, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe78155>); //  15.25%
	m_dsmap.SetAt(0x1fe84175, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe84175>); //   5.36%
	m_dsmap.SetAt(0x1fee8975, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee8975>); //  53.72%
	m_dsmap.SetAt(0x48268965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48268965>); //   2.27%
	m_dsmap.SetAt(0x48468965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48468965>); //  23.92%
	m_dsmap.SetAt(0x48478165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48478165>); //   2.04%
	m_dsmap.SetAt(0x48804055, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804055>); //  13.51%
	m_dsmap.SetAt(0x4880414d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4880414d>); //   9.97%
	m_dsmap.SetAt(0x48804155, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804155>); //  14.30%
	m_dsmap.SetAt(0x488041cd, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488041cd>); //   2.69%
	m_dsmap.SetAt(0x48804855, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804855>); //  14.28%
	m_dsmap.SetAt(0x4880494d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4880494d>); //   6.36%
	m_dsmap.SetAt(0x48804965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804965>); //  16.08%
	m_dsmap.SetAt(0x48820165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48820165>); //   6.05%
	m_dsmap.SetAt(0x48820965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48820965>); //  26.53%
	m_dsmap.SetAt(0x48828965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48828965>); //   2.32%
	m_dsmap.SetAt(0x4883014d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883014d>); //  18.75%
	m_dsmap.SetAt(0x48830155, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48830155>); //  12.23%
	m_dsmap.SetAt(0x48830875, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48830875>); //  18.23%
	m_dsmap.SetAt(0x48868165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868165>); //  10.32%
	m_dsmap.SetAt(0x48868965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868965>); //  25.16%
	m_dsmap.SetAt(0x4887814d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887814d>); //   8.68%
	m_dsmap.SetAt(0x48878165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878165>); //  43.16%
	m_dsmap.SetAt(0x488789cd, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488789cd>); //   5.04%
	m_dsmap.SetAt(0x4887914d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887914d>); //   2.06%
	m_dsmap.SetAt(0x488791cd, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488791cd>); //   4.09%
	m_dsmap.SetAt(0x488791e5, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488791e5>); //  49.19%
	m_dsmap.SetAt(0x488e8965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8965>); //   5.78%
	m_dsmap.SetAt(0x488f89f5, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488f89f5>); //  15.95%
	m_dsmap.SetAt(0x49004165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004165>); //  13.61%
	m_dsmap.SetAt(0x49004875, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004875>); //   4.22%
	m_dsmap.SetAt(0x49068165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068165>); //  11.28%
	m_dsmap.SetAt(0x49068965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068965>); //  42.74%
	m_dsmap.SetAt(0x49069165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49069165>); //  48.78%
	m_dsmap.SetAt(0x4907814d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4907814d>); //  10.74%
	m_dsmap.SetAt(0x49078165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078165>); //  10.86%
	m_dsmap.SetAt(0x4c811055, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c811055>); //  15.40%
	m_dsmap.SetAt(0x4c831065, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c831065>); //   7.62%
	m_dsmap.SetAt(0x4d068075, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d068075>); //  13.70%
	m_dsmap.SetAt(0x4d078075, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d078075>); //  10.34%
	m_dsmap.SetAt(0x5100414d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5100414d>); //   9.62%
	m_dsmap.SetAt(0x51004155, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x51004155>); //  13.90%
	m_dsmap.SetAt(0x51020965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x51020965>); //  19.50%
	m_dsmap.SetAt(0x55204055, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x55204055>); //  13.44%
	m_dsmap.SetAt(0x1fee8175, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee8175>); //   2.11%
	m_dsmap.SetAt(0x48878965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878965>); //  51.94%
	m_dsmap.SetAt(0x488e8165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8165>); //   4.45%
	m_dsmap.SetAt(0x49004865, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004865>); //   3.36%
	m_dsmap.SetAt(0x49004965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004965>); //   2.24%

	// ffx-2

	m_dsmap.SetAt(0x1fe30069, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe30069>); //  11.15%
	m_dsmap.SetAt(0x48478965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48478965>); //   2.84%
	m_dsmap.SetAt(0x4880404d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4880404d>); //  16.04%
	m_dsmap.SetAt(0x48804165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804165>); //  25.67%
	m_dsmap.SetAt(0x488049cd, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488049cd>); //   3.18%
	m_dsmap.SetAt(0x4881004d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4881004d>); //  13.32%
	m_dsmap.SetAt(0x4881814d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4881814d>); //  30.64%
	m_dsmap.SetAt(0x4881894d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4881894d>); //   5.29%
	m_dsmap.SetAt(0x488191cd, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488191cd>); //  11.56%
	m_dsmap.SetAt(0x488391cd, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488391cd>); //  19.16%
	m_dsmap.SetAt(0x4884804e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4884804e>); //   2.98%
	m_dsmap.SetAt(0x4885004d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4885004d>); //  13.67%
	m_dsmap.SetAt(0x4885894d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4885894d>); //   7.18%
	m_dsmap.SetAt(0x488781cd, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488781cd>); //   3.95%
	m_dsmap.SetAt(0x488781f5, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488781f5>); //   6.04%
	m_dsmap.SetAt(0x48879165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879165>); //   2.94%
	m_dsmap.SetAt(0x49004059, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004059>); //  13.65%
	m_dsmap.SetAt(0x49004155, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004155>); //  13.71%
	m_dsmap.SetAt(0x49004859, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004859>); //  15.97%
	m_dsmap.SetAt(0x49004869, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004869>); //   6.07%
	m_dsmap.SetAt(0x4900494d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4900494d>); //   4.01%
	m_dsmap.SetAt(0x49059155, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49059155>); //  12.92%
	m_dsmap.SetAt(0x49078965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078965>); //  12.75%
	m_dsmap.SetAt(0x490e8165, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8165>); //   7.08%
	m_dsmap.SetAt(0x4c45894d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c45894d>); //   6.93%
	m_dsmap.SetAt(0x4c850055, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c850055>); //  17.44%
	m_dsmap.SetAt(0x4d0481e5, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d0481e5>); //   2.13%
	m_dsmap.SetAt(0x4d05894d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d05894d>); //   6.95%
	m_dsmap.SetAt(0x5fe0404e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe0404e>); //   9.04%
	m_dsmap.SetAt(0x5fe1004d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe1004d>); //  16.33%
	m_dsmap.SetAt(0x5fe5884d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe5884d>); //  11.57%

	// ffxii

	m_dsmap.SetAt(0x1fe04054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04054>); //   3.09%
	m_dsmap.SetAt(0x1fe3804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3804c>); //   4.74%
	m_dsmap.SetAt(0x1fe3904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3904c>); //   8.97%
	m_dsmap.SetAt(0x1fe6804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe6804c>); //   3.93%
	m_dsmap.SetAt(0x1fe68164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe68164>); //   7.59%
	m_dsmap.SetAt(0x1fe6884c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe6884c>); //  19.02%
	m_dsmap.SetAt(0x1fec8864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fec8864>); //   3.35%
	m_dsmap.SetAt(0x1fee8964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee8964>); //  81.44%
	m_dsmap.SetAt(0x48404064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48404064>); //   7.14%
	m_dsmap.SetAt(0x4847004c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4847004c>); //  22.92%
	m_dsmap.SetAt(0x4880404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4880404c>); //   4.82%
	m_dsmap.SetAt(0x48804064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804064>); //   3.61%
	m_dsmap.SetAt(0x48828064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48828064>); //  68.74%
	m_dsmap.SetAt(0x48828864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48828864>); //   7.78%
	m_dsmap.SetAt(0x4883004c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883004c>); //  20.47%
	m_dsmap.SetAt(0x4883084c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883084c>); //  13.18%
	m_dsmap.SetAt(0x4883804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883804c>); //  21.09%
	m_dsmap.SetAt(0x4883884c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883884c>); //  28.30%
	m_dsmap.SetAt(0x4883904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883904c>); //  71.52%
	m_dsmap.SetAt(0x4886804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4886804c>); //   4.74%
	m_dsmap.SetAt(0x48868064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868064>); //   5.40%
	m_dsmap.SetAt(0x4886884c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4886884c>); //  15.40%
	m_dsmap.SetAt(0x4887084c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887084c>); // 309.13%
	m_dsmap.SetAt(0x4887804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887804c>); //   6.68%
	m_dsmap.SetAt(0x48878064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878064>); //   5.95%
	m_dsmap.SetAt(0x4887884c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887884c>); //   3.85%
	m_dsmap.SetAt(0x4887904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887904c>); //   3.84%
	m_dsmap.SetAt(0x48879064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879064>); //  26.55%
	m_dsmap.SetAt(0x488e8b64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8b64>); //  23.72%
	m_dsmap.SetAt(0x48904064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48904064>); //   6.55%
	m_dsmap.SetAt(0x48c0404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48c0404c>); //   9.43%
	m_dsmap.SetAt(0x48c0484c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48c0484c>); //  10.19%
	m_dsmap.SetAt(0x48c7804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48c7804c>); //  22.30%
	m_dsmap.SetAt(0x48c7884c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48c7884c>); //   5.60%
	m_dsmap.SetAt(0x4903804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4903804c>); //  13.92%
	m_dsmap.SetAt(0x49068864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068864>); //   3.22%
	m_dsmap.SetAt(0x49068b64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068b64>); //   2.86%
	m_dsmap.SetAt(0x4907804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4907804c>); //  95.02%
	m_dsmap.SetAt(0x49078064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078064>); //   3.87%
	m_dsmap.SetAt(0x4907884c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4907884c>); //  12.32%
	m_dsmap.SetAt(0x490e8b64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8b64>); //   4.55%
	m_dsmap.SetAt(0x49278064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49278064>); //  87.48%
	m_dsmap.SetAt(0x5fe0404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe0404c>); //  24.18%
	m_dsmap.SetAt(0x5fe04064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe04064>); //  33.59%

	// kingdom hearts

	m_dsmap.SetAt(0x1fe0404d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe0404d>); //   3.00%
	m_dsmap.SetAt(0x1fe1804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe1804c>); //   7.68%
	m_dsmap.SetAt(0x1fe3004d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3004d>); //  21.82%
	m_dsmap.SetAt(0x1fe3804d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3804d>); //  18.18%
	m_dsmap.SetAt(0x1fe3904d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3904d>); //  16.26%
	m_dsmap.SetAt(0x1fee9164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee9164>); //   5.95%
	m_dsmap.SetAt(0x4840414c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4840414c>); //  16.75%
	m_dsmap.SetAt(0x4846814c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4846814c>); //   2.10%
	m_dsmap.SetAt(0x48468174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48468174>); //  14.86%
	m_dsmap.SetAt(0x48468974, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48468974>); //  18.45%
	m_dsmap.SetAt(0x4847814c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4847814c>); // 159.55%
	m_dsmap.SetAt(0x48478174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48478174>); //  32.95%
	m_dsmap.SetAt(0x48804054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804054>); //  11.46%
	m_dsmap.SetAt(0x4880414c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4880414c>); //   9.35%
	m_dsmap.SetAt(0x4880494c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4880494c>); //   4.47%
	m_dsmap.SetAt(0x48804974, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804974>); //   3.16%
	m_dsmap.SetAt(0x4881004c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4881004c>); //  11.74%
	m_dsmap.SetAt(0x48818054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48818054>); //  14.49%
	m_dsmap.SetAt(0x48819054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48819054>); //  15.36%
	m_dsmap.SetAt(0x48829164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48829164>); //   5.19%
	m_dsmap.SetAt(0x48830054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48830054>); //  22.16%
	m_dsmap.SetAt(0x48830164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48830164>); // 126.37%
	m_dsmap.SetAt(0x48858054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48858054>); //  11.97%
	m_dsmap.SetAt(0x48858854, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48858854>); //   3.75%
	m_dsmap.SetAt(0x48859054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48859054>); //   2.28%
	m_dsmap.SetAt(0x4886814c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4886814c>); //   6.35%
	m_dsmap.SetAt(0x48868154, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868154>); //   6.93%
	m_dsmap.SetAt(0x48868174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868174>); //   4.04%
	m_dsmap.SetAt(0x48868974, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868974>); //   9.24%
	m_dsmap.SetAt(0x48878054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878054>); //   2.66%
	m_dsmap.SetAt(0x4887814c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887814c>); //   2.16%
	m_dsmap.SetAt(0x48878174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878174>); //   7.10%
	m_dsmap.SetAt(0x4887894c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887894c>); //   3.03%
	m_dsmap.SetAt(0x48879054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879054>); //   2.20%
	m_dsmap.SetAt(0x488e8164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8164>); //  16.75%
	m_dsmap.SetAt(0x488e8964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8964>); //  19.81%
	m_dsmap.SetAt(0x488e9164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e9164>); //  15.93%
	m_dsmap.SetAt(0x488e9168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e9168>); //   2.82%
	m_dsmap.SetAt(0x48c68174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48c68174>); //   4.21%
	m_dsmap.SetAt(0x49004054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004054>); //  13.40%
	m_dsmap.SetAt(0x4900414c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4900414c>); //   8.19%
	m_dsmap.SetAt(0x49068174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068174>); //   4.84%
	m_dsmap.SetAt(0x49068974, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068974>); //   6.05%
	m_dsmap.SetAt(0x4907814c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4907814c>); //   4.97%
	m_dsmap.SetAt(0x49078174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078174>); //   8.87%
	m_dsmap.SetAt(0x4907894c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4907894c>); //   4.50%
	m_dsmap.SetAt(0x49078974, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078974>); //   4.44%
	m_dsmap.SetAt(0x49079174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49079174>); //   2.45%
	m_dsmap.SetAt(0x490e814c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e814c>); //  21.77%
	m_dsmap.SetAt(0x490e894c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e894c>); //  44.60%
	m_dsmap.SetAt(0x490e8974, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8974>); //  54.05%
	m_dsmap.SetAt(0x5fe04168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe04168>); //   2.33%
	m_dsmap.SetAt(0x5fe3804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe3804c>); //   3.05%
	m_dsmap.SetAt(0x5fe3904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe3904c>); //   5.07%

	// kingdom hearts 2

	m_dsmap.SetAt(0x1fe31054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe31054>); //  15.33%
	m_dsmap.SetAt(0x48478854, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48478854>); //   2.54%
	m_dsmap.SetAt(0x48804160, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804160>); //   3.87%
	m_dsmap.SetAt(0x48810054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48810054>); //  23.41%
	m_dsmap.SetAt(0x4883814c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883814c>); //   5.49%
	m_dsmap.SetAt(0x48878854, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878854>); //  32.99%
	m_dsmap.SetAt(0x488a8964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488a8964>); //  14.08%
	m_dsmap.SetAt(0x49004974, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004974>); //   3.26%
	m_dsmap.SetAt(0x49078854, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078854>); //   3.90%
	m_dsmap.SetAt(0x4c83004d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c83004d>); //  56.70%
	m_dsmap.SetAt(0x4d03004d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d03004d>); //  19.56%

	// persona 3

	m_dsmap.SetAt(0x484e8068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x484e8068>); //  25.28%
	m_dsmap.SetAt(0x48804868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804868>); //  13.41%
	m_dsmap.SetAt(0x4881884c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4881884c>); //  18.40%
	m_dsmap.SetAt(0x48868068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868068>); //   3.75%
	m_dsmap.SetAt(0x488e8068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8068>); //   5.40%
	m_dsmap.SetAt(0x4907904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4907904c>); //  15.09%
	m_dsmap.SetAt(0x490e8068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8068>); //   5.20%
	m_dsmap.SetAt(0x4a43004c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a43004c>); //  18.93%
	m_dsmap.SetAt(0x4b07934c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b07934c>); //  32.19%
	m_dsmap.SetAt(0x4d47834c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d47834c>); // 100.32%
	m_dsmap.SetAt(0x4d478b4c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d478b4c>); //  19.56%
	m_dsmap.SetAt(0x4d47934c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d47934c>); //  26.08%

	// persona 4

	m_dsmap.SetAt(0x1fe04058, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04058>); //   2.56%
	m_dsmap.SetAt(0x1fe04858, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04858>); //   6.40%
	m_dsmap.SetAt(0x4840484c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4840484c>); //  13.36%
	m_dsmap.SetAt(0x48804058, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804058>); //  85.25%
	m_dsmap.SetAt(0x48804068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804068>); //   8.39%
	m_dsmap.SetAt(0x48804368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804368>); //  16.16%
	m_dsmap.SetAt(0x48804b68, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804b68>); //   7.87%
	m_dsmap.SetAt(0x4881834c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4881834c>); //  19.07%
	m_dsmap.SetAt(0x4881934c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4881934c>); //  18.32%
	m_dsmap.SetAt(0x48828368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48828368>); //  10.46%
	m_dsmap.SetAt(0x48828b68, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48828b68>); //   7.37%
	m_dsmap.SetAt(0x48868868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868868>); // 320.60%
	m_dsmap.SetAt(0x48868b68, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868b68>); //  48.70%
	m_dsmap.SetAt(0x48868f68, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868f68>); // 115.10%
	m_dsmap.SetAt(0x48869368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48869368>); //  17.55%
	m_dsmap.SetAt(0x48878b4c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878b4c>); //   3.82%
	m_dsmap.SetAt(0x48879168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879168>); //  18.94%
	m_dsmap.SetAt(0x48879368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879368>); //   5.59%
	m_dsmap.SetAt(0x488e8368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8368>); //   6.75%
	m_dsmap.SetAt(0x488e8868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8868>); //  12.82%
	m_dsmap.SetAt(0x488e8b68, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8b68>); // 104.26%
	m_dsmap.SetAt(0x4900404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4900404c>); //  12.48%
	m_dsmap.SetAt(0x4900484c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4900484c>); //  13.48%
	m_dsmap.SetAt(0x4903004c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4903004c>); //  23.84%
	m_dsmap.SetAt(0x49068068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068068>); //   9.24%
	m_dsmap.SetAt(0x49068868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068868>); //  14.91%
	m_dsmap.SetAt(0x49078068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078068>); //  42.69%
	m_dsmap.SetAt(0x49079068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49079068>); //  58.18%
	m_dsmap.SetAt(0x490e8868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8868>); //  18.50%
	m_dsmap.SetAt(0x4a47804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a47804c>); //  42.91%
	m_dsmap.SetAt(0x4a47904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a47904c>); //  24.18%
	m_dsmap.SetAt(0x4a80404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a80404c>); //  46.97%
	m_dsmap.SetAt(0x4a83004c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a83004c>); // 105.26%
	m_dsmap.SetAt(0x4a879068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a879068>); //  10.58%
	m_dsmap.SetAt(0x4b00404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b00404c>); //  15.94%
	m_dsmap.SetAt(0x4b004968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b004968>); //  11.52%
	m_dsmap.SetAt(0x4b07804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b07804c>); //  23.02%
	m_dsmap.SetAt(0x4b07884c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b07884c>); //  24.58%
	m_dsmap.SetAt(0x4d0e8868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d0e8868>); //   6.12%
	m_dsmap.SetAt(0x5fe04058, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe04058>); //   2.07%
	m_dsmap.SetAt(0x5fe04858, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe04858>); //  23.73%

	// sfex3

	m_dsmap.SetAt(0x1fe1004e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe1004e>); //   6.67%
	m_dsmap.SetAt(0x1fe3004e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3004e>); //  12.64%
	m_dsmap.SetAt(0x1fe6b168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe6b168>); //  15.50%
	m_dsmap.SetAt(0x1fe6b968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe6b968>); //   5.44%
	m_dsmap.SetAt(0x41268068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x41268068>); //   7.42%
	m_dsmap.SetAt(0x4856b1cc, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4856b1cc>); //   3.14%
	m_dsmap.SetAt(0x48804158, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804158>); //   8.13%
	m_dsmap.SetAt(0x4881004e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4881004e>); //   9.85%
	m_dsmap.SetAt(0x4885904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4885904c>); //   3.54%
	m_dsmap.SetAt(0x4885914c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4885914c>); //   2.89%
	m_dsmap.SetAt(0x48859158, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48859158>); //   8.72%
	m_dsmap.SetAt(0x4886b168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4886b168>); //  14.80%
	m_dsmap.SetAt(0x4886b968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4886b968>); //  41.18%
	m_dsmap.SetAt(0x48878158, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878158>); //   2.45%
	m_dsmap.SetAt(0x4887914c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887914c>); //  14.76%
	m_dsmap.SetAt(0x48879158, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879158>); //   2.67%
	m_dsmap.SetAt(0x49078178, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078178>); //   3.52%
	m_dsmap.SetAt(0x49079178, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49079178>); //   5.43%
	m_dsmap.SetAt(0x4c804158, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c804158>); //   8.27%
	m_dsmap.SetAt(0x4c868868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c868868>); //   4.63%
	m_dsmap.SetAt(0x4d06b168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d06b168>); //   2.18%
	m_dsmap.SetAt(0x4d404058, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d404058>); //   3.92%

	// gt4

	m_dsmap.SetAt(0x1fe04057, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04057>); //   5.12%
	m_dsmap.SetAt(0x1fe1904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe1904c>); //  16.45%
	m_dsmap.SetAt(0x1fe1904d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe1904d>); //   8.23%
	m_dsmap.SetAt(0x1fe7804d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe7804d>); //  11.63%
	m_dsmap.SetAt(0x1fe7904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe7904c>); //   2.63%
	m_dsmap.SetAt(0x1fe7904d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe7904d>); //  11.11%
	m_dsmap.SetAt(0x1fe84964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe84964>); //  10.37%
	m_dsmap.SetAt(0x1fee8164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee8164>); //   6.19%
	m_dsmap.SetAt(0x1fee8965, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee8965>); //   4.34%
	m_dsmap.SetAt(0x4880484d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4880484d>); //   5.27%
	m_dsmap.SetAt(0x488181d4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488181d4>); //  18.53%
	m_dsmap.SetAt(0x4881904d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4881904d>); //  11.24%
	m_dsmap.SetAt(0x488191d4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488191d4>); //  21.08%
	m_dsmap.SetAt(0x4883904d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883904d>); //   7.99%
	m_dsmap.SetAt(0x4887804d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887804d>); //   7.82%
	m_dsmap.SetAt(0x4887904d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887904d>); //   7.13%
	m_dsmap.SetAt(0x48884064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48884064>); //  10.93%
	m_dsmap.SetAt(0x48884f64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48884f64>); // 139.38%
	m_dsmap.SetAt(0x488e804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e804c>); //   4.67%
	m_dsmap.SetAt(0x488e8065, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8065>); //  31.32%
	m_dsmap.SetAt(0x488e8864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8864>); //  19.46%
	m_dsmap.SetAt(0x488e8865, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8865>); //  39.81%
	m_dsmap.SetAt(0x488e8f64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8f64>); //   7.42%
	m_dsmap.SetAt(0x488e9764, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e9764>); //  31.10%
	m_dsmap.SetAt(0x488e9765, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e9765>); //   2.28%
	m_dsmap.SetAt(0x4b03804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b03804c>); //  26.88%
	m_dsmap.SetAt(0x4b1a8864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b1a8864>); //   7.62%
	m_dsmap.SetAt(0x4c80404d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c80404d>); //   2.26%
	m_dsmap.SetAt(0x4c81804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c81804c>); //  10.92%
	m_dsmap.SetAt(0x4c81904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c81904c>); //  24.91%
	m_dsmap.SetAt(0x4c81904d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c81904d>); //  10.52%
	m_dsmap.SetAt(0x4c83804d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c83804d>); //  14.22%
	m_dsmap.SetAt(0x4c83904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c83904c>); //  43.74%
	m_dsmap.SetAt(0x4c83904d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c83904d>); //  13.01%
	m_dsmap.SetAt(0x4d05804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d05804c>); //  23.62%
	m_dsmap.SetAt(0x5520404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5520404c>); //  12.94%
	m_dsmap.SetAt(0x5520410c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5520410c>); //  14.56%
	m_dsmap.SetAt(0x5fe04054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe04054>); //   2.54%
	m_dsmap.SetAt(0x5fe1904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe1904c>); //   7.84%
	m_dsmap.SetAt(0x5fe2884e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe2884e>); //   4.46%
	m_dsmap.SetAt(0x5fe3904e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe3904e>); //   4.42%
	m_dsmap.SetAt(0x5fe5804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe5804c>); //  12.23%
	m_dsmap.SetAt(0x5fe5904d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe5904d>); //   9.86%
	m_dsmap.SetAt(0x5fee8864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fee8864>); //   9.83%

	// katamary damacy

	m_dsmap.SetAt(0x488041d4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488041d4>); //  13.32%
	m_dsmap.SetAt(0x488041e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488041e4>); //  15.42%
	m_dsmap.SetAt(0x488181cc, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488181cc>); //   6.97%
	m_dsmap.SetAt(0x488301cd, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488301cd>); //  10.03%
	m_dsmap.SetAt(0x488301e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488301e4>); //   9.52%
	m_dsmap.SetAt(0x488581e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488581e4>); //   9.61%
	m_dsmap.SetAt(0x488591e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488591e4>); //   4.94%
	m_dsmap.SetAt(0x488691e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488691e4>); //   2.74%
	m_dsmap.SetAt(0x488e81e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e81e4>); //   2.96%
	m_dsmap.SetAt(0x488e89e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e89e4>); // 102.81%
	m_dsmap.SetAt(0x488e9064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e9064>); //   3.01%
	m_dsmap.SetAt(0x488e91d4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e91d4>); //  17.06%
	m_dsmap.SetAt(0x488e91e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e91e4>); //  29.50%
	m_dsmap.SetAt(0x48904054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48904054>); //  16.23%
	m_dsmap.SetAt(0x490681e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490681e4>); //   2.12%

	// grandia 3

	m_dsmap.SetAt(0x4127904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4127904c>); //   3.34%
	m_dsmap.SetAt(0x484e8860, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x484e8860>); //   2.43%
	m_dsmap.SetAt(0x48868360, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868360>); //   6.79%
	m_dsmap.SetAt(0x48868870, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868870>); //  43.72%
	m_dsmap.SetAt(0x48868b60, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868b60>); //   5.10%
	m_dsmap.SetAt(0x48868f60, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868f60>); //   8.97%
	m_dsmap.SetAt(0x48869360, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48869360>); //   5.45%
	m_dsmap.SetAt(0x48869760, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48869760>); //   5.31%
	m_dsmap.SetAt(0x488a8060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488a8060>); //  15.76%
	m_dsmap.SetAt(0x488a8860, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488a8860>); //  18.09%
	m_dsmap.SetAt(0x488e8360, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8360>); //  27.85%
	m_dsmap.SetAt(0x488e8870, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8870>); //   2.22%
	m_dsmap.SetAt(0x488e8b60, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8b60>); //  38.40%
	m_dsmap.SetAt(0x488e8f60, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8f60>); //  57.46%
	m_dsmap.SetAt(0x488e9360, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e9360>); //   9.74%
	m_dsmap.SetAt(0x488e9760, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e9760>); //   3.27%
	m_dsmap.SetAt(0x49078860, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078860>); //   4.65%
	m_dsmap.SetAt(0x490e8060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8060>); //   2.28%
	m_dsmap.SetAt(0x4c839060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c839060>); //  65.04%
	m_dsmap.SetAt(0x4c83964c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c83964c>); //  74.95%
	m_dsmap.SetAt(0x4c839660, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c839660>); //  74.27%
	m_dsmap.SetAt(0x50368060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x50368060>); //  13.58%
	m_dsmap.SetAt(0x41268060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x41268060>); //  19.62%
	m_dsmap.SetAt(0x41268070, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x41268070>); //  13.02%
	m_dsmap.SetAt(0x41268870, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x41268870>); //   8.36%
	m_dsmap.SetAt(0x48868760, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868760>); //  13.93%
	m_dsmap.SetAt(0x48878760, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878760>); //   8.65%
	m_dsmap.SetAt(0x48879760, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879760>); //  16.83%
	m_dsmap.SetAt(0x488f8f60, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488f8f60>); //  41.28%
	m_dsmap.SetAt(0x49068860, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068860>); //  11.27%
	m_dsmap.SetAt(0x49068870, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068870>); //   8.85%
	m_dsmap.SetAt(0x49078060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078060>); //   6.82%

	// rumble roses

	m_dsmap.SetAt(0x1fe78164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe78164>); //  26.31%
	m_dsmap.SetAt(0x1fe79164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe79164>); //   9.96%
	m_dsmap.SetAt(0x48804164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804164>); //   3.72%
	m_dsmap.SetAt(0x48804964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804964>); //   2.84%
	m_dsmap.SetAt(0x48838164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48838164>); //  16.00%
	m_dsmap.SetAt(0x48878164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878164>); //  13.15%
	m_dsmap.SetAt(0x48878964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878964>); //  10.21%
	m_dsmap.SetAt(0x4887b964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887b964>); //  35.06%
	m_dsmap.SetAt(0x490e8164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8164>); //   3.90%
	m_dsmap.SetAt(0x490e8964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8964>); //   4.73%
	m_dsmap.SetAt(0x4b0e8864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b0e8864>); //   3.54%
	m_dsmap.SetAt(0x4c48484c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c48484c>); //   2.49%
	m_dsmap.SetAt(0x4c830164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c830164>); //  35.21%
	m_dsmap.SetAt(0x4c8e8864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c8e8864>); //   3.87%
	m_dsmap.SetAt(0x55384874, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x55384874>); //   7.93%

	// dmc

	m_dsmap.SetAt(0x1fe39058, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe39058>); //   8.57%
	m_dsmap.SetAt(0x1fe68968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe68968>); //   3.81%
	m_dsmap.SetAt(0x1fe78158, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe78158>); //   2.93%
	m_dsmap.SetAt(0x1fea8968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fea8968>); //   2.95%
	m_dsmap.SetAt(0x1fee8978, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee8978>); //   2.32%
	m_dsmap.SetAt(0x45204078, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x45204078>); //   3.29%
	m_dsmap.SetAt(0x4520424c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4520424c>); //   9.45%
	m_dsmap.SetAt(0x48804078, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804078>); //   7.61%
	m_dsmap.SetAt(0x48830068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48830068>); //  11.67%
	m_dsmap.SetAt(0x48878168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878168>); // 100.91%
	m_dsmap.SetAt(0x490e8968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8968>); //   3.44%
	m_dsmap.SetAt(0x4c43804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c43804c>); //  19.54%
	m_dsmap.SetAt(0x4c839068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c839068>); //  13.91%
	m_dsmap.SetAt(0x4d068968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d068968>); //   3.21%
	m_dsmap.SetAt(0x54204078, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x54204078>); //   3.06%
	m_dsmap.SetAt(0x5420424c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5420424c>); //   9.69%
	m_dsmap.SetAt(0x4423904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4423904c>); //   8.89%
	m_dsmap.SetAt(0x4427904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4427904c>); //  28.32%
	m_dsmap.SetAt(0x4c87914c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c87914c>); //  45.10%
	m_dsmap.SetAt(0x4d07804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d07804c>); //   2.21%
	m_dsmap.SetAt(0x1fe20968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe20968>); //   2.76%
	m_dsmap.SetAt(0x48830868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48830868>); //   5.27%
	m_dsmap.SetAt(0x4c8e8968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c8e8968>); //   2.39%

	// xenosaga 2

	m_dsmap.SetAt(0x1fe0404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe0404c>); //   2.08%
	m_dsmap.SetAt(0x1fe58174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe58174>); //   4.50%
	m_dsmap.SetAt(0x1fe68864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe68864>); //  22.20%
	m_dsmap.SetAt(0x1fea8964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fea8964>); //   2.70%
	m_dsmap.SetAt(0x1fee804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee804c>); //  13.85%
	m_dsmap.SetAt(0x48468864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48468864>); //   5.27%
	m_dsmap.SetAt(0x48478164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48478164>); //   4.61%
	m_dsmap.SetAt(0x48804074, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804074>); //  14.26%
	m_dsmap.SetAt(0x488688e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488688e4>); //  10.57%
	m_dsmap.SetAt(0x488a88e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488a88e4>); //   2.18%
	m_dsmap.SetAt(0x488e80e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e80e4>); //   2.10%
	m_dsmap.SetAt(0x488e88e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e88e4>); //   4.96%
	m_dsmap.SetAt(0x4901004c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4901004c>); //   2.15%
	m_dsmap.SetAt(0x4905804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4905804c>); //   3.05%
	m_dsmap.SetAt(0x49059054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49059054>); //   4.00%
	m_dsmap.SetAt(0x49069064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49069064>); //   2.42%
	m_dsmap.SetAt(0x4947824c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4947824c>); //   3.04%
	m_dsmap.SetAt(0x4c404054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c404054>); //  14.22%
	m_dsmap.SetAt(0x4c804054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c804054>); //  14.22%
	m_dsmap.SetAt(0x4c839054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c839054>); //  24.70%
	m_dsmap.SetAt(0x4d004054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d004054>); //  14.09%
	m_dsmap.SetAt(0x4d038054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d038054>); //  30.21%
	m_dsmap.SetAt(0x4d039054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d039054>); //  24.82%
	m_dsmap.SetAt(0x4d068864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d068864>); //  14.72%
	m_dsmap.SetAt(0x4d069064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d069064>); //   5.63%
	m_dsmap.SetAt(0x51229064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x51229064>); //   8.77%

	// nfs mw

	m_dsmap.SetAt(0x1fe68168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe68168>); //  13.79%
	m_dsmap.SetAt(0x1fe68964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe68964>); //  95.57%
	m_dsmap.SetAt(0x1fe6896a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe6896a>); //   3.34%
	m_dsmap.SetAt(0x4805904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4805904c>); //  13.79%
	m_dsmap.SetAt(0x4883814e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883814e>); //   6.64%
	m_dsmap.SetAt(0x48868168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868168>); //   4.64%
	m_dsmap.SetAt(0x4886816a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4886816a>); //   3.61%
	m_dsmap.SetAt(0x49078164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078164>); //  19.92%
	m_dsmap.SetAt(0x49078964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078964>); //  10.68%
	m_dsmap.SetAt(0x4927904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4927904c>); //  19.88%
	m_dsmap.SetAt(0x4b004064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b004064>); //   9.18%
	m_dsmap.SetAt(0x4b004068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b004068>); //  22.54%
	m_dsmap.SetAt(0x4b004864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b004864>); //  12.87%
	m_dsmap.SetAt(0x4b004868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b004868>); //  26.79%
	m_dsmap.SetAt(0x4b028064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b028064>); //   8.86%
	m_dsmap.SetAt(0x4b028068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b028068>); //  20.85%
	m_dsmap.SetAt(0x4b028864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b028864>); //   8.67%
	m_dsmap.SetAt(0x4b028868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b028868>); //  17.21%
	m_dsmap.SetAt(0x4b029064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b029064>); //   4.71%
	m_dsmap.SetAt(0x4b029068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b029068>); //  26.95%
	m_dsmap.SetAt(0x4b038164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b038164>); //   9.93%
	m_dsmap.SetAt(0x4b038964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b038964>); //   4.37%
	m_dsmap.SetAt(0x4c83804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c83804c>); //  20.46%
	m_dsmap.SetAt(0x4c83804e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c83804e>); //   5.63%
	m_dsmap.SetAt(0x5127904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5127904c>); //  18.99%
	m_dsmap.SetAt(0x5fe19064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe19064>); //  24.57%
	m_dsmap.SetAt(0x1fe6816a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe6816a>); //   8.86%
	m_dsmap.SetAt(0x4906816a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4906816a>); //   2.86%
	m_dsmap.SetAt(0x4906896a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4906896a>); //   2.99%
	m_dsmap.SetAt(0x49079164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49079164>); //   3.93%
	m_dsmap.SetAt(0x4a838964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a838964>); //   3.66%

	// berserk

	m_dsmap.SetAt(0x4c8e8b64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c8e8b64>); //  38.62%
	m_dsmap.SetAt(0x4c8fb964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c8fb964>); //  13.39%

	// castlevania

	m_dsmap.SetAt(0x1fe1004c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe1004c>); //  10.85%
	m_dsmap.SetAt(0x1fe3104d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3104d>); //  12.56%
	m_dsmap.SetAt(0x1fe3104e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3104e>); //  12.59%
	m_dsmap.SetAt(0x1fe5904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe5904c>); //  11.64%
	m_dsmap.SetAt(0x1fe78868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe78868>); //  23.47%
	m_dsmap.SetAt(0x48838058, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48838058>); //  22.09%
	m_dsmap.SetAt(0x48878868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878868>); // 101.01%
	m_dsmap.SetAt(0x48884168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48884168>); //   9.60%
	m_dsmap.SetAt(0x488c8968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488c8968>); //   8.74%
	m_dsmap.SetAt(0x488eb968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488eb968>); //   2.58%
	m_dsmap.SetAt(0x49078868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078868>); //  73.77%
	m_dsmap.SetAt(0x4c93904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c93904c>); //  15.49%
	m_dsmap.SetAt(0x4d00407a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d00407a>); //  22.47%

	// okami

	m_dsmap.SetAt(0x1fe18058, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe18058>); //  13.85%
	m_dsmap.SetAt(0x45218058, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x45218058>); //  34.00%
	m_dsmap.SetAt(0x48839158, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48839158>); //  25.00%
	m_dsmap.SetAt(0x48878058, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878058>); //  49.93%
	m_dsmap.SetAt(0x48879058, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879058>); //  11.51%
	m_dsmap.SetAt(0x488e8168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8168>); //  17.61%
	m_dsmap.SetAt(0x488f8168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488f8168>); //   2.56%
	m_dsmap.SetAt(0x4a83804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a83804c>); //  19.67%
	m_dsmap.SetAt(0x4c43904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c43904c>); //  91.95%
	m_dsmap.SetAt(0x5fe59068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe59068>); //  78.68%
	m_dsmap.SetAt(0x5fe7104c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe7104c>); //  17.56%

	// bully

	m_dsmap.SetAt(0x1fe04077, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04077>); //   9.73%
	m_dsmap.SetAt(0x1fe04864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04864>); //   7.26%
	m_dsmap.SetAt(0x48804864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804864>); //  10.05%
	m_dsmap.SetAt(0x4d03104c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d03104c>); //  44.70%
	m_dsmap.SetAt(0x4d068364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d068364>); //  17.88%
	m_dsmap.SetAt(0x4d068b64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d068b64>); //  34.20%
	m_dsmap.SetAt(0x4d07834c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d07834c>); //  15.04%
	m_dsmap.SetAt(0x510e8964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x510e8964>); // 145.14%
	m_dsmap.SetAt(0x5480404d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5480404d>); //  13.76%
	m_dsmap.SetAt(0x5501904e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5501904e>); //  26.34%
	m_dsmap.SetAt(0x4d078364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d078364>); //   3.95%
	m_dsmap.SetAt(0x510e8164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x510e8164>); //   2.53%

	// culdcept

	m_dsmap.SetAt(0x1fe04056, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04056>); //   4.52%
	m_dsmap.SetAt(0x1fe049e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe049e6>); //  12.56%
	m_dsmap.SetAt(0x1fe181e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe181e6>); //   2.99%
	m_dsmap.SetAt(0x1fe191e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe191e6>); //   5.99%
	m_dsmap.SetAt(0x1fe2a1e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe2a1e6>); //  14.05%
	m_dsmap.SetAt(0x1fe2a9e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe2a9e6>); //  19.56%
	m_dsmap.SetAt(0x1fe31066, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe31066>); //  30.02%
	m_dsmap.SetAt(0x1fe391e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe391e6>); //  31.11%
	m_dsmap.SetAt(0x1fe3a1e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3a1e6>); //  14.99%
	m_dsmap.SetAt(0x1fe581e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe581e6>); //   3.09%
	m_dsmap.SetAt(0x1fe591e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe591e6>); //   8.42%
	m_dsmap.SetAt(0x1fe991e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe991e6>); //   4.33%
	m_dsmap.SetAt(0x1feb91e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1feb91e6>); //  32.00%
	m_dsmap.SetAt(0x1fed91e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fed91e6>); //   9.42%
	m_dsmap.SetAt(0x488089e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488089e6>); //   4.87%
	m_dsmap.SetAt(0x488181e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488181e6>); //   6.56%
	m_dsmap.SetAt(0x488281e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488281e6>); //   2.17%
	m_dsmap.SetAt(0x488289e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488289e6>); //   7.07%
	m_dsmap.SetAt(0x488291e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488291e6>); //   3.84%
	m_dsmap.SetAt(0x488581e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488581e6>); //   6.96%
	m_dsmap.SetAt(0x488591e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488591e6>); //   2.98%
	m_dsmap.SetAt(0x490049e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490049e6>); //   8.71%
	m_dsmap.SetAt(0x4d0049e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d0049e6>); //   3.68%
	m_dsmap.SetAt(0x4d0181e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d0181e6>); //  14.14%
	m_dsmap.SetAt(0x4d0191e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d0191e6>); //   2.29%
	m_dsmap.SetAt(0x4d02a1e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d02a1e6>); //   8.65%
	m_dsmap.SetAt(0x4d0381e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d0381e6>); //   4.84%
	m_dsmap.SetAt(0x4d0391e6, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d0391e6>); //  17.85%

	// suikoden 5

	m_dsmap.SetAt(0x40428868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x40428868>); //  11.26%
	m_dsmap.SetAt(0x4846834c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4846834c>); //  27.05%
	m_dsmap.SetAt(0x4847834c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4847834c>); //  25.31%
	m_dsmap.SetAt(0x48829368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48829368>); //  14.92%
	m_dsmap.SetAt(0x4883934c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883934c>); //   2.92%
	m_dsmap.SetAt(0x48858b68, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48858b68>); //   6.03%
	m_dsmap.SetAt(0x488a8b68, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488a8b68>); //  11.15%
	m_dsmap.SetAt(0x49028868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49028868>); //  10.91%
	m_dsmap.SetAt(0x4d068868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d068868>); //  33.68%

	// dq8

	m_dsmap.SetAt(0x1fe0484c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe0484c>); //   3.23%
	m_dsmap.SetAt(0x1fe3914c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3914c>); //   2.27%
	m_dsmap.SetAt(0x48404164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48404164>); //  66.92%
	m_dsmap.SetAt(0x4883914c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883914c>); //  12.11%
	m_dsmap.SetAt(0x48c3804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48c3804c>); //  32.84%
	m_dsmap.SetAt(0x49004164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004164>); //  67.02%
	m_dsmap.SetAt(0x490a8164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490a8164>); //   9.60%
	m_dsmap.SetAt(0x490a8964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490a8964>); //   5.10%
	m_dsmap.SetAt(0x490e914c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e914c>); //  25.20%
	m_dsmap.SetAt(0x490f914c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490f914c>); //  19.10%
	m_dsmap.SetAt(0x4c83914c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c83914c>); //   3.48%
	m_dsmap.SetAt(0x5103804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5103804c>); //   3.03%
	m_dsmap.SetAt(0x5103b04c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5103b04c>); //   5.19%

	// resident evil 4

	m_dsmap.SetAt(0x1fe18064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe18064>); //  13.84%
	m_dsmap.SetAt(0x4903904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4903904c>); //  19.51%
	m_dsmap.SetAt(0x4b068164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b068164>); //   6.54%
	m_dsmap.SetAt(0x4c879164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c879164>); //  26.76%
	m_dsmap.SetAt(0x4d07814c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d07814c>); //   9.53%
	m_dsmap.SetAt(0x5120404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5120404c>); //   2.51%
	m_dsmap.SetAt(0x5483904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5483904c>); //   4.94%
	m_dsmap.SetAt(0x4883064c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883064c>); //  39.22%
	m_dsmap.SetAt(0x4903814c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4903814c>); //  12.20%
	m_dsmap.SetAt(0x5fe68964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe68964>); //   5.03%

	// tomoyo after 

	m_dsmap.SetAt(0x1fe38059, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe38059>); //  25.29%
	m_dsmap.SetAt(0x1fe39059, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe39059>); //  24.40%
	m_dsmap.SetAt(0x48478068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48478068>); //   9.36%
	m_dsmap.SetAt(0x48818068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48818068>); //  28.04%
	m_dsmap.SetAt(0x48878068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878068>); //  27.11%
	m_dsmap.SetAt(0x49058068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49058068>); //  19.28%
	m_dsmap.SetAt(0x4a858068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a858068>); //  19.13%

	// .hack redemption

	m_dsmap.SetAt(0x1fe1804d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe1804d>); //   3.91%
	m_dsmap.SetAt(0x1fe1914c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe1914c>); //   6.85%
	m_dsmap.SetAt(0x4123004c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4123004c>); //  23.69%
	m_dsmap.SetAt(0x48404074, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48404074>); //   9.77%
	m_dsmap.SetAt(0x48469064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48469064>); //  25.50%
	m_dsmap.SetAt(0x488f8364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488f8364>); //   3.84%
	m_dsmap.SetAt(0x488f9364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488f9364>); //   8.26%
	m_dsmap.SetAt(0x49004074, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004074>); //   9.71%
	m_dsmap.SetAt(0x49004864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004864>); //   8.46%
	m_dsmap.SetAt(0x4c41804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c41804c>); //  16.44%
	m_dsmap.SetAt(0x5fe1004c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe1004c>); //  14.89%
	m_dsmap.SetAt(0x1fe04b64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04b64>); //   7.80%
	m_dsmap.SetAt(0x48869364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48869364>); //  30.82%
	m_dsmap.SetAt(0x4d00404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d00404c>); //  10.07%

	// wild arms 5

	m_dsmap.SetAt(0x1fe19050, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe19050>); //   3.93%
	m_dsmap.SetAt(0x4845804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4845804c>); //  17.60%
	m_dsmap.SetAt(0x4845904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4845904c>); //  15.13%
	m_dsmap.SetAt(0x48804854, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804854>); //  17.14%
	m_dsmap.SetAt(0x4885884c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4885884c>); //  15.74%
	m_dsmap.SetAt(0x488e8764, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8764>); //  29.30%
	m_dsmap.SetAt(0x48c68864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48c68864>); //   9.65%
	m_dsmap.SetAt(0x49068964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068964>); //  10.47%
	m_dsmap.SetAt(0x4b068864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b068864>); //   2.18%
	m_dsmap.SetAt(0x4d068064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d068064>); //   3.54%
	m_dsmap.SetAt(0x5fe04764, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe04764>); //   4.41%
	m_dsmap.SetAt(0x5fe39054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe39054>); //  24.84%

	// shadow of the colossus

	m_dsmap.SetAt(0x1fe04064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04064>); //   8.04%
	m_dsmap.SetAt(0x1fe1004d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe1004d>); //  12.55%
	m_dsmap.SetAt(0x48868364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868364>); //   8.22%
	m_dsmap.SetAt(0x48868b24, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868b24>); //  16.58%
	m_dsmap.SetAt(0x488e8264, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8264>); //   4.32%
	m_dsmap.SetAt(0x48938064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48938064>); //  46.18%
	m_dsmap.SetAt(0x48939064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48939064>); //  21.41%
	m_dsmap.SetAt(0x49004064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004064>); //  50.31%
	m_dsmap.SetAt(0x490e8864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8864>); //  14.62%
	m_dsmap.SetAt(0x4d004064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d004064>); //  89.65%
	m_dsmap.SetAt(0x488e9364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e9364>); //   4.77%
	m_dsmap.SetAt(0x4c030064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c030064>); //  31.06%

	// tales of redemption

	m_dsmap.SetAt(0x48404254, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48404254>); //  10.04%
	m_dsmap.SetAt(0x48478254, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48478254>); //  12.51%
	m_dsmap.SetAt(0x48878b64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878b64>); //   8.94%
	m_dsmap.SetAt(0x48879374, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879374>); //   3.04%
	m_dsmap.SetAt(0x488b9054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488b9054>); //   6.91%
	m_dsmap.SetAt(0x49078264, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078264>); //   5.61%
	m_dsmap.SetAt(0x4c838064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c838064>); //  14.59%
	m_dsmap.SetAt(0x4c838854, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c838854>); //  14.37%

	// digital devil saga

	m_dsmap.SetAt(0x1fe39070, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe39070>); //   6.48%
	m_dsmap.SetAt(0x40204250, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x40204250>); //   5.59%
	m_dsmap.SetAt(0x48404050, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48404050>); //   6.08%
	m_dsmap.SetAt(0x484e9070, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x484e9070>); //   7.79%
	m_dsmap.SetAt(0x48804060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804060>); //   3.20%
	m_dsmap.SetAt(0x48804150, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804150>); //   3.11%
	m_dsmap.SetAt(0x48868b70, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868b70>); //   6.24%
	m_dsmap.SetAt(0x48878060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878060>); //   6.39%
	m_dsmap.SetAt(0x48878150, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878150>); //   9.31%
	m_dsmap.SetAt(0x48878360, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878360>); //   3.31%
	m_dsmap.SetAt(0x48879360, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879360>); //  14.19%
	m_dsmap.SetAt(0x48884870, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48884870>); //  19.31%
	m_dsmap.SetAt(0x488e8860, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8860>); //   6.53%
	m_dsmap.SetAt(0x48904270, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48904270>); //  27.83%
	m_dsmap.SetAt(0x49068070, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068070>); //   2.46%
	m_dsmap.SetAt(0x49078360, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078360>); //  15.46%
	m_dsmap.SetAt(0x49079060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49079060>); //   2.16%
	m_dsmap.SetAt(0x49079360, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49079360>); //   9.74%
	m_dsmap.SetAt(0x490e8860, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8860>); //   7.56%
	m_dsmap.SetAt(0x490e8870, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8870>); //   5.96%
	m_dsmap.SetAt(0x4a878060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a878060>); //   7.13%
	m_dsmap.SetAt(0x48804b60, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804b60>); //   2.22%
	m_dsmap.SetAt(0x49078070, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078070>); //   5.87%

	// dbzbt2

	m_dsmap.SetAt(0x48878674, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878674>); //  74.94%
	m_dsmap.SetAt(0x4887970c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887970c>); //   3.61%
	m_dsmap.SetAt(0x4906804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4906804c>); //  29.50%
	m_dsmap.SetAt(0x4906884c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4906884c>); //   9.09%
	m_dsmap.SetAt(0x49078054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078054>); //  29.33%
	m_dsmap.SetAt(0x49079054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49079054>); //   6.42%
	m_dsmap.SetAt(0x4c904064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c904064>); //   7.49%
	m_dsmap.SetAt(0x543081e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x543081e4>); //   5.74%
	m_dsmap.SetAt(0x543089e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x543089e4>); //   4.18%

	// dbzbt3

	m_dsmap.SetAt(0x4883104c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883104c>); //   4.75%
	m_dsmap.SetAt(0x488391cc, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488391cc>); //  12.37%
	m_dsmap.SetAt(0x4885904e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4885904e>); //  15.28%
	m_dsmap.SetAt(0x48859074, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48859074>); //  14.47%
	m_dsmap.SetAt(0x48868864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868864>); //   5.07%
	m_dsmap.SetAt(0x48868964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868964>); //  39.07%
	m_dsmap.SetAt(0x489081e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x489081e4>); //   7.98%
	m_dsmap.SetAt(0x48968864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48968864>); //  13.31%
	m_dsmap.SetAt(0x4905904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4905904c>); //  28.18%
	m_dsmap.SetAt(0x4a40404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a40404c>); //   9.29%
	m_dsmap.SetAt(0x4a879164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a879164>); //   4.04%
	m_dsmap.SetAt(0x4c45904e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c45904e>); //  30.87%
	m_dsmap.SetAt(0x4c469064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c469064>); //   8.57%
	m_dsmap.SetAt(0x4c80404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c80404c>); //  10.03%
	m_dsmap.SetAt(0x4c80404e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c80404e>); //  14.23%
	m_dsmap.SetAt(0x4c83004c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c83004c>); //  19.65%
	m_dsmap.SetAt(0x4c8391cc, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c8391cc>); //  24.75%
	m_dsmap.SetAt(0x4c869064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c869064>); //   9.50%
	m_dsmap.SetAt(0x4d03004c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d03004c>); //  36.68%
	m_dsmap.SetAt(0x5fe1904e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe1904e>); //  16.66%
	m_dsmap.SetAt(0x1fe3004c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3004c>); //   3.94%
	m_dsmap.SetAt(0x48478064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48478064>); //  20.73%
	m_dsmap.SetAt(0x48868164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868164>); //   8.42%
	m_dsmap.SetAt(0x48878864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878864>); //  13.89%
	m_dsmap.SetAt(0x49068064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068064>); //  10.95%
	m_dsmap.SetAt(0x49078864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078864>); //  15.01%
	m_dsmap.SetAt(0x4910404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4910404c>); //   4.15%
	m_dsmap.SetAt(0x4917804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4917804c>); //   5.33%
	m_dsmap.SetAt(0x4917904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4917904c>); //   3.66%
	m_dsmap.SetAt(0x49268064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49268064>); //  13.08%
	m_dsmap.SetAt(0x4887974c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887974c>); //   2.85%
	m_dsmap.SetAt(0x4897904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4897904c>); //   2.15%

	// disgaea 2

	m_dsmap.SetAt(0x1fe04164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04164>); //   7.12%
	m_dsmap.SetAt(0x1fe69174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe69174>); //  13.13%
	m_dsmap.SetAt(0x48820b64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48820b64>); //  16.33%
	m_dsmap.SetAt(0x48830064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48830064>); //  15.18%
	m_dsmap.SetAt(0x48869164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48869164>); //  12.42%
	m_dsmap.SetAt(0x48879164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879164>); //   8.40%

	// gradius 5

	m_dsmap.SetAt(0x4881814c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4881814c>); //  22.73%
	m_dsmap.SetAt(0x4885814c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4885814c>); //  21.84%
	m_dsmap.SetAt(0x48868968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868968>); //  21.44%
	m_dsmap.SetAt(0x48878968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878968>); //   7.33%
	m_dsmap.SetAt(0x49004968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004968>); //   2.44%
	m_dsmap.SetAt(0x49068168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068168>); //  37.23%
	m_dsmap.SetAt(0x49068968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068968>); //   4.96%
	m_dsmap.SetAt(0x49078168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078168>); //   5.55%
	m_dsmap.SetAt(0x49078968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078968>); //   4.92%
	m_dsmap.SetAt(0x5fe3814c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe3814c>); //  22.69%
	m_dsmap.SetAt(0x5fe68168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe68168>); //  31.44%
	m_dsmap.SetAt(0x5fe68968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe68968>); //   6.92%
	m_dsmap.SetAt(0x5fee8168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fee8168>); //   3.37%
	m_dsmap.SetAt(0x5fee8968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fee8968>); //  47.48%
	m_dsmap.SetAt(0x5ffe8968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5ffe8968>); //   6.57%

	// tales of abyss

	m_dsmap.SetAt(0x1fe04068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04068>); //   9.99%
	m_dsmap.SetAt(0x1fe390cc, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe390cc>); //   3.89%
	m_dsmap.SetAt(0x1fe3934c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3934c>); //   2.45%
	m_dsmap.SetAt(0x1fe39368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe39368>); //   8.12%
	m_dsmap.SetAt(0x41200b4c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x41200b4c>); //  17.03%
	m_dsmap.SetAt(0x4121034c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4121034c>); //  27.27%
	m_dsmap.SetAt(0x41210b4c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x41210b4c>); //  15.56%
	m_dsmap.SetAt(0x48848b68, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48848b68>); //   4.27%
	m_dsmap.SetAt(0x4885834c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4885834c>); //  15.82%
	m_dsmap.SetAt(0x48858368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48858368>); //   3.02%
	m_dsmap.SetAt(0x4886834c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4886834c>); //   4.93%
	m_dsmap.SetAt(0x48868368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868368>); //  12.08%
	m_dsmap.SetAt(0x48868b4c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868b4c>); //   4.99%
	m_dsmap.SetAt(0x4886934c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4886934c>); //   6.19%
	m_dsmap.SetAt(0x4887834c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887834c>); //   8.58%
	m_dsmap.SetAt(0x4887934c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887934c>); //   5.71%
	m_dsmap.SetAt(0x488c8368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488c8368>); //   2.61%
	m_dsmap.SetAt(0x488c8b68, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488c8b68>); //  21.36%
	m_dsmap.SetAt(0x48cf89e8, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48cf89e8>); //  53.45%
	m_dsmap.SetAt(0x4903834c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4903834c>); //  20.14%
	m_dsmap.SetAt(0x4906834c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4906834c>); //  10.33%
	m_dsmap.SetAt(0x49068368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068368>); //   7.60%
	m_dsmap.SetAt(0x49068b4c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068b4c>); //  14.29%
	m_dsmap.SetAt(0x490c8b68, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490c8b68>); //   5.78%
	m_dsmap.SetAt(0x490e8b68, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8b68>); //   4.01%
	m_dsmap.SetAt(0x490f89e8, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490f89e8>); //  56.65%
	m_dsmap.SetAt(0x4a83904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a83904c>); //  32.12%
	m_dsmap.SetAt(0x4d03914c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d03914c>); //  18.80%
	m_dsmap.SetAt(0x5fe59078, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe59078>); //  23.76%

	// Gundam Seed Destiny OMNI VS ZAFT II PLUS 

	m_dsmap.SetAt(0x1fe04255, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04255>); //   3.77%
	m_dsmap.SetAt(0x1fe19375, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe19375>); //  18.91%
	m_dsmap.SetAt(0x1fe68375, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe68375>); //  31.83%
	m_dsmap.SetAt(0x1fee8b75, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee8b75>); //  20.67%
	m_dsmap.SetAt(0x48818375, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48818375>); //  33.45%
	m_dsmap.SetAt(0x48819375, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48819375>); //  22.73%
	m_dsmap.SetAt(0x4886884d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4886884d>); //   2.10%
	m_dsmap.SetAt(0x48868b75, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868b75>); //  23.40%
	m_dsmap.SetAt(0x48878375, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878375>); //  16.21%
	m_dsmap.SetAt(0x4887884d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887884d>); //  16.31%
	m_dsmap.SetAt(0x48878b75, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878b75>); //  35.41%
	m_dsmap.SetAt(0x488e8375, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8375>); //  41.70%
	m_dsmap.SetAt(0x488e8b75, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8b75>); //  61.10%
	m_dsmap.SetAt(0x488e9375, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e9375>); //   2.06%
	m_dsmap.SetAt(0x49068375, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068375>); //  34.32%
	m_dsmap.SetAt(0x4906884d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4906884d>); //   6.31%
	m_dsmap.SetAt(0x490e8375, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8375>); //  25.96%
	m_dsmap.SetAt(0x490e8b75, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8b75>); //   9.70%

	// nba 2k8

	m_dsmap.SetAt(0x1fe04856, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04856>); //  43.04%
	m_dsmap.SetAt(0x1fe38966, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe38966>); //  27.76%
	m_dsmap.SetAt(0x1fe39156, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe39156>); //  27.86%
	m_dsmap.SetAt(0x1fe60966, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe60966>); //   6.48%
	m_dsmap.SetAt(0x1fe68966, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe68966>); //   7.65%
	m_dsmap.SetAt(0x1fe79156, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe79156>); //  31.61%
	m_dsmap.SetAt(0x4883804e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883804e>); //  26.11%
	m_dsmap.SetAt(0x48838166, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48838166>); //   7.20%
	m_dsmap.SetAt(0x48868166, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868166>); //  16.60%
	m_dsmap.SetAt(0x48868966, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868966>); //  55.21%
	m_dsmap.SetAt(0x48878166, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878166>); //   3.20%
	m_dsmap.SetAt(0x48879166, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879166>); //  17.34%
	m_dsmap.SetAt(0x49028966, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49028966>); //   6.79%
	m_dsmap.SetAt(0x5fe69166, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe69166>); //  13.11%
	m_dsmap.SetAt(0x5fe79166, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe79166>); //  30.25%
	m_dsmap.SetAt(0x1fe28956, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe28956>); //  27.21%
	m_dsmap.SetAt(0x48828966, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48828966>); //   8.93%
	m_dsmap.SetAt(0x5fe28966, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe28966>); //   2.79%
	m_dsmap.SetAt(0x5fe68166, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe68166>); //   2.34%
	m_dsmap.SetAt(0x5fe68966, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe68966>); //   2.29%

	// onimusha 3

	m_dsmap.SetAt(0x1fe18068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe18068>); //   6.42%
	m_dsmap.SetAt(0x1fe1904e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe1904e>); //   3.02%
	m_dsmap.SetAt(0x1fe3904e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3904e>); //   5.35%
	m_dsmap.SetAt(0x1fee014c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee014c>); //   4.64%
	m_dsmap.SetAt(0x1fee0b68, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee0b68>); //  41.49%
	m_dsmap.SetAt(0x1fee814c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee814c>); //   2.13%
	m_dsmap.SetAt(0x48268368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48268368>); //   3.69%
	m_dsmap.SetAt(0x48839168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48839168>); //   7.67%
	m_dsmap.SetAt(0x48c28368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48c28368>); //   6.75%
	m_dsmap.SetAt(0x4903884c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4903884c>); //  34.63%
	m_dsmap.SetAt(0x49078368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078368>); //   2.76%
	m_dsmap.SetAt(0x49278368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49278368>); //   3.22%
	m_dsmap.SetAt(0x4c804168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c804168>); //   6.85%
	m_dsmap.SetAt(0x4c804368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c804368>); //   5.05%
	m_dsmap.SetAt(0x4c878168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c878168>); //  13.59%
	m_dsmap.SetAt(0x4d004068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d004068>); //  33.75%
	m_dsmap.SetAt(0x4d03804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d03804c>); //   9.48%
	m_dsmap.SetAt(0x4d03904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d03904c>); //  10.16%
	m_dsmap.SetAt(0x4d05884c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d05884c>); //   7.73%
	m_dsmap.SetAt(0x5125904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5125904c>); //   6.73%
	m_dsmap.SetAt(0x5425904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5425904c>); //   7.59%
	m_dsmap.SetAt(0x5fe04078, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe04078>); //  32.30%
	m_dsmap.SetAt(0x5fe5904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe5904c>); //   6.22%
	m_dsmap.SetAt(0x5fe78368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe78368>); //   5.59%
	m_dsmap.SetAt(0x5fe7904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe7904c>); //   9.38%
	m_dsmap.SetAt(0x5fe79368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe79368>); //  11.34%
	m_dsmap.SetAt(0x5ff0404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5ff0404c>); //   2.92%
	m_dsmap.SetAt(0x5ff1804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5ff1804c>); //   4.63%

	// resident evil code veronica

	m_dsmap.SetAt(0x1fe04968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04968>); //   3.22%
	m_dsmap.SetAt(0x1fe31068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe31068>); //  23.55%
	m_dsmap.SetAt(0x1fe78168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe78168>); //  29.94%
	m_dsmap.SetAt(0x1fee8168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee8168>); //  10.74%
	m_dsmap.SetAt(0x1fee8968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee8968>); //  81.32%
	m_dsmap.SetAt(0x48878368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878368>); //  26.64%
	m_dsmap.SetAt(0x48878b68, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878b68>); //   5.40%
	m_dsmap.SetAt(0x488e88e8, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e88e8>); //   2.16%
	m_dsmap.SetAt(0x488e8968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8968>); //   4.17%
	m_dsmap.SetAt(0x488f9368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488f9368>); //   5.06%
	m_dsmap.SetAt(0x4c818058, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c818058>); //  25.60%
	m_dsmap.SetAt(0x4c819058, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c819058>); //  23.11%

	// armored core 3

	m_dsmap.SetAt(0x1fe0404e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe0404e>); //   2.34%
	m_dsmap.SetAt(0x1fe04074, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04074>); //   9.77%
	m_dsmap.SetAt(0x1fe041cc, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe041cc>); //   9.45%
	m_dsmap.SetAt(0x1fe841f4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe841f4>); //   4.54%
	m_dsmap.SetAt(0x1fee01f4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee01f4>); //   2.94%
	m_dsmap.SetAt(0x1fee09f4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee09f4>); //  53.50%
	m_dsmap.SetAt(0x1fee89f4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee89f4>); //   3.51%
	m_dsmap.SetAt(0x4840404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4840404c>); //   5.70%
	m_dsmap.SetAt(0x48404054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48404054>); //   8.32%
	m_dsmap.SetAt(0x484049d4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x484049d4>); //   6.21%
	m_dsmap.SetAt(0x484e81f4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x484e81f4>); //  12.40%
	m_dsmap.SetAt(0x488501d4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488501d4>); //   8.67%
	m_dsmap.SetAt(0x488581d4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488581d4>); //   6.37%
	m_dsmap.SetAt(0x488591d4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488591d4>); //   2.45%
	m_dsmap.SetAt(0x48868874, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868874>); //  13.58%
	m_dsmap.SetAt(0x48868b64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868b64>); //   3.30%
	m_dsmap.SetAt(0x48878074, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878074>); //   8.21%
	m_dsmap.SetAt(0x48878874, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878874>); //  13.60%
	m_dsmap.SetAt(0x488791d4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488791d4>); //   3.68%
	m_dsmap.SetAt(0x488e09f4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e09f4>); //  12.28%
	m_dsmap.SetAt(0x488e81f4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e81f4>); //  18.73%
	m_dsmap.SetAt(0x488e89f4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e89f4>); //   4.59%
	m_dsmap.SetAt(0x490591d4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490591d4>); //   3.76%
	m_dsmap.SetAt(0x49078074, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078074>); //   2.83%
	m_dsmap.SetAt(0x49078874, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49078874>); //   2.73%
	m_dsmap.SetAt(0x490e81f4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e81f4>); //  57.03%
	m_dsmap.SetAt(0x4c429064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c429064>); //  37.51%
	m_dsmap.SetAt(0x4c4e81f4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c4e81f4>); //   8.97%
	m_dsmap.SetAt(0x4c4e89f4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c4e89f4>); //   8.70%
	m_dsmap.SetAt(0x4d0b00e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d0b00e4>); //   3.44%
	m_dsmap.SetAt(0x4d0b08e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d0b08e4>); //   5.86%
	m_dsmap.SetAt(0x4d0e81f4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d0e81f4>); //   7.54%
	m_dsmap.SetAt(0x510e89f4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x510e89f4>); //   2.80%

	// aerial planet

	m_dsmap.SetAt(0x4820404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4820404c>); //  12.56%
	m_dsmap.SetAt(0x4847914c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4847914c>); //   5.94%
	m_dsmap.SetAt(0x4886894c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4886894c>); //  34.16%
	m_dsmap.SetAt(0x48878974, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878974>); //  11.98%
	m_dsmap.SetAt(0x488e814c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e814c>); //   4.01%
	m_dsmap.SetAt(0x488e894c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e894c>); //   4.17%
	m_dsmap.SetAt(0x4c804064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c804064>); //  45.01%
	m_dsmap.SetAt(0x4c80414c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c80414c>); //  12.32%
	m_dsmap.SetAt(0x4c86804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c86804c>); //  28.43%
	m_dsmap.SetAt(0x4c8681f4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c8681f4>); //  41.12%
	m_dsmap.SetAt(0x4c8e804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c8e804c>); //  15.77%
	m_dsmap.SetAt(0x4c8e8174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c8e8174>); //   5.75%
	m_dsmap.SetAt(0x4c8e81f4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c8e81f4>); //  10.36%
	m_dsmap.SetAt(0x4c8e8964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c8e8964>); //   3.19%
	m_dsmap.SetAt(0x4c8e8974, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c8e8974>); //  32.28%
	m_dsmap.SetAt(0x4c93814c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c93814c>); //  26.21%
	m_dsmap.SetAt(0x4cc0414c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4cc0414c>); //  13.24%
	m_dsmap.SetAt(0x4d068174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d068174>); //  10.90%
	m_dsmap.SetAt(0x4d10404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d10404c>); //  10.01%
	m_dsmap.SetAt(0x4d404054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d404054>); //   4.44%

	// one piece grand battle 3

	m_dsmap.SetAt(0x48839054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48839054>); //  27.20%
	m_dsmap.SetAt(0x4886914c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4886914c>); //   3.97%
	m_dsmap.SetAt(0x49068874, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068874>); //  28.98%
	m_dsmap.SetAt(0x49069174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49069174>); //   2.54%
	m_dsmap.SetAt(0x4ac0404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4ac0404c>); //   9.02%
	m_dsmap.SetAt(0x4c41904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c41904c>); //  12.41%
	m_dsmap.SetAt(0x4c4190cc, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c4190cc>); //   7.08%
	m_dsmap.SetAt(0x5321914c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5321914c>); //   7.12%
	m_dsmap.SetAt(0x5fe79054, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe79054>); //   2.17%

	// one piece grand adventure

	m_dsmap.SetAt(0x1fe0414c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe0414c>); //   2.25%
	m_dsmap.SetAt(0x1fe68064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe68064>); //   7.07%
	m_dsmap.SetAt(0x4421814c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4421814c>); //   8.39%
	m_dsmap.SetAt(0x4843b04c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4843b04c>); //  19.21%
	m_dsmap.SetAt(0x4881104c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4881104c>); //  15.21%
	m_dsmap.SetAt(0x4881184c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4881184c>); //  14.82%
	m_dsmap.SetAt(0x48849164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48849164>); //  24.94%
	m_dsmap.SetAt(0x48869154, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48869154>); //   4.70%
	m_dsmap.SetAt(0x48878154, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878154>); //   8.39%
	m_dsmap.SetAt(0x48879154, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879154>); //   6.80%

	// shadow hearts

	m_dsmap.SetAt(0x1fe6904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe6904c>); //   5.24%
	m_dsmap.SetAt(0x48459058, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48459058>); //   3.35%
	m_dsmap.SetAt(0x4847804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4847804c>); //   5.60%
	m_dsmap.SetAt(0x48479068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48479068>); //  21.30%
	m_dsmap.SetAt(0x4881104e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4881104e>); //   5.30%
	m_dsmap.SetAt(0x48819168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48819168>); //  15.55%
	m_dsmap.SetAt(0x48830058, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48830058>); //  14.83%
	m_dsmap.SetAt(0x4883104e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883104e>); //   3.58%
	m_dsmap.SetAt(0x48831058, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48831058>); //  14.24%
	m_dsmap.SetAt(0x48858058, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48858058>); //   2.27%
	m_dsmap.SetAt(0x48868768, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868768>); //   6.35%
	m_dsmap.SetAt(0x48868778, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868778>); //  15.38%
	m_dsmap.SetAt(0x48868f78, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868f78>); //   4.91%
	m_dsmap.SetAt(0x49004858, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004858>); //   8.72%
	m_dsmap.SetAt(0x4903004d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4903004d>); //   5.28%
	m_dsmap.SetAt(0x49030058, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49030058>); //  48.66%
	m_dsmap.SetAt(0x49031058, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49031058>); //  13.93%
	m_dsmap.SetAt(0x49068f68, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068f68>); //   5.56%
	m_dsmap.SetAt(0x4c830078, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c830078>); //   2.49%
	m_dsmap.SetAt(0x4c870878, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c870878>); //   9.09%

	// the punisher

	m_dsmap.SetAt(0x48420f64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48420f64>); //  11.13%
	m_dsmap.SetAt(0x48468f64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48468f64>); //  10.99%
	m_dsmap.SetAt(0x4880474c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4880474c>); //  17.06%
	m_dsmap.SetAt(0x4880474e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4880474e>); //   2.16%
	m_dsmap.SetAt(0x4883874c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883874c>); //   3.61%
	m_dsmap.SetAt(0x4883974c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883974c>); //   7.07%
	m_dsmap.SetAt(0x4885874c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4885874c>); //   3.78%
	m_dsmap.SetAt(0x48868764, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868764>); //   7.42%
	m_dsmap.SetAt(0x48868f64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868f64>); //  74.53%
	m_dsmap.SetAt(0x4886b764, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4886b764>); //   4.61%
	m_dsmap.SetAt(0x4886bf64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4886bf64>); //  24.46%
	m_dsmap.SetAt(0x4887874c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887874c>); //   6.85%
	m_dsmap.SetAt(0x4906874c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4906874c>); //   2.84%
	m_dsmap.SetAt(0x49068764, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068764>); //   5.77%
	m_dsmap.SetAt(0x49068f64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068f64>); //   6.21%
	m_dsmap.SetAt(0x4906974c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4906974c>); //  15.63%
	m_dsmap.SetAt(0x4d00474c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d00474c>); //  13.53%
	m_dsmap.SetAt(0x4d068764, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d068764>); //   2.83%
	m_dsmap.SetAt(0x4d068f64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d068f64>); //  13.52%
	m_dsmap.SetAt(0x5fe0474e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe0474e>); //   5.68%
	m_dsmap.SetAt(0x5fe3974c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe3974c>); //  21.28%
	m_dsmap.SetAt(0x5fe68f64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe68f64>); //  93.35%
	m_dsmap.SetAt(0x5fe6af64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe6af64>); //   2.11%

	// guitar hero

	m_dsmap.SetAt(0x1fe3106a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3106a>); //  10.62%
	m_dsmap.SetAt(0x1fe3905a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3905a>); //   4.12%
	m_dsmap.SetAt(0x1fe6887a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe6887a>); //   9.12%
	m_dsmap.SetAt(0x48804d4e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804d4e>); //  28.75%
	m_dsmap.SetAt(0x48804d6a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804d6a>); //   4.04%
	m_dsmap.SetAt(0x48804d7a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804d7a>); //  15.30%
	m_dsmap.SetAt(0x4886815a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4886815a>); //   2.33%
	m_dsmap.SetAt(0x4886854e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4886854e>); //   8.02%
	m_dsmap.SetAt(0x4886857a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4886857a>); //   4.70%
	m_dsmap.SetAt(0x48868d5a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868d5a>); //   5.95%
	m_dsmap.SetAt(0x48868d6a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868d6a>); //   8.10%
	m_dsmap.SetAt(0x48868d7a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868d7a>); //  46.88%
	m_dsmap.SetAt(0x4886957a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4886957a>); //   2.24%
	m_dsmap.SetAt(0x4887854e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887854e>); //   8.47%
	m_dsmap.SetAt(0x4887857a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887857a>); //  26.93%
	m_dsmap.SetAt(0x48878d4e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878d4e>); //   3.63%
	m_dsmap.SetAt(0x48878d7a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878d7a>); //  29.77%
	m_dsmap.SetAt(0x4887917a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887917a>); //  24.09%
	m_dsmap.SetAt(0x4887954e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887954e>); //  11.46%
	m_dsmap.SetAt(0x4887957a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887957a>); //  27.16%
	m_dsmap.SetAt(0x48884d7a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48884d7a>); //   8.23%
	m_dsmap.SetAt(0x488a917a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488a917a>); //  19.80%
	m_dsmap.SetAt(0x488e8d6a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8d6a>); //   4.32%
	m_dsmap.SetAt(0x488e8d7a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8d7a>); //  60.16%
	m_dsmap.SetAt(0x4906806a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4906806a>); //  25.96%
	m_dsmap.SetAt(0x4906886a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4906886a>); //   5.88%
	m_dsmap.SetAt(0x4907804e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4907804e>); //   3.48%
	m_dsmap.SetAt(0x4d03204e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d03204e>); //  64.22%
	m_dsmap.SetAt(0x4d06986a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d06986a>); //   3.72%
	m_dsmap.SetAt(0x4d06a06a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d06a06a>); //  19.99%
	m_dsmap.SetAt(0x4d06a86a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d06a86a>); //  12.55%
	m_dsmap.SetAt(0x4d07806a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d07806a>); //   7.69%
	m_dsmap.SetAt(0x4d0ea06a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d0ea06a>); //   3.71%
	m_dsmap.SetAt(0x4d0ea86a, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d0ea86a>); //   5.03%
	m_dsmap.SetAt(0x5503204e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5503204e>); //  21.69%

	// ico

	m_dsmap.SetAt(0x1fe04060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04060>); //   6.21%
	m_dsmap.SetAt(0x1fe380cc, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe380cc>); //  15.32%
	m_dsmap.SetAt(0x1fe3a04c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3a04c>); //  44.62%
	m_dsmap.SetAt(0x1fe68360, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe68360>); //   6.82%
	m_dsmap.SetAt(0x1fe68b60, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe68b60>); //  41.46%
	m_dsmap.SetAt(0x48859060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48859060>); //   9.43%
	m_dsmap.SetAt(0x4893814c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4893814c>); //  40.73%
	m_dsmap.SetAt(0x49004060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004060>); //   5.53%
	m_dsmap.SetAt(0x49068b60, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068b60>); //  21.00%
	m_dsmap.SetAt(0x4d004060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d004060>); // 195.95%
	m_dsmap.SetAt(0x4d068360, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d068360>); //  15.67%
	m_dsmap.SetAt(0x4d068b60, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d068b60>); // 120.45%
	m_dsmap.SetAt(0x4d078360, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d078360>); //   5.76%
	m_dsmap.SetAt(0x48858060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48858060>); //  15.74%
	m_dsmap.SetAt(0x4c468b60, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c468b60>); //  36.86%
	m_dsmap.SetAt(0x4c478b60, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c478b60>); //  10.36%
	m_dsmap.SetAt(0x1fe28060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe28060>); //  14.34%
	m_dsmap.SetAt(0x4d028060, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d028060>); //  15.65%

	// kuon

	m_dsmap.SetAt(0x4847004d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4847004d>); //   9.19%
	m_dsmap.SetAt(0x48470065, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48470065>); //  17.45%
	m_dsmap.SetAt(0x4847084d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4847084d>); //  12.91%
	m_dsmap.SetAt(0x4880404e, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4880404e>); //   4.30%
	m_dsmap.SetAt(0x48860365, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48860365>); //  10.49%
	m_dsmap.SetAt(0x48860b65, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48860b65>); //  26.88%
	m_dsmap.SetAt(0x48868365, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868365>); //   2.99%
	m_dsmap.SetAt(0x48868b65, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868b65>); //  17.44%
	m_dsmap.SetAt(0x4887004d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4887004d>); //  21.13%
	m_dsmap.SetAt(0x48870365, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48870365>); //  30.82%
	m_dsmap.SetAt(0x48878b65, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878b65>); //  13.19%
	m_dsmap.SetAt(0x488e0b65, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e0b65>); //  56.84%
	m_dsmap.SetAt(0x488e8b65, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8b65>); //  19.74%
	m_dsmap.SetAt(0x49060865, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49060865>); //  11.56%
	m_dsmap.SetAt(0x4907004d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4907004d>); //  14.36%
	m_dsmap.SetAt(0x49070065, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49070065>); //   5.51%
	m_dsmap.SetAt(0x4907084d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4907084d>); //  14.16%
	m_dsmap.SetAt(0x4907884d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4907884d>); //  18.56%
	m_dsmap.SetAt(0x4c429065, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c429065>); //  16.47%
	m_dsmap.SetAt(0x5fe04055, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe04055>); //   4.02%
	m_dsmap.SetAt(0x5fe3004d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe3004d>); //  12.18%
	m_dsmap.SetAt(0x5fe3804d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe3804d>); //  17.31%
	m_dsmap.SetAt(0x5fee0b65, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fee0b65>); //  27.96%

	// hxh

	m_dsmap.SetAt(0x1fe04176, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04176>); //   2.08%
	m_dsmap.SetAt(0x1fe78176, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe78176>); //   3.41%
	m_dsmap.SetAt(0x1fee8876, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee8876>); //  23.15%
	m_dsmap.SetAt(0x1fee8976, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee8976>); //  10.99%
	m_dsmap.SetAt(0x48804176, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804176>); //   3.38%
	m_dsmap.SetAt(0x48839176, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48839176>); //   6.41%
	m_dsmap.SetAt(0x488e8876, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8876>); //   3.25%
	m_dsmap.SetAt(0x488e8976, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8976>); //   2.41%
	m_dsmap.SetAt(0x489e8876, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x489e8876>); //   3.00%
	m_dsmap.SetAt(0x49004976, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004976>); //   3.51%
	m_dsmap.SetAt(0x49068176, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068176>); //  21.64%
	m_dsmap.SetAt(0x1fe04976, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04976>); //  23.04%
	m_dsmap.SetAt(0x1fee8076, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee8076>); //   2.87%
	m_dsmap.SetAt(0x48838176, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48838176>); //   6.49%
	m_dsmap.SetAt(0x48878176, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878176>); //   6.63%
	m_dsmap.SetAt(0x48879176, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879176>); //   4.28%
	m_dsmap.SetAt(0x48884976, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48884976>); //   3.84%

	// grandia extreme

	m_dsmap.SetAt(0x1fe04050, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04050>); //   5.03%
	m_dsmap.SetAt(0x1fe1104c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe1104c>); //  29.26%
	m_dsmap.SetAt(0x1fe3884c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3884c>); //  29.05%
	m_dsmap.SetAt(0x4520404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4520404c>); //  15.63%
	m_dsmap.SetAt(0x45269070, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x45269070>); //   6.47%
	m_dsmap.SetAt(0x452e9070, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x452e9070>); //   9.20%
	m_dsmap.SetAt(0x4880434c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4880434c>); //  16.35%
	m_dsmap.SetAt(0x48804350, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804350>); //  21.63%
	m_dsmap.SetAt(0x48804b4c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804b4c>); //   3.40%
	m_dsmap.SetAt(0x48804b50, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804b50>); //   4.16%
	m_dsmap.SetAt(0x48868370, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48868370>); //  15.75%
	m_dsmap.SetAt(0x48869370, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48869370>); //  27.39%
	m_dsmap.SetAt(0x48878350, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878350>); //   2.11%
	m_dsmap.SetAt(0x48878370, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878370>); //  21.73%
	m_dsmap.SetAt(0x48878b50, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878b50>); //   4.87%
	m_dsmap.SetAt(0x48879370, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879370>); //  33.27%
	m_dsmap.SetAt(0x4888434c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4888434c>); //  14.48%
	m_dsmap.SetAt(0x48884350, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48884350>); //  18.31%
	m_dsmap.SetAt(0x488e8b70, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8b70>); //  72.05%
	m_dsmap.SetAt(0x488e9370, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e9370>); //  15.63%
	m_dsmap.SetAt(0x4c81934c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c81934c>); //  76.79%
	m_dsmap.SetAt(0x4c984070, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c984070>); //   2.36%

	// enthusa

	m_dsmap.SetAt(0x1fe04067, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04067>); //   3.85%
	m_dsmap.SetAt(0x1fe04854, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04854>); //  26.01%
	m_dsmap.SetAt(0x1fe60168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe60168>); //   5.84%
	m_dsmap.SetAt(0x1fe60968, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe60968>); //   5.03%
	m_dsmap.SetAt(0x1fe79364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe79364>); //  13.59%
	m_dsmap.SetAt(0x1fe84064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe84064>); //   6.10%
	m_dsmap.SetAt(0x1fee0964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee0964>); //  32.09%
	m_dsmap.SetAt(0x1fee8064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee8064>); //   2.00%
	m_dsmap.SetAt(0x1fee8864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee8864>); //  22.87%
	m_dsmap.SetAt(0x48804364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804364>); //   6.93%
	m_dsmap.SetAt(0x48804b64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804b64>); //  16.53%
	m_dsmap.SetAt(0x48804ee4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804ee4>); //   3.43%
	m_dsmap.SetAt(0x4883004d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4883004d>); //  24.27%
	m_dsmap.SetAt(0x48858064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48858064>); //   3.08%
	m_dsmap.SetAt(0x48858364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48858364>); //   3.38%
	m_dsmap.SetAt(0x48860f64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48860f64>); //  16.25%
	m_dsmap.SetAt(0x48878364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48878364>); //   7.33%
	m_dsmap.SetAt(0x48879364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48879364>); //  13.53%
	m_dsmap.SetAt(0x4a46884c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a46884c>); //  15.20%
	m_dsmap.SetAt(0x4b020864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b020864>); //   4.27%
	m_dsmap.SetAt(0x4b060864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b060864>); //  19.55%
	m_dsmap.SetAt(0x4b120864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b120864>); //  10.40%
	m_dsmap.SetAt(0x4b168064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b168064>); //   2.64%
	m_dsmap.SetAt(0x4c83104c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c83104c>); //  24.37%
	m_dsmap.SetAt(0x4d40404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d40404c>); //  11.69%
	m_dsmap.SetAt(0x5fe1104c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe1104c>); //  12.87%
	m_dsmap.SetAt(0x5fe3104c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe3104c>); //   2.39%
	m_dsmap.SetAt(0x5fe3104d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe3104d>); //  20.74%

	// ys 1/2 eternal story

	m_dsmap.SetAt(0x1fe3104c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe3104c>); //   8.54%
	m_dsmap.SetAt(0x490701cc, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490701cc>); //   8.44%
	m_dsmap.SetAt(0x4c8791cc, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c8791cc>); //  10.82%
	m_dsmap.SetAt(0x5420404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5420404c>); //   6.71%

	// bloody roar

	m_dsmap.SetAt(0x1fe84868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe84868>); //   9.84%
	m_dsmap.SetAt(0x1fee8b68, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fee8b68>); //  24.88%
	m_dsmap.SetAt(0x48810068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48810068>); //  22.47%
	m_dsmap.SetAt(0x48810368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48810368>); //  24.36%
	m_dsmap.SetAt(0x48818368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48818368>); //  10.37%
	m_dsmap.SetAt(0x48819368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48819368>); //   8.72%
	m_dsmap.SetAt(0x48848368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48848368>); //  18.03%
	m_dsmap.SetAt(0x48859368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48859368>); //   5.29%
	m_dsmap.SetAt(0x488791e8, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488791e8>); //  12.45%
	m_dsmap.SetAt(0x488e9368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e9368>); //   8.65%
	m_dsmap.SetAt(0x49004068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004068>); //   8.69%
	m_dsmap.SetAt(0x49004b68, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004b68>); //   6.86%
	m_dsmap.SetAt(0x49018368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49018368>); //  12.57%
	m_dsmap.SetAt(0x49019368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49019368>); //  12.39%
	m_dsmap.SetAt(0x49020b4c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49020b4c>); //  17.18%
	m_dsmap.SetAt(0x49058368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49058368>); //   2.61%
	m_dsmap.SetAt(0x49068b68, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49068b68>); //   3.40%
	m_dsmap.SetAt(0x4907834c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4907834c>); //   2.86%
	m_dsmap.SetAt(0x490789e8, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490789e8>); //   2.47%
	m_dsmap.SetAt(0x4b068068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b068068>); //  14.59%
	m_dsmap.SetAt(0x4b068868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b068868>); //  13.91%
	m_dsmap.SetAt(0x4b0a8868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b0a8868>); //   4.19%
	m_dsmap.SetAt(0x4b0e8868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b0e8868>); //   3.64%
	m_dsmap.SetAt(0x4b0e9068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4b0e9068>); //   2.42%
	m_dsmap.SetAt(0x4c469368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c469368>); //   6.06%

	// ferrari f355 challenge

	m_dsmap.SetAt(0x48858164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48858164>); //   3.11%
	m_dsmap.SetAt(0x48858168, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48858168>); //  39.59%
	m_dsmap.SetAt(0x488e8064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8064>); //   4.33%
	m_dsmap.SetAt(0x488e8364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8364>); //   4.76%
	m_dsmap.SetAt(0x4890404c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4890404c>); //   5.97%
	m_dsmap.SetAt(0x48984064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48984064>); //   2.27%
	m_dsmap.SetAt(0x489e8364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x489e8364>); //   6.39%
	m_dsmap.SetAt(0x489e8b64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x489e8b64>); //   7.29%
	m_dsmap.SetAt(0x49484064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49484064>); //   2.44%
	m_dsmap.SetAt(0x49484864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49484864>); //   3.30%
	m_dsmap.SetAt(0x5fe04068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe04068>); //  17.08%
	m_dsmap.SetAt(0x5fe04868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe04868>); //   6.03%
	m_dsmap.SetAt(0x5fe60064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fe60064>); //  15.71%
	m_dsmap.SetAt(0x5fee0064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fee0064>); //  11.00%
	m_dsmap.SetAt(0x5fee0864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fee0864>); //  33.74%
	m_dsmap.SetAt(0x5feeb864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5feeb864>); //   7.91%
	m_dsmap.SetAt(0x5feeb868, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5feeb868>); //   2.80%
	m_dsmap.SetAt(0x5ff60064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5ff60064>); //  18.85%
	m_dsmap.SetAt(0x5ffe0064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5ffe0064>); //  17.14%
	m_dsmap.SetAt(0x5ffe0864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5ffe0864>); //  32.32%
	m_dsmap.SetAt(0x5ffeb864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5ffeb864>); //   4.45%

	// king of fighters xi

	m_dsmap.SetAt(0x4880484c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4880484c>); //  15.57%
	m_dsmap.SetAt(0x4881804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4881804c>); //   5.26%
	m_dsmap.SetAt(0x4881904c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4881904c>); //  20.91%
	m_dsmap.SetAt(0x4885804c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4885804c>); //  27.10%
	m_dsmap.SetAt(0x488589e8, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488589e8>); //  99.82%
	m_dsmap.SetAt(0x488591e8, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488591e8>); //  56.90%
	m_dsmap.SetAt(0x74819050, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x74819050>); //  11.28%

	// mana khemia

	m_dsmap.SetAt(0x4885904d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4885904d>); //  10.84%
	m_dsmap.SetAt(0x49038059, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49038059>); //  30.86%

	// ar tonelico 2

	m_dsmap.SetAt(0x1fe04059, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe04059>); //   3.09%
	m_dsmap.SetAt(0x484f8369, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x484f8369>); //   7.81%
	m_dsmap.SetAt(0x48804059, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48804059>); //  15.96%
	m_dsmap.SetAt(0x4881804d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4881804d>); //  19.93%
	m_dsmap.SetAt(0x48819059, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48819059>); //  22.64%
	m_dsmap.SetAt(0x4885804d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4885804d>); //   6.01%
	m_dsmap.SetAt(0x48859059, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48859059>); //  11.13%
	m_dsmap.SetAt(0x488e8369, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e8369>); //  20.98%
	m_dsmap.SetAt(0x488e9369, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e9369>); // 136.48%
	m_dsmap.SetAt(0x488f8369, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488f8369>); //  21.74%
	m_dsmap.SetAt(0x488f9369, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488f9369>); //  59.36%
	m_dsmap.SetAt(0x4905804d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4905804d>); //   3.85%
	m_dsmap.SetAt(0x4905904d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4905904d>); //   5.55%
	m_dsmap.SetAt(0x4907804d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4907804d>); //  12.62%
	m_dsmap.SetAt(0x4907904d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4907904d>); //   3.97%
	m_dsmap.SetAt(0x49079059, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49079059>); //  90.14%
	m_dsmap.SetAt(0x490f8369, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490f8369>); //  25.78%

	// rouge galaxy

	m_dsmap.SetAt(0x484e8164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x484e8164>); //  15.77%
	m_dsmap.SetAt(0x48858154, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48858154>); //  15.37%
	m_dsmap.SetAt(0x488b0964, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488b0964>); //  98.90%

	// mobile suit gundam seed battle assault 3

	m_dsmap.SetAt(0x48584064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48584064>); //   6.20%
	m_dsmap.SetAt(0x488390cc, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488390cc>); //  19.78%
	m_dsmap.SetAt(0x488780e4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488780e4>); //   6.17%
	m_dsmap.SetAt(0x488781cc, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488781cc>); //   9.42%
	m_dsmap.SetAt(0x488791cc, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488791cc>); //   9.41%
	m_dsmap.SetAt(0x488e90f4, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488e90f4>); //   2.39%
	m_dsmap.SetAt(0x49004854, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49004854>); //  15.80%
	m_dsmap.SetAt(0x490781cc, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490781cc>); //   2.27%
	m_dsmap.SetAt(0x490e8074, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e8074>); //   5.53%
	m_dsmap.SetAt(0x490e9074, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e9074>); //   5.99%
	m_dsmap.SetAt(0x490e9164, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490e9164>); //   9.80%
	m_dsmap.SetAt(0x49184064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49184064>); //   6.57%
	m_dsmap.SetAt(0x4c81004d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c81004d>); //  11.58%
	m_dsmap.SetAt(0x5fee8174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fee8174>); //   9.44%
	m_dsmap.SetAt(0x5fee8974, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fee8974>); //  50.05%
	m_dsmap.SetAt(0x5fee9174, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5fee9174>); //   3.41%

	// hajime no ippo 2

	m_dsmap.SetAt(0x1fe39358, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe39358>); //   4.42%
	m_dsmap.SetAt(0x4881034c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4881034c>); //  20.41%
	m_dsmap.SetAt(0x48884068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48884068>); //   3.46%
	m_dsmap.SetAt(0x488a9368, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488a9368>); //  28.16%
	m_dsmap.SetAt(0x48c28b68, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48c28b68>); //   6.85%

	// virtual tennis 2

	m_dsmap.SetAt(0x1fe681f5, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe681f5>); //   4.41%
	m_dsmap.SetAt(0x488041e5, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488041e5>); //   2.00%
	m_dsmap.SetAt(0x488049e5, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488049e5>); //   3.09%
	m_dsmap.SetAt(0x488581e5, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488581e5>); //   2.22%
	m_dsmap.SetAt(0x488589e5, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488589e5>); //   2.12%
	m_dsmap.SetAt(0x488591e5, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488591e5>); //   7.06%
	m_dsmap.SetAt(0x488681f5, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488681f5>); //  22.24%
	m_dsmap.SetAt(0x488689f5, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488689f5>); //  27.62%
	m_dsmap.SetAt(0x4c818055, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c818055>); //  14.73%
	m_dsmap.SetAt(0x4c819055, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c819055>); //  20.88%
	m_dsmap.SetAt(0x4c8781f5, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c8781f5>); //  11.33%
	m_dsmap.SetAt(0x4d0681cd, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d0681cd>); //   3.65%
	m_dsmap.SetAt(0x5100404d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5100404d>); //   3.70%
	m_dsmap.SetAt(0x5540404d, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x5540404d>); //   5.28%

	// crash wrath of cortex

	m_dsmap.SetAt(0x1fe20864, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe20864>); //  14.55%
	m_dsmap.SetAt(0x1fe28064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe28064>); //   2.29%
	m_dsmap.SetAt(0x1fe5834c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe5834c>); //   2.80%
	m_dsmap.SetAt(0x1fe78064, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe78064>); //   9.75%
	m_dsmap.SetAt(0x4840474c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4840474c>); //   6.45%
	m_dsmap.SetAt(0x48818364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48818364>); //   5.97%
	m_dsmap.SetAt(0x48828764, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48828764>); //   3.79%
	m_dsmap.SetAt(0x48828f64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48828f64>); //  12.66%
	m_dsmap.SetAt(0x48838364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48838364>); //  12.39%
	m_dsmap.SetAt(0x48838f64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48838f64>); //   7.37%
	m_dsmap.SetAt(0x49028f64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49028f64>); //  16.23%
	m_dsmap.SetAt(0x49030764, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49030764>); // 139.71%
	m_dsmap.SetAt(0x49038f64, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x49038f64>); //  14.67%
	m_dsmap.SetAt(0x4a838364, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4a838364>); //   6.38%
	m_dsmap.SetAt(0x4d05834c, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4d05834c>); //   8.50%

	// sbam 2

	m_dsmap.SetAt(0x1fe041e8, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe041e8>); //   7.38%
	m_dsmap.SetAt(0x1fe591e8, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x1fe591e8>); //  10.23%
	m_dsmap.SetAt(0x488041e8, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x488041e8>); //  10.21%
	m_dsmap.SetAt(0x48859068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48859068>); //   4.45%
	m_dsmap.SetAt(0x490041e8, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490041e8>); //  10.73%
	m_dsmap.SetAt(0x490049e8, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x490049e8>); //   3.33%
	m_dsmap.SetAt(0x552041e8, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x552041e8>); //  10.89%

	// remember 11

	m_dsmap.SetAt(0x48819068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48819068>); //  13.06%
	m_dsmap.SetAt(0x48839068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x48839068>); //  43.44%
	m_dsmap.SetAt(0x4c818068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c818068>); //  22.85%
	m_dsmap.SetAt(0x4c819068, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineExT<0x4c819068>); //  22.51%

	// prince of tennis

	// ar tonelico

	// dbz sagas

	// tourist trophy

	// svr2k8

	// tokyo bus guide

	// 12riven

	// xenosaga

	// mgs3s1

	// god of war

	#endif
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
