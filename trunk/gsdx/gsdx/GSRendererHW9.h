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
#include "GSTextureFX9.h"

class GSRendererHW9 : public GSRendererHW<GSDevice9, GSVertexHW9>
{
protected:
	GSTextureFX9 m_tfx;
	bool m_logz;

	void VertexKick(bool skip);
	void Draw();
	bool WrapZ(float maxz);

	__forceinline int ScissorTest(const GSVector4& p0, const GSVector4& p1);

	void DrawingKickPoint(GSVertexHW9* v, int& count);
	void DrawingKickLine(GSVertexHW9* v, int& count);
	void DrawingKickTriangle(GSVertexHW9* v, int& count);
	void DrawingKickSprite(GSVertexHW9* v, int& count);

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

	void SetupDATE(Texture& rt, Texture& ds);
	void UpdateFBA(Texture& rt);

public:
	GSRendererHW9(BYTE* base, bool mt, void (*irq)(), int nloophack, const GSRendererSettings& rs);

	bool Create(LPCTSTR title);
};
