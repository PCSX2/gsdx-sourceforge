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
protected:
	GSTextureFX m_tfx;

	void DrawingKick(bool skip);
	void Draw();

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

	void SetupDATE(GSTextureCache<GSDeviceDX9>::GSRenderTarget* rt, GSTextureCache<GSDeviceDX9>::GSDepthStencil* ds);
	void UpdateFBA(GSTextureCache<GSDeviceDX9>::GSRenderTarget* rt);
	bool OverrideInput(int& prim, GSTextureCache<GSDeviceDX9>::GSTexture* tex);	

public:
	GSRendererHWDX9(BYTE* base, bool mt, void (*irq)(), int nloophack, int interlace, int aspectratio, int filter, bool vsync);

	bool Create(LPCTSTR title);
};
