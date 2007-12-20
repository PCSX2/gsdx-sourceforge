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

#pragma pack(push, 1)

__declspec(align(16)) union GSVertexHW
{
	struct
	{
		float x, y, z, w;
		union {struct {BYTE r, g, b, a;}; DWORD c0;};
		union {struct {BYTE ta0, ta1, res, f;}; DWORD c1;};
		float u, v;
	};
	
	struct {__m128i m128i[2];};
	struct {__m128 m128[2];};

#if _M_IX86_FP >= 2 || defined(_M_AMD64)
	GSVertexHW& operator = (GSVertexHW& v) {m128i[0] = v.m128i[0]; m128i[1] = v.m128i[1]; return *this;}
#endif
};

#pragma pack(pop)