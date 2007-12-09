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

bool HasSharedBits(DWORD spsm, DWORD dpsm)
{
	switch(spsm)
	{
	case PSM_PSMCT32:
	case PSM_PSMCT16:
	case PSM_PSMCT16S:
	case PSM_PSMT8:
	case PSM_PSMT4:
	case PSM_PSMZ32:
	case PSM_PSMZ16:
	case PSM_PSMZ16S:
		return true;
	case PSM_PSMCT24:
	case PSM_PSMZ24:
		return !(dpsm == PSM_PSMT8H || dpsm == PSM_PSMT4HL || dpsm == PSM_PSMT4HH);
	case PSM_PSMT8H:
		return !(dpsm == PSM_PSMCT24 || dpsm == PSM_PSMZ24);
	case PSM_PSMT4HL:
		return !(dpsm == PSM_PSMCT24 || dpsm == PSM_PSMZ24 || dpsm == PSM_PSMT4HH);
	case PSM_PSMT4HH:
		return !(dpsm == PSM_PSMCT24 || dpsm == PSM_PSMZ24 || dpsm == PSM_PSMT4HL);
	}

	return true;
}

bool HasSharedBits(DWORD sbp, DWORD spsm, DWORD dbp, DWORD dpsm)
{
	if(sbp != dbp) return false;

	return HasSharedBits(spsm, dpsm);
}

bool HasCompatibleBits(DWORD spsm, DWORD dpsm)
{
	if(spsm == dpsm) return true;

	switch(spsm)
	{
	case PSM_PSMCT32:
	case PSM_PSMCT24:
		return dpsm == PSM_PSMCT32 || dpsm == PSM_PSMCT24;
	case PSM_PSMCT16:
	case PSM_PSMCT16S:
		return dpsm == PSM_PSMCT16 || dpsm == PSM_PSMCT16S;
	case PSM_PSMZ32:
	case PSM_PSMZ24:
		return dpsm == PSM_PSMZ32 || dpsm == PSM_PSMZ24;
	case PSM_PSMZ16:
	case PSM_PSMZ16S:
		return dpsm == PSM_PSMZ16 || dpsm == PSM_PSMZ16S;
	}

	return false;
}

int GetPrimClass(DWORD prim)
{
	switch(prim)
	{
	case GS_POINTLIST:
		return 0;
		break;
	case GS_LINELIST:
	case GS_LINESTRIP:
		return 1;
		break;
	case GS_TRIANGLELIST:
	case GS_TRIANGLESTRIP:
	case GS_TRIANGLEFAN:
		return 2;
		break;
	case GS_SPRITE:
		return 3;
		break;
	}

	ASSERT(0);

	return -1;
}

bool IsRectInRect(const CRect& inner, const CRect& outer)
{
	return outer.left <= inner.left && inner.right <= outer.right && outer.top <= inner.top && inner.bottom <= outer.bottom;
}

bool IsRectInRectH(const CRect& inner, const CRect& outer)
{
	return outer.top <= inner.top && inner.bottom <= outer.bottom;
}

bool IsRectInRectV(const CRect& inner, const CRect& outer)
{
	return outer.left <= inner.left && inner.right <= outer.right;
}
