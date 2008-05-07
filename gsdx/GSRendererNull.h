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

template<class Device> class GSRendererNull : public GSRendererT<Device, GSVertexNull>
{
protected:
	void VertexKick(bool skip)
	{
		m_vl.AddTail();

		__super::VertexKick(skip);
	}

	void DrawingKick(bool skip)
	{
		GSVertexNull v;

		switch(PRIM->PRIM)
		{
		case GS_POINTLIST:
			m_vl.RemoveAt(0, v);
			break;
		case GS_LINELIST:
			m_vl.RemoveAt(0, v);
			m_vl.RemoveAt(0, v);
			break;
		case GS_LINESTRIP:
			m_vl.RemoveAt(0, v);
			m_vl.GetAt(0, v);
			break;
		case GS_TRIANGLELIST:
			m_vl.RemoveAt(0, v);
			m_vl.RemoveAt(0, v);
			m_vl.RemoveAt(0, v);
			break;
		case GS_TRIANGLESTRIP:
			m_vl.RemoveAt(0, v);
			m_vl.GetAt(0, v);
			m_vl.GetAt(1, v);
			break;
		case GS_TRIANGLEFAN:
			m_vl.GetAt(0, v);
			m_vl.RemoveAt(1, v);
			m_vl.GetAt(1, v);
			break;
		case GS_SPRITE:
			m_vl.RemoveAt(0, v);
			m_vl.RemoveAt(0, v);
			break;
		default:
			ASSERT(0);
			m_vl.RemoveAll();
			return;
		}

		if(!skip)
		{
			m_perfmon.Put(GSPerfMon::Prim, 1);
		}
	}

	void Draw() 
	{
	}

	bool GetOutput(int i, Texture& t) 
	{
		return false;
	}

public:
	GSRendererNull(BYTE* base, bool mt, void (*irq)(), int nloophack, const GSRendererSettings& rs)
		: GSRendererT<Device, GSVertexNull>(base, mt, irq, nloophack, rs)
	{
	}
};