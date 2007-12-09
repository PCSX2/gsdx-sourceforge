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

#include "StdAfx.h"
#include "GSTextureCache.h"
#include "GSRendererHW.h"
#include "resource.h"

GSTextureCache::GSTextureCache(GSRendererHW* renderer)
	: m_renderer(renderer)
{
	m_nativeres = !!AfxGetApp()->GetProfileInt(_T("Settings"), _T("nativeres"), FALSE);
}

GSTextureCache::~GSTextureCache()
{
	RemoveAll();
}

void GSTextureCache::RemoveAll()
{
	while(m_rt.GetCount()) delete m_rt.RemoveHead();
	while(m_ds.GetCount()) delete m_ds.RemoveHead();
	while(m_tex.GetCount()) delete m_tex.RemoveHead();
}

GSTextureCache::GSRenderTarget* GSTextureCache::GetRenderTarget(const GIFRegTEX0& TEX0, int w, int h, bool fb)
{
	POSITION pos = m_tex.GetHeadPosition();

	while(pos)
	{
		POSITION cur = pos;

		GSTexture* t = m_tex.GetNext(pos);

		if(HasSharedBits(TEX0.TBP0, TEX0.PSM, t->m_TEX0.TBP0, t->m_TEX0.PSM))
		{
			m_tex.RemoveAt(cur);

			delete t;
		}
	}

	GSRenderTarget* rt = NULL;

	if(rt == NULL)
	{
		for(POSITION pos = m_rt.GetHeadPosition(); pos; m_rt.GetNext(pos))
		{
			GSRenderTarget* rt2 = m_rt.GetAt(pos);

			if(rt2->m_TEX0.TBP0 == TEX0.TBP0)
			{
				m_rt.MoveToHead(pos);

				rt = rt2;

				if(!fb) rt->m_TEX0 = TEX0;

				rt->Update();

				break;
			}
		}
	}

	if(rt == NULL && fb)
	{
		// HACK: try to find something close to the base pointer

		for(POSITION pos = m_rt.GetHeadPosition(); pos; m_rt.GetNext(pos))
		{
			GSRenderTarget* rt2 = m_rt.GetAt(pos);

			if(rt2->m_TEX0.TBP0 <= TEX0.TBP0 && TEX0.TBP0 < rt2->m_TEX0.TBP0 + 0xe00 && (!rt || rt2->m_TEX0.TBP0 >= rt->m_TEX0.TBP0))
			{
				rt = rt2;
			}
		}

		if(rt)
		{
			rt->Update();
		}
	}

	if(rt == NULL)
	{
		rt = new GSRenderTarget(this);

		rt->m_TEX0 = TEX0;

		if(!rt->Create(w, h))
		{
			delete rt;

			return NULL;
		}

		m_rt.AddHead(rt);
	}

	if(!m_nativeres)
	{
		rt->m_scale.x = (float)w / (m_renderer->GetFramePos().cx + rt->m_TEX0.TBW * 64);
		rt->m_scale.y = (float)h / (m_renderer->GetFramePos().cy + m_renderer->GetDisplaySize().cy);
	}

	if(!fb)
	{
		rt->m_used = true;
	}

	return rt;
}

GSTextureCache::GSDepthStencil* GSTextureCache::GetDepthStencil(const GIFRegTEX0& TEX0, int w, int h)
{
	POSITION pos = m_tex.GetHeadPosition();

	while(pos)
	{
		POSITION cur = pos;

		GSTexture* t = m_tex.GetNext(pos);

		if(HasSharedBits(TEX0.TBP0, TEX0.PSM, t->m_TEX0.TBP0, t->m_TEX0.PSM))
		{
			m_tex.RemoveAt(cur);

			delete t;
		}
	}

	GSDepthStencil* ds = NULL;

	if(ds == NULL)
	{
		for(POSITION pos = m_ds.GetHeadPosition(); pos; m_ds.GetNext(pos))
		{
			GSDepthStencil* ds2 = m_ds.GetAt(pos);

			if(ds2->m_TEX0.TBP0 == TEX0.TBP0)
			{
				m_ds.MoveToHead(pos);

				ds = ds2;

				ds->m_TEX0 = TEX0;

				ds->Update();

				break;
			}
		}
	}

	if(ds == NULL)
	{
		ds = new GSDepthStencil(this);

		ds->m_TEX0 = TEX0;

		if(!ds->Create(w, h))
		{
			delete ds;

			return NULL;
		}

		m_ds.AddHead(ds);
	}

	if(!m_renderer->m_context->ZBUF.ZMSK)
	{
		ds->m_used = true;
	}

	return ds;
}

GSTextureCache::GSTexture* GSTextureCache::GetTexture()
{
	const GIFRegTEX0& TEX0 = m_renderer->m_context->TEX0;
	const GIFRegCLAMP& CLAMP = m_renderer->m_context->CLAMP;

	DWORD clut[256];

	int pal = GSLocalMemory::m_psm[TEX0.PSM].pal;

	if(pal > 0)
	{
		m_renderer->m_mem.SetupCLUT(TEX0);
		m_renderer->m_mem.CopyCLUT32(clut, pal);
/*
		POSITION pos = m_tex.GetHeadPosition();

		while(pos)
		{
			POSITION cur = pos;

			GSSurface* s = m_tex.GetNext(pos);

			if(s->m_TEX0.TBP0 == TEX0.CBP)
			{
				m_tex.RemoveAt(cur);

				delete s;
			}
		}

		pos = m_rt.GetHeadPosition();

		while(pos)
		{
			POSITION cur = pos;

			GSSurface* s = m_rt.GetNext(pos);

			if(s->m_TEX0.TBP0 == TEX0.CBP)
			{
				m_rt.RemoveAt(cur);

				delete s;
			}
		}

		pos = m_ds.GetHeadPosition();

		while(pos)
		{
			POSITION cur = pos;

			GSSurface* s = m_ds.GetNext(pos);

			if(s->m_TEX0.TBP0 == TEX0.CBP)
			{
				m_ds.RemoveAt(cur);

				delete s;
			}
		}*/
	}

	GSTexture* t = NULL;

	for(POSITION pos = m_tex.GetHeadPosition(); pos; m_tex.GetNext(pos))
	{
		t = m_tex.GetAt(pos);

		if(HasSharedBits(t->m_TEX0.TBP0, t->m_TEX0.PSM, TEX0.TBP0, TEX0.PSM))
		{
			if(TEX0.PSM == t->m_TEX0.PSM && TEX0.TBW == t->m_TEX0.TBW
			&& TEX0.TW == t->m_TEX0.TW && TEX0.TH == t->m_TEX0.TH
			&& (CLAMP.WMS != 3 && t->m_CLAMP.WMS != 3 && CLAMP.WMT != 3 && t->m_CLAMP.WMT != 3 || CLAMP.i64 == t->m_CLAMP.i64)
			&& (pal == 0 || TEX0.CPSM == t->m_TEX0.CPSM && !memcmp(t->m_clut, clut, pal * sizeof(clut[0]))))
			{
				m_tex.MoveToHead(pos);
				break;
			}
		}

		t = NULL;
	}

	if(t == NULL)
	{
		for(POSITION pos = m_rt.GetHeadPosition(); pos; m_rt.GetNext(pos))
		{
			GSRenderTarget* rt = m_rt.GetAt(pos);

			if(rt->m_dirty.IsEmpty() && HasSharedBits(rt->m_TEX0.TBP0, rt->m_TEX0.PSM, TEX0.TBP0, TEX0.PSM))
			{
				t = new GSTexture(this);

				if(!t->Create(rt))
				{
					delete t;

					return NULL;
				}

				m_tex.AddHead(t);

				break;
			}
		}
	}

	if(t == NULL)
	{
		for(POSITION pos = m_ds.GetHeadPosition(); pos; m_ds.GetNext(pos))
		{
			GSDepthStencil* ds = m_ds.GetAt(pos);

			if(ds->m_dirty.IsEmpty() && ds->m_used && HasSharedBits(ds->m_TEX0.TBP0, ds->m_TEX0.PSM, TEX0.TBP0, TEX0.PSM))
			{
				t = new GSTexture(this);

				if(!t->Create(ds))
				{
					delete t;

					return NULL;
				}

				m_tex.AddHead(t);

				break;
			}
		}
	}

	if(t == NULL)
	{
		t = new GSTexture(this);

		if(!t->Create())
		{
			delete t;

			return NULL;
		}

		m_tex.AddHead(t);
	}

	if(pal > 0)
	{
		int size = pal * sizeof(clut[0]);

		if(t->m_palette)
		{
			// TODO: sse2

			DWORD sum = 0;
			
			for(int i = 0; i < pal; i++)
			{
				sum |= t->m_clut[i] ^ clut[i];

				t->m_clut[i] = clut[i];
			}

			if(sum != 0) 
			{
				D3D10_BOX box = {0, 0, 0, pal, 1, 1};

				m_renderer->m_dev->UpdateSubresource(t->m_palette, 0, &box, t->m_clut, size, 0);

				// m_renderer->m_perfmon.Put(GSPerfMon::Texture, size);
			}
		}
		else
		{
			memcpy(t->m_clut, clut, size);
		}
	}

	t->Update(&GSLocalMemory::ReadTextureNP);

	return t;
}

void GSTextureCache::InvalidateTexture(const GIFRegBITBLTBUF& BITBLTBUF, const CRect& r)
{
	POSITION pos = m_tex.GetHeadPosition();

	while(pos)
	{
		POSITION cur = pos;

		GSTexture* t = m_tex.GetNext(pos);

		if(HasSharedBits(BITBLTBUF.DBP, BITBLTBUF.DPSM, t->m_TEX0.TBP0, t->m_TEX0.PSM))
		{
			if(BITBLTBUF.DBW == t->m_TEX0.TBW)
			{
				t->m_dirty.AddTail(GSDirtyRect(BITBLTBUF.DPSM, r));
			}
			else
			{
				m_tex.RemoveAt(cur);

				delete t;
			}
		}
	}

	pos = m_rt.GetHeadPosition();

	while(pos)
	{
		POSITION cur = pos;

		GSRenderTarget* rt = m_rt.GetNext(pos);

		if(HasSharedBits(BITBLTBUF.DBP, BITBLTBUF.DPSM, rt->m_TEX0.TBP0, rt->m_TEX0.PSM))
		{
			if(BITBLTBUF.DPSM == PSM_PSMCT32 
			|| BITBLTBUF.DPSM == PSM_PSMCT24 
			|| BITBLTBUF.DPSM == PSM_PSMCT16 
			|| BITBLTBUF.DPSM == PSM_PSMCT16S
			|| BITBLTBUF.DPSM == PSM_PSMZ32 
			|| BITBLTBUF.DPSM == PSM_PSMZ24 
			|| BITBLTBUF.DPSM == PSM_PSMZ16 
			|| BITBLTBUF.DPSM == PSM_PSMZ16S)
			{
				rt->m_dirty.AddTail(GSDirtyRect(BITBLTBUF.DPSM, r));
				rt->m_TEX0.TBW = BITBLTBUF.DBW;
			}
			else
			{
				m_rt.RemoveAt(cur);

				delete rt;

				continue;
			}
		}

		if(HasSharedBits(BITBLTBUF.DPSM, rt->m_TEX0.PSM) && BITBLTBUF.DBP < rt->m_TEX0.TBP0)
		{
			DWORD rowsize = BITBLTBUF.DBW * 8192;
			DWORD offset = (rt->m_TEX0.TBP0 - BITBLTBUF.DBP) * 256;

			if(rowsize > 0 && offset % rowsize == 0)
			{
				int y = m_renderer->m_mem.m_psm[BITBLTBUF.DPSM].pgs.cy * offset / rowsize;

				if(r.top >= y)
				{
					// TODO: do not add this rect above too
					rt->m_dirty.AddTail(GSDirtyRect(BITBLTBUF.DPSM, CRect(r.left, r.top - y, r.right, r.bottom - y)));
					rt->m_TEX0.TBW = BITBLTBUF.DBW;
					continue;
				}
			}
		}
	}

	// copypaste for ds

	pos = m_ds.GetHeadPosition();

	while(pos)
	{
		POSITION cur = pos;

		GSDepthStencil* ds = m_ds.GetNext(pos);

		if(HasSharedBits(BITBLTBUF.DBP, BITBLTBUF.DPSM, ds->m_TEX0.TBP0, ds->m_TEX0.PSM))
		{
			if(BITBLTBUF.DPSM == PSM_PSMCT32 
			|| BITBLTBUF.DPSM == PSM_PSMCT24 
			|| BITBLTBUF.DPSM == PSM_PSMCT16 
			|| BITBLTBUF.DPSM == PSM_PSMCT16S
			|| BITBLTBUF.DPSM == PSM_PSMZ32 
			|| BITBLTBUF.DPSM == PSM_PSMZ24 
			|| BITBLTBUF.DPSM == PSM_PSMZ16 
			|| BITBLTBUF.DPSM == PSM_PSMZ16S)
			{
				ds->m_dirty.AddTail(GSDirtyRect(BITBLTBUF.DPSM, r));
				ds->m_TEX0.TBW = BITBLTBUF.DBW;
			}
			else
			{
				m_ds.RemoveAt(cur);

				delete ds;

				continue;
			}
		}

		if(HasSharedBits(BITBLTBUF.DPSM, ds->m_TEX0.PSM) && BITBLTBUF.DBP < ds->m_TEX0.TBP0)
		{
			DWORD rowsize = BITBLTBUF.DBW * 8192;
			DWORD offset = (ds->m_TEX0.TBP0 - BITBLTBUF.DBP) * 256;

			if(rowsize > 0 && offset % rowsize == 0)
			{
				int y = m_renderer->m_mem.m_psm[BITBLTBUF.DPSM].pgs.cy * offset / rowsize;

				if(r.top >= y)
				{
					// TODO: do not add this rect above too
					ds->m_dirty.AddTail(GSDirtyRect(BITBLTBUF.DPSM, CRect(r.left, r.top - y, r.right, r.bottom - y)));
					ds->m_TEX0.TBW = BITBLTBUF.DBW;
					continue;
				}
			}
		}
	}
}

void GSTextureCache::InvalidateLocalMem(const GIFRegBITBLTBUF& BITBLTBUF, const CRect& r)
{
	POSITION pos = m_rt.GetHeadPosition();

	while(pos)
	{
		GSRenderTarget* rt = m_rt.GetNext(pos);

		if(HasSharedBits(BITBLTBUF.SBP, BITBLTBUF.SPSM, rt->m_TEX0.TBP0, rt->m_TEX0.PSM))
		{
			rt->Read(r);
			break;
		}
	}
}

void GSTextureCache::IncAge()
{
	RecycleByAge(m_tex, 3);
	RecycleByAge(m_rt);
	RecycleByAge(m_ds);
}

template<class T> void GSTextureCache::RecycleByAge(CAtlList<T*>& l, int maxage)
{
	POSITION pos = l.GetHeadPosition();

	while(pos)
	{
		POSITION cur = pos;

		T* t = l.GetNext(pos);

		if(++t->m_age >= maxage)
		{
			l.RemoveAt(cur);

			delete t;
		}
	}
}

//

GSTextureCache::GSSurface::GSSurface(GSTextureCache* tc)
	: m_tc(tc)
	, m_scale(1, 1)
	, m_age(0)
{
	m_TEX0.TBP0 = ~0;
}

GSTextureCache::GSSurface::~GSSurface()
{
	m_tc->m_renderer->m_dev.Recycle(m_texture);
	m_tc->m_renderer->m_dev.Recycle(m_palette);
}

void GSTextureCache::GSSurface::Update()
{
	m_age = 0;
}
