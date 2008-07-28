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

	// bios

	m_dsmap.SetAt(0x00002230, &GSRasterizer::DrawScanlineEx<0x00002230>);
	m_dsmap.SetAt(0x00002244, &GSRasterizer::DrawScanlineEx<0x00002244>);
	m_dsmap.SetAt(0x00002250, &GSRasterizer::DrawScanlineEx<0x00002250>);
	m_dsmap.SetAt(0x00003060, &GSRasterizer::DrawScanlineEx<0x00003060>);
	m_dsmap.SetAt(0x00003404, &GSRasterizer::DrawScanlineEx<0x00003404>);
	m_dsmap.SetAt(0x00003470, &GSRasterizer::DrawScanlineEx<0x00003470>);
	m_dsmap.SetAt(0x00003c10, &GSRasterizer::DrawScanlineEx<0x00003c10>);
	m_dsmap.SetAt(0x00003c20, &GSRasterizer::DrawScanlineEx<0x00003c20>);
	m_dsmap.SetAt(0x00003c24, &GSRasterizer::DrawScanlineEx<0x00003c24>);
	m_dsmap.SetAt(0x00003c30, &GSRasterizer::DrawScanlineEx<0x00003c30>);
	m_dsmap.SetAt(0x40102060, &GSRasterizer::DrawScanlineEx<0x40102060>);
	m_dsmap.SetAt(0x50903410, &GSRasterizer::DrawScanlineEx<0x50903410>);
	m_dsmap.SetAt(0x50903420, &GSRasterizer::DrawScanlineEx<0x50903420>);
	m_dsmap.SetAt(0x51102210, &GSRasterizer::DrawScanlineEx<0x51102210>);
	m_dsmap.SetAt(0x51102260, &GSRasterizer::DrawScanlineEx<0x51102260>);
	m_dsmap.SetAt(0x51103c10, &GSRasterizer::DrawScanlineEx<0x51103c10>);
	m_dsmap.SetAt(0x51103c24, &GSRasterizer::DrawScanlineEx<0x51103c24>);
	m_dsmap.SetAt(0x5110dc10, &GSRasterizer::DrawScanlineEx<0x5110dc10>);
	m_dsmap.SetAt(0x52103404, &GSRasterizer::DrawScanlineEx<0x52103404>);
	m_dsmap.SetAt(0x52103420, &GSRasterizer::DrawScanlineEx<0x52103420>);
	m_dsmap.SetAt(0x52103444, &GSRasterizer::DrawScanlineEx<0x52103444>);
	m_dsmap.SetAt(0x52103c10, &GSRasterizer::DrawScanlineEx<0x52103c10>);
	m_dsmap.SetAt(0x56103404, &GSRasterizer::DrawScanlineEx<0x56103404>);
	m_dsmap.SetAt(0x58902204, &GSRasterizer::DrawScanlineEx<0x58902204>);
	m_dsmap.SetAt(0x58902c10, &GSRasterizer::DrawScanlineEx<0x58902c10>);
	m_dsmap.SetAt(0x59102210, &GSRasterizer::DrawScanlineEx<0x59102210>);
	m_dsmap.SetAt(0x59103c04, &GSRasterizer::DrawScanlineEx<0x59103c04>);
	m_dsmap.SetAt(0x59203c04, &GSRasterizer::DrawScanlineEx<0x59203c04>);
	m_dsmap.SetAt(0x5a102c10, &GSRasterizer::DrawScanlineEx<0x5a102c10>);
	m_dsmap.SetAt(0x5a203c24, &GSRasterizer::DrawScanlineEx<0x5a203c24>);
	m_dsmap.SetAt(0x00003020, &GSRasterizer::DrawScanlineEx<0x00003020>);
	m_dsmap.SetAt(0x00003c14, &GSRasterizer::DrawScanlineEx<0x00003c14>);
	m_dsmap.SetAt(0x52102210, &GSRasterizer::DrawScanlineEx<0x52102210>);
	m_dsmap.SetAt(0x59102250, &GSRasterizer::DrawScanlineEx<0x59102250>);
	m_dsmap.SetAt(0x51102220, &GSRasterizer::DrawScanlineEx<0x51102220>);

	// ffx

	m_dsmap.SetAt(0x5110e255, &GSRasterizer::DrawScanlineEx<0x5110e255>);
	m_dsmap.SetAt(0x51102205, &GSRasterizer::DrawScanlineEx<0x51102205>);
	m_dsmap.SetAt(0x5210e245, &GSRasterizer::DrawScanlineEx<0x5210e245>);
	m_dsmap.SetAt(0x5210c265, &GSRasterizer::DrawScanlineEx<0x5210c265>);
	m_dsmap.SetAt(0x5110d465, &GSRasterizer::DrawScanlineEx<0x5110d465>);
	m_dsmap.SetAt(0x591038a5, &GSRasterizer::DrawScanlineEx<0x591038a5>);
	m_dsmap.SetAt(0x5210d465, &GSRasterizer::DrawScanlineEx<0x5210d465>);
	m_dsmap.SetAt(0x6a502215, &GSRasterizer::DrawScanlineEx<0x6a502215>);
	m_dsmap.SetAt(0x00002896, &GSRasterizer::DrawScanlineEx<0x00002896>);
	m_dsmap.SetAt(0x5110e215, &GSRasterizer::DrawScanlineEx<0x5110e215>);
	m_dsmap.SetAt(0x00002215, &GSRasterizer::DrawScanlineEx<0x00002215>);
	m_dsmap.SetAt(0x5114c265, &GSRasterizer::DrawScanlineEx<0x5114c265>);
	m_dsmap.SetAt(0x59102895, &GSRasterizer::DrawScanlineEx<0x59102895>);
	m_dsmap.SetAt(0x5110d425, &GSRasterizer::DrawScanlineEx<0x5110d425>);
	m_dsmap.SetAt(0x51910265, &GSRasterizer::DrawScanlineEx<0x51910265>);
	m_dsmap.SetAt(0x5114d465, &GSRasterizer::DrawScanlineEx<0x5114d465>);
	m_dsmap.SetAt(0x0000d475, &GSRasterizer::DrawScanlineEx<0x0000d475>);
	m_dsmap.SetAt(0x5090dc05, &GSRasterizer::DrawScanlineEx<0x5090dc05>);
	m_dsmap.SetAt(0x5210d425, &GSRasterizer::DrawScanlineEx<0x5210d425>);
	m_dsmap.SetAt(0x5110fc55, &GSRasterizer::DrawScanlineEx<0x5110fc55>);
	m_dsmap.SetAt(0x0004c275, &GSRasterizer::DrawScanlineEx<0x0004c275>);
	m_dsmap.SetAt(0x0004d475, &GSRasterizer::DrawScanlineEx<0x0004d475>);
	m_dsmap.SetAt(0x5114fc75, &GSRasterizer::DrawScanlineEx<0x5114fc75>);
	m_dsmap.SetAt(0x5214fc75, &GSRasterizer::DrawScanlineEx<0x5214fc75>);
	m_dsmap.SetAt(0x5110fc25, &GSRasterizer::DrawScanlineEx<0x5110fc25>);
	m_dsmap.SetAt(0x5110fc15, &GSRasterizer::DrawScanlineEx<0x5110fc15>);
	m_dsmap.SetAt(0x51102245, &GSRasterizer::DrawScanlineEx<0x51102245>);
	m_dsmap.SetAt(0x5110fc05, &GSRasterizer::DrawScanlineEx<0x5110fc05>);
	m_dsmap.SetAt(0x5a10e425, &GSRasterizer::DrawScanlineEx<0x5a10e425>);
	m_dsmap.SetAt(0x5110dc05, &GSRasterizer::DrawScanlineEx<0x5110dc05>);
	m_dsmap.SetAt(0x00003435, &GSRasterizer::DrawScanlineEx<0x00003435>);
	m_dsmap.SetAt(0x0000d415, &GSRasterizer::DrawScanlineEx<0x0000d415>);
	m_dsmap.SetAt(0x5090c265, &GSRasterizer::DrawScanlineEx<0x5090c265>);
	m_dsmap.SetAt(0x5090e245, &GSRasterizer::DrawScanlineEx<0x5090e245>);
	m_dsmap.SetAt(0x51102255, &GSRasterizer::DrawScanlineEx<0x51102255>);
	m_dsmap.SetAt(0x51102275, &GSRasterizer::DrawScanlineEx<0x51102275>);
	m_dsmap.SetAt(0x51102815, &GSRasterizer::DrawScanlineEx<0x51102815>);
	m_dsmap.SetAt(0x5110c265, &GSRasterizer::DrawScanlineEx<0x5110c265>);
	m_dsmap.SetAt(0x5110d405, &GSRasterizer::DrawScanlineEx<0x5110d405>);
	m_dsmap.SetAt(0x5110fc45, &GSRasterizer::DrawScanlineEx<0x5110fc45>);
	m_dsmap.SetAt(0x52102245, &GSRasterizer::DrawScanlineEx<0x52102245>);
	m_dsmap.SetAt(0x52102275, &GSRasterizer::DrawScanlineEx<0x52102275>);
	m_dsmap.SetAt(0x5210d405, &GSRasterizer::DrawScanlineEx<0x5210d405>);
	m_dsmap.SetAt(0x59102275, &GSRasterizer::DrawScanlineEx<0x59102275>);
	m_dsmap.SetAt(0x5a103435, &GSRasterizer::DrawScanlineEx<0x5a103435>);
	m_dsmap.SetAt(0x6210c205, &GSRasterizer::DrawScanlineEx<0x6210c205>);
	m_dsmap.SetAt(0x0000d806, &GSRasterizer::DrawScanlineEx<0x0000d806>);
	m_dsmap.SetAt(0x50902245, &GSRasterizer::DrawScanlineEx<0x50902245>);
	m_dsmap.SetAt(0x5110c205, &GSRasterizer::DrawScanlineEx<0x5110c205>);
	m_dsmap.SetAt(0x51111045, &GSRasterizer::DrawScanlineEx<0x51111045>);
	m_dsmap.SetAt(0x51111075, &GSRasterizer::DrawScanlineEx<0x51111075>);
	m_dsmap.SetAt(0x52110265, &GSRasterizer::DrawScanlineEx<0x52110265>);

	// ffxii

	m_dsmap.SetAt(0x00020214, &GSRasterizer::DrawScanlineEx<0x00020214>);
	m_dsmap.SetAt(0x51110254, &GSRasterizer::DrawScanlineEx<0x51110254>);
	m_dsmap.SetAt(0x50903054, &GSRasterizer::DrawScanlineEx<0x50903054>);
	m_dsmap.SetAt(0x00011c14, &GSRasterizer::DrawScanlineEx<0x00011c14>);
	m_dsmap.SetAt(0x51110c14, &GSRasterizer::DrawScanlineEx<0x51110c14>);
	m_dsmap.SetAt(0x51130c14, &GSRasterizer::DrawScanlineEx<0x51130c14>);
	m_dsmap.SetAt(0x51110c54, &GSRasterizer::DrawScanlineEx<0x51110c54>);
	m_dsmap.SetAt(0x51130c54, &GSRasterizer::DrawScanlineEx<0x51130c54>);
	m_dsmap.SetAt(0x51111c14, &GSRasterizer::DrawScanlineEx<0x51111c14>);
	m_dsmap.SetAt(0x00002214, &GSRasterizer::DrawScanlineEx<0x00002214>);
	m_dsmap.SetAt(0x51111c54, &GSRasterizer::DrawScanlineEx<0x51111c54>);
	m_dsmap.SetAt(0x51131c54, &GSRasterizer::DrawScanlineEx<0x51131c54>);
	m_dsmap.SetAt(0x51131054, &GSRasterizer::DrawScanlineEx<0x51131054>);
	m_dsmap.SetAt(0x00003814, &GSRasterizer::DrawScanlineEx<0x00003814>);
	m_dsmap.SetAt(0x51110214, &GSRasterizer::DrawScanlineEx<0x51110214>);
	m_dsmap.SetAt(0x51102214, &GSRasterizer::DrawScanlineEx<0x51102214>);
	m_dsmap.SetAt(0x40030204, &GSRasterizer::DrawScanlineEx<0x40030204>);
	m_dsmap.SetAt(0x51111454, &GSRasterizer::DrawScanlineEx<0x51111454>);
	m_dsmap.SetAt(0x51111424, &GSRasterizer::DrawScanlineEx<0x51111424>);
	m_dsmap.SetAt(0x56130214, &GSRasterizer::DrawScanlineEx<0x56130214>);
	m_dsmap.SetAt(0x52511424, &GSRasterizer::DrawScanlineEx<0x52511424>);
	m_dsmap.SetAt(0x40082204, &GSRasterizer::DrawScanlineEx<0x40082204>);
	m_dsmap.SetAt(0x5115b464, &GSRasterizer::DrawScanlineEx<0x5115b464>);
	m_dsmap.SetAt(0x00002204, &GSRasterizer::DrawScanlineEx<0x00002204>);
	m_dsmap.SetAt(0x40030214, &GSRasterizer::DrawScanlineEx<0x40030214>);
	m_dsmap.SetAt(0x51111464, &GSRasterizer::DrawScanlineEx<0x51111464>);
	m_dsmap.SetAt(0x52111424, &GSRasterizer::DrawScanlineEx<0x52111424>);
	m_dsmap.SetAt(0x52511464, &GSRasterizer::DrawScanlineEx<0x52511464>);
	m_dsmap.SetAt(0x52130254, &GSRasterizer::DrawScanlineEx<0x52130254>);
	m_dsmap.SetAt(0x51131464, &GSRasterizer::DrawScanlineEx<0x51131464>);
	m_dsmap.SetAt(0x52110224, &GSRasterizer::DrawScanlineEx<0x52110224>);
	m_dsmap.SetAt(0x0004b464, &GSRasterizer::DrawScanlineEx<0x0004b464>);
	m_dsmap.SetAt(0x511b0224, &GSRasterizer::DrawScanlineEx<0x511b0224>);
	m_dsmap.SetAt(0x52131c04, &GSRasterizer::DrawScanlineEx<0x52131c04>);
	m_dsmap.SetAt(0x0004a264, &GSRasterizer::DrawScanlineEx<0x0004a264>);
	m_dsmap.SetAt(0x52130c14, &GSRasterizer::DrawScanlineEx<0x52130c14>);
	m_dsmap.SetAt(0x51131c14, &GSRasterizer::DrawScanlineEx<0x51131c14>);
	m_dsmap.SetAt(0x5215b464, &GSRasterizer::DrawScanlineEx<0x5215b464>);
	m_dsmap.SetAt(0x50910224, &GSRasterizer::DrawScanlineEx<0x50910224>);
	m_dsmap.SetAt(0x52111464, &GSRasterizer::DrawScanlineEx<0x52111464>);
	m_dsmap.SetAt(0x52131464, &GSRasterizer::DrawScanlineEx<0x52131464>);
	m_dsmap.SetAt(0x52110264, &GSRasterizer::DrawScanlineEx<0x52110264>);
	m_dsmap.SetAt(0x00011454, &GSRasterizer::DrawScanlineEx<0x00011454>);
	m_dsmap.SetAt(0x5115d464, &GSRasterizer::DrawScanlineEx<0x5115d464>);
	m_dsmap.SetAt(0x6a182204, &GSRasterizer::DrawScanlineEx<0x6a182204>);

	// culdcept

	m_dsmap.SetAt(0x00002216, &GSRasterizer::DrawScanlineEx<0x00002216>);
	m_dsmap.SetAt(0x0000e266, &GSRasterizer::DrawScanlineEx<0x0000e266>);
	m_dsmap.SetAt(0x0000ec26, &GSRasterizer::DrawScanlineEx<0x0000ec26>);
	m_dsmap.SetAt(0x0000f526, &GSRasterizer::DrawScanlineEx<0x0000f526>);
	m_dsmap.SetAt(0x0000f566, &GSRasterizer::DrawScanlineEx<0x0000f566>);
	m_dsmap.SetAt(0x0000fc26, &GSRasterizer::DrawScanlineEx<0x0000fc26>);
	m_dsmap.SetAt(0x0004f526, &GSRasterizer::DrawScanlineEx<0x0004f526>);
	m_dsmap.SetAt(0x0004f566, &GSRasterizer::DrawScanlineEx<0x0004f566>);
	m_dsmap.SetAt(0x5110e266, &GSRasterizer::DrawScanlineEx<0x5110e266>);
	m_dsmap.SetAt(0x5110e466, &GSRasterizer::DrawScanlineEx<0x5110e466>);
	m_dsmap.SetAt(0x5110ec26, &GSRasterizer::DrawScanlineEx<0x5110ec26>);
	m_dsmap.SetAt(0x5110f466, &GSRasterizer::DrawScanlineEx<0x5110f466>);
	m_dsmap.SetAt(0x5110fc26, &GSRasterizer::DrawScanlineEx<0x5110fc26>);
	m_dsmap.SetAt(0x5210e266, &GSRasterizer::DrawScanlineEx<0x5210e266>);
	m_dsmap.SetAt(0x5a10e266, &GSRasterizer::DrawScanlineEx<0x5a10e266>);
	m_dsmap.SetAt(0x5a10f566, &GSRasterizer::DrawScanlineEx<0x5a10f566>);
	m_dsmap.SetAt(0x5a10fc26, &GSRasterizer::DrawScanlineEx<0x5a10fc26>);
	m_dsmap.SetAt(0x6850f566, &GSRasterizer::DrawScanlineEx<0x6850f566>);

	// kingdom hearts

	m_dsmap.SetAt(0x00002205, &GSRasterizer::DrawScanlineEx<0x00002205>);
	m_dsmap.SetAt(0x00003c04, &GSRasterizer::DrawScanlineEx<0x00003c04>);
	m_dsmap.SetAt(0x00003c05, &GSRasterizer::DrawScanlineEx<0x00003c05>);
	m_dsmap.SetAt(0x0004d464, &GSRasterizer::DrawScanlineEx<0x0004d464>);
	m_dsmap.SetAt(0x40002218, &GSRasterizer::DrawScanlineEx<0x40002218>);
	m_dsmap.SetAt(0x40003c00, &GSRasterizer::DrawScanlineEx<0x40003c00>);
	m_dsmap.SetAt(0x4000c228, &GSRasterizer::DrawScanlineEx<0x4000c228>);
	m_dsmap.SetAt(0x4a94d468, &GSRasterizer::DrawScanlineEx<0x4a94d468>);
	m_dsmap.SetAt(0x5090c254, &GSRasterizer::DrawScanlineEx<0x5090c254>);
	m_dsmap.SetAt(0x51103414, &GSRasterizer::DrawScanlineEx<0x51103414>);
	m_dsmap.SetAt(0x51103c54, &GSRasterizer::DrawScanlineEx<0x51103c54>);
	m_dsmap.SetAt(0x5110c214, &GSRasterizer::DrawScanlineEx<0x5110c214>);
	m_dsmap.SetAt(0x5110c254, &GSRasterizer::DrawScanlineEx<0x5110c254>);
	m_dsmap.SetAt(0x5110d434, &GSRasterizer::DrawScanlineEx<0x5110d434>);
	m_dsmap.SetAt(0x5110d474, &GSRasterizer::DrawScanlineEx<0x5110d474>);
	m_dsmap.SetAt(0x5110d824, &GSRasterizer::DrawScanlineEx<0x5110d824>);
	m_dsmap.SetAt(0x5110dc04, &GSRasterizer::DrawScanlineEx<0x5110dc04>);
	m_dsmap.SetAt(0x5110dc14, &GSRasterizer::DrawScanlineEx<0x5110dc14>);
	m_dsmap.SetAt(0x5110dc54, &GSRasterizer::DrawScanlineEx<0x5110dc54>);
	m_dsmap.SetAt(0x5114b464, &GSRasterizer::DrawScanlineEx<0x5114b464>);
	m_dsmap.SetAt(0x5114b468, &GSRasterizer::DrawScanlineEx<0x5114b468>);
	m_dsmap.SetAt(0x5114d464, &GSRasterizer::DrawScanlineEx<0x5114d464>);
	m_dsmap.SetAt(0x5210c254, &GSRasterizer::DrawScanlineEx<0x5210c254>);
	m_dsmap.SetAt(0x5210d434, &GSRasterizer::DrawScanlineEx<0x5210d434>);
	m_dsmap.SetAt(0x5210d474, &GSRasterizer::DrawScanlineEx<0x5210d474>);
	m_dsmap.SetAt(0x5210dc14, &GSRasterizer::DrawScanlineEx<0x5210dc14>);
	m_dsmap.SetAt(0x5210dc34, &GSRasterizer::DrawScanlineEx<0x5210dc34>);
	m_dsmap.SetAt(0x5210dc54, &GSRasterizer::DrawScanlineEx<0x5210dc54>);
	m_dsmap.SetAt(0x5214d474, &GSRasterizer::DrawScanlineEx<0x5214d474>);
	m_dsmap.SetAt(0x60502204, &GSRasterizer::DrawScanlineEx<0x60502204>);

	// kh2

	m_dsmap.SetAt(0x00003894, &GSRasterizer::DrawScanlineEx<0x00003894>);
	m_dsmap.SetAt(0x0000dc05, &GSRasterizer::DrawScanlineEx<0x0000dc05>);
	m_dsmap.SetAt(0x4a94d464, &GSRasterizer::DrawScanlineEx<0x4a94d464>);
	m_dsmap.SetAt(0x50903c54, &GSRasterizer::DrawScanlineEx<0x50903c54>);
	m_dsmap.SetAt(0x51102814, &GSRasterizer::DrawScanlineEx<0x51102814>);
	m_dsmap.SetAt(0x51102c54, &GSRasterizer::DrawScanlineEx<0x51102c54>);
	m_dsmap.SetAt(0x52102214, &GSRasterizer::DrawScanlineEx<0x52102214>);
	m_dsmap.SetAt(0x52103c54, &GSRasterizer::DrawScanlineEx<0x52103c54>);
	m_dsmap.SetAt(0x5210c274, &GSRasterizer::DrawScanlineEx<0x5210c274>);
	m_dsmap.SetAt(0x5214d464, &GSRasterizer::DrawScanlineEx<0x5214d464>);
	m_dsmap.SetAt(0x59103805, &GSRasterizer::DrawScanlineEx<0x59103805>);
	m_dsmap.SetAt(0x5a103805, &GSRasterizer::DrawScanlineEx<0x5a103805>);
	m_dsmap.SetAt(0x00003c00, &GSRasterizer::DrawScanlineEx<0x00003c00>);
	m_dsmap.SetAt(0x4010d434, &GSRasterizer::DrawScanlineEx<0x4010d434>);
	m_dsmap.SetAt(0x4510d434, &GSRasterizer::DrawScanlineEx<0x4510d434>);
	m_dsmap.SetAt(0x51102c45, &GSRasterizer::DrawScanlineEx<0x51102c45>);
	m_dsmap.SetAt(0x5110c220, &GSRasterizer::DrawScanlineEx<0x5110c220>);
	m_dsmap.SetAt(0x5110dc00, &GSRasterizer::DrawScanlineEx<0x5110dc00>);
	m_dsmap.SetAt(0x51147464, &GSRasterizer::DrawScanlineEx<0x51147464>);
	m_dsmap.SetAt(0x6090d434, &GSRasterizer::DrawScanlineEx<0x6090d434>);

	// resident evil 4

	m_dsmap.SetAt(0x00002c24, &GSRasterizer::DrawScanlineEx<0x00002c24>);
	m_dsmap.SetAt(0x00002c84, &GSRasterizer::DrawScanlineEx<0x00002c84>);
	m_dsmap.SetAt(0x00003884, &GSRasterizer::DrawScanlineEx<0x00003884>);
	m_dsmap.SetAt(0x40002c84, &GSRasterizer::DrawScanlineEx<0x40002c84>);
	m_dsmap.SetAt(0x40003484, &GSRasterizer::DrawScanlineEx<0x40003484>);
	m_dsmap.SetAt(0x4a902c84, &GSRasterizer::DrawScanlineEx<0x4a902c84>);
	m_dsmap.SetAt(0x4a903c84, &GSRasterizer::DrawScanlineEx<0x4a903c84>);
	m_dsmap.SetAt(0x5110c204, &GSRasterizer::DrawScanlineEx<0x5110c204>);
	m_dsmap.SetAt(0x5110d404, &GSRasterizer::DrawScanlineEx<0x5110d404>);
	m_dsmap.SetAt(0x5110d444, &GSRasterizer::DrawScanlineEx<0x5110d444>);
	m_dsmap.SetAt(0x52102204, &GSRasterizer::DrawScanlineEx<0x52102204>);
	m_dsmap.SetAt(0x52103c04, &GSRasterizer::DrawScanlineEx<0x52103c04>);
	m_dsmap.SetAt(0x5210c224, &GSRasterizer::DrawScanlineEx<0x5210c224>);
	m_dsmap.SetAt(0x5210d444, &GSRasterizer::DrawScanlineEx<0x5210d444>);
	m_dsmap.SetAt(0x5210d464, &GSRasterizer::DrawScanlineEx<0x5210d464>);
	m_dsmap.SetAt(0x5211c264, &GSRasterizer::DrawScanlineEx<0x5211c264>);
	m_dsmap.SetAt(0x5610d464, &GSRasterizer::DrawScanlineEx<0x5610d464>);
	m_dsmap.SetAt(0x59103c84, &GSRasterizer::DrawScanlineEx<0x59103c84>);
	m_dsmap.SetAt(0x5910c224, &GSRasterizer::DrawScanlineEx<0x5910c224>);
	m_dsmap.SetAt(0x5910d464, &GSRasterizer::DrawScanlineEx<0x5910d464>);
	m_dsmap.SetAt(0x59120214, &GSRasterizer::DrawScanlineEx<0x59120214>);
	m_dsmap.SetAt(0x5a102224, &GSRasterizer::DrawScanlineEx<0x5a102224>);
	m_dsmap.SetAt(0x5a103c84, &GSRasterizer::DrawScanlineEx<0x5a103c84>);
	m_dsmap.SetAt(0x62502204, &GSRasterizer::DrawScanlineEx<0x62502204>);
	m_dsmap.SetAt(0x69103c84, &GSRasterizer::DrawScanlineEx<0x69103c84>);

	// dmc (fixme)

	m_dsmap.SetAt(0x0000bc18, &GSRasterizer::DrawScanlineEx<0x0000bc18>);
	m_dsmap.SetAt(0x0000d468, &GSRasterizer::DrawScanlineEx<0x0000d468>);
	m_dsmap.SetAt(0x00012218, &GSRasterizer::DrawScanlineEx<0x00012218>);
	m_dsmap.SetAt(0x0004b468, &GSRasterizer::DrawScanlineEx<0x0004b468>);
	m_dsmap.SetAt(0x0004d468, &GSRasterizer::DrawScanlineEx<0x0004d468>);
	m_dsmap.SetAt(0x0004d478, &GSRasterizer::DrawScanlineEx<0x0004d478>);
	m_dsmap.SetAt(0x4a510238, &GSRasterizer::DrawScanlineEx<0x4a510238>);
	m_dsmap.SetAt(0x4a512218, &GSRasterizer::DrawScanlineEx<0x4a512218>);
	m_dsmap.SetAt(0x51102228, &GSRasterizer::DrawScanlineEx<0x51102228>);
	m_dsmap.SetAt(0x51103828, &GSRasterizer::DrawScanlineEx<0x51103828>);
	m_dsmap.SetAt(0x5110bc28, &GSRasterizer::DrawScanlineEx<0x5110bc28>);
	m_dsmap.SetAt(0x5114d468, &GSRasterizer::DrawScanlineEx<0x5114d468>);
	m_dsmap.SetAt(0x5210bc28, &GSRasterizer::DrawScanlineEx<0x5210bc28>);
	m_dsmap.SetAt(0x58911c18, &GSRasterizer::DrawScanlineEx<0x58911c18>);
	m_dsmap.SetAt(0x5a10d468, &GSRasterizer::DrawScanlineEx<0x5a10d468>);
	m_dsmap.SetAt(0x68510238, &GSRasterizer::DrawScanlineEx<0x68510238>);
	m_dsmap.SetAt(0x68512218, &GSRasterizer::DrawScanlineEx<0x68512218>);

	// tomoyo after 

	m_dsmap.SetAt(0x0000220a, &GSRasterizer::DrawScanlineEx<0x0000220a>);
	m_dsmap.SetAt(0x00002c19, &GSRasterizer::DrawScanlineEx<0x00002c19>);
	m_dsmap.SetAt(0x00003c19, &GSRasterizer::DrawScanlineEx<0x00003c19>);
	m_dsmap.SetAt(0x50902268, &GSRasterizer::DrawScanlineEx<0x50902268>);
	m_dsmap.SetAt(0x50903c68, &GSRasterizer::DrawScanlineEx<0x50903c68>);
	m_dsmap.SetAt(0x51102c68, &GSRasterizer::DrawScanlineEx<0x51102c68>);
	m_dsmap.SetAt(0x51103c68, &GSRasterizer::DrawScanlineEx<0x51103c68>);
	m_dsmap.SetAt(0x52102c68, &GSRasterizer::DrawScanlineEx<0x52102c68>);
	m_dsmap.SetAt(0x55102c68, &GSRasterizer::DrawScanlineEx<0x55102c68>);

	// okami

	// shadow of the colossus

	m_dsmap.SetAt(0x00002210, &GSRasterizer::DrawScanlineEx<0x00002210>);
	m_dsmap.SetAt(0x00002805, &GSRasterizer::DrawScanlineEx<0x00002805>);
	m_dsmap.SetAt(0x00010214, &GSRasterizer::DrawScanlineEx<0x00010214>);
	m_dsmap.SetAt(0x00010224, &GSRasterizer::DrawScanlineEx<0x00010224>);
	m_dsmap.SetAt(0x00010254, &GSRasterizer::DrawScanlineEx<0x00010254>);
	m_dsmap.SetAt(0x00010264, &GSRasterizer::DrawScanlineEx<0x00010264>);
	m_dsmap.SetAt(0x00011c15, &GSRasterizer::DrawScanlineEx<0x00011c15>);
	m_dsmap.SetAt(0x50951464, &GSRasterizer::DrawScanlineEx<0x50951464>);
	m_dsmap.SetAt(0x51111814, &GSRasterizer::DrawScanlineEx<0x51111814>);
	m_dsmap.SetAt(0x51111c64, &GSRasterizer::DrawScanlineEx<0x51111c64>);
	m_dsmap.SetAt(0x51119464, &GSRasterizer::DrawScanlineEx<0x51119464>);
	m_dsmap.SetAt(0x5111cc64, &GSRasterizer::DrawScanlineEx<0x5111cc64>);
	m_dsmap.SetAt(0x5111dc24, &GSRasterizer::DrawScanlineEx<0x5111dc24>);
	m_dsmap.SetAt(0x5111dc64, &GSRasterizer::DrawScanlineEx<0x5111dc64>);
	m_dsmap.SetAt(0x51120214, &GSRasterizer::DrawScanlineEx<0x51120214>);
	m_dsmap.SetAt(0x51151064, &GSRasterizer::DrawScanlineEx<0x51151064>);
	m_dsmap.SetAt(0x51153464, &GSRasterizer::DrawScanlineEx<0x51153464>);
	m_dsmap.SetAt(0x51191c24, &GSRasterizer::DrawScanlineEx<0x51191c24>);
	m_dsmap.SetAt(0x51251464, &GSRasterizer::DrawScanlineEx<0x51251464>);
	m_dsmap.SetAt(0x52110214, &GSRasterizer::DrawScanlineEx<0x52110214>);
	m_dsmap.SetAt(0x52110254, &GSRasterizer::DrawScanlineEx<0x52110254>);
	m_dsmap.SetAt(0x52111c14, &GSRasterizer::DrawScanlineEx<0x52111c14>);
	m_dsmap.SetAt(0x52151464, &GSRasterizer::DrawScanlineEx<0x52151464>);
	m_dsmap.SetAt(0x55111814, &GSRasterizer::DrawScanlineEx<0x55111814>);
	m_dsmap.SetAt(0x59110214, &GSRasterizer::DrawScanlineEx<0x59110214>);
	m_dsmap.SetAt(0x5a110224, &GSRasterizer::DrawScanlineEx<0x5a110224>);

	// mgs3s1

	// 12riven

	m_dsmap.SetAt(0x51102208, &GSRasterizer::DrawScanlineEx<0x51102208>);
	m_dsmap.SetAt(0x51102c08, &GSRasterizer::DrawScanlineEx<0x51102c08>);
	m_dsmap.SetAt(0x51102c48, &GSRasterizer::DrawScanlineEx<0x51102c48>);
	m_dsmap.SetAt(0x51103c08, &GSRasterizer::DrawScanlineEx<0x51103c08>);

	// persona 3 fes

	m_dsmap.SetAt(0x00002248, &GSRasterizer::DrawScanlineEx<0x00002248>);
	m_dsmap.SetAt(0x00002268, &GSRasterizer::DrawScanlineEx<0x00002268>);
	m_dsmap.SetAt(0x0001a228, &GSRasterizer::DrawScanlineEx<0x0001a228>);
	m_dsmap.SetAt(0x4001a22b, &GSRasterizer::DrawScanlineEx<0x4001a22b>);
	m_dsmap.SetAt(0x51110448, &GSRasterizer::DrawScanlineEx<0x51110448>);
	m_dsmap.SetAt(0x51111468, &GSRasterizer::DrawScanlineEx<0x51111468>);
	m_dsmap.SetAt(0x5111a068, &GSRasterizer::DrawScanlineEx<0x5111a068>);
	m_dsmap.SetAt(0x5111a0e8, &GSRasterizer::DrawScanlineEx<0x5111a0e8>);
	m_dsmap.SetAt(0x5111a168, &GSRasterizer::DrawScanlineEx<0x5111a168>);

	// bully

	// nfs mw

	// xenosaga

	m_dsmap.SetAt(0x4000b464, &GSRasterizer::DrawScanlineEx<0x4000b464>);
	m_dsmap.SetAt(0x5a110214, &GSRasterizer::DrawScanlineEx<0x5a110214>);
	m_dsmap.SetAt(0x00010234, &GSRasterizer::DrawScanlineEx<0x00010234>);
	m_dsmap.SetAt(0x5210d424, &GSRasterizer::DrawScanlineEx<0x5210d424>);
	m_dsmap.SetAt(0x4450cc34, &GSRasterizer::DrawScanlineEx<0x4450cc34>);
	m_dsmap.SetAt(0x5810c274, &GSRasterizer::DrawScanlineEx<0x5810c274>);
	m_dsmap.SetAt(0x5090d424, &GSRasterizer::DrawScanlineEx<0x5090d424>);
	m_dsmap.SetAt(0x51103464, &GSRasterizer::DrawScanlineEx<0x51103464>);
	m_dsmap.SetAt(0x51130264, &GSRasterizer::DrawScanlineEx<0x51130264>);
	m_dsmap.SetAt(0x4010cc34, &GSRasterizer::DrawScanlineEx<0x4010cc34>);
	m_dsmap.SetAt(0x51107464, &GSRasterizer::DrawScanlineEx<0x51107464>);
	m_dsmap.SetAt(0x00002c34, &GSRasterizer::DrawScanlineEx<0x00002c34>);
	m_dsmap.SetAt(0x5a103464, &GSRasterizer::DrawScanlineEx<0x5a103464>);
	m_dsmap.SetAt(0x5a102264, &GSRasterizer::DrawScanlineEx<0x5a102264>);
	m_dsmap.SetAt(0x40003464, &GSRasterizer::DrawScanlineEx<0x40003464>);
	m_dsmap.SetAt(0x51103834, &GSRasterizer::DrawScanlineEx<0x51103834>);
	m_dsmap.SetAt(0x40002264, &GSRasterizer::DrawScanlineEx<0x40002264>);
	m_dsmap.SetAt(0x59108434, &GSRasterizer::DrawScanlineEx<0x59108434>);
	m_dsmap.SetAt(0x40010234, &GSRasterizer::DrawScanlineEx<0x40010234>);

	// mana khemia

	// Gundam Seed Destiny OMNI VS ZAFT II PLUS 

	m_dsmap.SetAt(0x00012205, &GSRasterizer::DrawScanlineEx<0x00012205>);
	m_dsmap.SetAt(0x00012215, &GSRasterizer::DrawScanlineEx<0x00012215>);
	m_dsmap.SetAt(0x0001b475, &GSRasterizer::DrawScanlineEx<0x0001b475>);
	m_dsmap.SetAt(0x0005b475, &GSRasterizer::DrawScanlineEx<0x0005b475>);
	m_dsmap.SetAt(0x51102c05, &GSRasterizer::DrawScanlineEx<0x51102c05>);
	m_dsmap.SetAt(0x51103445, &GSRasterizer::DrawScanlineEx<0x51103445>);
	m_dsmap.SetAt(0x51103c05, &GSRasterizer::DrawScanlineEx<0x51103c05>);
	m_dsmap.SetAt(0x5115b475, &GSRasterizer::DrawScanlineEx<0x5115b475>);
	m_dsmap.SetAt(0x52103445, &GSRasterizer::DrawScanlineEx<0x52103445>);
	m_dsmap.SetAt(0x5215b475, &GSRasterizer::DrawScanlineEx<0x5215b475>);

	// wild arms 5

	// rouge galaxy

	// tokyo bus guide

	m_dsmap.SetAt(0x0000e230, &GSRasterizer::DrawScanlineEx<0x0000e230>);
	m_dsmap.SetAt(0x0000e270, &GSRasterizer::DrawScanlineEx<0x0000e270>);
	m_dsmap.SetAt(0x0004d470, &GSRasterizer::DrawScanlineEx<0x0004d470>);
	m_dsmap.SetAt(0x0004f470, &GSRasterizer::DrawScanlineEx<0x0004f470>);
	m_dsmap.SetAt(0x5110d470, &GSRasterizer::DrawScanlineEx<0x5110d470>);
	m_dsmap.SetAt(0x5110e230, &GSRasterizer::DrawScanlineEx<0x5110e230>);
	m_dsmap.SetAt(0x5110e270, &GSRasterizer::DrawScanlineEx<0x5110e270>);
	m_dsmap.SetAt(0x5110f430, &GSRasterizer::DrawScanlineEx<0x5110f430>);
	m_dsmap.SetAt(0x5110fc30, &GSRasterizer::DrawScanlineEx<0x5110fc30>);
	m_dsmap.SetAt(0x51111450, &GSRasterizer::DrawScanlineEx<0x51111450>);
	m_dsmap.SetAt(0x51111470, &GSRasterizer::DrawScanlineEx<0x51111470>);
	m_dsmap.SetAt(0x51120210, &GSRasterizer::DrawScanlineEx<0x51120210>);
	m_dsmap.SetAt(0x5114f470, &GSRasterizer::DrawScanlineEx<0x5114f470>);
	m_dsmap.SetAt(0x51153450, &GSRasterizer::DrawScanlineEx<0x51153450>);
	m_dsmap.SetAt(0x59103c20, &GSRasterizer::DrawScanlineEx<0x59103c20>);

	// the punisher

	// sfex3

	m_dsmap.SetAt(0x00002200, &GSRasterizer::DrawScanlineEx<0x00002200>);
	m_dsmap.SetAt(0x00002258, &GSRasterizer::DrawScanlineEx<0x00002258>);
	m_dsmap.SetAt(0x0000280a, &GSRasterizer::DrawScanlineEx<0x0000280a>);
	m_dsmap.SetAt(0x0000380a, &GSRasterizer::DrawScanlineEx<0x0000380a>);
	m_dsmap.SetAt(0x0000c278, &GSRasterizer::DrawScanlineEx<0x0000c278>);
	m_dsmap.SetAt(0x0000c5e8, &GSRasterizer::DrawScanlineEx<0x0000c5e8>);
	m_dsmap.SetAt(0x5098e248, &GSRasterizer::DrawScanlineEx<0x5098e248>);
	m_dsmap.SetAt(0x5098e5c8, &GSRasterizer::DrawScanlineEx<0x5098e5c8>);
	m_dsmap.SetAt(0x51102218, &GSRasterizer::DrawScanlineEx<0x51102218>);
	m_dsmap.SetAt(0x5110280a, &GSRasterizer::DrawScanlineEx<0x5110280a>);
	m_dsmap.SetAt(0x51102c18, &GSRasterizer::DrawScanlineEx<0x51102c18>);
	m_dsmap.SetAt(0x51102cb8, &GSRasterizer::DrawScanlineEx<0x51102cb8>);
	m_dsmap.SetAt(0x51103468, &GSRasterizer::DrawScanlineEx<0x51103468>);
	m_dsmap.SetAt(0x51103c18, &GSRasterizer::DrawScanlineEx<0x51103c18>);
	m_dsmap.SetAt(0x5110c218, &GSRasterizer::DrawScanlineEx<0x5110c218>);
	m_dsmap.SetAt(0x5110c5e8, &GSRasterizer::DrawScanlineEx<0x5110c5e8>);
	m_dsmap.SetAt(0x5110cc18, &GSRasterizer::DrawScanlineEx<0x5110cc18>);
	m_dsmap.SetAt(0x5110d5f8, &GSRasterizer::DrawScanlineEx<0x5110d5f8>);
	m_dsmap.SetAt(0x5110dc18, &GSRasterizer::DrawScanlineEx<0x5110dc18>);
	m_dsmap.SetAt(0x52102278, &GSRasterizer::DrawScanlineEx<0x52102278>);
	m_dsmap.SetAt(0x52103438, &GSRasterizer::DrawScanlineEx<0x52103438>);
	m_dsmap.SetAt(0x5210d478, &GSRasterizer::DrawScanlineEx<0x5210d478>);
	m_dsmap.SetAt(0x5210dc38, &GSRasterizer::DrawScanlineEx<0x5210dc38>);
	m_dsmap.SetAt(0x5210e278, &GSRasterizer::DrawScanlineEx<0x5210e278>);
	m_dsmap.SetAt(0x5210fc38, &GSRasterizer::DrawScanlineEx<0x5210fc38>);
	m_dsmap.SetAt(0x59102258, &GSRasterizer::DrawScanlineEx<0x59102258>);
	m_dsmap.SetAt(0x5a902218, &GSRasterizer::DrawScanlineEx<0x5a902218>);
	m_dsmap.SetAt(0x6a50c238, &GSRasterizer::DrawScanlineEx<0x6a50c238>);

	// ico

	// ffx-2

	m_dsmap.SetAt(0x40002805, &GSRasterizer::DrawScanlineEx<0x40002805>);
	m_dsmap.SetAt(0x5110ec25, &GSRasterizer::DrawScanlineEx<0x5110ec25>);
	m_dsmap.SetAt(0x5110c245, &GSRasterizer::DrawScanlineEx<0x5110c245>);
	m_dsmap.SetAt(0x4000c245, &GSRasterizer::DrawScanlineEx<0x4000c245>);
	m_dsmap.SetAt(0x59102215, &GSRasterizer::DrawScanlineEx<0x59102215>);
	m_dsmap.SetAt(0x5110e225, &GSRasterizer::DrawScanlineEx<0x5110e225>);
	m_dsmap.SetAt(0x5110ec55, &GSRasterizer::DrawScanlineEx<0x5110ec55>);
	m_dsmap.SetAt(0x5110c445, &GSRasterizer::DrawScanlineEx<0x5110c445>);
	m_dsmap.SetAt(0x5210dc05, &GSRasterizer::DrawScanlineEx<0x5210dc05>);
	m_dsmap.SetAt(0x5110d805, &GSRasterizer::DrawScanlineEx<0x5110d805>);
	m_dsmap.SetAt(0x6210d065, &GSRasterizer::DrawScanlineEx<0x6210d065>);
	m_dsmap.SetAt(0x5110d065, &GSRasterizer::DrawScanlineEx<0x5110d065>);
	m_dsmap.SetAt(0x5090d425, &GSRasterizer::DrawScanlineEx<0x5090d425>);
	m_dsmap.SetAt(0x51110215, &GSRasterizer::DrawScanlineEx<0x51110215>);
	m_dsmap.SetAt(0x5110d815, &GSRasterizer::DrawScanlineEx<0x5110d815>);
	m_dsmap.SetAt(0x51120215, &GSRasterizer::DrawScanlineEx<0x51120215>);
	m_dsmap.SetAt(0x5110cc05, &GSRasterizer::DrawScanlineEx<0x5110cc05>);
	m_dsmap.SetAt(0x4000cc05, &GSRasterizer::DrawScanlineEx<0x4000cc05>);
	m_dsmap.SetAt(0x5210dc25, &GSRasterizer::DrawScanlineEx<0x5210dc25>);
	m_dsmap.SetAt(0x5210ec15, &GSRasterizer::DrawScanlineEx<0x5210ec15>);
	m_dsmap.SetAt(0x5110fc75, &GSRasterizer::DrawScanlineEx<0x5110fc75>);
	m_dsmap.SetAt(0x5210c215, &GSRasterizer::DrawScanlineEx<0x5210c215>);
	m_dsmap.SetAt(0x5214d465, &GSRasterizer::DrawScanlineEx<0x5214d465>);
	m_dsmap.SetAt(0x51102215, &GSRasterizer::DrawScanlineEx<0x51102215>);
	m_dsmap.SetAt(0x40002c45, &GSRasterizer::DrawScanlineEx<0x40002c45>);
	m_dsmap.SetAt(0x6210d465, &GSRasterizer::DrawScanlineEx<0x6210d465>);
	m_dsmap.SetAt(0x59102815, &GSRasterizer::DrawScanlineEx<0x59102815>);
	m_dsmap.SetAt(0x5210fc15, &GSRasterizer::DrawScanlineEx<0x5210fc15>);
	m_dsmap.SetAt(0x5210c225, &GSRasterizer::DrawScanlineEx<0x5210c225>);
	m_dsmap.SetAt(0x5090d465, &GSRasterizer::DrawScanlineEx<0x5090d465>);
	m_dsmap.SetAt(0x5110dc25, &GSRasterizer::DrawScanlineEx<0x5110dc25>);
	m_dsmap.SetAt(0x5110ec15, &GSRasterizer::DrawScanlineEx<0x5110ec15>);
	m_dsmap.SetAt(0x4000cc45, &GSRasterizer::DrawScanlineEx<0x4000cc45>);
	m_dsmap.SetAt(0x51102805, &GSRasterizer::DrawScanlineEx<0x51102805>);
	m_dsmap.SetAt(0x40102806, &GSRasterizer::DrawScanlineEx<0x40102806>);
	m_dsmap.SetAt(0x0000b464, &GSRasterizer::DrawScanlineEx<0x0000b464>);
	m_dsmap.SetAt(0x0004a464, &GSRasterizer::DrawScanlineEx<0x0004a464>);
	m_dsmap.SetAt(0x51103054, &GSRasterizer::DrawScanlineEx<0x51103054>);
	m_dsmap.SetAt(0x51103474, &GSRasterizer::DrawScanlineEx<0x51103474>);
	m_dsmap.SetAt(0x5111b464, &GSRasterizer::DrawScanlineEx<0x5111b464>);
	m_dsmap.SetAt(0x51131454, &GSRasterizer::DrawScanlineEx<0x51131454>);
	m_dsmap.SetAt(0x51143064, &GSRasterizer::DrawScanlineEx<0x51143064>);
	m_dsmap.SetAt(0x51151464, &GSRasterizer::DrawScanlineEx<0x51151464>);
	m_dsmap.SetAt(0x5115a264, &GSRasterizer::DrawScanlineEx<0x5115a264>);
	m_dsmap.SetAt(0x5115a464, &GSRasterizer::DrawScanlineEx<0x5115a464>);
	m_dsmap.SetAt(0x52103454, &GSRasterizer::DrawScanlineEx<0x52103454>);
	m_dsmap.SetAt(0x5211b464, &GSRasterizer::DrawScanlineEx<0x5211b464>);

	// God of War

	// dbzbt2

	// dbzbt3

	m_dsmap.SetAt(0x40002c86, &GSRasterizer::DrawScanlineEx<0x40002c86>);
	m_dsmap.SetAt(0x51102204, &GSRasterizer::DrawScanlineEx<0x51102204>);
	m_dsmap.SetAt(0x51102884, &GSRasterizer::DrawScanlineEx<0x51102884>);
	m_dsmap.SetAt(0x51102c84, &GSRasterizer::DrawScanlineEx<0x51102c84>);
	m_dsmap.SetAt(0x51102c86, &GSRasterizer::DrawScanlineEx<0x51102c86>);
	m_dsmap.SetAt(0x51102cb4, &GSRasterizer::DrawScanlineEx<0x51102cb4>);
	m_dsmap.SetAt(0x51103804, &GSRasterizer::DrawScanlineEx<0x51103804>);
	m_dsmap.SetAt(0x51103884, &GSRasterizer::DrawScanlineEx<0x51103884>);
	m_dsmap.SetAt(0x51103c04, &GSRasterizer::DrawScanlineEx<0x51103c04>);
	m_dsmap.SetAt(0x5110b464, &GSRasterizer::DrawScanlineEx<0x5110b464>);
	m_dsmap.SetAt(0x5110fc84, &GSRasterizer::DrawScanlineEx<0x5110fc84>);
	m_dsmap.SetAt(0x51183464, &GSRasterizer::DrawScanlineEx<0x51183464>);
	m_dsmap.SetAt(0x5118e464, &GSRasterizer::DrawScanlineEx<0x5118e464>);
	m_dsmap.SetAt(0x52102264, &GSRasterizer::DrawScanlineEx<0x52102264>);
	m_dsmap.SetAt(0x52102c84, &GSRasterizer::DrawScanlineEx<0x52102c84>);
	m_dsmap.SetAt(0x54902204, &GSRasterizer::DrawScanlineEx<0x54902204>);
	m_dsmap.SetAt(0x55103804, &GSRasterizer::DrawScanlineEx<0x55103804>);
	m_dsmap.SetAt(0x561034e4, &GSRasterizer::DrawScanlineEx<0x561034e4>);
	m_dsmap.SetAt(0x58902c86, &GSRasterizer::DrawScanlineEx<0x58902c86>);
	m_dsmap.SetAt(0x589034e4, &GSRasterizer::DrawScanlineEx<0x589034e4>);
	m_dsmap.SetAt(0x59102204, &GSRasterizer::DrawScanlineEx<0x59102204>);
	m_dsmap.SetAt(0x59102206, &GSRasterizer::DrawScanlineEx<0x59102206>);
	m_dsmap.SetAt(0x59102244, &GSRasterizer::DrawScanlineEx<0x59102244>);
	m_dsmap.SetAt(0x59103464, &GSRasterizer::DrawScanlineEx<0x59103464>);
	m_dsmap.SetAt(0x59103804, &GSRasterizer::DrawScanlineEx<0x59103804>);
	m_dsmap.SetAt(0x5910fc84, &GSRasterizer::DrawScanlineEx<0x5910fc84>);
	m_dsmap.SetAt(0x5a103804, &GSRasterizer::DrawScanlineEx<0x5a103804>);

	// suikoden 5

	// disgaea 2

	m_dsmap.SetAt(0x0000c224, &GSRasterizer::DrawScanlineEx<0x0000c224>);
	m_dsmap.SetAt(0x4a90d474, &GSRasterizer::DrawScanlineEx<0x4a90d474>);
	m_dsmap.SetAt(0x51102224, &GSRasterizer::DrawScanlineEx<0x51102224>);
	m_dsmap.SetAt(0x5110c224, &GSRasterizer::DrawScanlineEx<0x5110c224>);
	m_dsmap.SetAt(0x5110d464, &GSRasterizer::DrawScanlineEx<0x5110d464>);
	m_dsmap.SetAt(0x5110dc24, &GSRasterizer::DrawScanlineEx<0x5110dc24>);
	m_dsmap.SetAt(0x5110dc64, &GSRasterizer::DrawScanlineEx<0x5110dc64>);
	m_dsmap.SetAt(0x5111d064, &GSRasterizer::DrawScanlineEx<0x5111d064>);
	m_dsmap.SetAt(0x5111d464, &GSRasterizer::DrawScanlineEx<0x5111d464>);
	m_dsmap.SetAt(0x5890d464, &GSRasterizer::DrawScanlineEx<0x5890d464>);
	m_dsmap.SetAt(0x6a102224, &GSRasterizer::DrawScanlineEx<0x6a102224>);
	m_dsmap.SetAt(0x6a10c224, &GSRasterizer::DrawScanlineEx<0x6a10c224>);

	// dq8

	// xenosaga 2

	m_dsmap.SetAt(0x5a102214, &GSRasterizer::DrawScanlineEx<0x5a102214>);
	m_dsmap.SetAt(0x52102c14, &GSRasterizer::DrawScanlineEx<0x52102c14>);
	m_dsmap.SetAt(0x00011c04, &GSRasterizer::DrawScanlineEx<0x00011c04>);
	m_dsmap.SetAt(0x00042244, &GSRasterizer::DrawScanlineEx<0x00042244>);
	m_dsmap.SetAt(0x52103464, &GSRasterizer::DrawScanlineEx<0x52103464>);
	m_dsmap.SetAt(0x5510d424, &GSRasterizer::DrawScanlineEx<0x5510d424>);
	m_dsmap.SetAt(0x59103c14, &GSRasterizer::DrawScanlineEx<0x59103c14>);
	m_dsmap.SetAt(0x52103c14, &GSRasterizer::DrawScanlineEx<0x52103c14>);
	m_dsmap.SetAt(0x00010204, &GSRasterizer::DrawScanlineEx<0x00010204>);
	m_dsmap.SetAt(0x00042264, &GSRasterizer::DrawScanlineEx<0x00042264>);
	m_dsmap.SetAt(0x5490d424, &GSRasterizer::DrawScanlineEx<0x5490d424>);
	m_dsmap.SetAt(0x00003464, &GSRasterizer::DrawScanlineEx<0x00003464>);
	m_dsmap.SetAt(0x59102214, &GSRasterizer::DrawScanlineEx<0x59102214>);
	m_dsmap.SetAt(0x5110d424, &GSRasterizer::DrawScanlineEx<0x5110d424>);
	m_dsmap.SetAt(0x00043444, &GSRasterizer::DrawScanlineEx<0x00043444>);
	m_dsmap.SetAt(0x62503464, &GSRasterizer::DrawScanlineEx<0x62503464>);
	m_dsmap.SetAt(0x51102c14, &GSRasterizer::DrawScanlineEx<0x51102c14>);
	m_dsmap.SetAt(0x40003c04, &GSRasterizer::DrawScanlineEx<0x40003c04>);
	m_dsmap.SetAt(0x52913404, &GSRasterizer::DrawScanlineEx<0x52913404>);
	m_dsmap.SetAt(0x51103c14, &GSRasterizer::DrawScanlineEx<0x51103c14>);
	m_dsmap.SetAt(0x00043464, &GSRasterizer::DrawScanlineEx<0x00043464>);
	m_dsmap.SetAt(0x40002204, &GSRasterizer::DrawScanlineEx<0x40002204>);
	m_dsmap.SetAt(0x51143464, &GSRasterizer::DrawScanlineEx<0x51143464>);
	m_dsmap.SetAt(0x51105464, &GSRasterizer::DrawScanlineEx<0x51105464>);
	m_dsmap.SetAt(0x5a103c04, &GSRasterizer::DrawScanlineEx<0x5a103c04>);
	m_dsmap.SetAt(0x5610d424, &GSRasterizer::DrawScanlineEx<0x5610d424>);
	m_dsmap.SetAt(0x51102234, &GSRasterizer::DrawScanlineEx<0x51102234>);
	m_dsmap.SetAt(0x40090204, &GSRasterizer::DrawScanlineEx<0x40090204>);
	m_dsmap.SetAt(0x51145464, &GSRasterizer::DrawScanlineEx<0x51145464>);
	m_dsmap.SetAt(0x4010dc34, &GSRasterizer::DrawScanlineEx<0x4010dc34>);

	// persona 4

	m_dsmap.SetAt(0x5a143468, &GSRasterizer::DrawScanlineEx<0x5a143468>);
	m_dsmap.SetAt(0x52142268, &GSRasterizer::DrawScanlineEx<0x52142268>);
	m_dsmap.SetAt(0x51102268, &GSRasterizer::DrawScanlineEx<0x51102268>);
	m_dsmap.SetAt(0x52151468, &GSRasterizer::DrawScanlineEx<0x52151468>);
	m_dsmap.SetAt(0x5111a448, &GSRasterizer::DrawScanlineEx<0x5111a448>);
	m_dsmap.SetAt(0x5111a268, &GSRasterizer::DrawScanlineEx<0x5111a268>);
	m_dsmap.SetAt(0x00042268, &GSRasterizer::DrawScanlineEx<0x00042268>);
	m_dsmap.SetAt(0x5111a468, &GSRasterizer::DrawScanlineEx<0x5111a468>);
	m_dsmap.SetAt(0x51142268, &GSRasterizer::DrawScanlineEx<0x51142268>);
	m_dsmap.SetAt(0x00002208, &GSRasterizer::DrawScanlineEx<0x00002208>);
	m_dsmap.SetAt(0x52111048, &GSRasterizer::DrawScanlineEx<0x52111048>);
	m_dsmap.SetAt(0x52110248, &GSRasterizer::DrawScanlineEx<0x52110248>);
	m_dsmap.SetAt(0x40082248, &GSRasterizer::DrawScanlineEx<0x40082248>);
	m_dsmap.SetAt(0x5111b468, &GSRasterizer::DrawScanlineEx<0x5111b468>);
	m_dsmap.SetAt(0x00002218, &GSRasterizer::DrawScanlineEx<0x00002218>);
	m_dsmap.SetAt(0x5a142068, &GSRasterizer::DrawScanlineEx<0x5a142068>);
	m_dsmap.SetAt(0x50951468, &GSRasterizer::DrawScanlineEx<0x50951468>);
	m_dsmap.SetAt(0x51143468, &GSRasterizer::DrawScanlineEx<0x51143468>);
	m_dsmap.SetAt(0x5110d468, &GSRasterizer::DrawScanlineEx<0x5110d468>);
	m_dsmap.SetAt(0x5115b468, &GSRasterizer::DrawScanlineEx<0x5115b468>);
	m_dsmap.SetAt(0x5111d468, &GSRasterizer::DrawScanlineEx<0x5111d468>);
	m_dsmap.SetAt(0x52102268, &GSRasterizer::DrawScanlineEx<0x52102268>);
	m_dsmap.SetAt(0x5117b468, &GSRasterizer::DrawScanlineEx<0x5117b468>);
	m_dsmap.SetAt(0x50910248, &GSRasterizer::DrawScanlineEx<0x50910248>);
	m_dsmap.SetAt(0x0001a248, &GSRasterizer::DrawScanlineEx<0x0001a248>);
	m_dsmap.SetAt(0x52111468, &GSRasterizer::DrawScanlineEx<0x52111468>);
	m_dsmap.SetAt(0x51102248, &GSRasterizer::DrawScanlineEx<0x51102248>);

	// nba 2k8

	m_dsmap.SetAt(0x40102256, &GSRasterizer::DrawScanlineEx<0x40102256>);
	m_dsmap.SetAt(0x51103446, &GSRasterizer::DrawScanlineEx<0x51103446>);
	m_dsmap.SetAt(0x5110a466, &GSRasterizer::DrawScanlineEx<0x5110a466>);
	m_dsmap.SetAt(0x5110b466, &GSRasterizer::DrawScanlineEx<0x5110b466>);
	m_dsmap.SetAt(0x5110c466, &GSRasterizer::DrawScanlineEx<0x5110c466>);
	m_dsmap.SetAt(0x5110d466, &GSRasterizer::DrawScanlineEx<0x5110d466>);
	m_dsmap.SetAt(0x5118d466, &GSRasterizer::DrawScanlineEx<0x5118d466>);
	m_dsmap.SetAt(0x5210b066, &GSRasterizer::DrawScanlineEx<0x5210b066>);
	m_dsmap.SetAt(0x5210b466, &GSRasterizer::DrawScanlineEx<0x5210b466>);
	m_dsmap.SetAt(0x6a10a466, &GSRasterizer::DrawScanlineEx<0x6a10a466>);
	m_dsmap.SetAt(0x6a10b066, &GSRasterizer::DrawScanlineEx<0x6a10b066>);
	m_dsmap.SetAt(0x6a10b456, &GSRasterizer::DrawScanlineEx<0x6a10b456>);
	m_dsmap.SetAt(0x6a10b466, &GSRasterizer::DrawScanlineEx<0x6a10b466>);
	m_dsmap.SetAt(0x51102220, &GSRasterizer::DrawScanlineEx<0x51102220>);

}

template<DWORD sel>
void GSRasterizer::DrawScanlineEx(int top, int left, int right, const Vertex& v)
{
	const DWORD fpsm = (sel >> 0) & 3;
	const DWORD zpsm = (sel >> 2) & 3;
	const DWORD ztst = (sel >> 4) & 3;
	const DWORD iip = (sel >> 6) & 1;
	const DWORD tfx = (sel >> 7) & 7;
	const DWORD tcc = (sel >> 10) & 1;
	const DWORD fst = (sel >> 11) & 1;
	const DWORD ltf = (sel >> 12) & 1;
	const DWORD atst = (sel >> 13) & 7;
	const DWORD afail = (sel >> 16) & 3;
	const DWORD fge = (sel >> 18) & 1;
	const DWORD date = (sel >> 19) & 1;
	const DWORD abe = (sel >> 20) & 3;
	const DWORD abea = (sel >> 22) & 3;
	const DWORD abeb = (sel >> 24) & 3;
	const DWORD abec = (sel >> 26) & 3;
	const DWORD abed = (sel >> 28) & 3;
	const DWORD rfb = (sel >> 30) & 1;

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

		GSVector4i fa = fa_base + GSVector4i::load<false>(fa_offset);
		GSVector4i za = za_base + GSVector4i::load<false>(za_offset);
		
		GSVector4i fm = m_slenv.fm;
		GSVector4i zm = m_slenv.zm;
		GSVector4i test = GSVector4i::zero();

		GSVector4i zs = (GSVector4i(z * 0.5f) << 1) | (GSVector4i(z) & GSVector4i::one());

		if(ztst > 1)
		{
			GSVector4i zd = m_state->m_mem.ReadZBufX(zpsm, za);

			GSVector4i zso = zs;
			GSVector4i zdo = zd;

			if(zpsm == 0)
			{
				zso = zs - GSVector4i::x80000000();
				zdo = zd - GSVector4i::x80000000();
			}

			switch(ztst)
			{
			case 2: test = zso < zdo; break; // ge
			case 3: test = zso <= zdo; break; // g
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
				GSVector4i uv1 = Wrap(uv + GSVector4i::x0001());

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
				fm |= t & GSVector4i::xff000000();
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
