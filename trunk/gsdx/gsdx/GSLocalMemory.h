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
#pragma warning(disable: 4244) // warning C4244: '=' : conversion from 'const UINT64' to 'int', possible loss of data (really???)

#include "GS.h"
#include "GSTables.h"

class GSLocalMemory
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
	typedef void (GSLocalMemory::*SwizzleTexture)(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);
	typedef void (GSLocalMemory::*unSwizzleTexture)(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	typedef void (GSLocalMemory::*readTexture)(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA, GIFRegCLAMP& CLAMP);

	typedef union 
	{
		struct
		{
			pixelAddress pa, ba, pga;
			readPixel rp;
			readPixelAddr rpa;
			writePixel wp;
			writePixelAddr wpa;
			readTexel rt, rtP, rtNP;
			readTexelAddr rta;
			writeFrameAddr wfa;
			SwizzleTexture st;
			unSwizzleTexture ust, ustP, ustNP;
			DWORD bpp, pal, trbpp; 
			CSize bs, pgs;
			int* rowOffset[8];
		};
		BYTE dummy[128];
	} psm_t;

	static psm_t m_psm[64];

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

	union {BYTE* m_vm8; WORD* m_vm16; DWORD* m_vm32;};

	DWORD m_CBP[2];
	WORD* m_pCLUT;
	DWORD* m_pCLUT32;
	UINT64* m_pCLUT64;

	GIFRegTEX0 m_prevTEX0;
	GIFRegTEXCLUT m_prevTEXCLUT;
	bool m_fCLUTMayBeDirty;

public:
	GSLocalMemory();
	virtual ~GSLocalMemory();

	BYTE* GetVM() 
	{
		return m_vm8;
	}

	template<class T>
	static void RoundDown(T& x, T& y, CSize bs)
	{
		x &= ~(bs.cx-1);
		y &= ~(bs.cy-1);
	}

	template<class T>
	static void RoundUp(T& x, T& y, CSize bs)
	{
		x = (x + (bs.cx-1)) & ~(bs.cx-1);
		y = (y + (bs.cy-1)) & ~(bs.cy-1);
	}

	__forceinline static DWORD Expand24To32(DWORD c, GIFRegTEXA& TEXA)
	{
		return (((!TEXA.AEM | (c & 0xffffff)) ? TEXA.TA0 : 0) << 24) | (c & 0xffffff);
	}

	__forceinline static DWORD Expand16To32(WORD c, GIFRegTEXA& TEXA)
	{
		return (((c & 0x8000) ? TEXA.TA1 : (!TEXA.AEM | c) ? TEXA.TA0 : 0) << 24) | ((c & 0x7c00) << 9) | ((c & 0x03e0) << 6) | ((c & 0x001f) << 3);
	}

	// address

	static DWORD pageAddress32(int x, int y, DWORD bp, DWORD bw)
	{
		return ((bp >> 5) + (y >> 5) * bw + (x >> 6)) << 11; 
	}

	static DWORD pageAddress16(int x, int y, DWORD bp, DWORD bw)
	{
		return ((bp >> 5) + (y >> 6) * bw + (x >> 6)) << 12;
	}

	static DWORD pageAddress8(int x, int y, DWORD bp, DWORD bw)
	{
		return ((bp >> 5) + (y >> 6) * ((bw + 1) >> 1) + (x >> 7)) << 13; 
	}

	static DWORD pageAddress4(int x, int y, DWORD bp, DWORD bw)
	{
		return ((bp >> 5) + (y >> 7) * ((bw + 1) >> 1) + (x >> 7)) << 14;
	}

	static DWORD blockAddress32(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + (y & ~0x1f) * bw + ((x >> 1) & ~0x1f);
		DWORD block = blockTable32[(y >> 3) & 3][(x >> 3) & 7];
		return (page + block) << 6;
	}

	static DWORD blockAddress16(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + ((y >> 1) & ~0x1f) * bw + ((x >> 1) & ~0x1f); 
		DWORD block = blockTable16[(y >> 3) & 7][(x >> 4) & 3];
		return (page + block) << 7;
	}

	static DWORD blockAddress16S(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + ((y >> 1) & ~0x1f) * bw + ((x >> 1) & ~0x1f); 
		DWORD block = blockTable16S[(y >> 3) & 7][(x >> 4) & 3];
		return (page + block) << 7;
	}

	static DWORD blockAddress8(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + ((y >> 1) & ~0x1f) * ((bw+1)>>1) + ((x >> 2) & ~0x1f); 
		DWORD block = blockTable8[(y >> 4) & 3][(x >> 4) & 7];
		return (page + block) << 8;
	}

	static DWORD blockAddress4(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + ((y >> 2) & ~0x1f) * ((bw+1)>>1) + ((x >> 2) & ~0x1f); 
		DWORD block = blockTable4[(y >> 4) & 7][(x >> 5) & 3];
		return (page + block) << 9;
	}

	static DWORD blockAddress32Z(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + (y & ~0x1f) * bw + ((x >> 1) & ~0x1f); 
		DWORD block = blockTable32Z[(y >> 3) & 3][(x >> 3) & 7];
		return (page + block) << 6;
	}

	static DWORD blockAddress16Z(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + ((y >> 1) & ~0x1f) * bw + ((x >> 1) & ~0x1f); 
		DWORD block = blockTable16Z[(y >> 3) & 7][(x >> 4) & 3];
		return (page + block) << 7;
	}

	static DWORD blockAddress16SZ(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + ((y >> 1) & ~0x1f) * bw + ((x >> 1) & ~0x1f); 
		DWORD block = blockTable16SZ[(y >> 3) & 7][(x >> 4) & 3];
		return (page + block) << 7;
	}

	static DWORD pixelAddressOrg32(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + (y & ~0x1f) * bw + ((x >> 1) & ~0x1f);
		DWORD block = blockTable32[(y >> 3) & 3][(x >> 3) & 7];
		DWORD word = ((page + block) << 6) + columnTable32[y & 7][x & 7];
		ASSERT(word < 1024*1024);
		return word;
	}

	static DWORD pixelAddressOrg16(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + ((y >> 1) & ~0x1f) * bw + ((x >> 1) & ~0x1f); 
		DWORD block = blockTable16[(y >> 3) & 7][(x >> 4) & 3];
		DWORD word = ((page + block) << 7) + columnTable16[y & 7][x & 15];
		ASSERT(word < 1024*1024*2);
		return word;
	}

	static DWORD pixelAddressOrg16S(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + ((y >> 1) & ~0x1f) * bw + ((x >> 1) & ~0x1f); 
		DWORD block = blockTable16S[(y >> 3) & 7][(x >> 4) & 3];
		DWORD word = ((page + block) << 7) + columnTable16[y & 7][x & 15];
		ASSERT(word < 1024*1024*2);
		return word;
	}

	static DWORD pixelAddressOrg8(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + ((y >> 1) & ~0x1f) * ((bw + 1)>>1) + ((x >> 2) & ~0x1f); 
		DWORD block = blockTable8[(y >> 4) & 3][(x >> 4) & 7];
		DWORD word = ((page + block) << 8) + columnTable8[y & 15][x & 15];
	//	ASSERT(word < 1024*1024*4);
		return word;
	}

	static DWORD pixelAddressOrg4(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + ((y >> 2) & ~0x1f) * ((bw + 1)>>1) + ((x >> 2) & ~0x1f); 
		DWORD block = blockTable4[(y >> 4) & 7][(x >> 5) & 3];
		DWORD word = ((page + block) << 9) + columnTable4[y & 15][x & 31];
		ASSERT(word < 1024*1024*8);
		return word;
	}

	static DWORD pixelAddressOrg32Z(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + (y & ~0x1f) * bw + ((x >> 1) & ~0x1f); 
		DWORD block = blockTable32Z[(y >> 3) & 3][(x >> 3) & 7];
		DWORD word = ((page + block) << 6) + columnTable32[y & 7][x & 7];
		ASSERT(word < 1024*1024);
		return word;
	}

	static DWORD pixelAddressOrg16Z(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + ((y >> 1) & ~0x1f) * bw + ((x >> 1) & ~0x1f); 
		DWORD block = blockTable16Z[(y >> 3) & 7][(x >> 4) & 3];
		DWORD word = ((page + block) << 7) + columnTable16[y & 7][x & 15];
		ASSERT(word < 1024*1024*2);
		return word;
	}

	static DWORD pixelAddressOrg16SZ(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = bp + ((y >> 1) & ~0x1f) * bw + ((x >> 1) & ~0x1f); 
		DWORD block = blockTable16SZ[(y >> 3) & 7][(x >> 4) & 3];
		DWORD word = ((page + block) << 7) + columnTable16[y & 7][x & 15];
		ASSERT(word < 1024*1024*2);
		return word;
	}

	static DWORD pixelAddress32(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = (bp >> 5) + (y >> 5) * bw + (x >> 6); 
		DWORD word = (page << 11) + pageOffset32[bp & 0x1f][y & 0x1f][x & 0x3f];
		ASSERT(word < 1024*1024);
		return word;
	}

	static DWORD pixelAddress16(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = (bp >> 5) + (y >> 6) * bw + (x >> 6); 
		DWORD word = (page << 12) + pageOffset16[bp & 0x1f][y & 0x3f][x & 0x3f];
		ASSERT(word < 1024*1024*2);
		return word;
	}

	static DWORD pixelAddress16S(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = (bp >> 5) + (y >> 6) * bw + (x >> 6); 
		DWORD word = (page << 12) + pageOffset16S[bp & 0x1f][y & 0x3f][x & 0x3f];
		ASSERT(word < 1024*1024*2);
		return word;
	}

	static DWORD pixelAddress8(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = (bp >> 5) + (y >> 6) * ((bw + 1)>>1) + (x >> 7); 
		DWORD word = (page << 13) + pageOffset8[bp & 0x1f][y & 0x3f][x & 0x7f];
		ASSERT(word < 1024*1024*4);
		return word;
	}

	static DWORD pixelAddress4(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = (bp >> 5) + (y >> 7) * ((bw + 1)>>1) + (x >> 7);
		DWORD word = (page << 14) + pageOffset4[bp & 0x1f][y & 0x7f][x & 0x7f];
		ASSERT(word < 1024*1024*8);
		return word;
	}

	static DWORD pixelAddress32Z(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = (bp >> 5) + (y >> 5) * bw + (x >> 6); 
		DWORD word = (page << 11) + pageOffset32Z[bp & 0x1f][y & 0x1f][x & 0x3f];
		ASSERT(word < 1024*1024);
		return word;
	}

	static DWORD pixelAddress16Z(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = (bp >> 5) + (y >> 6) * bw + (x >> 6); 
		DWORD word = (page << 12) + pageOffset16Z[bp & 0x1f][y & 0x3f][x & 0x3f];
		ASSERT(word < 1024*1024*2);
		return word;
	}

	static DWORD pixelAddress16SZ(int x, int y, DWORD bp, DWORD bw)
	{
		DWORD page = (bp >> 5) + (y >> 6) * bw + (x >> 6); 
		DWORD word = (page << 12) + pageOffset16SZ[bp & 0x1f][y & 0x3f][x & 0x3f];
		ASSERT(word < 1024*1024*2);
		return word;
	}

	// pixel R/W

	__forceinline DWORD readPixel32(DWORD addr) 
	{
		return m_vm32[addr];
	}

	__forceinline DWORD readPixel24(DWORD addr) 
	{
		return m_vm32[addr] & 0x00ffffff;
	}

	__forceinline DWORD readPixel16(DWORD addr) 
	{
		return (DWORD)m_vm16[addr];
	}

	__forceinline DWORD readPixel16S(DWORD addr) 
	{
		return (DWORD)m_vm16[addr];
	}

	__forceinline DWORD readPixel8(DWORD addr) 
	{
		return (DWORD)m_vm8[addr];
	}

	__forceinline DWORD readPixel4(DWORD addr) 
	{
		return (m_vm8[addr>>1] >> ((addr&1) << 2)) & 0x0f;
	}

	__forceinline DWORD readPixel8H(DWORD addr) 
	{
		return m_vm32[addr] >> 24;
	}

	__forceinline DWORD readPixel4HL(DWORD addr) 
	{
		return (m_vm32[addr] >> 24) & 0x0f;
	}

	__forceinline DWORD readPixel4HH(DWORD addr) 
	{
		return (m_vm32[addr] >> 28) & 0x0f;
	}

	__forceinline DWORD readPixel32(int x, int y, DWORD bp, DWORD bw)
	{
		return readPixel32(pixelAddress32(x, y, bp, bw));
	}

	__forceinline DWORD readPixel24(int x, int y, DWORD bp, DWORD bw)
	{
		return readPixel24(pixelAddress32(x, y, bp, bw));
	}

	__forceinline DWORD readPixel16(int x, int y, DWORD bp, DWORD bw)
	{
		return readPixel16(pixelAddress16(x, y, bp, bw));
	}

	__forceinline DWORD readPixel16S(int x, int y, DWORD bp, DWORD bw)
	{
		return readPixel16S(pixelAddress16S(x, y, bp, bw));
	}

	__forceinline DWORD readPixel8(int x, int y, DWORD bp, DWORD bw)
	{
		return readPixel8(pixelAddress8(x, y, bp, bw));
	}

	__forceinline DWORD readPixel4(int x, int y, DWORD bp, DWORD bw)
	{
		return readPixel4(pixelAddress4(x, y, bp, bw));
	}

	__forceinline DWORD readPixel8H(int x, int y, DWORD bp, DWORD bw)
	{
		return readPixel8H(pixelAddress32(x, y, bp, bw));
	}

	__forceinline DWORD readPixel4HL(int x, int y, DWORD bp, DWORD bw)
	{
		return readPixel4HL(pixelAddress32(x, y, bp, bw));
	}

	__forceinline DWORD readPixel4HH(int x, int y, DWORD bp, DWORD bw)
	{
		return readPixel4HH(pixelAddress32(x, y, bp, bw));
	}

	__forceinline DWORD readPixel32Z(int x, int y, DWORD bp, DWORD bw)
	{
		return readPixel32(pixelAddress32Z(x, y, bp, bw));
	}

	__forceinline DWORD readPixel24Z(int x, int y, DWORD bp, DWORD bw)
	{
		return readPixel24(pixelAddress32Z(x, y, bp, bw));
	}

	__forceinline DWORD readPixel16Z(int x, int y, DWORD bp, DWORD bw)
	{
		return readPixel16(pixelAddress16Z(x, y, bp, bw));
	}

	__forceinline DWORD readPixel16SZ(int x, int y, DWORD bp, DWORD bw)
	{
		return readPixel16S(pixelAddress16SZ(x, y, bp, bw));
	}

	__forceinline void writePixel32(DWORD addr, DWORD c) 
	{
		m_vm32[addr] = c;
	}

	__forceinline void writePixel24(DWORD addr, DWORD c) 
	{
		m_vm32[addr] = (m_vm32[addr] & 0xff000000) | (c & 0x00ffffff);
	}

	__forceinline void writePixel16(DWORD addr, DWORD c) 
	{
		m_vm16[addr] = (WORD)c;
	}

	__forceinline void writePixel16S(DWORD addr, DWORD c)
	{
		m_vm16[addr] = (WORD)c;
	}

	__forceinline void writePixel8(DWORD addr, DWORD c)
	{
		m_vm8[addr] = (BYTE)c;
	}

	__forceinline void writePixel4(DWORD addr, DWORD c) 
	{
		int shift = (addr&1) << 2; addr >>= 1; 
		m_vm8[addr] = (BYTE)((m_vm8[addr] & (0xf0 >> shift)) | ((c & 0x0f) << shift));
	}

	__forceinline void writePixel8H(DWORD addr, DWORD c)
	{
		m_vm32[addr] = (m_vm32[addr] & 0x00ffffff) | (c << 24);
	}

	__forceinline void writePixel4HL(DWORD addr, DWORD c) 
	{
		m_vm32[addr] = (m_vm32[addr] & 0xf0ffffff) | ((c & 0x0f) << 24);
	}

	__forceinline void writePixel4HH(DWORD addr, DWORD c)
	{
		m_vm32[addr] = (m_vm32[addr] & 0x0fffffff) | ((c & 0x0f) << 28);
	}

	__forceinline void writeFrame16(DWORD addr, DWORD c) 
	{
		writePixel16(addr, ((c>>16)&0x8000) | ((c>>9)&0x7c00) | ((c>>6)&0x03e0) | ((c>>3)&0x001f));
	}

	__forceinline void writeFrame16S(DWORD addr, DWORD c) 
	{
		writePixel16S(addr, ((c>>16)&0x8000) | ((c>>9)&0x7c00) | ((c>>6)&0x03e0) | ((c>>3)&0x001f));
	}

	__forceinline void writePixel32(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		writePixel32(pixelAddress32(x, y, bp, bw), c);
	}

	__forceinline void writePixel24(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		writePixel24(pixelAddress32(x, y, bp, bw), c);
	}

	__forceinline void writePixel16(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		writePixel16(pixelAddress16(x, y, bp, bw), c);
	}

	__forceinline void writePixel16S(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		writePixel16S(pixelAddress16S(x, y, bp, bw), c);
	}

	__forceinline void writePixel8(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		writePixel8(pixelAddress8(x, y, bp, bw), c);
	}

	__forceinline void writePixel4(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		writePixel4(pixelAddress4(x, y, bp, bw), c);
	}

	__forceinline void writePixel8H(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		writePixel8H(pixelAddress32(x, y, bp, bw), c);
	}

    __forceinline void writePixel4HL(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		writePixel4HL(pixelAddress32(x, y, bp, bw), c);
	}

	__forceinline void writePixel4HH(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		writePixel4HH(pixelAddress32(x, y, bp, bw), c);
	}

	__forceinline void writePixel32Z(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		writePixel32(pixelAddress32Z(x, y, bp, bw), c);
	}

	__forceinline void writePixel24Z(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		writePixel24(pixelAddress32Z(x, y, bp, bw), c);
	}

	__forceinline void writePixel16Z(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		writePixel16(pixelAddress16Z(x, y, bp, bw), c);
	}

	__forceinline void writePixel16SZ(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		writePixel16S(pixelAddress16SZ(x, y, bp, bw), c);
	}

	__forceinline void writeFrame16(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		writeFrame16(pixelAddress16(x, y, bp, bw), c);
	}

	__forceinline void writeFrame16S(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		writeFrame16S(pixelAddress16S(x, y, bp, bw), c);
	}

	__forceinline void writeFrame16Z(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		writeFrame16(pixelAddress16Z(x, y, bp, bw), c);
	}

	__forceinline void writeFrame16SZ(int x, int y, DWORD c, DWORD bp, DWORD bw)
	{
		writeFrame16S(pixelAddress16SZ(x, y, bp, bw), c);
	}

	__forceinline DWORD readTexel32(DWORD addr, GIFRegTEXA& TEXA) 
	{
		return m_vm32[addr];
	}

	__forceinline DWORD readTexel24(DWORD addr, GIFRegTEXA& TEXA) 
	{
		return Expand24To32(m_vm32[addr], TEXA);
	}

	__forceinline DWORD readTexel16(DWORD addr, GIFRegTEXA& TEXA) 
	{
		return Expand16To32(m_vm16[addr], TEXA);
	}

	__forceinline DWORD readTexel16S(DWORD addr, GIFRegTEXA& TEXA) 
	{
		return Expand16To32(m_vm16[addr], TEXA);
	}

	__forceinline DWORD readTexel8(DWORD addr, GIFRegTEXA& TEXA) 
	{
		return m_pCLUT32[readPixel8(addr)];
	}

	__forceinline DWORD readTexel4(DWORD addr, GIFRegTEXA& TEXA) 
	{
		return m_pCLUT32[readPixel4(addr)];
	}

	__forceinline DWORD readTexel8H(DWORD addr, GIFRegTEXA& TEXA) 
	{
		return m_pCLUT32[readPixel8H(addr)];
	}

	__forceinline DWORD readTexel4HL(DWORD addr, GIFRegTEXA& TEXA)
	{
		return m_pCLUT32[readPixel4HL(addr)];
	}

	__forceinline DWORD readTexel4HH(DWORD addr, GIFRegTEXA& TEXA) 
	{
		return m_pCLUT32[readPixel4HH(addr)];
	}

	__forceinline DWORD readTexel32(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return readTexel32(pixelAddress32(x, y, TEX0.TBP0, TEX0.TBW), TEXA);
	}

	__forceinline DWORD readTexel24(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return readTexel24(pixelAddress32(x, y, TEX0.TBP0, TEX0.TBW), TEXA);
	}

	__forceinline DWORD readTexel16(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return readTexel16(pixelAddress16(x, y, TEX0.TBP0, TEX0.TBW), TEXA);
	}

	__forceinline DWORD readTexel16S(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return readTexel16S(pixelAddress16S(x, y, TEX0.TBP0, TEX0.TBW), TEXA);
	}

	__forceinline DWORD readTexel8(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return readTexel8(pixelAddress8(x, y, TEX0.TBP0, TEX0.TBW), TEXA);
	}

	__forceinline DWORD readTexel4(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return readTexel4(pixelAddress4(x, y, TEX0.TBP0, TEX0.TBW), TEXA);
	}

	__forceinline DWORD readTexel8H(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return readTexel8H(pixelAddress32(x, y, TEX0.TBP0, TEX0.TBW), TEXA);
	}

	__forceinline DWORD readTexel4HL(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return readTexel4HL(pixelAddress32(x, y, TEX0.TBP0, TEX0.TBW), TEXA);
	}

	__forceinline DWORD readTexel4HH(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return readTexel4HH(pixelAddress32(x, y, TEX0.TBP0, TEX0.TBW), TEXA);
	}

	__forceinline DWORD readTexel32Z(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return readTexel32(pixelAddress32Z(x, y, TEX0.TBP0, TEX0.TBW), TEXA);
	}

	__forceinline DWORD readTexel24Z(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return readTexel24(pixelAddress32Z(x, y, TEX0.TBP0, TEX0.TBW), TEXA);
	}

	__forceinline DWORD readTexel16Z(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return readTexel16(pixelAddress16Z(x, y, TEX0.TBP0, TEX0.TBW), TEXA);
	}

	__forceinline DWORD readTexel16SZ(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return readTexel16S(pixelAddress16SZ(x, y, TEX0.TBP0, TEX0.TBW), TEXA);
	}

	__forceinline DWORD readTexel16P(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return readPixel16(x, y, TEX0.TBP0, TEX0.TBW);
	}

	__forceinline DWORD readTexel16SP(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return readPixel16S(x, y, TEX0.TBP0, TEX0.TBW);
	}

	__forceinline DWORD readTexel8P(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return readPixel8(x, y, TEX0.TBP0, TEX0.TBW);
	}

	__forceinline DWORD readTexel8HP(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return readPixel8H(x, y, TEX0.TBP0, TEX0.TBW);
	}

	__forceinline DWORD readTexel4P(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return readPixel4(x, y, TEX0.TBP0, TEX0.TBW);
	}

	__forceinline DWORD readTexel4HLP(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return readPixel4HL(x, y, TEX0.TBP0, TEX0.TBW);
	}

	__forceinline DWORD readTexel4HHP(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return readPixel4HH(x, y, TEX0.TBP0, TEX0.TBW);
	}

	__forceinline DWORD readTexel16ZP(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return readPixel16Z(x, y, TEX0.TBP0, TEX0.TBW);
	}

	__forceinline DWORD readTexel16SZP(int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		return readPixel16SZ(x, y, TEX0.TBP0, TEX0.TBW);
	}

	//

	__forceinline DWORD readPixelX(int PSM, DWORD addr)
	{
		switch(PSM)
		{
		case PSM_PSMCT32: return readPixel32(addr); 
		case PSM_PSMCT24: return readPixel24(addr); 
		case PSM_PSMCT16: return readPixel16(addr);
		case PSM_PSMCT16S: return readPixel16S(addr);
		case PSM_PSMT8: return readPixel8(addr);
		case PSM_PSMT4: return readPixel4(addr);
		case PSM_PSMT8H: return readPixel8H(addr);
		case PSM_PSMT4HL: return readPixel4HL(addr);
		case PSM_PSMT4HH: return readPixel4HH(addr);
		case PSM_PSMZ32: return readPixel32(addr);
		case PSM_PSMZ24: return readPixel24(addr);
		case PSM_PSMZ16: return readPixel16(addr);
		case PSM_PSMZ16S: return readPixel16S(addr);
		default: ASSERT(0); return readPixel32(addr);
		}
	}

	__forceinline DWORD readTexelX(int PSM, DWORD addr, GIFRegTEXA& TEXA)
	{
		switch(PSM)
		{
		case PSM_PSMCT32: return readTexel32(addr, TEXA);
		case PSM_PSMCT24: return readTexel24(addr, TEXA);
		case PSM_PSMCT16: return readTexel16(addr, TEXA);
		case PSM_PSMCT16S: return readTexel16S(addr, TEXA);
		case PSM_PSMT8: return readTexel8(addr, TEXA);
		case PSM_PSMT4: return readTexel4(addr, TEXA);
		case PSM_PSMT8H: return readTexel8H(addr, TEXA);
		case PSM_PSMT4HL: return readTexel4HL(addr, TEXA);
		case PSM_PSMT4HH: return readTexel4HH(addr, TEXA);
		case PSM_PSMZ32: return readTexel32(addr, TEXA);
		case PSM_PSMZ24: return readTexel24(addr, TEXA);
		case PSM_PSMZ16: return readTexel16(addr, TEXA);
		case PSM_PSMZ16S: return readTexel16S(addr, TEXA);
		default: ASSERT(0); return readTexel32(addr, TEXA);
		}
	}

	__forceinline DWORD readTexelX(int PSM, int x, int y, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
	{
		switch(PSM)
		{
		case PSM_PSMCT32: return readTexel32(x, y, TEX0, TEXA);
		case PSM_PSMCT24: return readTexel24(x, y, TEX0, TEXA);
		case PSM_PSMCT16: return readTexel16(x, y, TEX0, TEXA);
		case PSM_PSMCT16S: return readTexel16S(x, y, TEX0, TEXA);
		case PSM_PSMT8: return readTexel8(x, y, TEX0, TEXA);
		case PSM_PSMT4: return readTexel4(x, y, TEX0, TEXA);
		case PSM_PSMT8H: return readTexel8H(x, y, TEX0, TEXA);
		case PSM_PSMT4HL: return readTexel4HL(x, y, TEX0, TEXA);
		case PSM_PSMT4HH: return readTexel4HH(x, y, TEX0, TEXA);
		case PSM_PSMZ32: return readTexel32Z(x, y, TEX0, TEXA);
		case PSM_PSMZ24: return readTexel24Z(x, y, TEX0, TEXA);
		case PSM_PSMZ16: return readTexel16Z(x, y, TEX0, TEXA);
		case PSM_PSMZ16S: return readTexel16SZ(x, y, TEX0, TEXA);
		default: ASSERT(0); return readTexel32(x, y, TEX0, TEXA);
		}
	}

	__forceinline void writePixelX(int PSM, DWORD addr, DWORD c)
	{
		switch(PSM)
		{
		case PSM_PSMCT32: writePixel32(addr, c); break; 
		case PSM_PSMCT24: writePixel24(addr, c); break; 
		case PSM_PSMCT16: writePixel16(addr, c); break;
		case PSM_PSMCT16S: writePixel16S(addr, c); break;
		case PSM_PSMT8: writePixel8(addr, c); break;
		case PSM_PSMT4: writePixel4(addr, c); break;
		case PSM_PSMT8H: writePixel8H(addr, c); break;
		case PSM_PSMT4HL: writePixel4HL(addr, c); break;
		case PSM_PSMT4HH: writePixel4HH(addr, c); break;
		case PSM_PSMZ32: writePixel32(addr, c); break;
		case PSM_PSMZ24: writePixel24(addr, c); break;
		case PSM_PSMZ16: writePixel16(addr, c); break;
		case PSM_PSMZ16S: writePixel16S(addr, c); break;
		default: ASSERT(0); writePixel32(addr, c); break;
		}
	}

	__forceinline void writeFrameX(int PSM, DWORD addr, DWORD c)
	{
		switch(PSM)
		{
		case PSM_PSMCT32: writePixel32(addr, c); break; 
		case PSM_PSMCT24: writePixel24(addr, c); break; 
		case PSM_PSMCT16: writeFrame16(addr, c); break;
		case PSM_PSMCT16S: writeFrame16S(addr, c); break;
		case PSM_PSMZ32: writePixel32(addr, c); break; 
		case PSM_PSMZ24: writePixel24(addr, c); break; 
		case PSM_PSMZ16: writeFrame16(addr, c); break;
		case PSM_PSMZ16S: writeFrame16S(addr, c); break;
		default: ASSERT(0); writePixel32(addr, c); break;
		}
	}

	// FillRect

	bool FillRect(const CRect& r, DWORD c, DWORD psm, DWORD fbp, DWORD fbw);

	// CLUT

	void InvalidateCLUT() {m_fCLUTMayBeDirty = true;}
	bool IsCLUTDirty(GIFRegTEX0 TEX0, GIFRegTEXCLUT TEXCLUT);
	bool IsCLUTUpdating(GIFRegTEX0 TEX0, GIFRegTEXCLUT TEXCLUT);
	bool WriteCLUT(GIFRegTEX0 TEX0, GIFRegTEXCLUT TEXCLUT);

	void ReadCLUT(GIFRegTEX0 TEX0, DWORD* pCLUT32);
	void SetupCLUT(GIFRegTEX0 TEX0);

	// expands 16->32

	void ReadCLUT32(GIFRegTEX0 TEX0, GIFRegTEXA TEXA, DWORD* pCLUT32);
	void SetupCLUT32(GIFRegTEX0 TEX0, GIFRegTEXA TEXA);
	void CopyCLUT32(DWORD* pCLUT32, int nPaletteEntries);

	// 

	void SwizzleTexture32(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);
	void SwizzleTexture24(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);
	void SwizzleTexture16(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);
	void SwizzleTexture16S(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);
	void SwizzleTexture8(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);
	void SwizzleTexture8H(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);
	void SwizzleTexture4(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);
	void SwizzleTexture4HL(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);
	void SwizzleTexture4HH(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);
	void SwizzleTexture32Z(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);
	void SwizzleTexture24Z(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);
	void SwizzleTexture16Z(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);
	void SwizzleTexture16SZ(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);
	void SwizzleTextureX(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG);

	void unSwizzleTexture32(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void unSwizzleTexture24(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void unSwizzleTexture16(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void unSwizzleTexture16S(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void unSwizzleTexture8(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void unSwizzleTexture8H(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void unSwizzleTexture4(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void unSwizzleTexture4HL(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void unSwizzleTexture4HH(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void unSwizzleTexture32Z(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void unSwizzleTexture24Z(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void unSwizzleTexture16Z(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void unSwizzleTexture16SZ(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);

	void ReadTexture(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA, GIFRegCLAMP& CLAMP);

	// 32/16/8P

	void unSwizzleTexture16P(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void unSwizzleTexture16SP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void unSwizzleTexture8P(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void unSwizzleTexture8HP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void unSwizzleTexture4P(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void unSwizzleTexture4HLP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void unSwizzleTexture4HHP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void unSwizzleTexture16ZP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void unSwizzleTexture16SZP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);

	void ReadTextureP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA, GIFRegCLAMP& CLAMP);

	// 32/16

	void unSwizzleTexture8NP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void unSwizzleTexture8HNP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void unSwizzleTexture4NP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void unSwizzleTexture4HLNP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);
	void unSwizzleTexture4HHNP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA);

	void ReadTextureNP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA, GIFRegCLAMP& CLAMP);
	void ReadTextureNP2(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA, GIFRegCLAMP& CLAMP);

	//

	static DWORD m_xtbl[1024], m_ytbl[1024]; 

	template<typename T> void ReadTexture(CRect r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA, GIFRegCLAMP& CLAMP, readTexel rt, unSwizzleTexture st);
	template<typename T> void ReadTexture2(CRect r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA, readTexel rt, unSwizzleTexture st);

	HRESULT SaveBMP(LPCTSTR fn, DWORD bp, DWORD bw, DWORD psm, int w, int h);
};

#pragma warning(default: 4244)