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
 *	Special Notes: 
 *
 *	Based on Page.c from GSSoft
 *	Copyright (C) 2002-2004 GSsoft Team
 *
 */
 
#include "StdAfx.h"
#include "GSLocalMemory.h"

#define ASSERT_BLOCK(r, w, h) \
	ASSERT((r).Width() >= w && (r).Height() >= h && !((r).left&(w-1)) && !((r).top&(h-1)) && !((r).right&(w-1)) && !((r).bottom&(h-1))); \

#define FOREACH_BLOCK_START(r, w, h, t) \
	for(int y = (r).top; y < (r).bottom; y += (h)) \
	{ 	ASSERT_BLOCK(r, w, h); \
		BYTE* ptr = dst + (y-(r).top)*dstpitch; \
		for(int x = (r).left; x < (r).right; x += (w)) \
		{ \

#define FOREACH_BLOCK_END }}

//

DWORD GSLocalMemory::pageOffset32[32][32][64];
DWORD GSLocalMemory::pageOffset32Z[32][32][64];
DWORD GSLocalMemory::pageOffset16[32][64][64];
DWORD GSLocalMemory::pageOffset16S[32][64][64];
DWORD GSLocalMemory::pageOffset16Z[32][64][64];
DWORD GSLocalMemory::pageOffset16SZ[32][64][64];
DWORD GSLocalMemory::pageOffset8[32][64][128];
DWORD GSLocalMemory::pageOffset4[32][128][128];

int GSLocalMemory::rowOffset32[2048];
int GSLocalMemory::rowOffset32Z[2048];
int GSLocalMemory::rowOffset16[2048];
int GSLocalMemory::rowOffset16S[2048];
int GSLocalMemory::rowOffset16Z[2048];
int GSLocalMemory::rowOffset16SZ[2048];
int GSLocalMemory::rowOffset8[2][2048];
int GSLocalMemory::rowOffset4[2][2048];

//

DWORD GSLocalMemory::m_xtbl[1024];
DWORD GSLocalMemory::m_ytbl[1024]; 

//

GSLocalMemory::psm_t GSLocalMemory::m_psm[64];

//

GSLocalMemory::GSLocalMemory()
	: m_fCLUTMayBeDirty(true)
{
	const int vmsize = 1024 * 1024 * 4;
	m_vm8 = (BYTE*)_aligned_malloc(vmsize * 2, 16);
	memset(m_vm8, 0, vmsize);

	m_pCLUT = (WORD*)_aligned_malloc(256 * 2 * sizeof(WORD) * 2, 16);
	m_pCLUT32 = (DWORD*)_aligned_malloc(256 * sizeof(DWORD), 16);
	m_pCLUT64 = (UINT64*)_aligned_malloc(256 * sizeof(UINT64), 16);

	for(int bp = 0; bp < 32; bp++)
	{
		for(int y = 0; y < 32; y++) for(int x = 0; x < 64; x++)
		{
			pageOffset32[bp][y][x] = pixelAddressOrg32(x, y, bp, 0);
			pageOffset32Z[bp][y][x] = pixelAddressOrg32Z(x, y, bp, 0);
		}

		for(int y = 0; y < 64; y++) for(int x = 0; x < 64; x++) 
		{
			pageOffset16[bp][y][x] = pixelAddressOrg16(x, y, bp, 0);
			pageOffset16S[bp][y][x] = pixelAddressOrg16S(x, y, bp, 0);
			pageOffset16Z[bp][y][x] = pixelAddressOrg16Z(x, y, bp, 0);
			pageOffset16SZ[bp][y][x] = pixelAddressOrg16SZ(x, y, bp, 0);
		}

		for(int y = 0; y < 64; y++) for(int x = 0; x < 128; x++)
		{
			pageOffset8[bp][y][x] = pixelAddressOrg8(x, y, bp, 0);
		}

		for(int y = 0; y < 128; y++) for(int x = 0; x < 128; x++)
		{
			pageOffset4[bp][y][x] = pixelAddressOrg4(x, y, bp, 0);
		}
	}

	for(int x = 0; x < countof(rowOffset32); x++)
	{
		rowOffset32[x] = (int)pixelAddress32(x, 0, 0, 32) - (int)pixelAddress32(0, 0, 0, 32);
	}

	for(int x = 0; x < countof(rowOffset32Z); x++)
	{
		rowOffset32Z[x] = (int)pixelAddress32Z(x, 0, 0, 32) - (int)pixelAddress32Z(0, 0, 0, 32);
	}

	for(int x = 0; x < countof(rowOffset16); x++)
	{
		rowOffset16[x] = (int)pixelAddress16(x, 0, 0, 32) - (int)pixelAddress16(0, 0, 0, 32);
	}

	for(int x = 0; x < countof(rowOffset16S); x++)
	{
		rowOffset16S[x] = (int)pixelAddress16S(x, 0, 0, 32) - (int)pixelAddress16S(0, 0, 0, 32);
	}

	for(int x = 0; x < countof(rowOffset16Z); x++)
	{
		rowOffset16Z[x] = (int)pixelAddress16Z(x, 0, 0, 32) - (int)pixelAddress16Z(0, 0, 0, 32);
	}

	for(int x = 0; x < countof(rowOffset16SZ); x++)
	{
		rowOffset16SZ[x] = (int)pixelAddress16SZ(x, 0, 0, 32) - (int)pixelAddress16SZ(0, 0, 0, 32);
	}

	for(int x = 0; x < countof(rowOffset8[0]); x++)
	{
		rowOffset8[0][x] = (int)pixelAddress8(x, 0, 0, 32) - (int)pixelAddress8(0, 0, 0, 32),
		rowOffset8[1][x] = (int)pixelAddress8(x, 2, 0, 32) - (int)pixelAddress8(0, 2, 0, 32);
	}

	for(int x = 0; x < countof(rowOffset4[0]); x++)
	{
		rowOffset4[0][x] = (int)pixelAddress4(x, 0, 0, 32) - (int)pixelAddress4(0, 0, 0, 32),
		rowOffset4[1][x] = (int)pixelAddress4(x, 2, 0, 32) - (int)pixelAddress4(0, 2, 0, 32);
	}

	for(int i = 0; i < countof(m_psm); i++)
	{
		m_psm[i].pa = &GSLocalMemory::pixelAddress32;
		m_psm[i].ba = &GSLocalMemory::blockAddress32;
		m_psm[i].pga = &GSLocalMemory::pageAddress32;
		m_psm[i].rp = &GSLocalMemory::readPixel32;
		m_psm[i].rpa = &GSLocalMemory::readPixel32;
		m_psm[i].wp = &GSLocalMemory::writePixel32;
		m_psm[i].wpa = &GSLocalMemory::writePixel32;
		m_psm[i].rt = &GSLocalMemory::readTexel32;
		m_psm[i].rtNP = &GSLocalMemory::readTexel32;
		m_psm[i].rta = &GSLocalMemory::readTexel32;
		m_psm[i].wfa = &GSLocalMemory::writePixel32;
		m_psm[i].st = &GSLocalMemory::SwizzleTexture32;
		m_psm[i].ust = &GSLocalMemory::unSwizzleTexture32;
		m_psm[i].ustNP = &GSLocalMemory::unSwizzleTexture32;
		m_psm[i].bpp = m_psm[i].trbpp = 32;
		m_psm[i].pal = 0;
		m_psm[i].bs = CSize(8, 8);
		m_psm[i].pgs = CSize(64, 32);
		for(int j = 0; j < 8; j++) m_psm[i].rowOffset[j] = rowOffset32;
	}

	m_psm[PSM_PSMCT16].pa = &GSLocalMemory::pixelAddress16;
	m_psm[PSM_PSMCT16S].pa = &GSLocalMemory::pixelAddress16S;
	m_psm[PSM_PSMT8].pa = &GSLocalMemory::pixelAddress8;
	m_psm[PSM_PSMT4].pa = &GSLocalMemory::pixelAddress4;
	m_psm[PSM_PSMZ32].pa = &GSLocalMemory::pixelAddress32Z;
	m_psm[PSM_PSMZ24].pa = &GSLocalMemory::pixelAddress32Z;
	m_psm[PSM_PSMZ16].pa = &GSLocalMemory::pixelAddress16Z;
	m_psm[PSM_PSMZ16S].pa = &GSLocalMemory::pixelAddress16SZ;

	m_psm[PSM_PSMCT16].ba = &GSLocalMemory::blockAddress16;
	m_psm[PSM_PSMCT16S].ba = &GSLocalMemory::blockAddress16S;
	m_psm[PSM_PSMT8].ba = &GSLocalMemory::blockAddress8;
	m_psm[PSM_PSMT4].ba = &GSLocalMemory::blockAddress4;
	m_psm[PSM_PSMZ32].ba = &GSLocalMemory::blockAddress32Z;
	m_psm[PSM_PSMZ24].ba = &GSLocalMemory::blockAddress32Z;
	m_psm[PSM_PSMZ16].ba = &GSLocalMemory::blockAddress16Z;
	m_psm[PSM_PSMZ16S].ba = &GSLocalMemory::blockAddress16SZ;

	m_psm[PSM_PSMCT16].pga = &GSLocalMemory::pageAddress16;
	m_psm[PSM_PSMCT16S].pga = &GSLocalMemory::pageAddress16;
	m_psm[PSM_PSMZ16].pga = &GSLocalMemory::pageAddress16;
	m_psm[PSM_PSMZ16S].pga = &GSLocalMemory::pageAddress16;
	m_psm[PSM_PSMT8].pga = &GSLocalMemory::pageAddress8;
	m_psm[PSM_PSMT4].pga = &GSLocalMemory::pageAddress4;

	m_psm[PSM_PSMCT24].rp = &GSLocalMemory::readPixel24;
	m_psm[PSM_PSMCT16].rp = &GSLocalMemory::readPixel16;
	m_psm[PSM_PSMCT16S].rp = &GSLocalMemory::readPixel16S;
	m_psm[PSM_PSMT8].rp = &GSLocalMemory::readPixel8;
	m_psm[PSM_PSMT4].rp = &GSLocalMemory::readPixel4;
	m_psm[PSM_PSMT8H].rp = &GSLocalMemory::readPixel8H;
	m_psm[PSM_PSMT4HL].rp = &GSLocalMemory::readPixel4HL;
	m_psm[PSM_PSMT4HH].rp = &GSLocalMemory::readPixel4HH;
	m_psm[PSM_PSMZ32].rp = &GSLocalMemory::readPixel32Z;
	m_psm[PSM_PSMZ24].rp = &GSLocalMemory::readPixel24Z;
	m_psm[PSM_PSMZ16].rp = &GSLocalMemory::readPixel16Z;
	m_psm[PSM_PSMZ16S].rp = &GSLocalMemory::readPixel16SZ;

	m_psm[PSM_PSMCT24].rpa = &GSLocalMemory::readPixel24;
	m_psm[PSM_PSMCT16].rpa = &GSLocalMemory::readPixel16;
	m_psm[PSM_PSMCT16S].rpa = &GSLocalMemory::readPixel16;
	m_psm[PSM_PSMT8].rpa = &GSLocalMemory::readPixel8;
	m_psm[PSM_PSMT4].rpa = &GSLocalMemory::readPixel4;
	m_psm[PSM_PSMT8H].rpa = &GSLocalMemory::readPixel8H;
	m_psm[PSM_PSMT4HL].rpa = &GSLocalMemory::readPixel4HL;
	m_psm[PSM_PSMT4HH].rpa = &GSLocalMemory::readPixel4HH;
	m_psm[PSM_PSMZ32].rpa = &GSLocalMemory::readPixel32;
	m_psm[PSM_PSMZ24].rpa = &GSLocalMemory::readPixel24;
	m_psm[PSM_PSMZ16].rpa = &GSLocalMemory::readPixel16;
	m_psm[PSM_PSMZ16S].rpa = &GSLocalMemory::readPixel16;

	m_psm[PSM_PSMCT32].wp = &GSLocalMemory::writePixel32;
	m_psm[PSM_PSMCT24].wp = &GSLocalMemory::writePixel24;
	m_psm[PSM_PSMCT16].wp = &GSLocalMemory::writePixel16;
	m_psm[PSM_PSMCT16S].wp = &GSLocalMemory::writePixel16S;
	m_psm[PSM_PSMT8].wp = &GSLocalMemory::writePixel8;
	m_psm[PSM_PSMT4].wp = &GSLocalMemory::writePixel4;
	m_psm[PSM_PSMT8H].wp = &GSLocalMemory::writePixel8H;
	m_psm[PSM_PSMT4HL].wp = &GSLocalMemory::writePixel4HL;
	m_psm[PSM_PSMT4HH].wp = &GSLocalMemory::writePixel4HH;
	m_psm[PSM_PSMZ32].wp = &GSLocalMemory::writePixel32Z;
	m_psm[PSM_PSMZ24].wp = &GSLocalMemory::writePixel24Z;
	m_psm[PSM_PSMZ16].wp = &GSLocalMemory::writePixel16Z;
	m_psm[PSM_PSMZ16S].wp = &GSLocalMemory::writePixel16SZ;

	m_psm[PSM_PSMCT32].wpa = &GSLocalMemory::writePixel32;
	m_psm[PSM_PSMCT24].wpa = &GSLocalMemory::writePixel24;
	m_psm[PSM_PSMCT16].wpa = &GSLocalMemory::writePixel16;
	m_psm[PSM_PSMCT16S].wpa = &GSLocalMemory::writePixel16;
	m_psm[PSM_PSMT8].wpa = &GSLocalMemory::writePixel8;
	m_psm[PSM_PSMT4].wpa = &GSLocalMemory::writePixel4;
	m_psm[PSM_PSMT8H].wpa = &GSLocalMemory::writePixel8H;
	m_psm[PSM_PSMT4HL].wpa = &GSLocalMemory::writePixel4HL;
	m_psm[PSM_PSMT4HH].wpa = &GSLocalMemory::writePixel4HH;
	m_psm[PSM_PSMZ32].wpa = &GSLocalMemory::writePixel32;
	m_psm[PSM_PSMZ24].wpa = &GSLocalMemory::writePixel24;
	m_psm[PSM_PSMZ16].wpa = &GSLocalMemory::writePixel16;
	m_psm[PSM_PSMZ16S].wpa = &GSLocalMemory::writePixel16;

	m_psm[PSM_PSMCT24].rt = &GSLocalMemory::readTexel24;
	m_psm[PSM_PSMCT16].rt = &GSLocalMemory::readTexel16;
	m_psm[PSM_PSMCT16S].rt = &GSLocalMemory::readTexel16S;
	m_psm[PSM_PSMT8].rt = &GSLocalMemory::readTexel8;
	m_psm[PSM_PSMT4].rt = &GSLocalMemory::readTexel4;
	m_psm[PSM_PSMT8H].rt = &GSLocalMemory::readTexel8H;
	m_psm[PSM_PSMT4HL].rt = &GSLocalMemory::readTexel4HL;
	m_psm[PSM_PSMT4HH].rt = &GSLocalMemory::readTexel4HH;
	m_psm[PSM_PSMZ32].rt = &GSLocalMemory::readTexel32Z;
	m_psm[PSM_PSMZ24].rt = &GSLocalMemory::readTexel24Z;
	m_psm[PSM_PSMZ16].rt = &GSLocalMemory::readTexel16Z;
	m_psm[PSM_PSMZ16S].rt = &GSLocalMemory::readTexel16SZ;

	m_psm[PSM_PSMCT24].rta = &GSLocalMemory::readTexel24;
	m_psm[PSM_PSMCT16].rta = &GSLocalMemory::readTexel16;
	m_psm[PSM_PSMCT16S].rta = &GSLocalMemory::readTexel16;
	m_psm[PSM_PSMT8].rta = &GSLocalMemory::readTexel8;
	m_psm[PSM_PSMT4].rta = &GSLocalMemory::readTexel4;
	m_psm[PSM_PSMT8H].rta = &GSLocalMemory::readTexel8H;
	m_psm[PSM_PSMT4HL].rta = &GSLocalMemory::readTexel4HL;
	m_psm[PSM_PSMT4HH].rta = &GSLocalMemory::readTexel4HH;
	m_psm[PSM_PSMZ24].rta = &GSLocalMemory::readTexel24;
	m_psm[PSM_PSMZ16].rta = &GSLocalMemory::readTexel16;
	m_psm[PSM_PSMZ16S].rta = &GSLocalMemory::readTexel16;

	m_psm[PSM_PSMCT24].wfa = &GSLocalMemory::writePixel24;
	m_psm[PSM_PSMCT16].wfa = &GSLocalMemory::writeFrame16;
	m_psm[PSM_PSMCT16S].wfa = &GSLocalMemory::writeFrame16;
	m_psm[PSM_PSMZ24].wfa = &GSLocalMemory::writePixel24;
	m_psm[PSM_PSMZ16].wfa = &GSLocalMemory::writeFrame16;
	m_psm[PSM_PSMZ16S].wfa = &GSLocalMemory::writeFrame16;

	m_psm[PSM_PSMCT16].rtNP = &GSLocalMemory::readTexel16NP;
	m_psm[PSM_PSMCT16S].rtNP = &GSLocalMemory::readTexel16SNP;
	m_psm[PSM_PSMT8].rtNP = &GSLocalMemory::readTexel8;
	m_psm[PSM_PSMT4].rtNP = &GSLocalMemory::readTexel4;
	m_psm[PSM_PSMT8H].rtNP = &GSLocalMemory::readTexel8H;
	m_psm[PSM_PSMT4HL].rtNP = &GSLocalMemory::readTexel4HL;
	m_psm[PSM_PSMT4HH].rtNP = &GSLocalMemory::readTexel4HH;
	m_psm[PSM_PSMZ32].rtNP = &GSLocalMemory::readTexel32Z;
	m_psm[PSM_PSMZ24].rtNP = &GSLocalMemory::readTexel24Z;
	m_psm[PSM_PSMZ16].rtNP = &GSLocalMemory::readTexel16ZNP;
	m_psm[PSM_PSMZ16S].rtNP = &GSLocalMemory::readTexel16SZNP;

	m_psm[PSM_PSMCT24].st = &GSLocalMemory::SwizzleTexture24;
	m_psm[PSM_PSMCT16].st = &GSLocalMemory::SwizzleTexture16;
	m_psm[PSM_PSMCT16S].st = &GSLocalMemory::SwizzleTexture16S;
	m_psm[PSM_PSMT8].st = &GSLocalMemory::SwizzleTexture8;
	m_psm[PSM_PSMT4].st = &GSLocalMemory::SwizzleTexture4;
	m_psm[PSM_PSMT8H].st = &GSLocalMemory::SwizzleTexture8H;
	m_psm[PSM_PSMT4HL].st = &GSLocalMemory::SwizzleTexture4HL;
	m_psm[PSM_PSMT4HH].st = &GSLocalMemory::SwizzleTexture4HH;
	m_psm[PSM_PSMZ32].st = &GSLocalMemory::SwizzleTexture32Z;
	m_psm[PSM_PSMZ24].st = &GSLocalMemory::SwizzleTexture24Z;
	m_psm[PSM_PSMZ16].st = &GSLocalMemory::SwizzleTexture16Z;
	m_psm[PSM_PSMZ16S].st = &GSLocalMemory::SwizzleTexture16SZ;

	m_psm[PSM_PSMCT24].ust = &GSLocalMemory::unSwizzleTexture24;
	m_psm[PSM_PSMCT16].ust = &GSLocalMemory::unSwizzleTexture16;
	m_psm[PSM_PSMCT16S].ust = &GSLocalMemory::unSwizzleTexture16S;
	m_psm[PSM_PSMT8].ust = &GSLocalMemory::unSwizzleTexture8;
	m_psm[PSM_PSMT4].ust = &GSLocalMemory::unSwizzleTexture4;
	m_psm[PSM_PSMT8H].ust = &GSLocalMemory::unSwizzleTexture8H;
	m_psm[PSM_PSMT4HL].ust = &GSLocalMemory::unSwizzleTexture4HL;
	m_psm[PSM_PSMT4HH].ust = &GSLocalMemory::unSwizzleTexture4HH;
	m_psm[PSM_PSMZ32].ust = &GSLocalMemory::unSwizzleTexture32Z;
	m_psm[PSM_PSMZ24].ust = &GSLocalMemory::unSwizzleTexture24Z;
	m_psm[PSM_PSMZ16].ust = &GSLocalMemory::unSwizzleTexture16Z;
	m_psm[PSM_PSMZ16S].ust = &GSLocalMemory::unSwizzleTexture16SZ;

	m_psm[PSM_PSMCT16].ustNP = &GSLocalMemory::unSwizzleTexture16NP;
	m_psm[PSM_PSMCT16S].ustNP = &GSLocalMemory::unSwizzleTexture16SNP;
	m_psm[PSM_PSMT8].ustNP = &GSLocalMemory::unSwizzleTexture8NP;
	m_psm[PSM_PSMT4].ustNP = &GSLocalMemory::unSwizzleTexture4NP;
	m_psm[PSM_PSMT8H].ustNP = &GSLocalMemory::unSwizzleTexture8HNP;
	m_psm[PSM_PSMT4HL].ustNP = &GSLocalMemory::unSwizzleTexture4HLNP;
	m_psm[PSM_PSMT4HH].ustNP = &GSLocalMemory::unSwizzleTexture4HHNP;
	m_psm[PSM_PSMZ32].ustNP = &GSLocalMemory::unSwizzleTexture32Z;
	m_psm[PSM_PSMZ24].ustNP = &GSLocalMemory::unSwizzleTexture24Z;
	m_psm[PSM_PSMZ16].ustNP = &GSLocalMemory::unSwizzleTexture16ZNP;
	m_psm[PSM_PSMZ16S].ustNP = &GSLocalMemory::unSwizzleTexture16SZNP;

	m_psm[PSM_PSMT8].pal = m_psm[PSM_PSMT8H].pal = 256;
	m_psm[PSM_PSMT4].pal = m_psm[PSM_PSMT4HL].pal = m_psm[PSM_PSMT4HH].pal = 16;

	m_psm[PSM_PSMCT16].bpp = m_psm[PSM_PSMCT16S].bpp = 16;
	m_psm[PSM_PSMT8].bpp = 8;
	m_psm[PSM_PSMT4].bpp = 4;
	m_psm[PSM_PSMZ16].bpp = m_psm[PSM_PSMZ16S].bpp = 16;

	m_psm[PSM_PSMCT24].trbpp = 24;
	m_psm[PSM_PSMCT16].trbpp = m_psm[PSM_PSMCT16S].trbpp = 16;
	m_psm[PSM_PSMT8].trbpp = m_psm[PSM_PSMT8H].trbpp = 8;
	m_psm[PSM_PSMT4].trbpp = m_psm[PSM_PSMT4HL].trbpp = m_psm[PSM_PSMT4HH].trbpp = 4;
	m_psm[PSM_PSMZ24].trbpp = 24;
	m_psm[PSM_PSMZ16].trbpp = m_psm[PSM_PSMZ16S].trbpp = 16;

	m_psm[PSM_PSMCT16].bs = m_psm[PSM_PSMCT16S].bs = CSize(16, 8);
	m_psm[PSM_PSMT8].bs = CSize(16, 16);
	m_psm[PSM_PSMT4].bs = CSize(32, 16);
	m_psm[PSM_PSMZ16].bs = m_psm[PSM_PSMZ16S].bs = CSize(16, 8);

	m_psm[PSM_PSMCT16].pgs = m_psm[PSM_PSMCT16S].pgs = CSize(64, 64);
	m_psm[PSM_PSMT8].pgs = CSize(128, 64);
	m_psm[PSM_PSMT4].pgs = CSize(128, 128);
	m_psm[PSM_PSMZ16].pgs = m_psm[PSM_PSMZ16S].pgs = CSize(64, 64);

	for(int i = 0; i < 8; i++) m_psm[PSM_PSMCT16].rowOffset[i] = rowOffset16;
	for(int i = 0; i < 8; i++) m_psm[PSM_PSMCT16S].rowOffset[i] = rowOffset16S;
	for(int i = 0; i < 8; i++) m_psm[PSM_PSMT8].rowOffset[i] = rowOffset8[((i+2)>>2)&1];
	for(int i = 0; i < 8; i++) m_psm[PSM_PSMT4].rowOffset[i] = rowOffset4[((i+2)>>2)&1];
	for(int i = 0; i < 8; i++) m_psm[PSM_PSMZ32].rowOffset[i] = rowOffset32Z;
	for(int i = 0; i < 8; i++) m_psm[PSM_PSMZ24].rowOffset[i] = rowOffset32Z;
	for(int i = 0; i < 8; i++) m_psm[PSM_PSMZ16].rowOffset[i] = rowOffset16Z;
	for(int i = 0; i < 8; i++) m_psm[PSM_PSMZ16S].rowOffset[i] = rowOffset16SZ;
}

GSLocalMemory::~GSLocalMemory()
{
	_aligned_free(m_vm8);
	_aligned_free(m_pCLUT);
	_aligned_free(m_pCLUT32);
	_aligned_free(m_pCLUT64);	
}

bool GSLocalMemory::FillRect(const CRect& r, DWORD c, DWORD psm, DWORD fbp, DWORD fbw)
{
	const psm_t& tbl = m_psm[psm];

	writePixel wp = tbl.wp;
	pixelAddress ba = tbl.ba;

	int w = tbl.bs.cx;
	int h = tbl.bs.cy;
	int bpp = tbl.bpp;

	int shift = 0;

	switch(bpp)
	{
	case 32: shift = 0; break;
	case 16: shift = 1; c = (c & 0xffff) * 0x00010001; break;
	case 8: shift = 2; c = (c & 0xff) * 0x01010101; break;
	case 4: shift = 3; c = (c & 0xf) * 0x11111111; break;
	}

	CRect clip;
	
	clip.left = (r.left + (w-1)) & ~(w-1);
	clip.top = (r.top + (h-1)) & ~(h-1);
	clip.right = r.right & ~(w-1);
	clip.bottom = r.bottom & ~(h-1);

	for(int y = r.top; y < clip.top; y++)
	{
		for(int x = r.left; x < r.right; x++)
		{
			(this->*wp)(x, y, c, fbp, fbw);
		}
	}

	if(r.left < clip.left || clip.right < r.right)
	{
		for(int y = clip.top; y < clip.bottom; y += h)
		{
			for(int ys = y, ye = y + h; ys < ye; ys++)
			{
				for(int x = r.left; x < clip.left; x++)
				{
					(this->*wp)(x, ys, c, fbp, fbw);
				}

				for(int x = clip.right; x < r.right; x++)
				{
					(this->*wp)(x, ys, c, fbp, fbw);
				}
			}
		}
	}

	if(psm == PSM_PSMCT24 || psm == PSM_PSMZ24)
	{
		c &= 0x00ffffff;

		for(int y = clip.top; y < clip.bottom; y += h)
		{
			for(int x = clip.left; x < clip.right; x += w)
			{
				DWORD* p = &m_vm32[ba(x, y, fbp, fbw)];

				for(int i = 0; i < 64; i++)
				{
					p[i] = (p[i] & 0xff000000) | c;
				}
			}
		}
	}
	else
	{
		#if _M_SSE >= 0x200

		__m128i c128 = _mm_set1_epi32(c);

		for(int y = clip.top; y < clip.bottom; y += h)
		{
			for(int x = clip.left; x < clip.right; x += w)
			{
				__m128i* p = (__m128i*)&m_vm8[ba(x, y, fbp, fbw) << 2 >> shift];

				for(int i = 0; i < 16; i += 4)
				{
					p[i + 0] = c128;
					p[i + 1] = c128;
					p[i + 2] = c128;
					p[i + 3] = c128;
				}
			}
		}

		#else

		for(int y = clip.top; y < clip.bottom; y += h)
		{
			for(int x = clip.left; x < clip.right; x += w)
			{
				DWORD* p = (DWORD*)&m_vm8[ba(x, y, fbp, fbw) << 2 >> shift];

				for(int i = 0; i < 64; i += 4)
				{
					p[i + 0] = c;
					p[i + 1] = c;
					p[i + 2] = c;
					p[i + 3] = c;
				}
			}
		}

		#endif
	}

	for(int y = clip.bottom; y < r.bottom; y++)
	{
		for(int x = r.left; x < r.right; x++)
		{
			(this->*wp)(x, y, c, fbp, fbw);
		}
	}

	return true;
}

////////////////////

bool GSLocalMemory::IsCLUTDirty(GIFRegTEX0 TEX0, GIFRegTEXCLUT TEXCLUT)
{
	return m_fCLUTMayBeDirty || m_prevTEX0.i64 != TEX0.i64 || m_prevTEXCLUT.i64 != TEXCLUT.i64;
}

bool GSLocalMemory::IsCLUTUpdating(GIFRegTEX0 TEX0, GIFRegTEXCLUT TEXCLUT)
{
	switch(TEX0.CLD)
	{
	case 0: 
		return false;
	case 1:
	case 2:
	case 3:
		break;
	case 4:
		if(m_CBP[0] == TEX0.CBP) return false;
	case 5:
		if(m_CBP[1] == TEX0.CBP) return false;
	}

	return IsCLUTDirty(TEX0, TEXCLUT);
}

bool GSLocalMemory::WriteCLUT(GIFRegTEX0 TEX0, GIFRegTEXCLUT TEXCLUT)
{
	switch(TEX0.CLD)
	{
	default:
	case 0: return false;
	case 1: break;
	case 2: m_CBP[0] = TEX0.CBP; break;
	case 3: m_CBP[1] = TEX0.CBP; break;
	case 4: if(m_CBP[0] == TEX0.CBP) return false;
	case 5: if(m_CBP[1] == TEX0.CBP) return false;
	}

	if(!IsCLUTDirty(TEX0, TEXCLUT))
	{
		return false;
	}

	m_prevTEX0 = TEX0;
	m_prevTEXCLUT = TEXCLUT;

	m_fCLUTMayBeDirty = false;

	DWORD bp = TEX0.CBP;
	DWORD bw = TEX0.CSM == 0 ? 1 : TEXCLUT.CBW;

	WORD* pCLUT = m_pCLUT + (TEX0.CSA << 4);

	// NOTE: TEX0.CPSM == PSM_PSMCT24 is non-standard, KH uses it

	if(TEX0.CSM == 0)
	{
		if(TEX0.CPSM == PSM_PSMCT16 || TEX0.CPSM == PSM_PSMCT16S)
		{
			WORD* vm = &m_vm16[TEX0.CPSM == PSM_PSMCT16 ? blockAddress16(0, 0, bp, bw) : blockAddress16S(0, 0, bp, bw)];

			if(TEX0.PSM == PSM_PSMT8 || TEX0.PSM == PSM_PSMT8H)
			{
				WriteCLUT_T16_I8_CSM1(vm, pCLUT);
			}
			else if(TEX0.PSM == PSM_PSMT4HH || TEX0.PSM == PSM_PSMT4HL || TEX0.PSM == PSM_PSMT4)
			{
				WriteCLUT_T16_I4_CSM1(vm, pCLUT);
			}
		}
		else if(TEX0.CPSM == PSM_PSMCT32 || TEX0.CPSM == PSM_PSMCT24)
		{
			DWORD* vm = &m_vm32[blockAddress32(0, 0, bp, bw)];

			if(TEX0.PSM == PSM_PSMT8 || TEX0.PSM == PSM_PSMT8H)
			{
				WriteCLUT_T32_I8_CSM1(vm, pCLUT);
			}
			else if(TEX0.PSM == PSM_PSMT4HH || TEX0.PSM == PSM_PSMT4HL || TEX0.PSM == PSM_PSMT4)
			{
				WriteCLUT_T32_I4_CSM1(vm, pCLUT);
			}
		}
	}
	else
	{
		// ASSERT(TEX0.CSA == 0);

		readPixel rp = m_psm[TEX0.CPSM].rp;

		int pal = m_psm[TEX0.PSM].pal;

		// ASSERT(pal == 0 || TEX0.CPSM == PSM_PSMCT16); // this is the only allowed format for CSM2, but we implement all of them, just in case...

		if(TEX0.CPSM == PSM_PSMCT16 || TEX0.CPSM == PSM_PSMCT16S)
		{
			for(int i = 0; i < pal; i++)
			{
				pCLUT[i] = (WORD)(this->*rp)((TEXCLUT.COU << 4) + i, TEXCLUT.COV, bp, bw);
			}
		}
		else if(TEX0.CPSM == PSM_PSMCT32 || TEX0.CPSM == PSM_PSMCT24)
		{
			for(int i = 0; i < pal; i++)
			{
				DWORD dw = (this->*rp)((TEXCLUT.COU << 4) + i, TEXCLUT.COV, bp, bw);

				pCLUT[i] = (WORD)(dw & 0xffff);
				pCLUT[i + 256] = (WORD)(dw >> 16);
			}
		}
	}

	return true;
}

//

void GSLocalMemory::ReadCLUT(GIFRegTEX0 TEX0, DWORD* pCLUT32)
{
	ASSERT(pCLUT32);

	WORD* pCLUT = m_pCLUT + (TEX0.CSA << 4);

	if(TEX0.CPSM == PSM_PSMCT32)
	{
		switch(TEX0.PSM)
		{
		case PSM_PSMT8:
		case PSM_PSMT8H:
			ReadCLUT32_T32_I8(pCLUT, pCLUT32);
			break;
		case PSM_PSMT4:
		case PSM_PSMT4HL:
		case PSM_PSMT4HH:
			ReadCLUT32_T32_I4(pCLUT, pCLUT32);
			break;
		}
	}
	else if(TEX0.CPSM == PSM_PSMCT16 || TEX0.CPSM == PSM_PSMCT16S)
	{
		switch(TEX0.PSM)
		{
		case PSM_PSMT8:
		case PSM_PSMT8H:
			ReadCLUT32_T16_I8(pCLUT, pCLUT32);
			break;
		case PSM_PSMT4:
		case PSM_PSMT4HL:
		case PSM_PSMT4HH:
			ReadCLUT32_T16_I4(pCLUT, pCLUT32);
			break;
		}
	}
}

void GSLocalMemory::SetupCLUT(GIFRegTEX0 TEX0)
{
	// TODO: cache m_pCLUT*

	ReadCLUT(TEX0, m_pCLUT32);

	switch(TEX0.PSM)
	{
	case PSM_PSMT4:
	case PSM_PSMT4HL:
	case PSM_PSMT4HH:
		// sse2?
		if(TEX0.CPSM == PSM_PSMCT32 || TEX0.CPSM == PSM_PSMCT24)
		{
			for(int j = 0, k = 0; j < 16; j++)
			{
				UINT64 hi = (UINT64)m_pCLUT32[j] << 32;

				for(int i = 0; i < 16; i++, k++)
				{
					m_pCLUT64[k] = hi | m_pCLUT32[i];
				}
			}
		}
		else
		{
			for(int j = 0, k = 0; j < 16; j++)
			{
				DWORD hi = m_pCLUT32[j] << 16;

				for(int i = 0; i < 16; i++, k++)
				{
					((DWORD*)&m_pCLUT64[k])[0] = hi | (m_pCLUT32[i] & 0xffff);
				}
			}
		}

		break;
	}
}

//

void GSLocalMemory::ReadCLUT32(GIFRegTEX0 TEX0, GIFRegTEXA TEXA, DWORD* pCLUT32)
{
	ASSERT(pCLUT32);

	WORD* pCLUT = m_pCLUT + (TEX0.CSA << 4);

	if(TEX0.CPSM == PSM_PSMCT32)
	{
		switch(TEX0.PSM)
		{
		case PSM_PSMT8:
		case PSM_PSMT8H:
			ReadCLUT32_T32_I8(pCLUT, pCLUT32);
			break;
		case PSM_PSMT4:
		case PSM_PSMT4HL:
		case PSM_PSMT4HH:
			ReadCLUT32_T32_I4(pCLUT, pCLUT32);
			break;
		}
	}
	else if(TEX0.CPSM == PSM_PSMCT16 || TEX0.CPSM == PSM_PSMCT16S)
	{
		Expand16(pCLUT, pCLUT32, m_psm[TEX0.PSM].pal, &TEXA);
	}
}

void GSLocalMemory::SetupCLUT32(GIFRegTEX0 TEX0, GIFRegTEXA TEXA)
{
	// TODO: cache m_pCLUT*

	ReadCLUT32(TEX0, TEXA, m_pCLUT32);

	switch(TEX0.PSM)
	{
	case PSM_PSMT4:
	case PSM_PSMT4HL:
	case PSM_PSMT4HH:
		// sse2?
		for(int j = 0, k = 0; j < 16; j++)
		{
			UINT64 hi = ((UINT64)m_pCLUT32[j] << 32);

			for(int i = 0; i < 16; i++, k++)
			{
				m_pCLUT64[k] = hi | m_pCLUT32[i];
			}
		}

		break;
	}
}

void GSLocalMemory::CopyCLUT32(DWORD* pCLUT32, int n)
{
	memcpy(pCLUT32, m_pCLUT32, sizeof(DWORD) * n);
}

////////////////////

static void SwizzleTextureStep(int& tx, int& ty, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG)
{
//	if(ty == TRXREG.RRH && tx == TRXPOS.DSAX) ASSERT(0);

	if(++tx == (int)TRXREG.RRW)
	{
		tx = TRXPOS.DSAX;
		ty++;
	}
}

#define IsTopLeftAligned(dsax, tx, ty, bw, bh) \
	((((int)dsax) & ((bw)-1)) == 0 && ((tx) & ((bw)-1)) == 0 && ((int)dsax) == (tx) && ((ty) & ((bh)-1)) == 0)

void GSLocalMemory::SwizzleTexture32(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG)
{
	if(TRXREG.RRW == 0) return;

	int tw = TRXREG.RRW, srcpitch = (TRXREG.RRW - TRXPOS.DSAX)*4;
	int th = len / srcpitch;

	bool aligned = IsTopLeftAligned(TRXPOS.DSAX, tx, ty, 8, 8);

	if(!aligned || (tw & 7) || (th & 7) || (len % srcpitch))
	{
		if(aligned && tw >= 8 && th >= 8)
		{
			int twa = tw & ~7;
			int tha = th & ~7;

			len -= tha * srcpitch;
			th -= tha;

			for(int j = 0; j < tha; j += 8)
			{
				for(int x = tx; x < twa; x += 8)
				{
					SwizzleBlock32u((BYTE*)&m_vm32[blockAddress32(x, ty, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 4, srcpitch);
				}

				for(int i = 0; i < 8; i++, ty++, src += srcpitch)
				{
					for(int x = twa; x < tw; x++)
					{
						writePixel32(x, ty, ((DWORD*)src)[x - tx], BITBLTBUF.DBP, BITBLTBUF.DBW);
					}
				}
			}
		}

		if(len > 0 && tw >= 8 && th >= 2 && IsTopLeftAligned(TRXPOS.DSAX, tx, ty, 8, 2))
		{
			int twa = tw & ~7;
			int tha = th & ~1;

			len -= tha * srcpitch;
			th -= tha;

			for(int j = 0; j < tha; j += 2)
			{
				for(int x = tx; x < twa; x += 8)
				{
					SwizzleColumn32(ty, (BYTE*)&m_vm32[blockAddress32(x, ty & ~7, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 4, srcpitch);
				}

				for(int i = 0; i < 2; i++, ty++, src += srcpitch)
				{
					for(int x = twa; x < tw; x++)
					{
						writePixel32(x, ty, ((DWORD*)src)[x - tx], BITBLTBUF.DBP, BITBLTBUF.DBW);
					}
				}
			}
		}

		SwizzleTextureX(tx, ty, src, len, BITBLTBUF, TRXPOS, TRXREG);
	}
	else
	{
		th += ty;

		if((DWORD_PTR)src & 0xf)
		{
			for(int y = ty; y < th; y += 8, src += srcpitch * 8)
			{
				for(int x = tx; x < tw; x += 8)
				{
					SwizzleBlock32u((BYTE*)&m_vm32[blockAddress32(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 4, srcpitch);
				}
			}
		}
		else
		{
			for(int y = ty; y < th; y += 8, src += srcpitch * 8)
			{
				for(int x = tx; x < tw; x += 8)
				{
					SwizzleBlock32((BYTE*)&m_vm32[blockAddress32(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 4, srcpitch);
				}
			}
		}

		ty = th;
	}
}

void GSLocalMemory::SwizzleTexture24(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG)
{
	if(TRXREG.RRW == 0) return;

	int tw = TRXREG.RRW, srcpitch = (TRXREG.RRW - TRXPOS.DSAX) * 3;
	int th = len / srcpitch;

	bool aligned = IsTopLeftAligned(TRXPOS.DSAX, tx, ty, 8, 8);

	if(!aligned || (tw & 7) || (th & 7) || (len % srcpitch))
	{
		// TODO

		SwizzleTextureX(tx, ty, src, len, BITBLTBUF, TRXPOS, TRXREG);
	}
	else
	{
		__declspec(align(16)) DWORD block[8 * 8];

		th += ty;

		for(int y = ty; y < th; y += 8, src += srcpitch * 8)
		{
			for(int x = tx; x < tw; x += 8)
			{
				ExpandBlock24(src + (x - tx) * 3, srcpitch, block);

				SwizzleBlock32((BYTE*)&m_vm32[blockAddress32(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], (BYTE*)block, sizeof(block) / 8, 0x00ffffff);
			}
		}

		ty = th;
	}
}

void GSLocalMemory::SwizzleTexture16(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG)
{
	if(TRXREG.RRW == 0) return;

	int tw = TRXREG.RRW, srcpitch = (TRXREG.RRW - TRXPOS.DSAX) * 2;
	int th = len / srcpitch;

	bool aligned = IsTopLeftAligned(TRXPOS.DSAX, tx, ty, 16, 8);

	if(!aligned || (tw & 15) || (th & 7) || (len % srcpitch))
	{
		if(aligned && tw >= 16 && th >= 8)
		{
			int twa = tw & ~15;
			int tha = th & ~7;

			len -= tha * srcpitch;
			th -= tha;

			for(int j = 0; j < tha; j += 8)
			{
				for(int x = tx; x < twa; x += 16)
				{
					SwizzleBlock16u((BYTE*)&m_vm16[blockAddress16(x, ty, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}

				for(int i = 0; i < 8; i++, ty++, src += srcpitch)
				{
					for(int x = twa; x < tw; x++)
					{
						writePixel16(x, ty, ((WORD*)src)[x - tx], BITBLTBUF.DBP, BITBLTBUF.DBW);
					}
				}
			}
		}

		if(len > 0 && tw >= 16 && th >= 2 && IsTopLeftAligned(TRXPOS.DSAX, tx, ty, 16, 2))
		{
			int twa = tw & ~15;
			int tha = th & ~1;

			len -= tha * srcpitch;
			th -= tha;

			for(int j = 0; j < tha; j += 2)
			{
				for(int x = tx; x < twa; x += 16)
				{
					SwizzleColumn16(ty, (BYTE*)&m_vm16[blockAddress16(x, ty & ~7, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}

				for(int i = 0; i < 2; i++, ty++, src += srcpitch)
				{
					for(int x = twa; x < tw; x++)
					{
						writePixel16(x, ty, ((WORD*)src)[x - tx], BITBLTBUF.DBP, BITBLTBUF.DBW);
					}
				}
			}
		}

		SwizzleTextureX(tx, ty, src, len, BITBLTBUF, TRXPOS, TRXREG);
	}
	else
	{
		th += ty;

		if((DWORD_PTR)src & 0xf)
		{
			for(int y = ty; y < th; y += 8, src += srcpitch * 8)
			{
				for(int x = tx; x < tw; x += 16)
				{
					SwizzleBlock16u((BYTE*)&m_vm16[blockAddress16(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}
			}
		}
		else
		{
			for(int y = ty; y < th; y += 8, src += srcpitch * 8)
			{
				for(int x = tx; x < tw; x += 16)
				{
					SwizzleBlock16((BYTE*)&m_vm16[blockAddress16(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}
			}
		}

		ty = th;
	}
}

void GSLocalMemory::SwizzleTexture16S(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG)
{
	if(TRXREG.RRW == 0) return;

	int tw = TRXREG.RRW, srcpitch = (TRXREG.RRW - TRXPOS.DSAX) * 2;
	int th = len / srcpitch;

	bool aligned = IsTopLeftAligned(TRXPOS.DSAX, tx, ty, 16, 8);

	if(!aligned || (tw & 15) || (th & 7) || (len % srcpitch))
	{
		if(aligned && tw >= 16 && th >= 8)
		{
			int twa = tw & ~15;
			int tha = th & ~7;

			len -= tha * srcpitch;
			th -= tha;

			for(int j = 0; j < tha; j += 8)
			{
				for(int x = tx; x < twa; x += 16)
				{
					SwizzleBlock16u((BYTE*)&m_vm16[blockAddress16S(x, ty, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}

				for(int i = 0; i < 8; i++, ty++, src += srcpitch)
				{
					for(int x = twa; x < tw; x++)
					{
						writePixel16S(x, ty, ((WORD*)src)[x - tx], BITBLTBUF.DBP, BITBLTBUF.DBW);
					}
				}
			}
		}

		if(len > 0 && tw >= 16 && th >= 2 && IsTopLeftAligned(TRXPOS.DSAX, tx, ty, 16, 2))
		{
			int twa = tw & ~15;
			int tha = th & ~1;

			len -= tha * srcpitch;
			th -= tha;

			for(int j = 0; j < tha; j += 2)
			{
				for(int x = tx; x < twa; x += 16)
				{
					SwizzleColumn16(ty, (BYTE*)&m_vm16[blockAddress16S(x, ty & ~7, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}

				for(int i = 0; i < 2; i++, ty++, src += srcpitch)
				{
					for(int x = twa; x < tw; x++)
					{
						writePixel16S(x, ty, ((WORD*)src)[x - tx], BITBLTBUF.DBP, BITBLTBUF.DBW);
					}
				}
			}
		}

		SwizzleTextureX(tx, ty, src, len, BITBLTBUF, TRXPOS, TRXREG);
	}
	else
	{
		th += ty;

		if((DWORD_PTR)src & 0xf)
		{
			for(int y = ty; y < th; y += 8, src += srcpitch * 8)
			{
				for(int x = tx; x < tw; x += 16)
				{
					SwizzleBlock16((BYTE*)&m_vm16[blockAddress16S(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}
			}
		}
		else
		{
			for(int y = ty; y < th; y += 8, src += srcpitch * 8)
			{
				for(int x = tx; x < tw; x += 16)
				{
					SwizzleBlock16((BYTE*)&m_vm16[blockAddress16S(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}
			}
		}

		ty = th;
	}
}

void GSLocalMemory::SwizzleTexture8(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG)
{
	if(TRXREG.RRW == 0) return;

	int tw = TRXREG.RRW, srcpitch = TRXREG.RRW - TRXPOS.DSAX;
	int th = len / srcpitch;

	bool aligned = IsTopLeftAligned(TRXPOS.DSAX, tx, ty, 16, 16);

	if(!aligned || (tw & 15) || (th & 15) || (len % srcpitch))
	{
		if(aligned && tw >= 16 && th >= 16)
		{
			int twa = tw & ~15;
			int tha = th & ~15;

			len -= tha * srcpitch;
			th -= tha;

			for(int j = 0; j < tha; j += 16)
			{
				for(int x = tx; x < twa; x += 16)
				{
					SwizzleBlock8u((BYTE*)&m_vm8[blockAddress8(x, ty, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx), srcpitch);
				}

				for(int i = 0; i < 16; i++, ty++, src += srcpitch)
				{
					for(int x = twa; x < tw; x++)
					{
						writePixel8(x, ty, src[x - tx], BITBLTBUF.DBP, BITBLTBUF.DBW);
					}
				}
			}
		}

		if(len > 0 && tw >= 16 && th >= 4 && IsTopLeftAligned(TRXPOS.DSAX, tx, ty, 16, 4))
		{
			int twa = tw & ~15;
			int tha = th & ~3;

			len -= tha * srcpitch;
			th -= tha;

			for(int j = 0; j < tha; j += 4)
			{
				for(int x = tx; x < twa; x += 16)
				{
					SwizzleColumn8(ty, (BYTE*)&m_vm8[blockAddress8(x, ty & ~15, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx), srcpitch);
				}

				for(int i = 0; i < 4; i++, ty++, src += srcpitch)
				{
					for(int x = twa; x < tw; x++)
					{
						writePixel8(x, ty, src[x - tx], BITBLTBUF.DBP, BITBLTBUF.DBW);
					}
				}
			}
		}

		SwizzleTextureX(tx, ty, src, len, BITBLTBUF, TRXPOS, TRXREG);
	}
	else
	{
		th += ty;

		if((DWORD_PTR)src & 0xf)
		{
			for(int y = ty; y < th; y += 16, src += srcpitch * 16)
			{
				for(int x = tx; x < tw; x += 16)
				{
					SwizzleBlock8u((BYTE*)&m_vm8[blockAddress8(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx), srcpitch);
				}
			}
		}
		else
		{
			for(int y = ty; y < th; y += 16, src += srcpitch * 16)
			{
				for(int x = tx; x < tw; x += 16)
				{
					SwizzleBlock8((BYTE*)&m_vm8[blockAddress8(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx), srcpitch);
				}
			}
		}

		ty = th;
	}
}

void GSLocalMemory::SwizzleTexture8H(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG)
{
	if(TRXREG.RRW == 0) return;

	int tw = TRXREG.RRW, srcpitch = TRXREG.RRW - TRXPOS.DSAX;
	int th = len / srcpitch;

	bool aligned = IsTopLeftAligned(TRXPOS.DSAX, tx, ty, 8, 8);

	if(!aligned || (tw & 7) || (th & 7) || (len % srcpitch))
	{
		// TODO

		SwizzleTextureX(tx, ty, src, len, BITBLTBUF, TRXPOS, TRXREG);
	}
	else
	{
		__declspec(align(16)) DWORD block[8 * 8];

		th += ty;

		for(int y = ty; y < th; y += 8, src += srcpitch * 8)
		{
			for(int x = tx; x < tw; x += 8)
			{
				ExpandBlock8H(src + (x - tx), srcpitch, block);

				SwizzleBlock32((BYTE*)&m_vm32[blockAddress32(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], (BYTE*)block, sizeof(block) / 8, 0xff000000);
			}
		}

		ty = th;
	}
}

void GSLocalMemory::SwizzleTexture4(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG)
{
	if(TRXREG.RRW == 0) return;

	int tw = TRXREG.RRW, srcpitch = (TRXREG.RRW - TRXPOS.DSAX) / 2;
	int th = len / srcpitch;

	bool aligned = IsTopLeftAligned(TRXPOS.DSAX, tx, ty, 32, 16);

	if(!aligned || (tw & 31) || (th & 15) || (len % srcpitch))
	{
		if(aligned && tw >= 32 && th >= 16)
		{
			int twa = tw & ~31;
			int tha = th & ~15;

			len -= tha * srcpitch;
			th -= tha;

			for(int j = 0; j < tha; j += 16)
			{
				for(int x = tx; x < twa; x += 32)
				{
					SwizzleBlock4u((BYTE*)&m_vm8[blockAddress4(x, ty, BITBLTBUF.DBP, BITBLTBUF.DBW) >> 1], src + (x - tx) / 2, srcpitch);
				}

				for(int i = 0; i < 16; i++, ty++, src += srcpitch)
				{
					BYTE* s = src + (twa - tx) / 2;

					for(int x = twa; x < tw; x += 2, s++)
					{
						writePixel4(x, ty, *s & 0xf, BITBLTBUF.DBP, BITBLTBUF.DBW),
						writePixel4(x + 1, ty, *s >> 4, BITBLTBUF.DBP, BITBLTBUF.DBW);
					}
				}
			}
		}

		if(len > 0 && tw >= 32 && th >= 4 && IsTopLeftAligned(TRXPOS.DSAX, tx, ty, 32, 4))
		{
			int twa = tw & ~31;
			int tha = th & ~3;

			len -= tha * srcpitch;
			th -= tha;

			for(int j = 0; j < tha; j += 4)
			{
				for(int x = tx; x < twa; x += 32)
				{
					SwizzleColumn4(ty, (BYTE*)&m_vm8[blockAddress4(x, ty & ~15, BITBLTBUF.DBP, BITBLTBUF.DBW) >> 1], src + (x - tx) / 2, srcpitch);
				}

				for(int i = 0; i < 4; i++, ty++, src += srcpitch)
				{
					BYTE* s = src + (twa - tx) / 2;

					for(int x = twa; x < tw; x += 2, s++)
					{
						writePixel4(x, ty, *s & 0xf, BITBLTBUF.DBP, BITBLTBUF.DBW),
						writePixel4(x + 1, ty, *s >> 4, BITBLTBUF.DBP, BITBLTBUF.DBW);
					}
				}
			}
		}

		SwizzleTextureX(tx, ty, src, len, BITBLTBUF, TRXPOS, TRXREG);
	}
	else
	{
		th += ty;

		if((DWORD_PTR)src & 0xf)
		{
			for(int y = ty; y < th; y += 16, src += srcpitch * 16)
			{
				for(int x = tx; x < tw; x += 32)
				{
					SwizzleBlock4u((BYTE*)&m_vm8[blockAddress4(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW) >> 1], src + (x - tx) / 2, srcpitch);
				}
			}
		}
		else
		{
			for(int y = ty; y < th; y += 16, src += srcpitch * 16)
			{
				for(int x = tx; x < tw; x += 32)
				{
					SwizzleBlock4((BYTE*)&m_vm8[blockAddress4(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW) >> 1], src + (x - tx) / 2, srcpitch);
				}
			}
		}

		ty = th;
	}
}

void GSLocalMemory::SwizzleTexture4HL(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG)
{
	if(TRXREG.RRW == 0) return;

	int tw = TRXREG.RRW, srcpitch = (TRXREG.RRW - TRXPOS.DSAX) / 2;
	int th = len / srcpitch;

	bool aligned = IsTopLeftAligned(TRXPOS.DSAX, tx, ty, 8, 8);

	if(!aligned || (tw & 7) || (th & 7) || (len % srcpitch))
	{
		// TODO

		SwizzleTextureX(tx, ty, src, len, BITBLTBUF, TRXPOS, TRXREG);
	}
	else
	{
		__declspec(align(16)) DWORD block[8 * 8];

		th += ty;

		for(int y = ty; y < th; y += 8, src += srcpitch * 8)
		{
			for(int x = tx; x < tw; x += 8)
			{
				ExpandBlock4HL(src + (x - tx) / 2, srcpitch, block);

				SwizzleBlock32((BYTE*)&m_vm32[blockAddress32(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], (BYTE*)block, sizeof(block) / 8, 0x0f000000);
			}
		}

		ty = th;
	}
}

void GSLocalMemory::SwizzleTexture4HH(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG)
{
	if(TRXREG.RRW == 0) return;

	int tw = TRXREG.RRW, srcpitch = (TRXREG.RRW - TRXPOS.DSAX) / 2;
	int th = len / srcpitch;

	bool aligned = IsTopLeftAligned(TRXPOS.DSAX, tx, ty, 8, 8);

	if(!aligned || (tw & 7) || (th & 7) || (len % srcpitch))
	{
		// TODO

		SwizzleTextureX(tx, ty, src, len, BITBLTBUF, TRXPOS, TRXREG);
	}
	else
	{
		__declspec(align(16)) DWORD block[8 * 8];

		th += ty;

		for(int y = ty; y < th; y += 8, src += srcpitch * 8)
		{
			for(int x = tx; x < tw; x += 8)
			{
				ExpandBlock4HH(src + (x - tx) / 2, srcpitch, block);

				SwizzleBlock32((BYTE*)&m_vm32[blockAddress32(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], (BYTE*)block, sizeof(block) / 8, 0xf0000000);
			}
		}

		ty = th;
	}
}

void GSLocalMemory::SwizzleTexture32Z(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG)
{
	if(TRXREG.RRW == 0) return;

	int tw = TRXREG.RRW, srcpitch = (TRXREG.RRW - TRXPOS.DSAX) * 4;
	int th = len / srcpitch;

	bool aligned = IsTopLeftAligned(TRXPOS.DSAX, tx, ty, 8, 8);

	if(!aligned || (tw & 7) || (th & 7) || (len % srcpitch))
	{
		if(aligned && tw >= 8 && th >= 8)
		{
			int twa = tw & ~7;
			int tha = th & ~7;

			len -= tha * srcpitch;
			th -= tha;

			for(int j = 0; j < tha; j += 8)
			{
				for(int x = tx; x < twa; x += 8)
				{
					SwizzleBlock32u((BYTE*)&m_vm32[blockAddress32Z(x, ty, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 4, srcpitch);
				}

				for(int i = 0; i < 8; i++, ty++, src += srcpitch)
				{
					for(int x = twa; x < tw; x++)
					{
						writePixel32Z(x, ty, ((DWORD*)src)[x - tx], BITBLTBUF.DBP, BITBLTBUF.DBW);
					}
				}
			}
		}

		if(len > 0 && tw >= 8 && th >= 2 && IsTopLeftAligned(TRXPOS.DSAX, tx, ty, 8, 2))
		{
			int twa = tw & ~7;
			int tha = th & ~1;

			len -= tha * srcpitch;
			th -= tha;

			for(int j = 0; j < tha; j += 2)
			{
				for(int x = tx; x < twa; x += 8)
				{
					SwizzleColumn32(ty, (BYTE*)&m_vm32[blockAddress32Z(x, ty & ~7, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 4, srcpitch);
				}

				for(int i = 0; i < 2; i++, ty++, src += srcpitch)
				{
					for(int x = twa; x < tw; x++)
					{
						writePixel32Z(x, ty, ((DWORD*)src)[x - tx], BITBLTBUF.DBP, BITBLTBUF.DBW);
					}
				}
			}
		}

		SwizzleTextureX(tx, ty, src, len, BITBLTBUF, TRXPOS, TRXREG);
	}
	else
	{
		th += ty;

		if((DWORD_PTR)src & 0xf)
		{
			for(int y = ty; y < th; y += 8, src += srcpitch * 8)
			{
				for(int x = tx; x < tw; x += 8)
				{
					SwizzleBlock32u((BYTE*)&m_vm32[blockAddress32Z(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 4, srcpitch);
				}
			}
		}
		else
		{
			for(int y = ty; y < th; y += 8, src += srcpitch * 8)
			{
				for(int x = tx; x < tw; x += 8)
				{
					SwizzleBlock32((BYTE*)&m_vm32[blockAddress32Z(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 4, srcpitch);
				}
			}
		}

		ty = th;
	}
}

void GSLocalMemory::SwizzleTexture24Z(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG)
{
	if(TRXREG.RRW == 0) return;

	int tw = TRXREG.RRW, srcpitch = (TRXREG.RRW - TRXPOS.DSAX) * 3;
	int th = len / srcpitch;

	bool aligned = IsTopLeftAligned(TRXPOS.DSAX, tx, ty, 8, 8);

	if(!aligned || (tw & 7) || (th & 7) || (len % srcpitch))
	{
		// TODO

		SwizzleTextureX(tx, ty, src, len, BITBLTBUF, TRXPOS, TRXREG);
	}
	else
	{
		__declspec(align(16)) DWORD block[8 * 8];

		th += ty;

		for(int y = ty; y < th; y += 8, src += srcpitch * 8)
		{
			for(int x = tx; x < tw; x += 8)
			{
				ExpandBlock24(src + (x - tx) * 3, srcpitch, block);

				SwizzleBlock32((BYTE*)&m_vm32[blockAddress32Z(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], (BYTE*)block, sizeof(block) / 8, 0x00ffffff);
			}
		}

		ty = th;
	}
}

void GSLocalMemory::SwizzleTexture16Z(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG)
{
	if(TRXREG.RRW == 0) return;

	int tw = TRXREG.RRW, srcpitch = (TRXREG.RRW - TRXPOS.DSAX) * 2;
	int th = len / srcpitch;

	bool aligned = IsTopLeftAligned(TRXPOS.DSAX, tx, ty, 16, 8);

	if(!aligned || (tw & 15) || (th & 7) || (len % srcpitch))
	{
		if(aligned && tw >= 16 && th >= 8)
		{
			int twa = tw & ~15;
			int tha = th & ~7;

			len -= tha * srcpitch;
			th -= tha;

			for(int j = 0; j < tha; j += 8)
			{
				for(int x = tx; x < twa; x += 16)
				{
					SwizzleBlock16u((BYTE*)&m_vm16[blockAddress16Z(x, ty, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}

				for(int i = 0; i < 8; i++, ty++, src += srcpitch)
				{
					for(int x = twa; x < tw; x++)
					{
						writePixel16Z(x, ty, ((WORD*)src)[x - tx], BITBLTBUF.DBP, BITBLTBUF.DBW);
					}
				}
			}
		}

		if(len > 0 && tw >= 16 && th >= 2 && IsTopLeftAligned(TRXPOS.DSAX, tx, ty, 16, 2))
		{
			int twa = tw & ~15;
			int tha = th & ~1;

			len -= tha * srcpitch;
			th -= tha;

			for(int j = 0; j < tha; j += 2)
			{
				for(int x = tx; x < twa; x += 16)
				{
					SwizzleColumn16(ty, (BYTE*)&m_vm16[blockAddress16Z(x, ty & ~7, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}

				for(int i = 0; i < 2; i++, ty++, src += srcpitch)
				{
					for(int x = twa; x < tw; x++)
					{
						writePixel16Z(x, ty, ((WORD*)src)[x - tx], BITBLTBUF.DBP, BITBLTBUF.DBW);
					}
				}
			}
		}

		SwizzleTextureX(tx, ty, src, len, BITBLTBUF, TRXPOS, TRXREG);
	}
	else
	{
		th += ty;

		if((DWORD_PTR)src & 0xf)
		{
			for(int y = ty; y < th; y += 8, src += srcpitch * 8)
			{
				for(int x = tx; x < tw; x += 16)
				{
					SwizzleBlock16u((BYTE*)&m_vm16[blockAddress16Z(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}
			}
		}
		else
		{
			for(int y = ty; y < th; y += 8, src += srcpitch * 8)
			{
				for(int x = tx; x < tw; x += 16)
				{
					SwizzleBlock16((BYTE*)&m_vm16[blockAddress16Z(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}
			} 
		}

		ty = th;
	}
}

void GSLocalMemory::SwizzleTexture16SZ(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG)
{
	if(TRXREG.RRW == 0) return;

	int tw = TRXREG.RRW, srcpitch = (TRXREG.RRW - TRXPOS.DSAX) * 2;
	int th = len / srcpitch;

	bool aligned = IsTopLeftAligned(TRXPOS.DSAX, tx, ty, 16, 8);

	if(!aligned || (tw & 15) || (th & 7) || (len % srcpitch))
	{
		if(aligned && tw >= 16 && th >= 8)
		{
			int twa = tw & ~15;
			int tha = th & ~7;

			len -= tha * srcpitch;
			th -= tha;

			for(int j = 0; j < tha; j += 8)
			{
				for(int x = tx; x < twa; x += 16)
				{
					SwizzleBlock16u((BYTE*)&m_vm16[blockAddress16SZ(x, ty, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}

				for(int i = 0; i < 8; i++, ty++, src += srcpitch)
				{
					for(int x = twa; x < tw; x++)
					{
						writePixel16SZ(x, ty, ((WORD*)src)[x - tx], BITBLTBUF.DBP, BITBLTBUF.DBW);
					}
				}
			}
		}

		if(len > 0 && tw >= 16 && th >= 2 && IsTopLeftAligned(TRXPOS.DSAX, tx, ty, 16, 2))
		{
			int twa = tw & ~15;
			int tha = th & ~1;

			len -= tha * srcpitch;
			th -= tha;

			for(int j = 0; j < tha; j += 2)
			{
				for(int x = tx; x < twa; x += 16)
				{
					SwizzleColumn16(ty, (BYTE*)&m_vm16[blockAddress16SZ(x, ty & ~7, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}

				for(int i = 0; i < 2; i++, ty++, src += srcpitch)
				{
					for(int x = twa; x < tw; x++)
					{
						writePixel16SZ(x, ty, ((WORD*)src)[x - tx], BITBLTBUF.DBP, BITBLTBUF.DBW);
					}
				}
			}
		}

		SwizzleTextureX(tx, ty, src, len, BITBLTBUF, TRXPOS, TRXREG);
	}
	else
	{
		th += ty;

		if((DWORD_PTR)src & 0xf)
		{
			for(int y = ty; y < th; y += 8, src += srcpitch * 8)
			{
				for(int x = tx; x < tw; x += 16)
				{
					SwizzleBlock16((BYTE*)&m_vm16[blockAddress16SZ(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}
			}
		}
		else
		{
			for(int y = ty; y < th; y += 8, src += srcpitch * 8)
			{
				for(int x = tx; x < tw; x += 16)
				{
					SwizzleBlock16((BYTE*)&m_vm16[blockAddress16SZ(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}
			}
		}

		ty = th;
	}
}

void GSLocalMemory::SwizzleTextureX(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG)
{
	if(len <= 0) return;

	BYTE* pb = (BYTE*)src;
	WORD* pw = (WORD*)src;
	DWORD* pd = (DWORD*)src;

	// if(ty >= (int)TRXREG.RRH) {ASSERT(0); return;}

	switch(BITBLTBUF.DPSM)
	{
	case PSM_PSMCT32:
		for(len /= 4; len-- > 0; SwizzleTextureStep(tx, ty, TRXPOS, TRXREG), pd++)
			writePixel32(tx, ty, *pd, BITBLTBUF.DBP, BITBLTBUF.DBW);
		break;
	case PSM_PSMCT24:
		for(len /= 3; len-- > 0; SwizzleTextureStep(tx, ty, TRXPOS, TRXREG), pb+=3)
			writePixel24(tx, ty, *(DWORD*)pb, BITBLTBUF.DBP, BITBLTBUF.DBW);
		break;
	case PSM_PSMCT16:
		for(len /= 2; len-- > 0; SwizzleTextureStep(tx, ty, TRXPOS, TRXREG), pw++)
			writePixel16(tx, ty, *pw, BITBLTBUF.DBP, BITBLTBUF.DBW);
		break;
	case PSM_PSMCT16S:
		for(len /= 2; len-- > 0; SwizzleTextureStep(tx, ty, TRXPOS, TRXREG), pw++)
			writePixel16S(tx, ty, *pw, BITBLTBUF.DBP, BITBLTBUF.DBW);
		break;
	case PSM_PSMT8:
		for(; len-- > 0; SwizzleTextureStep(tx, ty, TRXPOS, TRXREG), pb++)
			writePixel8(tx, ty, *pb, BITBLTBUF.DBP, BITBLTBUF.DBW);
		break;
	case PSM_PSMT4:
		for(; len-- > 0; SwizzleTextureStep(tx, ty, TRXPOS, TRXREG), SwizzleTextureStep(tx, ty, TRXPOS, TRXREG), pb++)
			writePixel4(tx, ty, *pb&0xf, BITBLTBUF.DBP, BITBLTBUF.DBW),
			writePixel4(tx+1, ty, *pb>>4, BITBLTBUF.DBP, BITBLTBUF.DBW);
		break;
	case PSM_PSMT8H:
		for(; len-- > 0; SwizzleTextureStep(tx, ty, TRXPOS, TRXREG), pb++)
			writePixel8H(tx, ty, *pb, BITBLTBUF.DBP, BITBLTBUF.DBW);
		break;
	case PSM_PSMT4HL:
		for(; len-- > 0; SwizzleTextureStep(tx, ty, TRXPOS, TRXREG), SwizzleTextureStep(tx, ty, TRXPOS, TRXREG), pb++)
			writePixel4HL(tx, ty, *pb & 0xf, BITBLTBUF.DBP, BITBLTBUF.DBW),
			writePixel4HL(tx+1, ty, *pb >> 4, BITBLTBUF.DBP, BITBLTBUF.DBW);
		break;
	case PSM_PSMT4HH:
		for(; len-- > 0; SwizzleTextureStep(tx, ty, TRXPOS, TRXREG), SwizzleTextureStep(tx, ty, TRXPOS, TRXREG), pb++)
			writePixel4HH(tx, ty, *pb & 0xf, BITBLTBUF.DBP, BITBLTBUF.DBW),
			writePixel4HH(tx+1, ty, *pb >> 4, BITBLTBUF.DBP, BITBLTBUF.DBW);
		break;
	case PSM_PSMZ32:
		for(len /= 4; len-- > 0; SwizzleTextureStep(tx, ty, TRXPOS, TRXREG), pd++)
			writePixel32Z(tx, ty, *pd, BITBLTBUF.DBP, BITBLTBUF.DBW);
		break;
	case PSM_PSMZ24:
		for(len /= 3; len-- > 0; SwizzleTextureStep(tx, ty, TRXPOS, TRXREG), pb+=3)
			writePixel24Z(tx, ty, *(DWORD*)pb, BITBLTBUF.DBP, BITBLTBUF.DBW);
		break;
	case PSM_PSMZ16:
		for(len /= 2; len-- > 0; SwizzleTextureStep(tx, ty, TRXPOS, TRXREG), pw++)
			writePixel16Z(tx, ty, *pw, BITBLTBUF.DBP, BITBLTBUF.DBW);
		break;
	case PSM_PSMZ16S:
		for(len /= 2; len-- > 0; SwizzleTextureStep(tx, ty, TRXPOS, TRXREG), pw++)
			writePixel16SZ(tx, ty, *pw, BITBLTBUF.DBP, BITBLTBUF.DBW);
		break;
	}
}

///////////////////

void GSLocalMemory::unSwizzleTexture32(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	FOREACH_BLOCK_START(r, 8, 8, 32)
	{
		unSwizzleBlock32((BYTE*)&m_vm32[blockAddress32(x, y, TEX0.TBP0, TEX0.TBW)], ptr + (x-r.left)*4, dstpitch);
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::unSwizzleTexture24(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	FOREACH_BLOCK_START(r, 8, 8, 32)
	{
		__declspec(align(16)) DWORD block[8*8];
		unSwizzleBlock32((BYTE*)&m_vm32[blockAddress32(x, y, TEX0.TBP0, TEX0.TBW)], (BYTE*)block, sizeof(block)/8);
		ExpandBlock24(block, (DWORD*)ptr + (x-r.left), dstpitch, &TEXA);
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::unSwizzleTexture16(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	FOREACH_BLOCK_START(r, 16, 8, 16)
	{
		__declspec(align(16)) WORD block[16*8];
		unSwizzleBlock16((BYTE*)&m_vm16[blockAddress16(x, y, TEX0.TBP0, TEX0.TBW)], (BYTE*)block, sizeof(block)/8);
		ExpandBlock16(block, (DWORD*)ptr + (x-r.left), dstpitch, &TEXA);
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::unSwizzleTexture16S(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	FOREACH_BLOCK_START(r, 16, 8, 16S)
	{
		__declspec(align(16)) WORD block[16*8];
		unSwizzleBlock16((BYTE*)&m_vm16[blockAddress16S(x, y, TEX0.TBP0, TEX0.TBW)], (BYTE*)block, sizeof(block)/8);
		ExpandBlock16(block, (DWORD*)ptr + (x-r.left), dstpitch, &TEXA);
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::unSwizzleTexture8(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	FOREACH_BLOCK_START(r, 16, 16, 8)
	{
		__declspec(align(16)) BYTE block[16*16];
		unSwizzleBlock8((BYTE*)&m_vm8[blockAddress8(x, y, TEX0.TBP0, TEX0.TBW)], (BYTE*)block, sizeof(block)/16);

		BYTE* s = block;
		BYTE* d = ptr + (x-r.left)*4;

		for(int j = 0; j < 16; j++, s += 16, d += dstpitch)
			for(int i = 0; i < 16; i++)
				((DWORD*)d)[i] = m_pCLUT32[s[i]];
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::unSwizzleTexture8H(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	FOREACH_BLOCK_START(r, 8, 8, 32)
	{
		__declspec(align(16)) DWORD block[8*8];
		unSwizzleBlock32((BYTE*)&m_vm32[blockAddress32(x, y, TEX0.TBP0, TEX0.TBW)], (BYTE*)block, sizeof(block)/8);

		DWORD* s = block;
		BYTE* d = ptr + (x-r.left)*4;

		for(int j = 0; j < 8; j++, s += 8, d += dstpitch)
			for(int i = 0; i < 8; i++)
				((DWORD*)d)[i] = m_pCLUT32[s[i] >> 24];
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::unSwizzleTexture4(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	FOREACH_BLOCK_START(r, 32, 16, 4)
	{
		__declspec(align(16)) BYTE block[(32/2)*16];
		unSwizzleBlock4((BYTE*)&m_vm8[blockAddress4(x, y, TEX0.TBP0, TEX0.TBW)>>1], (BYTE*)block, sizeof(block)/16);

		BYTE* s = block;
		BYTE* d = ptr + (x-r.left)*4;

		for(int j = 0; j < 16; j++, s += 32/2, d += dstpitch)
			for(int i = 0; i < 32/2; i++)
				((UINT64*)d)[i] = m_pCLUT64[s[i]];
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::unSwizzleTexture4HL(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	FOREACH_BLOCK_START(r, 8, 8, 32)
	{
		__declspec(align(16)) DWORD block[8*8];
		unSwizzleBlock32((BYTE*)&m_vm32[blockAddress32(x, y, TEX0.TBP0, TEX0.TBW)], (BYTE*)block, sizeof(block)/8);

		DWORD* s = block;
		BYTE* d = ptr + (x-r.left)*4;

		for(int j = 0; j < 8; j++, s += 8, d += dstpitch)
			for(int i = 0; i < 8; i++)
				((DWORD*)d)[i] = m_pCLUT32[(s[i] >> 24)&0x0f];
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::unSwizzleTexture4HH(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	FOREACH_BLOCK_START(r, 8, 8, 32)
	{
		__declspec(align(16)) DWORD block[8*8];
		unSwizzleBlock32((BYTE*)&m_vm32[blockAddress32(x, y, TEX0.TBP0, TEX0.TBW)], (BYTE*)block, sizeof(block)/8);

		DWORD* s = block;
		BYTE* d = ptr + (x-r.left)*4;

		for(int j = 0; j < 8; j++, s += 8, d += dstpitch)
			for(int i = 0; i < 8; i++)
				((DWORD*)d)[i] = m_pCLUT32[s[i] >> 28];
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::unSwizzleTexture32Z(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	FOREACH_BLOCK_START(r, 8, 8, 32)
	{
		unSwizzleBlock32((BYTE*)&m_vm32[blockAddress32Z(x, y, TEX0.TBP0, TEX0.TBW)], ptr + (x-r.left)*4, dstpitch);
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::unSwizzleTexture24Z(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	FOREACH_BLOCK_START(r, 8, 8, 32)
	{
		__declspec(align(16)) DWORD block[8*8];
		unSwizzleBlock32((BYTE*)&m_vm32[blockAddress32Z(x, y, TEX0.TBP0, TEX0.TBW)], (BYTE*)block, sizeof(block)/8);
		ExpandBlock24(block, (DWORD*)ptr + (x-r.left), dstpitch, &TEXA);
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::unSwizzleTexture16Z(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	FOREACH_BLOCK_START(r, 16, 8, 16)
	{
		__declspec(align(16)) WORD block[16*8];
		unSwizzleBlock16((BYTE*)&m_vm16[blockAddress16Z(x, y, TEX0.TBP0, TEX0.TBW)], (BYTE*)block, sizeof(block)/8);
		ExpandBlock16(block, (DWORD*)ptr + (x-r.left), dstpitch, &TEXA);
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::unSwizzleTexture16SZ(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	FOREACH_BLOCK_START(r, 16, 8, 16S)
	{
		__declspec(align(16)) WORD block[16*8];
		unSwizzleBlock16((BYTE*)&m_vm16[blockAddress16SZ(x, y, TEX0.TBP0, TEX0.TBW)], (BYTE*)block, sizeof(block)/8);
		ExpandBlock16(block, (DWORD*)ptr + (x-r.left), dstpitch, &TEXA);
	}
	FOREACH_BLOCK_END
}

///////////////////

void GSLocalMemory::ReadTexture(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA, GIFRegCLAMP& CLAMP)
{
	unSwizzleTexture st = m_psm[TEX0.PSM].ust;
	readTexel rt = m_psm[TEX0.PSM].rt;
	CSize bs = m_psm[TEX0.PSM].bs;

	if(r.Width() < bs.cx || r.Height() < bs.cy 
	|| (r.left & (bs.cx-1)) || (r.top & (bs.cy-1)) 
	|| (r.right & (bs.cx-1)) || (r.bottom & (bs.cy-1)) 
	|| (CLAMP.WMS == 3) || (CLAMP.WMT == 3))
	{
		ReadTexture<DWORD>(r, dst, dstpitch, TEX0, TEXA, CLAMP, rt, st);
	}
	else
	{
		(this->*st)(r, dst, dstpitch, TEX0, TEXA);
	}
}

void GSLocalMemory::ReadTextureNC(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA, GIFRegCLAMP& CLAMP)
{
	unSwizzleTexture st = m_psm[TEX0.PSM].ust;
	readTexel rt = m_psm[TEX0.PSM].rt;
	CSize bs = m_psm[TEX0.PSM].bs;

	if(r.Width() < bs.cx || r.Height() < bs.cy 
	|| (r.left & (bs.cx-1)) || (r.top & (bs.cy-1)) 
	|| (r.right & (bs.cx-1)) || (r.bottom & (bs.cy-1)))
	{
		ReadTextureNC<DWORD>(r, dst, dstpitch, TEX0, TEXA, rt, st);
	}
	else
	{
		(this->*st)(r, dst, dstpitch, TEX0, TEXA);
	}
}
///////////////////

void GSLocalMemory::unSwizzleTexture16NP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	FOREACH_BLOCK_START(r, 16, 8, 16)
	{
		unSwizzleBlock16((BYTE*)&m_vm16[blockAddress16(x, y, TEX0.TBP0, TEX0.TBW)], ptr + (x-r.left)*2, dstpitch);
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::unSwizzleTexture16SNP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	FOREACH_BLOCK_START(r, 16, 8, 16S)
	{
		unSwizzleBlock16((BYTE*)&m_vm16[blockAddress16S(x, y, TEX0.TBP0, TEX0.TBW)], ptr + (x-r.left)*2, dstpitch);
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::unSwizzleTexture8NP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	FOREACH_BLOCK_START(r, 16, 16, 8)
	{
		__declspec(align(16)) BYTE block[16*16];
		unSwizzleBlock8((BYTE*)&m_vm8[blockAddress8(x, y, TEX0.TBP0, TEX0.TBW)], (BYTE*)block, sizeof(block)/16);

		BYTE* s = block;

		if(TEX0.CPSM == PSM_PSMCT32)
		{
			BYTE* d = ptr + (x-r.left)*4;
			for(int j = 0; j < 16; j++, s += 16, d += dstpitch)
				for(int i = 0; i < 16; i++)
					((DWORD*)d)[i] = m_pCLUT32[s[i]];
		}
		else
		{
			ASSERT(TEX0.CPSM == PSM_PSMCT16 || TEX0.CPSM == PSM_PSMCT16S);

			BYTE* d = ptr + (x-r.left)*2;
			for(int j = 0; j < 16; j++, s += 16, d += dstpitch)
				for(int i = 0; i < 16; i++)
					((WORD*)d)[i] = (WORD)m_pCLUT32[s[i]];
		}
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::unSwizzleTexture8HNP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	FOREACH_BLOCK_START(r, 8, 8, 32)
	{
		__declspec(align(16)) DWORD block[8*8];
		unSwizzleBlock32((BYTE*)&m_vm32[blockAddress32(x, y, TEX0.TBP0, TEX0.TBW)], (BYTE*)block, sizeof(block)/8);

		DWORD* s = block;

		if(TEX0.CPSM == PSM_PSMCT32)
		{
			BYTE* d = ptr + (x-r.left)*4;
			for(int j = 0; j < 8; j++, s += 8, d += dstpitch)
				for(int i = 0; i < 8; i++)
					((DWORD*)d)[i] = m_pCLUT32[s[i] >> 24];
		}
		else
		{
			ASSERT(TEX0.CPSM == PSM_PSMCT16 || TEX0.CPSM == PSM_PSMCT16S);

			BYTE* d = ptr + (x-r.left)*2;
			for(int j = 0; j < 8; j++, s += 8, d += dstpitch)
				for(int i = 0; i < 8; i++)
					((WORD*)d)[i] = (WORD)m_pCLUT32[s[i] >> 24];
		}
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::unSwizzleTexture4NP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	FOREACH_BLOCK_START(r, 32, 16, 4)
	{
		__declspec(align(16)) BYTE block[(32/2)*16];
		unSwizzleBlock4((BYTE*)&m_vm8[blockAddress4(x, y, TEX0.TBP0, TEX0.TBW)>>1], (BYTE*)block, sizeof(block)/16);

		BYTE* s = block;

		if(TEX0.CPSM == PSM_PSMCT32)
		{
			BYTE* d = ptr + (x-r.left)*4;

			for(int j = 0; j < 16; j++, s += 32/2, d += dstpitch)
				for(int i = 0; i < 32/2; i++)
					((UINT64*)d)[i] = m_pCLUT64[s[i]];
		}
		else
		{
			ASSERT(TEX0.CPSM == PSM_PSMCT16 || TEX0.CPSM == PSM_PSMCT16S);

			BYTE* d = ptr + (x-r.left)*2;
			for(int j = 0; j < 16; j++, s += 32/2, d += dstpitch)
				for(int i = 0; i < 32/2; i++)
					((DWORD*)d)[i] = (DWORD)m_pCLUT64[s[i]];
		}
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::unSwizzleTexture4HLNP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	FOREACH_BLOCK_START(r, 8, 8, 32)
	{
		__declspec(align(16)) DWORD block[8*8];
		unSwizzleBlock32((BYTE*)&m_vm32[blockAddress32(x, y, TEX0.TBP0, TEX0.TBW)], (BYTE*)block, sizeof(block)/8);

		DWORD* s = block;

		if(TEX0.CPSM == PSM_PSMCT32)
		{
			BYTE* d = ptr + (x-r.left)*4;
			for(int j = 0; j < 8; j++, s += 8, d += dstpitch)
				for(int i = 0; i < 8; i++)
					((DWORD*)d)[i] = m_pCLUT32[(s[i] >> 24)&0x0f];
		}
		else
		{
			ASSERT(TEX0.CPSM == PSM_PSMCT16 || TEX0.CPSM == PSM_PSMCT16S);

			BYTE* d = ptr + (x-r.left)*2;
			for(int j = 0; j < 8; j++, s += 8, d += dstpitch)
				for(int i = 0; i < 8; i++)
					((WORD*)d)[i] = (WORD)m_pCLUT32[(s[i] >> 24)&0x0f];
		}
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::unSwizzleTexture4HHNP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	FOREACH_BLOCK_START(r, 8, 8, 32)
	{
		__declspec(align(16)) DWORD block[8*8];
		unSwizzleBlock32((BYTE*)&m_vm32[blockAddress32(x, y, TEX0.TBP0, TEX0.TBW)], (BYTE*)block, sizeof(block)/8);

		DWORD* s = block;

		if(TEX0.CPSM == PSM_PSMCT32)
		{
			BYTE* d = ptr + (x-r.left)*4;
			for(int j = 0; j < 8; j++, s += 8, d += dstpitch)
				for(int i = 0; i < 8; i++)
					((DWORD*)d)[i] = m_pCLUT32[s[i] >> 28];
		}
		else
		{
			ASSERT(TEX0.CPSM == PSM_PSMCT16 || TEX0.CPSM == PSM_PSMCT16S);

			BYTE* d = ptr + (x-r.left)*2;
			for(int j = 0; j < 8; j++, s += 8, d += dstpitch)
				for(int i = 0; i < 8; i++)
					((WORD*)d)[i] = (WORD)m_pCLUT32[s[i] >> 28];
		}
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::unSwizzleTexture16ZNP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	FOREACH_BLOCK_START(r, 16, 8, 16)
	{
		unSwizzleBlock16((BYTE*)&m_vm16[blockAddress16Z(x, y, TEX0.TBP0, TEX0.TBW)], ptr + (x-r.left)*2, dstpitch);
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::unSwizzleTexture16SZNP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	FOREACH_BLOCK_START(r, 16, 8, 16S)
	{
		unSwizzleBlock16((BYTE*)&m_vm16[blockAddress16SZ(x, y, TEX0.TBP0, TEX0.TBW)], ptr + (x-r.left)*2, dstpitch);
	}
	FOREACH_BLOCK_END
}

///////////////////

void GSLocalMemory::ReadTextureNP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA, GIFRegCLAMP& CLAMP)
{
	unSwizzleTexture st = m_psm[TEX0.PSM].ustNP;
	readTexel rt = m_psm[TEX0.PSM].rtNP;
	CSize bs = m_psm[TEX0.PSM].bs;

	if(r.Width() < bs.cx || r.Height() < bs.cy 
	|| (r.left & (bs.cx-1)) || (r.top & (bs.cy-1)) 
	|| (r.right & (bs.cx-1)) || (r.bottom & (bs.cy-1)) 
	|| (CLAMP.WMS == 3) || (CLAMP.WMT == 3))
	{
		DWORD psm = TEX0.PSM;

		switch(psm)
		{
		case PSM_PSMT8:
		case PSM_PSMT8H:
		case PSM_PSMT4:
		case PSM_PSMT4HL:
		case PSM_PSMT4HH:
			psm = TEX0.CPSM;
			break;
		}

		switch(psm)
		{
		default:
		case PSM_PSMCT32:
		case PSM_PSMCT24:
			ReadTexture<DWORD>(r, dst, dstpitch, TEX0, TEXA, CLAMP, rt, st);
			break;
		case PSM_PSMCT16:
		case PSM_PSMCT16S:
			ReadTexture<WORD>(r, dst, dstpitch, TEX0, TEXA, CLAMP, rt, st);
			break;
		}
	}
	else
	{
		(this->*st)(r, dst, dstpitch, TEX0, TEXA);
	}
}

void GSLocalMemory::ReadTextureNPNC(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA, GIFRegCLAMP& CLAMP)
{
	unSwizzleTexture st = m_psm[TEX0.PSM].ustNP;
	readTexel rt = m_psm[TEX0.PSM].rtNP;
	CSize bs = m_psm[TEX0.PSM].bs;

	if(r.Width() < bs.cx || r.Height() < bs.cy 
	|| (r.left & (bs.cx-1)) || (r.top & (bs.cy-1)) 
	|| (r.right & (bs.cx-1)) || (r.bottom & (bs.cy-1)))
	{
		DWORD psm = TEX0.PSM;

		switch(psm)
		{
		case PSM_PSMT8:
		case PSM_PSMT8H:
		case PSM_PSMT4:
		case PSM_PSMT4HL:
		case PSM_PSMT4HH:
			psm = TEX0.CPSM;
			break;
		}

		switch(psm)
		{
		default:
		case PSM_PSMCT32:
		case PSM_PSMCT24:
			ReadTextureNC<DWORD>(r, dst, dstpitch, TEX0, TEXA, rt, st);
			break;
		case PSM_PSMCT16:
		case PSM_PSMCT16S:
			ReadTextureNC<WORD>(r, dst, dstpitch, TEX0, TEXA, rt, st);
			break;
		}
	}
	else
	{
		(this->*st)(r, dst, dstpitch, TEX0, TEXA);
	}
}

//

template<typename T> 
void GSLocalMemory::ReadTexture(CRect r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA, GIFRegCLAMP& CLAMP, readTexel rt, unSwizzleTexture st)
{
	// TODO: this is a mess, make it more simple

	DWORD wms = CLAMP.WMS, wmt = CLAMP.WMT;
	DWORD minu = CLAMP.MINU, maxu = CLAMP.MAXU;
	DWORD minv = CLAMP.MINV, maxv = CLAMP.MAXV;

	CSize bs = m_psm[TEX0.PSM].bs;

	int bsxm = bs.cx - 1;
	int bsym = bs.cy - 1;

	if(wms == 3 || wmt == 3)
	{
		if(wms == 3 && wmt == 3)
		{
			int w = minu + 1;
			int h = minv + 1;

			w = (w + bsxm) & ~bsxm;
			h = (h + bsym) & ~bsym;

			if(w % bs.cx == 0 && maxu % bs.cx == 0 && h % bs.cy == 0 && maxv % bs.cy == 0)
			{
// printf("!!! 1 wms = %d, wmt = %d, %3x %3x %3x %3x, %d %d - %d %d\n", wms, wmt, minu, maxu, minv, maxv, r.left, r.top, r.right, r.bottom);

				T* buff = (T*)_aligned_malloc(w * h * sizeof(T), 16);

				(this->*st)(CRect(CPoint(maxu, maxv), CSize(w, h)), (BYTE*)buff, w * sizeof(T), TEX0, TEXA);

				dst -= r.left * sizeof(T);

//				int left = (r.left + minu) & ~minu;
//				int right = r.right & ~minu;

				for(int y = r.top; y < r.bottom; y++, dst += dstpitch)
				{
					T* src = &buff[(y & minv) * w];

					int x = r.left;
/*
					for(; x < left; x++)
					{
						((T*)dst)[x] = src[x & minu];
					}

					for(; x < right; x += minu + 1)
					{
						memcpy(&((T*)dst)[x], src, sizeof(T) * (minu + 1));
					}
*/
					for(; x < r.right; x++)
					{
						((T*)dst)[x] = src[x & minu];
					}
				}

				_aligned_free(buff);

				return;
			}
		}

		if(wms == 2)
		{
			r.left = min(r.right, max(r.left, (int)minu));
			r.right = max(r.left, min(r.right, (int)maxu));
		}

		if(wmt == 2)
		{
			r.top = min(r.bottom, max(r.top, (int)minv));
			r.bottom = max(r.top, min(r.bottom, (int)maxv));
		}

		if(wms == 3 && wmt != 3)
		{
			int w = ((minu + 1) + bsxm) & ~bsxm;

			if(w % bs.cx == 0 && maxu % bs.cx == 0)
			{
// printf("!!! 2 wms = %d, wmt = %d, %3x %3x %3x %3x, %d %d - %d %d\n", wms, wmt, minu, maxu, minv, maxv, r.left, r.top, r.right, r.bottom);
				int top = r.top & ~bsym; 
				int bottom = (r.bottom + bsym) & ~bsym;
				
				int h = bottom - top;

				T* buff = (T*)_aligned_malloc(w * h * sizeof(T), 16);

				(this->*st)(CRect(CPoint(maxu, top), CSize(w, h)), (BYTE*)buff, w * sizeof(T), TEX0, TEXA);

				dst -= r.left * sizeof(T);

//				int left = (r.left + minu) & ~minu;
//				int right = r.right & ~minu;

				for(int y = r.top; y < r.bottom; y++, dst += dstpitch)
				{
					T* src = &buff[(y - top) * w];

					int x = r.left;
/*
					for(; x < left; x++)
					{
						((T*)dst)[x] = src[x & minu];
					}

					for(; x < right; x += minu + 1)
					{
						memcpy(&((T*)dst)[x], src, sizeof(T) * (minu + 1));
					}
*/
					for(; x < r.right; x++)
					{
						((T*)dst)[x] = src[x & minu];
					}
				}

				_aligned_free(buff);

				return;
			}
		}

		if(wms != 3 && wmt == 3)
		{
			int h = (minv + 1 + bsym) & ~bsym;

			if(h % bs.cy == 0 && maxv % bs.cy == 0)
			{
// printf("!!! 3 wms = %d, wmt = %d, %3x %3x %3x %3x, %d %d - %d %d\n", wms, wmt, minu, maxu, minv, maxv, r.left, r.top, r.right, r.bottom);
				int left = r.left & ~bsxm; 
				int right = (r.right + bsxm) & ~bsxm;
				
				int w = right - left;

				T* buff = (T*)_aligned_malloc(w * h * sizeof(T), 16);

				(this->*st)(CRect(CPoint(left, maxv), CSize(w, h)), (BYTE*)buff, w * sizeof(T), TEX0, TEXA);

				for(int y = r.top; y < r.bottom; y++, dst += dstpitch)
				{
					T* src = &buff[(y & minv) * w + (r.left - left)];

					memcpy(dst, src, sizeof(T) * r.Width());
				}

				_aligned_free(buff);

				return;
			}
		}

		switch(wms)
		{
		default: for(int x = r.left; x < r.right; x++) m_xtbl[x] = x; break;
		case 3: for(int x = r.left; x < r.right; x++) m_xtbl[x] = (x & minu) | maxu; break;
		}

		switch(wmt)
		{
		default: for(int y = r.top; y < r.bottom; y++) m_ytbl[y] = y; break;
		case 3: for(int y = r.top; y < r.bottom; y++) m_ytbl[y] = (y & minv) | maxv; break;
		}

// printf("!!! 4 wms = %d, wmt = %d, %3x %3x %3x %3x, %d %d - %d %d\n", wms, wmt, minu, maxu, minv, maxv, r.left, r.top, r.right, r.bottom);

		for(int y = r.top; y < r.bottom; y++, dst += dstpitch)
			for(int x = r.left, i = 0; x < r.right; x++, i++)
				((T*)dst)[i] = (T)(this->*rt)(m_xtbl[x], m_ytbl[y], TEX0, TEXA);
	}
	else
	{
		// find a block-aligned rect that fits between r and the region clamped area (if any)

		CRect r1 = r;
		CRect r2 = r;

		r1.left = (r1.left + bsxm) & ~bsxm;
		r1.top = (r1.top + bsym) & ~bsym;
		r1.right = r1.right & ~bsxm; 
		r1.bottom = r1.bottom & ~bsym; 

		if(wms == 2 && minu < maxu) 
		{
			r2.left = minu & ~bsxm; 
			r2.right = (maxu + bsxm) & ~bsxm;
		}

		if(wmt == 2 && minv < maxv) 
		{
			r2.top = minv & ~bsym; 
			r2.bottom = (maxv + bsym) & ~bsym;
		}

		CRect cr = r1 & r2;

		bool aligned = ((DWORD_PTR)(dst + (cr.left - r.left) * sizeof(T)) & 0xf) == 0;

		if(cr.left >= cr.right && cr.top >= cr.bottom || !aligned)
		{
			// TODO: expand r to block size, read into temp buffer, copy to r (like above)

if(!aligned) printf("unaligned memory pointer passed to ReadTexture\n");

// printf("!!! 5 wms = %d, wmt = %d, %3x %3x %3x %3x, %d %d - %d %d\n", wms, wmt, minu, maxu, minv, maxv, r.left, r.top, r.right, r.bottom);

			for(int y = r.top; y < r.bottom; y++, dst += dstpitch)
				for(int x = r.left, i = 0; x < r.right; x++, i++)
					((T*)dst)[i] = (T)(this->*rt)(x, y, TEX0, TEXA);
		}
		else
		{
// printf("!!! 6 wms = %d, wmt = %d, %3x %3x %3x %3x, %d %d - %d %d\n", wms, wmt, minu, maxu, minv, maxv, r.left, r.top, r.right, r.bottom);

			for(int y = r.top; y < cr.top; y++, dst += dstpitch)
				for(int x = r.left, i = 0; x < r.right; x++, i++)
					((T*)dst)[i] = (T)(this->*rt)(x, y, TEX0, TEXA);

			if(!cr.IsRectEmpty())
			{
				(this->*st)(cr, dst + (cr.left - r.left)*sizeof(T), dstpitch, TEX0, TEXA);
			}

			for(int y = cr.top; y < cr.bottom; y++, dst += dstpitch)
			{
				for(int x = r.left, i = 0; x < cr.left; x++, i++)
					((T*)dst)[i] = (T)(this->*rt)(x, y, TEX0, TEXA);
				for(int x = cr.right, i = x - r.left; x < r.right; x++, i++)
					((T*)dst)[i] = (T)(this->*rt)(x, y, TEX0, TEXA);
			}

			for(int y = cr.bottom; y < r.bottom; y++, dst += dstpitch)
				for(int x = r.left, i = 0; x < r.right; x++, i++)
					((T*)dst)[i] = (T)(this->*rt)(x, y, TEX0, TEXA);
		}
	}
}

template<typename T> 
void GSLocalMemory::ReadTextureNC(CRect r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA, readTexel rt, unSwizzleTexture st)
{
	CSize bs = m_psm[TEX0.PSM].bs;

	int bsxm = bs.cx - 1;
	int bsym = bs.cy - 1;

	CRect cr;

	cr.left = (r.left + bsxm) & ~bsxm;
	cr.top = (r.top + bsym) & ~bsym;
	cr.right = r.right & ~bsxm; 
	cr.bottom = r.bottom & ~bsym; 

	bool aligned = ((DWORD_PTR)(dst + (cr.left - r.left) * sizeof(T)) & 0xf) == 0;

	if(cr.left >= cr.right && cr.top >= cr.bottom || !aligned)
	{
		// TODO: expand r to block size, read into temp buffer, copy to r (like above)

if(!aligned) printf("unaligned memory pointer passed to ReadTexture\n");

		for(int y = r.top; y < r.bottom; y++, dst += dstpitch)
			for(int x = r.left, i = 0; x < r.right; x++, i++)
				((T*)dst)[i] = (T)(this->*rt)(x, y, TEX0, TEXA);
	}
	else
	{
		for(int y = r.top; y < cr.top; y++, dst += dstpitch)
			for(int x = r.left, i = 0; x < r.right; x++, i++)
				((T*)dst)[i] = (T)(this->*rt)(x, y, TEX0, TEXA);

		if(!cr.IsRectEmpty())
			(this->*st)(cr, dst + (cr.left - r.left)*sizeof(T), dstpitch, TEX0, TEXA);

		for(int y = cr.top; y < cr.bottom; y++, dst += dstpitch)
		{
			for(int x = r.left, i = 0; x < cr.left; x++, i++)
				((T*)dst)[i] = (T)(this->*rt)(x, y, TEX0, TEXA);
			for(int x = cr.right, i = x - r.left; x < r.right; x++, i++)
				((T*)dst)[i] = (T)(this->*rt)(x, y, TEX0, TEXA);
		}

		for(int y = cr.bottom; y < r.bottom; y++, dst += dstpitch)
			for(int x = r.left, i = 0; x < r.right; x++, i++)
				((T*)dst)[i] = (T)(this->*rt)(x, y, TEX0, TEXA);
	}
}

//

void GSLocalMemory::unSwizzleBlock32(BYTE* src, BYTE* dst, int dstpitch)
{
#if _M_SSE >= 0x200

	GSVector4i* s = (GSVector4i*)src;

	GSVector4i v0, v1, v2, v3;

	v0 = s[0]; 
	v1 = s[1]; 
	v2 = s[2]; 
	v3 = s[3];

	GSVector4i::sw64(v0, v1, v2, v3);

	*(GSVector4i*)&dst[dstpitch * 0 +  0] = v0;
	*(GSVector4i*)&dst[dstpitch * 0 + 16] = v1;
	*(GSVector4i*)&dst[dstpitch * 1 +  0] = v2;
	*(GSVector4i*)&dst[dstpitch * 1 + 16] = v3;

	v0 = s[4];
	v1 = s[5];
	v2 = s[6];
	v3 = s[7];

	GSVector4i::sw64(v0, v1, v2, v3);

	*(GSVector4i*)&dst[dstpitch * 2 +  0] = v0;
	*(GSVector4i*)&dst[dstpitch * 2 + 16] = v1;
	*(GSVector4i*)&dst[dstpitch * 3 +  0] = v2;
	*(GSVector4i*)&dst[dstpitch * 3 + 16] = v3;

	dst += dstpitch * 4;

	v0 = s[8];
	v1 = s[9];
	v2 = s[10];
	v3 = s[11];

	GSVector4i::sw64(v0, v1, v2, v3);

	*(GSVector4i*)&dst[dstpitch * 0 +  0] = v0;
	*(GSVector4i*)&dst[dstpitch * 0 + 16] = v1;
	*(GSVector4i*)&dst[dstpitch * 1 +  0] = v2;
	*(GSVector4i*)&dst[dstpitch * 1 + 16] = v3;

	v0 = s[12];
	v1 = s[13];
	v2 = s[14];
	v3 = s[15];

	GSVector4i::sw64(v0, v1, v2, v3);

	*(GSVector4i*)&dst[dstpitch * 2 +  0] = v0;
	*(GSVector4i*)&dst[dstpitch * 2 + 16] = v1;
	*(GSVector4i*)&dst[dstpitch * 3 +  0] = v2;
	*(GSVector4i*)&dst[dstpitch * 3 + 16] = v3;

#else

	const DWORD* s = &columnTable32[0][0];

	for(int j = 0; j < 8; j++, s += 8, dst += dstpitch)
	{
		for(int i = 0; i < 8; i++)
		{
			((DWORD*)dst)[i] = ((DWORD*)src)[s[i]];
		}
	}

#endif
}

static const GSVector4i s_us16mask(0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15);

void GSLocalMemory::unSwizzleBlock16(BYTE* src, BYTE* dst, int dstpitch)
{
#if _M_SSE >= 0x301

	GSVector4i* s = (GSVector4i*)src;

	GSVector4i v0, v1, v2, v3;

	GSVector4i mask = s_us16mask;

	for(int i = 0; i < 4; i++)
	{
		v0 = s[i * 4 + 0].shuffle8(mask);
		v1 = s[i * 4 + 1].shuffle8(mask);
		v2 = s[i * 4 + 2].shuffle8(mask);
		v3 = s[i * 4 + 3].shuffle8(mask);

		GSVector4i::sw32(v0, v1, v2, v3);
		GSVector4i::sw64(v0, v1, v2, v3);

		*(GSVector4i*)&dst[dstpitch * (i * 2 + 0) +  0] = v0;
		*(GSVector4i*)&dst[dstpitch * (i * 2 + 0) + 16] = v2;
		*(GSVector4i*)&dst[dstpitch * (i * 2 + 1) +  0] = v1;
		*(GSVector4i*)&dst[dstpitch * (i * 2 + 1) + 16] = v3;
	}

#elif _M_SSE >= 0x200

	GSVector4i* s = (GSVector4i*)src;

	GSVector4i v0, v1, v2, v3;

	for(int i = 0; i < 4; i++)
	{
		v0 = s[i * 4 + 0]; 
		v1 = s[i * 4 + 1]; 
		v2 = s[i * 4 + 2]; 
		v3 = s[i * 4 + 3];

		GSVector4i::sw16(v0, v1, v2, v3);
		GSVector4i::sw32(v0, v1, v2, v3);
		GSVector4i::sw16(v0, v2, v1, v3);

		*(GSVector4i*)&dst[dstpitch * (i * 2 + 0) +  0] = v0;
		*(GSVector4i*)&dst[dstpitch * (i * 2 + 0) + 16] = v1;
		*(GSVector4i*)&dst[dstpitch * (i * 2 + 1) +  0] = v2;
		*(GSVector4i*)&dst[dstpitch * (i * 2 + 1) + 16] = v3;
	}

#else

	const DWORD* s = &columnTable16[0][0];

	for(int j = 0; j < 8; j++, s += 16, dst += dstpitch)
	{
		for(int i = 0; i < 16; i++)
		{
			((WORD*)dst)[i] = ((WORD*)src)[s[i]];
		}
	}

#endif
}

static const GSVector4i s_us8mask(0, 4, 2, 6, 8, 12, 10, 14, 1, 5, 3, 7, 9, 13, 11, 15);

void GSLocalMemory::unSwizzleBlock8(BYTE* src, BYTE* dst, int dstpitch)
{
#if _M_SSE >= 0x301

	GSVector4i* s = (GSVector4i*)src;

	GSVector4i v0, v1, v2, v3;

	GSVector4i mask = s_us8mask;

	for(int i = 0; i < 2; i++)
	{
		v0 = s[i * 8 + 0].shuffle8(mask); 
		v1 = s[i * 8 + 1].shuffle8(mask); 
		v2 = s[i * 8 + 2].shuffle8(mask); 
		v3 = s[i * 8 + 3].shuffle8(mask);

		GSVector4i::sw16(v0, v1, v2, v3);
		GSVector4i::sw32(v0, v1, v3, v2);

		*(GSVector4i*)&dst[dstpitch * 0] = v0;
		*(GSVector4i*)&dst[dstpitch * 1] = v3;
		*(GSVector4i*)&dst[dstpitch * 2] = v1;
		*(GSVector4i*)&dst[dstpitch * 3] = v2;

		dst += dstpitch * 4;

		v2 = s[i * 8 + 4].shuffle8(mask); 
		v3 = s[i * 8 + 5].shuffle8(mask); 
		v0 = s[i * 8 + 6].shuffle8(mask); 
		v1 = s[i * 8 + 7].shuffle8(mask);

		GSVector4i::sw16(v0, v1, v2, v3);
		GSVector4i::sw32(v0, v1, v3, v2);

		*(GSVector4i*)&dst[dstpitch * 0] = v0;
		*(GSVector4i*)&dst[dstpitch * 1] = v3;
		*(GSVector4i*)&dst[dstpitch * 2] = v1;
		*(GSVector4i*)&dst[dstpitch * 3] = v2;

		dst += dstpitch * 4;
	}

#elif _M_SSE >= 0x200

	GSVector4i* s = (GSVector4i*)src;

	GSVector4i v0, v1, v2, v3;

	for(int i = 0; i < 2; i++)
	{
		v0 = s[i * 8 + 0]; 
		v1 = s[i * 8 + 1]; 
		v2 = s[i * 8 + 2]; 
		v3 = s[i * 8 + 3];

		GSVector4i::sw8(v0, v1, v2, v3);
		GSVector4i::sw16(v0, v1, v2, v3);
		GSVector4i::sw8(v0, v2, v1, v3);
		GSVector4i::sw64(v0, v1, v2, v3);

		*(GSVector4i*)&dst[dstpitch * 0] = v0;
		*(GSVector4i*)&dst[dstpitch * 1] = v1;
		*(GSVector4i*)&dst[dstpitch * 2] = v2.yxwz();
		*(GSVector4i*)&dst[dstpitch * 3] = v3.yxwz();

		dst += dstpitch * 4;

		v0 = s[i * 8 + 4]; 
		v1 = s[i * 8 + 5]; 
		v2 = s[i * 8 + 6]; 
		v3 = s[i * 8 + 7];

		GSVector4i::sw8(v0, v1, v2, v3);
		GSVector4i::sw16(v0, v1, v2, v3);
		GSVector4i::sw8(v0, v2, v1, v3);
		GSVector4i::sw64(v0, v1, v2, v3);

		*(GSVector4i*)&dst[dstpitch * 0] = v0.yxwz();
		*(GSVector4i*)&dst[dstpitch * 1] = v1.yxwz();
		*(GSVector4i*)&dst[dstpitch * 2] = v2;
		*(GSVector4i*)&dst[dstpitch * 3] = v3;

		dst += dstpitch * 4;
	}

#else

	const DWORD* s = &columnTable8[0][0];

	for(int j = 0; j < 16; j++, s += 16, dst += dstpitch)
	{
		for(int i = 0; i < 16; i++)
		{
			dst[i] = src[s[i]];
		}
	}

#endif
}

void GSLocalMemory::unSwizzleBlock4(BYTE* src, BYTE* dst, int dstpitch)
{
#if _M_SSE >= 0x200

	GSVector4i* s = (GSVector4i*)src;

	GSVector4i v0, v1, v2, v3;

	for(int i = 0; i < 2; i++)
	{
		v0 = s[i * 8 + 0]; 
		v1 = s[i * 8 + 1]; 
		v2 = s[i * 8 + 2]; 
		v3 = s[i * 8 + 3];

		GSVector4i::sw32(v0, v1, v2, v3);
		GSVector4i::sw32(v0, v1, v2, v3);
		GSVector4i::sw4(v0, v2, v1, v3);
		GSVector4i::sw8(v0, v1, v2, v3);
		GSVector4i::sw16(v0, v2, v1, v3);

		v0 = v0.xzyw();
		v1 = v1.xzyw();
		v2 = v2.xzyw();
		v3 = v3.xzyw();

		GSVector4i::sw64(v0, v1, v2, v3);

		*(GSVector4i*)&dst[dstpitch * 0] = v0;
		*(GSVector4i*)&dst[dstpitch * 1] = v1;
		*(GSVector4i*)&dst[dstpitch * 2] = v2.yxwzl().yxwzh();
		*(GSVector4i*)&dst[dstpitch * 3] = v3.yxwzl().yxwzh();

		dst += dstpitch * 4;

		v0 = s[i * 8 + 4]; 
		v1 = s[i * 8 + 5]; 
		v2 = s[i * 8 + 6]; 
		v3 = s[i * 8 + 7];

		GSVector4i::sw32(v0, v1, v2, v3);
		GSVector4i::sw32(v0, v1, v2, v3);
		GSVector4i::sw4(v0, v2, v1, v3);
		GSVector4i::sw8(v0, v1, v2, v3);
		GSVector4i::sw16(v0, v2, v1, v3);

		v0 = v0.xzyw();
		v1 = v1.xzyw();
		v2 = v2.xzyw();
		v3 = v3.xzyw();

		GSVector4i::sw64(v0, v1, v2, v3);

		*(GSVector4i*)&dst[dstpitch * 0] = v0.yxwzl().yxwzh();
		*(GSVector4i*)&dst[dstpitch * 1] = v1.yxwzl().yxwzh();
		*(GSVector4i*)&dst[dstpitch * 2] = v2;
		*(GSVector4i*)&dst[dstpitch * 3] = v3;

		dst += dstpitch * 4;
	}

#else

	const DWORD* s = &columnTable4[0][0];

	for(int j = 0; j < 16; j++, s += 32, dst += dstpitch)
	{
		for(int i = 0; i < 32; i++)
		{
			DWORD addr = s[i];
			BYTE c = (src[addr >> 1] >> ((addr & 1) << 2)) & 0x0f;
			int shift = (i & 1) << 2;
			dst[i >> 1] = (dst[i >> 1] & (0xf0 >> shift)) | (c << shift);
		}
	}

#endif
}

void GSLocalMemory::SwizzleBlock32(BYTE* dst, BYTE* src, int srcpitch)
{
#if _M_SSE >= 0x200

	GSVector4i* d = (GSVector4i*)dst;
	
	GSVector4i v0, v1, v2, v3;

	v0 = *(GSVector4i*)&src[srcpitch * 0 +  0];
	v1 = *(GSVector4i*)&src[srcpitch * 0 + 16];
	v2 = *(GSVector4i*)&src[srcpitch * 1 +  0];
	v3 = *(GSVector4i*)&src[srcpitch * 1 + 16];

	GSVector4i::sw64(v0, v2, v1, v3);

	d[0] = v0;
	d[1] = v1;
	d[2] = v2;
	d[3] = v3;

	v0 = *(GSVector4i*)&src[srcpitch * 2 +  0];
	v1 = *(GSVector4i*)&src[srcpitch * 2 + 16];
	v2 = *(GSVector4i*)&src[srcpitch * 3 +  0];
	v3 = *(GSVector4i*)&src[srcpitch * 3 + 16];

	GSVector4i::sw64(v0, v2, v1, v3);

	d[4] = v0;
	d[5] = v1;
	d[6] = v2;
	d[7] = v3;

	src += srcpitch * 4;

	v0 = *(GSVector4i*)&src[srcpitch * 0 +  0];
	v1 = *(GSVector4i*)&src[srcpitch * 0 + 16];
	v2 = *(GSVector4i*)&src[srcpitch * 1 +  0];
	v3 = *(GSVector4i*)&src[srcpitch * 1 + 16];

	GSVector4i::sw64(v0, v2, v1, v3);

	d[8] = v0;
	d[9] = v1;
	d[10] = v2;
	d[11] = v3;

	v0 = *(GSVector4i*)&src[srcpitch * 2 +  0];
	v1 = *(GSVector4i*)&src[srcpitch * 2 + 16];
	v2 = *(GSVector4i*)&src[srcpitch * 3 +  0];
	v3 = *(GSVector4i*)&src[srcpitch * 3 + 16];

	GSVector4i::sw64(v0, v2, v1, v3);

	d[12] = v0;
	d[13] = v1;
	d[14] = v2;
	d[15] = v3;

#else

	const DWORD* d = &columnTable32[0][0];

	for(int j = 0; j < 8; j++, d += 8, src += srcpitch)
	{
		for(int i = 0; i < 8; i++)
		{
			((DWORD*)dst)[d[i]] = ((DWORD*)src)[i];
		}
	}

#endif
}

void GSLocalMemory::SwizzleBlock32(BYTE* dst, BYTE* src, int srcpitch, DWORD mask)
{
#if _M_SSE >= 0x200

	GSVector4i* d = (GSVector4i*)dst;

	GSVector4i vm((int)mask);
	
	GSVector4i v0, v1, v2, v3;

	v0 = *(GSVector4i*)&src[srcpitch * 0 +  0];
	v1 = *(GSVector4i*)&src[srcpitch * 0 + 16];
	v2 = *(GSVector4i*)&src[srcpitch * 1 +  0];
	v3 = *(GSVector4i*)&src[srcpitch * 1 + 16];

	GSVector4i::sw64(v0, v2, v1, v3);

	d[0] = d[0].blend(v0, vm);
	d[1] = d[1].blend(v1, vm);
	d[2] = d[2].blend(v2, vm);
	d[3] = d[3].blend(v3, vm);

	v0 = *(GSVector4i*)&src[srcpitch * 2 +  0];
	v1 = *(GSVector4i*)&src[srcpitch * 2 + 16];
	v2 = *(GSVector4i*)&src[srcpitch * 3 +  0];
	v3 = *(GSVector4i*)&src[srcpitch * 3 + 16];

	GSVector4i::sw64(v0, v2, v1, v3);

	d[4] = d[4].blend(v0, vm);
	d[5] = d[5].blend(v1, vm);
	d[6] = d[6].blend(v2, vm);
	d[7] = d[7].blend(v3, vm);

	src += srcpitch * 4;

	v0 = *(GSVector4i*)&src[srcpitch * 0 +  0];
	v1 = *(GSVector4i*)&src[srcpitch * 0 + 16];
	v2 = *(GSVector4i*)&src[srcpitch * 1 +  0];
	v3 = *(GSVector4i*)&src[srcpitch * 1 + 16];

	GSVector4i::sw64(v0, v2, v1, v3);

	d[8] = d[8].blend(v0, vm);
	d[9] = d[9].blend(v1, vm);
	d[10] = d[10].blend(v2, vm);
	d[11] = d[11].blend(v3, vm);

	v0 = *(GSVector4i*)&src[srcpitch * 2 +  0];
	v1 = *(GSVector4i*)&src[srcpitch * 2 + 16];
	v2 = *(GSVector4i*)&src[srcpitch * 3 +  0];
	v3 = *(GSVector4i*)&src[srcpitch * 3 + 16];

	GSVector4i::sw64(v0, v2, v1, v3);

	d[12] = d[12].blend(v0, vm);
	d[13] = d[13].blend(v1, vm);
	d[14] = d[14].blend(v2, vm);
	d[15] = d[15].blend(v3, vm);

#else

	const DWORD* d = &columnTable32[0][0];

	for(int j = 0; j < 8; j++, d += 8, src += srcpitch)
	{
		for(int i = 0; i < 8; i++)
		{
			((DWORD*)dst)[d[i]] = (((DWORD*)dst)[d[i]] & ~mask) | (((DWORD*)src)[i] & mask);
		}
	}

#endif
}

void GSLocalMemory::SwizzleBlock32u(BYTE* dst, BYTE* src, int srcpitch)
{
#if _M_SSE >= 0x200

	GSVector4i* d = (GSVector4i*)dst;
	
	GSVector4i v0, v1, v2, v3;

	v0 = GSVector4i::loadu(&src[srcpitch * 0 +  0]);
	v1 = GSVector4i::loadu(&src[srcpitch * 0 + 16]);
	v2 = GSVector4i::loadu(&src[srcpitch * 1 +  0]);
	v3 = GSVector4i::loadu(&src[srcpitch * 1 + 16]);

	GSVector4i::sw64(v0, v2, v1, v3);

	d[0] = v0;
	d[1] = v1;
	d[2] = v2;
	d[3] = v3;

	v0 = GSVector4i::loadu(&src[srcpitch * 2 +  0]);
	v1 = GSVector4i::loadu(&src[srcpitch * 2 + 16]);
	v2 = GSVector4i::loadu(&src[srcpitch * 3 +  0]);
	v3 = GSVector4i::loadu(&src[srcpitch * 3 + 16]);

	GSVector4i::sw64(v0, v2, v1, v3);

	d[4] = v0;
	d[5] = v1;
	d[6] = v2;
	d[7] = v3;

	src += srcpitch * 4;

	v0 = GSVector4i::loadu(&src[srcpitch * 0 +  0]);
	v1 = GSVector4i::loadu(&src[srcpitch * 0 + 16]);
	v2 = GSVector4i::loadu(&src[srcpitch * 1 +  0]);
	v3 = GSVector4i::loadu(&src[srcpitch * 1 + 16]);

	GSVector4i::sw64(v0, v2, v1, v3);

	d[8] = v0;
	d[9] = v1;
	d[10] = v2;
	d[11] = v3;

	v0 = GSVector4i::loadu(&src[srcpitch * 2 +  0]);
	v1 = GSVector4i::loadu(&src[srcpitch * 2 + 16]);
	v2 = GSVector4i::loadu(&src[srcpitch * 3 +  0]);
	v3 = GSVector4i::loadu(&src[srcpitch * 3 + 16]);

	GSVector4i::sw64(v0, v2, v1, v3);

	d[12] = v0;
	d[13] = v1;
	d[14] = v2;
	d[15] = v3;

#else

	SwizzleBlock32(dst, src, srcpitch);

#endif
}

void GSLocalMemory::SwizzleBlock32u(BYTE* dst, BYTE* src, int srcpitch, DWORD mask)
{
#if _M_SSE >= 0x200

	GSVector4i* d = (GSVector4i*)dst;

	GSVector4i vm((int)mask);
	
	GSVector4i v0, v1, v2, v3;

	v0 = GSVector4i::loadu(&src[srcpitch * 0 +  0]);
	v1 = GSVector4i::loadu(&src[srcpitch * 0 + 16]);
	v2 = GSVector4i::loadu(&src[srcpitch * 1 +  0]);
	v3 = GSVector4i::loadu(&src[srcpitch * 1 + 16]);

	GSVector4i::sw64(v0, v2, v1, v3);

	d[0] = d[0].blend(v0, vm);
	d[1] = d[1].blend(v1, vm);
	d[2] = d[2].blend(v2, vm);
	d[3] = d[3].blend(v3, vm);

	v0 = GSVector4i::loadu(&src[srcpitch * 2 +  0]);
	v1 = GSVector4i::loadu(&src[srcpitch * 2 + 16]);
	v2 = GSVector4i::loadu(&src[srcpitch * 3 +  0]);
	v3 = GSVector4i::loadu(&src[srcpitch * 3 + 16]);

	GSVector4i::sw64(v0, v2, v1, v3);

	d[4] = d[4].blend(v0, vm);
	d[5] = d[5].blend(v1, vm);
	d[6] = d[6].blend(v2, vm);
	d[7] = d[7].blend(v3, vm);

	src += srcpitch * 4;

	v0 = GSVector4i::loadu(&src[srcpitch * 0 +  0]);
	v1 = GSVector4i::loadu(&src[srcpitch * 0 + 16]);
	v2 = GSVector4i::loadu(&src[srcpitch * 1 +  0]);
	v3 = GSVector4i::loadu(&src[srcpitch * 1 + 16]);

	GSVector4i::sw64(v0, v2, v1, v3);

	d[8] = d[8].blend(v0, vm);
	d[9] = d[9].blend(v1, vm);
	d[10] = d[10].blend(v2, vm);
	d[11] = d[11].blend(v3, vm);

	v0 = GSVector4i::loadu(&src[srcpitch * 2 +  0]);
	v1 = GSVector4i::loadu(&src[srcpitch * 2 + 16]);
	v2 = GSVector4i::loadu(&src[srcpitch * 3 +  0]);
	v3 = GSVector4i::loadu(&src[srcpitch * 3 + 16]);

	GSVector4i::sw64(v0, v2, v1, v3);

	d[12] = d[12].blend(v0, vm);
	d[13] = d[13].blend(v1, vm);
	d[14] = d[14].blend(v2, vm);
	d[15] = d[15].blend(v3, vm);

#else

	SwizzleBlock32(dst, src, srcpitch, mask);

#endif
}

void GSLocalMemory::SwizzleBlock16(BYTE* dst, BYTE* src, int srcpitch)
{
#if _M_SSE >= 0x200

	GSVector4i* d = (GSVector4i*)dst;
	
	GSVector4i v0, v1, v2, v3;

	for(int i = 0; i < 4; i++)
	{
		v0 = *(GSVector4i*)&src[srcpitch * (i * 2 + 0) +  0];
		v1 = *(GSVector4i*)&src[srcpitch * (i * 2 + 0) + 16];
		v2 = *(GSVector4i*)&src[srcpitch * (i * 2 + 1) +  0];
		v3 = *(GSVector4i*)&src[srcpitch * (i * 2 + 1) + 16];

		GSVector4i::sw16(v0, v1, v2, v3);
		GSVector4i::sw64(v0, v1, v2, v3);

		d[i * 4 + 0] = v0;
		d[i * 4 + 1] = v2;
		d[i * 4 + 2] = v1;
		d[i * 4 + 3] = v3;
	}

#else

	const DWORD* d = &columnTable16[0][0];

	for(int j = 0; j < 8; j++, d += 16, src += srcpitch)
	{
		for(int i = 0; i < 16; i++)
		{
			((WORD*)dst)[d[i]] = ((WORD*)src)[i];
		}
	}

#endif
}

void GSLocalMemory::SwizzleBlock16u(BYTE* dst, BYTE* src, int srcpitch)
{
#if _M_SSE >= 0x200

	GSVector4i* d = (GSVector4i*)dst;
	
	GSVector4i v0, v1, v2, v3;

	for(int i = 0; i < 4; i++)
	{
		v0 = GSVector4i::loadu(&src[srcpitch * (i * 2 + 0) +  0]);
		v1 = GSVector4i::loadu(&src[srcpitch * (i * 2 + 0) + 16]);
		v2 = GSVector4i::loadu(&src[srcpitch * (i * 2 + 1) +  0]);
		v3 = GSVector4i::loadu(&src[srcpitch * (i * 2 + 1) + 16]);

		GSVector4i::sw16(v0, v1, v2, v3);
		GSVector4i::sw64(v0, v1, v2, v3);

		d[i * 4 + 0] = v0;
		d[i * 4 + 1] = v2;
		d[i * 4 + 2] = v1;
		d[i * 4 + 3] = v3;
	}

#else

	SwizzleBlock16(dst, src, srcpitch);

#endif
}

void GSLocalMemory::SwizzleBlock8(BYTE* dst, BYTE* src, int srcpitch)
{
#if _M_SSE >= 0x200

	GSVector4i* d = (GSVector4i*)dst;

	GSVector4i v0, v1, v2, v3;

	for(int i = 0; i < 2; i++)
	{
		v0 = *(GSVector4i*)&src[srcpitch * 0]; 
		v1 = *(GSVector4i*)&src[srcpitch * 1];
		v2 = ((GSVector4i*)&src[srcpitch * 2])->yxwz(); 
		v3 = ((GSVector4i*)&src[srcpitch * 3])->yxwz();

		GSVector4i::sw8(v0, v2, v1, v3);
		GSVector4i::sw16(v0, v1, v2, v3);
		GSVector4i::sw64(v0, v1, v2, v3);

		d[i * 8 + 0] = v0;
		d[i * 8 + 1] = v2;
		d[i * 8 + 2] = v1;
		d[i * 8 + 3] = v3;

		src += srcpitch * 4;

		v0 = ((GSVector4i*)&src[srcpitch * 0])->yxwz(); 
		v1 = ((GSVector4i*)&src[srcpitch * 1])->yxwz(); 
		v2 = *(GSVector4i*)&src[srcpitch * 2]; 
		v3 = *(GSVector4i*)&src[srcpitch * 3];

		GSVector4i::sw8(v0, v2, v1, v3);
		GSVector4i::sw16(v0, v1, v2, v3);
		GSVector4i::sw64(v0, v1, v2, v3);

		d[i * 8 + 4] = v0;
		d[i * 8 + 5] = v2;
		d[i * 8 + 6] = v1;
		d[i * 8 + 7] = v3;

		src += srcpitch * 4;
	}

#else

	const DWORD* d = &columnTable8[0][0];

	for(int j = 0; j < 16; j++, d += 16, src += srcpitch)
	{
		for(int i = 0; i < 16; i++)
		{
			dst[d[i]] = src[i];
		}
	}

#endif
}

void GSLocalMemory::SwizzleBlock8u(BYTE* dst, BYTE* src, int srcpitch)
{
#if _M_SSE >= 0x200

	GSVector4i* d = (GSVector4i*)dst;

	GSVector4i v0, v1, v2, v3;

	for(int i = 0; i < 2; i++)
	{
		v0 = GSVector4i::loadu(&src[srcpitch * 0]); 
		v1 = GSVector4i::loadu(&src[srcpitch * 1]);
		v2 = GSVector4i::loadu(&src[srcpitch * 2]).yxwz(); 
		v3 = GSVector4i::loadu(&src[srcpitch * 3]).yxwz();

		GSVector4i::sw8(v0, v2, v1, v3);
		GSVector4i::sw16(v0, v1, v2, v3);
		GSVector4i::sw64(v0, v1, v2, v3);

		d[i * 8 + 0] = v0;
		d[i * 8 + 1] = v2;
		d[i * 8 + 2] = v1;
		d[i * 8 + 3] = v3;

		src += srcpitch * 4;

		v0 = GSVector4i::loadu(&src[srcpitch * 0]).yxwz(); 
		v1 = GSVector4i::loadu(&src[srcpitch * 1]).yxwz(); 
		v2 = GSVector4i::loadu(&src[srcpitch * 2]); 
		v3 = GSVector4i::loadu(&src[srcpitch * 3]);

		GSVector4i::sw8(v0, v2, v1, v3);
		GSVector4i::sw16(v0, v1, v2, v3);
		GSVector4i::sw64(v0, v1, v2, v3);

		d[i * 8 + 4] = v0;
		d[i * 8 + 5] = v2;
		d[i * 8 + 6] = v1;
		d[i * 8 + 7] = v3;

		src += srcpitch * 4;
	}

#else

	SwizzleBlock8(dst, src, srcpitch);

#endif
}

void GSLocalMemory::SwizzleBlock4(BYTE* dst, BYTE* src, int srcpitch)
{
#if _M_SSE >= 0x200

	GSVector4i* d = (GSVector4i*)dst;

	GSVector4i v0, v1, v2, v3;

	for(int i = 0; i < 2; i++)
	{
		v0 = *(GSVector4i*)&src[srcpitch * 0]; 
		v1 = *(GSVector4i*)&src[srcpitch * 1];
		v2 = ((GSVector4i*)&src[srcpitch * 2])->yxwzl().yxwzh(); 
		v3 = ((GSVector4i*)&src[srcpitch * 3])->yxwzl().yxwzh();

		GSVector4i::sw4(v0, v2, v1, v3);
		GSVector4i::sw8(v0, v1, v2, v3);
		GSVector4i::sw8(v0, v2, v1, v3);
		GSVector4i::sw64(v0, v2, v1, v3);

		d[i * 8 + 0] = v0;
		d[i * 8 + 1] = v1;
		d[i * 8 + 2] = v2;
		d[i * 8 + 3] = v3;

		src += srcpitch * 4;

		v0 = ((GSVector4i*)&src[srcpitch * 0])->yxwzl().yxwzh(); 
		v1 = ((GSVector4i*)&src[srcpitch * 1])->yxwzl().yxwzh(); 
		v2 = *(GSVector4i*)&src[srcpitch * 2]; 
		v3 = *(GSVector4i*)&src[srcpitch * 3];

		GSVector4i::sw4(v0, v2, v1, v3);
		GSVector4i::sw8(v0, v1, v2, v3);
		GSVector4i::sw8(v0, v2, v1, v3);
		GSVector4i::sw64(v0, v2, v1, v3);

		d[i * 8 + 4] = v0;
		d[i * 8 + 5] = v1;
		d[i * 8 + 6] = v2;
		d[i * 8 + 7] = v3;

		src += srcpitch * 4;
	}

#else

	const DWORD* d = &columnTable4[0][0];

	for(int j = 0; j < 16; j++, d += 32, src += srcpitch)
	{
		for(int i = 0; i < 32; i++)
		{
			DWORD addr = d[i];
			BYTE c = (src[i >> 1] >> ((i & 1) << 2)) & 0x0f;
			DWORD shift = (addr & 1) << 2;
			dst[addr >> 1] = (dst[addr >> 1] & (0xf0 >> shift)) | (c << shift);
		}
	}

#endif
}

void GSLocalMemory::SwizzleBlock4u(BYTE* dst, BYTE* src, int srcpitch)
{
#if _M_SSE >= 0x200

	GSVector4i* d = (GSVector4i*)dst;

	GSVector4i v0, v1, v2, v3;

	for(int i = 0; i < 2; i++)
	{
		v0 = GSVector4i::loadu(&src[srcpitch * 0]); 
		v1 = GSVector4i::loadu(&src[srcpitch * 1]);
		v2 = GSVector4i::loadu(&src[srcpitch * 2]).yxwzl().yxwzh(); 
		v3 = GSVector4i::loadu(&src[srcpitch * 3]).yxwzl().yxwzh();

		GSVector4i::sw4(v0, v2, v1, v3);
		GSVector4i::sw8(v0, v1, v2, v3);
		GSVector4i::sw8(v0, v2, v1, v3);
		GSVector4i::sw64(v0, v2, v1, v3);

		d[i * 8 + 0] = v0;
		d[i * 8 + 1] = v1;
		d[i * 8 + 2] = v2;
		d[i * 8 + 3] = v3;

		src += srcpitch * 4;

		v0 = GSVector4i::loadu(&src[srcpitch * 0]).yxwzl().yxwzh(); 
		v1 = GSVector4i::loadu(&src[srcpitch * 1]).yxwzl().yxwzh();
		v2 = GSVector4i::loadu(&src[srcpitch * 2]); 
		v3 = GSVector4i::loadu(&src[srcpitch * 3]);

		GSVector4i::sw4(v0, v2, v1, v3);
		GSVector4i::sw8(v0, v1, v2, v3);
		GSVector4i::sw8(v0, v2, v1, v3);
		GSVector4i::sw64(v0, v2, v1, v3);

		d[i * 8 + 4] = v0;
		d[i * 8 + 5] = v1;
		d[i * 8 + 6] = v2;
		d[i * 8 + 7] = v3;

		src += srcpitch * 4;
	}

#else

	SwizzleBlock4(dst, src, srcpitch);

#endif
}

void GSLocalMemory::SwizzleColumn32(int y, BYTE* dst, BYTE* src, int srcpitch)
{
	const DWORD* d = &columnTable32[y & (3 << 1)][0];

	for(int j = 0; j < 2; j++, d += 8, src += srcpitch)
	{
		for(int i = 0; i < 8; i++)
		{
			((DWORD*)dst)[d[i]] = ((DWORD*)src)[i];
		}
	}
}

void GSLocalMemory::SwizzleColumn32(int y, BYTE* dst, BYTE* src, int srcpitch, DWORD mask)
{
	const DWORD* d = &columnTable32[y & (3 << 1)][0];

	for(int j = 0; j < 2; j++, d += 8, src += srcpitch)
	{
		for(int i = 0; i < 8; i++)
		{
			((DWORD*)dst)[d[i]] = (((DWORD*)dst)[d[i]] & ~mask) | (((DWORD*)src)[i] & mask);
		}
	}
}

void GSLocalMemory::SwizzleColumn16(int y, BYTE* dst, BYTE* src, int srcpitch)
{
	const DWORD* d = &columnTable16[y & (3 << 1)][0];

	for(int j = 0; j < 2; j++, d += 16, src += srcpitch)
	{
		for(int i = 0; i < 16; i++)
		{
			((WORD*)dst)[d[i]] = ((WORD*)src)[i];
		}
	}
}

void GSLocalMemory::SwizzleColumn8(int y, BYTE* dst, BYTE* src, int srcpitch)
{
	const DWORD* d = &columnTable8[y & (3 << 2)][0];

	for(int j = 0; j < 4; j++, d += 16, src += srcpitch)
	{
		for(int i = 0; i < 16; i++)
		{
			dst[d[i]] = src[i];
		}
	}
}

void GSLocalMemory::SwizzleColumn4(int y, BYTE* dst, BYTE* src, int srcpitch)
{
	const DWORD* d = &columnTable4[y & (3 << 2)][0];

	for(int j = 0; j < 4; j++, d += 32, src += srcpitch)
	{
		for(int i = 0; i < 32; i++)
		{
			DWORD addr = d[i];
			BYTE c = (src[i >> 1] >> ((i & 1) << 2)) & 0x0f;
			DWORD shift = (addr & 1) << 2;
			dst[addr >> 1] = (dst[addr >> 1] & (0xf0 >> shift)) | (c << shift);
		}
	}
}

//

__forceinline static void WriteCLUT_T32_I4_CSM1(GSVector4i* s, GSVector4i* d)
{
	GSVector4i r0, r1, r2, r3;
	GSVector4i r4, r5, r6, r7;

	r0 = s[0].upl64(s[1]);
	r1 = s[2].upl64(s[3]);
	r2 = s[0].uph64(s[1]);
	r3 = s[2].uph64(s[3]);

	r4 = r0.upl16(r1);
	r5 = r0.uph16(r1);
	r6 = r2.upl16(r3);
	r7 = r2.uph16(r3);

	r0 = r4.upl16(r5);
	r1 = r4.uph16(r5);
	r2 = r6.upl16(r7);
	r3 = r6.uph16(r7);

	d[0] = r0.upl16(r1);
	d[1] = r2.upl16(r3);
	d[32] = r0.uph16(r1);
	d[33] = r2.uph16(r3);
}

void GSLocalMemory::WriteCLUT_T32_I8_CSM1(DWORD* vm, WORD* clut)
{
#if _M_SSE >= 0x200

	GSVector4i* s = (GSVector4i*)vm;
	GSVector4i* d = (GSVector4i*)clut;

	for(int i = 0; i < 16; i += 4)
	{
		::WriteCLUT_T32_I4_CSM1(&s[i +  0], &d[i +  0]);
		::WriteCLUT_T32_I4_CSM1(&s[i + 16], &d[i +  2]);
		::WriteCLUT_T32_I4_CSM1(&s[i + 32], &d[i + 16]);
		::WriteCLUT_T32_I4_CSM1(&s[i + 48], &d[i + 18]);
	}

#else

	for(int j = 0; j < 2; j++, vm += 128, clut += 128)
	{
		for(int i = 0; i < 128; i++) 
		{
			DWORD dw = vm[clutTableT32I8[i]];
			clut[i] = (WORD)(dw & 0xffff);
			clut[i + 256] = (WORD)(dw >> 16);
		}
	}

#endif
}

void GSLocalMemory::WriteCLUT_T32_I4_CSM1(DWORD* vm, WORD* clut)
{
#if _M_SSE >= 0x200

	GSVector4i* s = (GSVector4i*)vm;
	GSVector4i* d = (GSVector4i*)clut;

	::WriteCLUT_T32_I4_CSM1(s, d);

#else

	for(int i = 0; i < 16; i++) 
	{
		DWORD dw = vm[clutTableT32I4[i]];
		clut[i] = (WORD)(dw & 0xffff);
		clut[i + 256] = (WORD)(dw >> 16);
	}

#endif
}

void GSLocalMemory::WriteCLUT_T16_I8_CSM1(WORD* vm, WORD* clut)
{
#if _M_SSE >= 0x200

	GSVector4i* s = (GSVector4i*)vm;
	GSVector4i* d = (GSVector4i*)clut;

	for(int i = 0; i < 32; i += 4)
	{
		GSVector4i r0, r1, r2, r3;
		GSVector4i r4, r5, r6, r7;

		r0 = s[i+0].upl16(s[i+1]);
		r1 = s[i+2].upl16(s[i+3]);
		r2 = s[i+0].uph16(s[i+1]);
		r3 = s[i+2].uph16(s[i+3]);

		r4 = r0.upl32(r1);
		r5 = r0.uph32(r1);
		r6 = r2.upl32(r3);
		r7 = r2.uph32(r3);

		d[i+0] = r4.upl16(r5);
		d[i+1] = r6.upl16(r7);
		d[i+2] = r4.uph16(r5);
		d[i+3] = r6.uph16(r7);
	}

#else

	for(int j = 0; j < 8; j++, vm += 32, clut += 32) 
	{
		for(int i = 0; i < 32; i++)
		{
			clut[i] = vm[clutTableT16I8[i]];
		}
	}

#endif
}

void GSLocalMemory::WriteCLUT_T16_I4_CSM1(WORD* vm, WORD* clut)
{
	for(int i = 0; i < 16; i++) 
	{
		clut[i] = vm[clutTableT16I4[i]];
	}
}

//

void GSLocalMemory::ReadCLUT32_T32_I8(WORD* src, DWORD* dst)
{
#if _M_SSE >= 0x200

	for(int i = 0; i < 256; i += 16)
	{
		ReadCLUT32_T32_I4(&src[i], &dst[i]); // going to be inlined nicely
	}

#else 

	for(int i = 0; i < 256; i++)
	{
		dst[i] = ((DWORD)src[i + 256] << 16) | src[i];
	}

#endif
}

void GSLocalMemory::ReadCLUT32_T32_I4(WORD* src, DWORD* dst)
{
#if _M_SSE >= 0x200

	GSVector4i* s = (GSVector4i*)src;
	GSVector4i* d = (GSVector4i*)dst;

	GSVector4i r0 = s[0];
	GSVector4i r1 = s[1];
	GSVector4i r2 = s[32];
	GSVector4i r3 = s[33];

	d[0] = r0.upl16(r2);
	d[1] = r0.uph16(r2);
	d[2] = r1.upl16(r3);
	d[3] = r1.uph16(r3);

#else 

	for(int i = 0; i < 16; i++)
	{
		dst[i] = ((DWORD)src[i + 256] << 16) | src[i];
	}

#endif
}

void GSLocalMemory::ReadCLUT32_T16_I8(WORD* src, DWORD* dst)
{
#if _M_SSE >= 0x200

	for(int i = 0; i < 256; i += 16)
	{
		ReadCLUT32_T16_I4(&src[i], &dst[i]);
	}

#else 

	for(int i = 0; i < 256; i++)
	{
		dst[i] = (DWORD)src[i];
	}

#endif
}

void GSLocalMemory::ReadCLUT32_T16_I4(WORD* src, DWORD* dst)
{
#if _M_SSE >= 0x200

	GSVector4i* s = (GSVector4i*)src;
	GSVector4i* d = (GSVector4i*)dst;

	GSVector4i r0 = s[0];
	GSVector4i r1 = s[1];

	d[0] = r0.upl16();
	d[1] = r0.uph16();
	d[2] = r1.upl16();
	d[3] = r1.uph16();

#else 

	for(int i = 0; i < 16; i++)
	{
		dst[i] = (DWORD)src[i];
	}

#endif
}

//

static const GSVector4i s_rgbm(0x00ffffff);
static const GSVector4i s_am(0x00008000);
static const GSVector4i s_bm(0x00007c00);
static const GSVector4i s_gm(0x000003e0);
static const GSVector4i s_rm(0x0000001f);

void GSLocalMemory::ExpandBlock24(BYTE* src, int srcpitch, DWORD* dst)
{
#if _M_SSE >= 0x200

	const GSVector4i rgbm = s_rgbm;

	GSVector4i* d = (GSVector4i*)dst;

	for(int i = 0; i < 8; i += 2, src += srcpitch * 2, d += 4)
	{
		GSVector4i r0 = GSVector4i::loadu(src);
		GSVector4i r1 = GSVector4i::loadu(src + 16, src + srcpitch);
		GSVector4i r2 = GSVector4i::loadu(src + srcpitch + 8);

		d[0] = r0.upl32(r0.srl<3>()).upl64(r0.srl<6>().upl32(r0.srl<9>())) & rgbm;

		r0 = r0.srl<12>() | r1.sll<4>();

		d[1] = r0.upl32(r0.srl<3>()).upl64(r0.srl<6>().upl32(r0.srl<9>())) & rgbm;

		r0 = r1.srl<8>() | r2.sll<8>();

		d[2] = r0.upl32(r0.srl<3>()).upl64(r0.srl<6>().upl32(r0.srl<9>())) & rgbm;

		r0 = r2.srl<4>();

		d[3] = r0.upl32(r0.srl<3>()).upl64(r0.srl<6>().upl32(r0.srl<9>())) & rgbm;
	}

#else 

	for(int j = 0, diff = srcpitch - 8*3; j < 8; j++, src += diff, dst += 8)
	{
		for(int i = 0; i < 8; i++, src += 3)
		{
			dst[i] = (src[2] << 16) | (src[1] << 8) | src[0];
		}
	}

#endif
}

void GSLocalMemory::ExpandBlock8H(BYTE* src, int srcpitch, DWORD* dst)
{
#if _M_SSE >= 0x200

	GSVector4i* d = (GSVector4i*)dst;
	GSVector4i zero = GSVector4i::zero();

	for(int i = 0; i < 8; i += 2)
	{
		GSVector4i v = GSVector4i::loadu(
			&src[(i + 0) * srcpitch],
			&src[(i + 1) * srcpitch]);

		GSVector4i v0 = zero.upl8(v);
		GSVector4i v1 = zero.uph8(v);

		d[i * 2 + 0] = zero.upl16(v0);
		d[i * 2 + 1] = zero.uph16(v0);
		d[i * 2 + 2] = zero.upl16(v1);
		d[i * 2 + 3] = zero.uph16(v1);
	}

#else

	for(int j = 0; j < 8; j++, src += srcpitch, dst += 8)
	{
		for(int i = 0; i < 8; i++)
		{
			dst[i] = src[i] << 24;
		}
	}

#endif

}

void GSLocalMemory::ExpandBlock4HL(BYTE* src, int srcpitch, DWORD* dst)
{
#if _M_SSE >= 0x200

	GSVector4i* d = (GSVector4i*)dst;
	GSVector4i zero = GSVector4i::zero();
	GSVector4i mask(0x0f0f0f0f);

	for(int i = 0; i < 8; i += 4)
	{
		GSVector4i v(
			*(DWORD*)&src[(i + 0) * srcpitch], 
			*(DWORD*)&src[(i + 1) * srcpitch], 
			*(DWORD*)&src[(i + 2) * srcpitch], 
			*(DWORD*)&src[(i + 3) * srcpitch]);

		GSVector4i lo = v & mask;
		GSVector4i hi = (v >> 4) & mask;

		GSVector4i v0 = lo.upl8(hi);
		GSVector4i v1 = lo.uph8(hi);

		GSVector4i v2 = zero.upl8(v0);
		GSVector4i v3 = zero.uph8(v0);
		GSVector4i v4 = zero.upl8(v1);
		GSVector4i v5 = zero.uph8(v1);

		d[i * 2 + 0] = zero.upl16(v2);
		d[i * 2 + 1] = zero.uph16(v2);
		d[i * 2 + 2] = zero.upl16(v3);
		d[i * 2 + 3] = zero.uph16(v3);
		d[i * 2 + 4] = zero.upl16(v4);
		d[i * 2 + 5] = zero.uph16(v4);
		d[i * 2 + 6] = zero.upl16(v5);
		d[i * 2 + 7] = zero.uph16(v5);
	}

#else

	for(int j = 0; j < 8; j++, src += srcpitch, dst += 8)
	{
		for(int i = 0; i < 4; i++)
		{
			dst[i * 2 + 0] = (src[i] & 0x0f) << 24;
			dst[i * 2 + 1] = (src[i] & 0xf0) << 20;
		}
	}

#endif
}

void GSLocalMemory::ExpandBlock4HH(BYTE* src, int srcpitch, DWORD* dst)
{
#if _M_SSE >= 0x200

	GSVector4i* d = (GSVector4i*)dst;
	GSVector4i zero = GSVector4i::zero();
	GSVector4i mask(0xf0f0f0f0);

	for(int i = 0; i < 8; i += 4)
	{
		GSVector4i v(
			*(DWORD*)&src[(i + 0) * srcpitch], 
			*(DWORD*)&src[(i + 1) * srcpitch], 
			*(DWORD*)&src[(i + 2) * srcpitch], 
			*(DWORD*)&src[(i + 3) * srcpitch]);

		GSVector4i lo = (v << 4) & mask;
		GSVector4i hi = v & mask;

		GSVector4i v0 = lo.upl8(hi);
		GSVector4i v1 = lo.uph8(hi);

		GSVector4i v2 = zero.upl8(v0);
		GSVector4i v3 = zero.uph8(v0);
		GSVector4i v4 = zero.upl8(v1);
		GSVector4i v5 = zero.uph8(v1);

		d[i * 2 + 0] = zero.upl16(v2);
		d[i * 2 + 1] = zero.uph16(v2);
		d[i * 2 + 2] = zero.upl16(v3);
		d[i * 2 + 3] = zero.uph16(v3);
		d[i * 2 + 4] = zero.upl16(v4);
		d[i * 2 + 5] = zero.uph16(v4);
		d[i * 2 + 6] = zero.upl16(v5);
		d[i * 2 + 7] = zero.uph16(v5);
	}

#else

	for(int j = 0; j < 8; j++, src += srcpitch, dst += 8)
	{
		for(int i = 0; i < 4; i++)
		{
			dst[i * 2 + 0] = (src[i] & 0x0f) << 28;
			dst[i * 2 + 1] = (src[i] & 0xf0) << 24;
		}
	}

#endif
}

void GSLocalMemory::ExpandBlock24(DWORD* src, DWORD* dst, int dstpitch, GIFRegTEXA* TEXA)
{
#if _M_SSE >= 0x200

	const GSVector4i rgbm = s_rgbm;

	GSVector4i TA0(TEXA->TA0 << 24);

	GSVector4i c;

	if(!TEXA->AEM)
	{
		for(int j = 0; j < 8; j++, src += 8, dst += dstpitch >> 2)
		{
			GSVector4i* s = (GSVector4i*)src;
			GSVector4i* d = (GSVector4i*)dst;

			d[0] = (s[0] & rgbm) | TA0;
			d[1] = (s[1] & rgbm) | TA0;
		}
	}
	else
	{
		for(int j = 0; j < 8; j++, src += 8, dst += dstpitch >> 2)
		{
			GSVector4i* s = (GSVector4i*)src;
			GSVector4i* d = (GSVector4i*)dst;

			c = s[0] & rgbm;
			d[0] = c | TA0.andnot(c == GSVector4i::zero()); // TA0 & (c != GSVector4i::zero())

			c = s[1] & rgbm;
			d[1] = c | TA0.andnot(c == GSVector4i::zero()); // TA0 & (c != GSVector4i::zero())
		}
	}

#else

	DWORD TA0 = (DWORD)TEXA->TA0 << 24;

	if(!TEXA->AEM)
	{
		for(int j = 0; j < 8; j++, src += 8, dst += dstpitch >> 2)
		{
			for(int i = 0; i < 8; i++)
			{
				dst[i] = TA0 | (src[i] & 0xffffff);
			}
		}
	}
	else
	{
		for(int j = 0; j < 8; j++, src += 8, dst += dstpitch >> 2)
		{
			for(int i = 0; i < 8; i++)
			{
				dst[i] = ((src[i] & 0xffffff) ? TA0 : 0) | (src[i] & 0xffffff);
			}
		}
	}

#endif
}

void GSLocalMemory::ExpandBlock16(WORD* src, DWORD* dst, int dstpitch, GIFRegTEXA* TEXA)
{
#if _M_SSE >= 0x200

	const GSVector4i rm = s_rm;
	const GSVector4i gm = s_gm;
	const GSVector4i bm = s_bm;
	const GSVector4i am = s_am;

	GSVector4i TA0(TEXA->TA0 << 24);
	GSVector4i TA1(TEXA->TA1 << 24);

	GSVector4i c, cl, ch;

	if(!TEXA->AEM)
	{
		for(int j = 0; j < 8; j++, src += 16, dst += dstpitch >> 2)
		{
			GSVector4i* s = (GSVector4i*)src;
			GSVector4i* d = (GSVector4i*)dst;

			c = s[0];
			cl = c.upl16();
			ch = c.uph16();
			d[0] = ((cl & rm) << 3) | ((cl & gm) << 6) | ((cl & bm) << 9) | TA1.blend(TA0, cl < am);
			d[1] = ((ch & rm) << 3) | ((ch & gm) << 6) | ((ch & bm) << 9) | TA1.blend(TA0, ch < am);

			c = s[1];
			cl = c.upl16();
			ch = c.uph16();
			d[2] = ((cl & rm) << 3) | ((cl & gm) << 6) | ((cl & bm) << 9) | TA1.blend(TA0, cl < am);
			d[3] = ((ch & rm) << 3) | ((ch & gm) << 6) | ((ch & bm) << 9) | TA1.blend(TA0, ch < am);
		}
	}
	else
	{
		for(int j = 0; j < 8; j++, src += 16, dst += dstpitch >> 2)
		{
			GSVector4i* s = (GSVector4i*)src;
			GSVector4i* d = (GSVector4i*)dst;

			c = s[0];
			cl = c.upl16();
			ch = c.uph16();
			d[0] = ((cl & rm) << 3) | ((cl & gm) << 6) | ((cl & bm) << 9) | TA1.blend(TA0, cl < am).andnot(cl == GSVector4i::zero());
			d[1] = ((ch & rm) << 3) | ((ch & gm) << 6) | ((ch & bm) << 9) | TA1.blend(TA0, ch < am).andnot(ch == GSVector4i::zero());

			c = s[1];
			cl = c.upl16();
			ch = c.uph16();
			d[2] = ((cl & rm) << 3) | ((cl & gm) << 6) | ((cl & bm) << 9) | TA1.blend(TA0, cl < am).andnot(cl == GSVector4i::zero());
			d[3] = ((ch & rm) << 3) | ((ch & gm) << 6) | ((ch & bm) << 9) | TA1.blend(TA0, ch < am).andnot(ch == GSVector4i::zero());
		}
	}

#else

	DWORD TA0 = (DWORD)TEXA->TA0 << 24;
	DWORD TA1 = (DWORD)TEXA->TA1 << 24;

	if(!TEXA->AEM)
	{
		for(int j = 0; j < 8; j++, src += 16, dst += dstpitch >> 2)
		{
			for(int i = 0; i < 16; i++)
			{
				dst[i] = ((src[i] & 0x8000) ? TA1 : TA0) | ((src[i] & 0x7c00) << 9) | ((src[i] & 0x03e0) << 6) | ((src[i] & 0x001f) << 3);
			}
		}
	}
	else
	{
		for(int j = 0; j < 8; j++, src += 16, dst += dstpitch >> 2)
		{
			for(int i = 0; i < 16; i++)
			{
				dst[i] = ((src[i] & 0x8000) ? TA1 : src[i] ? TA0 : 0) | ((src[i] & 0x7c00) << 9) | ((src[i] & 0x03e0) << 6) | ((src[i] & 0x001f) << 3);
			}
		}
	}

#endif
}

void GSLocalMemory::Expand16(WORD* src, DWORD* dst, int w, GIFRegTEXA* TEXA)
{
#if _M_SSE >= 0x200

	ASSERT(!(w&7));

	const GSVector4i rm = s_rm;
	const GSVector4i gm = s_gm;
	const GSVector4i bm = s_bm;
	const GSVector4i am = s_am;

	GSVector4i TA0(TEXA->TA0 << 24);
	GSVector4i TA1(TEXA->TA1 << 24);

	GSVector4i c, cl, ch;

	GSVector4i* s = (GSVector4i*)src;
	GSVector4i* d = (GSVector4i*)dst;

	if(!TEXA->AEM)
	{
		for(int i = 0, j = w >> 3; i < j; i++)
		{
			c = s[i];
			cl = c.upl16();
			ch = c.uph16();
			d[i * 2 + 0] = ((cl & rm) << 3) | ((cl & gm) << 6) | ((cl & bm) << 9) | TA1.blend(TA0, cl < am);
			d[i * 2 + 1] = ((ch & rm) << 3) | ((ch & gm) << 6) | ((ch & bm) << 9) | TA1.blend(TA0, ch < am);
		}
	}
	else
	{
		for(int i = 0, j = w >> 3; i < j; i++)
		{
			c = s[i];
			cl = c.upl16();
			ch = c.uph16();
			d[i * 2 + 0] = ((cl & rm) << 3) | ((cl & gm) << 6) | ((cl & bm) << 9) | TA1.blend(TA0, cl < am).andnot(cl == GSVector4i::zero());
			d[i * 2 + 1] = ((ch & rm) << 3) | ((ch & gm) << 6) | ((ch & bm) << 9) | TA1.blend(TA0, ch < am).andnot(ch == GSVector4i::zero());
		}
	}

#else

	DWORD TA0 = (DWORD)TEXA->TA0 << 24;
	DWORD TA1 = (DWORD)TEXA->TA1 << 24;

	if(!TEXA->AEM)
	{
		for(int i = 0; i < w; i++)
		{
			dst[i] = ((src[i] & 0x8000) ? TA1 : TA0) | ((src[i] & 0x7c00) << 9) | ((src[i] & 0x03e0) << 6) | ((src[i] & 0x001f) << 3);
		}
	}
	else
	{
		for(int i = 0; i < w; i++)
		{
			dst[i] = ((src[i] & 0x8000) ? TA1 : src[i] ? TA0 : 0) | ((src[i] & 0x7c00) << 9) | ((src[i] & 0x03e0) << 6) | ((src[i] & 0x001f) << 3);
		}
	}

#endif
}

//

HRESULT GSLocalMemory::SaveBMP(LPCTSTR fn, DWORD bp, DWORD bw, DWORD psm, int w, int h)
{
	int pitch = w * 4;
	int size = pitch * h;
	void* bits = ::_aligned_malloc(size, 16);

	GIFRegTEX0 TEX0;

	TEX0.TBP0 = bp;
	TEX0.TBW = bw;
	TEX0.PSM = psm;

	GIFRegTEXA TEXA;

	TEXA.AEM = 0;
	TEXA.TA0 = 0;
	TEXA.TA1 = 0x80;

	// (this->*m_psm[TEX0.PSM].ust)(CRect(0, 0, w, h), bits, pitch, TEX0, TEXA);

	readPixel rp = m_psm[psm].rp;

	BYTE* p = (BYTE*)bits;

	for(int j = h-1; j >= 0; j--, p += pitch)
		for(int i = 0; i < w; i++)
			((DWORD*)p)[i] = (this->*rp)(i, j, TEX0.TBP0, TEX0.TBW);

	if(FILE* fp = _tfopen(fn, _T("wb")))
	{
		BITMAPINFOHEADER bih;
		memset(&bih, 0, sizeof(bih));
        bih.biSize = sizeof(bih);
        bih.biWidth = w;
        bih.biHeight = h;
        bih.biPlanes = 1;
        bih.biBitCount = 32;
        bih.biCompression = BI_RGB;
        bih.biSizeImage = size;

		BITMAPFILEHEADER bfh;
		memset(&bfh, 0, sizeof(bfh));
		bfh.bfType = 'MB';
		bfh.bfOffBits = sizeof(bfh) + sizeof(bih);
		bfh.bfSize = bfh.bfOffBits + size;
		bfh.bfReserved1 = bfh.bfReserved2 = 0;

		fwrite(&bfh, 1, sizeof(bfh), fp);
		fwrite(&bih, 1, sizeof(bih), fp);
		fwrite(bits, 1, size, fp);

		fclose(fp);
	}

	::_aligned_free(bits);

	return true;
}
