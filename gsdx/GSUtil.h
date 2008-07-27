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

class GSUtil
{
public:
	static int GetPrimClass(DWORD prim);
	static int GetPrimVertexCount(DWORD prim);
	/*
	static int GetPrimClass(DWORD prim)
	{
		switch(prim)
		{
		case GS_POINTLIST: return 0;
		case GS_LINELIST: return 1;
		case GS_LINESTRIP: return 1;
		case GS_TRIANGLELIST: return 2;
		case GS_TRIANGLESTRIP: return 2;
		case GS_TRIANGLEFAN: return 2;
		case GS_SPRITE: return 3;
		case GS_INVALID: return -1;
		default: __assume(0);
		}
	}

	static int GetPrimVertexCount(DWORD prim)
	{
		switch(prim)
		{
		case GS_POINTLIST: return 1;
		case GS_LINELIST: return 2;
		case GS_LINESTRIP: return 2;
		case GS_TRIANGLELIST: return 3;
		case GS_TRIANGLESTRIP: return 3;
		case GS_TRIANGLEFAN: return 3;
		case GS_SPRITE: return 2;
		case GS_INVALID: return 1;
		default: __assume(0);
		}
	}
	*/
	static bool HasSharedBits(DWORD spsm, DWORD dpsm);
	static bool HasSharedBits(DWORD sbp, DWORD spsm, DWORD dbp, DWORD dpsm);
	static bool HasCompatibleBits(DWORD spsm, DWORD dpsm);

	static bool IsRectInRect(const CRect& inner, const CRect& outer);
	static bool IsRectInRectH(const CRect& inner, const CRect& outer);
	static bool IsRectInRectV(const CRect& inner, const CRect& outer);
	
	static int EncodePSM(int psm)
	{
		switch(psm)
		{
		case PSM_PSMCT32: 
		case PSM_PSMZ32: 
			return 0;
		case PSM_PSMCT24: 
		case PSM_PSMZ24:
			return 1;
		case PSM_PSMCT16: 
		case PSM_PSMCT16S: 
		case PSM_PSMZ16:
		case PSM_PSMZ16S:
			return 2;
		default:
			return 3;
		}
	}
};

