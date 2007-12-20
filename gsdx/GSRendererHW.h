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
#include "GSVertexHW.h"

template<class Device>
class GSRendererHW : public GSRendererT<Device, GSVertexHW>
{
public:
	GSRendererHW(BYTE* base, bool mt, void (*irq)(), int nloophack, int interlace, int aspectratio, int filter, bool vsync)
		: GSRendererT<Device, GSVertexHW>(base, mt, irq, nloophack, interlace, aspectratio, filter, vsync)
	{
	}

protected:
	void VertexKick(bool skip)
	{
		GSVertexHW& v = m_vl.AddTail();

		v.x = (float)m_v.XYZ.X;
		v.y = (float)m_v.XYZ.Y;
		v.z = (float)m_v.XYZ.Z;

		v.c0 = m_v.RGBAQ.ai32[0];
		v.c1 = m_v.FOG.ai32[1];

		if(PRIM->TME)
		{
			if(PRIM->FST)
			{
				v.w = 1.0f;
				v.u = (float)(int)m_v.UV.U;
				v.v = (float)(int)m_v.UV.V;
			}
			else
			{
				v.w = m_v.RGBAQ.Q;
				v.u = m_v.ST.S;
				v.v = m_v.ST.T;
			}
		}
		else
		{
			v.w = 1.0f;
			v.u = 0;
			v.v = 0;
		}

		__super::VertexKick(skip);
	}
};
