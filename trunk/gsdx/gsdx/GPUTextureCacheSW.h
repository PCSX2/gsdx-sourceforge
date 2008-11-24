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

#include "GPURenderer.h"

class GPUTextureCacheSW
{
protected:
	GPUState* m_state;
	BYTE* m_buff[3];
	void* m_texture[3][2][16];
	DWORD m_valid[3];

public:
	GPUTextureCacheSW(GPUState* state);
	virtual ~GPUTextureCacheSW();

	const void* Lookup(const GPURegSTATUS& TPAGE);

	void Invalidate(const CRect& r);
};
