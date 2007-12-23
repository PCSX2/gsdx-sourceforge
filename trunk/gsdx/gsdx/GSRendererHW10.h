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

#include "GSRendererHW.h"
#include "GSTextureFX10.h"

class GSRendererHW10 : public GSRendererHW<GSDevice10>
{
protected:
	GSTextureFX10 m_tfx;

	void DrawingKick(bool skip);
	void Draw();

	struct
	{
		CComPtr<ID3D10DepthStencilState> dss;
		CComPtr<ID3D10BlendState> bs;
	} m_date;

	void SetupDATE(GSTextureCache<GSDevice10>::GSRenderTarget* rt, GSTextureCache<GSDevice10>::GSDepthStencil* ds);
	bool OverrideInput(int& prim, GSTextureCache<GSDevice10>::GSTexture* tex);	

public:
	GSRendererHW10(BYTE* base, bool mt, void (*irq)(), int nloophack, int interlace, int aspectratio, int filter, bool vsync);

	bool Create(LPCTSTR title);
};