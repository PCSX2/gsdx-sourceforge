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
#include "GPUTextureCacheSW.h"

GPUTextureCacheSW::GPUTextureCacheSW(GPUState* state)
	: m_state(state)
{
	int size = 256 * 256 * (1 + 1 + 4) * 32;

	m_buff[0] = (BYTE*)VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	m_buff[1] = m_buff[0] + 256 * 256 * 32;
	m_buff[2] = m_buff[1] + 256 * 256 * 32;

	memset(m_buff[0], 0, size);

	memset(m_valid, 0, sizeof(m_valid));

	for(int y = 0, offset = 0; y < 2; y++)
	{
		for(int x = 0; x < 16; x++, offset += 256 * 256)
		{
			m_texture[0][y][x] = &((BYTE*)m_buff[0])[offset];
			m_texture[1][y][x] = &((BYTE*)m_buff[1])[offset];
		}
	}

	for(int y = 0, offset = 0; y < 2; y++)
	{
		for(int x = 0; x < 16; x++, offset += 256 * 256)
		{
			m_texture[2][y][x] = &((DWORD*)m_buff[2])[offset];
		}
	}
}

GPUTextureCacheSW::~GPUTextureCacheSW()
{
	VirtualFree(m_buff[0], 0, MEM_RELEASE);
}

const void* GPUTextureCacheSW::Lookup(const GPURegSTATUS& TPAGE)
{
	if(TPAGE.TP == 3)
	{
		ASSERT(0);

		return NULL;
	}

	void* buff = m_texture[TPAGE.TP][TPAGE.TY][TPAGE.TX];

	UINT32 flag = 1 << TPAGE.TX;

	if(TPAGE.TY) flag <<= 16;

	if((m_valid[TPAGE.TP] & flag) == 0)
	{
		int bpp = 0;

		switch(TPAGE.TP)
		{
		case 0: 
			m_state->m_mem.ReadPage4(TPAGE.TX, TPAGE.TY, (BYTE*)buff); 
			bpp = 4;
			break;
		case 1: 
			m_state->m_mem.ReadPage8(TPAGE.TX, TPAGE.TY, (BYTE*)buff);
			bpp = 8;
			break;
		case 2: 
		case 3: 
			m_state->m_mem.ReadPage16(TPAGE.TX, TPAGE.TY, (WORD*)buff);
			bpp = 16;
		default:
			// FIXME: __assume(0); // vc9 generates bogus code in release mode
			break;
		}

		m_state->m_perfmon.Put(GSPerfMon::Unswizzle, 256 * 256 * bpp >> 3);

		m_valid[TPAGE.TP] |= flag;
	}

	return buff;
}

void GPUTextureCacheSW::Invalidate(const CRect& r)
{
	for(int y = 0, ye = min(r.bottom, 512), j = 0; y < ye; y += 256, j += 16)
	{
		if(r.top >= y + 256) continue;

		for(int x = 0, xe = min(r.right, 1024), i = 0; x < xe; x += 64, i++)
		{
			DWORD flag = (1 << i) << j;

			if(r.left >= x + 256) continue;

			m_valid[2] &= ~flag;

			if(r.left >= x + 128) continue;

			m_valid[1] &= ~flag;

			if(r.left >= x + 64) continue;

			m_valid[0] &= ~flag;
		}
	}
}