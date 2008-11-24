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
 
#include "StdAfx.h"
#include "GPULocalMemory.h"

const GSVector4i GPULocalMemory::m_xxxa(0x00008000);
const GSVector4i GPULocalMemory::m_xxbx(0x00007c00);
const GSVector4i GPULocalMemory::m_xgxx(0x000003e0);
const GSVector4i GPULocalMemory::m_rxxx(0x0000001f);

static void CheckRect(const CRect& r)
{
	ASSERT(r.left >= 0 && r.left <= 1024);
	ASSERT(r.right >= 0 && r.right <= 1024);
	ASSERT(r.top >= 0 && r.top <= 512);
	ASSERT(r.bottom >= 0 && r.bottom <= 512);
	ASSERT(r.left <= r.right);
	ASSERT(r.top <= r.bottom);
}

GPULocalMemory::GPULocalMemory()
{
	m_vm8 = (BYTE*)VirtualAlloc(NULL, m_vmsize * 2, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	memset(m_vm8, 0, m_vmsize);
}

GPULocalMemory::~GPULocalMemory()
{
	VirtualFree(m_vm8, 0, MEM_RELEASE);
}

void GPULocalMemory::FillRect(const CRect& r, WORD c)
{
	CheckRect(r);

	CRect clip;
	
	clip.left = (r.left + 7) & ~7;
	clip.top = r.top;
	clip.right = r.right & ~7;
	clip.bottom = r.bottom;

	for(int y = r.top; y < clip.top; y++)
	{
		WORD* p = &m_vm16[y << 10];

		for(int x = r.left; x < r.right; x++)
		{
			p[x] = c;
		}
	}

	for(int y = clip.bottom; y < r.bottom; y++)
	{
		WORD* p = &m_vm16[y << 10];

		for(int x = r.left; x < r.right; x++)
		{
			p[x] = c;
		}
	}

	if(r.left < clip.left || clip.right < r.right)
	{
		for(int y = clip.top; y < clip.bottom; y++)
		{
			WORD* p = &m_vm16[y << 10];

			for(int x = r.left; x < clip.left; x++)
			{
				p[x] = c;
			}

			for(int x = clip.right; x < r.right; x++)
			{
				p[x] = c;
			}
		}
	}

	GSVector4i c128((c << 16) | c);

	for(int y = clip.top; y < clip.bottom; y++)
	{
		GSVector4i* p = (GSVector4i*)&m_vm16[(y << 10) + clip.left];

		for(int i = 0, n = (clip.right - clip.left) >> 3; i < n; i++)
		{
			p[i] = c128;
		}
	}
}

void GPULocalMemory::WriteRect(const CRect& r, const WORD* c)
{
	CheckRect(r);

	int w = r.Width();

	for(int y = r.top; y < r.bottom; y++)
	{
		WORD* p = &m_vm16[y << 10];

		memcpy(&p[r.left], c, w * 2);

		c += w;
	}
}

void GPULocalMemory::ReadRect(const CRect& r, WORD* c)
{
	CheckRect(r);

	int w = r.Width();

	for(int y = r.top; y < r.bottom; y++)
	{
		WORD* p = &m_vm16[y << 10];

		memcpy(c, &p[r.left], w * 2);

		c += w;
	}
}

void GPULocalMemory::MoveRect(const CPoint& src, const CPoint& dst, int w, int h)
{
	CheckRect(CRect(src, CSize(w, h)));
	CheckRect(CRect(dst, CSize(w, h)));

	WORD* s = &m_vm16[(src.y << 10) + src.x];
	WORD* d = &m_vm16[(dst.y << 10) + dst.x];

	for(int i = 0; i < h; i++, s += 1024, d += 1024)
	{
		memcpy(d, s, w * 2);
	}
}

void GPULocalMemory::ReadPage4(int tx, int ty, BYTE* dst)
{
	GSVector4i mask(0x0f0f0f0f);

	WORD* src = &m_vm16[(ty << 18) + (tx << 6)];

	for(int j = 0; j < 256; j++, src += 1024, dst += 256)
	{
		GSVector4i* s = (GSVector4i*)src;
		GSVector4i* d = (GSVector4i*)dst;

		for(int i = 0; i < 8; i++)
		{
			GSVector4i c = s[i];

			GSVector4i l = c & mask;
			GSVector4i h = c.andnot(mask) >> 4;

			d[i * 2 + 0] = l.upl8(h);
			d[i * 2 + 1] = l.uph8(h);
		}
	}
}

void GPULocalMemory::ReadPage8(int tx, int ty, BYTE* dst)
{
	WORD* src = &m_vm16[(ty << 18) + (tx << 6)];

	for(int j = 0; j < 256; j++, src += 1024, dst += 256)
	{
		memcpy(dst, src, 256);
	}
}

void GPULocalMemory::ReadPage16(int tx, int ty, WORD* dst)
{
	WORD* src = &m_vm16[(ty << 18) + (tx << 6)];

	for(int j = 0; j < 256; j++, src += 1024, dst += 256)
	{
		memcpy(dst, src, 256 * sizeof(WORD));
	}
}

void GPULocalMemory::ReadPalette4(int cx, int cy, WORD* dst)
{
	memcpy(dst, &m_vm16[(cy << 10) + (cx << 4)], 16 * sizeof(WORD));
}

void GPULocalMemory::ReadPalette8(int cx, int cy, WORD* dst)
{
	memcpy(dst, &m_vm16[(cy << 10) + (cx << 4)], 256 * sizeof(WORD));
}

void GPULocalMemory::Expand16(const WORD* RESTRICT src, DWORD* RESTRICT dst, int pixels)
{
	GSVector4i rm = m_rxxx;
	GSVector4i gm = m_xgxx;
	GSVector4i bm = m_xxbx;
	GSVector4i am = m_xxxa;

	GSVector4i* s = (GSVector4i*)src;
	GSVector4i* d = (GSVector4i*)dst;

	for(int i = 0, j = pixels >> 3; i < j; i++)
	{
		GSVector4i c = s[i];

		GSVector4i l = c.upl16();
		GSVector4i h = c.uph16();

		d[i * 2 + 0] = ((l & rm) << 3) | ((l & gm) << 6) | ((l & bm) << 9) | ((l & am) << 16);
		d[i * 2 + 1] = ((h & rm) << 3) | ((h & gm) << 6) | ((h & bm) << 9) | ((h & am) << 16);
	}
}

void GPULocalMemory::Expand24(const WORD* RESTRICT src, DWORD* RESTRICT dst, int pixels)
{
	// TODO: sse

	BYTE* s = (BYTE*)src;

	for(int i = 0; i < pixels; i++, s += 3)
	{
		dst[i] = (s[2] << 16) | (s[1] << 8) | s[0];
	}
}
