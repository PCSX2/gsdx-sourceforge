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
			pageOffset32[bp][y][x] = PixelAddressOrg32(x, y, bp, 0);
			pageOffset32Z[bp][y][x] = PixelAddressOrg32Z(x, y, bp, 0);
		}

		for(int y = 0; y < 64; y++) for(int x = 0; x < 64; x++) 
		{
			pageOffset16[bp][y][x] = PixelAddressOrg16(x, y, bp, 0);
			pageOffset16S[bp][y][x] = PixelAddressOrg16S(x, y, bp, 0);
			pageOffset16Z[bp][y][x] = PixelAddressOrg16Z(x, y, bp, 0);
			pageOffset16SZ[bp][y][x] = PixelAddressOrg16SZ(x, y, bp, 0);
		}

		for(int y = 0; y < 64; y++) for(int x = 0; x < 128; x++)
		{
			pageOffset8[bp][y][x] = PixelAddressOrg8(x, y, bp, 0);
		}

		for(int y = 0; y < 128; y++) for(int x = 0; x < 128; x++)
		{
			pageOffset4[bp][y][x] = PixelAddressOrg4(x, y, bp, 0);
		}
	}

	for(int x = 0; x < countof(rowOffset32); x++)
	{
		rowOffset32[x] = (int)PixelAddress32(x, 0, 0, 32) - (int)PixelAddress32(0, 0, 0, 32);
	}

	for(int x = 0; x < countof(rowOffset32Z); x++)
	{
		rowOffset32Z[x] = (int)PixelAddress32Z(x, 0, 0, 32) - (int)PixelAddress32Z(0, 0, 0, 32);
	}

	for(int x = 0; x < countof(rowOffset16); x++)
	{
		rowOffset16[x] = (int)PixelAddress16(x, 0, 0, 32) - (int)PixelAddress16(0, 0, 0, 32);
	}

	for(int x = 0; x < countof(rowOffset16S); x++)
	{
		rowOffset16S[x] = (int)PixelAddress16S(x, 0, 0, 32) - (int)PixelAddress16S(0, 0, 0, 32);
	}

	for(int x = 0; x < countof(rowOffset16Z); x++)
	{
		rowOffset16Z[x] = (int)PixelAddress16Z(x, 0, 0, 32) - (int)PixelAddress16Z(0, 0, 0, 32);
	}

	for(int x = 0; x < countof(rowOffset16SZ); x++)
	{
		rowOffset16SZ[x] = (int)PixelAddress16SZ(x, 0, 0, 32) - (int)PixelAddress16SZ(0, 0, 0, 32);
	}

	for(int x = 0; x < countof(rowOffset8[0]); x++)
	{
		rowOffset8[0][x] = (int)PixelAddress8(x, 0, 0, 32) - (int)PixelAddress8(0, 0, 0, 32),
		rowOffset8[1][x] = (int)PixelAddress8(x, 2, 0, 32) - (int)PixelAddress8(0, 2, 0, 32);
	}

	for(int x = 0; x < countof(rowOffset4[0]); x++)
	{
		rowOffset4[0][x] = (int)PixelAddress4(x, 0, 0, 32) - (int)PixelAddress4(0, 0, 0, 32),
		rowOffset4[1][x] = (int)PixelAddress4(x, 2, 0, 32) - (int)PixelAddress4(0, 2, 0, 32);
	}

	for(int i = 0; i < countof(m_psm); i++)
	{
		m_psm[i].pa = &GSLocalMemory::PixelAddress32;
		m_psm[i].ba = &GSLocalMemory::BlockAddress32;
		m_psm[i].pga = &GSLocalMemory::PageAddress32;
		m_psm[i].rp = &GSLocalMemory::ReadPixel32;
		m_psm[i].rpa = &GSLocalMemory::ReadPixel32;
		m_psm[i].wp = &GSLocalMemory::WritePixel32;
		m_psm[i].wpa = &GSLocalMemory::WritePixel32;
		m_psm[i].rt = &GSLocalMemory::ReadTexel32;
		m_psm[i].rtNP = &GSLocalMemory::ReadTexel32;
		m_psm[i].rta = &GSLocalMemory::ReadTexel32;
		m_psm[i].wfa = &GSLocalMemory::WritePixel32;
		m_psm[i].wi = &GSLocalMemory::WriteImage32;
		m_psm[i].ri = NULL; // TODO
		m_psm[i].rtx = &GSLocalMemory::ReadTexture32;
		m_psm[i].rtxNP = &GSLocalMemory::ReadTexture32;
		m_psm[i].bpp = m_psm[i].trbpp = 32;
		m_psm[i].pal = 0;
		m_psm[i].bs = CSize(8, 8);
		m_psm[i].pgs = CSize(64, 32);
		for(int j = 0; j < 8; j++) m_psm[i].rowOffset[j] = rowOffset32;
	}

	m_psm[PSM_PSMCT16].pa = &GSLocalMemory::PixelAddress16;
	m_psm[PSM_PSMCT16S].pa = &GSLocalMemory::PixelAddress16S;
	m_psm[PSM_PSMT8].pa = &GSLocalMemory::PixelAddress8;
	m_psm[PSM_PSMT4].pa = &GSLocalMemory::PixelAddress4;
	m_psm[PSM_PSMZ32].pa = &GSLocalMemory::PixelAddress32Z;
	m_psm[PSM_PSMZ24].pa = &GSLocalMemory::PixelAddress32Z;
	m_psm[PSM_PSMZ16].pa = &GSLocalMemory::PixelAddress16Z;
	m_psm[PSM_PSMZ16S].pa = &GSLocalMemory::PixelAddress16SZ;

	m_psm[PSM_PSMCT16].ba = &GSLocalMemory::BlockAddress16;
	m_psm[PSM_PSMCT16S].ba = &GSLocalMemory::BlockAddress16S;
	m_psm[PSM_PSMT8].ba = &GSLocalMemory::BlockAddress8;
	m_psm[PSM_PSMT4].ba = &GSLocalMemory::BlockAddress4;
	m_psm[PSM_PSMZ32].ba = &GSLocalMemory::BlockAddress32Z;
	m_psm[PSM_PSMZ24].ba = &GSLocalMemory::BlockAddress32Z;
	m_psm[PSM_PSMZ16].ba = &GSLocalMemory::BlockAddress16Z;
	m_psm[PSM_PSMZ16S].ba = &GSLocalMemory::BlockAddress16SZ;

	m_psm[PSM_PSMCT16].pga = &GSLocalMemory::PageAddress16;
	m_psm[PSM_PSMCT16S].pga = &GSLocalMemory::PageAddress16;
	m_psm[PSM_PSMZ16].pga = &GSLocalMemory::PageAddress16;
	m_psm[PSM_PSMZ16S].pga = &GSLocalMemory::PageAddress16;
	m_psm[PSM_PSMT8].pga = &GSLocalMemory::PageAddress8;
	m_psm[PSM_PSMT4].pga = &GSLocalMemory::PageAddress4;

	m_psm[PSM_PSMCT24].rp = &GSLocalMemory::ReadPixel24;
	m_psm[PSM_PSMCT16].rp = &GSLocalMemory::ReadPixel16;
	m_psm[PSM_PSMCT16S].rp = &GSLocalMemory::ReadPixel16S;
	m_psm[PSM_PSMT8].rp = &GSLocalMemory::ReadPixel8;
	m_psm[PSM_PSMT4].rp = &GSLocalMemory::ReadPixel4;
	m_psm[PSM_PSMT8H].rp = &GSLocalMemory::ReadPixel8H;
	m_psm[PSM_PSMT4HL].rp = &GSLocalMemory::ReadPixel4HL;
	m_psm[PSM_PSMT4HH].rp = &GSLocalMemory::ReadPixel4HH;
	m_psm[PSM_PSMZ32].rp = &GSLocalMemory::ReadPixel32Z;
	m_psm[PSM_PSMZ24].rp = &GSLocalMemory::ReadPixel24Z;
	m_psm[PSM_PSMZ16].rp = &GSLocalMemory::ReadPixel16Z;
	m_psm[PSM_PSMZ16S].rp = &GSLocalMemory::ReadPixel16SZ;

	m_psm[PSM_PSMCT24].rpa = &GSLocalMemory::ReadPixel24;
	m_psm[PSM_PSMCT16].rpa = &GSLocalMemory::ReadPixel16;
	m_psm[PSM_PSMCT16S].rpa = &GSLocalMemory::ReadPixel16;
	m_psm[PSM_PSMT8].rpa = &GSLocalMemory::ReadPixel8;
	m_psm[PSM_PSMT4].rpa = &GSLocalMemory::ReadPixel4;
	m_psm[PSM_PSMT8H].rpa = &GSLocalMemory::ReadPixel8H;
	m_psm[PSM_PSMT4HL].rpa = &GSLocalMemory::ReadPixel4HL;
	m_psm[PSM_PSMT4HH].rpa = &GSLocalMemory::ReadPixel4HH;
	m_psm[PSM_PSMZ32].rpa = &GSLocalMemory::ReadPixel32;
	m_psm[PSM_PSMZ24].rpa = &GSLocalMemory::ReadPixel24;
	m_psm[PSM_PSMZ16].rpa = &GSLocalMemory::ReadPixel16;
	m_psm[PSM_PSMZ16S].rpa = &GSLocalMemory::ReadPixel16;

	m_psm[PSM_PSMCT32].wp = &GSLocalMemory::WritePixel32;
	m_psm[PSM_PSMCT24].wp = &GSLocalMemory::WritePixel24;
	m_psm[PSM_PSMCT16].wp = &GSLocalMemory::WritePixel16;
	m_psm[PSM_PSMCT16S].wp = &GSLocalMemory::WritePixel16S;
	m_psm[PSM_PSMT8].wp = &GSLocalMemory::WritePixel8;
	m_psm[PSM_PSMT4].wp = &GSLocalMemory::WritePixel4;
	m_psm[PSM_PSMT8H].wp = &GSLocalMemory::WritePixel8H;
	m_psm[PSM_PSMT4HL].wp = &GSLocalMemory::WritePixel4HL;
	m_psm[PSM_PSMT4HH].wp = &GSLocalMemory::WritePixel4HH;
	m_psm[PSM_PSMZ32].wp = &GSLocalMemory::WritePixel32Z;
	m_psm[PSM_PSMZ24].wp = &GSLocalMemory::WritePixel24Z;
	m_psm[PSM_PSMZ16].wp = &GSLocalMemory::WritePixel16Z;
	m_psm[PSM_PSMZ16S].wp = &GSLocalMemory::WritePixel16SZ;

	m_psm[PSM_PSMCT32].wpa = &GSLocalMemory::WritePixel32;
	m_psm[PSM_PSMCT24].wpa = &GSLocalMemory::WritePixel24;
	m_psm[PSM_PSMCT16].wpa = &GSLocalMemory::WritePixel16;
	m_psm[PSM_PSMCT16S].wpa = &GSLocalMemory::WritePixel16;
	m_psm[PSM_PSMT8].wpa = &GSLocalMemory::WritePixel8;
	m_psm[PSM_PSMT4].wpa = &GSLocalMemory::WritePixel4;
	m_psm[PSM_PSMT8H].wpa = &GSLocalMemory::WritePixel8H;
	m_psm[PSM_PSMT4HL].wpa = &GSLocalMemory::WritePixel4HL;
	m_psm[PSM_PSMT4HH].wpa = &GSLocalMemory::WritePixel4HH;
	m_psm[PSM_PSMZ32].wpa = &GSLocalMemory::WritePixel32;
	m_psm[PSM_PSMZ24].wpa = &GSLocalMemory::WritePixel24;
	m_psm[PSM_PSMZ16].wpa = &GSLocalMemory::WritePixel16;
	m_psm[PSM_PSMZ16S].wpa = &GSLocalMemory::WritePixel16;

	m_psm[PSM_PSMCT24].rt = &GSLocalMemory::ReadTexel24;
	m_psm[PSM_PSMCT16].rt = &GSLocalMemory::ReadTexel16;
	m_psm[PSM_PSMCT16S].rt = &GSLocalMemory::ReadTexel16S;
	m_psm[PSM_PSMT8].rt = &GSLocalMemory::ReadTexel8;
	m_psm[PSM_PSMT4].rt = &GSLocalMemory::ReadTexel4;
	m_psm[PSM_PSMT8H].rt = &GSLocalMemory::ReadTexel8H;
	m_psm[PSM_PSMT4HL].rt = &GSLocalMemory::ReadTexel4HL;
	m_psm[PSM_PSMT4HH].rt = &GSLocalMemory::ReadTexel4HH;
	m_psm[PSM_PSMZ32].rt = &GSLocalMemory::ReadTexel32Z;
	m_psm[PSM_PSMZ24].rt = &GSLocalMemory::ReadTexel24Z;
	m_psm[PSM_PSMZ16].rt = &GSLocalMemory::ReadTexel16Z;
	m_psm[PSM_PSMZ16S].rt = &GSLocalMemory::ReadTexel16SZ;

	m_psm[PSM_PSMCT24].rta = &GSLocalMemory::ReadTexel24;
	m_psm[PSM_PSMCT16].rta = &GSLocalMemory::ReadTexel16;
	m_psm[PSM_PSMCT16S].rta = &GSLocalMemory::ReadTexel16;
	m_psm[PSM_PSMT8].rta = &GSLocalMemory::ReadTexel8;
	m_psm[PSM_PSMT4].rta = &GSLocalMemory::ReadTexel4;
	m_psm[PSM_PSMT8H].rta = &GSLocalMemory::ReadTexel8H;
	m_psm[PSM_PSMT4HL].rta = &GSLocalMemory::ReadTexel4HL;
	m_psm[PSM_PSMT4HH].rta = &GSLocalMemory::ReadTexel4HH;
	m_psm[PSM_PSMZ24].rta = &GSLocalMemory::ReadTexel24;
	m_psm[PSM_PSMZ16].rta = &GSLocalMemory::ReadTexel16;
	m_psm[PSM_PSMZ16S].rta = &GSLocalMemory::ReadTexel16;

	m_psm[PSM_PSMCT24].wfa = &GSLocalMemory::WritePixel24;
	m_psm[PSM_PSMCT16].wfa = &GSLocalMemory::WriteFrame16;
	m_psm[PSM_PSMCT16S].wfa = &GSLocalMemory::WriteFrame16;
	m_psm[PSM_PSMZ24].wfa = &GSLocalMemory::WritePixel24;
	m_psm[PSM_PSMZ16].wfa = &GSLocalMemory::WriteFrame16;
	m_psm[PSM_PSMZ16S].wfa = &GSLocalMemory::WriteFrame16;

	m_psm[PSM_PSMCT16].rtNP = &GSLocalMemory::ReadTexel16NP;
	m_psm[PSM_PSMCT16S].rtNP = &GSLocalMemory::ReadTexel16SNP;
	m_psm[PSM_PSMT8].rtNP = &GSLocalMemory::ReadTexel8;
	m_psm[PSM_PSMT4].rtNP = &GSLocalMemory::ReadTexel4;
	m_psm[PSM_PSMT8H].rtNP = &GSLocalMemory::ReadTexel8H;
	m_psm[PSM_PSMT4HL].rtNP = &GSLocalMemory::ReadTexel4HL;
	m_psm[PSM_PSMT4HH].rtNP = &GSLocalMemory::ReadTexel4HH;
	m_psm[PSM_PSMZ32].rtNP = &GSLocalMemory::ReadTexel32Z;
	m_psm[PSM_PSMZ24].rtNP = &GSLocalMemory::ReadTexel24Z;
	m_psm[PSM_PSMZ16].rtNP = &GSLocalMemory::ReadTexel16ZNP;
	m_psm[PSM_PSMZ16S].rtNP = &GSLocalMemory::ReadTexel16SZNP;

	m_psm[PSM_PSMCT24].wi = &GSLocalMemory::WriteImage24;
	m_psm[PSM_PSMCT16].wi = &GSLocalMemory::WriteImage16;
	m_psm[PSM_PSMCT16S].wi = &GSLocalMemory::WriteImage16S;
	m_psm[PSM_PSMT8].wi = &GSLocalMemory::WriteImage8;
	m_psm[PSM_PSMT4].wi = &GSLocalMemory::WriteImage4;
	m_psm[PSM_PSMT8H].wi = &GSLocalMemory::WriteImage8H;
	m_psm[PSM_PSMT4HL].wi = &GSLocalMemory::WriteImage4HL;
	m_psm[PSM_PSMT4HH].wi = &GSLocalMemory::WriteImage4HH;
	m_psm[PSM_PSMZ32].wi = &GSLocalMemory::WriteImage32Z;
	m_psm[PSM_PSMZ24].wi = &GSLocalMemory::WriteImage24Z;
	m_psm[PSM_PSMZ16].wi = &GSLocalMemory::WriteImage16Z;
	m_psm[PSM_PSMZ16S].wi = &GSLocalMemory::WriteImage16SZ;

	m_psm[PSM_PSMCT24].rtx = &GSLocalMemory::ReadTexture24;
	m_psm[PSM_PSMCT16].rtx = &GSLocalMemory::ReadTexture16;
	m_psm[PSM_PSMCT16S].rtx = &GSLocalMemory::ReadTexture16S;
	m_psm[PSM_PSMT8].rtx = &GSLocalMemory::ReadTexture8;
	m_psm[PSM_PSMT4].rtx = &GSLocalMemory::ReadTexture4;
	m_psm[PSM_PSMT8H].rtx = &GSLocalMemory::ReadTexture8H;
	m_psm[PSM_PSMT4HL].rtx = &GSLocalMemory::ReadTexture4HL;
	m_psm[PSM_PSMT4HH].rtx = &GSLocalMemory::ReadTexture4HH;
	m_psm[PSM_PSMZ32].rtx = &GSLocalMemory::ReadTexture32Z;
	m_psm[PSM_PSMZ24].rtx = &GSLocalMemory::ReadTexture24Z;
	m_psm[PSM_PSMZ16].rtx = &GSLocalMemory::ReadTexture16Z;
	m_psm[PSM_PSMZ16S].rtx = &GSLocalMemory::ReadTexture16SZ;

	m_psm[PSM_PSMCT16].rtxNP = &GSLocalMemory::ReadTexture16NP;
	m_psm[PSM_PSMCT16S].rtxNP = &GSLocalMemory::ReadTexture16SNP;
	m_psm[PSM_PSMT8].rtxNP = &GSLocalMemory::ReadTexture8NP;
	m_psm[PSM_PSMT4].rtxNP = &GSLocalMemory::ReadTexture4NP;
	m_psm[PSM_PSMT8H].rtxNP = &GSLocalMemory::ReadTexture8HNP;
	m_psm[PSM_PSMT4HL].rtxNP = &GSLocalMemory::ReadTexture4HLNP;
	m_psm[PSM_PSMT4HH].rtxNP = &GSLocalMemory::ReadTexture4HHNP;
	m_psm[PSM_PSMZ32].rtxNP = &GSLocalMemory::ReadTexture32Z;
	m_psm[PSM_PSMZ24].rtxNP = &GSLocalMemory::ReadTexture24Z;
	m_psm[PSM_PSMZ16].rtxNP = &GSLocalMemory::ReadTexture16ZNP;
	m_psm[PSM_PSMZ16S].rtxNP = &GSLocalMemory::ReadTexture16SZNP;

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
		#if _M_SSE >= 0x200

		GSVector4i c128(c);
		GSVector4i mask(0x00ffffff);

		for(int y = clip.top; y < clip.bottom; y += h)
		{
			for(int x = clip.left; x < clip.right; x += w)
			{
				GSVector4i* p = (GSVector4i*)&m_vm8[ba(x, y, fbp, fbw) << 2 >> shift];

				for(int i = 0; i < 16; i += 4)
				{
					p[i + 0] = p[i + 0].blend8(c128, mask);
					p[i + 1] = p[i + 1].blend8(c128, mask);
					p[i + 2] = p[i + 2].blend8(c128, mask);
					p[i + 3] = p[i + 3].blend8(c128, mask);
				}
			}
		}

		#else

		c &= 0x00ffffff;

		for(int y = clip.top; y < clip.bottom; y += h)
		{
			for(int x = clip.left; x < clip.right; x += w)
			{
				DWORD* p = &m_vm32[ba(x, y, fbp, fbw)];

				for(int i = 0; i < 64; i += 4)
				{
					p[i + 0] = (p[i + 0] & 0xff000000) | c;
					p[i + 1] = (p[i + 1] & 0xff000000) | c;
					p[i + 2] = (p[i + 2] & 0xff000000) | c;
					p[i + 3] = (p[i + 3] & 0xff000000) | c;
				}
			}
		}

		#endif
	}
	else
	{
		#if _M_SSE >= 0x200

		GSVector4i c128(c);

		for(int y = clip.top; y < clip.bottom; y += h)
		{
			for(int x = clip.left; x < clip.right; x += w)
			{
				GSVector4i* p = (GSVector4i*)&m_vm8[ba(x, y, fbp, fbw) << 2 >> shift];

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
			WORD* vm = &m_vm16[TEX0.CPSM == PSM_PSMCT16 ? BlockAddress16(0, 0, bp, bw) : BlockAddress16S(0, 0, bp, bw)];

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
			DWORD* vm = &m_vm32[BlockAddress32(0, 0, bp, bw)];

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

	if(TEX0.CPSM == PSM_PSMCT32 || TEX0.CPSM == PSM_PSMCT24)
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
			DWORD* src = m_pCLUT32;
			DWORD* dst = (DWORD*)m_pCLUT64;

			for(int j = 0; j < 16; j++, dst += 32)
			{
				DWORD hi = src[j];

				for(int i = 0; i < 16; i++)
				{
					dst[i * 2 + 0] = src[i];
					dst[i * 2 + 1] = hi;
				}
			}
		}
		else
		{
			DWORD* src = m_pCLUT32;
			DWORD* dst = (DWORD*)m_pCLUT64;

			for(int j = 0; j < 16; j++, dst += 32)
			{
				DWORD hi = src[j] << 16;

				for(int i = 0; i < 16; i++)
				{
					dst[i * 2 + 0] = hi | (src[i] & 0xffff);
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

	if(TEX0.CPSM == PSM_PSMCT32 || TEX0.CPSM == PSM_PSMCT24)
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
		{
			DWORD* src = m_pCLUT32;
			DWORD* dst = (DWORD*)m_pCLUT64;

			for(int j = 0; j < 16; j++, dst += 32)
			{
				DWORD hi = src[j];

				for(int i = 0; i < 16; i++)
				{
					dst[i * 2 + 0] = src[i];
					dst[i * 2 + 1] = hi;
				}
			}
		}

		break;
	}
}

void GSLocalMemory::CopyCLUT32(DWORD* pCLUT32, int n)
{
	memcpy(pCLUT32, m_pCLUT32, sizeof(DWORD) * n);
}

#define IsTopLeftAligned(dsax, tx, ty, bw, bh) \
	((((int)dsax) & ((bw)-1)) == 0 && ((tx) & ((bw)-1)) == 0 && ((int)dsax) == (tx) && ((ty) & ((bh)-1)) == 0)

void GSLocalMemory::WriteImage32(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG)
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
					WriteBlock32u((BYTE*)&m_vm32[BlockAddress32(x, ty, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 4, srcpitch);
				}

				for(int i = 0; i < 8; i++, ty++, src += srcpitch)
				{
					for(int x = twa; x < tw; x++)
					{
						WritePixel32(x, ty, ((DWORD*)src)[x - tx], BITBLTBUF.DBP, BITBLTBUF.DBW);
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
					WriteColumn32(ty, (BYTE*)&m_vm32[BlockAddress32(x, ty & ~7, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 4, srcpitch);
				}

				for(int i = 0; i < 2; i++, ty++, src += srcpitch)
				{
					for(int x = twa; x < tw; x++)
					{
						WritePixel32(x, ty, ((DWORD*)src)[x - tx], BITBLTBUF.DBP, BITBLTBUF.DBW);
					}
				}
			}
		}

		WriteImageX(tx, ty, src, len, BITBLTBUF, TRXPOS, TRXREG);
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
					WriteBlock32u((BYTE*)&m_vm32[BlockAddress32(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 4, srcpitch);
				}
			}
		}
		else
		{
			for(int y = ty; y < th; y += 8, src += srcpitch * 8)
			{
				for(int x = tx; x < tw; x += 8)
				{
					WriteBlock32((BYTE*)&m_vm32[BlockAddress32(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 4, srcpitch);
				}
			}
		}

		ty = th;
	}
}

void GSLocalMemory::WriteImage24(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG)
{
	if(TRXREG.RRW == 0) return;

	int tw = TRXREG.RRW, srcpitch = (TRXREG.RRW - TRXPOS.DSAX) * 3;
	int th = len / srcpitch;

	bool aligned = IsTopLeftAligned(TRXPOS.DSAX, tx, ty, 8, 8);

	if(!aligned || (tw & 7) || (th & 7) || (len % srcpitch))
	{
		// TODO

		WriteImageX(tx, ty, src, len, BITBLTBUF, TRXPOS, TRXREG);
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

				WriteBlock32((BYTE*)&m_vm32[BlockAddress32(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], (BYTE*)block, sizeof(block) / 8, 0x00ffffff);
			}
		}

		ty = th;
	}
}

void GSLocalMemory::WriteImage16(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG)
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
					WriteBlock16u((BYTE*)&m_vm16[BlockAddress16(x, ty, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}

				for(int i = 0; i < 8; i++, ty++, src += srcpitch)
				{
					for(int x = twa; x < tw; x++)
					{
						WritePixel16(x, ty, ((WORD*)src)[x - tx], BITBLTBUF.DBP, BITBLTBUF.DBW);
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
					WriteColumn16(ty, (BYTE*)&m_vm16[BlockAddress16(x, ty & ~7, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}

				for(int i = 0; i < 2; i++, ty++, src += srcpitch)
				{
					for(int x = twa; x < tw; x++)
					{
						WritePixel16(x, ty, ((WORD*)src)[x - tx], BITBLTBUF.DBP, BITBLTBUF.DBW);
					}
				}
			}
		}

		WriteImageX(tx, ty, src, len, BITBLTBUF, TRXPOS, TRXREG);
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
					WriteBlock16u((BYTE*)&m_vm16[BlockAddress16(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}
			}
		}
		else
		{
			for(int y = ty; y < th; y += 8, src += srcpitch * 8)
			{
				for(int x = tx; x < tw; x += 16)
				{
					WriteBlock16((BYTE*)&m_vm16[BlockAddress16(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}
			}
		}

		ty = th;
	}
}

void GSLocalMemory::WriteImage16S(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG)
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
					WriteBlock16u((BYTE*)&m_vm16[BlockAddress16S(x, ty, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}

				for(int i = 0; i < 8; i++, ty++, src += srcpitch)
				{
					for(int x = twa; x < tw; x++)
					{
						WritePixel16S(x, ty, ((WORD*)src)[x - tx], BITBLTBUF.DBP, BITBLTBUF.DBW);
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
					WriteColumn16(ty, (BYTE*)&m_vm16[BlockAddress16S(x, ty & ~7, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}

				for(int i = 0; i < 2; i++, ty++, src += srcpitch)
				{
					for(int x = twa; x < tw; x++)
					{
						WritePixel16S(x, ty, ((WORD*)src)[x - tx], BITBLTBUF.DBP, BITBLTBUF.DBW);
					}
				}
			}
		}

		WriteImageX(tx, ty, src, len, BITBLTBUF, TRXPOS, TRXREG);
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
					WriteBlock16((BYTE*)&m_vm16[BlockAddress16S(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}
			}
		}
		else
		{
			for(int y = ty; y < th; y += 8, src += srcpitch * 8)
			{
				for(int x = tx; x < tw; x += 16)
				{
					WriteBlock16((BYTE*)&m_vm16[BlockAddress16S(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}
			}
		}

		ty = th;
	}
}

void GSLocalMemory::WriteImage8(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG)
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
					WriteBlock8u((BYTE*)&m_vm8[BlockAddress8(x, ty, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx), srcpitch);
				}

				for(int i = 0; i < 16; i++, ty++, src += srcpitch)
				{
					for(int x = twa; x < tw; x++)
					{
						WritePixel8(x, ty, src[x - tx], BITBLTBUF.DBP, BITBLTBUF.DBW);
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
					WriteColumn8(ty, (BYTE*)&m_vm8[BlockAddress8(x, ty & ~15, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx), srcpitch);
				}

				for(int i = 0; i < 4; i++, ty++, src += srcpitch)
				{
					for(int x = twa; x < tw; x++)
					{
						WritePixel8(x, ty, src[x - tx], BITBLTBUF.DBP, BITBLTBUF.DBW);
					}
				}
			}
		}

		WriteImageX(tx, ty, src, len, BITBLTBUF, TRXPOS, TRXREG);
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
					WriteBlock8u((BYTE*)&m_vm8[BlockAddress8(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx), srcpitch);
				}
			}
		}
		else
		{
			for(int y = ty; y < th; y += 16, src += srcpitch * 16)
			{
				for(int x = tx; x < tw; x += 16)
				{
					WriteBlock8((BYTE*)&m_vm8[BlockAddress8(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx), srcpitch);
				}
			}
		}

		ty = th;
	}
}

void GSLocalMemory::WriteImage4(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG)
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
					WriteBlock4u((BYTE*)&m_vm8[BlockAddress4(x, ty, BITBLTBUF.DBP, BITBLTBUF.DBW) >> 1], src + (x - tx) / 2, srcpitch);
				}

				for(int i = 0; i < 16; i++, ty++, src += srcpitch)
				{
					BYTE* s = src + (twa - tx) / 2;

					for(int x = twa; x < tw; x += 2, s++)
					{
						WritePixel4(x, ty, *s & 0xf, BITBLTBUF.DBP, BITBLTBUF.DBW),
						WritePixel4(x + 1, ty, *s >> 4, BITBLTBUF.DBP, BITBLTBUF.DBW);
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
					WriteColumn4(ty, (BYTE*)&m_vm8[BlockAddress4(x, ty & ~15, BITBLTBUF.DBP, BITBLTBUF.DBW) >> 1], src + (x - tx) / 2, srcpitch);
				}

				for(int i = 0; i < 4; i++, ty++, src += srcpitch)
				{
					BYTE* s = src + (twa - tx) / 2;

					for(int x = twa; x < tw; x += 2, s++)
					{
						WritePixel4(x, ty, *s & 0xf, BITBLTBUF.DBP, BITBLTBUF.DBW),
						WritePixel4(x + 1, ty, *s >> 4, BITBLTBUF.DBP, BITBLTBUF.DBW);
					}
				}
			}
		}

		WriteImageX(tx, ty, src, len, BITBLTBUF, TRXPOS, TRXREG);
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
					WriteBlock4u((BYTE*)&m_vm8[BlockAddress4(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW) >> 1], src + (x - tx) / 2, srcpitch);
				}
			}
		}
		else
		{
			for(int y = ty; y < th; y += 16, src += srcpitch * 16)
			{
				for(int x = tx; x < tw; x += 32)
				{
					WriteBlock4((BYTE*)&m_vm8[BlockAddress4(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW) >> 1], src + (x - tx) / 2, srcpitch);
				}
			}
		}

		ty = th;
	}
}

void GSLocalMemory::WriteImage8H(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG)
{
	if(TRXREG.RRW == 0) return;

	int tw = TRXREG.RRW, srcpitch = TRXREG.RRW - TRXPOS.DSAX;
	int th = len / srcpitch;

	bool aligned = IsTopLeftAligned(TRXPOS.DSAX, tx, ty, 8, 8);

	if(!aligned || (tw & 7) || (th & 7) || (len % srcpitch))
	{
		// TODO

		WriteImageX(tx, ty, src, len, BITBLTBUF, TRXPOS, TRXREG);
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

				WriteBlock32((BYTE*)&m_vm32[BlockAddress32(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], (BYTE*)block, sizeof(block) / 8, 0xff000000);
			}
		}

		ty = th;
	}
}

void GSLocalMemory::WriteImage4HL(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG)
{
	if(TRXREG.RRW == 0) return;

	int tw = TRXREG.RRW, srcpitch = (TRXREG.RRW - TRXPOS.DSAX) / 2;
	int th = len / srcpitch;

	bool aligned = IsTopLeftAligned(TRXPOS.DSAX, tx, ty, 8, 8);

	if(!aligned || (tw & 7) || (th & 7) || (len % srcpitch))
	{
		// TODO

		WriteImageX(tx, ty, src, len, BITBLTBUF, TRXPOS, TRXREG);
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

				WriteBlock32((BYTE*)&m_vm32[BlockAddress32(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], (BYTE*)block, sizeof(block) / 8, 0x0f000000);
			}
		}

		ty = th;
	}
}

void GSLocalMemory::WriteImage4HH(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG)
{
	if(TRXREG.RRW == 0) return;

	int tw = TRXREG.RRW, srcpitch = (TRXREG.RRW - TRXPOS.DSAX) / 2;
	int th = len / srcpitch;

	bool aligned = IsTopLeftAligned(TRXPOS.DSAX, tx, ty, 8, 8);

	if(!aligned || (tw & 7) || (th & 7) || (len % srcpitch))
	{
		// TODO

		WriteImageX(tx, ty, src, len, BITBLTBUF, TRXPOS, TRXREG);
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

				WriteBlock32((BYTE*)&m_vm32[BlockAddress32(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], (BYTE*)block, sizeof(block) / 8, 0xf0000000);
			}
		}

		ty = th;
	}
}

void GSLocalMemory::WriteImage32Z(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG)
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
					WriteBlock32u((BYTE*)&m_vm32[BlockAddress32Z(x, ty, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 4, srcpitch);
				}

				for(int i = 0; i < 8; i++, ty++, src += srcpitch)
				{
					for(int x = twa; x < tw; x++)
					{
						WritePixel32Z(x, ty, ((DWORD*)src)[x - tx], BITBLTBUF.DBP, BITBLTBUF.DBW);
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
					WriteColumn32(ty, (BYTE*)&m_vm32[BlockAddress32Z(x, ty & ~7, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 4, srcpitch);
				}

				for(int i = 0; i < 2; i++, ty++, src += srcpitch)
				{
					for(int x = twa; x < tw; x++)
					{
						WritePixel32Z(x, ty, ((DWORD*)src)[x - tx], BITBLTBUF.DBP, BITBLTBUF.DBW);
					}
				}
			}
		}

		WriteImageX(tx, ty, src, len, BITBLTBUF, TRXPOS, TRXREG);
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
					WriteBlock32u((BYTE*)&m_vm32[BlockAddress32Z(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 4, srcpitch);
				}
			}
		}
		else
		{
			for(int y = ty; y < th; y += 8, src += srcpitch * 8)
			{
				for(int x = tx; x < tw; x += 8)
				{
					WriteBlock32((BYTE*)&m_vm32[BlockAddress32Z(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 4, srcpitch);
				}
			}
		}

		ty = th;
	}
}

void GSLocalMemory::WriteImage24Z(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG)
{
	if(TRXREG.RRW == 0) return;

	int tw = TRXREG.RRW, srcpitch = (TRXREG.RRW - TRXPOS.DSAX) * 3;
	int th = len / srcpitch;

	bool aligned = IsTopLeftAligned(TRXPOS.DSAX, tx, ty, 8, 8);

	if(!aligned || (tw & 7) || (th & 7) || (len % srcpitch))
	{
		// TODO

		WriteImageX(tx, ty, src, len, BITBLTBUF, TRXPOS, TRXREG);
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

				WriteBlock32((BYTE*)&m_vm32[BlockAddress32Z(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], (BYTE*)block, sizeof(block) / 8, 0x00ffffff);
			}
		}

		ty = th;
	}
}

void GSLocalMemory::WriteImage16Z(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG)
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
					WriteBlock16u((BYTE*)&m_vm16[BlockAddress16Z(x, ty, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}

				for(int i = 0; i < 8; i++, ty++, src += srcpitch)
				{
					for(int x = twa; x < tw; x++)
					{
						WritePixel16Z(x, ty, ((WORD*)src)[x - tx], BITBLTBUF.DBP, BITBLTBUF.DBW);
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
					WriteColumn16(ty, (BYTE*)&m_vm16[BlockAddress16Z(x, ty & ~7, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}

				for(int i = 0; i < 2; i++, ty++, src += srcpitch)
				{
					for(int x = twa; x < tw; x++)
					{
						WritePixel16Z(x, ty, ((WORD*)src)[x - tx], BITBLTBUF.DBP, BITBLTBUF.DBW);
					}
				}
			}
		}

		WriteImageX(tx, ty, src, len, BITBLTBUF, TRXPOS, TRXREG);
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
					WriteBlock16u((BYTE*)&m_vm16[BlockAddress16Z(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}
			}
		}
		else
		{
			for(int y = ty; y < th; y += 8, src += srcpitch * 8)
			{
				for(int x = tx; x < tw; x += 16)
				{
					WriteBlock16((BYTE*)&m_vm16[BlockAddress16Z(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}
			} 
		}

		ty = th;
	}
}

void GSLocalMemory::WriteImage16SZ(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG)
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
					WriteBlock16u((BYTE*)&m_vm16[BlockAddress16SZ(x, ty, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}

				for(int i = 0; i < 8; i++, ty++, src += srcpitch)
				{
					for(int x = twa; x < tw; x++)
					{
						WritePixel16SZ(x, ty, ((WORD*)src)[x - tx], BITBLTBUF.DBP, BITBLTBUF.DBW);
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
					WriteColumn16(ty, (BYTE*)&m_vm16[BlockAddress16SZ(x, ty & ~7, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}

				for(int i = 0; i < 2; i++, ty++, src += srcpitch)
				{
					for(int x = twa; x < tw; x++)
					{
						WritePixel16SZ(x, ty, ((WORD*)src)[x - tx], BITBLTBUF.DBP, BITBLTBUF.DBW);
					}
				}
			}
		}

		WriteImageX(tx, ty, src, len, BITBLTBUF, TRXPOS, TRXREG);
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
					WriteBlock16((BYTE*)&m_vm16[BlockAddress16SZ(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}
			}
		}
		else
		{
			for(int y = ty; y < th; y += 8, src += srcpitch * 8)
			{
				for(int x = tx; x < tw; x += 16)
				{
					WriteBlock16((BYTE*)&m_vm16[BlockAddress16SZ(x, y, BITBLTBUF.DBP, BITBLTBUF.DBW)], src + (x - tx) * 2, srcpitch);
				}
			}
		}

		ty = th;
	}
}

void GSLocalMemory::WriteImageX(int& tx, int& ty, BYTE* src, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG)
{
	if(len <= 0) return;

	// if(ty >= (int)TRXREG.RRH) {ASSERT(0); return;}

	BYTE* pb = (BYTE*)src;
	WORD* pw = (WORD*)src;
	DWORD* pd = (DWORD*)src;

	DWORD bp = BITBLTBUF.DBP;
	DWORD bw = BITBLTBUF.DBW;
	psm_t* psm = &m_psm[BITBLTBUF.DPSM];

	int x = tx;
	int y = ty;
	int sx = (int)TRXPOS.DSAX;
	int ex = (int)TRXREG.RRW;

	switch(BITBLTBUF.DPSM)
	{
	case PSM_PSMCT32:
	case PSM_PSMZ32:

		len /= 4;

		while(len > 0)
		{
			DWORD addr = psm->pa(0, y, bp, bw);
			int* offset = psm->rowOffset[y & 7];

			for(; len > 0 && x < ex; len--, x++, pd++)
			{
				WritePixel32(addr + offset[x], *pd);
			}

			if(x == ex) {x = sx; y++;}
		}

		break;

	case PSM_PSMCT24:
	case PSM_PSMZ24:

		len /= 3;

		while(len > 0)
		{
			DWORD addr = psm->pa(0, y, bp, bw);
			int* offset = psm->rowOffset[y & 7];

			for(; len > 0 && x < ex; len--, x++, pb += 3)
			{
				WritePixel24(addr + offset[x], *(DWORD*)pb);
			}

			if(x == ex) {x = sx; y++;}
		}

		break;

	case PSM_PSMCT16:
	case PSM_PSMCT16S:
	case PSM_PSMZ16:
	case PSM_PSMZ16S:

		len /= 2;

		while(len > 0)
		{
			DWORD addr = psm->pa(0, y, bp, bw);
			int* offset = psm->rowOffset[y & 7];

			for(; len > 0 && x < ex; len--, x++, pw++)
			{
				WritePixel16(addr + offset[x], *pw);
			}

			if(x == ex) {x = sx; y++;}
		}

		break;

	case PSM_PSMT8:

		while(len > 0)
		{
			DWORD addr = psm->pa(0, y, bp, bw);
			int* offset = psm->rowOffset[y & 7];

			for(; len > 0 && x < ex; len--, x++, pb++)
			{
				WritePixel8(addr + offset[x], *pb);
			}

			if(x == ex) {x = sx; y++;}
		}

		break;

	case PSM_PSMT4:

		while(len > 0)
		{
			DWORD addr = psm->pa(0, y, bp, bw);
			int* offset = psm->rowOffset[y & 7];

			for(; len > 0 && x < ex; len--, x += 2, pb++)
			{
				WritePixel4(addr + offset[x + 0], *pb & 0xf);
				WritePixel4(addr + offset[x + 1], *pb >> 4);
			}

			if(x == ex) {x = sx; y++;}
		}

		break;

	case PSM_PSMT8H:

		while(len > 0)
		{
			DWORD addr = psm->pa(0, y, bp, bw);
			int* offset = psm->rowOffset[y & 7];

			for(; len > 0 && x < ex; len--, x++, pb++)
			{
				WritePixel8H(addr + offset[x], *pb);
			}

			if(x == ex) {x = sx; y++;}
		}

		break;

	case PSM_PSMT4HL:

		while(len > 0)
		{
			DWORD addr = psm->pa(0, y, bp, bw);
			int* offset = psm->rowOffset[y & 7];

			for(; len > 0 && x < ex; len--, x += 2, pb++)
			{
				WritePixel4HL(addr + offset[x + 0], *pb & 0xf);
				WritePixel4HL(addr + offset[x + 1], *pb >> 4);
			}

			if(x == ex) {x = sx; y++;}
		}

		break;

	case PSM_PSMT4HH:

		while(len > 0)
		{
			DWORD addr = psm->pa(0, y, bp, bw);
			int* offset = psm->rowOffset[y & 7];

			for(; len > 0 && x < ex; len--, x += 2, pb++)
			{
				WritePixel4HH(addr + offset[x + 0], *pb & 0xf);
				WritePixel4HH(addr + offset[x + 1], *pb >> 4);
			}

			if(x == ex) {x = sx; y++;}
		}

		break;
	}

	tx = x;
	ty = y;
}

//

void GSLocalMemory::ReadImageX(int& tx, int& ty, BYTE* dst, int len, GIFRegBITBLTBUF& BITBLTBUF, GIFRegTRXPOS& TRXPOS, GIFRegTRXREG& TRXREG)
{
	if(len <= 0) return;

	// if(ty >= (int)TRXREG.RRH) {ASSERT(0); return;}

	BYTE* pb = (BYTE*)dst;
	WORD* pw = (WORD*)dst;
	DWORD* pd = (DWORD*)dst;

	DWORD bp = BITBLTBUF.SBP;
	DWORD bw = BITBLTBUF.SBW;
	psm_t* psm = &m_psm[BITBLTBUF.SPSM];

	int x = tx;
	int y = ty;
	int sx = (int)TRXPOS.SSAX;
	int ex = (int)TRXREG.RRW;

	switch(BITBLTBUF.SPSM)
	{
	case PSM_PSMCT32:
	case PSM_PSMZ32:

		len /= 4;

		while(len > 0)
		{
			DWORD addr = psm->pa(0, y, bp, bw);
			int* offset = psm->rowOffset[y & 7];

			for(; len > 0 && x < ex; len--, x++, pd++)
			{
				*pd = ReadPixel32(addr + offset[x]);
			}

			if(x == ex) {x = sx; y++;}
		}

		break;

	case PSM_PSMCT24:
	case PSM_PSMZ24:

		len /= 3;

		while(len > 0)
		{
			DWORD addr = psm->pa(0, y, bp, bw);
			int* offset = psm->rowOffset[y & 7];

			for(; len > 0 && x < ex; len--, x++, pb += 3)
			{
				DWORD dw = ReadPixel32(addr + offset[x]);
				
				pb[0] = ((BYTE*)&dw)[0]; 
				pb[1] = ((BYTE*)&dw)[1]; 
				pb[2] = ((BYTE*)&dw)[2];
			}

			if(x == ex) {x = sx; y++;}
		}

		break;

	case PSM_PSMCT16:
	case PSM_PSMCT16S:
	case PSM_PSMZ16:
	case PSM_PSMZ16S:

		len /= 2;

		while(len > 0)
		{
			DWORD addr = psm->pa(0, y, bp, bw);
			int* offset = psm->rowOffset[y & 7];

			for(; len > 0 && x < ex; len--, x++, pw++)
			{
				*pw = ReadPixel16(addr + offset[x]);
			}

			if(x == ex) {x = sx; y++;}
		}

		break;

	case PSM_PSMT8:

		while(len > 0)
		{
			DWORD addr = psm->pa(0, y, bp, bw);
			int* offset = psm->rowOffset[y & 7];

			for(; len > 0 && x < ex; len--, x++, pb++)
			{
				*pb = ReadPixel8(addr + offset[x]);
			}

			if(x == ex) {x = sx; y++;}
		}

		break;

	case PSM_PSMT4:

		while(len > 0)
		{
			DWORD addr = psm->pa(0, y, bp, bw);
			int* offset = psm->rowOffset[y & 7];

			for(; len > 0 && x < ex; len--, x += 2, pb++)
			{
				*pb = ReadPixel4(addr + offset[x + 0]) | (ReadPixel4(addr + offset[x + 1]) << 4);
			}

			if(x == ex) {x = sx; y++;}
		}

		break;

	case PSM_PSMT8H:

		while(len > 0)
		{
			DWORD addr = psm->pa(0, y, bp, bw);
			int* offset = psm->rowOffset[y & 7];

			for(; len > 0 && x < ex; len--, x++, pb++)
			{
				*pb = ReadPixel8H(addr + offset[x]);
			}

			if(x == ex) {x = sx; y++;}
		}

		break;

	case PSM_PSMT4HL:

		while(len > 0)
		{
			DWORD addr = psm->pa(0, y, bp, bw);
			int* offset = psm->rowOffset[y & 7];

			for(; len > 0 && x < ex; len--, x += 2, pb++)
			{
				*pb = ReadPixel4HL(addr + offset[x + 0]) | (ReadPixel4HL(addr + offset[x + 1]) << 4);
			}

			if(x == ex) {x = sx; y++;}
		}

		break;
	
	case PSM_PSMT4HH:

		while(len > 0)
		{
			DWORD addr = psm->pa(0, y, bp, bw);
			int* offset = psm->rowOffset[y & 7];

			for(; len > 0 && x < ex; len--, x += 2, pb++)
			{
				*pb = ReadPixel4HH(addr + offset[x + 0]) | (ReadPixel4HH(addr + offset[x + 1]) << 4);
			}

			if(x == ex) {x = sx; y++;}
		}

		break;
	}

	tx = x;
	ty = y;
}

///////////////////

void GSLocalMemory::ReadTexture32(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	FOREACH_BLOCK_START(r, 8, 8, 32)
	{
		ReadBlock32((BYTE*)&m_vm32[BlockAddress32(x, y, TEX0.TBP0, TEX0.TBW)], ptr + (x - r.left) * 4, dstpitch);
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::ReadTexture24(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	__declspec(align(16)) DWORD block[8 * 8];

	FOREACH_BLOCK_START(r, 8, 8, 32)
	{
		ReadBlock32((BYTE*)&m_vm32[BlockAddress32(x, y, TEX0.TBP0, TEX0.TBW)], (BYTE*)block, sizeof(block) / 8);

		ExpandBlock24(block, (DWORD*)ptr + (x - r.left), dstpitch, &TEXA);
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::ReadTexture16(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	__declspec(align(16)) WORD block[16 * 8];

	FOREACH_BLOCK_START(r, 16, 8, 16)
	{
		ReadBlock16((BYTE*)&m_vm16[BlockAddress16(x, y, TEX0.TBP0, TEX0.TBW)], (BYTE*)block, sizeof(block) / 8);

		ExpandBlock16(block, (DWORD*)ptr + (x - r.left), dstpitch, &TEXA);
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::ReadTexture16S(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	__declspec(align(16)) WORD block[16 * 8];

	FOREACH_BLOCK_START(r, 16, 8, 16S)
	{
		ReadBlock16((BYTE*)&m_vm16[BlockAddress16S(x, y, TEX0.TBP0, TEX0.TBW)], (BYTE*)block, sizeof(block) / 8);

		ExpandBlock16(block, (DWORD*)ptr + (x - r.left), dstpitch, &TEXA);
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::ReadTexture8(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	__declspec(align(16)) BYTE block[16 * 16];

	DWORD* pal = m_pCLUT32;

	FOREACH_BLOCK_START(r, 16, 16, 8)
	{
		ReadBlock8((BYTE*)&m_vm8[BlockAddress8(x, y, TEX0.TBP0, TEX0.TBW)], (BYTE*)block, sizeof(block) / 16);

		ExpandBlock8(block, (DWORD*)ptr + (x - r.left), dstpitch, pal);
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::ReadTexture4(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	__declspec(align(16)) BYTE block[(32 / 2) * 16];

	DWORD* pal32 = m_pCLUT32;
	UINT64* pal64 = m_pCLUT64;

	FOREACH_BLOCK_START(r, 32, 16, 4)
	{
		ReadBlock4((BYTE*)&m_vm8[BlockAddress4(x, y, TEX0.TBP0, TEX0.TBW)>>1], (BYTE*)block, sizeof(block) / 16);

		ExpandBlock4(block, (DWORD*)ptr + (x - r.left), dstpitch, pal32, pal64);
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::ReadTexture8H(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	__declspec(align(16)) DWORD block[8 * 8];

	DWORD* pal = m_pCLUT32;

	FOREACH_BLOCK_START(r, 8, 8, 32)
	{
		ReadBlock32((BYTE*)&m_vm32[BlockAddress32(x, y, TEX0.TBP0, TEX0.TBW)], (BYTE*)block, sizeof(block) / 8);

		ExpandBlock8H(block, (DWORD*)ptr + (x - r.left), dstpitch, pal);
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::ReadTexture4HL(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	__declspec(align(16)) DWORD block[8 * 8];

	DWORD* pal = m_pCLUT32;

	FOREACH_BLOCK_START(r, 8, 8, 32)
	{
		ReadBlock32((BYTE*)&m_vm32[BlockAddress32(x, y, TEX0.TBP0, TEX0.TBW)], (BYTE*)block, sizeof(block) / 8);

		ExpandBlock4HL(block, (DWORD*)ptr + (x - r.left), dstpitch, pal);
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::ReadTexture4HH(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	__declspec(align(16)) DWORD block[8 * 8];

	DWORD* pal = m_pCLUT32;

	FOREACH_BLOCK_START(r, 8, 8, 32)
	{
		ReadBlock32((BYTE*)&m_vm32[BlockAddress32(x, y, TEX0.TBP0, TEX0.TBW)], (BYTE*)block, sizeof(block)/8);

		ExpandBlock4HH(block, (DWORD*)ptr + (x - r.left), dstpitch, pal);
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::ReadTexture32Z(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	FOREACH_BLOCK_START(r, 8, 8, 32)
	{
		ReadBlock32((BYTE*)&m_vm32[BlockAddress32Z(x, y, TEX0.TBP0, TEX0.TBW)], ptr + (x - r.left) * 4, dstpitch);
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::ReadTexture24Z(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	__declspec(align(16)) DWORD block[8 * 8];

	FOREACH_BLOCK_START(r, 8, 8, 32)
	{
		ReadBlock32((BYTE*)&m_vm32[BlockAddress32Z(x, y, TEX0.TBP0, TEX0.TBW)], (BYTE*)block, sizeof(block) / 8);

		ExpandBlock24(block, (DWORD*)ptr + (x - r.left), dstpitch, &TEXA);
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::ReadTexture16Z(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	__declspec(align(16)) WORD block[16 * 8];

	FOREACH_BLOCK_START(r, 16, 8, 16)
	{
		ReadBlock16((BYTE*)&m_vm16[BlockAddress16Z(x, y, TEX0.TBP0, TEX0.TBW)], (BYTE*)block, sizeof(block) / 8);

		ExpandBlock16(block, (DWORD*)ptr + (x - r.left), dstpitch, &TEXA);
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::ReadTexture16SZ(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	__declspec(align(16)) WORD block[16 * 8];

	FOREACH_BLOCK_START(r, 16, 8, 16S)
	{
		ReadBlock16((BYTE*)&m_vm16[BlockAddress16SZ(x, y, TEX0.TBP0, TEX0.TBW)], (BYTE*)block, sizeof(block) / 8);

		ExpandBlock16(block, (DWORD*)ptr + (x - r.left), dstpitch, &TEXA);
	}
	FOREACH_BLOCK_END
}

///////////////////

void GSLocalMemory::ReadTexture(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA, GIFRegCLAMP& CLAMP)
{
	readTexture rtx = m_psm[TEX0.PSM].rtx;
	readTexel rt = m_psm[TEX0.PSM].rt;
	CSize bs = m_psm[TEX0.PSM].bs;

	if(r.Width() < bs.cx || r.Height() < bs.cy 
	|| (r.left & (bs.cx-1)) || (r.top & (bs.cy-1)) 
	|| (r.right & (bs.cx-1)) || (r.bottom & (bs.cy-1)) 
	|| (CLAMP.WMS == 3) || (CLAMP.WMT == 3))
	{
		ReadTexture<DWORD>(r, dst, dstpitch, TEX0, TEXA, CLAMP, rt, rtx);
	}
	else
	{
		(this->*rtx)(r, dst, dstpitch, TEX0, TEXA);
	}
}

void GSLocalMemory::ReadTextureNC(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA, GIFRegCLAMP& CLAMP)
{
	readTexture rtx = m_psm[TEX0.PSM].rtx;
	readTexel rt = m_psm[TEX0.PSM].rt;
	CSize bs = m_psm[TEX0.PSM].bs;

	if(r.Width() < bs.cx || r.Height() < bs.cy 
	|| (r.left & (bs.cx-1)) || (r.top & (bs.cy-1)) 
	|| (r.right & (bs.cx-1)) || (r.bottom & (bs.cy-1)))
	{
		ReadTextureNC<DWORD>(r, dst, dstpitch, TEX0, TEXA, rt, rtx);
	}
	else
	{
		(this->*rtx)(r, dst, dstpitch, TEX0, TEXA);
	}
}
///////////////////

void GSLocalMemory::ReadTexture16NP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	FOREACH_BLOCK_START(r, 16, 8, 16)
	{
		ReadBlock16((BYTE*)&m_vm16[BlockAddress16(x, y, TEX0.TBP0, TEX0.TBW)], ptr + (x - r.left) * 2, dstpitch);
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::ReadTexture16SNP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	FOREACH_BLOCK_START(r, 16, 8, 16S)
	{
		ReadBlock16((BYTE*)&m_vm16[BlockAddress16S(x, y, TEX0.TBP0, TEX0.TBW)], ptr + (x - r.left) * 2, dstpitch);
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::ReadTexture8NP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	__declspec(align(16)) BYTE block[16 * 16];

	DWORD* pal = m_pCLUT32;

	FOREACH_BLOCK_START(r, 16, 16, 8)
	{
		ReadBlock8((BYTE*)&m_vm8[BlockAddress8(x, y, TEX0.TBP0, TEX0.TBW)], (BYTE*)block, sizeof(block) / 16);

		if(TEX0.CPSM == PSM_PSMCT32 || TEX0.CPSM == PSM_PSMCT24)
		{
			ExpandBlock8(block, (DWORD*)ptr + (x - r.left), dstpitch, pal);
		}
		else
		{
			ASSERT(TEX0.CPSM == PSM_PSMCT16 || TEX0.CPSM == PSM_PSMCT16S);

			ExpandBlock8(block, (WORD*)ptr + (x - r.left), dstpitch, pal);
		}
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::ReadTexture4NP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	__declspec(align(16)) BYTE block[(32 / 2) * 16];

	DWORD* pal32 = m_pCLUT32;
	UINT64* pal64 = m_pCLUT64;

	FOREACH_BLOCK_START(r, 32, 16, 4)
	{
		ReadBlock4((BYTE*)&m_vm8[BlockAddress4(x, y, TEX0.TBP0, TEX0.TBW)>>1], (BYTE*)block, sizeof(block) / 16);

		if(TEX0.CPSM == PSM_PSMCT32 || TEX0.CPSM == PSM_PSMCT24)
		{
			ExpandBlock4(block, (DWORD*)ptr + (x - r.left), dstpitch, pal32, pal64);
		}
		else
		{
			ASSERT(TEX0.CPSM == PSM_PSMCT16 || TEX0.CPSM == PSM_PSMCT16S);

			ExpandBlock4(block, (WORD*)ptr + (x - r.left), dstpitch, pal64);
		}
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::ReadTexture8HNP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	__declspec(align(16)) DWORD block[8 * 8];

	DWORD* pal = m_pCLUT32;

	FOREACH_BLOCK_START(r, 8, 8, 32)
	{
		ReadBlock32((BYTE*)&m_vm32[BlockAddress32(x, y, TEX0.TBP0, TEX0.TBW)], (BYTE*)block, sizeof(block) / 8);

		if(TEX0.CPSM == PSM_PSMCT32 || TEX0.CPSM == PSM_PSMCT24)
		{
			ExpandBlock8H(block, (DWORD*)ptr + (x - r.left), dstpitch, pal);
		}
		else
		{
			ASSERT(TEX0.CPSM == PSM_PSMCT16 || TEX0.CPSM == PSM_PSMCT16S);

			ExpandBlock8H(block, (WORD*)ptr + (x - r.left), dstpitch, pal);
		}
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::ReadTexture4HLNP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	__declspec(align(16)) DWORD block[8 * 8];

	DWORD* pal = m_pCLUT32;

	FOREACH_BLOCK_START(r, 8, 8, 32)
	{
		ReadBlock32((BYTE*)&m_vm32[BlockAddress32(x, y, TEX0.TBP0, TEX0.TBW)], (BYTE*)block, sizeof(block) / 8);

		if(TEX0.CPSM == PSM_PSMCT32 || TEX0.CPSM == PSM_PSMCT24)
		{
			ExpandBlock4HL(block, (DWORD*)ptr + (x - r.left), dstpitch, pal);
		}
		else
		{
			ASSERT(TEX0.CPSM == PSM_PSMCT16 || TEX0.CPSM == PSM_PSMCT16S);

			ExpandBlock4HL(block, (WORD*)ptr + (x - r.left), dstpitch, pal);
		}
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::ReadTexture4HHNP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	__declspec(align(16)) DWORD block[8 * 8];

	DWORD* pal = m_pCLUT32;

	FOREACH_BLOCK_START(r, 8, 8, 32)
	{
		ReadBlock32((BYTE*)&m_vm32[BlockAddress32(x, y, TEX0.TBP0, TEX0.TBW)], (BYTE*)block, sizeof(block)/8);

		if(TEX0.CPSM == PSM_PSMCT32 || TEX0.CPSM == PSM_PSMCT24)
		{
			ExpandBlock4HH(block, (DWORD*)ptr + (x - r.left), dstpitch, pal);
		}
		else
		{
			ASSERT(TEX0.CPSM == PSM_PSMCT16 || TEX0.CPSM == PSM_PSMCT16S);

			ExpandBlock4HH(block, (WORD*)ptr + (x - r.left), dstpitch, pal);
		}
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::ReadTexture16ZNP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	FOREACH_BLOCK_START(r, 16, 8, 16)
	{
		ReadBlock16((BYTE*)&m_vm16[BlockAddress16Z(x, y, TEX0.TBP0, TEX0.TBW)], ptr + (x - r.left) * 2, dstpitch);
	}
	FOREACH_BLOCK_END
}

void GSLocalMemory::ReadTexture16SZNP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA)
{
	FOREACH_BLOCK_START(r, 16, 8, 16S)
	{
		ReadBlock16((BYTE*)&m_vm16[BlockAddress16SZ(x, y, TEX0.TBP0, TEX0.TBW)], ptr + (x - r.left) * 2, dstpitch);
	}
	FOREACH_BLOCK_END
}

///////////////////

void GSLocalMemory::ReadTextureNP(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA, GIFRegCLAMP& CLAMP)
{
	readTexture rtx = m_psm[TEX0.PSM].rtxNP;
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
			ReadTexture<DWORD>(r, dst, dstpitch, TEX0, TEXA, CLAMP, rt, rtx);
			break;
		case PSM_PSMCT16:
		case PSM_PSMCT16S:
			ReadTexture<WORD>(r, dst, dstpitch, TEX0, TEXA, CLAMP, rt, rtx);
			break;
		}
	}
	else
	{
		(this->*rtx)(r, dst, dstpitch, TEX0, TEXA);
	}
}

void GSLocalMemory::ReadTextureNPNC(const CRect& r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA, GIFRegCLAMP& CLAMP)
{
	readTexture rtx = m_psm[TEX0.PSM].rtxNP;
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
			ReadTextureNC<DWORD>(r, dst, dstpitch, TEX0, TEXA, rt, rtx);
			break;
		case PSM_PSMCT16:
		case PSM_PSMCT16S:
			ReadTextureNC<WORD>(r, dst, dstpitch, TEX0, TEXA, rt, rtx);
			break;
		}
	}
	else
	{
		(this->*rtx)(r, dst, dstpitch, TEX0, TEXA);
	}
}

//

template<typename T> 
void GSLocalMemory::ReadTexture(CRect r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA, GIFRegCLAMP& CLAMP, readTexel rt, readTexture rtx)
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

				(this->*rtx)(CRect(CPoint(maxu, maxv), CSize(w, h)), (BYTE*)buff, w * sizeof(T), TEX0, TEXA);

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

				(this->*rtx)(CRect(CPoint(maxu, top), CSize(w, h)), (BYTE*)buff, w * sizeof(T), TEX0, TEXA);

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

				(this->*rtx)(CRect(CPoint(left, maxv), CSize(w, h)), (BYTE*)buff, w * sizeof(T), TEX0, TEXA);

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
				(this->*rtx)(cr, dst + (cr.left - r.left) * sizeof(T), dstpitch, TEX0, TEXA);
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
void GSLocalMemory::ReadTextureNC(CRect r, BYTE* dst, int dstpitch, GIFRegTEX0& TEX0, GIFRegTEXA& TEXA, readTexel rt, readTexture rtx)
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
			(this->*rtx)(cr, dst + (cr.left - r.left) * sizeof(T), dstpitch, TEX0, TEXA);

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

static const GSVector4i s_rb16mask(0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15);
static const GSVector4i s_rb8mask(0, 4, 2, 6, 8, 12, 10, 14, 1, 5, 3, 7, 9, 13, 11, 15);

void GSLocalMemory::ReadBlock32(BYTE* src, BYTE* dst, int dstpitch)
{
#if _M_SSE >= 0x200

	GSVector4i* s = (GSVector4i*)src;

	GSVector4i v0, v1, v2, v3;

	for(int i = 0; i < 4; i++)
	{
		v0 = s[i * 4 + 0]; 
		v1 = s[i * 4 + 1]; 
		v2 = s[i * 4 + 2]; 
		v3 = s[i * 4 + 3];

		GSVector4i::sw64(v0, v1, v2, v3);

		*(GSVector4i*)&dst[dstpitch * (i * 2 + 0) +  0] = v0;
		*(GSVector4i*)&dst[dstpitch * (i * 2 + 0) + 16] = v1;
		*(GSVector4i*)&dst[dstpitch * (i * 2 + 1) +  0] = v2;
		*(GSVector4i*)&dst[dstpitch * (i * 2 + 1) + 16] = v3;
	}

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

void GSLocalMemory::ReadBlock16(BYTE* src, BYTE* dst, int dstpitch)
{
#if _M_SSE >= 0x301

	GSVector4i* s = (GSVector4i*)src;

	GSVector4i v0, v1, v2, v3;

	GSVector4i mask = s_rb16mask;

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

void GSLocalMemory::ReadBlock8(BYTE* src, BYTE* dst, int dstpitch)
{
#if _M_SSE >= 0x301

	GSVector4i* s = (GSVector4i*)src;

	GSVector4i v0, v1, v2, v3;

	GSVector4i mask = s_rb8mask;

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

void GSLocalMemory::ReadBlock4(BYTE* src, BYTE* dst, int dstpitch)
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

void GSLocalMemory::WriteBlock32(BYTE* dst, BYTE* src, int srcpitch)
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

		GSVector4i::sw64(v0, v2, v1, v3);

		d[i * 4 + 0] = v0;
		d[i * 4 + 1] = v1;
		d[i * 4 + 2] = v2;
		d[i * 4 + 3] = v3;
	}

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

void GSLocalMemory::WriteBlock32(BYTE* dst, BYTE* src, int srcpitch, DWORD mask)
{
#if _M_SSE >= 0x200

	GSVector4i* d = (GSVector4i*)dst;

	GSVector4i vm((int)mask);
	
	GSVector4i v0, v1, v2, v3;

	for(int i = 0; i < 4; i++)
	{
		v0 = *(GSVector4i*)&src[srcpitch * (i * 2 + 0) +  0];
		v1 = *(GSVector4i*)&src[srcpitch * (i * 2 + 0) + 16];
		v2 = *(GSVector4i*)&src[srcpitch * (i * 2 + 1) +  0];
		v3 = *(GSVector4i*)&src[srcpitch * (i * 2 + 1) + 16];

		GSVector4i::sw64(v0, v2, v1, v3);

		d[i * 4 + 0] = d[i * 4 + 0].blend(v0, vm);
		d[i * 4 + 1] = d[i * 4 + 1].blend(v1, vm);
		d[i * 4 + 2] = d[i * 4 + 2].blend(v2, vm);
		d[i * 4 + 3] = d[i * 4 + 3].blend(v3, vm);
	}

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

void GSLocalMemory::WriteBlock32u(BYTE* dst, BYTE* src, int srcpitch)
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

		GSVector4i::sw64(v0, v2, v1, v3);

		d[i * 4 + 0] = v0;
		d[i * 4 + 1] = v1;
		d[i * 4 + 2] = v2;
		d[i * 4 + 3] = v3;
	}

#else

	WriteBlock32(dst, src, srcpitch);

#endif
}

void GSLocalMemory::WriteBlock32u(BYTE* dst, BYTE* src, int srcpitch, DWORD mask)
{
#if _M_SSE >= 0x200

	GSVector4i* d = (GSVector4i*)dst;

	GSVector4i vm((int)mask);
	
	GSVector4i v0, v1, v2, v3;

	for(int i = 0; i < 4; i++)
	{
		v0 = GSVector4i::loadu(&src[srcpitch * (i * 2 + 0) +  0]);
		v1 = GSVector4i::loadu(&src[srcpitch * (i * 2 + 0) + 16]);
		v2 = GSVector4i::loadu(&src[srcpitch * (i * 2 + 1) +  0]);
		v3 = GSVector4i::loadu(&src[srcpitch * (i * 2 + 1) + 16]);

		GSVector4i::sw64(v0, v2, v1, v3);

		d[i * 4 + 0] = d[i * 4 + 0].blend(v0, vm);
		d[i * 4 + 1] = d[i * 4 + 1].blend(v1, vm);
		d[i * 4 + 2] = d[i * 4 + 2].blend(v2, vm);
		d[i * 4 + 3] = d[i * 4 + 3].blend(v3, vm);
	}

#else

	WriteBlock32(dst, src, srcpitch, mask);

#endif
}

void GSLocalMemory::WriteBlock16(BYTE* dst, BYTE* src, int srcpitch)
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

void GSLocalMemory::WriteBlock16u(BYTE* dst, BYTE* src, int srcpitch)
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

	WriteBlock16(dst, src, srcpitch);

#endif
}

void GSLocalMemory::WriteBlock8(BYTE* dst, BYTE* src, int srcpitch)
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

void GSLocalMemory::WriteBlock8u(BYTE* dst, BYTE* src, int srcpitch)
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

	WriteBlock8(dst, src, srcpitch);

#endif
}

void GSLocalMemory::WriteBlock4(BYTE* dst, BYTE* src, int srcpitch)
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

void GSLocalMemory::WriteBlock4u(BYTE* dst, BYTE* src, int srcpitch)
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

	WriteBlock4(dst, src, srcpitch);

#endif
}

void GSLocalMemory::WriteColumn32(int y, BYTE* dst, BYTE* src, int srcpitch)
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

void GSLocalMemory::WriteColumn32(int y, BYTE* dst, BYTE* src, int srcpitch, DWORD mask)
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

void GSLocalMemory::WriteColumn16(int y, BYTE* dst, BYTE* src, int srcpitch)
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

void GSLocalMemory::WriteColumn8(int y, BYTE* dst, BYTE* src, int srcpitch)
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

void GSLocalMemory::WriteColumn4(int y, BYTE* dst, BYTE* src, int srcpitch)
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
	GSVector4i v0 = s[0];
	GSVector4i v1 = s[1];
	GSVector4i v2 = s[2];
	GSVector4i v3 = s[3];

	GSVector4i::sw64(v0, v1, v2, v3);
	GSVector4i::sw16(v0, v1, v2, v3);
	GSVector4i::sw16(v0, v2, v1, v3);
	GSVector4i::sw16(v0, v1, v2, v3);

	d[0] = v0;
	d[1] = v1;
	d[32] = v2;
	d[33] = v3;
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
		GSVector4i v0 = s[i + 0];
		GSVector4i v1 = s[i + 1];
		GSVector4i v2 = s[i + 2];
		GSVector4i v3 = s[i + 3];

		GSVector4i::sw16(v0, v1, v2, v3);
		GSVector4i::sw32(v0, v1, v2, v3);
		GSVector4i::sw16(v0, v2, v1, v3);

		d[i + 0] = v0;
		d[i + 1] = v2;
		d[i + 2] = v1;
		d[i + 3] = v3;
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

		r0 = r0.srl<12>(r1);

		d[1] = r0.upl32(r0.srl<3>()).upl64(r0.srl<6>().upl32(r0.srl<9>())) & rgbm;

		r0 = r1.srl<8>(r2);

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

void GSLocalMemory::ExpandBlock8(BYTE* src, DWORD* dst, int dstpitch, DWORD* pal)
{
	for(int j = 0; j < 16; j++, dst += dstpitch >> 2)
	{
		#if _M_SSE >= 0x400

		GSVector4i* s = (GSVector4i*)src;
		GSVector4i* d = (GSVector4i*)dst;

		GSVector4i v = s[j];

		d[0] = v.gather32_8<0>(pal);
		d[1] = v.gather32_8<4>(pal);
		d[2] = v.gather32_8<8>(pal);
		d[3] = v.gather32_8<12>(pal);

		#else

		for(int i = 0; i < 16; i++)
		{
			dst[i] = pal[src[j * 16 + i]];
		}

		#endif
	}
}

void GSLocalMemory::ExpandBlock8(BYTE* src, WORD* dst, int dstpitch, DWORD* pal)
{
	for(int j = 0; j < 16; j++, dst += dstpitch >> 1)
	{
		#if _M_SSE >= 0x400

		GSVector4i* s = (GSVector4i*)src;
		GSVector4i* d = (GSVector4i*)dst;

		GSVector4i v = s[j];

		d[0] = v.gather16_8<0>(pal);
		d[1] = v.gather16_8<8>(pal);

		#else

		for(int i = 0; i < 16; i++)
		{
			dst[i] = (WORD)pal[src[j * 16 + i]];
		}

		#endif
	}
}

void GSLocalMemory::ExpandBlock4(BYTE* src, DWORD* dst, int dstpitch, DWORD* pal32, UINT64* pal64)
{
	for(int j = 0; j < 16; j++, dst += dstpitch >> 2)
	{
		#if _M_SSE >= 0x400 && defined(_M_AMD64)

		GSVector4i* s = (GSVector4i*)src;
		GSVector4i* d = (GSVector4i*)dst;

		GSVector4i v = s[j];

		d[0] = v.gather64_8<0>(pal64);
		d[1] = v.gather64_8<2>(pal64);
		d[2] = v.gather64_8<4>(pal64);
		d[3] = v.gather64_8<6>(pal64);
		d[4] = v.gather64_8<8>(pal64);
		d[5] = v.gather64_8<10>(pal64);
		d[6] = v.gather64_8<12>(pal64);
		d[7] = v.gather64_8<14>(pal64);

		#elif 0 // _M_SSE >= 0x400 // TODO: test if this is actually faster than the normal c version

		GSVector4i* s = (GSVector4i*)src;
		GSVector4i* d = (GSVector4i*)dst;

		GSVector4i v = s[j];

		d[0] = v.gather32_4<0>(pal32);
		d[1] = v.gather32_4<2>(pal32);
		d[2] = v.gather32_4<4>(pal32);
		d[3] = v.gather32_4<6>(pal32);
		d[4] = v.gather32_4<8>(pal32);
		d[5] = v.gather32_4<10>(pal32);
		d[6] = v.gather32_4<12>(pal32);
		d[7] = v.gather32_4<14>(pal32);

		#else

		for(int i = 0; i < 32 / 2; i++)
		{
			((UINT64*)dst)[i] = pal64[src[j * 16 + i]];
		}

		#endif
	}
}

void GSLocalMemory::ExpandBlock4(BYTE* src, WORD* dst, int dstpitch, UINT64* pal)
{
	for(int j = 0; j < 16; j++, dst += dstpitch >> 1)
	{
		#if _M_SSE >= 0x400

		GSVector4i* s = (GSVector4i*)src;
		GSVector4i* d = (GSVector4i*)dst;

		GSVector4i v = s[j];

		d[0] = v.gather32_8<0>(pal);
		d[1] = v.gather32_8<4>(pal);
		d[2] = v.gather32_8<8>(pal);
		d[3] = v.gather32_8<12>(pal);

		#else

		for(int i = 0; i < 32 / 2; i++)
		{
			((DWORD*)dst)[i] = (DWORD)pal[src[j * 16 + i]];
		}

		#endif
	}
}

void GSLocalMemory::ExpandBlock8H(DWORD* src, DWORD* dst, int dstpitch, DWORD* pal)
{
	for(int j = 0; j < 8; j++, dst += dstpitch >> 2)
	{
		#if _M_SSE >= 0x400

		GSVector4i* s = (GSVector4i*)src;
		GSVector4i* d = (GSVector4i*)dst;

		d[0] = (s[j * 2 + 0] >> 24).gather32_32<>(pal);
		d[1] = (s[j * 2 + 1] >> 24).gather32_32<>(pal);

		#else

		for(int i = 0; i < 8; i++)
		{
			dst[i] = pal[src[j * 8 + i] >> 24];
		}

		#endif
	}
}

void GSLocalMemory::ExpandBlock8H(DWORD* src, WORD* dst, int dstpitch, DWORD* pal)
{
	for(int j = 0; j < 8; j++, dst += dstpitch >> 1)
	{
		#if _M_SSE >= 0x400

		GSVector4i* s = (GSVector4i*)src;
		GSVector4i* d = (GSVector4i*)dst;

		GSVector4i v;

		v = (s[j * 2 + 0] >> 24).gather16_32<0>(pal, v);
		v = (s[j * 2 + 1] >> 24).gather16_32<4>(pal, v);

		d[0] = v;

		#else

		for(int i = 0; i < 8; i++)
		{
			dst[i] = (WORD)pal[src[j * 8 + i] >> 24];
		}

		#endif
	}
}

void GSLocalMemory::ExpandBlock4HL(DWORD* src, DWORD* dst, int dstpitch, DWORD* pal)
{
	for(int j = 0; j < 8; j++, dst += dstpitch >> 2)
	{
		#if _M_SSE >= 0x400

		GSVector4i* s = (GSVector4i*)src;
		GSVector4i* d = (GSVector4i*)dst;

		d[0] = ((s[j * 2 + 0] >> 24) & 0xf).gather32_32<>(pal);
		d[1] = ((s[j * 2 + 1] >> 24) & 0xf).gather32_32<>(pal);

		#else

		for(int i = 0; i < 8; i++)
		{
			dst[i] = pal[(src[j * 8 + i] >> 24) & 0xf];
		}

		#endif
	}
}

void GSLocalMemory::ExpandBlock4HL(DWORD* src, WORD* dst, int dstpitch, DWORD* pal)
{
	for(int j = 0; j < 8; j++, dst += dstpitch >> 2)
	{
		#if _M_SSE >= 0x400

		GSVector4i* s = (GSVector4i*)src;
		GSVector4i* d = (GSVector4i*)dst;

		GSVector4i v;

		v = ((s[j * 2 + 0] >> 24) & 0xf).gather16_32<0>(pal, v);
		v = ((s[j * 2 + 1] >> 24) & 0xf).gather16_32<4>(pal, v);

		d[0] = v;

		#else

		for(int i = 0; i < 8; i++)
		{
			dst[i] = (WORD)pal[(src[j * 8 + i] >> 24) & 0xf];
		}

		#endif
	}
}

void GSLocalMemory::ExpandBlock4HH(DWORD* src, DWORD* dst, int dstpitch, DWORD* pal)
{
	for(int j = 0; j < 8; j++, dst += dstpitch >> 2)
	{
		#if _M_SSE >= 0x400

		GSVector4i* s = (GSVector4i*)src;
		GSVector4i* d = (GSVector4i*)dst;

		d[0] = (s[j * 2 + 0] >> 28).gather32_32<>(pal);
		d[1] = (s[j * 2 + 1] >> 28).gather32_32<>(pal);

		#else

		for(int i = 0; i < 8; i++)
		{
			dst[i] = pal[src[j * 8 + i] >> 28];
		}

		#endif
	}
}

void GSLocalMemory::ExpandBlock4HH(DWORD* src, WORD* dst, int dstpitch, DWORD* pal)
{
	for(int j = 0; j < 8; j++, dst += dstpitch >> 2)
	{
		#if _M_SSE >= 0x400

		GSVector4i* s = (GSVector4i*)src;
		GSVector4i* d = (GSVector4i*)dst;

		GSVector4i v;

		v = (s[j * 2 + 0] >> 28).gather16_32<0>(pal, v);
		v = (s[j * 2 + 1] >> 28).gather16_32<4>(pal, v);

		d[0] = v;

		#else

		for(int i = 0; i < 8; i++)
		{
			dst[i] = (WORD)pal[src[j * 8 + i] >> 28];
		}

		#endif
	}
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

	// (this->*m_psm[TEX0.PSM].rtx)(CRect(0, 0, w, h), bits, pitch, TEX0, TEXA);

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
