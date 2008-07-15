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

#include "StdAfx.h"
#include "GSRasterizer.h"

void GSRasterizer::InitEx()
{
	// our little "profile guided optimization", gathered from different games

	m_dsmap[0x445068c0] = &GSRasterizer::DrawScanlineEx<0x445068c0>;

	// ffx

	m_dsmap[0x465204c9] = &GSRasterizer::DrawScanlineEx<0x465204c9>;
	m_dsmap[0x000984e9] = &GSRasterizer::DrawScanlineEx<0x000984e9>;
	m_dsmap[0x4251a809] = &GSRasterizer::DrawScanlineEx<0x4251a809>;
	m_dsmap[0xa9504429] = &GSRasterizer::DrawScanlineEx<0xa9504429>;
	m_dsmap[0x0009a8e9] = &GSRasterizer::DrawScanlineEx<0x0009a8e9>;
	m_dsmap[0x4451a809] = &GSRasterizer::DrawScanlineEx<0x4451a809>;
	m_dsmap[0x4851a849] = &GSRasterizer::DrawScanlineEx<0x4851a849>;
	m_dsmap[0x44504409] = &GSRasterizer::DrawScanlineEx<0x44504409>;
	m_dsmap[0x0000512b] = &GSRasterizer::DrawScanlineEx<0x0000512b>;
	m_dsmap[0x00004429] = &GSRasterizer::DrawScanlineEx<0x00004429>;
	m_dsmap[0x4451a8c9] = &GSRasterizer::DrawScanlineEx<0x4451a8c9>;
	m_dsmap[0x4459f8e9] = &GSRasterizer::DrawScanlineEx<0x4459f8e9>;
	m_dsmap[0x4851c489] = &GSRasterizer::DrawScanlineEx<0x4851c489>;
	m_dsmap[0x485184c9] = &GSRasterizer::DrawScanlineEx<0x485184c9>;
	m_dsmap[0x0001a8e9] = &GSRasterizer::DrawScanlineEx<0x0001a8e9>;
	m_dsmap[0x4451f849] = &GSRasterizer::DrawScanlineEx<0x4451f849>;
	m_dsmap[0x4851a8c9] = &GSRasterizer::DrawScanlineEx<0x4851a8c9>;
	m_dsmap[0x44504489] = &GSRasterizer::DrawScanlineEx<0x44504489>;
	m_dsmap[0x445984c9] = &GSRasterizer::DrawScanlineEx<0x445984c9>;
	m_dsmap[0x4451f809] = &GSRasterizer::DrawScanlineEx<0x4451f809>;
	m_dsmap[0x4451a849] = &GSRasterizer::DrawScanlineEx<0x4451a849>;
	m_dsmap[0x6851c849] = &GSRasterizer::DrawScanlineEx<0x6851c849>;
	m_dsmap[0x4459a8c9] = &GSRasterizer::DrawScanlineEx<0x4459a8c9>;
	m_dsmap[0x4851a809] = &GSRasterizer::DrawScanlineEx<0x4851a809>;
	m_dsmap[0x68506869] = &GSRasterizer::DrawScanlineEx<0x68506869>;
	m_dsmap[0x44505029] = &GSRasterizer::DrawScanlineEx<0x44505029>;
	m_dsmap[0x4451f829] = &GSRasterizer::DrawScanlineEx<0x4451f829>;
	m_dsmap[0x44504429] = &GSRasterizer::DrawScanlineEx<0x44504429>;
	m_dsmap[0x4251c489] = &GSRasterizer::DrawScanlineEx<0x4251c489>;
	m_dsmap[0x00006869] = &GSRasterizer::DrawScanlineEx<0x00006869>;
	m_dsmap[0x425184c9] = &GSRasterizer::DrawScanlineEx<0x425184c9>;
	m_dsmap[0x4451c4a9] = &GSRasterizer::DrawScanlineEx<0x4451c4a9>;
	m_dsmap[0x4451f8a9] = &GSRasterizer::DrawScanlineEx<0x4451f8a9>;
	m_dsmap[0x445044a9] = &GSRasterizer::DrawScanlineEx<0x445044a9>;
	m_dsmap[0x645044e9] = &GSRasterizer::DrawScanlineEx<0x645044e9>;
	m_dsmap[0x485044e9] = &GSRasterizer::DrawScanlineEx<0x485044e9>;
	m_dsmap[0x0001a829] = &GSRasterizer::DrawScanlineEx<0x0001a829>;
	m_dsmap[0x445044e9] = &GSRasterizer::DrawScanlineEx<0x445044e9>;
	m_dsmap[0x445184c9] = &GSRasterizer::DrawScanlineEx<0x445184c9>;
	m_dsmap[0x88518409] = &GSRasterizer::DrawScanlineEx<0x88518409>;
	m_dsmap[0x4451f889] = &GSRasterizer::DrawScanlineEx<0x4451f889>;
	m_dsmap[0x4451c429] = &GSRasterizer::DrawScanlineEx<0x4451c429>;
	m_dsmap[0x48504489] = &GSRasterizer::DrawScanlineEx<0x48504489>;
	m_dsmap[0x4451d8a9] = &GSRasterizer::DrawScanlineEx<0x4451d8a9>;
	m_dsmap[0x4451a0c9] = &GSRasterizer::DrawScanlineEx<0x4451a0c9>;
	m_dsmap[0x885184c9] = &GSRasterizer::DrawScanlineEx<0x885184c9>;
	m_dsmap[0x4851f849] = &GSRasterizer::DrawScanlineEx<0x4851f849>;
	m_dsmap[0x64505129] = &GSRasterizer::DrawScanlineEx<0x64505129>;
	m_dsmap[0x445220e9] = &GSRasterizer::DrawScanlineEx<0x445220e9>;
	m_dsmap[0x485204c9] = &GSRasterizer::DrawScanlineEx<0x485204c9>;
	m_dsmap[0x00004428] = &GSRasterizer::DrawScanlineEx<0x00004428>;
	m_dsmap[0x4251a8c9] = &GSRasterizer::DrawScanlineEx<0x4251a8c9>;
	m_dsmap[0x0059a8c9] = &GSRasterizer::DrawScanlineEx<0x0059a8c9>;
	m_dsmap[0x885068e8] = &GSRasterizer::DrawScanlineEx<0x885068e8>;
	m_dsmap[0x48518449] = &GSRasterizer::DrawScanlineEx<0x48518449>;
	m_dsmap[0x4451a009] = &GSRasterizer::DrawScanlineEx<0x4451a009>;
	m_dsmap[0x88518429] = &GSRasterizer::DrawScanlineEx<0x88518429>;
	m_dsmap[0x00004409] = &GSRasterizer::DrawScanlineEx<0x00004409>;
	m_dsmap[0x8851a0c9] = &GSRasterizer::DrawScanlineEx<0x8851a0c9>;
	m_dsmap[0x4251a849] = &GSRasterizer::DrawScanlineEx<0x4251a849>;
	m_dsmap[0x44518429] = &GSRasterizer::DrawScanlineEx<0x44518429>;
	m_dsmap[0x6451a809] = &GSRasterizer::DrawScanlineEx<0x6451a809>;
	m_dsmap[0x4859f8e9] = &GSRasterizer::DrawScanlineEx<0x4859f8e9>;
	m_dsmap[0x8850400b] = &GSRasterizer::DrawScanlineEx<0x8850400b>;
	m_dsmap[0x64507149] = &GSRasterizer::DrawScanlineEx<0x64507149>;
	m_dsmap[0x44518409] = &GSRasterizer::DrawScanlineEx<0x44518409>;
	m_dsmap[0x0001b809] = &GSRasterizer::DrawScanlineEx<0x0001b809>;
	m_dsmap[0x42504489] = &GSRasterizer::DrawScanlineEx<0x42504489>;
	m_dsmap[0x44518489] = &GSRasterizer::DrawScanlineEx<0x44518489>;
	m_dsmap[0x0001b00b] = &GSRasterizer::DrawScanlineEx<0x0001b00b>;
	m_dsmap[0x4251b809] = &GSRasterizer::DrawScanlineEx<0x4251b809>;
	m_dsmap[0x4451b809] = &GSRasterizer::DrawScanlineEx<0x4451b809>;

	// ffxii

	m_dsmap[0x445ba8c8] = &GSRasterizer::DrawScanlineEx<0x445ba8c8>;
	m_dsmap[0x44520428] = &GSRasterizer::DrawScanlineEx<0x44520428>;
	m_dsmap[0x485628c8] = &GSRasterizer::DrawScanlineEx<0x485628c8>;
	m_dsmap[0x485204c8] = &GSRasterizer::DrawScanlineEx<0x485204c8>;
	m_dsmap[0x58560428] = &GSRasterizer::DrawScanlineEx<0x58560428>;
	m_dsmap[0x445628c8] = &GSRasterizer::DrawScanlineEx<0x445628c8>;
	m_dsmap[0x00160408] = &GSRasterizer::DrawScanlineEx<0x00160408>;
	m_dsmap[0x00040428] = &GSRasterizer::DrawScanlineEx<0x00040428>;
	m_dsmap[0x42520448] = &GSRasterizer::DrawScanlineEx<0x42520448>;
	m_dsmap[0x00004428] = &GSRasterizer::DrawScanlineEx<0x00004428>;
	m_dsmap[0x44523828] = &GSRasterizer::DrawScanlineEx<0x44523828>;
	m_dsmap[0x00023828] = &GSRasterizer::DrawScanlineEx<0x00023828>;
	m_dsmap[0x425060a8] = &GSRasterizer::DrawScanlineEx<0x425060a8>;
	m_dsmap[0x485228a8] = &GSRasterizer::DrawScanlineEx<0x485228a8>;
	m_dsmap[0x44563828] = &GSRasterizer::DrawScanlineEx<0x44563828>;
	m_dsmap[0x445228a8] = &GSRasterizer::DrawScanlineEx<0x445228a8>;
	m_dsmap[0x000228a8] = &GSRasterizer::DrawScanlineEx<0x000228a8>;
	m_dsmap[0x48562808] = &GSRasterizer::DrawScanlineEx<0x48562808>;
	m_dsmap[0x445204a8] = &GSRasterizer::DrawScanlineEx<0x445204a8>;
	m_dsmap[0x00304408] = &GSRasterizer::DrawScanlineEx<0x00304408>;
	m_dsmap[0x48522848] = &GSRasterizer::DrawScanlineEx<0x48522848>;
	m_dsmap[0x44522848] = &GSRasterizer::DrawScanlineEx<0x44522848>;
	m_dsmap[0x485604a8] = &GSRasterizer::DrawScanlineEx<0x485604a8>;
	m_dsmap[0x00004408] = &GSRasterizer::DrawScanlineEx<0x00004408>;
	m_dsmap[0x445238a8] = &GSRasterizer::DrawScanlineEx<0x445238a8>;
	m_dsmap[0x48520448] = &GSRasterizer::DrawScanlineEx<0x48520448>;
	m_dsmap[0x44760448] = &GSRasterizer::DrawScanlineEx<0x44760448>;
	m_dsmap[0x445638a8] = &GSRasterizer::DrawScanlineEx<0x445638a8>;
	m_dsmap[0x48561828] = &GSRasterizer::DrawScanlineEx<0x48561828>;
	m_dsmap[0x44561828] = &GSRasterizer::DrawScanlineEx<0x44561828>;
	m_dsmap[0x49522848] = &GSRasterizer::DrawScanlineEx<0x49522848>;
	m_dsmap[0x445060a8] = &GSRasterizer::DrawScanlineEx<0x445060a8>;
	m_dsmap[0x445620a8] = &GSRasterizer::DrawScanlineEx<0x445620a8>;
	m_dsmap[0x485b68c8] = &GSRasterizer::DrawScanlineEx<0x485b68c8>;
	m_dsmap[0x445b68c8] = &GSRasterizer::DrawScanlineEx<0x445b68c8>;
	m_dsmap[0x000968c8] = &GSRasterizer::DrawScanlineEx<0x000968c8>;
	m_dsmap[0x485228c8] = &GSRasterizer::DrawScanlineEx<0x485228c8>;
	m_dsmap[0x000944c8] = &GSRasterizer::DrawScanlineEx<0x000944c8>;
	m_dsmap[0x445228c8] = &GSRasterizer::DrawScanlineEx<0x445228c8>;
	m_dsmap[0xa8704408] = &GSRasterizer::DrawScanlineEx<0xa8704408>;
	m_dsmap[0x44504428] = &GSRasterizer::DrawScanlineEx<0x44504428>;
	m_dsmap[0x445068a8] = &GSRasterizer::DrawScanlineEx<0x445068a8>;
	m_dsmap[0x445044a8] = &GSRasterizer::DrawScanlineEx<0x445044a8>;
	m_dsmap[0x00160428] = &GSRasterizer::DrawScanlineEx<0x00160428>;
	m_dsmap[0x48562848] = &GSRasterizer::DrawScanlineEx<0x48562848>;
	m_dsmap[0x495228c8] = &GSRasterizer::DrawScanlineEx<0x495228c8>;
	m_dsmap[0x49520448] = &GSRasterizer::DrawScanlineEx<0x49520448>;
	m_dsmap[0x465228a8] = &GSRasterizer::DrawScanlineEx<0x465228a8>;
	m_dsmap[0x445b44c8] = &GSRasterizer::DrawScanlineEx<0x445b44c8>;
	m_dsmap[0x00022808] = &GSRasterizer::DrawScanlineEx<0x00022808>;
	m_dsmap[0x445068e8] = &GSRasterizer::DrawScanlineEx<0x445068e8>;
	m_dsmap[0x445204c8] = &GSRasterizer::DrawScanlineEx<0x445204c8>;
	m_dsmap[0x445604a8] = &GSRasterizer::DrawScanlineEx<0x445604a8>;
	m_dsmap[0x42560488] = &GSRasterizer::DrawScanlineEx<0x42560488>;
	m_dsmap[0x445668c8] = &GSRasterizer::DrawScanlineEx<0x445668c8>;
	m_dsmap[0x44522808] = &GSRasterizer::DrawScanlineEx<0x44522808>;
	m_dsmap[0x425b68c8] = &GSRasterizer::DrawScanlineEx<0x425b68c8>;
	m_dsmap[0x00023808] = &GSRasterizer::DrawScanlineEx<0x00023808>;
	m_dsmap[0x48563808] = &GSRasterizer::DrawScanlineEx<0x48563808>;

	// culdcept

	m_dsmap[0x00004400] = &GSRasterizer::DrawScanlineEx<0x00004400>;
	m_dsmap[0x0000714b] = &GSRasterizer::DrawScanlineEx<0x0000714b>;
	m_dsmap[0x0001f84b] = &GSRasterizer::DrawScanlineEx<0x0001f84b>;
	m_dsmap[0x0001d84b] = &GSRasterizer::DrawScanlineEx<0x0001d84b>;
	m_dsmap[0x4459f84b] = &GSRasterizer::DrawScanlineEx<0x4459f84b>;
	m_dsmap[0x0009eacb] = &GSRasterizer::DrawScanlineEx<0x0009eacb>;
	m_dsmap[0x0001ea4b] = &GSRasterizer::DrawScanlineEx<0x0001ea4b>;
	m_dsmap[0x4451c4cb] = &GSRasterizer::DrawScanlineEx<0x4451c4cb>;
	m_dsmap[0x6851c4cb] = &GSRasterizer::DrawScanlineEx<0x6851c4cb>;
	m_dsmap[0x4451e8cb] = &GSRasterizer::DrawScanlineEx<0x4451e8cb>;
	m_dsmap[0x4851c4cb] = &GSRasterizer::DrawScanlineEx<0x4851c4cb>;
	m_dsmap[0x4451c8cb] = &GSRasterizer::DrawScanlineEx<0x4451c8cb>;
	m_dsmap[0x0001c4cb] = &GSRasterizer::DrawScanlineEx<0x0001c4cb>;
	m_dsmap[0x0009f84b] = &GSRasterizer::DrawScanlineEx<0x0009f84b>;
	m_dsmap[0x0000442b] = &GSRasterizer::DrawScanlineEx<0x0000442b>;
	m_dsmap[0x6851eacb] = &GSRasterizer::DrawScanlineEx<0x6851eacb>;
	m_dsmap[0x0009d84b] = &GSRasterizer::DrawScanlineEx<0x0009d84b>;
	m_dsmap[0x00007100] = &GSRasterizer::DrawScanlineEx<0x00007100>;
	m_dsmap[0x0009ea4b] = &GSRasterizer::DrawScanlineEx<0x0009ea4b>;
	m_dsmap[0x4451f84b] = &GSRasterizer::DrawScanlineEx<0x4451f84b>;
	m_dsmap[0x0001eacb] = &GSRasterizer::DrawScanlineEx<0x0001eacb>;
	m_dsmap[0x6451ea4b] = &GSRasterizer::DrawScanlineEx<0x6451ea4b>;
	m_dsmap[0x6851f84b] = &GSRasterizer::DrawScanlineEx<0x6851f84b>;
	m_dsmap[0x4451d84b] = &GSRasterizer::DrawScanlineEx<0x4451d84b>;
	m_dsmap[0xa151eacb] = &GSRasterizer::DrawScanlineEx<0xa151eacb>;
	m_dsmap[0x6851d84b] = &GSRasterizer::DrawScanlineEx<0x6851d84b>;

	// kingdom hearts

	m_dsmap[0x445968c8] = &GSRasterizer::DrawScanlineEx<0x445968c8>;
	m_dsmap[0x4451a8c8] = &GSRasterizer::DrawScanlineEx<0x4451a8c8>;
	m_dsmap[0x4451a828] = &GSRasterizer::DrawScanlineEx<0x4451a828>;
	m_dsmap[0x4851a8e8] = &GSRasterizer::DrawScanlineEx<0x4851a8e8>;
	m_dsmap[0x00104438] = &GSRasterizer::DrawScanlineEx<0x00104438>;
	m_dsmap[0x4451a8e8] = &GSRasterizer::DrawScanlineEx<0x4451a8e8>;
	m_dsmap[0x4851a868] = &GSRasterizer::DrawScanlineEx<0x4851a868>;
	m_dsmap[0x2a59a8d8] = &GSRasterizer::DrawScanlineEx<0x2a59a8d8>;
	m_dsmap[0x44505828] = &GSRasterizer::DrawScanlineEx<0x44505828>;
	m_dsmap[0x445078a8] = &GSRasterizer::DrawScanlineEx<0x445078a8>;
	m_dsmap[0x4451a868] = &GSRasterizer::DrawScanlineEx<0x4451a868>;
	m_dsmap[0x00107800] = &GSRasterizer::DrawScanlineEx<0x00107800>;
	m_dsmap[0x4458a8c8] = &GSRasterizer::DrawScanlineEx<0x4458a8c8>;
	m_dsmap[0x44506828] = &GSRasterizer::DrawScanlineEx<0x44506828>;
	m_dsmap[0x4459a8c8] = &GSRasterizer::DrawScanlineEx<0x4459a8c8>;
	m_dsmap[0x00007809] = &GSRasterizer::DrawScanlineEx<0x00007809>;
	m_dsmap[0x4859a8e8] = &GSRasterizer::DrawScanlineEx<0x4859a8e8>;
	m_dsmap[0x44507828] = &GSRasterizer::DrawScanlineEx<0x44507828>;
	m_dsmap[0x0009a8c8] = &GSRasterizer::DrawScanlineEx<0x0009a8c8>;
	m_dsmap[0x4851a808] = &GSRasterizer::DrawScanlineEx<0x4851a808>;
	m_dsmap[0x81504408] = &GSRasterizer::DrawScanlineEx<0x81504408>;
	m_dsmap[0x4451a808] = &GSRasterizer::DrawScanlineEx<0x4451a808>;
	m_dsmap[0x00004409] = &GSRasterizer::DrawScanlineEx<0x00004409>;
	m_dsmap[0x00118458] = &GSRasterizer::DrawScanlineEx<0x00118458>;
	m_dsmap[0x4451b808] = &GSRasterizer::DrawScanlineEx<0x4451b808>;
	m_dsmap[0x4251a868] = &GSRasterizer::DrawScanlineEx<0x4251a868>;
	m_dsmap[0x44507028] = &GSRasterizer::DrawScanlineEx<0x44507028>;
	m_dsmap[0x445968d8] = &GSRasterizer::DrawScanlineEx<0x445968d8>;
	m_dsmap[0x445058a8] = &GSRasterizer::DrawScanlineEx<0x445058a8>;
	m_dsmap[0x00007028] = &GSRasterizer::DrawScanlineEx<0x00007028>;
	m_dsmap[0x48505828] = &GSRasterizer::DrawScanlineEx<0x48505828>;
	m_dsmap[0x485078a8] = &GSRasterizer::DrawScanlineEx<0x485078a8>;
	m_dsmap[0x00005808] = &GSRasterizer::DrawScanlineEx<0x00005808>;
	m_dsmap[0x4851a888] = &GSRasterizer::DrawScanlineEx<0x4851a888>;
	m_dsmap[0x44505008] = &GSRasterizer::DrawScanlineEx<0x44505008>;
	m_dsmap[0x4459a8d8] = &GSRasterizer::DrawScanlineEx<0x4459a8d8>;
	m_dsmap[0x4651a8e8] = &GSRasterizer::DrawScanlineEx<0x4651a8e8>;
	m_dsmap[0x00004488] = &GSRasterizer::DrawScanlineEx<0x00004488>;
	m_dsmap[0x445184e8] = &GSRasterizer::DrawScanlineEx<0x445184e8>;
	m_dsmap[0x48504428] = &GSRasterizer::DrawScanlineEx<0x48504428>;
	m_dsmap[0x4251a8e8] = &GSRasterizer::DrawScanlineEx<0x4251a8e8>;
	m_dsmap[0x44507008] = &GSRasterizer::DrawScanlineEx<0x44507008>;
	m_dsmap[0x4859a888] = &GSRasterizer::DrawScanlineEx<0x4859a888>;
	m_dsmap[0x4251a888] = &GSRasterizer::DrawScanlineEx<0x4251a888>;
	m_dsmap[0x4251a808] = &GSRasterizer::DrawScanlineEx<0x4251a808>;
	m_dsmap[0x4451b048] = &GSRasterizer::DrawScanlineEx<0x4451b048>;
	m_dsmap[0x00120408] = &GSRasterizer::DrawScanlineEx<0x00120408>;
	m_dsmap[0x00120448] = &GSRasterizer::DrawScanlineEx<0x00120448>;
	m_dsmap[0x44523808] = &GSRasterizer::DrawScanlineEx<0x44523808>;
	m_dsmap[0x4851b868] = &GSRasterizer::DrawScanlineEx<0x4851b868>;
	m_dsmap[0x4851b8a8] = &GSRasterizer::DrawScanlineEx<0x4851b8a8>;
	m_dsmap[0x8851a8e8] = &GSRasterizer::DrawScanlineEx<0x8851a8e8>;
	m_dsmap[0x4451b8a8] = &GSRasterizer::DrawScanlineEx<0x4451b8a8>;
	m_dsmap[0x44518428] = &GSRasterizer::DrawScanlineEx<0x44518428>;
	m_dsmap[0x485184a8] = &GSRasterizer::DrawScanlineEx<0x485184a8>;
	m_dsmap[0x445184a8] = &GSRasterizer::DrawScanlineEx<0x445184a8>;
	m_dsmap[0x425184a8] = &GSRasterizer::DrawScanlineEx<0x425184a8>;
	m_dsmap[0x4851b828] = &GSRasterizer::DrawScanlineEx<0x4851b828>;
	m_dsmap[0x4451b828] = &GSRasterizer::DrawScanlineEx<0x4451b828>;
	m_dsmap[0x44507808] = &GSRasterizer::DrawScanlineEx<0x44507808>;
	m_dsmap[0x00007808] = &GSRasterizer::DrawScanlineEx<0x00007808>;

	// kh2

	m_dsmap[0x2a59a8c8] = &GSRasterizer::DrawScanlineEx<0x2a59a8c8>;
	m_dsmap[0x44505028] = &GSRasterizer::DrawScanlineEx<0x44505028>;
	m_dsmap[0x425078a8] = &GSRasterizer::DrawScanlineEx<0x425078a8>;
	m_dsmap[0x64507009] = &GSRasterizer::DrawScanlineEx<0x64507009>;
	m_dsmap[0x44505889] = &GSRasterizer::DrawScanlineEx<0x44505889>;
	m_dsmap[0x4458e8c8] = &GSRasterizer::DrawScanlineEx<0x4458e8c8>;
	m_dsmap[0xa551a868] = &GSRasterizer::DrawScanlineEx<0xa551a868>;
	m_dsmap[0x44518440] = &GSRasterizer::DrawScanlineEx<0x44518440>;
	m_dsmap[0x0251a868] = &GSRasterizer::DrawScanlineEx<0x0251a868>;
	m_dsmap[0x4859a8c8] = &GSRasterizer::DrawScanlineEx<0x4859a8c8>;
	m_dsmap[0x00007128] = &GSRasterizer::DrawScanlineEx<0x00007128>;
	m_dsmap[0x485184e8] = &GSRasterizer::DrawScanlineEx<0x485184e8>;
	m_dsmap[0x0051a868] = &GSRasterizer::DrawScanlineEx<0x0051a868>;
	m_dsmap[0x4851b808] = &GSRasterizer::DrawScanlineEx<0x4851b808>;
	m_dsmap[0x48507889] = &GSRasterizer::DrawScanlineEx<0x48507889>;
	m_dsmap[0x2651a868] = &GSRasterizer::DrawScanlineEx<0x2651a868>;
	m_dsmap[0x8951a868] = &GSRasterizer::DrawScanlineEx<0x8951a868>;
	m_dsmap[0x4451b800] = &GSRasterizer::DrawScanlineEx<0x4451b800>;
	m_dsmap[0x44507889] = &GSRasterizer::DrawScanlineEx<0x44507889>;
	m_dsmap[0x68507009] = &GSRasterizer::DrawScanlineEx<0x68507009>;

	// resident evil 4

	m_dsmap[0x4851a848] = &GSRasterizer::DrawScanlineEx<0x4851a848>;
	m_dsmap[0x5851a8c8] = &GSRasterizer::DrawScanlineEx<0x5851a8c8>;
	m_dsmap[0x48507808] = &GSRasterizer::DrawScanlineEx<0x48507808>;
	m_dsmap[0x64540428] = &GSRasterizer::DrawScanlineEx<0x64540428>;
	m_dsmap[0x4451a848] = &GSRasterizer::DrawScanlineEx<0x4451a848>;
	m_dsmap[0x64518448] = &GSRasterizer::DrawScanlineEx<0x64518448>;
	m_dsmap[0x64504408] = &GSRasterizer::DrawScanlineEx<0x64504408>;
	m_dsmap[0x44518408] = &GSRasterizer::DrawScanlineEx<0x44518408>;
	m_dsmap[0x68507908] = &GSRasterizer::DrawScanlineEx<0x68507908>;
	m_dsmap[0x2a505908] = &GSRasterizer::DrawScanlineEx<0x2a505908>;
	m_dsmap[0x89504408] = &GSRasterizer::DrawScanlineEx<0x89504408>;
	m_dsmap[0x485384c8] = &GSRasterizer::DrawScanlineEx<0x485384c8>;
	m_dsmap[0x00007800] = &GSRasterizer::DrawScanlineEx<0x00007800>;
	m_dsmap[0x00007108] = &GSRasterizer::DrawScanlineEx<0x00007108>;
	m_dsmap[0x48518448] = &GSRasterizer::DrawScanlineEx<0x48518448>;
	m_dsmap[0x68504448] = &GSRasterizer::DrawScanlineEx<0x68504448>;
	m_dsmap[0x64507908] = &GSRasterizer::DrawScanlineEx<0x64507908>;
	m_dsmap[0x48504408] = &GSRasterizer::DrawScanlineEx<0x48504408>;
	m_dsmap[0x6451a8c8] = &GSRasterizer::DrawScanlineEx<0x6451a8c8>;
	m_dsmap[0x2a507908] = &GSRasterizer::DrawScanlineEx<0x2a507908>;
	m_dsmap[0x4451a888] = &GSRasterizer::DrawScanlineEx<0x4451a888>;
	m_dsmap[0x00005848] = &GSRasterizer::DrawScanlineEx<0x00005848>;
	m_dsmap[0xa4507908] = &GSRasterizer::DrawScanlineEx<0xa4507908>;
	m_dsmap[0x44505908] = &GSRasterizer::DrawScanlineEx<0x44505908>;
	m_dsmap[0x44504408] = &GSRasterizer::DrawScanlineEx<0x44504408>;
	m_dsmap[0x00105908] = &GSRasterizer::DrawScanlineEx<0x00105908>;
	m_dsmap[0x0010590a] = &GSRasterizer::DrawScanlineEx<0x0010590a>;
	m_dsmap[0x00005908] = &GSRasterizer::DrawScanlineEx<0x00005908>;
	m_dsmap[0x00104408] = &GSRasterizer::DrawScanlineEx<0x00104408>;
	m_dsmap[0x4851a8c8] = &GSRasterizer::DrawScanlineEx<0x4851a8c8>;
	m_dsmap[0x00106908] = &GSRasterizer::DrawScanlineEx<0x00106908>;

	// dmc (fixme)

	m_dsmap[0x29524438] = &GSRasterizer::DrawScanlineEx<0x29524438>;
	m_dsmap[0x00004438] = &GSRasterizer::DrawScanlineEx<0x00004438>;
	m_dsmap[0x000968d8] = &GSRasterizer::DrawScanlineEx<0x000968d8>;
	m_dsmap[0x44504458] = &GSRasterizer::DrawScanlineEx<0x44504458>;
	m_dsmap[0x0001a8d8] = &GSRasterizer::DrawScanlineEx<0x0001a8d8>;
	m_dsmap[0x00024438] = &GSRasterizer::DrawScanlineEx<0x00024438>;
	m_dsmap[0x44507058] = &GSRasterizer::DrawScanlineEx<0x44507058>;
	m_dsmap[0x62522838] = &GSRasterizer::DrawScanlineEx<0x62522838>;
	m_dsmap[0x6851a8d8] = &GSRasterizer::DrawScanlineEx<0x6851a8d8>;
	m_dsmap[0xa1520478] = &GSRasterizer::DrawScanlineEx<0xa1520478>;
	m_dsmap[0x48516858] = &GSRasterizer::DrawScanlineEx<0x48516858>;
	m_dsmap[0x0009a8d8] = &GSRasterizer::DrawScanlineEx<0x0009a8d8>;
	m_dsmap[0xa1524438] = &GSRasterizer::DrawScanlineEx<0xa1524438>;
	m_dsmap[0x44516858] = &GSRasterizer::DrawScanlineEx<0x44516858>;
	m_dsmap[0x00017838] = &GSRasterizer::DrawScanlineEx<0x00017838>;
	m_dsmap[0x0009a8f8] = &GSRasterizer::DrawScanlineEx<0x0009a8f8>;
	m_dsmap[0x00004418] = &GSRasterizer::DrawScanlineEx<0x00004418>;
	m_dsmap[0x29520478] = &GSRasterizer::DrawScanlineEx<0x29520478>;

	// tomoyo after 

	m_dsmap[0x445044d8] = &GSRasterizer::DrawScanlineEx<0x445044d8>;
	m_dsmap[0x00004438] = &GSRasterizer::DrawScanlineEx<0x00004438>;
	m_dsmap[0x485058d8] = &GSRasterizer::DrawScanlineEx<0x485058d8>;
	m_dsmap[0x425078d8] = &GSRasterizer::DrawScanlineEx<0x425078d8>;
	m_dsmap[0x445058d8] = &GSRasterizer::DrawScanlineEx<0x445058d8>;
	m_dsmap[0x425044d8] = &GSRasterizer::DrawScanlineEx<0x425044d8>;
	m_dsmap[0x445078d8] = &GSRasterizer::DrawScanlineEx<0x445078d8>;
	m_dsmap[0x545058d8] = &GSRasterizer::DrawScanlineEx<0x545058d8>;
	m_dsmap[0x00007839] = &GSRasterizer::DrawScanlineEx<0x00007839>;
	m_dsmap[0x00004418] = &GSRasterizer::DrawScanlineEx<0x00004418>;
	m_dsmap[0x0000441b] = &GSRasterizer::DrawScanlineEx<0x0000441b>;
	m_dsmap[0x845044d8] = &GSRasterizer::DrawScanlineEx<0x845044d8>;
	m_dsmap[0x00005839] = &GSRasterizer::DrawScanlineEx<0x00005839>;

	// okami

	m_dsmap[0x44504438] = &GSRasterizer::DrawScanlineEx<0x44504438>;
	m_dsmap[0x44507918] = &GSRasterizer::DrawScanlineEx<0x44507918>;
	m_dsmap[0x445984d8] = &GSRasterizer::DrawScanlineEx<0x445984d8>;
	m_dsmap[0x64507118] = &GSRasterizer::DrawScanlineEx<0x64507118>;
	m_dsmap[0x4451a838] = &GSRasterizer::DrawScanlineEx<0x4451a838>;
	m_dsmap[0x445228d8] = &GSRasterizer::DrawScanlineEx<0x445228d8>;
	m_dsmap[0x4851a858] = &GSRasterizer::DrawScanlineEx<0x4851a858>;
	m_dsmap[0x00107918] = &GSRasterizer::DrawScanlineEx<0x00107918>;
	m_dsmap[0x00007918] = &GSRasterizer::DrawScanlineEx<0x00007918>;
	m_dsmap[0x62507918] = &GSRasterizer::DrawScanlineEx<0x62507918>;
	m_dsmap[0x44505818] = &GSRasterizer::DrawScanlineEx<0x44505818>;
	m_dsmap[0x4451b838] = &GSRasterizer::DrawScanlineEx<0x4451b838>;
	m_dsmap[0x00105958] = &GSRasterizer::DrawScanlineEx<0x00105958>;
	m_dsmap[0x4451a858] = &GSRasterizer::DrawScanlineEx<0x4451a858>;
	m_dsmap[0x29505838] = &GSRasterizer::DrawScanlineEx<0x29505838>;
	m_dsmap[0x44504478] = &GSRasterizer::DrawScanlineEx<0x44504478>;
	m_dsmap[0x485a28d8] = &GSRasterizer::DrawScanlineEx<0x485a28d8>;
	m_dsmap[0x00106118] = &GSRasterizer::DrawScanlineEx<0x00106118>;
	m_dsmap[0x00005838] = &GSRasterizer::DrawScanlineEx<0x00005838>;
	m_dsmap[0x44506838] = &GSRasterizer::DrawScanlineEx<0x44506838>;
	m_dsmap[0x64507918] = &GSRasterizer::DrawScanlineEx<0x64507918>;
	m_dsmap[0x89507918] = &GSRasterizer::DrawScanlineEx<0x89507918>;
	m_dsmap[0x54507818] = &GSRasterizer::DrawScanlineEx<0x54507818>;
	m_dsmap[0x68507118] = &GSRasterizer::DrawScanlineEx<0x68507118>;
	m_dsmap[0x00105918] = &GSRasterizer::DrawScanlineEx<0x00105918>;
	m_dsmap[0x0010591a] = &GSRasterizer::DrawScanlineEx<0x0010591a>;
	m_dsmap[0x00005918] = &GSRasterizer::DrawScanlineEx<0x00005918>;

	// shadow of the colossus

	m_dsmap[0x445398c8] = &GSRasterizer::DrawScanlineEx<0x445398c8>;
	m_dsmap[0x48523828] = &GSRasterizer::DrawScanlineEx<0x48523828>;
	m_dsmap[0x445a68c8] = &GSRasterizer::DrawScanlineEx<0x445a68c8>;
	m_dsmap[0x00021828] = &GSRasterizer::DrawScanlineEx<0x00021828>;
	m_dsmap[0x64520428] = &GSRasterizer::DrawScanlineEx<0x64520428>;
	m_dsmap[0x445328c8] = &GSRasterizer::DrawScanlineEx<0x445328c8>;
	m_dsmap[0x68520448] = &GSRasterizer::DrawScanlineEx<0x68520448>;
	m_dsmap[0x425a28c8] = &GSRasterizer::DrawScanlineEx<0x425a28c8>;
	m_dsmap[0x445238c8] = &GSRasterizer::DrawScanlineEx<0x445238c8>;
	m_dsmap[0x4453b8c8] = &GSRasterizer::DrawScanlineEx<0x4453b8c8>;
	m_dsmap[0x48520428] = &GSRasterizer::DrawScanlineEx<0x48520428>;
	m_dsmap[0x4453a848] = &GSRasterizer::DrawScanlineEx<0x4453a848>;
	m_dsmap[0x44723848] = &GSRasterizer::DrawScanlineEx<0x44723848>;
	m_dsmap[0x485a28c8] = &GSRasterizer::DrawScanlineEx<0x485a28c8>;
	m_dsmap[0x000204a8] = &GSRasterizer::DrawScanlineEx<0x000204a8>;
	m_dsmap[0x449a28c8] = &GSRasterizer::DrawScanlineEx<0x449a28c8>;
	m_dsmap[0x4453b848] = &GSRasterizer::DrawScanlineEx<0x4453b848>;
	m_dsmap[0x445a28c8] = &GSRasterizer::DrawScanlineEx<0x445a28c8>;
	m_dsmap[0x445368c8] = &GSRasterizer::DrawScanlineEx<0x445368c8>;
	m_dsmap[0x44523028] = &GSRasterizer::DrawScanlineEx<0x44523028>;
	m_dsmap[0x00020428] = &GSRasterizer::DrawScanlineEx<0x00020428>;
	m_dsmap[0x44540428] = &GSRasterizer::DrawScanlineEx<0x44540428>;
	m_dsmap[0x00005009] = &GSRasterizer::DrawScanlineEx<0x00005009>;
	m_dsmap[0x00018448] = &GSRasterizer::DrawScanlineEx<0x00018448>;
	m_dsmap[0x00020448] = &GSRasterizer::DrawScanlineEx<0x00020448>;
	m_dsmap[0x54523028] = &GSRasterizer::DrawScanlineEx<0x54523028>;
	m_dsmap[0x44521828] = &GSRasterizer::DrawScanlineEx<0x44521828>;
	m_dsmap[0x445a20c8] = &GSRasterizer::DrawScanlineEx<0x445a20c8>;
	m_dsmap[0x00004420] = &GSRasterizer::DrawScanlineEx<0x00004420>;
	m_dsmap[0x44538848] = &GSRasterizer::DrawScanlineEx<0x44538848>;
	m_dsmap[0x00023829] = &GSRasterizer::DrawScanlineEx<0x00023829>;
	m_dsmap[0x485a69c8] = &GSRasterizer::DrawScanlineEx<0x485a69c8>;
	m_dsmap[0x44539848] = &GSRasterizer::DrawScanlineEx<0x44539848>;
	m_dsmap[0x44520428] = &GSRasterizer::DrawScanlineEx<0x44520428>;
	m_dsmap[0x60527048] = &GSRasterizer::DrawScanlineEx<0x60527048>;
	m_dsmap[0x44523828] = &GSRasterizer::DrawScanlineEx<0x44523828>;
	m_dsmap[0x00023828] = &GSRasterizer::DrawScanlineEx<0x00023828>;
	m_dsmap[0x445204a8] = &GSRasterizer::DrawScanlineEx<0x445204a8>;
	m_dsmap[0x445268a8] = &GSRasterizer::DrawScanlineEx<0x445268a8>;
	m_dsmap[0x48520448] = &GSRasterizer::DrawScanlineEx<0x48520448>;
	m_dsmap[0x4453a8c8] = &GSRasterizer::DrawScanlineEx<0x4453a8c8>;
	m_dsmap[0x445968c8] = &GSRasterizer::DrawScanlineEx<0x445968c8>;
	m_dsmap[0x485b68c8] = &GSRasterizer::DrawScanlineEx<0x485b68c8>;
	m_dsmap[0x445b68c8] = &GSRasterizer::DrawScanlineEx<0x445b68c8>;
	m_dsmap[0x445b44c8] = &GSRasterizer::DrawScanlineEx<0x445b44c8>;

	// mgs3s1

	m_dsmap[0x2a52102d] = &GSRasterizer::DrawScanlineEx<0x2a52102d>;
	m_dsmap[0x445344c8] = &GSRasterizer::DrawScanlineEx<0x445344c8>;
	m_dsmap[0x48534448] = &GSRasterizer::DrawScanlineEx<0x48534448>;
	m_dsmap[0x64523028] = &GSRasterizer::DrawScanlineEx<0x64523028>;
	m_dsmap[0x2a523028] = &GSRasterizer::DrawScanlineEx<0x2a523028>;
	m_dsmap[0x4452a848] = &GSRasterizer::DrawScanlineEx<0x4452a848>;
	m_dsmap[0x44534448] = &GSRasterizer::DrawScanlineEx<0x44534448>;
	m_dsmap[0x0002383a] = &GSRasterizer::DrawScanlineEx<0x0002383a>;
	m_dsmap[0x44535848] = &GSRasterizer::DrawScanlineEx<0x44535848>;
	m_dsmap[0x6a520468] = &GSRasterizer::DrawScanlineEx<0x6a520468>;
	m_dsmap[0x44560428] = &GSRasterizer::DrawScanlineEx<0x44560428>;
	m_dsmap[0x48537848] = &GSRasterizer::DrawScanlineEx<0x48537848>;
	m_dsmap[0x44536848] = &GSRasterizer::DrawScanlineEx<0x44536848>;
	m_dsmap[0x64563068] = &GSRasterizer::DrawScanlineEx<0x64563068>;
	m_dsmap[0x44537848] = &GSRasterizer::DrawScanlineEx<0x44537848>;

	// 12riven

	m_dsmap[0x44505898] = &GSRasterizer::DrawScanlineEx<0x44505898>;
	m_dsmap[0x44507818] = &GSRasterizer::DrawScanlineEx<0x44507818>;
	m_dsmap[0x44504498] = &GSRasterizer::DrawScanlineEx<0x44504498>;
	m_dsmap[0x64505818] = &GSRasterizer::DrawScanlineEx<0x64505818>;
	m_dsmap[0x44504418] = &GSRasterizer::DrawScanlineEx<0x44504418>;

	// persona 3 fes

	m_dsmap[0x445868d8] = &GSRasterizer::DrawScanlineEx<0x445868d8>;
	m_dsmap[0x445244d8] = &GSRasterizer::DrawScanlineEx<0x445244d8>;
	m_dsmap[0x6a536898] = &GSRasterizer::DrawScanlineEx<0x6a536898>;
	m_dsmap[0x445b68d8] = &GSRasterizer::DrawScanlineEx<0x445b68d8>;
	m_dsmap[0x000244d8] = &GSRasterizer::DrawScanlineEx<0x000244d8>;
	m_dsmap[0x4453a8d8] = &GSRasterizer::DrawScanlineEx<0x4453a8d8>;
	m_dsmap[0x00124438] = &GSRasterizer::DrawScanlineEx<0x00124438>;
	m_dsmap[0x44536898] = &GSRasterizer::DrawScanlineEx<0x44536898>;
	m_dsmap[0x58536898] = &GSRasterizer::DrawScanlineEx<0x58536898>;
	m_dsmap[0x445f68d8] = &GSRasterizer::DrawScanlineEx<0x445f68d8>;
	m_dsmap[0x445348d8] = &GSRasterizer::DrawScanlineEx<0x445348d8>;
	m_dsmap[0x44520898] = &GSRasterizer::DrawScanlineEx<0x44520898>;
	m_dsmap[0x48522898] = &GSRasterizer::DrawScanlineEx<0x48522898>;
	m_dsmap[0x445a44d8] = &GSRasterizer::DrawScanlineEx<0x445a44d8>;
	m_dsmap[0x445368d8] = &GSRasterizer::DrawScanlineEx<0x445368d8>;
	m_dsmap[0x44524498] = &GSRasterizer::DrawScanlineEx<0x44524498>;
	m_dsmap[0x42520498] = &GSRasterizer::DrawScanlineEx<0x42520498>;
	m_dsmap[0x00024498] = &GSRasterizer::DrawScanlineEx<0x00024498>;
	m_dsmap[0x485228d8] = &GSRasterizer::DrawScanlineEx<0x485228d8>;
	m_dsmap[0x48520498] = &GSRasterizer::DrawScanlineEx<0x48520498>;
	m_dsmap[0x44534898] = &GSRasterizer::DrawScanlineEx<0x44534898>;
	m_dsmap[0x685868d8] = &GSRasterizer::DrawScanlineEx<0x685868d8>;
	m_dsmap[0x00304498] = &GSRasterizer::DrawScanlineEx<0x00304498>;
	m_dsmap[0x445c28d8] = &GSRasterizer::DrawScanlineEx<0x445c28d8>;
	m_dsmap[0x00034498] = &GSRasterizer::DrawScanlineEx<0x00034498>;

	// bully

	m_dsmap[0x00024408] = &GSRasterizer::DrawScanlineEx<0x00024408>;
	m_dsmap[0x00007828] = &GSRasterizer::DrawScanlineEx<0x00007828>;
	m_dsmap[0x44537808] = &GSRasterizer::DrawScanlineEx<0x44537808>;
	m_dsmap[0x445244c8] = &GSRasterizer::DrawScanlineEx<0x445244c8>;
	m_dsmap[0x445b48c8] = &GSRasterizer::DrawScanlineEx<0x445b48c8>;
	m_dsmap[0x00107108] = &GSRasterizer::DrawScanlineEx<0x00107108>;
	m_dsmap[0x00024488] = &GSRasterizer::DrawScanlineEx<0x00024488>;
	m_dsmap[0x68537808] = &GSRasterizer::DrawScanlineEx<0x68537808>;
	m_dsmap[0x44537888] = &GSRasterizer::DrawScanlineEx<0x44537888>;
	m_dsmap[0x8859a8c8] = &GSRasterizer::DrawScanlineEx<0x8859a8c8>;
	m_dsmap[0xa4504409] = &GSRasterizer::DrawScanlineEx<0xa4504409>;
	m_dsmap[0x485368c8] = &GSRasterizer::DrawScanlineEx<0x485368c8>;
	m_dsmap[0x000244c8] = &GSRasterizer::DrawScanlineEx<0x000244c8>;
	m_dsmap[0x68537848] = &GSRasterizer::DrawScanlineEx<0x68537848>;
	m_dsmap[0x54507908] = &GSRasterizer::DrawScanlineEx<0x54507908>;
	m_dsmap[0x68507108] = &GSRasterizer::DrawScanlineEx<0x68507108>;
	m_dsmap[0xa850590b] = &GSRasterizer::DrawScanlineEx<0xa850590b>;
	m_dsmap[0x4a524428] = &GSRasterizer::DrawScanlineEx<0x4a524428>;
	m_dsmap[0x68537888] = &GSRasterizer::DrawScanlineEx<0x68537888>;
	m_dsmap[0x00124408] = &GSRasterizer::DrawScanlineEx<0x00124408>;
	m_dsmap[0x685378c8] = &GSRasterizer::DrawScanlineEx<0x685378c8>;
	m_dsmap[0x68536888] = &GSRasterizer::DrawScanlineEx<0x68536888>;
	m_dsmap[0x00124428] = &GSRasterizer::DrawScanlineEx<0x00124428>;
	m_dsmap[0x61504408] = &GSRasterizer::DrawScanlineEx<0x61504408>;
	m_dsmap[0x44524408] = &GSRasterizer::DrawScanlineEx<0x44524408>;
	m_dsmap[0x685368c8] = &GSRasterizer::DrawScanlineEx<0x685368c8>;
	m_dsmap[0x48505008] = &GSRasterizer::DrawScanlineEx<0x48505008>;
	m_dsmap[0x00140469] = &GSRasterizer::DrawScanlineEx<0x00140469>;
	m_dsmap[0x44524488] = &GSRasterizer::DrawScanlineEx<0x44524488>;
	m_dsmap[0x00007808] = &GSRasterizer::DrawScanlineEx<0x00007808>;

	// nfs mw

	m_dsmap[0x445168d2] = &GSRasterizer::DrawScanlineEx<0x445168d2>;
	m_dsmap[0x445168d8] = &GSRasterizer::DrawScanlineEx<0x445168d8>;
	m_dsmap[0x64507818] = &GSRasterizer::DrawScanlineEx<0x64507818>;
	m_dsmap[0x6450781b] = &GSRasterizer::DrawScanlineEx<0x6450781b>;
	m_dsmap[0x64507018] = &GSRasterizer::DrawScanlineEx<0x64507018>;
	m_dsmap[0x585068c8] = &GSRasterizer::DrawScanlineEx<0x585068c8>;
	m_dsmap[0x00120418] = &GSRasterizer::DrawScanlineEx<0x00120418>;
	m_dsmap[0x585068d8] = &GSRasterizer::DrawScanlineEx<0x585068d8>;
	m_dsmap[0x40521818] = &GSRasterizer::DrawScanlineEx<0x40521818>;
	m_dsmap[0x0a51a8d2] = &GSRasterizer::DrawScanlineEx<0x0a51a8d2>;
	m_dsmap[0x49507808] = &GSRasterizer::DrawScanlineEx<0x49507808>;
	m_dsmap[0x00105948] = &GSRasterizer::DrawScanlineEx<0x00105948>;
	m_dsmap[0x00004432] = &GSRasterizer::DrawScanlineEx<0x00004432>;
	m_dsmap[0x585044c8] = &GSRasterizer::DrawScanlineEx<0x585044c8>;
	m_dsmap[0x0000443b] = &GSRasterizer::DrawScanlineEx<0x0000443b>;
	m_dsmap[0x0012191a] = &GSRasterizer::DrawScanlineEx<0x0012191a>;
	m_dsmap[0xa1504418] = &GSRasterizer::DrawScanlineEx<0xa1504418>;
	m_dsmap[0xa150441b] = &GSRasterizer::DrawScanlineEx<0xa150441b>;
	m_dsmap[0x48516848] = &GSRasterizer::DrawScanlineEx<0x48516848>;
	m_dsmap[0x585044d8] = &GSRasterizer::DrawScanlineEx<0x585044d8>;
	m_dsmap[0x54523918] = &GSRasterizer::DrawScanlineEx<0x54523918>;
	m_dsmap[0x68523918] = &GSRasterizer::DrawScanlineEx<0x68523918>;
	m_dsmap[0x00020418] = &GSRasterizer::DrawScanlineEx<0x00020418>;
	m_dsmap[0x44516812] = &GSRasterizer::DrawScanlineEx<0x44516812>;
	m_dsmap[0x68517818] = &GSRasterizer::DrawScanlineEx<0x68517818>;
	m_dsmap[0x89507908] = &GSRasterizer::DrawScanlineEx<0x89507908>;
	m_dsmap[0x44516848] = &GSRasterizer::DrawScanlineEx<0x44516848>;
	m_dsmap[0x485168c8] = &GSRasterizer::DrawScanlineEx<0x485168c8>;
	m_dsmap[0x0a51689b] = &GSRasterizer::DrawScanlineEx<0x0a51689b>;
	m_dsmap[0x485168d2] = &GSRasterizer::DrawScanlineEx<0x485168d2>;
	m_dsmap[0x485168d8] = &GSRasterizer::DrawScanlineEx<0x485168d8>;
	m_dsmap[0x0a5168c8] = &GSRasterizer::DrawScanlineEx<0x0a5168c8>;
	m_dsmap[0x0a5168d2] = &GSRasterizer::DrawScanlineEx<0x0a5168d2>;
	m_dsmap[0x0a5168d8] = &GSRasterizer::DrawScanlineEx<0x0a5168d8>;
	m_dsmap[0x68507818] = &GSRasterizer::DrawScanlineEx<0x68507818>;
	m_dsmap[0x44507898] = &GSRasterizer::DrawScanlineEx<0x44507898>;
	m_dsmap[0x445168c8] = &GSRasterizer::DrawScanlineEx<0x445168c8>;
	m_dsmap[0x585168c8] = &GSRasterizer::DrawScanlineEx<0x585168c8>;

	// xenosaga

	m_dsmap[0x64510868] = &GSRasterizer::DrawScanlineEx<0x64510868>;
	m_dsmap[0x4450e8c8] = &GSRasterizer::DrawScanlineEx<0x4450e8c8>;
	m_dsmap[0x001068c8] = &GSRasterizer::DrawScanlineEx<0x001068c8>;
	m_dsmap[0x00507828] = &GSRasterizer::DrawScanlineEx<0x00507828>;
	m_dsmap[0x44505868] = &GSRasterizer::DrawScanlineEx<0x44505868>;
	m_dsmap[0x00007868] = &GSRasterizer::DrawScanlineEx<0x00007868>;
	m_dsmap[0x685168c8] = &GSRasterizer::DrawScanlineEx<0x685168c8>;
	m_dsmap[0x445068c8] = &GSRasterizer::DrawScanlineEx<0x445068c8>;
	m_dsmap[0xa9504448] = &GSRasterizer::DrawScanlineEx<0xa9504448>;
	m_dsmap[0x44519828] = &GSRasterizer::DrawScanlineEx<0x44519828>;
	m_dsmap[0x4251a848] = &GSRasterizer::DrawScanlineEx<0x4251a848>;
	m_dsmap[0x44504468] = &GSRasterizer::DrawScanlineEx<0x44504468>;
	m_dsmap[0x44519848] = &GSRasterizer::DrawScanlineEx<0x44519848>;
	m_dsmap[0x00120468] = &GSRasterizer::DrawScanlineEx<0x00120468>;
	m_dsmap[0x00021808] = &GSRasterizer::DrawScanlineEx<0x00021808>;
	m_dsmap[0x44519868] = &GSRasterizer::DrawScanlineEx<0x44519868>;
	m_dsmap[0x685068c8] = &GSRasterizer::DrawScanlineEx<0x685068c8>;
	m_dsmap[0x605184e8] = &GSRasterizer::DrawScanlineEx<0x605184e8>;
	m_dsmap[0x001044c8] = &GSRasterizer::DrawScanlineEx<0x001044c8>;
	m_dsmap[0x00005868] = &GSRasterizer::DrawScanlineEx<0x00005868>;
	m_dsmap[0x68519828] = &GSRasterizer::DrawScanlineEx<0x68519828>;
	m_dsmap[0x445044c8] = &GSRasterizer::DrawScanlineEx<0x445044c8>;
	m_dsmap[0x68520428] = &GSRasterizer::DrawScanlineEx<0x68520428>;
	m_dsmap[0x485184c8] = &GSRasterizer::DrawScanlineEx<0x485184c8>;
	m_dsmap[0x445044e8] = &GSRasterizer::DrawScanlineEx<0x445044e8>;
	m_dsmap[0x445198c8] = &GSRasterizer::DrawScanlineEx<0x445198c8>;
	m_dsmap[0x645068c8] = &GSRasterizer::DrawScanlineEx<0x645068c8>;
	m_dsmap[0x29519828] = &GSRasterizer::DrawScanlineEx<0x29519828>;
	m_dsmap[0x00519828] = &GSRasterizer::DrawScanlineEx<0x00519828>;
	m_dsmap[0x625068c8] = &GSRasterizer::DrawScanlineEx<0x625068c8>;
	m_dsmap[0x89522868] = &GSRasterizer::DrawScanlineEx<0x89522868>;
	m_dsmap[0x685044c8] = &GSRasterizer::DrawScanlineEx<0x685044c8>;
	m_dsmap[0x00519848] = &GSRasterizer::DrawScanlineEx<0x00519848>;
	m_dsmap[0x00020468] = &GSRasterizer::DrawScanlineEx<0x00020468>;
	m_dsmap[0x445184c8] = &GSRasterizer::DrawScanlineEx<0x445184c8>;
	m_dsmap[0x00107868] = &GSRasterizer::DrawScanlineEx<0x00107868>;
	m_dsmap[0x00519868] = &GSRasterizer::DrawScanlineEx<0x00519868>;
	m_dsmap[0x445604c8] = &GSRasterizer::DrawScanlineEx<0x445604c8>;
	m_dsmap[0x00518428] = &GSRasterizer::DrawScanlineEx<0x00518428>;
	m_dsmap[0x64561868] = &GSRasterizer::DrawScanlineEx<0x64561868>;
	m_dsmap[0x001168c8] = &GSRasterizer::DrawScanlineEx<0x001168c8>;
	m_dsmap[0x11519868] = &GSRasterizer::DrawScanlineEx<0x11519868>;
	m_dsmap[0x44507068] = &GSRasterizer::DrawScanlineEx<0x44507068>;
	m_dsmap[0x4451b828] = &GSRasterizer::DrawScanlineEx<0x4451b828>;

	// mana khemia

	m_dsmap[0x44505819] = &GSRasterizer::DrawScanlineEx<0x44505819>;
	m_dsmap[0x44505839] = &GSRasterizer::DrawScanlineEx<0x44505839>;
	m_dsmap[0x44504419] = &GSRasterizer::DrawScanlineEx<0x44504419>;
	m_dsmap[0x44585819] = &GSRasterizer::DrawScanlineEx<0x44585819>;
	m_dsmap[0x00004439] = &GSRasterizer::DrawScanlineEx<0x00004439>;
	m_dsmap[0x445b7859] = &GSRasterizer::DrawScanlineEx<0x445b7859>;
	m_dsmap[0x44507819] = &GSRasterizer::DrawScanlineEx<0x44507819>;
	m_dsmap[0x445b68d9] = &GSRasterizer::DrawScanlineEx<0x445b68d9>;
	m_dsmap[0x485b68d9] = &GSRasterizer::DrawScanlineEx<0x485b68d9>;
	m_dsmap[0x485b7859] = &GSRasterizer::DrawScanlineEx<0x485b7859>;
	m_dsmap[0x48584899] = &GSRasterizer::DrawScanlineEx<0x48584899>;
	m_dsmap[0x48507819] = &GSRasterizer::DrawScanlineEx<0x48507819>;
	m_dsmap[0x48505819] = &GSRasterizer::DrawScanlineEx<0x48505819>;

	// Gundam Seed Destiny OMNI VS ZAFT II PLUS 

	m_dsmap[0x00024409] = &GSRasterizer::DrawScanlineEx<0x00024409>;
	m_dsmap[0x00024429] = &GSRasterizer::DrawScanlineEx<0x00024429>;
	m_dsmap[0x44506889] = &GSRasterizer::DrawScanlineEx<0x44506889>;
	m_dsmap[0x000b68e9] = &GSRasterizer::DrawScanlineEx<0x000b68e9>;
	m_dsmap[0x485368e9] = &GSRasterizer::DrawScanlineEx<0x485368e9>;
	m_dsmap[0x44535869] = &GSRasterizer::DrawScanlineEx<0x44535869>;
	m_dsmap[0x445368e9] = &GSRasterizer::DrawScanlineEx<0x445368e9>;
	m_dsmap[0x44507809] = &GSRasterizer::DrawScanlineEx<0x44507809>;
	m_dsmap[0x485b68e9] = &GSRasterizer::DrawScanlineEx<0x485b68e9>;
	m_dsmap[0x00035869] = &GSRasterizer::DrawScanlineEx<0x00035869>;
	m_dsmap[0x48506889] = &GSRasterizer::DrawScanlineEx<0x48506889>;
	m_dsmap[0x000368e9] = &GSRasterizer::DrawScanlineEx<0x000368e9>;
	m_dsmap[0x445b68e9] = &GSRasterizer::DrawScanlineEx<0x445b68e9>;
	m_dsmap[0x48504409] = &GSRasterizer::DrawScanlineEx<0x48504409>;
	m_dsmap[0x44505809] = &GSRasterizer::DrawScanlineEx<0x44505809>;
	m_dsmap[0x48537869] = &GSRasterizer::DrawScanlineEx<0x48537869>;
	m_dsmap[0x44504409] = &GSRasterizer::DrawScanlineEx<0x44504409>;
	m_dsmap[0x44504489] = &GSRasterizer::DrawScanlineEx<0x44504489>;
	m_dsmap[0x0010510a] = &GSRasterizer::DrawScanlineEx<0x0010510a>;
	m_dsmap[0x000b6be9] = &GSRasterizer::DrawScanlineEx<0x000b6be9>;

	// wild arms 5

	m_dsmap[0x485068c8] = &GSRasterizer::DrawScanlineEx<0x485068c8>;
	m_dsmap[0x445f68c8] = &GSRasterizer::DrawScanlineEx<0x445f68c8>;
	m_dsmap[0x4451b848] = &GSRasterizer::DrawScanlineEx<0x4451b848>;
	m_dsmap[0x42505808] = &GSRasterizer::DrawScanlineEx<0x42505808>;
	m_dsmap[0x00507800] = &GSRasterizer::DrawScanlineEx<0x00507800>;
	m_dsmap[0x44506888] = &GSRasterizer::DrawScanlineEx<0x44506888>;
	m_dsmap[0x44504808] = &GSRasterizer::DrawScanlineEx<0x44504808>;
	m_dsmap[0x00107928] = &GSRasterizer::DrawScanlineEx<0x00107928>;
	m_dsmap[0x00104448] = &GSRasterizer::DrawScanlineEx<0x00104448>;
	m_dsmap[0x68563808] = &GSRasterizer::DrawScanlineEx<0x68563808>;
	m_dsmap[0x44505888] = &GSRasterizer::DrawScanlineEx<0x44505888>;
	m_dsmap[0x6851b848] = &GSRasterizer::DrawScanlineEx<0x6851b848>;
	m_dsmap[0x425068c8] = &GSRasterizer::DrawScanlineEx<0x425068c8>;
	m_dsmap[0x44504888] = &GSRasterizer::DrawScanlineEx<0x44504888>;
	m_dsmap[0x465628c8] = &GSRasterizer::DrawScanlineEx<0x465628c8>;
	m_dsmap[0x00105928] = &GSRasterizer::DrawScanlineEx<0x00105928>;
	m_dsmap[0x585628c8] = &GSRasterizer::DrawScanlineEx<0x585628c8>;
	m_dsmap[0x445868c8] = &GSRasterizer::DrawScanlineEx<0x445868c8>;
	m_dsmap[0x44507808] = &GSRasterizer::DrawScanlineEx<0x44507808>;
	m_dsmap[0xa8507808] = &GSRasterizer::DrawScanlineEx<0xa8507808>;
	m_dsmap[0x000868c8] = &GSRasterizer::DrawScanlineEx<0x000868c8>;
	m_dsmap[0x00005924] = &GSRasterizer::DrawScanlineEx<0x00005924>;
	m_dsmap[0x48506888] = &GSRasterizer::DrawScanlineEx<0x48506888>;
	m_dsmap[0x68507808] = &GSRasterizer::DrawScanlineEx<0x68507808>;
	m_dsmap[0x645868c8] = &GSRasterizer::DrawScanlineEx<0x645868c8>;
	m_dsmap[0x44505808] = &GSRasterizer::DrawScanlineEx<0x44505808>;

	// rouge galaxy

	m_dsmap[0x64507808] = &GSRasterizer::DrawScanlineEx<0x64507808>;
	m_dsmap[0x485968c8] = &GSRasterizer::DrawScanlineEx<0x485968c8>;
	m_dsmap[0x48504488] = &GSRasterizer::DrawScanlineEx<0x48504488>;
	m_dsmap[0x62507808] = &GSRasterizer::DrawScanlineEx<0x62507808>;
	m_dsmap[0x0001b808] = &GSRasterizer::DrawScanlineEx<0x0001b808>;
	m_dsmap[0x42504408] = &GSRasterizer::DrawScanlineEx<0x42504408>;
	m_dsmap[0x00304408] = &GSRasterizer::DrawScanlineEx<0x00304408>;
	m_dsmap[0x88507808] = &GSRasterizer::DrawScanlineEx<0x88507808>;
	m_dsmap[0x445960c8] = &GSRasterizer::DrawScanlineEx<0x445960c8>;
	m_dsmap[0x44517808] = &GSRasterizer::DrawScanlineEx<0x44517808>;
	m_dsmap[0x00107808] = &GSRasterizer::DrawScanlineEx<0x00107808>;
	m_dsmap[0x0010780a] = &GSRasterizer::DrawScanlineEx<0x0010780a>;
	m_dsmap[0x445944c8] = &GSRasterizer::DrawScanlineEx<0x445944c8>;
	m_dsmap[0x00009808] = &GSRasterizer::DrawScanlineEx<0x00009808>;
	m_dsmap[0x00005909] = &GSRasterizer::DrawScanlineEx<0x00005909>;
	m_dsmap[0x88507b08] = &GSRasterizer::DrawScanlineEx<0x88507b08>;
	m_dsmap[0x0010580a] = &GSRasterizer::DrawScanlineEx<0x0010580a>;
	m_dsmap[0x44507048] = &GSRasterizer::DrawScanlineEx<0x44507048>;
	m_dsmap[0x98504408] = &GSRasterizer::DrawScanlineEx<0x98504408>;
	m_dsmap[0x00007008] = &GSRasterizer::DrawScanlineEx<0x00007008>;
	m_dsmap[0x485144c8] = &GSRasterizer::DrawScanlineEx<0x485144c8>;
	m_dsmap[0x445144c8] = &GSRasterizer::DrawScanlineEx<0x445144c8>;
	m_dsmap[0x425144c8] = &GSRasterizer::DrawScanlineEx<0x425144c8>;
	m_dsmap[0x48505808] = &GSRasterizer::DrawScanlineEx<0x48505808>;
	m_dsmap[0x44507848] = &GSRasterizer::DrawScanlineEx<0x44507848>;
	m_dsmap[0x64517808] = &GSRasterizer::DrawScanlineEx<0x64517808>;

	// tokyo bus guide

	m_dsmap[0x445a68a0] = &GSRasterizer::DrawScanlineEx<0x445a68a0>;
	m_dsmap[0x0009a8e0] = &GSRasterizer::DrawScanlineEx<0x0009a8e0>;
	m_dsmap[0x4451c4e0] = &GSRasterizer::DrawScanlineEx<0x4451c4e0>;
	m_dsmap[0x4459e8e0] = &GSRasterizer::DrawScanlineEx<0x4459e8e0>;
	m_dsmap[0x445228a0] = &GSRasterizer::DrawScanlineEx<0x445228a0>;
	m_dsmap[0x0001c460] = &GSRasterizer::DrawScanlineEx<0x0001c460>;
	m_dsmap[0x64506840] = &GSRasterizer::DrawScanlineEx<0x64506840>;
	m_dsmap[0x445228e0] = &GSRasterizer::DrawScanlineEx<0x445228e0>;
	m_dsmap[0x4451a8e0] = &GSRasterizer::DrawScanlineEx<0x4451a8e0>;
	m_dsmap[0x0009e8e0] = &GSRasterizer::DrawScanlineEx<0x0009e8e0>;
	m_dsmap[0x0001c4e0] = &GSRasterizer::DrawScanlineEx<0x0001c4e0>;
	m_dsmap[0x4451f860] = &GSRasterizer::DrawScanlineEx<0x4451f860>;
	m_dsmap[0x4451e860] = &GSRasterizer::DrawScanlineEx<0x4451e860>;
	m_dsmap[0x4459a8e0] = &GSRasterizer::DrawScanlineEx<0x4459a8e0>;
	m_dsmap[0x44540420] = &GSRasterizer::DrawScanlineEx<0x44540420>;
	m_dsmap[0x4451c460] = &GSRasterizer::DrawScanlineEx<0x4451c460>;

	// the punisher

	m_dsmap[0x44578408] = &GSRasterizer::DrawScanlineEx<0x44578408>;
	m_dsmap[0x4457840a] = &GSRasterizer::DrawScanlineEx<0x4457840a>;
	m_dsmap[0x4457aac8] = &GSRasterizer::DrawScanlineEx<0x4457aac8>;
	m_dsmap[0xa857aac8] = &GSRasterizer::DrawScanlineEx<0xa857aac8>;
	m_dsmap[0x4457a8c8] = &GSRasterizer::DrawScanlineEx<0x4457a8c8>;
	m_dsmap[0xa857a8c8] = &GSRasterizer::DrawScanlineEx<0xa857a8c8>;
	m_dsmap[0x4257a8c8] = &GSRasterizer::DrawScanlineEx<0x4257a8c8>;
	m_dsmap[0x68578408] = &GSRasterizer::DrawScanlineEx<0x68578408>;
	m_dsmap[0x4257a0c8] = &GSRasterizer::DrawScanlineEx<0x4257a0c8>;
	m_dsmap[0xa857848a] = &GSRasterizer::DrawScanlineEx<0xa857848a>;
	m_dsmap[0x6857a8c8] = &GSRasterizer::DrawScanlineEx<0x6857a8c8>;
	m_dsmap[0x4857b848] = &GSRasterizer::DrawScanlineEx<0x4857b848>;
	m_dsmap[0x4457abc8] = &GSRasterizer::DrawScanlineEx<0x4457abc8>;
	m_dsmap[0x4457b808] = &GSRasterizer::DrawScanlineEx<0x4457b808>;
	m_dsmap[0x4457b848] = &GSRasterizer::DrawScanlineEx<0x4457b848>;
	m_dsmap[0x4857a888] = &GSRasterizer::DrawScanlineEx<0x4857a888>;
	m_dsmap[0x44579808] = &GSRasterizer::DrawScanlineEx<0x44579808>;
	m_dsmap[0x4857a8c8] = &GSRasterizer::DrawScanlineEx<0x4857a8c8>;

	// sfex3

	m_dsmap[0x4451abf8] = &GSRasterizer::DrawScanlineEx<0x4451abf8>;
	m_dsmap[0x44505012] = &GSRasterizer::DrawScanlineEx<0x44505012>;
	m_dsmap[0x44505838] = &GSRasterizer::DrawScanlineEx<0x44505838>;
	m_dsmap[0x00007012] = &GSRasterizer::DrawScanlineEx<0x00007012>;
	m_dsmap[0x4851c4f8] = &GSRasterizer::DrawScanlineEx<0x4851c4f8>;
	m_dsmap[0x000184f8] = &GSRasterizer::DrawScanlineEx<0x000184f8>;
	m_dsmap[0x44518bd8] = &GSRasterizer::DrawScanlineEx<0x44518bd8>;
	m_dsmap[0x445068d8] = &GSRasterizer::DrawScanlineEx<0x445068d8>;
	m_dsmap[0x44519838] = &GSRasterizer::DrawScanlineEx<0x44519838>;
	m_dsmap[0x4851a8f8] = &GSRasterizer::DrawScanlineEx<0x4851a8f8>;
	m_dsmap[0x6a504438] = &GSRasterizer::DrawScanlineEx<0x6a504438>;
	m_dsmap[0x00005012] = &GSRasterizer::DrawScanlineEx<0x00005012>;
	m_dsmap[0x4851f878] = &GSRasterizer::DrawScanlineEx<0x4851f878>;
	m_dsmap[0x485044f8] = &GSRasterizer::DrawScanlineEx<0x485044f8>;
	m_dsmap[0x4271c498] = &GSRasterizer::DrawScanlineEx<0x4271c498>;
	m_dsmap[0x44518438] = &GSRasterizer::DrawScanlineEx<0x44518438>;
	m_dsmap[0x00018bd8] = &GSRasterizer::DrawScanlineEx<0x00018bd8>;
	m_dsmap[0xa9518478] = &GSRasterizer::DrawScanlineEx<0xa9518478>;
	m_dsmap[0x44505978] = &GSRasterizer::DrawScanlineEx<0x44505978>;
	m_dsmap[0x000044b8] = &GSRasterizer::DrawScanlineEx<0x000044b8>;
	m_dsmap[0x48506878] = &GSRasterizer::DrawScanlineEx<0x48506878>;
	m_dsmap[0x645044b8] = &GSRasterizer::DrawScanlineEx<0x645044b8>;
	m_dsmap[0x4271cb98] = &GSRasterizer::DrawScanlineEx<0x4271cb98>;
	m_dsmap[0x4851b878] = &GSRasterizer::DrawScanlineEx<0x4851b878>;

	// ico

	m_dsmap[0x4453a8c0] = &GSRasterizer::DrawScanlineEx<0x4453a8c0>;
	m_dsmap[0x64904400] = &GSRasterizer::DrawScanlineEx<0x64904400>;
	m_dsmap[0x48504440] = &GSRasterizer::DrawScanlineEx<0x48504440>;
	m_dsmap[0x44505840] = &GSRasterizer::DrawScanlineEx<0x44505840>;
	m_dsmap[0x64507800] = &GSRasterizer::DrawScanlineEx<0x64507800>;
	m_dsmap[0x44504400] = &GSRasterizer::DrawScanlineEx<0x44504400>;
	m_dsmap[0x4471b800] = &GSRasterizer::DrawScanlineEx<0x4471b800>;
	m_dsmap[0x44907800] = &GSRasterizer::DrawScanlineEx<0x44907800>;
	m_dsmap[0x44504420] = &GSRasterizer::DrawScanlineEx<0x44504420>;
	m_dsmap[0x6853a8c0] = &GSRasterizer::DrawScanlineEx<0x6853a8c0>;
	m_dsmap[0x0003a8c0] = &GSRasterizer::DrawScanlineEx<0x0003a8c0>;
	m_dsmap[0x485044c0] = &GSRasterizer::DrawScanlineEx<0x485044c0>;
	m_dsmap[0x68504400] = &GSRasterizer::DrawScanlineEx<0x68504400>;
	m_dsmap[0x68504440] = &GSRasterizer::DrawScanlineEx<0x68504440>;
	m_dsmap[0x48507800] = &GSRasterizer::DrawScanlineEx<0x48507800>;
	m_dsmap[0x4493a8c0] = &GSRasterizer::DrawScanlineEx<0x4493a8c0>;
	m_dsmap[0x6253a8c0] = &GSRasterizer::DrawScanlineEx<0x6253a8c0>;
	m_dsmap[0x64504400] = &GSRasterizer::DrawScanlineEx<0x64504400>;
	m_dsmap[0x00004440] = &GSRasterizer::DrawScanlineEx<0x00004440>;
	m_dsmap[0x4853a840] = &GSRasterizer::DrawScanlineEx<0x4853a840>;
	m_dsmap[0x64907800] = &GSRasterizer::DrawScanlineEx<0x64907800>;
	m_dsmap[0x4490b800] = &GSRasterizer::DrawScanlineEx<0x4490b800>;
	m_dsmap[0x4453b840] = &GSRasterizer::DrawScanlineEx<0x4453b840>;
	m_dsmap[0x44507800] = &GSRasterizer::DrawScanlineEx<0x44507800>;
	m_dsmap[0x4853a8c0] = &GSRasterizer::DrawScanlineEx<0x4853a8c0>;
	m_dsmap[0x68507800] = &GSRasterizer::DrawScanlineEx<0x68507800>;
	m_dsmap[0x48504400] = &GSRasterizer::DrawScanlineEx<0x48504400>;
	m_dsmap[0x00007a00] = &GSRasterizer::DrawScanlineEx<0x00007a00>;

	// ffx-2

	m_dsmap[0x8851a8c9] = &GSRasterizer::DrawScanlineEx<0x8851a8c9>;
	m_dsmap[0x44521029] = &GSRasterizer::DrawScanlineEx<0x44521029>;
	m_dsmap[0x4851f829] = &GSRasterizer::DrawScanlineEx<0x4851f829>;
	m_dsmap[0x44520429] = &GSRasterizer::DrawScanlineEx<0x44520429>;
	m_dsmap[0x44763028] = &GSRasterizer::DrawScanlineEx<0x44763028>;
	m_dsmap[0x447230a8] = &GSRasterizer::DrawScanlineEx<0x447230a8>;
	m_dsmap[0x44721028] = &GSRasterizer::DrawScanlineEx<0x44721028>;
	m_dsmap[0x64505029] = &GSRasterizer::DrawScanlineEx<0x64505029>;
	m_dsmap[0x4859a8c9] = &GSRasterizer::DrawScanlineEx<0x4859a8c9>;
	m_dsmap[0x44720428] = &GSRasterizer::DrawScanlineEx<0x44720428>;
	m_dsmap[0x4451d829] = &GSRasterizer::DrawScanlineEx<0x4451d829>;
	m_dsmap[0x4451d849] = &GSRasterizer::DrawScanlineEx<0x4451d849>;
	m_dsmap[0x4451f8e9] = &GSRasterizer::DrawScanlineEx<0x4451f8e9>;
	m_dsmap[0x44540429] = &GSRasterizer::DrawScanlineEx<0x44540429>;
	m_dsmap[0x4451c449] = &GSRasterizer::DrawScanlineEx<0x4451c449>;
	m_dsmap[0x4451b009] = &GSRasterizer::DrawScanlineEx<0x4451b009>;
	m_dsmap[0x48504439] = &GSRasterizer::DrawScanlineEx<0x48504439>;
	m_dsmap[0x44505009] = &GSRasterizer::DrawScanlineEx<0x44505009>;
	m_dsmap[0x4451b849] = &GSRasterizer::DrawScanlineEx<0x4451b849>;
	m_dsmap[0xa9504409] = &GSRasterizer::DrawScanlineEx<0xa9504409>;
	m_dsmap[0x00119809] = &GSRasterizer::DrawScanlineEx<0x00119809>;
	m_dsmap[0x00105889] = &GSRasterizer::DrawScanlineEx<0x00105889>;
	m_dsmap[0x00007059] = &GSRasterizer::DrawScanlineEx<0x00007059>;
	m_dsmap[0x44519809] = &GSRasterizer::DrawScanlineEx<0x44519809>;
	m_dsmap[0x48518429] = &GSRasterizer::DrawScanlineEx<0x48518429>;
	m_dsmap[0x4451c4e9] = &GSRasterizer::DrawScanlineEx<0x4451c4e9>;
	m_dsmap[0x485044d9] = &GSRasterizer::DrawScanlineEx<0x485044d9>;
	m_dsmap[0x00119889] = &GSRasterizer::DrawScanlineEx<0x00119889>;
	m_dsmap[0x4851d829] = &GSRasterizer::DrawScanlineEx<0x4851d829>;
	m_dsmap[0x00118489] = &GSRasterizer::DrawScanlineEx<0x00118489>;
	m_dsmap[0x44518889] = &GSRasterizer::DrawScanlineEx<0x44518889>;
	m_dsmap[0x64504429] = &GSRasterizer::DrawScanlineEx<0x64504429>;
	m_dsmap[0x4851b809] = &GSRasterizer::DrawScanlineEx<0x4851b809>;
	m_dsmap[0x48505009] = &GSRasterizer::DrawScanlineEx<0x48505009>;
	m_dsmap[0x4851b849] = &GSRasterizer::DrawScanlineEx<0x4851b849>;
	m_dsmap[0x485244b9] = &GSRasterizer::DrawScanlineEx<0x485244b9>;
	m_dsmap[0x44523839] = &GSRasterizer::DrawScanlineEx<0x44523839>;
	m_dsmap[0x00105009] = &GSRasterizer::DrawScanlineEx<0x00105009>;

	// God of War

	m_dsmap[0x445768c8] = &GSRasterizer::DrawScanlineEx<0x445768c8>;
	m_dsmap[0x68504448] = &GSRasterizer::DrawScanlineEx<0x68504448>;
	m_dsmap[0x44504428] = &GSRasterizer::DrawScanlineEx<0x44504428>;
	m_dsmap[0x015768c8] = &GSRasterizer::DrawScanlineEx<0x015768c8>;
	m_dsmap[0x00504428] = &GSRasterizer::DrawScanlineEx<0x00504428>;
	m_dsmap[0x00504408] = &GSRasterizer::DrawScanlineEx<0x00504408>;
	m_dsmap[0x44576bc8] = &GSRasterizer::DrawScanlineEx<0x44576bc8>;
	m_dsmap[0x485768c8] = &GSRasterizer::DrawScanlineEx<0x485768c8>;
	m_dsmap[0x445044c8] = &GSRasterizer::DrawScanlineEx<0x445044c8>;
	m_dsmap[0x445068c8] = &GSRasterizer::DrawScanlineEx<0x445068c8>;
	m_dsmap[0x0010590a] = &GSRasterizer::DrawScanlineEx<0x0010590a>;
	m_dsmap[0x00120409] = &GSRasterizer::DrawScanlineEx<0x00120409>;
	m_dsmap[0x00123808] = &GSRasterizer::DrawScanlineEx<0x00123808>;
	m_dsmap[0xa4504409] = &GSRasterizer::DrawScanlineEx<0xa4504409>;
	m_dsmap[0x425768c8] = &GSRasterizer::DrawScanlineEx<0x425768c8>;
	m_dsmap[0x00123809] = &GSRasterizer::DrawScanlineEx<0x00123809>;
	m_dsmap[0x0051b828] = &GSRasterizer::DrawScanlineEx<0x0051b828>;
	m_dsmap[0x00504448] = &GSRasterizer::DrawScanlineEx<0x00504448>;

	// dbzbt2

	m_dsmap[0x64507008] = &GSRasterizer::DrawScanlineEx<0x64507008>;
	m_dsmap[0x625069c8] = &GSRasterizer::DrawScanlineEx<0x625069c8>;
	m_dsmap[0x44577808] = &GSRasterizer::DrawScanlineEx<0x44577808>;
	m_dsmap[0x48505908] = &GSRasterizer::DrawScanlineEx<0x48505908>;
	m_dsmap[0x6451f908] = &GSRasterizer::DrawScanlineEx<0x6451f908>;
	m_dsmap[0x4451f808] = &GSRasterizer::DrawScanlineEx<0x4451f808>;
	m_dsmap[0x64505008] = &GSRasterizer::DrawScanlineEx<0x64505008>;
	m_dsmap[0x52504408] = &GSRasterizer::DrawScanlineEx<0x52504408>;
	m_dsmap[0x447068c8] = &GSRasterizer::DrawScanlineEx<0x447068c8>;
	m_dsmap[0x54507108] = &GSRasterizer::DrawScanlineEx<0x54507108>;
	m_dsmap[0x4450590a] = &GSRasterizer::DrawScanlineEx<0x4450590a>;
	m_dsmap[0x44505108] = &GSRasterizer::DrawScanlineEx<0x44505108>;
	m_dsmap[0x6450440a] = &GSRasterizer::DrawScanlineEx<0x6450440a>;
	m_dsmap[0x44505968] = &GSRasterizer::DrawScanlineEx<0x44505968>;
	m_dsmap[0x585069c8] = &GSRasterizer::DrawScanlineEx<0x585069c8>;
	m_dsmap[0x58507808] = &GSRasterizer::DrawScanlineEx<0x58507808>;
	m_dsmap[0xa171c8c8] = &GSRasterizer::DrawScanlineEx<0xa171c8c8>;
	m_dsmap[0x44573808] = &GSRasterizer::DrawScanlineEx<0x44573808>;
	m_dsmap[0x64504488] = &GSRasterizer::DrawScanlineEx<0x64504488>;
	m_dsmap[0x64505908] = &GSRasterizer::DrawScanlineEx<0x64505908>;
	m_dsmap[0x4451f908] = &GSRasterizer::DrawScanlineEx<0x4451f908>;
	m_dsmap[0x44707808] = &GSRasterizer::DrawScanlineEx<0x44707808>;
	m_dsmap[0x44506848] = &GSRasterizer::DrawScanlineEx<0x44506848>;
	m_dsmap[0x54507008] = &GSRasterizer::DrawScanlineEx<0x54507008>;
	m_dsmap[0x68507008] = &GSRasterizer::DrawScanlineEx<0x68507008>;
	m_dsmap[0x6250590a] = &GSRasterizer::DrawScanlineEx<0x6250590a>;
	m_dsmap[0x68504408] = &GSRasterizer::DrawScanlineEx<0x68504408>;
	m_dsmap[0x48507828] = &GSRasterizer::DrawScanlineEx<0x48507828>;
	m_dsmap[0x48506848] = &GSRasterizer::DrawScanlineEx<0x48506848>;
	m_dsmap[0x54507808] = &GSRasterizer::DrawScanlineEx<0x54507808>;

	// dbzbt3

	m_dsmap[0x48704408] = &GSRasterizer::DrawScanlineEx<0x48704408>;
	m_dsmap[0x44507108] = &GSRasterizer::DrawScanlineEx<0x44507108>;
	m_dsmap[0x5451b848] = &GSRasterizer::DrawScanlineEx<0x5451b848>;
	m_dsmap[0x485044c8] = &GSRasterizer::DrawScanlineEx<0x485044c8>;
	m_dsmap[0x4471c8c8] = &GSRasterizer::DrawScanlineEx<0x4471c8c8>;
	m_dsmap[0x48707808] = &GSRasterizer::DrawScanlineEx<0x48707808>;
	m_dsmap[0x495068c8] = &GSRasterizer::DrawScanlineEx<0x495068c8>;

	// suikoden 5

	m_dsmap[0x485068d8] = &GSRasterizer::DrawScanlineEx<0x485068d8>;
	m_dsmap[0x00024418] = &GSRasterizer::DrawScanlineEx<0x00024418>;
	m_dsmap[0x445244b8] = &GSRasterizer::DrawScanlineEx<0x445244b8>;
	m_dsmap[0x48536898] = &GSRasterizer::DrawScanlineEx<0x48536898>;
	m_dsmap[0x68506898] = &GSRasterizer::DrawScanlineEx<0x68506898>;
	m_dsmap[0x485868d8] = &GSRasterizer::DrawScanlineEx<0x485868d8>;
	m_dsmap[0x0a536898] = &GSRasterizer::DrawScanlineEx<0x0a536898>;
	m_dsmap[0x685068d8] = &GSRasterizer::DrawScanlineEx<0x685068d8>;
	m_dsmap[0x025068d8] = &GSRasterizer::DrawScanlineEx<0x025068d8>;
	m_dsmap[0x42536898] = &GSRasterizer::DrawScanlineEx<0x42536898>;
	m_dsmap[0x44534858] = &GSRasterizer::DrawScanlineEx<0x44534858>;
	m_dsmap[0x025868d8] = &GSRasterizer::DrawScanlineEx<0x025868d8>;
	m_dsmap[0x0a524498] = &GSRasterizer::DrawScanlineEx<0x0a524498>;

	// disgaea 2

	m_dsmap[0x4453a0c8] = &GSRasterizer::DrawScanlineEx<0x4453a0c8>;
	m_dsmap[0x44504448] = &GSRasterizer::DrawScanlineEx<0x44504448>;
	m_dsmap[0x4451b8c8] = &GSRasterizer::DrawScanlineEx<0x4451b8c8>;
	m_dsmap[0x2a51a8e8] = &GSRasterizer::DrawScanlineEx<0x2a51a8e8>;
	m_dsmap[0x6251a8c8] = &GSRasterizer::DrawScanlineEx<0x6251a8c8>;

	// dq8

	m_dsmap[0x2a505808] = &GSRasterizer::DrawScanlineEx<0x2a505808>;
	m_dsmap[0x44504488] = &GSRasterizer::DrawScanlineEx<0x44504488>;
	m_dsmap[0x46507808] = &GSRasterizer::DrawScanlineEx<0x46507808>;
	m_dsmap[0x48507848] = &GSRasterizer::DrawScanlineEx<0x48507848>;

	// xenosaga 2

	m_dsmap[0x000844c8] = &GSRasterizer::DrawScanlineEx<0x000844c8>;
	m_dsmap[0x4a526808] = &GSRasterizer::DrawScanlineEx<0x4a526808>;
	m_dsmap[0x64507828] = &GSRasterizer::DrawScanlineEx<0x64507828>;
	m_dsmap[0x5851a848] = &GSRasterizer::DrawScanlineEx<0x5851a848>;
	m_dsmap[0x5451a848] = &GSRasterizer::DrawScanlineEx<0x5451a848>;
	m_dsmap[0x68504428] = &GSRasterizer::DrawScanlineEx<0x68504428>;
	m_dsmap[0x00320408] = &GSRasterizer::DrawScanlineEx<0x00320408>;
	m_dsmap[0x5251a848] = &GSRasterizer::DrawScanlineEx<0x5251a848>;
	m_dsmap[0x00020408] = &GSRasterizer::DrawScanlineEx<0x00020408>;
	m_dsmap[0x64504428] = &GSRasterizer::DrawScanlineEx<0x64504428>;
	m_dsmap[0x00086888] = &GSRasterizer::DrawScanlineEx<0x00086888>;
	m_dsmap[0x895068c8] = &GSRasterizer::DrawScanlineEx<0x895068c8>;
	m_dsmap[0x00084488] = &GSRasterizer::DrawScanlineEx<0x00084488>;
	m_dsmap[0x4451b868] = &GSRasterizer::DrawScanlineEx<0x4451b868>;
	m_dsmap[0x000068c8] = &GSRasterizer::DrawScanlineEx<0x000068c8>;
	m_dsmap[0x0051b868] = &GSRasterizer::DrawScanlineEx<0x0051b868>;
	m_dsmap[0x4450a8c8] = &GSRasterizer::DrawScanlineEx<0x4450a8c8>;
	m_dsmap[0x62504428] = &GSRasterizer::DrawScanlineEx<0x62504428>;
	m_dsmap[0x54507828] = &GSRasterizer::DrawScanlineEx<0x54507828>;

	// persona 4

	m_dsmap[0x685840d8] = &GSRasterizer::DrawScanlineEx<0x685840d8>;
	m_dsmap[0x48522098] = &GSRasterizer::DrawScanlineEx<0x48522098>;
	m_dsmap[0x485a44d8] = &GSRasterizer::DrawScanlineEx<0x485a44d8>;
	m_dsmap[0x44522898] = &GSRasterizer::DrawScanlineEx<0x44522898>;
	m_dsmap[0x58522098] = &GSRasterizer::DrawScanlineEx<0x58522098>;
	m_dsmap[0x6451e8d8] = &GSRasterizer::DrawScanlineEx<0x6451e8d8>;
	m_dsmap[0x001044b8] = &GSRasterizer::DrawScanlineEx<0x001044b8>;
	m_dsmap[0x58562898] = &GSRasterizer::DrawScanlineEx<0x58562898>;
	m_dsmap[0x545068d8] = &GSRasterizer::DrawScanlineEx<0x545068d8>;
	m_dsmap[0x001044d8] = &GSRasterizer::DrawScanlineEx<0x001044d8>;
	m_dsmap[0x4451a8d8] = &GSRasterizer::DrawScanlineEx<0x4451a8d8>;
	m_dsmap[0x445044b8] = &GSRasterizer::DrawScanlineEx<0x445044b8>;
	m_dsmap[0x425a44d8] = &GSRasterizer::DrawScanlineEx<0x425a44d8>;
	m_dsmap[0x4451a0d8] = &GSRasterizer::DrawScanlineEx<0x4451a0d8>;
	m_dsmap[0x54504498] = &GSRasterizer::DrawScanlineEx<0x54504498>;
	m_dsmap[0x52562898] = &GSRasterizer::DrawScanlineEx<0x52562898>;
	m_dsmap[0x00004498] = &GSRasterizer::DrawScanlineEx<0x00004498>;
	m_dsmap[0x58560498] = &GSRasterizer::DrawScanlineEx<0x58560498>;
	m_dsmap[0x425a28d8] = &GSRasterizer::DrawScanlineEx<0x425a28d8>;
	m_dsmap[0x585184d8] = &GSRasterizer::DrawScanlineEx<0x585184d8>;
	m_dsmap[0x44534498] = &GSRasterizer::DrawScanlineEx<0x44534498>;
	m_dsmap[0x000044d8] = &GSRasterizer::DrawScanlineEx<0x000044d8>;
	m_dsmap[0x445344d8] = &GSRasterizer::DrawScanlineEx<0x445344d8>;
	m_dsmap[0x485244d8] = &GSRasterizer::DrawScanlineEx<0x485244d8>;
	m_dsmap[0x445b44d8] = &GSRasterizer::DrawScanlineEx<0x445b44d8>;
	m_dsmap[0x000a44d8] = &GSRasterizer::DrawScanlineEx<0x000a44d8>;

/*
	CAtlMap<DWORD, bool> dsmap;

	POSITION pos = m_dsmap.GetStartPosition();

	while(pos)
	{
		CAtlMap<DWORD, DrawScanlinePtr>::CPair* pair = m_dsmap.GetNext(pos);

		ScanlineSelector sel = *(ScanlineSelector*)&pair->m_key;

		if(sel.tfx == 4)
		{
			sel.tcc = 0;
			sel.fst = 0;
			sel.ltf = 0;
		}

		if(sel.atst == 1)
		{
			sel.afail = 0;
		}

		ASSERT(!sel.date || sel.rfb);
		ASSERT(!sel.abe || sel.rfb);

		if(!sel.rfb)
		{
			sel.date = 0;
			sel.abe = 0;
		}

		if(!sel.abe)
		{
			sel.abea = sel.abeb = sel.abec = sel.abed = 0;
		}

		dsmap[sel] = true;
	}

	TRACE(_T("%d %d\n"), m_dsmap.GetCount(), dsmap.GetCount());
*/
}

template<DWORD sel>
void GSRasterizer::DrawScanlineEx(int top, int left, int right, const Vertex& v)
{
	const DWORD ztst = (sel >> 5) & 3;
	const DWORD iip = (sel >> 7) & 1;
	const DWORD tfx = (sel >> 8) & 7;
	const DWORD tcc = (sel >> 11) & 1;
	const DWORD fst = (sel >> 12) & 1;
	const DWORD ltf = (sel >> 13) & 1;
	const DWORD atst = (sel >> 14) & 7;
	const DWORD afail = (sel >> 17) & 3;
	const DWORD fge = (sel >> 19) & 1;
	const DWORD rfb = (sel >> 20) & 1;
	const DWORD date = (sel >> 21) & 1;
	const DWORD abe = (sel >> 22) & 3;
	const DWORD abea = (sel >> 24) & 3;
	const DWORD abeb = (sel >> 26) & 3;
	const DWORD abec = (sel >> 28) & 3;
	const DWORD abed = (sel >> 30) & 3;

	int fpsm = GSUtil::DecodeFPSM(((sel >> 0) & 7));
	int zpsm = GSUtil::DecodeZPSM(((sel >> 3) & 3));

	GSVector4i fa_base = m_slenv.fbco[top];
	GSVector4i* fa_offset = (GSVector4i*)&m_slenv.fo[left];

	GSVector4i za_base = m_slenv.zbco[top];
	GSVector4i* za_offset = (GSVector4i*)&m_slenv.zo[left];

	GSVector4 vp = v.p;
	GSVector4 z = vp.zzzz(); z += m_slenv.dz0123;
	GSVector4 f = vp.wwww(); f += m_slenv.df0123;

	GSVector4 vt = v.t;
	GSVector4 s = vt.xxxx(); s += m_slenv.ds0123;
	GSVector4 t = vt.yyyy(); t += m_slenv.dt0123;
	GSVector4 q = vt.zzzz(); q += m_slenv.dq0123;

	GSVector4 vc = v.c;
	GSVector4 r = vc.xxxx(); if(iip) r += m_slenv.dr0123;
	GSVector4 g = vc.yyyy(); if(iip) g += m_slenv.dg0123;
	GSVector4 b = vc.zzzz(); if(iip) b += m_slenv.db0123;
	GSVector4 a = vc.wwww(); if(iip) a += m_slenv.da0123;

	int steps = right - left;

	m_slenv.steps += steps;

	while(1)
	{
		do
		{

		int pixels = min(steps, 4);

		GSVector4i fa = fa_base + GSVector4i::loadu(fa_offset);
		GSVector4i za = za_base + GSVector4i::loadu(za_offset);
		
		GSVector4i fm = m_slenv.fm;
		GSVector4i zm = m_slenv.zm;
		GSVector4i test = GSVector4i::zero();

		GSVector4i zs = (GSVector4i(z * 0.5f) << 1) | (GSVector4i(z) & 1);
		GSVector4i zd;

		if(ztst > 1)
		{
			zd = m_state->m_mem.ReadZBufX(zpsm, za);

			switch(ztst)
			{
			case 2: test = (zs - 0x80000000) < (zd - 0x80000000); break; // ge
			case 3: test = (zs - 0x80000000) <= (zd - 0x80000000); break; // g
			default: __assume(0);
			}

			if(test.mask() == 0xffff)
			{
				continue;
			}
		}

		GSVector4 c[12];

		if(tfx < 4)
		{
			GSVector4 u = s;
			GSVector4 v = t;

			if(!fst)
			{
				GSVector4 w = q.rcp();

				u *= w;
				v *= w;
			}

			if(ltf)
			{
				u -= 0.5f;
				v -= 0.5f;

				GSVector4 uf = u.floor();
				GSVector4 vf = v.floor();
				
				GSVector4 uff = u - uf;
				GSVector4 vff = v - vf;

				GSVector4i uv = GSVector4i(uf).ps32(GSVector4i(vf));

				GSVector4i uv0 = Wrap(uv);
				GSVector4i uv1 = Wrap(uv + 0x00010001);

				for(int i = 0; i < pixels; i++)
				{
					if(ztst > 1 && test.u32[i])
					{
						continue;
					}

					FetchTexel(uv0.u16[i], uv0.u16[i + 4]);
					FetchTexel(uv1.u16[i], uv0.u16[i + 4]);
					FetchTexel(uv0.u16[i], uv1.u16[i + 4]);
					FetchTexel(uv1.u16[i], uv1.u16[i + 4]);

					GSVector4 c00(ReadTexelNoFetch(uv0.u16[i], uv0.u16[i + 4]));
					GSVector4 c01(ReadTexelNoFetch(uv1.u16[i], uv0.u16[i + 4]));
					GSVector4 c10(ReadTexelNoFetch(uv0.u16[i], uv1.u16[i + 4]));
					GSVector4 c11(ReadTexelNoFetch(uv1.u16[i], uv1.u16[i + 4]));

					c00 = c00.lerp(c01, uff.v[i]);
					c10 = c10.lerp(c11, uff.v[i]);
					c00 = c00.lerp(c10, vff.v[i]);

					c[i] = c00;
				}

				GSVector4::transpose(c[0], c[1], c[2], c[3]);
			}
			else
			{
				GSVector4i uv = Wrap(GSVector4i(u).ps32(GSVector4i(v)));

				GSVector4i c00;

				for(int i = 0; i < pixels; i++)
				{
					if(ztst > 1 && test.u32[i])
					{
						continue;
					}

					c00.u32[i] = ReadTexel(uv.u16[i], uv.u16[i + 4]);
				}

				GSVector4::expand(c00, c[0], c[1], c[2], c[3]);
			}
		}

		switch(tfx)
		{
		case 0: c[3] = tcc ? c[3].mod2x(a).sat() : a; break;
		case 1: break;
		case 2: c[3] = tcc ? (c[3] + a).sat() : a; break;
		case 3: if(!tcc) c[3] = a; break;
		case 4: c[3] = a; break; 
		default: __assume(0);
		}

		if(atst != 1)
		{
			GSVector4i t;

			switch(atst)
			{
			case 0: t = GSVector4i::invzero(); break; // never 
			case 1: t = GSVector4i::zero(); break; // always
			case 2: case 3: t = GSVector4i(c[3]) > m_slenv.aref; break; // l, le
			case 4: t = GSVector4i(c[3]) != m_slenv.aref; break; // e
			case 5: case 6: t = GSVector4i(c[3]) < m_slenv.aref; break; // ge, g
			case 7: t = GSVector4i(c[3]) == m_slenv.aref; break; // ne 
			default: __assume(0);
			}

			switch(afail)
			{
			case 0:
				fm |= t;
				zm |= t;
				test |= t;
				if(test.mask() == 0xffff) continue;
				break;
			case 1:
				zm |= t;
				break;
			case 2:
				fm |= t;
				break;
			case 3: 
				fm |= t & 0xff000000;
				zm |= t;
				break;
			default: 
				__assume(0);
			}
		}

		switch(tfx)
		{
		case 0:
			c[0] = c[0].mod2x(r);
			c[1] = c[1].mod2x(g);
			c[2] = c[2].mod2x(b);
			c[0] = c[0].sat();
			c[1] = c[1].sat();
			c[2] = c[2].sat();
			break;
		case 1:
			break;
		case 2:
		case 3:
			c[0] = c[0].mod2x(r) + a;
			c[1] = c[1].mod2x(g) + a;
			c[2] = c[2].mod2x(b) + a;
			c[0] = c[0].sat();
			c[1] = c[1].sat();
			c[2] = c[2].sat();
			break;
		case 4:
			c[0] = r;
			c[1] = g;
			c[2] = b;
			break;
		default:
			__assume(0);
		}

		if(fge)
		{
			c[0] = m_slenv.f.r.lerp(c[0], f);
			c[1] = m_slenv.f.g.lerp(c[1], f);
			c[2] = m_slenv.f.b.lerp(c[2], f);
		}

		GSVector4i d = GSVector4i::zero();

		if(rfb)
		{
			d = m_state->m_mem.ReadFrameX(fpsm, fa);

			if(date)
			{
				test |= (d ^ m_slenv.datm).sra32(31);

				if(test.mask() == 0xffff)
				{
					continue;
				}
			}
		}

		fm |= test;
		zm |= test;

		if(abe)
		{
			GSVector4::expand(d, c[4], c[5], c[6], c[7]);

			c[8] = GSVector4::zero();
			c[9] = GSVector4::zero();
			c[10] = GSVector4::zero();
			c[11] = m_slenv.afix;

			GSVector4 r = (c[abea*4 + 0] - c[abeb*4 + 0]).mod2x(c[abec*4 + 3]) + c[abed*4 + 0];
			GSVector4 g = (c[abea*4 + 1] - c[abeb*4 + 1]).mod2x(c[abec*4 + 3]) + c[abed*4 + 1];
			GSVector4 b = (c[abea*4 + 2] - c[abeb*4 + 2]).mod2x(c[abec*4 + 3]) + c[abed*4 + 2];

			if(abe == 2)
			{
				GSVector4 mask = c[3] >= GSVector4(128.0f);

				c[0] = c[0].blend8(r, mask);
				c[1] = c[1].blend8(g, mask);
				c[2] = c[2].blend8(b, mask);
			}
			else
			{
				c[0] = r;
				c[1] = g;
				c[2] = b;
			}
		}

		GSVector4i rb = GSVector4i(c[0]).ps32(GSVector4i(c[2]));
		GSVector4i ga = GSVector4i(c[1]).ps32(GSVector4i(c[3]));
		
		GSVector4i rg = rb.upl16(ga) & m_slenv.colclamp;
		GSVector4i ba = rb.uph16(ga) & m_slenv.colclamp;
		
		GSVector4i s = rg.upl32(ba).pu16(rg.uph32(ba)) | m_slenv.fba;

		if(rfb)
		{
			s = s.blend(d, fm);
		}

		m_state->m_mem.WriteFrameX(fpsm, fa, s, fm, pixels);

		if(ztst > 0 && !(atst == 0 && afail != 2))
		{
			m_state->m_mem.WriteZBufX(zpsm, za, zs, zm, pixels);
		}

		}
		while(0);

		steps -= 4;

		if(steps <= 0) break;

		fa_offset++;
		za_offset++;
		z += m_slenv.dz;
		f += m_slenv.df;
		s += m_slenv.ds;
		t += m_slenv.dt;
		q += m_slenv.dq;
		if(iip) r += m_slenv.dr;
		if(iip) g += m_slenv.dg;
		if(iip) b += m_slenv.db;
		if(iip) a += m_slenv.da;
	}
}
