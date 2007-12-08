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

	struct {float x0, y0, x1, y1;} scissor;

	void UpdateScissor()
	{
		scissor.x0 = (float)(int)((int)(SCISSOR.SCAX0 << 4) + (int)XYOFFSET.OFX);
		scissor.y0 = (float)(int)((int)(SCISSOR.SCAY0 << 4) + (int)XYOFFSET.OFY);
		scissor.x1 = (float)(int)((int)(SCISSOR.SCAX1 << 4) + (int)XYOFFSET.OFX);
		scissor.y1 = (float)(int)((int)(SCISSOR.SCAY1 << 4) + (int)XYOFFSET.OFY);
	}
};

#pragma pack(pop)