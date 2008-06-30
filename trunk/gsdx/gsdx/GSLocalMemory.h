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

#pragma warning(disable: 4100) // warning C4100: 'TEXA' : unreferenced formal parameter

#include "GS.h"
#include "GSTables.h"
#include "GSVector.h"
#include "GSBlock.h"

class GSLocalMemory : public GSBlock
{
public:
	typedef DWORD (*pixelAddress)(int x, int y, DWORD bp, DWORD bw);
	typedef void (GSLocalMemory::*writePixel)(int x, int y, DWORD c, DWORD bp, DWORD bw);
	typedef void (GSLocalMemory::*writeFrame)(int x, int y, DWORD c, DWORD bp, DWORD bw);
	typedef DWORD (GSLocalMemory::*readPixel)(int x, int y, DWORD bp, DWORD bw);
	typedef DWORD (GSLocalMemory::*readTexel)(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	typedef void (GSLocalMemory::*writePixelAddr)(DWORD addr, DWORD c);
	typedef void (GSLocalMemory::*writeFrameAddr)(DWORD addr, DWORD c);
	typedef DWORD (GSLocalMemory::*readPixelAddr)(DWORD addr);
	typedef DWORD (GSLocalMemory::*readTexelAddr)(DWORD addr, GIFRegTEXA& TEXA);
	typedef void (GSLocalMemory::*writeImage)(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);
	typedef void (GSLocalMemory::*readImage)(int& tx, int& ty, BYTE* dst, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);
	typedef void (GSLocalMemory::*readTexture)(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);

	typedef union 
	{
		struct
		{
			pixelAddress pa, ba, pga;
			readPixel rp;
			readPixelAddr rpa;
			writePixel wp;
			writePixelAddr wpa;
			readTexel rt, rtNP;
			readTexelAddr rta;
			writeFrameAddr wfa;
			writeImage wi;
			readImage ri;
			readTexture rtx, rtxNP;
			DWORD bpp, pal, trbpp; 
			CSize bs, pgs;
			int* rowOffset[8];
		};
		BYTE dummy[128];
	} psm_t;

	static psm_t m_psm[64];

	static const int m_vmsize = 1024 * 1024 * 4;

	union {BYTE* m_vm8; WORD* m_vm16; DWORD* m_vm32;};

protected:
	static DWORD pageOffset32[32][32][64];
	static DWORD pageOffset32Z[32][32][64];
	static DWORD pageOffset16[32][64][64];
	static DWORD pageOffset16S[32][64][64];
	static DWORD pageOffset16Z[32][64][64];
	static DWORD pageOffset16SZ[32][64][64];
	static DWORD pageOffset8[32][64][128];
	static DWORD pageOffset4[32][128][128];

	static int rowOffset32[2048];
	static int rowOffset32Z[2048];
	static int rowOffset16[2048];
	static int rowOffset16S[2048];
	static int rowOffset16Z[2048];
	static int rowOffset16SZ[2048];
	static int rowOffset8[2][2048];
	static int rowOffset4[2][2048];

	DWORD m_CBP[2];
	WORD* m_clut;
	DWORD* m_clut32;
	UINT64* m_clut64;

	GIFRegTEX0 m_prevTEX0;
	GIFRegTEXCLUT m_prevTEXCLUT;
	bool m_fCLUTMayBeDirty;

	static void Expand16(WORD* src, DWORD* dst, int w, GIFRegTEXA* TEXA);

	__forceinline static DWORD Expand24To32(DWORD c, GIFRegTEXA& TEXA)
	{
		return (((!TEXA.AEM | (c & 0xffffff)) ? TEXA.TA0 : 0) << 24) | (c & 0xffffff);
	}

	__forceinline static DWORD Expand16To32(WORD c, GIFRegTEXA& TEXA)
	{
		return (((c & 0x8000) ? TEXA.TA1 : (!TEXA.AEM | c) ? TEXA.TA0 : 0) << 24) | ((c & 0x7c00) << 9) | ((c & 0x03e0) << 6) | ((c & 0x001f) << 3);
	}

public:
	GSLocalMemory();
	virtual ~GSLocalMemory();

	// address

	static DWORD PageAddress32(int x, int y, DWORD bp, DWORD bw)
	{
		return ((bp >> 5) + (y >> 5) * bw + (x >> 6)) << 11; 
	}

	static DWORD PageAddress16(int x, int y, DWORD bp, DWORD bw)
	{
		return ((bp >> 5) + (y >> 6) * bw + (x >> 6)) << 12;
	}

	static DWORD PageAddress8(int x, int y, DWORD bp, DWORD bw)
	{
		return ((bp >> 5) + (y >> 6) * ((bw + 1) >> 1) + (x >> 7)) << 13; 
	}

	static DWORD PageAddress4(int x, int y, DWORD bp, DWORD bw)
	{
		return ((bp >> 5) + (y >> 7) * ((bw + 1) >> 1) + (x >> 7)) << 14;
	}

	static DWORD BlockAddress32(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + (y & ~0x1f) * bw + ((x >> 1) & ~0x1f);
		DWORD block = blockTable32[(y >> 3) & 3][(x >> 3) & 7];
		return (page + block) << 6;
	}

	static DWORD BlockAddress16(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + ((y >> 1) & ~0x1f) * bw + ((x >> 1) & ~0x1f); 
		DWORD block = blockTable16[(y >> 3) & 7][(x >> 4) & 3];
		return (page + block) << 7;
	}

	static DWORD BlockAddress16S(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + ((y >> 1) & ~0x1f) * bw + ((x >> 1) & ~0x1f); 
		DWORD block = blockTable16S[(y >> 3) & 7][(x >> 4) & 3];
		return (page + block) << 7;
	}

	static DWORD BlockAddress8(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + ((y >> 1) & ~0x1f) * ((bw+1)>>1) + ((x >> 2) & ~0x1f); 
		DWORD block = blockTable8[(y >> 4) & 3][(x >> 4) & 7];
		return (page + block) << 8;
	}

	static DWORD BlockAddress4(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + ((y >> 2) & ~0x1f) * ((bw+1)>>1) + ((x >> 2) & ~0x1f); 
		DWORD block = blockTable4[(y >> 4) & 7][(x >> 5) & 3];
		return (page + block) << 9;
	}

	static DWORD BlockAddress32Z(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + (y & ~0x1f) * bw + ((x >> 1) & ~0x1f); 
		DWORD block = blockTable32Z[(y >> 3) & 3][(x >> 3) & 7];
		return (page + block) << 6;
	}

	static DWORD BlockAddress16Z(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + ((y >> 1) & ~0x1f) * bw + ((x >> 1) & ~0x1f); 
		DWORD block = blockTable16Z[(y >> 3) & 7][(x >> 4) & 3];
		return (page + block) << 7;
	}

	static DWORD BlockAddress16SZ(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + ((y >> 1) & ~0x1f) * bw + ((x >> 1) & ~0x1f); 
		DWORD block = blockTable16SZ[(y >> 3) & 7][(x >> 4) & 3];
		return (page + block) << 7;
	}

	static DWORD PixelAddressOrg32(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + (y & ~0x1f) * bw + ((x >> 1) & ~0x1f);
		DWORD block = blockTable32[(y >> 3) & 3][(x >> 3) & 7];
		DWORD word = ((page + block) << 6) + columnTable32[y & 7][x & 7];
		ASSERT(word < 1024*1024);
		return word;
	}

	static DWORD PixelAddressOrg16(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + ((y >> 1) & ~0x1f) * bw + ((x >> 1) & ~0x1f); 
		DWORD block = blockTable16[(y >> 3) & 7][(x >> 4) & 3];
		DWORD word = ((page + block) << 7) + columnTable16[y & 7][x & 15];
		ASSERT(word < 1024*1024*2);
		return word;
	}

	static DWORD PixelAddressOrg16S(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + ((y >> 1) & ~0x1f) * bw + ((x >> 1) & ~0x1f); 
		DWORD block = blockTable16S[(y >> 3) & 7][(x >> 4) & 3];
		DWORD word = ((page + block) << 7) + columnTable16[y & 7][x & 15];
		ASSERT(word < 1024*1024*2);
		return word;
	}

	static DWORD PixelAddressOrg8(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + ((y >> 1) & ~0x1f) * ((bw + 1)>>1) + ((x >> 2) & ~0x1f); 
		DWORD block = blockTable8[(y >> 4) & 3][(x >> 4) & 7];
		DWORD word = ((page + block) << 8) + columnTable8[y & 15][x & 15];
	//	ASSERT(word < 1024*1024*4);
		return word;
	}

	static DWORD PixelAddressOrg4(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + ((y >> 2) & ~0x1f) * ((bw + 1)>>1) + ((x >> 2) & ~0x1f); 
		DWORD block = blockTable4[(y >> 4) & 7][(x >> 5) & 3];
		DWORD word = ((page + block) << 9) + columnTable4[y & 15][x & 31];
		ASSERT(word < 1024*1024*8);
		return word;
	}

	static DWORD PixelAddressOrg32Z(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + (y & ~0x1f) * bw + ((x >> 1) & ~0x1f); 
		DWORD block = blockTable32Z[(y >> 3) & 3][(x >> 3) & 7];
		DWORD word = ((page + block) << 6) + columnTable32[y & 7][x & 7];
		ASSERT(word < 1024*1024);
		return word;
	}

	static DWORD PixelAddressOrg16Z(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + ((y >> 1) & ~0x1f) * bw + ((x >> 1) & ~0x1f); 
		DWORD block = blockTable16Z[(y >> 3) & 7][(x >> 4) & 3];
		DWORD word = ((page + block) << 7) + columnTable16[y & 7][x & 15];
		ASSERT(word < 1024*1024*2);
		return word;
	}

	static DWORD PixelAddressOrg16SZ(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + ((y >> 1) & ~0x1f) * bw + ((x >> 1) & ~0x1f); 
		DWORD block = blockTable16SZ[(y >> 3) & 7][(x >> 4) & 3];
		DWORD word = ((page + block) << 7) + columnTable16[y & 7][x & 15];
		ASSERT(word < 1024*1024*2);
		return word;
	}

	static __forceinline DWORD PixelAddress32(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = (bp >> 5) + (y >> 5) * bw + (x >> 6); 
		DWORD word = (page << 11) + pageOffset32[bp & 0x1f][y & 0x1f][x & 0x3f];
		return word;
	}

	static __forceinline DWORD PixelAddress16(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = (bp >> 5) + (y >> 6) * bw + (x >> 6); 
		DWORD word = (page << 12) + pageOffset16[bp & 0x1f][y & 0x3f][x & 0x3f];
		return word;
	}

	static __forceinline DWORD PixelAddress16S(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = (bp >> 5) + (y >> 6) * bw + (x >> 6); 
		DWORD word = (page << 12) + pageOffset16S[bp & 0x1f][y & 0x3f][x & 0x3f];
		return word;
	}

	static __forceinline DWORD PixelAddress8(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = (bp >> 5) + (y >> 6) * ((bw + 1)>>1) + (x >> 7); 
		DWORD word = (page << 13) + pageOffset8[bp & 0x1f][y & 0x3f][x & 0x7f];
		return word;
	}

	static __forceinline DWORD PixelAddress4(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = (bp >> 5) + (y >> 7) * ((bw + 1)>>1) + (x >> 7);
		DWORD word = (page << 14) + pageOffset4[bp & 0x1f][y & 0x7f][x & 0x7f];
		return word;
	}

	static __forceinline DWORD PixelAddress32Z(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = (bp >> 5) + (y >> 5) * bw + (x >> 6); 
		DWORD word = (page << 11) + pageOffset32Z[bp & 0x1f][y & 0x1f][x & 0x3f];
		return word;
	}

	static __forceinline DWORD PixelAddress16Z(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = (bp >> 5) + (y >> 6) * bw + (x >> 6); 
		DWORD word = (page << 12) + pageOffset16Z[bp & 0x1f][y & 0x3f][x & 0x3f];
		return word;
	}

	static __forceinline DWORD PixelAddress16SZ(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = (bp >> 5) + (y >> 6) * bw + (x >> 6); 
		DWORD word = (page << 12) + pageOffset16SZ[bp & 0x1f][y & 0x3f][x & 0x3f];
		return word;
	}

	// pixel R/W

	__forceinline DWORD ReadPixel32(DWORD addr) 
	{
		return m_vm32[addr];
	}

	__forceinline DWORD ReadPixel24(DWORD addr) 
	{
		return m_vm32[addr] & 0x00ffffff;
	}

	__forceinline DWORD ReadPixel16(DWORD addr) 
	{
		return (DWORD)m_vm16[addr];
	}

	__forceinline DWORD ReadPixel8(DWORD addr) 
	{
		return (DWORD)m_vm8[addr];
	}

	__forceinline DWORD ReadPixel4(DWORD addr) 
	{
		return (m_vm8[addr >> 1] >> ((addr & 1) << 2)) & 0x0f;
	}

	__forceinline DWORD ReadPixel8H(DWORD addr) 
	{
		return m_vm32[addr] >> 24;
	}

	__forceinline DWORD ReadPixel4HL(DWORD addr) 
	{
		return (m_vm32[addr] >> 24) & 0x0f;
	}

	__forceinline DWORD ReadPixel4HH(DWORD addr) 
	{
		return (m_vm32[addr] >> 28) & 0x0f;
	}

	__forceinline DWORD ReadFrame24(DWORD addr) 
	{
		return 0x80000000 | (m_vm32[addr] & 0xffffff);
	}

	__forceinline DWORD ReadFrame16(DWORD addr) 
	{
		DWORD c = (DWORD)m_vm16[addr];

		return ((c & 0x8000) << 16) | ((c & 0x7c00) << 9) | ((c & 0x03e0) << 6) | ((c & 0x001f) << 3);
	}

	__forceinline DWORD ReadPixel32(int x, int y, DWORD bp, DWORD bw)
	{
		return ReadPixel32(PixelAddress32(x, y, bp, bw));
	}

	__forceinline DWORD ReadPixel24(int x, int y, DWORD bp, DWORD bw)
	{
		return ReadPixel24(PixelAddress32(x, y, bp, bw));
	}

	__forceinline DWORD ReadPixel16(int x, int y, DWORD bp, DWORD bw)
	{
		return ReadPixel16(PixelAddress16(x, y, bp, bw));
	}

	__forceinline DWORD ReadPixel16S(int x, int y, DWORD bp, DWORD bw)
	{
		return ReadPixel16(PixelAddress16S(x, y, bp, bw));
	}

	__forceinline DWORD ReadPixel8(int x, int y, DWORD bp, DWORD bw)
	{
		return ReadPixel8(PixelAddress8(x, y, bp, bw));
	}

	__forceinline DWORD ReadPixel4(int x, int y, DWORD bp, DWORD bw)
	{
		return ReadPixel4(PixelAddress4(x, y, bp, bw));
	}

	__forceinline DWORD ReadPixel8H(int x, int y, DWORD bp, DWORD bw)
	{
		return ReadPixel8H(PixelAddress32(x, y, bp, bw));
	}

	__forceinline DWORD ReadPixel4HL(int x, int y, DWORD bp, DWORD bw)
	{
		return ReadPixel4HL(PixelAddress32(x, y, bp, bw));
	}

	__forceinline DWORD ReadPixel4HH(int x, int y, DWORD bp, DWORD bw)
	{
		return ReadPixel4HH(PixelAddress32(x, y, bp, bw));
	}

	__forceinline DWORD ReadPixel32Z(int x, int y, DWORD bp, DWORD bw)
	{
		return ReadPixel32(PixelAddress32Z(x, y, bp, bw));
	}

	__forceinline DWORD ReadPixel24Z(int x, int y, DWORD bp, DWORD bw)
	{
		return ReadPixel24(PixelAddress32Z(x, y, bp, bw));
	}

	__forceinline DWORD ReadPixel16Z(int x, int y, DWORD bp, DWORD bw)
	{
		return ReadPixel16(PixelAddress16Z(x, y, bp, bw));
	}

	__forceinline DWORD ReadPixel16SZ(int x, int y, DWORD bp, DWORD bw)
	{
		return ReadPixel16(PixelAddress16SZ(x, y, bp, bw));
	}

	__forceinline DWORD ReadFrame24(int x, int y, DWORD bp, DWORD bw)
	{
		return ReadFrame24(PixelAddress32(x, y, bp, bw));
	}

	__forceinline DWORD ReadFrame16(int x, int y, DWORD bp, DWORD bw)
	{
		return ReadFrame16(PixelAddress16(x, y, bp, bw));
	}

	__forceinline DWORD ReadFrame16S(int x, int y, DWORD bp, DWORD bw)
	{
		return ReadFrame16(PixelAddress16S(x, y, bp, bw));
	}

	__forceinline DWORD ReadFrame24Z(int x, int y, DWORD bp, DWORD bw)
	{
		return ReadFrame24(PixelAddress32Z(x, y, bp, bw));
	}

	__forceinline DWORD ReadFrame16Z(int x, int y, DWORD bp, DWORD bw)
	{
		return ReadFrame16(PixelAddress16Z(x, y, bp, bw));
	}

	__forceinline DWORD ReadFrame16SZ(int x, int y, DWORD bp, DWORD bw)
	{
		return ReadFrame16(PixelAddress16SZ(x, y, bp, bw));
	}

	__forceinline void WritePixel32(DWORD addr, DWORD c) 
	{
		m_vm32[addr] = c;
	}

	__forceinline void WritePixel24(DWORD addr, DWORD c) 
	{
		m_vm32[addr] = (m_vm32[addr] & 0xff000000) | (c & 0x00ffffff);
	}

	__forceinline void WritePixel16(DWORD addr, DWORD c) 
	{
		m_vm16[addr] = (WORD)c;
	}

	__forceinline void WritePixel8(DWORD addr, DWORD c)
	{
		m_vm8[addr] = (BYTE)c;
	}

	__forceinline void WritePixel4(DWORD addr, DWORD c) 
	{
		int shift = (addr & 1) << 2; addr >>= 1; 

		m_vm8[addr] = (BYTE)((m_vm8[addr] & (0xf0 >> shift)) | ((c & 0x0f) << shift));
	}

	__forceinline void WritePixel8H(DWORD addr, DWORD c)
	{
		m_vm32[addr] = (m_vm32[addr] & 0x00ffffff) | (c << 24);
	}

	__forceinline void WritePixel4HL(DWORD addr, DWORD c) 
	{
		m_vm32[addr] = (m_vm32[addr] & 0xf0ffffff) | ((c & 0x0f) << 24);
	}

	__forceinline void WritePixel4HH(DWORD addr, DWORD c)
	{
		m_vm32[addr] = (m_vm32[addr] & 0x0fffffff) | ((c & 0x0f) << 28);
	}

	__forceinline void WriteFrame16(DWORD addr, DWORD c) 
	{
		DWORD rb = c & 0x00f800f8;
		DWORD ga = c & 0x8000f800;

		WritePixel16(addr, (ga >> 16) | (rb >> 9) | (ga >> 6) | (rb >> 3));
	}

	__forceinline void WritePixel32(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		WritePixel32(PixelAddress32(x, y, bp, bw), c);
	}

	__forceinline void WritePixel24(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		WritePixel24(PixelAddress32(x, y, bp, bw), c);
	}

	__forceinline void WritePixel16(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		WritePixel16(PixelAddress16(x, y, bp, bw), c);
	}

	__forceinline void WritePixel16S(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		WritePixel16(PixelAddress16S(x, y, bp, bw), c);
	}

	__forceinline void WritePixel8(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		WritePixel8(PixelAddress8(x, y, bp, bw), c);
	}

	__forceinline void WritePixel4(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		WritePixel4(PixelAddress4(x, y, bp, bw), c);
	}

	__forceinline void WritePixel8H(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		WritePixel8H(PixelAddress32(x, y, bp, bw), c);
	}

    __forceinline void WritePixel4HL(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		WritePixel4HL(PixelAddress32(x, y, bp, bw), c);
	}

	__forceinline void WritePixel4HH(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		WritePixel4HH(PixelAddress32(x, y, bp, bw), c);
	}

	__forceinline void WritePixel32Z(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		WritePixel32(PixelAddress32Z(x, y, bp, bw), c);
	}

	__forceinline void WritePixel24Z(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		WritePixel24(PixelAddress32Z(x, y, bp, bw), c);
	}

	__forceinline void WritePixel16Z(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		WritePixel16(PixelAddress16Z(x, y, bp, bw), c);
	}

	__forceinline void WritePixel16SZ(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		WritePixel16(PixelAddress16SZ(x, y, bp, bw), c);
	}

	__forceinline void WriteFrame16(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		WriteFrame16(PixelAddress16(x, y, bp, bw), c);
	}

	__forceinline void WriteFrame16S(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		WriteFrame16(PixelAddress16S(x, y, bp, bw), c);
	}

	__forceinline void WriteFrame16Z(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		WriteFrame16(PixelAddress16Z(x, y, bp, bw), c);
	}

	__forceinline void WriteFrame16SZ(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		WriteFrame16(PixelAddress16SZ(x, y, bp, bw), c);
	}

	__forceinline DWORD ReadTexel32(DWORD addr, GIFRegTEXA& TEXA) 
	{
		return m_vm32[addr];
	}

	__forceinline DWORD ReadTexel24(DWORD addr, GIFRegTEXA& TEXA) 
	{
		return Expand24To32(m_vm32[addr], TEXA);
	}

	__forceinline DWORD ReadTexel16(DWORD addr, GIFRegTEXA& TEXA) 
	{
		return Expand16To32(m_vm16[addr], TEXA);
	}

	__forceinline DWORD ReadTexel8(DWORD addr, GIFRegTEXA& TEXA) 
	{
		return m_clut32[ReadPixel8(addr)];
	}

	__forceinline DWORD ReadTexel4(DWORD addr, GIFRegTEXA& TEXA) 
	{
		return m_clut32[ReadPixel4(addr)];
	}

	__forceinline DWORD ReadTexel8H(DWORD addr, GIFRegTEXA& TEXA) 
	{
		return m_clut32[ReadPixel8H(addr)];
	}

	__forceinline DWORD ReadTexel4HL(DWORD addr, GIFRegTEXA& TEXA)
	{
		return m_clut32[ReadPixel4HL(addr)];
	}

	__forceinline DWORD ReadTexel4HH(DWORD addr, GIFRegTEXA& TEXA) 
	{
		return m_clut32[ReadPixel4HH(addr)];
	}

	__forceinline DWORD ReadTexel32(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return ReadTexel32(PixelAddress32(x, y, TEX0.TBP0, TEX0.TBW), TEXA);
	}

	__forceinline DWORD ReadTexel24(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return ReadTexel24(PixelAddress32(x, y, TEX0.TBP0, TEX0.TBW), TEXA);
	}

	__forceinline DWORD ReadTexel16(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return ReadTexel16(PixelAddress16(x, y, TEX0.TBP0, TEX0.TBW), TEXA);
	}

	__forceinline DWORD ReadTexel16S(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return ReadTexel16(PixelAddress16S(x, y, TEX0.TBP0, TEX0.TBW), TEXA);
	}

	__forceinline DWORD ReadTexel8(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return ReadTexel8(PixelAddress8(x, y, TEX0.TBP0, TEX0.TBW), TEXA);
	}

	__forceinline DWORD ReadTexel4(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return ReadTexel4(PixelAddress4(x, y, TEX0.TBP0, TEX0.TBW), TEXA);
	}

	__forceinline DWORD ReadTexel8H(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return ReadTexel8H(PixelAddress32(x, y, TEX0.TBP0, TEX0.TBW), TEXA);
	}

	__forceinline DWORD ReadTexel4HL(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return ReadTexel4HL(PixelAddress32(x, y, TEX0.TBP0, TEX0.TBW), TEXA);
	}

	__forceinline DWORD ReadTexel4HH(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return ReadTexel4HH(PixelAddress32(x, y, TEX0.TBP0, TEX0.TBW), TEXA);
	}

	__forceinline DWORD ReadTexel32Z(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return ReadTexel32(PixelAddress32Z(x, y, TEX0.TBP0, TEX0.TBW), TEXA);
	}

	__forceinline DWORD ReadTexel24Z(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return ReadTexel24(PixelAddress32Z(x, y, TEX0.TBP0, TEX0.TBW), TEXA);
	}

	__forceinline DWORD ReadTexel16Z(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return ReadTexel16(PixelAddress16Z(x, y, TEX0.TBP0, TEX0.TBW), TEXA);
	}

	__forceinline DWORD ReadTexel16SZ(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return ReadTexel16(PixelAddress16SZ(x, y, TEX0.TBP0, TEX0.TBW), TEXA);
	}

	__forceinline DWORD ReadTexel16NP(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return ReadPixel16(x, y, TEX0.TBP0, TEX0.TBW);
	}

	__forceinline DWORD ReadTexel16SNP(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return ReadPixel16S(x, y, TEX0.TBP0, TEX0.TBW);
	}

	__forceinline DWORD ReadTexel16ZNP(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return ReadPixel16Z(x, y, TEX0.TBP0, TEX0.TBW);
	}

	__forceinline DWORD ReadTexel16SZNP(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return ReadPixel16SZ(x, y, TEX0.TBP0, TEX0.TBW);
	}

	//

	__forceinline DWORD PixelAddressX(int PSM, int x, int y, DWORD bp, DWORD bw)
	{
		switch(PSM)
		{
		case PSM_PSMCT32: return PixelAddress32(x, y, bp, bw); 
		case PSM_PSMCT24: return PixelAddress32(x, y, bp, bw); 
		case PSM_PSMCT16: return PixelAddress16(x, y, bp, bw);
		case PSM_PSMCT16S: return PixelAddress16S(x, y, bp, bw);
		case PSM_PSMT8: return PixelAddress8(x, y, bp, bw);
		case PSM_PSMT4: return PixelAddress4(x, y, bp, bw);
		case PSM_PSMT8H: return PixelAddress32(x, y, bp, bw);
		case PSM_PSMT4HL: return PixelAddress32(x, y, bp, bw);
		case PSM_PSMT4HH: return PixelAddress32(x, y, bp, bw);
		case PSM_PSMZ32: return PixelAddress32Z(x, y, bp, bw);
		case PSM_PSMZ24: return PixelAddress32Z(x, y, bp, bw);
		case PSM_PSMZ16: return PixelAddress16Z(x, y, bp, bw);
		case PSM_PSMZ16S: return PixelAddress16SZ(x, y, bp, bw);
		default: ASSERT(0); return PixelAddress32(x, y, bp, bw);
		}
	}

	__forceinline DWORD ReadPixelX(int PSM, DWORD addr)
	{
		switch(PSM)
		{
		case PSM_PSMCT32: return ReadPixel32(addr); 
		case PSM_PSMCT24: return ReadPixel24(addr); 
		case PSM_PSMCT16: return ReadPixel16(addr);
		case PSM_PSMCT16S: return ReadPixel16(addr);
		case PSM_PSMT8: return ReadPixel8(addr);
		case PSM_PSMT4: return ReadPixel4(addr);
		case PSM_PSMT8H: return ReadPixel8H(addr);
		case PSM_PSMT4HL: return ReadPixel4HL(addr);
		case PSM_PSMT4HH: return ReadPixel4HH(addr);
		case PSM_PSMZ32: return ReadPixel32(addr);
		case PSM_PSMZ24: return ReadPixel24(addr);
		case PSM_PSMZ16: return ReadPixel16(addr);
		case PSM_PSMZ16S: return ReadPixel16(addr);
		default: ASSERT(0); return ReadPixel32(addr);
		}
	}
	
	__forceinline DWORD ReadFrameX(int PSM, DWORD addr)
	{
		switch(PSM)
		{
		case PSM_PSMCT32: return ReadPixel32(addr);
		case PSM_PSMCT24: return ReadFrame24(addr);
		case PSM_PSMCT16: return ReadFrame16(addr);
		case PSM_PSMCT16S: return ReadFrame16(addr);
		case PSM_PSMZ32: return ReadPixel32(addr);
		case PSM_PSMZ24: return ReadFrame24(addr);
		case PSM_PSMZ16: return ReadFrame16(addr);
		case PSM_PSMZ16S: return ReadFrame16(addr);
		default: ASSERT(0); return ReadPixel32(addr);
		}
	}

	__forceinline DWORD ReadTexelX(int PSM, DWORD addr, GIFRegTEXA& TEXA)
	{
		switch(PSM)
		{
		case PSM_PSMCT32: return ReadTexel32(addr, TEXA);
		case PSM_PSMCT24: return ReadTexel24(addr, TEXA);
		case PSM_PSMCT16: return ReadTexel16(addr, TEXA);
		case PSM_PSMCT16S: return ReadTexel16(addr, TEXA);
		case PSM_PSMT8: return ReadTexel8(addr, TEXA);
		case PSM_PSMT4: return ReadTexel4(addr, TEXA);
		case PSM_PSMT8H: return ReadTexel8H(addr, TEXA);
		case PSM_PSMT4HL: return ReadTexel4HL(addr, TEXA);
		case PSM_PSMT4HH: return ReadTexel4HH(addr, TEXA);
		case PSM_PSMZ32: return ReadTexel32(addr, TEXA);
		case PSM_PSMZ24: return ReadTexel24(addr, TEXA);
		case PSM_PSMZ16: return ReadTexel16(addr, TEXA);
		case PSM_PSMZ16S: return ReadTexel16(addr, TEXA);
		default: ASSERT(0); return ReadTexel32(addr, TEXA);
		}
	}

	__forceinline DWORD ReadTexelX(int PSM, int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		switch(PSM)
		{
		case PSM_PSMCT32: return ReadTexel32(x, y, TEX0, TEXA);
		case PSM_PSMCT24: return ReadTexel24(x, y, TEX0, TEXA);
		case PSM_PSMCT16: return ReadTexel16(x, y, TEX0, TEXA);
		case PSM_PSMCT16S: return ReadTexel16(x, y, TEX0, TEXA);
		case PSM_PSMT8: return ReadTexel8(x, y, TEX0, TEXA);
		case PSM_PSMT4: return ReadTexel4(x, y, TEX0, TEXA);
		case PSM_PSMT8H: return ReadTexel8H(x, y, TEX0, TEXA);
		case PSM_PSMT4HL: return ReadTexel4HL(x, y, TEX0, TEXA);
		case PSM_PSMT4HH: return ReadTexel4HH(x, y, TEX0, TEXA);
		case PSM_PSMZ32: return ReadTexel32Z(x, y, TEX0, TEXA);
		case PSM_PSMZ24: return ReadTexel24Z(x, y, TEX0, TEXA);
		case PSM_PSMZ16: return ReadTexel16Z(x, y, TEX0, TEXA);
		case PSM_PSMZ16S: return ReadTexel16Z(x, y, TEX0, TEXA);
		default: ASSERT(0); return ReadTexel32(x, y, TEX0, TEXA);
		}
	}

	__forceinline void WritePixelX(int PSM, DWORD addr, DWORD c)
	{
		switch(PSM)
		{
		case PSM_PSMCT32: WritePixel32(addr, c); break; 
		case PSM_PSMCT24: WritePixel24(addr, c); break; 
		case PSM_PSMCT16: WritePixel16(addr, c); break;
		case PSM_PSMCT16S: WritePixel16(addr, c); break;
		case PSM_PSMT8: WritePixel8(addr, c); break;
		case PSM_PSMT4: WritePixel4(addr, c); break;
		case PSM_PSMT8H: WritePixel8H(addr, c); break;
		case PSM_PSMT4HL: WritePixel4HL(addr, c); break;
		case PSM_PSMT4HH: WritePixel4HH(addr, c); break;
		case PSM_PSMZ32: WritePixel32(addr, c); break;
		case PSM_PSMZ24: WritePixel24(addr, c); break;
		case PSM_PSMZ16: WritePixel16(addr, c); break;
		case PSM_PSMZ16S: WritePixel16(addr, c); break;
		default: ASSERT(0); WritePixel32(addr, c); break;
		}
	}

	__forceinline void WriteFrameX(int PSM, DWORD addr, DWORD c)
	{
		switch(PSM)
		{
		case PSM_PSMCT32: WritePixel32(addr, c); break; 
		case PSM_PSMCT24: WritePixel24(addr, c); break; 
		case PSM_PSMCT16: WriteFrame16(addr, c); break;
		case PSM_PSMCT16S: WriteFrame16(addr, c); break;
		case PSM_PSMZ32: WritePixel32(addr, c); break; 
		case PSM_PSMZ24: WritePixel24(addr, c); break; 
		case PSM_PSMZ16: WriteFrame16(addr, c); break;
		case PSM_PSMZ16S: WriteFrame16(addr, c); break;
		default: ASSERT(0); WritePixel32(addr, c); break;
		}
	}

	__forceinline GSVector4i ReadFrameX(int PSM, const GSVector4i& addr)
	{
		GSVector4i c, r, g, b, a;

		// TODO

		switch(PSM)
		{
		case PSM_PSMCT32: 
		case PSM_PSMZ32: 
			#if _M_SSE >= 0x400
			c = addr.gather32_32(m_vm32);
			#else
			c = GSVector4i(
				(int)ReadPixel32(addr.u32[0]), 
				(int)ReadPixel32(addr.u32[1]), 
				(int)ReadPixel32(addr.u32[2]), 
				(int)ReadPixel32(addr.u32[3]));
			#endif
			break;
		case PSM_PSMCT24: 
		case PSM_PSMZ24: 
			#if _M_SSE >= 0x400
			c = addr.gather32_32(m_vm32);
			#else
			c = GSVector4i(
				(int)ReadPixel32(addr.u32[0]), 
				(int)ReadPixel32(addr.u32[1]), 
				(int)ReadPixel32(addr.u32[2]), 
				(int)ReadPixel32(addr.u32[3]));
			#endif
			c = (c & 0x00ffffff) | 0x80000000;
			break;
		case PSM_PSMCT16: 
		case PSM_PSMCT16S: 
		case PSM_PSMZ16: 
		case PSM_PSMZ16S: 
			#if _M_SSE >= 0x400
			c = addr.gather32_32(m_vm16);
			#else
			c = GSVector4i(
				(int)ReadPixel16(addr.u32[0]), 
				(int)ReadPixel16(addr.u32[1]), 
				(int)ReadPixel16(addr.u32[2]), 
				(int)ReadPixel16(addr.u32[3]));
			#endif
			c = ((c & 0x8000) << 16) | ((c & 0x7c00) << 9) | ((c & 0x03e0) << 6) | ((c & 0x001f) << 3); 
			break;
		default: 
			ASSERT(0); 
			c = GSVector4i::zero();
		}
		
		return c;
	}

	__forceinline GSVector4i ReadZBufX(int PSM, const GSVector4i& addr)
	{
		GSVector4i z;

		switch(PSM)
		{
		case PSM_PSMZ32: 
			#if _M_SSE >= 0x400
			z = addr.gather32_32(m_vm32);
			#else
			z = GSVector4i(
				(int)ReadPixel32(addr.u32[0]), 
				(int)ReadPixel32(addr.u32[1]), 
				(int)ReadPixel32(addr.u32[2]), 
				(int)ReadPixel32(addr.u32[3]));
			#endif
			break;
		case PSM_PSMZ24: 
			#if _M_SSE >= 0x400
			z = addr.gather32_32(m_vm32) & 0x00ffffff;
			#else
			z = GSVector4i(
				(int)ReadPixel32(addr.u32[0]), 
				(int)ReadPixel32(addr.u32[1]), 
				(int)ReadPixel32(addr.u32[2]), 
				(int)ReadPixel32(addr.u32[3]));
			z = z & 0x00ffffff;
			#endif
			break;
		case PSM_PSMZ16: 
		case PSM_PSMZ16S: 
			#if _M_SSE >= 0x400
			z = addr.gather32_32(m_vm16);
			#else
			z = GSVector4i(
				(int)ReadPixel16(addr.u32[0]), 
				(int)ReadPixel16(addr.u32[1]), 
				(int)ReadPixel16(addr.u32[2]), 
				(int)ReadPixel16(addr.u32[3]));
			#endif
			break;
		default: 
			ASSERT(0); 
			z = GSVector4i::zero();
		}

		return z;
	}

	__forceinline void WriteFrameX(int PSM, const GSVector4i& addr, const GSVector4i& c, const GSVector4i& mask, int pixels)
	{
		GSVector4i rb, ga, tmp;

		switch(PSM)
		{
		case PSM_PSMCT32: 
		case PSM_PSMZ32: 
			for(int i = 0; i < pixels; i++)
				if(mask.u32[i] != 0xffffffff)
					WritePixel32(addr.u32[i], c.u32[i]); 
			break; 
		case PSM_PSMCT24: 
		case PSM_PSMZ24: 
			for(int i = 0; i < pixels; i++)
				if(mask.u32[i] != 0xffffffff)
					WritePixel24(addr.u32[i], c.u32[i]); 
			break; 
		case PSM_PSMCT16: 
		case PSM_PSMCT16S: 
		case PSM_PSMZ16: 
		case PSM_PSMZ16S: 
			rb = c & 0x00f800f8;
			ga = c & 0x8000f800;
			tmp = (ga >> 16) | (rb >> 9) | (ga >> 6) | (rb >> 3);
			for(int i = 0; i < pixels; i++)
				if(mask.u32[i] != 0xffffffff)
					WritePixel16(addr.u32[i], tmp.u16[i*2]); 
			break;
		default: 
			ASSERT(0); 
			break;
		}
	}

	__forceinline void WriteZBufX(int PSM, const GSVector4i& addr, const GSVector4i& z, const GSVector4i& mask, int pixels)
	{
		switch(PSM)
		{
		case PSM_PSMZ32: 
			for(int i = 0; i < pixels; i++)
				if(mask.u32[i] != 0xffffffff)
					WritePixel32(addr.u32[i], z.u32[i]); 
			break; 
		case PSM_PSMZ24: 
			for(int i = 0; i < pixels; i++)
				if(mask.u32[i] != 0xffffffff)
					WritePixel24(addr.u32[i], z.u32[i]); 
			break; 
		case PSM_PSMZ16: 
		case PSM_PSMZ16S: 
			for(int i = 0; i < pixels; i++)
				if(mask.u32[i] != 0xffffffff)
					WritePixel16(addr.u32[i], z.u32[i]); 
			break; 
		default: 
			ASSERT(0); 
			break;
		}
	}

	// FillRect

	bool FillRect(const CRect& r, DWORD c, DWORD psm, DWORD fbp, DWORD fbw);

	// CLUT

	void InvalidateCLUT() {m_fCLUTMayBeDirty = true;}
	bool IsCLUTDirty(GIFRegTEX0 TEX0, GIFRegTEXCLUT TEXCLUT);
	bool IsCLUTUpdating(GIFRegTEX0 TEX0, GIFRegTEXCLUT TEXCLUT);
	bool WriteCLUT(GIFRegTEX0 TEX0, GIFRegTEXCLUT TEXCLUT);

	void ReadCLUT(GIFRegTEX0 TEX0, DWORD* clut32);
	void SetupCLUT(GIFRegTEX0 TEX0);

	// expands 16->32

	void ReadCLUT32(GIFRegTEX0 TEX0, GIFRegTEXA TEXA, DWORD* clut32);
	void SetupCLUT32(GIFRegTEX0 TEX0, GIFRegTEXA TEXA);
	void CopyCLUT32(DWORD* clut32, int n);
	DWORD* GetCLUT32() {return m_clut32;}

	//

	void WriteImage32(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);
	void WriteImage24(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);
	void WriteImage16(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);
	void WriteImage16S(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);
	void WriteImage8(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);
	void WriteImage4(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);
	void WriteImage8H(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);
	void WriteImage4HL(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);
	void WriteImage4HH(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);
	void WriteImage32Z(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);
	void WriteImage24Z(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);
	void WriteImage16Z(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);
	void WriteImage16SZ(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);
	void WriteImageX(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);

	// TODO: ReadImage32/24/...

	void ReadImageX(int& tx, int& ty, BYTE* dst, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);

	//

	void ReadTexture32(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void ReadTexture24(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void ReadTexture16(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void ReadTexture16S(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void ReadTexture8(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void ReadTexture4(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void ReadTexture8H(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void ReadTexture4HL(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void ReadTexture4HH(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void ReadTexture32Z(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void ReadTexture24Z(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void ReadTexture16Z(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void ReadTexture16SZ(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);

	void ReadTexture(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA, GIFRegCLAMP& CLAMP);
	void ReadTextureNC(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA, GIFRegCLAMP& CLAMP);

	// 32/16

	void ReadTexture16NP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void ReadTexture16SNP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void ReadTexture8NP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void ReadTexture4NP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void ReadTexture8HNP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void ReadTexture4HLNP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void ReadTexture4HHNP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void ReadTexture16ZNP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void ReadTexture16SZNP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);

	void ReadTextureNP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA, GIFRegCLAMP& CLAMP);
	void ReadTextureNPNC(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA, GIFRegCLAMP& CLAMP);

	//

	static DWORD m_xtbl[1024], m_ytbl[1024]; 

	template<typename T> void ReadTexture(CRect r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA, GIFRegCLAMP& CLAMP, readTexel rt, readTexture rtx);
	template<typename T> void ReadTextureNC(CRect r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA, readTexel rt, readTexture rtx);

	//

	HRESULT SaveBMP(LPCTSTR fn, DWORD bp, DWORD bw, DWORD psm, int w, int h);
};

#pragma warning(default: 4244)