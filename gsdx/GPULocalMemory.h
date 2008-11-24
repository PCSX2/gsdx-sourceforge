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

#include "GPU.h"
#include "GSVector.h"

class GPULocalMemory
{
	static const GSVector4i m_xxxa;
	static const GSVector4i m_xxbx;
	static const GSVector4i m_xgxx;
	static const GSVector4i m_rxxx;

public:
	static const int m_vmsize = 1024 * 1024;

	union {BYTE* m_vm8; WORD* m_vm16; DWORD* m_vm32;};

public:
	GPULocalMemory();
	virtual ~GPULocalMemory();

	void FillRect(const CRect& r, WORD c);
	void WriteRect(const CRect& r, const WORD* c);
	void ReadRect(const CRect& r, WORD* c);
	void MoveRect(const CPoint& src, const CPoint& dst, int w, int h);

	void ReadPage4(int tx, int ty, BYTE* dst);
	void ReadPage8(int tx, int ty, BYTE* dst);
	void ReadPage16(int tx, int ty, WORD* dst);

	void ReadPalette4(int cx, int cy, WORD* dst);
	void ReadPalette8(int cx, int cy, WORD* dst);

	static void Expand16(const WORD* RESTRICT src, DWORD* RESTRICT dst, int pixels);
	static void Expand24(const WORD* RESTRICT src, DWORD* RESTRICT dst, int pixels);
};

#pragma warning(default: 4244)