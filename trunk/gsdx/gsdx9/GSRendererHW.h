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

#include "GSTextureCache.h"
#include "GSTextureFX.h"

class GSRendererHWDX9 : public GSRendererHW<GSDeviceDX9>
{
	friend class GSTextureCache;

protected:
	int m_width;
	int m_height;
	int m_skip;

	GSTextureCache m_tc;
	GSTextureFX m_tfx;

	void VSync(int field);
	bool GetOutput(int i, GSTextureDX9& t, GSVector2& s);
	void DrawingKick(bool skip);
	void Draw();
	void InvalidateVideoMem(const GIFRegBITBLTBUF& BITBLTBUF, CRect r);
	void InvalidateLocalMem(const GIFRegBITBLTBUF& BITBLTBUF, CRect r);
	void MinMaxUV(int w, int h, CRect& r);

	struct
	{
		Direct3DDepthStencilState9 dss;
		Direct3DBlendState9 bs;
	} m_date;

	struct
	{
		bool enabled;
		Direct3DDepthStencilState9 dss;
		Direct3DBlendState9 bs;
	} m_fba;

	void SetupDATE(GSTextureCache::GSRenderTarget* rt, GSTextureCache::GSDepthStencil* ds);
	void UpdateFBA(GSTextureCache::GSRenderTarget* rt);
	bool OverrideInput(int& prim, GSTextureCache::GSTexture* tex);	

public:
	GSRendererHWDX9(BYTE* base, bool mt, void (*irq)(), int nloophack, int interlace, int aspectratio, int filter, bool vsync);

	bool Create(LPCTSTR title);
	void ResetDevice() {m_tc.RemoveAll();}
};
