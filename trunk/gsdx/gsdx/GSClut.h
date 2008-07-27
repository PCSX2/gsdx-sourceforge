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

#include "GS.h"
#include "GSVector.h"
#include "GSTables.h"

class GSLocalMemory;

class GSClut
{
	DWORD m_CBP[2];
	WORD* m_clut;
	DWORD* m_buff32;
	UINT64* m_buff64;

	struct
	{
		GIFRegTEX0 TEX0;
		GIFRegTEXCLUT TEXCLUT;
		bool dirty;
	} m_write;

	struct
	{
		GIFRegTEX0 TEX0;
		GIFRegTEXA TEXA;
		bool dirty;
	} m_read;

	typedef void (GSClut::*writeCLUT)(const GIFRegTEX0& TEX0, const GIFRegTEXCLUT& TEXCLUT, const GSLocalMemory* mem);

	writeCLUT m_wc[2][16][64];

	void WriteCLUT32_I8_CSM1(const GIFRegTEX0& TEX0, const GIFRegTEXCLUT& TEXCLUT, const GSLocalMemory* mem);
	void WriteCLUT32_I4_CSM1(const GIFRegTEX0& TEX0, const GIFRegTEXCLUT& TEXCLUT, const GSLocalMemory* mem);
	void WriteCLUT16_I8_CSM1(const GIFRegTEX0& TEX0, const GIFRegTEXCLUT& TEXCLUT, const GSLocalMemory* mem);
	void WriteCLUT16_I4_CSM1(const GIFRegTEX0& TEX0, const GIFRegTEXCLUT& TEXCLUT, const GSLocalMemory* mem);
	void WriteCLUT16S_I8_CSM1(const GIFRegTEX0& TEX0, const GIFRegTEXCLUT& TEXCLUT, const GSLocalMemory* mem);
	void WriteCLUT16S_I4_CSM1(const GIFRegTEX0& TEX0, const GIFRegTEXCLUT& TEXCLUT, const GSLocalMemory* mem);

	void WriteCLUT32_I8_CSM2(const GIFRegTEX0& TEX0, const GIFRegTEXCLUT& TEXCLUT, const GSLocalMemory* mem);
	void WriteCLUT32_I4_CSM2(const GIFRegTEX0& TEX0, const GIFRegTEXCLUT& TEXCLUT, const GSLocalMemory* mem);
	void WriteCLUT16_I8_CSM2(const GIFRegTEX0& TEX0, const GIFRegTEXCLUT& TEXCLUT, const GSLocalMemory* mem);
	void WriteCLUT16_I4_CSM2(const GIFRegTEX0& TEX0, const GIFRegTEXCLUT& TEXCLUT, const GSLocalMemory* mem);
	void WriteCLUT16S_I8_CSM2(const GIFRegTEX0& TEX0, const GIFRegTEXCLUT& TEXCLUT, const GSLocalMemory* mem);
	void WriteCLUT16S_I4_CSM2(const GIFRegTEX0& TEX0, const GIFRegTEXCLUT& TEXCLUT, const GSLocalMemory* mem);

	void WriteCLUT_NULL(const GIFRegTEX0& TEX0, const GIFRegTEXCLUT& TEXCLUT, const GSLocalMemory* mem) {}

	static void WriteCLUT_T32_I8_CSM1(const DWORD* RESTRICT src, WORD* RESTRICT clut);
	static void WriteCLUT_T32_I4_CSM1(const DWORD* RESTRICT src, WORD* RESTRICT clut);
	static void WriteCLUT_T16_I8_CSM1(const WORD* RESTRICT src, WORD* RESTRICT clut);
	static void WriteCLUT_T16_I4_CSM1(const WORD* RESTRICT src, WORD* RESTRICT clut);
	static void ReadCLUT_T32_I8(const WORD* RESTRICT clut, DWORD* RESTRICT dst);
	static void ReadCLUT_T32_I4(const WORD* RESTRICT clut, DWORD* RESTRICT dst);
	static void ReadCLUT_T32_I4(const WORD* RESTRICT clut, DWORD* RESTRICT dst32, UINT64* RESTRICT dst64);
	static void ReadCLUT_T16_I8(const WORD* RESTRICT clut, DWORD* RESTRICT dst);
	static void ReadCLUT_T16_I4(const WORD* RESTRICT clut, DWORD* RESTRICT dst);
	static void ReadCLUT_T16_I4(const WORD* RESTRICT clut, DWORD* RESTRICT dst32, UINT64* RESTRICT dst64);
	static void ExpandCLUT64_T32_I8(const DWORD* RESTRICT src, UINT64* RESTRICT dst);
	static void ExpandCLUT64_T32(const GSVector4i& hi, const GSVector4i& lo0, const GSVector4i& lo1, const GSVector4i& lo2, const GSVector4i& lo3, GSVector4i* dst);
	static void ExpandCLUT64_T32(const GSVector4i& hi, const GSVector4i& lo, GSVector4i* dst);
	static void ExpandCLUT64_T16_I8(const DWORD* RESTRICT src, UINT64* RESTRICT dst);
	static void ExpandCLUT64_T16(const GSVector4i& hi, const GSVector4i& lo0, const GSVector4i& lo1, const GSVector4i& lo2, const GSVector4i& lo3, GSVector4i* dst);
	static void ExpandCLUT64_T16(const GSVector4i& hi, const GSVector4i& lo, GSVector4i* dst);

	static void Expand16(const WORD* RESTRICT src, DWORD* RESTRICT dst, int w, const GIFRegTEXA& TEXA);

public:
	GSClut();
	virtual ~GSClut();

	void Invalidate();
	bool IsDirty(const GIFRegTEX0& TEX0, const GIFRegTEXCLUT& TEXCLUT);
	bool IsWriting(const GIFRegTEX0& TEX0, const GIFRegTEXCLUT& TEXCLUT);
	bool Write(const GIFRegTEX0& TEX0, const GIFRegTEXCLUT& TEXCLUT, const GSLocalMemory* mem);
	void Read(const GIFRegTEX0& TEX0);
	void Read32(const GIFRegTEX0& TEX0, const GIFRegTEXA& TEXA);

	DWORD operator [] (size_t i) const {return m_buff32[i];}

	operator const DWORD*() const  {return m_buff32;}
	operator const UINT64*() const {return m_buff64;}
};
