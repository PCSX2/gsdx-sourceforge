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

#include "stdafx.h"
#include "GS.h"
#include "GSUtil.h"

static struct GSUtilMaps
{
	BYTE PrimClassField[8];
	BYTE PrimVertexCount[8];
	bool CompatibleBitsField[64][64];
	bool SharedBitsField[64][64];

	struct GSUtilMaps()
	{
		PrimClassField[GS_POINTLIST] = 0;
		PrimClassField[GS_LINELIST] = 1;
		PrimClassField[GS_LINESTRIP] = 1;
		PrimClassField[GS_TRIANGLELIST] = 2;
		PrimClassField[GS_TRIANGLESTRIP] = 2;
		PrimClassField[GS_TRIANGLEFAN] = 2;
		PrimClassField[GS_SPRITE] = 3;
		PrimClassField[GS_INVALID] = (BYTE)-1;

		PrimVertexCount[GS_POINTLIST] = 1;
		PrimVertexCount[GS_LINELIST] = 2;
		PrimVertexCount[GS_LINESTRIP] = 2;
		PrimVertexCount[GS_TRIANGLELIST] = 3;
		PrimVertexCount[GS_TRIANGLESTRIP] = 3;
		PrimVertexCount[GS_TRIANGLEFAN] = 3;
		PrimVertexCount[GS_SPRITE] = 2;
		PrimVertexCount[GS_INVALID] = 1;

		memset(CompatibleBitsField, 0, sizeof(CompatibleBitsField));

		CompatibleBitsField[PSM_PSMCT32][PSM_PSMCT24] = true;
		CompatibleBitsField[PSM_PSMCT24][PSM_PSMCT32] = true;
		CompatibleBitsField[PSM_PSMCT16][PSM_PSMCT16S] = true;
		CompatibleBitsField[PSM_PSMCT16S][PSM_PSMCT16] = true;
		CompatibleBitsField[PSM_PSMZ32][PSM_PSMZ24] = true;
		CompatibleBitsField[PSM_PSMZ24][PSM_PSMZ32] = true;
		CompatibleBitsField[PSM_PSMZ16][PSM_PSMZ16S] = true;
		CompatibleBitsField[PSM_PSMZ16S][PSM_PSMZ16] = true;

		memset(SharedBitsField, 1, sizeof(SharedBitsField));

		SharedBitsField[PSM_PSMCT24][PSM_PSMT8H] = false;
		SharedBitsField[PSM_PSMCT24][PSM_PSMT4HL] = false;
		SharedBitsField[PSM_PSMCT24][PSM_PSMT4HH] = false;
		SharedBitsField[PSM_PSMZ24][PSM_PSMT8H] = false;
		SharedBitsField[PSM_PSMZ24][PSM_PSMT4HL] = false;
		SharedBitsField[PSM_PSMZ24][PSM_PSMT4HH] = false;
		SharedBitsField[PSM_PSMT8H][PSM_PSMCT24] = false;
		SharedBitsField[PSM_PSMT8H][PSM_PSMZ24] = false;
		SharedBitsField[PSM_PSMT4HL][PSM_PSMCT24] = false;
		SharedBitsField[PSM_PSMT4HL][PSM_PSMZ24] = false;
		SharedBitsField[PSM_PSMT4HL][PSM_PSMT4HH] = false;
		SharedBitsField[PSM_PSMT4HH][PSM_PSMCT24] = false;
		SharedBitsField[PSM_PSMT4HH][PSM_PSMZ24] = false;
		SharedBitsField[PSM_PSMT4HH][PSM_PSMT4HL] = false;
	}

} s_maps;

int GSUtil::GetPrimClass(DWORD prim)
{
	return s_maps.PrimClassField[prim];
}

int GSUtil::GetPrimVertexCount(DWORD prim)
{
	return s_maps.PrimVertexCount[prim];
}
bool GSUtil::HasSharedBits(DWORD spsm, DWORD dpsm)
{
	return s_maps.SharedBitsField[spsm][dpsm];
}

bool GSUtil::HasSharedBits(DWORD sbp, DWORD spsm, DWORD dbp, DWORD dpsm)
{
	if(sbp != dbp) return false;

	return HasSharedBits(spsm, dpsm);
}

bool GSUtil::HasCompatibleBits(DWORD spsm, DWORD dpsm)
{
	if(spsm == dpsm) return true;

	return s_maps.CompatibleBitsField[spsm][dpsm];
}

bool GSUtil::IsRectInRect(const CRect& inner, const CRect& outer)
{
	return outer.left <= inner.left && inner.right <= outer.right && outer.top <= inner.top && inner.bottom <= outer.bottom;
}

bool GSUtil::IsRectInRectH(const CRect& inner, const CRect& outer)
{
	return outer.top <= inner.top && inner.bottom <= outer.bottom;
}

bool GSUtil::IsRectInRectV(const CRect& inner, const CRect& outer)
{
	return outer.left <= inner.left && inner.right <= outer.right;
}


