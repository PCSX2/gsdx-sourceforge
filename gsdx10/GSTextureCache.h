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

#include "GSTexture2D.h"

class GSRendererHW;

class GSTextureCache
{
public:
	class GSSurface
	{
	protected:
		GSTextureCache* m_tc;

	public:
		GSTexture2D m_texture;
		GSTexture2D m_palette;
		GSVector2 m_scale;
		int m_age;
		GSDirtyRectList m_dirty;
		GIFRegTEX0 m_TEX0;

		explicit GSSurface(GSTextureCache* tc);
		virtual ~GSSurface();

		void Update();
	};

	class GSRenderTarget : public GSSurface
	{
	public:
		bool m_used;

		explicit GSRenderTarget(GSTextureCache* tc);

		bool Create(int w, int h);
		void Update();
		void Read(CRect r);
	};

	class GSDepthStencil : public GSSurface
	{
	public:
		bool m_used;

		explicit GSDepthStencil(GSTextureCache* tc);

		bool Create(int w, int h);
		void Update();
	};

	class GSTexture : public GSSurface
	{
		bool GetDirtyRect(CRect& r);

	public:
		DWORD m_clut[256]; // *
		CRect m_valid;
		int m_bpp;
		int m_bpp2;
		bool m_rendered;

		explicit GSTexture(GSTextureCache* tc);

		bool Create();
		bool Create(GSRenderTarget* rt);
		bool Create(GSDepthStencil* ds);
		void Update();
	};

protected:
	GSRendererHW* m_renderer;
	CAtlList<GSRenderTarget*> m_rt;
	CAtlList<GSDepthStencil*> m_ds;
	CAtlList<GSTexture*> m_tex;
	bool m_nativeres;

	template<class T> void RecycleByAge(CAtlList<T*>& l, int maxage = 10);

public:
	GSTextureCache(GSRendererHW* renderer);
	virtual ~GSTextureCache();

	void RemoveAll();

	GSRenderTarget* GetRenderTarget(const GIFRegTEX0& TEX0, int w, int h, bool fb = false);
	GSDepthStencil* GetDepthStencil(const GIFRegTEX0& TEX0, int w, int h);
	GSTexture* GetTexture();

	void InvalidateVideoMem(const GIFRegBITBLTBUF& BITBLTBUF, const CRect& r);
	void InvalidateLocalMem(const GIFRegBITBLTBUF& BITBLTBUF, const CRect& r);

	void IncAge();
};
