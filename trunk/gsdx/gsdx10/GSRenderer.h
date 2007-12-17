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

#include "GSDeviceDX10.h"
#include "GSMergeFX.h"

class GSRenderer : public CWnd, public GSState
{
	DECLARE_MESSAGE_MAP()

protected:
	int m_interlace;
	int m_aspectratio;
	int m_filter;
	bool m_vsync;
	bool m_osd;
	int m_field;

public:
	GSRenderer(BYTE* base, bool mt, void (*irq)(), int nloophack);
	virtual ~GSRenderer();

	virtual bool Create(LPCTSTR title);

	void Show();
	void Hide();

	void OnClose();

	void VSync(int field);

	// TODO

	GSDeviceDX10 m_dev;
	GSMergeFX m_merge;

	struct FlipInfo 
	{
		GSTextureDX10 t; 
		GSVector2 s;
	};

	virtual void Flip() = 0;

	void FinishFlip(FlipInfo src[2]);
	void Merge(FlipInfo src[2], GSTextureDX10& dst);
	void Present();
};

template <class Vertex> class GSRendererT : public GSRenderer
{
protected:
	Vertex* m_vertices;
	int m_count;
	int m_maxcount;
	GSVertexList<Vertex> m_vl;

	void Reset()
	{
		m_count = 0;
		m_vl.RemoveAll();

		__super::Reset();
	}

	void VertexKick(bool skip)
	{
		while(m_vl.GetCount() >= primVertexCount[PRIM->PRIM])
		{
			if(m_count + 6 > m_maxcount)
			{
				m_maxcount = max(10000, m_maxcount * 3/2);

				Vertex* vertices = (Vertex*)_aligned_malloc(sizeof(Vertex) * m_maxcount, 16);

				if(m_vertices)
				{
					memcpy(vertices, m_vertices, sizeof(Vertex) * m_count);

					_aligned_free(m_vertices);
				}

				m_vertices = vertices;
			}

			DrawingKick(skip);
		}
	}

	virtual void DrawingKick(bool skip) = 0;

	void ResetPrim()
	{
		m_vl.RemoveAll();
	}

	void FlushPrim() 
	{
		if(m_count > 0)
		{
			Draw();

			m_count = 0;
		}
	}

	virtual void Draw() = 0;

public:
	GSRendererT(BYTE* base, bool mt, void (*irq)(), int nloophack)
		: GSRenderer(base, mt, irq, nloophack)
		, m_vertices(NULL)
		, m_maxcount(0)
	{
	}

	virtual ~GSRendererT()
	{
		if(m_vertices)
		{
			_aligned_free(m_vertices);
		}
	}
};