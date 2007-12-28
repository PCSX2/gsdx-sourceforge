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

template<class Device> class GSTextureCache
{
	typedef typename Device::Texture Texture;

public:
	class GSSurface
	{
	protected:
		GSRenderer<Device>* m_renderer;

		typedef typename Device::Texture Texture;

	public:
		Texture m_texture;
		Texture m_palette;
		int m_age;
		GSDirtyRectList m_dirty;
		GIFRegTEX0 m_TEX0;

		explicit GSSurface(GSRenderer<Device>* renderer)
			: m_renderer(renderer)
			, m_age(0)
		{
			m_TEX0.TBP0 = (UINT64)~0;
		}

		virtual ~GSSurface()
		{
			m_renderer->m_dev.Recycle(m_texture);
			m_renderer->m_dev.Recycle(m_palette);
		}

		virtual void Update()
		{
			m_age = 0;
		}
	};

	class GSRenderTarget : public GSSurface
	{
	public:
		bool m_used;

		explicit GSRenderTarget(GSRenderer<Device>* renderer)
			: GSSurface(renderer)
			, m_used(true)
		{
		}

		virtual bool Create(int w, int h)
		{
			// FIXME: initial data should be unswizzled from local mem in Update() if dirty

			return m_renderer->m_dev.CreateRenderTarget(m_texture, w, h);
		}

		virtual void Read(CRect r) = 0;
	};

	class GSDepthStencil : public GSSurface
	{
	public:
		bool m_used;

		explicit GSDepthStencil(GSRenderer<Device>* renderer)
			: GSSurface(renderer)
			, m_used(false)
		{
		}

		virtual bool Create(int w, int h)
		{
			// FIXME: initial data should be unswizzled from local mem in Update() if dirty

			return m_renderer->m_dev.CreateDepthStencil(m_texture, w, h);
		}

	};

	class GSTexture : public GSSurface
	{
	protected:
		bool GetDirtyRect(CRect& r)
		{
			int w = 1 << m_TEX0.TW;
			int h = 1 << m_TEX0.TH;

			r.SetRect(0, 0, w, h);

			m_renderer->MinMaxUV(w, h, r);

			CRect dirty = m_dirty.GetDirtyRect(m_TEX0);
			CRect valid = m_valid;

			dirty &= CRect(0, 0, m_texture.GetWidth(), m_texture.GetHeight());

			if(IsRectInRect(r, valid))
			{
				if(dirty.IsRectEmpty()) return false;
				else if(IsRectInRect(dirty, r)) r = dirty;
				else if(IsRectInRect(dirty, valid)) r |= dirty;
				else r = valid & dirty;
			}
			else if(IsRectInRectH(r, valid) && (r.left >= valid.left || r.right <= valid.right))
			{
				r.top = valid.top;
				r.bottom = valid.bottom;
				if(r.left < valid.left) r.right = valid.left;
				else /*if(r.right > valid.right)*/ r.left = valid.right;
			}
			else if(IsRectInRectV(r, valid) && (r.top >= valid.top || r.bottom <= valid.bottom))
			{
				r.left = valid.left;
				r.right = valid.right;
				if(r.top < valid.top) r.bottom = valid.top;
				else /*if(r.bottom > valid.bottom)*/ r.top = valid.bottom;
			}
			else
			{
				r |= valid;
			}

			return !r.IsRectEmpty();
		}

	public:
		GIFRegCLAMP m_CLAMP;
		DWORD m_clut[256]; // *
		CRect m_valid;
		int m_bpp;
		int m_bpp2;
		bool m_rendered;

		explicit GSTexture(GSRenderer<Device>* renderer)
			: GSSurface(renderer)
			, m_valid(0, 0, 0, 0)
			, m_bpp(0)
			, m_bpp2(0)
			, m_rendered(false)
		{
			memset(m_clut, 0, sizeof(m_clut));
		}

		virtual bool Create() = 0;
		virtual bool Create(GSRenderTarget* rt) = 0;
		virtual bool Create(GSDepthStencil* ds) = 0;
	};

protected:
	GSRenderer<Device>* m_renderer;
	CAtlList<GSRenderTarget*> m_rt;
	CAtlList<GSDepthStencil*> m_ds;
	CAtlList<GSTexture*> m_tex;
	bool m_nativeres;

	template<class T> void RecycleByAge(CAtlList<T*>& l, int maxage = 10)
	{
		POSITION pos = l.GetHeadPosition();

		while(pos)
		{
			POSITION cur = pos;

			T* t = l.GetNext(pos);

			if(++t->m_age > maxage)
			{
				l.RemoveAt(cur);

				delete t;
			}
		}
	}

	virtual GSRenderTarget* CreateRenderTarget() = 0;
	virtual GSDepthStencil* CreateDepthStencil() = 0;
	virtual GSTexture* CreateTexture() = 0;

public:
	GSTextureCache(GSRenderer<Device>* renderer, bool nativeres)
		: m_renderer(renderer)
		, m_nativeres(nativeres)
	{
	}

	virtual ~GSTextureCache()
	{
		RemoveAll();
	}

	void RemoveAll()
	{
		while(m_rt.GetCount()) delete m_rt.RemoveHead();
		while(m_ds.GetCount()) delete m_ds.RemoveHead();
		while(m_tex.GetCount()) delete m_tex.RemoveHead();
	}

	GSRenderTarget* GetRenderTarget(const GIFRegTEX0& TEX0, int w, int h, bool fb = false)
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
			rt = CreateRenderTarget();

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
			int ww = (int)(m_renderer->GetFramePos().cx + rt->m_TEX0.TBW * 64);
			int hh = (int)(m_renderer->GetFramePos().cy + m_renderer->GetDisplaySize().cy);

			if(hh <= m_renderer->GetDeviceSize().cy / 2)
			{
				hh *= 2;
			}

			if(ww > 0 && hh > 0)
			{
				rt->m_texture.m_scale.x = (float)w / ww;
				rt->m_texture.m_scale.y = (float)h / hh;
			}
		}

		if(!fb)
		{
			rt->m_used = true;
		}

		return rt;
	}

	GSDepthStencil* GetDepthStencil(const GIFRegTEX0& TEX0, int w, int h)
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
			ds = CreateDepthStencil();

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

	GSTexture* GetTexture()
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
			}
			*/
		}

		GSTexture* t = NULL;

		for(POSITION pos = m_tex.GetHeadPosition(); pos; m_tex.GetNext(pos))
		{
			t = m_tex.GetAt(pos);

			if(HasSharedBits(t->m_TEX0.TBP0, t->m_TEX0.PSM, TEX0.TBP0, TEX0.PSM))
			{
				if(TEX0.PSM == t->m_TEX0.PSM && TEX0.TBW == t->m_TEX0.TBW
				&& TEX0.TW == t->m_TEX0.TW && TEX0.TH == t->m_TEX0.TH
				&& (m_renderer->m_psrr || (CLAMP.WMS != 3 && t->m_CLAMP.WMS != 3 && CLAMP.WMT != 3 && t->m_CLAMP.WMT != 3 || CLAMP.i64 == t->m_CLAMP.i64))
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
					t = CreateTexture();

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
					t = CreateTexture();

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
			t = CreateTexture();

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
					t->m_palette.Update(CRect(0, 0, pal, 1), t->m_clut, size);

					m_renderer->m_perfmon.Put(GSPerfMon::Texture, size);
				}
			}
			else
			{
				memcpy(t->m_clut, clut, size);
			}
		}

		t->Update();

		return t;
	}

	void InvalidateVideoMem(const GIFRegBITBLTBUF& BITBLTBUF, const CRect& r)
	{
		bool found = false;

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

					found = true;
				}
				else
				{
					m_tex.RemoveAt(cur);

					delete t;
				}
			}
		}

		if(found) return;

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
				DWORD offset = (DWORD)((rt->m_TEX0.TBP0 - BITBLTBUF.DBP) * 256);

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
				DWORD offset = (DWORD)((ds->m_TEX0.TBP0 - BITBLTBUF.DBP) * 256);

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

	void InvalidateLocalMem(const GIFRegBITBLTBUF& BITBLTBUF, const CRect& r)
	{
		POSITION pos = m_rt.GetHeadPosition();

		while(pos)
		{
			POSITION cur = pos;

			GSRenderTarget* rt = m_rt.GetNext(pos);

			if(HasSharedBits(BITBLTBUF.SBP, BITBLTBUF.SPSM, rt->m_TEX0.TBP0, rt->m_TEX0.PSM))
			{
				if(HasCompatibleBits(BITBLTBUF.SPSM, rt->m_TEX0.PSM))
				{
					rt->Read(r);
					return;
				}
				else
				{
					m_rt.RemoveAt(cur);
					delete rt;
					continue;
				}
			}
		}
/*
		// no good, ffx does a lot of readback after exiting menu, at 0x02f00 this wrongly finds rt 0x02100 (0,448 - 512,480)

		GSRenderTarget* rt2 = NULL;
		int ymin = INT_MAX;

		pos = m_rt.GetHeadPosition();

		while(pos)
		{
			GSRenderTarget* rt = m_rt.GetNext(pos);

			if(HasSharedBits(BITBLTBUF.SPSM, rt->m_TEX0.PSM) && BITBLTBUF.SBP > rt->m_TEX0.TBP0)
			{
				// ffx2 pause screen background

				DWORD rowsize = BITBLTBUF.SBW * 8192;
				DWORD offset = (DWORD)((BITBLTBUF.SBP - rt->m_TEX0.TBP0) * 256);

				if(rowsize > 0 && offset % rowsize == 0)
				{
					int y = m_renderer->m_mem.m_psm[BITBLTBUF.SPSM].pgs.cy * offset / rowsize;

					if(y < ymin && y < 512)
					{
						rt2 = rt;
						ymin = y;
					}
				}
			}
		}

		if(rt2)
		{
			rt2->Read(CRect(r.left, r.top + ymin, r.right, r.bottom + ymin));
		}

		// TODO: ds
*/
	}

	void IncAge()
	{
		RecycleByAge(m_tex, 2);
		RecycleByAge(m_rt);
		RecycleByAge(m_ds);
	}
};
