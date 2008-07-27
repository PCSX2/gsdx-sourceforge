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
#include "GSVertexHW.h"
#include "GSTextureFX10.h"

class GSRendererHW10 : public GSRendererHW<GSDevice10, GSVertexHW10>
{
protected:
	GSTextureFX10 m_tfx;

	void VertexKick(bool skip);
	void DrawingKick(GSVertexHW10* v, int& count); // TODO
	void Draw();
	bool WrapZ(DWORD maxz);

	__forceinline int ScissorTest(const GSVector4i& p0, const GSVector4i& p1);

	void DrawingKickPoint(GSVertexHW10* v, int& count);

	#if _M_SSE >= 0x401

	void DrawingKickLine(GSVertexHW10* v, int& count);
	void DrawingKickTriangle(GSVertexHW10* v, int& count);
	void DrawingKickSprite(GSVertexHW10* v, int& count);

	#endif

	struct
	{
		CComPtr<ID3D10DepthStencilState> dss;
		CComPtr<ID3D10BlendState> bs;
	} m_date;

	void SetupDATE(Texture& rt, Texture& ds);

public:
	GSRendererHW10(BYTE* base, bool mt, void (*irq)(), int nloophack, const GSRendererSettings& rs);

	bool Create(LPCTSTR title);
};