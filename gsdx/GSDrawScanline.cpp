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

// TODO: 
// - if iip == 0 && tfx == TFX_NONE && fog == 0 && abe a/b/c != 1 => e = (a - b) * c, dst = e + d (simple addus8)
// - detect and convert quads to sprite

#include "StdAfx.h"
#include "GSDrawScanline.h"
#include "GSTextureCacheSW.h"

GSDrawScanline::GSDrawScanline(GSState* state, int id)
	: m_state(state)
	, m_id(id)
{
	memset(&m_env, 0, sizeof(m_env));
}

GSDrawScanline::~GSDrawScanline()
{
}

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
	m_env.fzbr = p->fzbo->row;
	m_env.fzbc = p->fzbo->col;
	m_env.fm = GSVector4i(p->fm);
	m_env.zm = GSVector4i(p->zm);
	m_env.datm = context->TEST.DATM ? GSVector4i::x80000000() : GSVector4i::zero();
	m_env.colclamp = env.COLCLAMP.CLAMP ? GSVector4i::xffffffff() : GSVector4i::x00ff();
	m_env.fba = context->FBA.FBA ? GSVector4i::x80000000() : GSVector4i::zero();
	m_env.aref = GSVector4i((int)context->TEST.AREF);
	m_env.afix = GSVector4i((int)context->ALPHA.FIX << 16);
	m_env.afix2 = m_env.afix.yywwlh().sll16(7);
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

		unsigned short tw = (unsigned short)(1 << context->TEX0.TW);
		unsigned short th = (unsigned short)(1 << context->TEX0.TH);

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
		case CLAMP_REGION_CLAMP: 
			m_env.t.min.u16[0] = min(context->CLAMP.MINU, tw - 1);
			m_env.t.max.u16[0] = min(context->CLAMP.MAXU, tw - 1);
			m_env.t.mask.u32[0] = 0; 
			break;
		case CLAMP_REGION_REPEAT: 
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
		case CLAMP_REGION_CLAMP: 
			m_env.t.min.u16[4] = min(context->CLAMP.MINV, th - 1);
			m_env.t.max.u16[4] = min(context->CLAMP.MAXV, th - 1); // ffx anima summon scene, when the anchor appears (th = 256, maxv > 256)
			m_env.t.mask.u32[2] = 0; 
			break;
		case CLAMP_REGION_REPEAT: 
			m_env.t.min.u16[4] = context->CLAMP.MINV;
			m_env.t.max.u16[4] = context->CLAMP.MAXV;
			m_env.t.mask.u32[2] = 0xffffffff; 
			break;
		default: 
			__assume(0);
		}

		m_env.t.min = m_env.t.min.xxxxlh();
		m_env.t.max = m_env.t.max.xxxxlh();
		m_env.t.mask = m_env.t.mask.xxzz();
	}

	//

	f->sl = m_ds.Lookup(m_env.sel);

	//

	if(m_env.sel.IsSolidRect())
	{
		f->sr = (DrawSolidRectPtr)&GSDrawScanline::DrawSolidRect;
	}

	//

	DWORD sel = 0;

	if(data->primclass != GS_POINT_CLASS)
	{
		sel |= (m_env.sel.ztst ? 1 : 0) << 0;
		sel |= m_env.sel.fge << 1;
		sel |= (m_env.sel.tfx != TFX_NONE ? 1 : 0) << 2;
		sel |= m_env.sel.fst << 3;
		sel |= m_env.sel.iip << 4;
	}

	f->sp = m_sp.Lookup(sel);
}

void GSDrawScanline::EndDraw(const GSRasterizerStats& stats)
{
	m_ds.UpdateStats(stats, m_state->m_perfmon.GetFrame());
}

template<DWORD zbe, DWORD fge, DWORD tme, DWORD fst, DWORD iip>
void GSDrawScanline::SetupPrim(const GSVertexSW* vertices, const GSVertexSW& dscan)
{
	// p

	GSVector4 p = dscan.p;
	
	GSVector4 dz = p.zzzz();
	GSVector4 df = p.wwww();

	if(zbe)
	{
		m_env.d4.z = dz * 4.0f;
	}

	if(fge)
	{
		m_env.d4.f = GSVector4i(df * 4.0f).xxzzlh();
	}

	for(int i = 0; i < 4; i++)
	{
		GSVector4 v = m_shift[i];

		if(zbe)
		{
			m_env.d[i].z = dz * v;
		}

		if(fge)
		{
			m_env.d[i].f = GSVector4i(df * v).xxzzlh();
		}
	}

	if(iip == 0) // should be sprite == 1, but close enough
	{
		GSVector4 p = vertices[0].p;

		if(zbe)
		{
			GSVector4 z = p.zzzz();

			m_env.p.z = (GSVector4i(z * 0.5f) << 1) | (GSVector4i(z) & GSVector4i::x00000001());
		}

		if(fge)
		{
			m_env.p.f = GSVector4i(p).zzzzh().zzzz();
		}
	}

	// t

	if(tme)
	{
		GSVector4 t = dscan.t;

		if(fst)
		{
			m_env.d4.st = GSVector4i(t * 4.0f);

			GSVector4 ds = t.xxxx();
			GSVector4 dt = t.yyyy();

			for(int i = 0; i < 4; i++)
			{
				GSVector4 v = m_shift[i];

				m_env.d[i].si = GSVector4i(ds * v);
				m_env.d[i].ti = GSVector4i(dt * v);
			}
		}
		else
		{
			m_env.d4.stq = t * 4.0f;

			GSVector4 ds = t.xxxx();
			GSVector4 dt = t.yyyy();
			GSVector4 dq = t.zzzz();

			for(int i = 0; i < 4; i++)
			{
				GSVector4 v = m_shift[i];

				m_env.d[i].s = ds * v;
				m_env.d[i].t = dt * v;
				m_env.d[i].q = dq * v;
			}
		}
	}

	// c

	if(iip)
	{
		GSVector4 c = dscan.c;

		m_env.d4.c = GSVector4i(c * 4.0f).xzyw().ps32();

		GSVector4 dr = c.xxxx();
		GSVector4 dg = c.yyyy();
		GSVector4 db = c.zzzz();
		GSVector4 da = c.wwww();

		for(int i = 0; i < 4; i++)
		{
			GSVector4 v = m_shift[i];

			GSVector4i rg = GSVector4i(dr * v).ps32(GSVector4i(dg * v));
			GSVector4i ba = GSVector4i(db * v).ps32(GSVector4i(da * v));

			m_env.d[i].rb = rg.upl16(ba);
			m_env.d[i].ga = rg.uph16(ba);
		}
	}
	else
	{
		GSVector4i c = GSVector4i(vertices[0].c);
		
		c = c.upl16(c.zwxy());

		if(!tme) c = c.srl16(7);

		m_env.c.rb = c.xxxx();
		m_env.c.ga = c.zzzz();
	}
}

GSVector4i GSDrawScanline::Wrap(const GSVector4i& t)
{
	GSVector4i clamp = t.sat_i16(m_env.t.min, m_env.t.max);
	GSVector4i repeat = (t & m_env.t.min) | m_env.t.max;

	return clamp.blend8(repeat, m_env.t.mask);
}

void GSDrawScanline::SampleTexture(DWORD ltf, DWORD tlu, const GSVector4i& u, const GSVector4i& v, GSVector4i* c)
{
	const void* RESTRICT tex = m_env.tex;
	const DWORD* RESTRICT clut = m_env.clut;
	const DWORD tw = m_env.tw;

	GSVector4i uv = u.sra32(16).ps32(v.sra32(16));

	GSVector4i c00, c01, c10, c11;

	if(ltf)
	{
		GSVector4i uf = u.xxzzlh().srl16(1);
		GSVector4i vf = v.xxzzlh().srl16(1);

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

		rb00 = rb00.lerp16<0>(rb01, uf);
		rb10 = rb10.lerp16<0>(rb11, uf);
		rb00 = rb00.lerp16<0>(rb10, vf);

		c[0] = rb00;

		GSVector4i ga00 = (c00 >> 8) & mask;
		GSVector4i ga01 = (c01 >> 8) & mask;
		GSVector4i ga10 = (c10 >> 8) & mask;
		GSVector4i ga11 = (c11 >> 8) & mask;

		ga00 = ga00.lerp16<0>(ga01, uf);
		ga10 = ga10.lerp16<0>(ga11, uf);
		ga00 = ga00.lerp16<0>(ga10, vf);

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

void GSDrawScanline::ColorTFX(DWORD iip, DWORD tfx, const GSVector4i& rbf, const GSVector4i& gaf, GSVector4i& rbt, GSVector4i& gat)
{
	GSVector4i rb = iip ? rbf : m_env.c.rb;
	GSVector4i ga = iip ? gaf : m_env.c.ga;

	GSVector4i af;

	switch(tfx)
	{
	case TFX_MODULATE:
		rbt = rbt.modulate16<1>(rb).clamp8();
		break;
	case TFX_DECAL:
		break;
	case TFX_HIGHLIGHT:
	case TFX_HIGHLIGHT2:
		af = ga.yywwlh().srl16(7);
		rbt = rbt.modulate16<1>(rb).add16(af).clamp8();
		gat = gat.modulate16<1>(ga).add16(af).clamp8().mix16(gat);
		break;
	case TFX_NONE:
		rbt = iip ? rb.srl16(7) : rb;
		break;
	default:
		__assume(0);
	}
}

void GSDrawScanline::AlphaTFX(DWORD iip, DWORD tfx, DWORD tcc, const GSVector4i& gaf, GSVector4i& gat)
{
	GSVector4i ga = iip ? gaf : m_env.c.ga;

	switch(tfx)
	{
	case TFX_MODULATE:
		gat = gat.modulate16<1>(ga).clamp8(); // mul16hrs rounds and breaks fogging in resident evil 4 (only modulate16<0> uses mul16hrs, but watch out)
		if(!tcc) gat = gat.mix16(ga.srl16(7));
		break;
	case TFX_DECAL: 
		break;
	case TFX_HIGHLIGHT: 
		gat = gat.mix16(!tcc ? ga.srl16(7) : gat.addus8(ga.srl16(7)));
		break;
	case TFX_HIGHLIGHT2:
		if(!tcc) gat = gat.mix16(ga.srl16(7));
		break;
	case TFX_NONE: 
		gat = iip ? ga.srl16(7) : ga;
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

bool GSDrawScanline::TestDestAlpha(DWORD fpsm, DWORD date, const GSVector4i& fd, GSVector4i& test)
{
	if(date)
	{
		switch(fpsm)
		{
		case 0:
			test |= (fd ^ m_env.datm).sra32(31);
			if(test.alltrue()) return false;
		case 1:
			break;
		case 2:
			test |= ((fd << 16) ^ m_env.datm).sra32(31);
			if(test.alltrue()) return false;
		case 3:
			break;
		default:
			__assume(0);
		}
	}

	return true;
}

void GSDrawScanline::ReadPixel(int psm, int addr, GSVector4i& c) const
{
	WORD* vm16 = (WORD*)m_env.vm;

	if(psm != 3)
	{
		c = GSVector4i::load(&vm16[addr], &vm16[addr + 8]);
	}
}

void GSDrawScanline::WritePixel(int psm, WORD* RESTRICT vm16, DWORD c) 
{
	DWORD* RESTRICT vm32 = (DWORD*)vm16;

	switch(psm)
	{
	case 0: *vm32 = c; break;
	case 1: *vm32 = (*vm32 & 0xff000000) | (c & 0x00ffffff); break;
	case 2: *vm16 = (WORD)c; break;
	}
}

void GSDrawScanline::WriteFrame(int fpsm, int rfb, GSVector4i* c, const GSVector4i& fd, const GSVector4i& fm, int addr, int fzm)
{
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
			if(fzm & 0x03) GSVector4i::storel(&vm16[addr + 0], fs); 
			if(fzm & 0x0c) GSVector4i::storeh(&vm16[addr + 8], fs); 

			return;
		}
	}

	if(fzm & 0x01) WritePixel(fpsm, &vm16[addr + 0], fs.extract32<0>()); 
	if(fzm & 0x02) WritePixel(fpsm, &vm16[addr + 2], fs.extract32<1>());
	if(fzm & 0x04) WritePixel(fpsm, &vm16[addr + 8], fs.extract32<2>()); 
	if(fzm & 0x08) WritePixel(fpsm, &vm16[addr + 10], fs.extract32<3>());
}

void GSDrawScanline::WriteZBuf(int zpsm, int ztst, const GSVector4i& z, const GSVector4i& zd, const GSVector4i& zm, int addr, int fzm)
{
	if(ztst == 0) return;

	WORD* RESTRICT vm16 = (WORD*)m_env.vm;

	GSVector4i zs = z;

	if(ztst > 1)
	{
		if(zpsm < 2)
		{
			zs = zs.blend8(zd, zm);

			if(fzm & 0x30) GSVector4i::storel(&vm16[addr + 0], zs); 
			if(fzm & 0xc0) GSVector4i::storeh(&vm16[addr + 8], zs); 

			return;
		}
	}

	if(fzm & 0x10) WritePixel(zpsm, &vm16[addr + 0], zs.extract32<0>()); 
	if(fzm & 0x20) WritePixel(zpsm, &vm16[addr + 2], zs.extract32<1>());
	if(fzm & 0x40) WritePixel(zpsm, &vm16[addr + 8], zs.extract32<2>()); 
	if(fzm & 0x80) WritePixel(zpsm, &vm16[addr + 10], zs.extract32<3>());
}

template<DWORD fpsm, DWORD zpsm, DWORD ztst, DWORD iip>
void GSDrawScanline::DrawScanline(int top, int left, int right, const GSVertexSW& v)
{
	int skip = left & 3;

	left -= skip;

	int steps = right - left - 4;

	GSVector4i test = m_test[skip] | m_test[7 + (steps & (steps >> 31))];

	//

	GSVector2i fza_base;
	GSVector2i* fza_offset;

	GSVector4 z, s, t, q;
	GSVector4i si, ti, f, rb, ga;

	// fza

	fza_base = m_env.fzbr[top];
	fza_offset = &m_env.fzbc[left >> 2];

	// v.p

	GSVector4 vp = v.p;

	z = vp.zzzz() + m_env.d[skip].z;
	f = GSVector4i(vp).zzzzh().zzzz().add16(m_env.d[skip].f);

	// v.t

	GSVector4 vt = v.t;

	if(m_env.sel.fst)
	{
		GSVector4i vti(vt);

		si = vti.xxxx() + m_env.d[skip].si;
		ti = vti.yyyy() + m_env.d[skip].ti;
	}
	else
	{
		s = vt.xxxx() + m_env.d[skip].s; 
		t = vt.yyyy() + m_env.d[skip].t;
		q = vt.zzzz() + m_env.d[skip].q;
	}

	// v.c

	if(iip)
	{
		GSVector4i vc = GSVector4i(v.c);

		vc = vc.upl16(vc.zwxy());

		rb = vc.xxxx().add16(m_env.d[skip].rb);
		ga = vc.zzzz().add16(m_env.d[skip].ga);
	}

	//

	while(1)
	{
		do
		{
			int fa = fza_base.x + fza_offset->x;
			int za = fza_base.y + fza_offset->y;
			
			GSVector4i zs = (GSVector4i(z * 0.5f) << 1) | (GSVector4i(z) & GSVector4i::x00000001());
			GSVector4i zd;

			if(ztst > 1)
			{
				ReadPixel(zpsm, za, zd);

				if(!TestZ(zpsm, ztst, zs, zd, test))
				{
					continue;
				}
			}

			GSVector4i c[6];

			if(m_env.sel.tfx != TFX_NONE)
			{
				GSVector4i u, v;

				if(m_env.sel.fst)
				{
					u = si;
					v = ti;
				}
				else
				{
					GSVector4 w = q.rcp();

					u = GSVector4i(s * w);
					v = GSVector4i(t * w);

					if(m_env.sel.ltf)
					{
						u -= 0x8000;
						v -= 0x8000;
					}
				}

				SampleTexture(m_env.sel.ltf, m_env.sel.tlu, u, v, c);
			}

			AlphaTFX(iip, m_env.sel.tfx, m_env.sel.tcc, ga, c[1]);

			GSVector4i fm = m_env.fm;
			GSVector4i zm = m_env.zm;

			if(!TestAlpha(m_env.sel.atst, m_env.sel.afail, c[1], fm, zm, test))
			{
				continue;
			}

			ColorTFX(iip, m_env.sel.tfx, rb, ga, c[0], c[1]);

			Fog(m_env.sel.fge, f, c[0], c[1]);

			GSVector4i fd;

			if(m_env.sel.rfb)
			{
				ReadPixel(fpsm, fa, fd);

				if(!TestDestAlpha(fpsm, m_env.sel.date, fd, test))
				{
					continue;
				}
			}

			fm |= test;
			zm |= test;

			int fzm = ~(fm == GSVector4i::xffffffff()).ps32(zm == GSVector4i::xffffffff()).ps32().mask();

			WriteZBuf(zpsm, ztst, zs, zd, zm, za, fzm);

			if(m_env.sel.abe != 255)
			{
				GSVector4i mask = GSVector4i::x00ff(fd);

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

				GSVector4i a = c[abec * 2 + 1].yywwlh().sll16(7);

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

			WriteFrame(fpsm, m_env.sel.rfb, c, fd, fm, fa, fzm);
		}
		while(0);

		if(steps <= 0) break;

		steps -= 4;

		test = m_test[7 + (steps & (steps >> 31))];

		fza_offset++;

		z += m_env.d4.z;
		f = f.add16(m_env.d4.f);

		if(m_env.sel.fst)
		{
			GSVector4i st = m_env.d4.st;

			si += st.xxxx();
			ti += st.yyyy();
		}
		else
		{
			GSVector4 stq = m_env.d4.stq;

			s += stq.xxxx();
			t += stq.yyyy();
			q += stq.zzzz();
		}

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
	const DWORD sprite = (sel >> 31) & 1;

	//

	int skip = left & 3;

	left -= skip;

	int steps = right - left - 4;

	GSVector4i test = m_test[skip] | m_test[7 + (steps & (steps >> 31))];

	//

	GSVector2i fza_base;
	GSVector2i* fza_offset;

	GSVector4 z, s, t, q;
	GSVector4i zi, si, ti, f, rb, ga;

	// fza

	fza_base = m_env.fzbr[top];
	fza_offset = &m_env.fzbc[left >> 2];

	// v.p

	GSVector4 vp = v.p;

	if(sprite)
	{
		zi = m_env.p.z;
		f = m_env.p.f;
	}
	else
	{
		z = vp.zzzz() + m_env.d[skip].z;
		f = GSVector4i(vp).zzzzh().zzzz().add16(m_env.d[skip].f);
	}

	// v.t

	GSVector4 vt = v.t;

	if(fst)
	{
		GSVector4i vti(vt);

		si = vti.xxxx();
		ti = vti.yyyy();

		si += m_env.d[skip].si;
		if(!sprite) ti += m_env.d[skip].ti;
	}
	else
	{
		s = vt.xxxx() + m_env.d[skip].s; 
		t = vt.yyyy() + m_env.d[skip].t;
		q = vt.zzzz() + m_env.d[skip].q;
	}

	// v.c

	if(iip)
	{
		GSVector4i vc = GSVector4i(v.c);

		vc = vc.upl16(vc.zwxy());

		rb = vc.xxxx().add16(m_env.d[skip].rb);
		ga = vc.zzzz().add16(m_env.d[skip].ga);
	}

	//

	while(1)
	{
		do
		{
			int fa = fza_base.x + fza_offset->x;
			int za = fza_base.y + fza_offset->y;

			GSVector4i zs = sprite ? zi : (GSVector4i(z * 0.5f) << 1) | (GSVector4i(z) & GSVector4i::x00000001());
			GSVector4i zd;

			if(ztst > 1)
			{
				ReadPixel(zpsm, za, zd);

				if(!TestZ(zpsm, ztst, zs, zd, test))
				{
					continue;
				}
			}

			GSVector4i c[6];

			if(tfx != TFX_NONE)
			{
				GSVector4i u, v;

				if(fst)
				{
					u = si;
					v = ti;
				}
				else
				{
					GSVector4 w = q.rcp();

					u = GSVector4i(s * w);
					v = GSVector4i(t * w);

					if(ltf)
					{
						u -= 0x8000;
						v -= 0x8000;
					}
				}

				SampleTexture(ltf, tlu, u, v, c);
			}

			AlphaTFX(iip, tfx, tcc, ga, c[1]);

			GSVector4i fm = m_env.fm;
			GSVector4i zm = m_env.zm;

			if(!TestAlpha(atst, afail, c[1], fm, zm, test))
			{
				continue;
			}

			ColorTFX(iip, tfx, rb, ga, c[0], c[1]);

			Fog(fge, f, c[0], c[1]);

			GSVector4i fd;

			if(rfb)
			{
				ReadPixel(fpsm, fa, fd);

				if(!TestDestAlpha(fpsm, date, fd, test))
				{
					continue;
				}
			}

			fm |= test;
			zm |= test;

			int fzm = ~(fm == GSVector4i::xffffffff()).ps32(zm == GSVector4i::xffffffff()).ps32().mask();

			WriteZBuf(zpsm, ztst, zs, zd, zm, za, fzm);

			if(abe != 255)
			{
				GSVector4i mask = GSVector4i::x00ff(fd);

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

					if(abeb < 2)
					{
						rb = rb.sub16(c[abeb * 2 + 0]);
						ga = ga.sub16(c[abeb * 2 + 1]);
					}

					if(!(fpsm == 1 && abec == 1))
					{
						GSVector4i a = abec < 2 ? c[abec * 2 + 1].yywwlh().sll16(7) : m_env.afix2;

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

			WriteFrame(fpsm, rfb, c, fd, fm, fa, fzm);
		}
		while(0);

		if(steps <= 0) break;

		steps -= 4;

		test = m_test[7 + (steps & (steps >> 31))];

		fza_offset++;

		if(!sprite)
		{
			z += m_env.d4.z;
			f = f.add16(m_env.d4.f);
		}

		if(fst)
		{
			GSVector4i st = m_env.d4.st;

			si += st.xxxx();
			if(!sprite) ti += st.yyyy();
		}
		else
		{
			GSVector4 stq = m_env.d4.stq;

			s += stq.xxxx();
			t += stq.yyyy();
			q += stq.zzzz();
		}

		if(iip)
		{
			GSVector4i c = m_env.d4.c;

			rb = rb.add16(c.xxxx());
			ga = ga.add16(c.yyyy());
		}
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

	// FIXME: sometimes the frame and z buffer may overlap, the outcome is undefined

	DWORD m;

	m = m_env.zm.u32[0];

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

	m = m_env.fm.u32[0];

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
		color = color.xxzzlh();
		mask = mask.xxzzlh();
	}

	color = color.andnot(mask);

	GSVector4i bm(8 * 4 / sizeof(T) - 1, 8 - 1);
	GSVector4i br = (r + bm).andnot(bm.xyxy());

	FillRect<T, masked>(row, col, GSVector4i(r.x, r.y, r.z, br.y), c, m);
	FillRect<T, masked>(row, col, GSVector4i(r.x, br.w, r.z, r.w), c, m);

	if(r.x < br.x || br.z < r.z)
	{
		FillRect<T, masked>(row, col, GSVector4i(r.x, br.y, br.x, br.w), c, m);
		FillRect<T, masked>(row, col, GSVector4i(br.z, br.y, r.z, br.w), c, m);
	}

	FillBlock<T, masked>(row, col, br, color, mask);
}

template<class T, bool masked> 
void GSDrawScanline::FillRect(const GSVector4i* row, int* col, const GSVector4i& r, DWORD c, DWORD m)
{
	if(r.x >= r.z) return;

	for(int y = r.y; y < r.w; y++)
	{
		DWORD base = row[y].x;

		for(int x = r.x; x < r.z; x++)
		{
			T* p = &((T*)m_env.vm)[base + col[x]];

			*p = (T)(!masked ? c : (c | (*p & m)));
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

//

GSDrawScanline::GSDrawScanlineMap::GSDrawScanlineMap()
{
	// w00t :P

	#define InitDS_IIP(fpsm, zpsm, ztst, iip) \
		m_default[fpsm][zpsm][ztst][iip] = (DrawScanlinePtr)&GSDrawScanline::DrawScanline<fpsm, zpsm, ztst, iip>; \

	#define InitDS_ZTST(fpsm, zpsm, ztst) \
		InitDS_IIP(fpsm, zpsm, ztst, 0) \
		InitDS_IIP(fpsm, zpsm, ztst, 1) \

	#define InitDS(fpsm, zpsm) \
		InitDS_ZTST(fpsm, zpsm, 0) \
		InitDS_ZTST(fpsm, zpsm, 1) \
		InitDS_ZTST(fpsm, zpsm, 2) \
		InitDS_ZTST(fpsm, zpsm, 3) \

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
		SetAt(sel, (DrawScanlinePtr)&GSDrawScanline::DrawScanlineEx<##sel##>);

	#ifdef FAST_DRAWSCANLINE

	// bios

	InitDS_Sel(0x1fe04850); //   5.33%
	InitDS_Sel(0x1fe28870); //  21.51%
	InitDS_Sel(0x1fe38050); //   9.64%
	InitDS_Sel(0x1fe38060); //   6.07%
	InitDS_Sel(0x48428050); //   6.74%
	InitDS_Sel(0x48428060); //   6.36%
	InitDS_Sel(0x48804860); //  15.48%
	InitDS_Sel(0x49028060); //   6.32%
	InitDS_Sel(0x4d00484c); //   6.65%
	InitDS_Sel(0x4d02884c); // 103.04%
	InitDS_Sel(0x4d038864); //  92.65%
	InitDS_Sel(0x9fe3804c); //  44.43%
	InitDS_Sel(0x9fe38050); //  10.78%
	InitDS_Sel(0x9fe39050); //  72.37%
	InitDS_Sel(0x9fe39064); //   8.55%
	InitDS_Sel(0xc880404c); //   9.30%
	InitDS_Sel(0xc8804050); //   6.78%
	InitDS_Sel(0xc8839050); //   6.87%
	InitDS_Sel(0xc9038050); //  10.83%
	InitDS_Sel(0xc9039050); //   8.62%
	InitDS_Sel(0xcc804050); //   7.20%
	InitDS_Sel(0xcc83904c); //  11.05%
	InitDS_Sel(0xcd03804c); //  36.79%
	InitDS_Sel(0xcd019050); //  70.44%

	// ffx

	InitDS_Sel(0x11020865); //   9.83%
	InitDS_Sel(0x1fe68875); //   8.41%
	InitDS_Sel(0x1fe69075); //  10.39%
	InitDS_Sel(0x1fe78075); //  19.27%
	InitDS_Sel(0x1fe78155); //  13.51%
	InitDS_Sel(0x1fee8875); //  40.85%
	InitDS_Sel(0x1fee8975); //   6.22%
	InitDS_Sel(0x48804855); //  12.08%
	InitDS_Sel(0x48804865); //  13.11%
	InitDS_Sel(0x4880494d); //   5.75%
	InitDS_Sel(0x48868065); //   5.24%
	InitDS_Sel(0x48868865); //  24.79%
	InitDS_Sel(0x48868965); //  10.88%
	InitDS_Sel(0x48878165); //  28.50%
	InitDS_Sel(0x488789cd); //   6.28%
	InitDS_Sel(0x48879065); //  24.07%
	InitDS_Sel(0x488791e5); //  82.67%
	InitDS_Sel(0x488e8965); //   6.02%
	InitDS_Sel(0x488f89f5); //  24.31%
	InitDS_Sel(0x49004875); //  11.54%
	InitDS_Sel(0x49068065); //  20.63%
	InitDS_Sel(0x49068165); //   8.94%
	InitDS_Sel(0x49068865); //   8.02%
	InitDS_Sel(0x49068965); //   6.38%
	InitDS_Sel(0x49078165); //   6.35%
	InitDS_Sel(0x4d078075); //   9.90%
	InitDS_Sel(0x9100404d); //   8.42%
	InitDS_Sel(0x9fe3914d); //  14.47%
	InitDS_Sel(0xc8804055); //  11.50%
	InitDS_Sel(0xc883004d); //  18.83%
	InitDS_Sel(0xc887814d); //  10.96%
	InitDS_Sel(0xc887914d); //   6.60%
	InitDS_Sel(0xc9078065); //   6.85%
	InitDS_Sel(0xc907814d); //   9.30%
	InitDS_Sel(0xcc819055); //  14.47%
	InitDS_Sel(0xcc839065); //  11.17%
	InitDS_Sel(0xd5204055); //  10.62%
	InitDS_Sel(0x1fea8965); //  11.36%
	InitDS_Sel(0x1fee8075); //   5.65%
	InitDS_Sel(0x48804055); //  11.32%
	InitDS_Sel(0x4883004d); //  18.03%
	InitDS_Sel(0x48830965); //  15.39%
	InitDS_Sel(0x91004055); //  11.63%
	InitDS_Sel(0xc9004065); //  12.81%
	InitDS_Sel(0xcc83914d); //  16.73%
	InitDS_Sel(0x1fee8876); //  21.71%
	InitDS_Sel(0x48404865); //   7.73%
	InitDS_Sel(0x48468865); //  16.38%
	InitDS_Sel(0x48478065); //  14.08%
	InitDS_Sel(0x48479065); //  14.14%
	InitDS_Sel(0x4883084d); //  34.87%
	InitDS_Sel(0x488781f5); //   6.82%
	InitDS_Sel(0x488f89f6); //  26.11%
	InitDS_Sel(0x49078065); //  26.07%
	InitDS_Sel(0x49078865); //   6.26%
	InitDS_Sel(0x9101004e); //  11.91%

	// ffx-2

	InitDS_Sel(0x1fe0404e); //  25.27%
	InitDS_Sel(0x1fe30069); //  19.52%
	InitDS_Sel(0x48804065); //  15.28%
	InitDS_Sel(0x4880484d); //  12.54%
	InitDS_Sel(0x488049cd); //   5.49%
	InitDS_Sel(0x4885884d); //   5.73%
	InitDS_Sel(0x48878865); //   6.14%
	InitDS_Sel(0x488e8165); //   5.90%
	InitDS_Sel(0x4890404c); //  18.46%
	InitDS_Sel(0x4893084c); //  18.84%
	InitDS_Sel(0x49004859); //  15.49%
	InitDS_Sel(0x49004869); //   9.52%
	InitDS_Sel(0x49078965); //   9.70%
	InitDS_Sel(0x49079165); //   9.69%
	InitDS_Sel(0x490e8165); //  13.52%
	InitDS_Sel(0x4c45884d); //   5.58%
	InitDS_Sel(0x4d05884d); //   5.53%
	InitDS_Sel(0xc880404d); //  25.08%
	InitDS_Sel(0xc881814d); //  26.06%
	InitDS_Sel(0xc883904d); //  18.71%
	InitDS_Sel(0xc885004d); //  11.09%
	InitDS_Sel(0xc885904d); //  12.54%
	InitDS_Sel(0xc8878165); //   5.43%
	InitDS_Sel(0xc893004c); // 103.92%
	InitDS_Sel(0xc895004c); //  12.04%
	InitDS_Sel(0xc9004059); //  11.58%
	InitDS_Sel(0xc9059155); //  10.72%
	InitDS_Sel(0xcc850055); //  14.49%
	InitDS_Sel(0x1fe5884d); //   9.56%
	InitDS_Sel(0x1fe68075); //  13.61%
	InitDS_Sel(0x4907804d); //  28.18%
	InitDS_Sel(0x4907814d); //  18.38%
	InitDS_Sel(0xc881004d); //  10.92%
	InitDS_Sel(0xc881904d); //  11.90%
	InitDS_Sel(0xc9004055); //  10.58%

	// ffxii

	InitDS_Sel(0x1fe6804c); //  10.03%
	InitDS_Sel(0x1fe68064); //   6.15%
	InitDS_Sel(0x1fe6884c); //  11.87%
	InitDS_Sel(0x1fee8864); //  76.59%
	InitDS_Sel(0x1fee8964); //  30.41%
	InitDS_Sel(0x48404064); //   5.99%
	InitDS_Sel(0x4847004c); //  16.70%
	InitDS_Sel(0x4880404c); //   6.22%
	InitDS_Sel(0x48828864); //   6.62%
	InitDS_Sel(0x4883004c); //  19.90%
	InitDS_Sel(0x4883084c); //  12.05%
	InitDS_Sel(0x4883804c); //  18.03%
	InitDS_Sel(0x4883884c); //  16.82%
	InitDS_Sel(0x4887084c); // 234.03%
	InitDS_Sel(0x4887804c); //  10.79%
	InitDS_Sel(0x48878064); //  11.52%
	InitDS_Sel(0x4887904c); //   8.31%
	InitDS_Sel(0x488e8b64); //  18.21%
	InitDS_Sel(0x48904064); //   5.01%
	InitDS_Sel(0x48c7804c); //  18.99%
	InitDS_Sel(0x4903804c); //  16.28%
	InitDS_Sel(0x4907804c); //  90.96%
	InitDS_Sel(0x49078064); //  19.62%
	InitDS_Sel(0x49278064); //  73.20%
	InitDS_Sel(0x5fe0404c); //  36.89%
	InitDS_Sel(0x9fe3904c); //   9.62%

	// kingdom hearts

	InitDS_Sel(0x4840404c); //  13.34%
	InitDS_Sel(0x48829164); //   5.20%
	InitDS_Sel(0x48830874); //  13.83%
	InitDS_Sel(0x4886804c); //  17.44%
	InitDS_Sel(0x48868154); //   8.08%
	InitDS_Sel(0x4886884c); //  24.36%
	InitDS_Sel(0x4886904c); //  15.57%
	InitDS_Sel(0x488e8064); //   7.43%
	InitDS_Sel(0x488e8964); //  14.92%
	InitDS_Sel(0x4900404c); //   6.64%
	InitDS_Sel(0x4907814c); //  12.95%
	InitDS_Sel(0x9fe3004d); //  22.66%
	InitDS_Sel(0x9fe3804d); //  18.36%
	InitDS_Sel(0x9fe3904d); //  15.00%
	InitDS_Sel(0xc8818054); //  12.48%
	InitDS_Sel(0xc8819054); //  12.83%
	InitDS_Sel(0xc8830064); // 142.61%
	InitDS_Sel(0xc883804c); // 112.14%
	InitDS_Sel(0xc8858054); //  10.55%
	InitDS_Sel(0xc8859054); //   5.21%
	InitDS_Sel(0xc9078174); //  14.38%
	InitDS_Sel(0xdfe3904c); //   5.38%

	// kingdom hearts 2

	InitDS_Sel(0x1fee9064); //   9.56%
	InitDS_Sel(0x48804060); //   5.66%
	InitDS_Sel(0x48810054); //  14.13%
	InitDS_Sel(0x4887884d); //   5.37%
	InitDS_Sel(0x48878854); //   9.68%
	InitDS_Sel(0x488a8964); //  16.42%
	InitDS_Sel(0x488e8164); //   7.96%
	InitDS_Sel(0x490e8964); //  12.17%
	InitDS_Sel(0xc883814c); //   5.78%
	InitDS_Sel(0xcc83004d); //  43.52%
	InitDS_Sel(0xcd03004d); //  15.94%
	InitDS_Sel(0x1fee9164); //   6.64%
	InitDS_Sel(0x48858854); //   6.58%
	InitDS_Sel(0x48859054); //   6.51%
	InitDS_Sel(0x49078054); //  18.24%
	InitDS_Sel(0x9fe39054); //  13.49%
	InitDS_Sel(0xc8810054); //  28.72%
	InitDS_Sel(0xc9004054); //  11.39%

	// persona 3

	InitDS_Sel(0x4881904c); //  16.87%
	InitDS_Sel(0x48878b68); //  13.03%
	InitDS_Sel(0x4907884c); //  26.82%
	InitDS_Sel(0x4907904c); //  13.53%
	InitDS_Sel(0x4b07904c); //  44.27%
	InitDS_Sel(0x4d47804c); //  73.78%
	InitDS_Sel(0x4d47834c); //  50.33%
	InitDS_Sel(0x4d47934c); //  21.51%

	// persona 4

	InitDS_Sel(0x1fe04058); //   9.41%
	InitDS_Sel(0x4840484c); //  11.79%
	InitDS_Sel(0x48804058); //  62.08%
	InitDS_Sel(0x48804068); //  14.60%
	InitDS_Sel(0x4880484c); //  16.29%
	InitDS_Sel(0x4881834c); //  19.60%
	InitDS_Sel(0x4881934c); //  18.66%
	InitDS_Sel(0x48828368); //   6.84%
	InitDS_Sel(0x48859368); //   6.15%
	InitDS_Sel(0x48868868); // 503.31%
	InitDS_Sel(0x48868f68); //  24.99%
	InitDS_Sel(0x48869068); //  16.84%
	InitDS_Sel(0x48879068); //  26.80%
	InitDS_Sel(0x48879168); //   5.30%
	InitDS_Sel(0x488e8868); //  55.07%
	InitDS_Sel(0x488e8b68); //  13.11%
	InitDS_Sel(0x4900484c); //  10.70%
	InitDS_Sel(0x4903004c); //  22.97%
	InitDS_Sel(0x49068068); //   8.85%
	InitDS_Sel(0x49068868); //   7.91%
	InitDS_Sel(0x49078068); //  15.89%
	InitDS_Sel(0x49079068); //  45.74%
	InitDS_Sel(0x490e8868); //  19.83%
	InitDS_Sel(0x4a47804c); //  30.59%
	InitDS_Sel(0x4a47904c); //  20.97%
	InitDS_Sel(0x4a80404c); //  42.08%
	InitDS_Sel(0x4a83004c); //  75.41%
	InitDS_Sel(0x4a87804c); //  23.46%
	InitDS_Sel(0x4a878068); //  25.53%
	InitDS_Sel(0x4a878868); //  20.03%
	InitDS_Sel(0x4a879068); //   9.94%
	InitDS_Sel(0x4b00404c); //  11.42%
	InitDS_Sel(0x4b004868); //   8.70%
	InitDS_Sel(0x4b07804c); //  20.68%
	InitDS_Sel(0x4b07884c); //  21.61%
	InitDS_Sel(0x4d0e8868); //   7.02%
	InitDS_Sel(0x5fe04058); //   5.85%
	InitDS_Sel(0x5fe04858); //  31.10%

	// sfex3

	InitDS_Sel(0x1fe6b068); //  13.61%
	InitDS_Sel(0x1fe6b868); //   5.51%
	InitDS_Sel(0x41268068); //   6.97%
	InitDS_Sel(0x41269068); //   6.32%
	InitDS_Sel(0x4886b068); //  13.15%
	InitDS_Sel(0x4886b868); //  29.78%
	InitDS_Sel(0x49079078); //   5.78%
	InitDS_Sel(0x9fe1004e); //   5.93%
	InitDS_Sel(0x9fe3004e); //  11.34%
	InitDS_Sel(0xc8804058); //   6.14%
	InitDS_Sel(0xc881004e); //   8.83%

	// gt4

	InitDS_Sel(0x1fe84864); //   7.28%
	InitDS_Sel(0x4881904d); //  11.12%
	InitDS_Sel(0x4887904d); //   9.99%
	InitDS_Sel(0x48884064); //  10.15%
	InitDS_Sel(0x488e8065); //  30.36%
	InitDS_Sel(0x488e8865); //  34.65%
	InitDS_Sel(0x488e9764); //  37.33%
	InitDS_Sel(0x4b1a8864); //   7.92%
	InitDS_Sel(0x5fe3804c); //   8.48%
	InitDS_Sel(0x5fe3904e); //   9.05%
	InitDS_Sel(0x5fee8864); //  16.67%
	InitDS_Sel(0x9fe5904d); //   8.89%
	InitDS_Sel(0x9fe7804d); //  11.18%
	InitDS_Sel(0xc88181d4); //  15.53%
	InitDS_Sel(0xc88191d4); //  19.28%
	InitDS_Sel(0xcb03804c); //  26.25%
	InitDS_Sel(0xcc80404d); //   5.93%
	InitDS_Sel(0xcc81904d); //  10.92%
	InitDS_Sel(0xcc83804d); //  12.73%
	InitDS_Sel(0xcd05804c); //  21.20%
	InitDS_Sel(0xd520404c); //  12.33%
	InitDS_Sel(0xdfe5804c); //   8.97%

	// katamary damacy

	InitDS_Sel(0xc88181cc); //   6.79%
	InitDS_Sel(0xc8904054); //  10.66%

	// grandia 3

	InitDS_Sel(0x41268060); //  18.14%
	InitDS_Sel(0x48868060); //  15.88%
	InitDS_Sel(0x48868360); //   5.80%
	InitDS_Sel(0x48868760); //  15.76%
	InitDS_Sel(0x48868860); //   7.13%
	InitDS_Sel(0x48868870); //  27.65%
	InitDS_Sel(0x48869060); //  10.18%
	InitDS_Sel(0x48869760); //  28.17%
	InitDS_Sel(0x48878060); //  16.38%
	InitDS_Sel(0x48879760); //  22.64%
	InitDS_Sel(0x488a8060); //  20.40%
	InitDS_Sel(0x488a8860); //  18.09%
	InitDS_Sel(0x488e8060); //   6.50%
	InitDS_Sel(0x488e8360); //   8.63%
	InitDS_Sel(0x488e8860); //  43.48%
	InitDS_Sel(0x488e8870); //   5.65%
	InitDS_Sel(0x488e8b60); //  34.62%
	InitDS_Sel(0x488e8f60); //  11.74%
	InitDS_Sel(0x488e9060); //   9.09%
	InitDS_Sel(0x488e9360); //   6.75%
	InitDS_Sel(0x488f8860); //  40.02%
	InitDS_Sel(0x4906804c); //  34.84%
	InitDS_Sel(0x49068860); //  16.96%
	InitDS_Sel(0x49078060); //   6.45%
	InitDS_Sel(0x49078860); //  22.80%
	InitDS_Sel(0x50368060); //  11.49%
	InitDS_Sel(0xcc81804c); //  43.58%
	InitDS_Sel(0xcc81904c); //  14.12%
	InitDS_Sel(0xcc839060); //  55.69%

	// rumble roses

	InitDS_Sel(0x1fe79064); //  10.59%
	InitDS_Sel(0x4c8e8864); //   5.38%
	InitDS_Sel(0xcc830064); //  43.47%

	// dmc

	InitDS_Sel(0x4423904c); //   7.90%
	InitDS_Sel(0x4427904c); //  25.37%
	InitDS_Sel(0x45204078); //   7.83%
	InitDS_Sel(0x4c87914c); //  42.27%
	InitDS_Sel(0x54204078); //   7.84%
	InitDS_Sel(0x9fe39058); //   7.33%
	InitDS_Sel(0x9fe78068); //   9.11%
	InitDS_Sel(0xc520404c); //   8.24%
	InitDS_Sel(0xc8804078); //   6.61%
	InitDS_Sel(0xc8830068); //  10.15%
	InitDS_Sel(0xc8878168); //  53.94%
	InitDS_Sel(0xc9078168); //  36.71%
	InitDS_Sel(0xcc43804c); //  17.76%
	InitDS_Sel(0xcc839068); //  11.83%
	InitDS_Sel(0xd4204078); //   6.35%

	// xenosaga 2

	InitDS_Sel(0x1fee804c); //  13.51%
	InitDS_Sel(0x1fee8064); //  18.76%
	InitDS_Sel(0x49079064); //  31.97%
	InitDS_Sel(0x4d069064); //   5.57%
	InitDS_Sel(0x51229064); //   7.30%
	InitDS_Sel(0x9fe58174); //   5.60%
	InitDS_Sel(0xc8804074); //  14.08%
	InitDS_Sel(0xc9078054); //   8.37%
	InitDS_Sel(0xc9079054); //  13.30%
	InitDS_Sel(0xcc404054); //  14.84%
	InitDS_Sel(0xcc804054); //  12.87%
	InitDS_Sel(0xcc839054); //  20.69%
	InitDS_Sel(0xcd004054); //  12.32%
	InitDS_Sel(0xdfe3804c); //   5.27%

	// nfs mw

	InitDS_Sel(0x1fe68068); //  10.83%
	InitDS_Sel(0x1fe68868); //  79.01%
	InitDS_Sel(0x1fe78068); //  20.74%
	InitDS_Sel(0x4b004068); //  13.74%
	InitDS_Sel(0x4b028068); //  12.42%
	InitDS_Sel(0x4b028868); //  11.91%
	InitDS_Sel(0xcc83804e); //   5.02%
	InitDS_Sel(0xd420404c); //  11.30%

	// berserk

	InitDS_Sel(0x4c8fb864); //   7.02%

	// castlevania

	InitDS_Sel(0x1fe78868); //  19.55%
	InitDS_Sel(0x48878868); //  87.66%
	InitDS_Sel(0x48884068); //  17.84%
	InitDS_Sel(0x49078868); //  25.88%
	InitDS_Sel(0x490e8068); //  12.36%
	InitDS_Sel(0x4d00407a); //  21.79%
	InitDS_Sel(0x9fe04068); //   8.94%
	InitDS_Sel(0x9fe3904e); //  14.50%
	InitDS_Sel(0xc8804068); //  13.13%
	InitDS_Sel(0xc8838058); //  18.48%
	InitDS_Sel(0xca80404c); //   8.59%
	InitDS_Sel(0xcc838058); //  71.04%
	InitDS_Sel(0xcc93904c); //  19.73%

	// okami

	InitDS_Sel(0x48878058); //  19.04%
	InitDS_Sel(0x48878168); // 155.22%
	InitDS_Sel(0x488e8068); //  27.58%
	InitDS_Sel(0x488e8168); //   5.45%
	InitDS_Sel(0x488e8968); //  34.71%
	InitDS_Sel(0x49078168); //  28.25%
	InitDS_Sel(0x9fe18058); //  10.97%
	InitDS_Sel(0x9fe1904c); //   8.21%
	InitDS_Sel(0xc5218058); //  26.60%
	InitDS_Sel(0xc881804c); //  11.83%
	InitDS_Sel(0xc8839158); //  22.18%
	InitDS_Sel(0xc8879168); //   6.43%
	InitDS_Sel(0xca83804c); //  16.68%
	InitDS_Sel(0xcc43904c); //  76.08%
	InitDS_Sel(0xdfe1904c); //  11.42%
	InitDS_Sel(0xdfe59068); //  64.33%
	InitDS_Sel(0xdfe7904c); //  27.67%

	// bully

	InitDS_Sel(0x110e8864); //  41.06%
	InitDS_Sel(0x110e8964); //  42.81%
	InitDS_Sel(0x1fe04864); //   5.32%
	InitDS_Sel(0x48804864); //  10.60%
	InitDS_Sel(0x48878b4c); //  13.64%
	InitDS_Sel(0x4d068364); //   8.75%
	InitDS_Sel(0x4d068864); //  33.00%
	InitDS_Sel(0x4d068b64); //  13.93%
	InitDS_Sel(0x4d07804c); //   8.65%
	InitDS_Sel(0x9fe04077); //   7.45%
	InitDS_Sel(0xc901004c); //  18.47%
	InitDS_Sel(0xca83904c); //  42.32%
	InitDS_Sel(0xd480404d); //  13.79%
	InitDS_Sel(0xd501904e); //  21.26%

	// culdcept

	InitDS_Sel(0x1fe04866); //   6.87%
	InitDS_Sel(0x1fe2a1e6); //  13.37%
	InitDS_Sel(0x1fe2a9e6); //  14.44%
	InitDS_Sel(0x1fe3a1e6); //  12.39%
	InitDS_Sel(0x488291e6); //   5.86%
	InitDS_Sel(0x4d02a1e6); //  12.21%
	InitDS_Sel(0x9fe39066); //  29.01%
	InitDS_Sel(0x9fe391e6); //  25.68%
	InitDS_Sel(0x9fe59066); //  17.30%
	InitDS_Sel(0x9fe991e6); //  18.01%

	// suikoden 5

	InitDS_Sel(0x00428868); //  11.84%
	InitDS_Sel(0x40428868); //  21.26%
	InitDS_Sel(0x4846804c); //  25.64%
	InitDS_Sel(0x4847804c); //  22.20%
	InitDS_Sel(0x48819368); //  26.62%
	InitDS_Sel(0x488a8b68); //  11.64%
	InitDS_Sel(0x49028868); //  11.82%
	InitDS_Sel(0x4d068868); //  31.18%

	// dq8

	InitDS_Sel(0x1fe0484c); //   5.43%
	InitDS_Sel(0x1fee8164); //  11.40%
	InitDS_Sel(0x48830064); //   7.50%
	InitDS_Sel(0x48869164); //   7.43%
	InitDS_Sel(0x49004064); //  78.04%
	InitDS_Sel(0x490e904c); //  19.01%
	InitDS_Sel(0x490f904c); //  15.78%
	InitDS_Sel(0x9103b04c); //   5.16%
	InitDS_Sel(0xc883004c); //  18.05%
	InitDS_Sel(0xc883914c); //   7.89%
	InitDS_Sel(0xc885804c); //  15.10%
	InitDS_Sel(0xc8878054); //  81.76%
	InitDS_Sel(0xc887904c); //  34.44%
	InitDS_Sel(0xc8c3804c); //  37.01%
	InitDS_Sel(0xcc83914c); //  10.40%
	InitDS_Sel(0xdfe3904e); //   5.61%

	// resident evil 4

	InitDS_Sel(0x1fe04057); //   6.23%
	InitDS_Sel(0x4883814c); //   6.85%
	InitDS_Sel(0x48868164); //   6.40%
	InitDS_Sel(0x4887814c); //  16.25%
	InitDS_Sel(0x48878164); //  66.20%
	InitDS_Sel(0x49078164); //   6.95%
	InitDS_Sel(0x4b068064); //   6.55%
	InitDS_Sel(0x4d07814c); //   8.50%
	InitDS_Sel(0x9fe18064); //  10.66%
	InitDS_Sel(0xc903904c); //  15.28%
	InitDS_Sel(0xcc879064); //  24.68%
	InitDS_Sel(0xcd004064); //  63.34%
	InitDS_Sel(0xcd03904c); //  16.52%
	InitDS_Sel(0xdfe5904c); //  12.35%

	// tomoyo after 

	InitDS_Sel(0x48478068); //   8.58%
	InitDS_Sel(0x48818068); //  23.60%
	InitDS_Sel(0x48878068); //  25.75%
	InitDS_Sel(0x49058068); //  17.22%
	InitDS_Sel(0x4a858068); //  16.11%
	InitDS_Sel(0x9fe38059); //  21.20%
	InitDS_Sel(0x9fe39059); //  20.53%

	// .hack redemption

	InitDS_Sel(0x48404074); //   7.33%
	InitDS_Sel(0x48404864); //   5.62%
	InitDS_Sel(0x48469064); //  12.02%
	InitDS_Sel(0x48478864); //   5.18%
	InitDS_Sel(0x48868364); //   7.31%
	InitDS_Sel(0x48869364); //  26.06%
	InitDS_Sel(0x488e9064); //  14.62%
	InitDS_Sel(0x488f9364); //  10.66%
	InitDS_Sel(0x49004074); //   7.07%
	InitDS_Sel(0x49004864); //   6.97%
	InitDS_Sel(0xc123004c); //  18.56%
	InitDS_Sel(0xc903004c); //  17.59%
	InitDS_Sel(0xcc41804c); //  11.57%
	InitDS_Sel(0xcd00404c); //   8.36%
	InitDS_Sel(0xdfe1004c); //  11.79%

	// wild arms 5

	InitDS_Sel(0x48804854); //  10.69%
	InitDS_Sel(0x4885884c); //   9.25%
	InitDS_Sel(0x488e8764); //  21.58%
	InitDS_Sel(0x488e8f64); //   9.52%
	InitDS_Sel(0x48c68864); //   5.69%
	InitDS_Sel(0xc845804c); //  13.47%
	InitDS_Sel(0xc845904c); //  12.48%
	InitDS_Sel(0xdfe39054); //  19.85%

	// shadow of the colossus

	InitDS_Sel(0x48868064); //   5.56%
	InitDS_Sel(0x48868b64); //  18.53%
	InitDS_Sel(0x48869064); //  27.01%
	InitDS_Sel(0x48879064); //  25.34%
	InitDS_Sel(0x488e8864); // 141.70%
	InitDS_Sel(0x488e9364); //   5.08%
	InitDS_Sel(0x490e8064); //  51.78%
	InitDS_Sel(0x490e8864); //   9.22%
	InitDS_Sel(0x490f8064); //   5.03%
	InitDS_Sel(0x4d004064); //  99.19%
	InitDS_Sel(0x9fe04064); //   8.61%
	InitDS_Sel(0x9fe1004d); //  10.53%
	InitDS_Sel(0xc8938064); //  42.57%
	InitDS_Sel(0xc8939064); //  15.38%
	InitDS_Sel(0xc900404c); //   8.59%
	InitDS_Sel(0xc9004064); //  59.82%
	InitDS_Sel(0xc903804c); //  36.96%
	InitDS_Sel(0x48858064); //  14.86%
	InitDS_Sel(0x48859364); //   5.16%
	InitDS_Sel(0x48878364); //  54.92%
	InitDS_Sel(0x48879364); //  21.03%
	InitDS_Sel(0xcc030064); //  20.97%

	// tales of redemption

	InitDS_Sel(0x4827804c); //  33.63%
	InitDS_Sel(0x48404054); //   8.79%
	InitDS_Sel(0x48478054); //  12.49%
	InitDS_Sel(0x48879164); //  30.25%
	InitDS_Sel(0x4c838064); //  17.71%
	InitDS_Sel(0x4c838854); //  12.15%
	InitDS_Sel(0xc88b9054); //   5.72%

	// digital devil saga

	InitDS_Sel(0x48404050); //   5.08%
	InitDS_Sel(0x48804050); //  13.51%
	InitDS_Sel(0x48878150); //   8.47%
	InitDS_Sel(0x48879060); //  10.22%
	InitDS_Sel(0x48879360); //   7.08%
	InitDS_Sel(0x48884870); //  15.96%
	InitDS_Sel(0x48904070); //  12.91%
	InitDS_Sel(0x49078360); //   5.47%
	InitDS_Sel(0x4a83804c); //   7.14%
	InitDS_Sel(0x4a878060); //   6.04%
	InitDS_Sel(0x9fe39070); //   5.97%
	InitDS_Sel(0xc8879060); //  10.09%
	InitDS_Sel(0xc907804c); //  20.53%

	// dbzbt2

	InitDS_Sel(0x4906884c); //  15.87%
	InitDS_Sel(0x4c904064); //   7.13%
	InitDS_Sel(0x543081e4); //   6.89%
	InitDS_Sel(0xc8804064); //  18.56%
	InitDS_Sel(0xc8878064); // 103.86%
	InitDS_Sel(0xc8878074); //  74.44%
	InitDS_Sel(0xc8879064); //  11.80%

	// dbzbt3

	InitDS_Sel(0x48868864); //  57.55%
	InitDS_Sel(0x48868964); //   9.17%
	InitDS_Sel(0x48878054); //  11.45%
	InitDS_Sel(0x48878864); //  63.04%
	InitDS_Sel(0x48879054); //  55.85%
	InitDS_Sel(0x48968864); //  14.54%
	InitDS_Sel(0x49068064); //   6.31%
	InitDS_Sel(0x49068864); //  12.58%
	InitDS_Sel(0x49078864); //  37.30%
	InitDS_Sel(0x4917804c); //   5.54%
	InitDS_Sel(0x4c469064); //  15.18%
	InitDS_Sel(0x4c80404c); //  29.01%
	InitDS_Sel(0x4c869064); //  16.34%
	InitDS_Sel(0xc8804054); //  26.19%
	InitDS_Sel(0xc881904c); //  14.57%
	InitDS_Sel(0xc883904c); //  15.84%
	InitDS_Sel(0xc88391cc); //  11.42%
	InitDS_Sel(0xc885904c); //  37.12%
	InitDS_Sel(0xc885904e); //  14.35%
	InitDS_Sel(0xc8859074); //  14.06%
	InitDS_Sel(0xc887804c); //   9.99%
	InitDS_Sel(0xc8879054); //  18.76%
	InitDS_Sel(0xc905904c); //  23.52%
	InitDS_Sel(0xca40404c); //   8.78%
	InitDS_Sel(0xca83004c); //  15.90%
	InitDS_Sel(0xcc45904e); //  29.99%
	InitDS_Sel(0xcc80404c); //  11.80%
	InitDS_Sel(0xcc80404e); //  37.55%
	InitDS_Sel(0xcc83004c); //  17.00%
	InitDS_Sel(0xcc8391cc); //  22.83%
	InitDS_Sel(0xcd03004c); //  31.59%
	InitDS_Sel(0xdfe1904e); //  14.28%
	InitDS_Sel(0x48468064); //   7.59%
	InitDS_Sel(0x48478064); //  19.34%
	InitDS_Sel(0x489081e4); //   6.18%
	InitDS_Sel(0x49268064); //  10.71%

	// dbz iw

	InitDS_Sel(0x1fe48864); //  11.82%
	InitDS_Sel(0x1fec8864); //  30.49%
	InitDS_Sel(0x48808064); //  10.31%
	InitDS_Sel(0x48848064); //   7.22%
	InitDS_Sel(0x48848864); //  23.80%
	InitDS_Sel(0x48849064); //   6.00%
	InitDS_Sel(0x48859064); //   6.29%
	InitDS_Sel(0x4904804c); //   9.48%
	InitDS_Sel(0x49058064); //   7.06%
	InitDS_Sel(0x49084064); //   7.95%
	InitDS_Sel(0x9fe19064); //  19.39%
	InitDS_Sel(0xc8858064); //  33.31%
	InitDS_Sel(0xc8859064); //  25.36%
	InitDS_Sel(0xc9058064); //   8.61%
	InitDS_Sel(0xc9084064); //  12.48%
	InitDS_Sel(0xc90d8064); //  16.16%
	InitDS_Sel(0xcd404054); //  21.86%

	// disgaea 2

	InitDS_Sel(0x1fe04064); //   5.64%
	InitDS_Sel(0x1fe69074); //   7.01%
	InitDS_Sel(0x48878964); //  55.59%

	// gradius 5

	InitDS_Sel(0x1fee8868); //  40.39%
	InitDS_Sel(0x1fee8968); //   5.24%
	InitDS_Sel(0x4881804c); //  21.16%
	InitDS_Sel(0x48868968); //  36.31%
	InitDS_Sel(0x48878968); //   6.78%
	InitDS_Sel(0x490e884c); //  32.03%
	InitDS_Sel(0x5fe68068); //  28.29%
	InitDS_Sel(0x5fe68968); //  13.35%
	InitDS_Sel(0x5fee8868); //  32.59%
	InitDS_Sel(0x5fee8968); //  14.03%
	InitDS_Sel(0x5ffe8868); //   6.10%

	// tales of abyss

	InitDS_Sel(0x1fe39368); //   8.05%
	InitDS_Sel(0x4121004c); //  23.01%
	InitDS_Sel(0x4885804c); //  12.31%
	InitDS_Sel(0x48868068); //   8.41%
	InitDS_Sel(0x4886934c); //   5.66%
	InitDS_Sel(0x4887834c); //   8.65%
	InitDS_Sel(0x48878368); //   9.17%
	InitDS_Sel(0x4887934c); //   5.16%
	InitDS_Sel(0x488c8868); //   6.06%
	InitDS_Sel(0x488c8b68); //  16.42%
	InitDS_Sel(0x488e8368); //  15.39%
	InitDS_Sel(0x48cf89e8); //  35.13%
	InitDS_Sel(0x4903834c); //  18.22%
	InitDS_Sel(0x4906834c); //   8.97%
	InitDS_Sel(0x49068b4c); //   8.99%
	InitDS_Sel(0x490e8b68); //   7.01%
	InitDS_Sel(0x490f89e8); //  37.03%
	InitDS_Sel(0x4d03914c); //  16.36%
	InitDS_Sel(0xdfe59078); //  19.07%

	// Gundam Seed Destiny OMNI VS ZAFT II PLUS 

	InitDS_Sel(0x1febb075); //  36.61%
	InitDS_Sel(0x48868875); //   8.59%
	InitDS_Sel(0x4887804d); //   6.51%
	InitDS_Sel(0x48878075); //   5.63%
	InitDS_Sel(0x48878375); //   9.61%
	InitDS_Sel(0x48878875); //  11.63%
	InitDS_Sel(0x48878b75); //  41.46%
	InitDS_Sel(0x488e8075); //  27.19%
	InitDS_Sel(0x488e8375); //  32.64%
	InitDS_Sel(0x488e8875); //  26.24%
	InitDS_Sel(0x488e8b75); //  36.39%
	InitDS_Sel(0x488e9075); //  19.17%
	InitDS_Sel(0x49068075); //  30.48%
	InitDS_Sel(0x4906884d); //   6.16%
	InitDS_Sel(0x490e8375); //  28.07%
	InitDS_Sel(0x490e8875); //  32.56%
	InitDS_Sel(0x490e8b75); //  51.42%
	InitDS_Sel(0x9fe19075); //  18.31%
	InitDS_Sel(0xc8818075); //  26.92%
	InitDS_Sel(0xc8819075); //  24.40%
	InitDS_Sel(0xc885804d); //  16.76%

	// nba 2k8

	InitDS_Sel(0x1fe04056); //  19.56%
	InitDS_Sel(0x1fe38966); //  23.69%
	InitDS_Sel(0x1fe39156); //  21.42%
	InitDS_Sel(0x1fe79056); //  24.33%
	InitDS_Sel(0x4883804e); //  24.58%
	InitDS_Sel(0x48838166); //   5.58%
	InitDS_Sel(0x48868166); //  13.24%
	InitDS_Sel(0x48868866); //  25.67%
	InitDS_Sel(0x48868966); //  19.61%
	InitDS_Sel(0x48879066); //   7.34%
	InitDS_Sel(0x48879166); //  14.62%
	InitDS_Sel(0x49028966); //   6.37%
	InitDS_Sel(0x49068066); //  25.16%
	InitDS_Sel(0x49068966); //   9.38%
	InitDS_Sel(0x49068976); //  46.00%
	InitDS_Sel(0x5fe48866); //  10.67%
	InitDS_Sel(0x5fe68866); //  18.90%
	InitDS_Sel(0x5fe79066); //  25.19%

	// onimusha 3

	InitDS_Sel(0x1fee004c); //   9.89%
	InitDS_Sel(0x1fee0868); //  47.83%
	InitDS_Sel(0x4903884c); //  37.05%
	InitDS_Sel(0x4c878168); //  11.00%
	InitDS_Sel(0x4d05884c); //   6.81%
	InitDS_Sel(0x5fe04078); //  31.14%
	InitDS_Sel(0x9fe18068); //   5.36%
	InitDS_Sel(0xc8839168); //   7.01%
	InitDS_Sel(0xcd004068); //  28.15%
	InitDS_Sel(0xd125904c); //   5.60%
	InitDS_Sel(0xd425904c); //   5.88%
	InitDS_Sel(0xdfe78368); //   6.56%
	InitDS_Sel(0xdfe79368); //  11.21%

	// resident evil code veronica

	InitDS_Sel(0x1fee8168); //  11.91%
	InitDS_Sel(0x48804b68); //   7.61%
	InitDS_Sel(0x9fe39068); //  17.32%
	InitDS_Sel(0x9fe79068); //  24.37%
	InitDS_Sel(0x9fe79168); //  25.25%
	InitDS_Sel(0xc8878368); //   9.51%
	InitDS_Sel(0xcc819058); //  22.37%

	// armored core 3

	InitDS_Sel(0x1fe84074); //  18.36%
	InitDS_Sel(0x1fee0874); //  50.89%
	InitDS_Sel(0x48404854); //   5.39%
	InitDS_Sel(0x48858054); //  13.21%
	InitDS_Sel(0x48878074); //   7.66%
	InitDS_Sel(0x488781d4); //   5.36%
	InitDS_Sel(0x48878874); //  12.35%
	InitDS_Sel(0x488791d4); //   5.12%
	InitDS_Sel(0x488e8074); //  17.11%
	InitDS_Sel(0x49058054); //   6.01%
	InitDS_Sel(0x49059054); //   9.52%
	InitDS_Sel(0x490e8074); //  51.58%
	InitDS_Sel(0x4c4e8074); //  15.89%
	InitDS_Sel(0x4d0e8074); //   5.73%
	InitDS_Sel(0x9fe04074); //  12.94%
	InitDS_Sel(0xc8404054); //   9.27%
	InitDS_Sel(0xc881004c); //   9.63%
	InitDS_Sel(0xc8850054); //   9.42%

	// aerial planet

	InitDS_Sel(0x48478164); //   9.96%
	InitDS_Sel(0x4847914c); //  12.69%
	InitDS_Sel(0x4886814c); //  25.36%
	InitDS_Sel(0x4887914c); //   7.49%
	InitDS_Sel(0x488e814c); //  19.25%
	InitDS_Sel(0x488f8164); //  16.66%
	InitDS_Sel(0x4c868074); //  42.55%
	InitDS_Sel(0x4c868934); //  18.04%
	InitDS_Sel(0x4c8e8074); //  11.29%
	InitDS_Sel(0x4c8e8874); //  19.85%
	InitDS_Sel(0x4cc0404c); //  15.74%
	InitDS_Sel(0x4d068074); //   9.04%
	InitDS_Sel(0xc820404c); //  15.19%
	InitDS_Sel(0xcc83804c); //  49.18%

	// one piece grand battle 3

	InitDS_Sel(0x48839054); //  20.48%
	InitDS_Sel(0x49068174); //   5.30%
	InitDS_Sel(0x49068874); //  34.91%
	InitDS_Sel(0x49068964); //  16.76%
	InitDS_Sel(0x49078174); //  15.70%
	InitDS_Sel(0xcac0404c); //   8.20%
	InitDS_Sel(0xcc41904c); //  11.07%
	InitDS_Sel(0xcc4190cc); //   6.72%
	InitDS_Sel(0xd321914c); //   6.72%

	// one piece grand adventure

	InitDS_Sel(0x1fe0404c); //   6.83%
	InitDS_Sel(0x48849164); //  44.38%
	InitDS_Sel(0x48879154); //   8.48%
	InitDS_Sel(0xc421814c); //   6.58%
	InitDS_Sel(0xc843b04c); //  18.58%

	// shadow hearts

	InitDS_Sel(0x48804868); //  69.52%
	InitDS_Sel(0x48868078); //  11.74%
	InitDS_Sel(0x48868778); //  10.34%
	InitDS_Sel(0x49004058); //   7.72%
	InitDS_Sel(0x49030058); //  41.88%
	InitDS_Sel(0x4c870878); //   7.00%
	InitDS_Sel(0x9fe3004c); //  21.51%
	InitDS_Sel(0xc881904e); //   5.22%
	InitDS_Sel(0xc8819168); //  14.01%
	InitDS_Sel(0xc8830058); //  13.22%
	InitDS_Sel(0xc8839058); //  12.81%
	InitDS_Sel(0xc8878058); //  11.50%
	InitDS_Sel(0xc8879058); //   6.63%
	InitDS_Sel(0xc9039058); //  12.58%
	InitDS_Sel(0xc9078068); //  30.88%

	// the punisher

	InitDS_Sel(0x48420864); //   7.51%
	InitDS_Sel(0x48468864); //  12.84%
	InitDS_Sel(0x48868764); //  22.52%
	InitDS_Sel(0x48868f64); //  44.09%
	InitDS_Sel(0x4886b764); //   5.67%
	InitDS_Sel(0x4886bf64); //  24.55%
	InitDS_Sel(0x4906904c); //  13.98%
	InitDS_Sel(0x4d068f64); //  11.33%
	InitDS_Sel(0x5fe0404e); //   5.05%
	InitDS_Sel(0x5fe68864); //  16.21%
	InitDS_Sel(0x5fe68f64); //  53.26%
	InitDS_Sel(0xc880474c); //  21.71%
	InitDS_Sel(0xc883974c); //   6.02%
	InitDS_Sel(0xc887874c); //  10.22%
	InitDS_Sel(0xc887974c); //   8.51%
	InitDS_Sel(0xdfe3974c); //  17.48%
	InitDS_Sel(0x4906874c); //   6.30%
	InitDS_Sel(0x49068764); //  13.02%
	InitDS_Sel(0xc9038764); //   9.38%
	InitDS_Sel(0xc9078064); //  36.67%

	// guitar hero

	InitDS_Sel(0x1fe0407b); //   7.43%
	InitDS_Sel(0x1fe6887a); //  10.06%
	InitDS_Sel(0x4880484e); //  33.80%
	InitDS_Sel(0x4880487a); //  15.13%
	InitDS_Sel(0x4886804e); //  21.96%
	InitDS_Sel(0x4886807a); //   9.20%
	InitDS_Sel(0x4886854e); //   8.79%
	InitDS_Sel(0x4886857a); //  10.05%
	InitDS_Sel(0x4886887a); //  42.97%
	InitDS_Sel(0x48868d4e); //   5.41%
	InitDS_Sel(0x4886907a); //  18.66%
	InitDS_Sel(0x4886956a); //   7.54%
	InitDS_Sel(0x4887804e); //  24.39%
	InitDS_Sel(0x4887854e); //   9.97%
	InitDS_Sel(0x4887857a); //  29.02%
	InitDS_Sel(0x4887887a); //  31.58%
	InitDS_Sel(0x48878d5a); //   7.12%
	InitDS_Sel(0x4887904e); //  22.78%
	InitDS_Sel(0x4887917a); //  26.89%
	InitDS_Sel(0x4887954e); //  14.99%
	InitDS_Sel(0x4887957a); //  27.93%
	InitDS_Sel(0x488e887a); //  77.76%
	InitDS_Sel(0x4900487a); //  18.48%
	InitDS_Sel(0x4906806a); //  17.69%
	InitDS_Sel(0x4d06a06a); //  21.65%
	InitDS_Sel(0x4d06a86a); //  16.11%
	InitDS_Sel(0x4d0ea06a); //   5.65%
	InitDS_Sel(0xcd07806a); //   8.60%
	InitDS_Sel(0x488e8d7a); //   5.26%
	InitDS_Sel(0x9503204e); //  22.34%
	InitDS_Sel(0x9fe3906a); //   9.16%
	InitDS_Sel(0xcd03204e); //  69.88%

	// ico

	InitDS_Sel(0x1fe28060); //  14.34%
	InitDS_Sel(0x1fe68860); //  39.24%
	InitDS_Sel(0x48868b60); //  91.17%
	InitDS_Sel(0x49068b60); //   7.39%
	InitDS_Sel(0x4d004060); //  48.88%
	InitDS_Sel(0x4d028060); //  14.64%
	InitDS_Sel(0x4d068360); //  15.33%
	InitDS_Sel(0x4d068860); //  20.67%
	InitDS_Sel(0x4d068b60); // 230.02%
	InitDS_Sel(0x4d078360); //   8.81%
	InitDS_Sel(0x9fe04060); //   6.07%
	InitDS_Sel(0x9fe380cc); //  13.34%
	InitDS_Sel(0x9fe3a04c); //  40.11%
	InitDS_Sel(0xc8859060); //  11.96%
	InitDS_Sel(0xc893814c); //  33.66%
	InitDS_Sel(0xc9004060); //   6.19%

	// kuon

	InitDS_Sel(0x1fee0865); //  15.33%
	InitDS_Sel(0x48860065); //   6.36%
	InitDS_Sel(0x48860865); //  21.12%
	InitDS_Sel(0x48868365); //  14.57%
	InitDS_Sel(0x48868b65); //  14.32%
	InitDS_Sel(0x48870065); //  25.94%
	InitDS_Sel(0x48878b65); //  13.59%
	InitDS_Sel(0x488e0865); //  34.62%
	InitDS_Sel(0x488e0b65); //  21.01%
	InitDS_Sel(0x488e8b65); //  11.71%
	InitDS_Sel(0x4907004d); //  26.20%
	InitDS_Sel(0x4907084d); //   5.18%
	InitDS_Sel(0x4c429065); //  23.90%
	InitDS_Sel(0x4d068365); //   7.66%
	InitDS_Sel(0xc847004d); //  11.21%
	InitDS_Sel(0xc887004d); //  16.73%
	InitDS_Sel(0xc887904d); //   6.03%
	InitDS_Sel(0xcc83904d); //  15.84%

	// hxh

	// grandia extreme

	InitDS_Sel(0x1fe3884c); //  30.28%
	InitDS_Sel(0x45269070); //   5.92%
	InitDS_Sel(0x452e9070); //   7.18%
	InitDS_Sel(0x48868070); //  13.48%
	InitDS_Sel(0x48869070); //  26.50%
	InitDS_Sel(0x48878370); //  27.62%
	InitDS_Sel(0x48879070); //  26.45%
	InitDS_Sel(0x48879370); //  15.48%
	InitDS_Sel(0x4888404c); //  12.35%
	InitDS_Sel(0x48884050); //  15.31%
	InitDS_Sel(0x488e8b70); //  47.58%
	InitDS_Sel(0x488e9370); //  16.74%
	InitDS_Sel(0x9fe3934c); //  18.59%
	InitDS_Sel(0xcc81934c); //  63.56%

	// enthusia

	InitDS_Sel(0x1fe04854); //  23.37%
	InitDS_Sel(0x1fee0864); //  30.31%
	InitDS_Sel(0x48860f64); //  15.00%
	InitDS_Sel(0x488e804c); //  10.47%
	InitDS_Sel(0x4a46884c); //  14.52%
	InitDS_Sel(0x4b020864); //  20.34%
	InitDS_Sel(0x4b060864); //  18.55%
	InitDS_Sel(0x9fe04067); //   5.04%
	InitDS_Sel(0xcd40404c); //  11.00%

	// ys 1/2 eternal story

	// bloody roar

	InitDS_Sel(0x48810068); //  23.83%
	InitDS_Sel(0x48848068); //  60.93%
	InitDS_Sel(0x488789e8); //  28.90%
	InitDS_Sel(0x488791e8); //   5.13%
	InitDS_Sel(0x49004068); //  21.53%
	InitDS_Sel(0x49004868); //  20.45%
	InitDS_Sel(0x49018368); //  13.81%
	InitDS_Sel(0x49019368); //  13.32%
	InitDS_Sel(0x49020b4c); //  15.90%
	InitDS_Sel(0x4b068068); //  15.38%

	// ferrari f355 challenge

	InitDS_Sel(0x489e8b64); //   6.29%
	InitDS_Sel(0x5fe04064); //  11.66%
	InitDS_Sel(0x5fe04068); //  13.72%
	InitDS_Sel(0x5fe04868); //   6.54%
	InitDS_Sel(0x5fe60064); //  13.47%
	InitDS_Sel(0x5fee0064); //  12.73%
	InitDS_Sel(0x5fee0864); //  27.22%
	InitDS_Sel(0x5feeb864); //   7.27%
	InitDS_Sel(0x5fef0064); //   6.96%
	InitDS_Sel(0x5ff60064); //  18.27%
	InitDS_Sel(0x5ffe0064); //  18.52%
	InitDS_Sel(0x5ffe0864); //  32.08%
	InitDS_Sel(0xc8858168); //  39.47%
	InitDS_Sel(0xc8878068); //  21.76%
	InitDS_Sel(0xc890404c); //   6.69%

	// king of fighters xi

	InitDS_Sel(0x488589e8); // 118.65%
	InitDS_Sel(0x488591e8); //  53.51%
	InitDS_Sel(0xf4819050); //  11.50%

	// mana khemia

	InitDS_Sel(0x488e8369); // 100.59%
	InitDS_Sel(0xc9038059); //  29.02%
	InitDS_Sel(0xc907804d); //  55.66%
	InitDS_Sel(0xc90f8369); //   6.99%

	// ar tonelico 2

	InitDS_Sel(0x484f8369); //   6.67%
	InitDS_Sel(0x488e8069); //  18.92%
	InitDS_Sel(0x488e9069); //  30.64%
	InitDS_Sel(0x488e9369); //  93.72%
	InitDS_Sel(0x488f8069); //   5.15%
	InitDS_Sel(0x488f8369); //  19.12%
	InitDS_Sel(0x49079059); //  74.64%
	InitDS_Sel(0xc8804059); //  13.31%
	InitDS_Sel(0xc8859059); //  13.51%
	InitDS_Sel(0xc887804d); //   5.79%

	// rouge galaxy

	InitDS_Sel(0x484e8164); //  38.29%
	InitDS_Sel(0x490a8164); //   6.45%
	InitDS_Sel(0x9fe1804c); //   8.57%
	InitDS_Sel(0xc8858154); //   6.83%
	InitDS_Sel(0xdff0404c); //   8.09%

	// mobile suit gundam seed battle assault 3

	InitDS_Sel(0x48804054); //  23.35%
	InitDS_Sel(0x488e88f4); //  11.56%
	InitDS_Sel(0x49004054); //  11.59%
	InitDS_Sel(0x49004854); //   5.41%
	InitDS_Sel(0x490e8164); //  17.59%
	InitDS_Sel(0x490e9074); //   6.56%
	InitDS_Sel(0x490e9164); //  14.35%
	InitDS_Sel(0x5fee8174); //   5.56%
	InitDS_Sel(0x5fee8874); //  33.51%
	InitDS_Sel(0x5fee8974); //  14.23%
	InitDS_Sel(0xc88390cc); //  19.25%
	InitDS_Sel(0xcc81004d); //  10.11%

	// hajime no ippo all stars

	InitDS_Sel(0x4881004c); //  18.60%
	InitDS_Sel(0x48828b68); //  27.89%
	InitDS_Sel(0x48829368); //  29.39%
	InitDS_Sel(0x48848868); //  19.61%
	InitDS_Sel(0x48858068); //  25.24%
	InitDS_Sel(0x48858368); //   6.23%
	InitDS_Sel(0x48859068); //  21.74%
	InitDS_Sel(0x48868b68); //  37.30%
	InitDS_Sel(0x48879368); //  11.75%
	InitDS_Sel(0x488e9068); //  15.88%
	InitDS_Sel(0x4b068868); //  10.33%
	InitDS_Sel(0x4b0e8868); //  13.65%
	InitDS_Sel(0x48858868); //   5.38%
	InitDS_Sel(0x48858b68); //   5.72%
	InitDS_Sel(0x48c28368); //  10.86%
	InitDS_Sel(0x49028368); //  10.40%

	// hajime no ippo 2

	InitDS_Sel(0x1fe04068); //  11.18%
	InitDS_Sel(0x48839368); //  29.25%
	InitDS_Sel(0x488c8968); //   5.46%
	InitDS_Sel(0x488e9368); //   5.97%
	InitDS_Sel(0x49028b68); //  35.91%
	InitDS_Sel(0x4d0e8068); //   8.25%

	// virtual tennis 2

	InitDS_Sel(0x1100404d); //   7.08%
	InitDS_Sel(0x48859065); //   8.71%
	InitDS_Sel(0x48868075); //  19.93%
	InitDS_Sel(0x4c868875); //   8.50%
	InitDS_Sel(0x4c8781f5); //   8.59%
	InitDS_Sel(0x9540404d); //   8.65%
	InitDS_Sel(0xc8859065); //   7.41%

	// crash wrath of cortex

	InitDS_Sel(0x1fe20864); //  12.48%
	InitDS_Sel(0x1fe68864); //  32.98%
	InitDS_Sel(0x1fe78064); //   7.49%
	InitDS_Sel(0x48828f64); //  12.90%
	InitDS_Sel(0x48838364); //   5.97%
	InitDS_Sel(0x48838f64); //   6.07%
	InitDS_Sel(0x49028f64); //  14.07%
	InitDS_Sel(0x49038f64); //  11.50%
	InitDS_Sel(0x9fe5904c); //   5.45%
	InitDS_Sel(0xc840404c); //   5.82%
	InitDS_Sel(0xc8818364); //   6.96%
	InitDS_Sel(0xc9030064); //  25.15%
	InitDS_Sel(0xca838364); //   6.51%
	InitDS_Sel(0xcd05834c); //   7.35%

	// sbam 2

	// remember 11

	// prince of tennis

	// ar tonelico

	InitDS_Sel(0x48868369); //  15.13%
	InitDS_Sel(0x48869369); //  16.64%
	InitDS_Sel(0x48878069); //  28.59%
	InitDS_Sel(0x48878369); //  10.05%
	InitDS_Sel(0x48879069); //  28.56%
	InitDS_Sel(0x48879369); //  17.10%
	InitDS_Sel(0x488f9369); //   5.35%
	InitDS_Sel(0x49078b69); //  48.28%
	InitDS_Sel(0x490f8069); //  39.04%
	InitDS_Sel(0x490f8369); // 128.93%
	InitDS_Sel(0xc8804069); //  14.41%
	InitDS_Sel(0xc881804d); //  13.77%
	InitDS_Sel(0xc8819059); //  19.64%
	InitDS_Sel(0xc88f9369); //  70.39%
	InitDS_Sel(0xc9038069); // 218.42%
	InitDS_Sel(0xc90f804d); //  41.70%

	// dbz sagas

	InitDS_Sel(0x48828964); //  40.29%

	// tourist trophy

	InitDS_Sel(0x1fe84064); //  11.57%
	InitDS_Sel(0x488a8064); //  15.87%
	InitDS_Sel(0x488e9065); //  16.45%
	InitDS_Sel(0x5fe3904c); //   7.75%
	InitDS_Sel(0x5fe84064); //  10.70%
	InitDS_Sel(0x5fee9064); //  11.56%
	InitDS_Sel(0x9fe181d4); //  10.21%
	InitDS_Sel(0xcc87904d); //  33.79%

	// svr2k8

	InitDS_Sel(0x1fe79066); //  15.77%
	InitDS_Sel(0x48804b4c); //   8.25%
	InitDS_Sel(0x48838164); //  36.19%
	InitDS_Sel(0x4887b864); //  27.81%
	InitDS_Sel(0x490e8364); //  44.11%

	// tokyo bus guide

	InitDS_Sel(0x1fe04870); //  11.69%
	InitDS_Sel(0x1fee8170); //  14.67%
	InitDS_Sel(0x1fee8970); //  80.07%
	InitDS_Sel(0x1fee89f0); //  28.98%
	InitDS_Sel(0x48804870); //  16.61%
	InitDS_Sel(0x488a8850); //  19.66%
	InitDS_Sel(0xc8804070); //   9.86%
	InitDS_Sel(0xc8879070); //  29.38%
	InitDS_Sel(0xc88791f0); //  11.44%

	// 12riven

	// xenosaga

	InitDS_Sel(0x1fe38054); //  60.59%
	InitDS_Sel(0x1fe38074); //  25.26%
	InitDS_Sel(0x4c468064); //  23.70%
	InitDS_Sel(0x4c818134); //   7.20%
	InitDS_Sel(0x4c868864); //  11.57%
	InitDS_Sel(0x4c938064); //  53.69%
	InitDS_Sel(0x4d004864); //   8.70%
	InitDS_Sel(0x4d084864); //  13.85%
	InitDS_Sel(0x5fe04074); //  52.81%
	InitDS_Sel(0x9fe19074); //  16.94%
	InitDS_Sel(0x9fe39174); //  21.89%
	InitDS_Sel(0xcc819074); //  20.62%
	InitDS_Sel(0xcc839074); //  26.80%
	InitDS_Sel(0xd5204064); //  14.16%
	InitDS_Sel(0xdfe04074); //  14.86%

	// mgs3s1

	InitDS_Sel(0x484e8864); //  14.51%
	InitDS_Sel(0x488e8364); //  39.86%
	InitDS_Sel(0x4b0e8b64); //  14.57%
	InitDS_Sel(0xc8804364); //  12.93%
	InitDS_Sel(0xc8819364); //  20.22%
	InitDS_Sel(0xc8878364); //  15.28%
	InitDS_Sel(0xcc830074); //  75.61%
	InitDS_Sel(0xcd404074); //   9.54%
	InitDS_Sel(0x482e8864); //  14.97%
	InitDS_Sel(0x48828ae4); //  13.37%
	InitDS_Sel(0x488f8064); //  30.49%
	InitDS_Sel(0x490e9364); //  41.95%
	InitDS_Sel(0x49268864); //   6.24%
	InitDS_Sel(0x4b0e8864); //   8.42%
	InitDS_Sel(0x9fe1804d); //   9.26%
	InitDS_Sel(0x9fe7904c); //  12.19%
	InitDS_Sel(0xc8830074); //  21.08%

	// god of war

	// gta sa

	InitDS_Sel(0x48810064); //   9.47%
	InitDS_Sel(0x4887884c); //   6.78%
	InitDS_Sel(0x488f8864); //  11.05%
	InitDS_Sel(0x4d078864); //  10.48%
	InitDS_Sel(0x4d0c8064); //   6.42%
	InitDS_Sel(0x4d0c8864); //   5.48%
	InitDS_Sel(0x9fe1004c); //  12.95%
	InitDS_Sel(0x9fe1904d); //  12.55%
	InitDS_Sel(0x9fe1904e); //  10.98%

	// hunting ground

	InitDS_Sel(0x1fe689e4); //  48.20%
	InitDS_Sel(0x48804064); //  14.53%
	InitDS_Sel(0x488689e4); //   6.96%
	InitDS_Sel(0x5fe59064); //   5.56%
	InitDS_Sel(0x9fe7904d); //  19.72%
	InitDS_Sel(0x9fe79064); //  21.53%
	InitDS_Sel(0xc843904c); //  16.51%
	InitDS_Sel(0xc88791e4); //   9.59%
	InitDS_Sel(0xcc23804c); //  16.15%
	InitDS_Sel(0xcd01904c); //  97.26%
	InitDS_Sel(0xdfe04064); //  14.96%
	InitDS_Sel(0xdff1904e); //  14.66%

	// odin sphere

	InitDS_Sel(0x4880404d); //   6.81%
	InitDS_Sel(0x4881004d); //  18.90%
	InitDS_Sel(0x4885804d); //   5.55%
	InitDS_Sel(0x4885904c); //   9.92%
	InitDS_Sel(0x488f904c); //   6.29%
	InitDS_Sel(0x488f904d); //   6.25%
	InitDS_Sel(0x4907884d); //   8.37%

	// kingdom hearts re:com

	InitDS_Sel(0x1fe04078); //   5.22%
	InitDS_Sel(0x4880494c); //   7.16%
	InitDS_Sel(0x4885814c); //  15.39%
	InitDS_Sel(0x4887894c); //   8.46%
	InitDS_Sel(0x4907894c); //  16.66%
	InitDS_Sel(0x490e8974); // 102.78%
	InitDS_Sel(0x9fe10054); //  12.50%
	InitDS_Sel(0x9fe3814d); //  12.91%
	InitDS_Sel(0xd120404c); //   8.42%

	// hyper street fighter 2 anniversary edition

	InitDS_Sel(0xc8804060); //  27.72%
	InitDS_Sel(0xc88391e0); //  70.48%
	InitDS_Sel(0xc88791e0); //  16.48%

	// aura for laura

	InitDS_Sel(0x1fe04070); //  89.38%
	InitDS_Sel(0x1fe38070); //  29.97%
	InitDS_Sel(0x1fe3904c); //  18.79%
	InitDS_Sel(0x1fe39050); //   5.48%
	InitDS_Sel(0x1fe68070); //  18.85%
	InitDS_Sel(0x1fe6904c); //  10.03%
	InitDS_Sel(0x1fe79070); //  73.43%
	InitDS_Sel(0x1fefb04c); //  17.16%
	InitDS_Sel(0x9fe19050); //   7.40%
	InitDS_Sel(0x9fe78050); //  11.67%
	InitDS_Sel(0xc523804c); //  78.85%
	InitDS_Sel(0xcc818050); //  45.96%

	#endif
}

IDrawScanline::DrawScanlinePtr GSDrawScanline::GSDrawScanlineMap::GetDefaultFunction(DWORD dw)
{
	GSScanlineSelector sel;

	sel.dw = dw;

	return m_default[sel.fpsm][sel.zpsm][sel.ztst][sel.iip];
}

void GSDrawScanline::GSDrawScanlineMap::PrintStats()
{
	__super::PrintStats();

	if(FILE* fp = fopen("c:\\1.txt", "w"))
	{
		POSITION pos = m_map_active.GetHeadPosition();

		while(pos)
		{
			DWORD dw;
			ActivePtr* p;
			
			m_map_active.GetNextAssoc(pos, dw, p);

			if(m_map.Lookup(dw))
			{
				continue;
			}

			GSScanlineSelector sel;

			sel.dw = dw;

			if(p->frames > 30 && !sel.IsSolidRect())
			{
				int tpf = (int)((p->ticks / p->frames) * 10000 / (3000000000 / 60)); // 3 GHz, 60 fps

				if(tpf >= 500)
				{
					_ftprintf(fp, _T("InitDS_Sel(0x%08x); // %6.2f%%\n"), sel.dw, (float)tpf / 100);
				}
			}
		}

		fclose(fp);
	}
}

//

GSDrawScanline::GSSetupPrimMap::GSSetupPrimMap()
{
	#define InitSP_IIP(zbe, fge, tme, fst, iip) \
		m_default[zbe][fge][tme][fst][iip] = (SetupPrimPtr)&GSDrawScanline::SetupPrim<zbe, fge, tme, fst, iip>; \

	#define InitSP_FST(zbe, fge, tme, fst) \
		InitSP_IIP(zbe, fge, tme, fst, 0) \
		InitSP_IIP(zbe, fge, tme, fst, 1) \

	#define InitSP_TME(zbe, fge, tme) \
		InitSP_FST(zbe, fge, tme, 0) \
		InitSP_FST(zbe, fge, tme, 1) \

	#define InitSP_FGE(zbe, fge) \
		InitSP_TME(zbe, fge, 0) \
		InitSP_TME(zbe, fge, 1) \

	#define InitSP_ZBE(zbe) \
		InitSP_FGE(zbe, 0) \
		InitSP_FGE(zbe, 1) \

	InitSP_ZBE(0);
	InitSP_ZBE(1);
}

IDrawScanline::SetupPrimPtr GSDrawScanline::GSSetupPrimMap::GetDefaultFunction(DWORD dw)
{
	DWORD zbe = (dw >> 0) & 1;
	DWORD fge = (dw >> 1) & 1;
	DWORD tme = (dw >> 2) & 1;
	DWORD fst = (dw >> 3) & 1;
	DWORD iip = (dw >> 4) & 1;

	return m_default[zbe][fge][tme][fst][iip];
}

//

const GSVector4 GSDrawScanline::m_shift[4] = 
{
	GSVector4(0.0f, 1.0f, 2.0f, 3.0f),
	GSVector4(-1.0f, 0.0f, 1.0f, 2.0f),
	GSVector4(-2.0f, -1.0f, 0.0f, 1.0f),
	GSVector4(-3.0f, -2.0f, -1.0f, 0.0f),
};

const GSVector4i GSDrawScanline::m_test[8] = 
{
	GSVector4i::zero(),
	GSVector4i(0xffffffff, 0x00000000, 0x00000000, 0x00000000),
	GSVector4i(0xffffffff, 0xffffffff, 0x00000000, 0x00000000),
	GSVector4i(0xffffffff, 0xffffffff, 0xffffffff, 0x00000000),
	GSVector4i(0x00000000, 0xffffffff, 0xffffffff, 0xffffffff),
	GSVector4i(0x00000000, 0x00000000, 0xffffffff, 0xffffffff),
	GSVector4i(0x00000000, 0x00000000, 0x00000000, 0xffffffff),
	GSVector4i::zero(),
};
