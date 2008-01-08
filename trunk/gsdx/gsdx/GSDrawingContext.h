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
#include "GSLocalMemory.h"

#pragma pack(push, 1)

struct GSDrawingContext
{
	struct GSDrawingContext() {memset(this, 0, sizeof(*this));}

	GIFRegXYOFFSET	XYOFFSET;
	GIFRegTEX0		TEX0;
	GIFRegTEX1		TEX1;
	GIFRegTEX2		TEX2;
	GIFRegCLAMP		CLAMP;
	GIFRegMIPTBP1	MIPTBP1;
	GIFRegMIPTBP2	MIPTBP2;
	GIFRegSCISSOR	SCISSOR;
	GIFRegALPHA		ALPHA;
	GIFRegTEST		TEST;
	GIFRegFBA		FBA;
	GIFRegFRAME		FRAME;
	GIFRegZBUF		ZBUF;

	GSLocalMemory::psm_t* ftbl;
	GSLocalMemory::psm_t* ztbl;
	GSLocalMemory::psm_t* ttbl;

	struct {DWORD x0, y0, x1, y1;} scissor;
	struct {float x0, y0, x1, y1;} fscissor;

	void UpdateScissor()
	{
		scissor.x0 = (SCISSOR.SCAX0 << 4) + XYOFFSET.OFX;
		scissor.y0 = (SCISSOR.SCAY0 << 4) + XYOFFSET.OFY;
		scissor.x1 = (SCISSOR.SCAX1 << 4) + XYOFFSET.OFX;
		scissor.y1 = (SCISSOR.SCAY1 << 4) + XYOFFSET.OFY;
		fscissor.x0 = (float)(int)scissor.x0;
		fscissor.y1 = (float)(int)scissor.y0;
		fscissor.x1 = (float)(int)scissor.x1;
		fscissor.y1 = (float)(int)scissor.y1;
	}

	bool DepthRead()
	{
		return TEST.ZTE && TEST.ZTST >= 2;
	}

	bool DepthWrite()
	{
		if(TEST.ATE && TEST.ATST == 0 && TEST.AFAIL != 2) // alpha test, all pixels fail, z buffer is not updated
		{
			return false;
		}

		return ZBUF.ZMSK == 0;
	}
};

#pragma pack(pop)