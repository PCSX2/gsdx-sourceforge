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

	m_dsmap.SetAt(0x0ff02244, &GSRasterizer::DrawScanlineEx<0x0ff02244>);
	m_dsmap.SetAt(0x0ff03404, &GSRasterizer::DrawScanlineEx<0x0ff03404>);
	m_dsmap.SetAt(0x0ff03c04, &GSRasterizer::DrawScanlineEx<0x0ff03c04>);
	m_dsmap.SetAt(0x0ff03c24, &GSRasterizer::DrawScanlineEx<0x0ff03c24>);
	m_dsmap.SetAt(0x24402204, &GSRasterizer::DrawScanlineEx<0x24402204>);
	m_dsmap.SetAt(0x24403c24, &GSRasterizer::DrawScanlineEx<0x24403c24>);
	m_dsmap.SetAt(0x24803404, &GSRasterizer::DrawScanlineEx<0x24803404>);
	m_dsmap.SetAt(0x24803444, &GSRasterizer::DrawScanlineEx<0x24803444>);
	m_dsmap.SetAt(0x25803404, &GSRasterizer::DrawScanlineEx<0x25803404>);
	m_dsmap.SetAt(0x26202204, &GSRasterizer::DrawScanlineEx<0x26202204>);
	m_dsmap.SetAt(0x26403c04, &GSRasterizer::DrawScanlineEx<0x26403c04>);
	m_dsmap.SetAt(0x26803464, &GSRasterizer::DrawScanlineEx<0x26803464>);
	m_dsmap.SetAt(0x4ff02210, &GSRasterizer::DrawScanlineEx<0x4ff02210>);
	m_dsmap.SetAt(0x4ff02230, &GSRasterizer::DrawScanlineEx<0x4ff02230>);
	m_dsmap.SetAt(0x4ff02250, &GSRasterizer::DrawScanlineEx<0x4ff02250>);
	m_dsmap.SetAt(0x4ff03020, &GSRasterizer::DrawScanlineEx<0x4ff03020>);
	m_dsmap.SetAt(0x4ff03060, &GSRasterizer::DrawScanlineEx<0x4ff03060>);
	m_dsmap.SetAt(0x4ff03464, &GSRasterizer::DrawScanlineEx<0x4ff03464>);
	m_dsmap.SetAt(0x4ff03470, &GSRasterizer::DrawScanlineEx<0x4ff03470>);
	m_dsmap.SetAt(0x4ff03c10, &GSRasterizer::DrawScanlineEx<0x4ff03c10>);
	m_dsmap.SetAt(0x4ff03c14, &GSRasterizer::DrawScanlineEx<0x4ff03c14>);
	m_dsmap.SetAt(0x4ff03c20, &GSRasterizer::DrawScanlineEx<0x4ff03c20>);
	m_dsmap.SetAt(0x4ff03c30, &GSRasterizer::DrawScanlineEx<0x4ff03c30>);
	m_dsmap.SetAt(0x60002060, &GSRasterizer::DrawScanlineEx<0x60002060>);
	m_dsmap.SetAt(0x64203410, &GSRasterizer::DrawScanlineEx<0x64203410>);
	m_dsmap.SetAt(0x64203420, &GSRasterizer::DrawScanlineEx<0x64203420>);
	m_dsmap.SetAt(0x64402210, &GSRasterizer::DrawScanlineEx<0x64402210>);
	m_dsmap.SetAt(0x64402260, &GSRasterizer::DrawScanlineEx<0x64402260>);
	m_dsmap.SetAt(0x64403c10, &GSRasterizer::DrawScanlineEx<0x64403c10>);
	m_dsmap.SetAt(0x6440dc10, &GSRasterizer::DrawScanlineEx<0x6440dc10>);
	m_dsmap.SetAt(0x64802210, &GSRasterizer::DrawScanlineEx<0x64802210>);
	m_dsmap.SetAt(0x64803420, &GSRasterizer::DrawScanlineEx<0x64803420>);
	m_dsmap.SetAt(0x64803c10, &GSRasterizer::DrawScanlineEx<0x64803c10>);
	m_dsmap.SetAt(0x66202c10, &GSRasterizer::DrawScanlineEx<0x66202c10>);
	m_dsmap.SetAt(0x66402210, &GSRasterizer::DrawScanlineEx<0x66402210>);
	m_dsmap.SetAt(0x66402250, &GSRasterizer::DrawScanlineEx<0x66402250>);
	m_dsmap.SetAt(0x66802c10, &GSRasterizer::DrawScanlineEx<0x66802c10>);

	// ffx

	m_dsmap.SetAt(0x0ff0d806, &GSRasterizer::DrawScanlineEx<0x0ff0d806>);
	m_dsmap.SetAt(0x0ff0dc05, &GSRasterizer::DrawScanlineEx<0x0ff0dc05>);
	m_dsmap.SetAt(0x2420c265, &GSRasterizer::DrawScanlineEx<0x2420c265>);
	m_dsmap.SetAt(0x2420d425, &GSRasterizer::DrawScanlineEx<0x2420d425>);
	m_dsmap.SetAt(0x2420d465, &GSRasterizer::DrawScanlineEx<0x2420d465>);
	m_dsmap.SetAt(0x2420dc05, &GSRasterizer::DrawScanlineEx<0x2420dc05>);
	m_dsmap.SetAt(0x2420e245, &GSRasterizer::DrawScanlineEx<0x2420e245>);
	m_dsmap.SetAt(0x24402205, &GSRasterizer::DrawScanlineEx<0x24402205>);
	m_dsmap.SetAt(0x24402245, &GSRasterizer::DrawScanlineEx<0x24402245>);
	m_dsmap.SetAt(0x24402275, &GSRasterizer::DrawScanlineEx<0x24402275>);
	m_dsmap.SetAt(0x2440c205, &GSRasterizer::DrawScanlineEx<0x2440c205>);
	m_dsmap.SetAt(0x2440c245, &GSRasterizer::DrawScanlineEx<0x2440c245>);
	m_dsmap.SetAt(0x2440c265, &GSRasterizer::DrawScanlineEx<0x2440c265>);
	m_dsmap.SetAt(0x2440d065, &GSRasterizer::DrawScanlineEx<0x2440d065>);
	m_dsmap.SetAt(0x2440d405, &GSRasterizer::DrawScanlineEx<0x2440d405>);
	m_dsmap.SetAt(0x2440d425, &GSRasterizer::DrawScanlineEx<0x2440d425>);
	m_dsmap.SetAt(0x2440d465, &GSRasterizer::DrawScanlineEx<0x2440d465>);
	m_dsmap.SetAt(0x2440dc05, &GSRasterizer::DrawScanlineEx<0x2440dc05>);
	m_dsmap.SetAt(0x2440fc05, &GSRasterizer::DrawScanlineEx<0x2440fc05>);
	m_dsmap.SetAt(0x2440fc45, &GSRasterizer::DrawScanlineEx<0x2440fc45>);
	m_dsmap.SetAt(0x24411045, &GSRasterizer::DrawScanlineEx<0x24411045>);
	m_dsmap.SetAt(0x24411075, &GSRasterizer::DrawScanlineEx<0x24411075>);
	m_dsmap.SetAt(0x2444d465, &GSRasterizer::DrawScanlineEx<0x2444d465>);
	m_dsmap.SetAt(0x24610265, &GSRasterizer::DrawScanlineEx<0x24610265>);
	m_dsmap.SetAt(0x24802245, &GSRasterizer::DrawScanlineEx<0x24802245>);
	m_dsmap.SetAt(0x24802275, &GSRasterizer::DrawScanlineEx<0x24802275>);
	m_dsmap.SetAt(0x2480c225, &GSRasterizer::DrawScanlineEx<0x2480c225>);
	m_dsmap.SetAt(0x2480c265, &GSRasterizer::DrawScanlineEx<0x2480c265>);
	m_dsmap.SetAt(0x2480d405, &GSRasterizer::DrawScanlineEx<0x2480d405>);
	m_dsmap.SetAt(0x2480d425, &GSRasterizer::DrawScanlineEx<0x2480d425>);
	m_dsmap.SetAt(0x2480d465, &GSRasterizer::DrawScanlineEx<0x2480d465>);
	m_dsmap.SetAt(0x2480dc05, &GSRasterizer::DrawScanlineEx<0x2480dc05>);
	m_dsmap.SetAt(0x2480dc25, &GSRasterizer::DrawScanlineEx<0x2480dc25>);
	m_dsmap.SetAt(0x2480e245, &GSRasterizer::DrawScanlineEx<0x2480e245>);
	m_dsmap.SetAt(0x24810265, &GSRasterizer::DrawScanlineEx<0x24810265>);
	m_dsmap.SetAt(0x26402275, &GSRasterizer::DrawScanlineEx<0x26402275>);
	m_dsmap.SetAt(0x26803435, &GSRasterizer::DrawScanlineEx<0x26803435>);
	m_dsmap.SetAt(0x2680e425, &GSRasterizer::DrawScanlineEx<0x2680e425>);
	m_dsmap.SetAt(0x2880c205, &GSRasterizer::DrawScanlineEx<0x2880c205>);
	m_dsmap.SetAt(0x4ff02215, &GSRasterizer::DrawScanlineEx<0x4ff02215>);
	m_dsmap.SetAt(0x4ff02896, &GSRasterizer::DrawScanlineEx<0x4ff02896>);
	m_dsmap.SetAt(0x4ff03435, &GSRasterizer::DrawScanlineEx<0x4ff03435>);
	m_dsmap.SetAt(0x4ff0d415, &GSRasterizer::DrawScanlineEx<0x4ff0d415>);
	m_dsmap.SetAt(0x4ff0d475, &GSRasterizer::DrawScanlineEx<0x4ff0d475>);
	m_dsmap.SetAt(0x4ff0fc15, &GSRasterizer::DrawScanlineEx<0x4ff0fc15>);
	m_dsmap.SetAt(0x4ff4c275, &GSRasterizer::DrawScanlineEx<0x4ff4c275>);
	m_dsmap.SetAt(0x4ff4d475, &GSRasterizer::DrawScanlineEx<0x4ff4d475>);
	m_dsmap.SetAt(0x64402215, &GSRasterizer::DrawScanlineEx<0x64402215>);
	m_dsmap.SetAt(0x64402255, &GSRasterizer::DrawScanlineEx<0x64402255>);
	m_dsmap.SetAt(0x64402815, &GSRasterizer::DrawScanlineEx<0x64402815>);
	m_dsmap.SetAt(0x6440d465, &GSRasterizer::DrawScanlineEx<0x6440d465>);
	m_dsmap.SetAt(0x6440e215, &GSRasterizer::DrawScanlineEx<0x6440e215>);
	m_dsmap.SetAt(0x6440e255, &GSRasterizer::DrawScanlineEx<0x6440e255>);
	m_dsmap.SetAt(0x6440ec55, &GSRasterizer::DrawScanlineEx<0x6440ec55>);
	m_dsmap.SetAt(0x6440fc15, &GSRasterizer::DrawScanlineEx<0x6440fc15>);
	m_dsmap.SetAt(0x6440fc25, &GSRasterizer::DrawScanlineEx<0x6440fc25>);
	m_dsmap.SetAt(0x6440fc55, &GSRasterizer::DrawScanlineEx<0x6440fc55>);
	m_dsmap.SetAt(0x6444c265, &GSRasterizer::DrawScanlineEx<0x6444c265>);
	m_dsmap.SetAt(0x6444d465, &GSRasterizer::DrawScanlineEx<0x6444d465>);
	m_dsmap.SetAt(0x6444fc75, &GSRasterizer::DrawScanlineEx<0x6444fc75>);
	m_dsmap.SetAt(0x6484fc75, &GSRasterizer::DrawScanlineEx<0x6484fc75>);
	m_dsmap.SetAt(0x66402275, &GSRasterizer::DrawScanlineEx<0x66402275>);
	m_dsmap.SetAt(0x66402895, &GSRasterizer::DrawScanlineEx<0x66402895>);
	m_dsmap.SetAt(0x664038a5, &GSRasterizer::DrawScanlineEx<0x664038a5>);
	m_dsmap.SetAt(0x6680e425, &GSRasterizer::DrawScanlineEx<0x6680e425>);
	m_dsmap.SetAt(0x6a902215, &GSRasterizer::DrawScanlineEx<0x6a902215>);

	// ffxii

	m_dsmap.SetAt(0x0ff02204, &GSRasterizer::DrawScanlineEx<0x0ff02204>);
	m_dsmap.SetAt(0x0ff11454, &GSRasterizer::DrawScanlineEx<0x0ff11454>);
	m_dsmap.SetAt(0x0ff11c14, &GSRasterizer::DrawScanlineEx<0x0ff11c14>);
	m_dsmap.SetAt(0x24210224, &GSRasterizer::DrawScanlineEx<0x24210224>);
	m_dsmap.SetAt(0x24410214, &GSRasterizer::DrawScanlineEx<0x24410214>);
	m_dsmap.SetAt(0x24410254, &GSRasterizer::DrawScanlineEx<0x24410254>);
	m_dsmap.SetAt(0x24411424, &GSRasterizer::DrawScanlineEx<0x24411424>);
	m_dsmap.SetAt(0x24411454, &GSRasterizer::DrawScanlineEx<0x24411454>);
	m_dsmap.SetAt(0x24411464, &GSRasterizer::DrawScanlineEx<0x24411464>);
	m_dsmap.SetAt(0x24411c14, &GSRasterizer::DrawScanlineEx<0x24411c14>);
	m_dsmap.SetAt(0x24411c54, &GSRasterizer::DrawScanlineEx<0x24411c54>);
	m_dsmap.SetAt(0x24430c14, &GSRasterizer::DrawScanlineEx<0x24430c14>);
	m_dsmap.SetAt(0x24431054, &GSRasterizer::DrawScanlineEx<0x24431054>);
	m_dsmap.SetAt(0x24431464, &GSRasterizer::DrawScanlineEx<0x24431464>);
	m_dsmap.SetAt(0x24431c14, &GSRasterizer::DrawScanlineEx<0x24431c14>);
	m_dsmap.SetAt(0x24431c54, &GSRasterizer::DrawScanlineEx<0x24431c54>);
	m_dsmap.SetAt(0x244b0224, &GSRasterizer::DrawScanlineEx<0x244b0224>);
	m_dsmap.SetAt(0x24810224, &GSRasterizer::DrawScanlineEx<0x24810224>);
	m_dsmap.SetAt(0x24810264, &GSRasterizer::DrawScanlineEx<0x24810264>);
	m_dsmap.SetAt(0x24811424, &GSRasterizer::DrawScanlineEx<0x24811424>);
	m_dsmap.SetAt(0x24811464, &GSRasterizer::DrawScanlineEx<0x24811464>);
	m_dsmap.SetAt(0x24830254, &GSRasterizer::DrawScanlineEx<0x24830254>);
	m_dsmap.SetAt(0x24830c14, &GSRasterizer::DrawScanlineEx<0x24830c14>);
	m_dsmap.SetAt(0x24831464, &GSRasterizer::DrawScanlineEx<0x24831464>);
	m_dsmap.SetAt(0x24831c04, &GSRasterizer::DrawScanlineEx<0x24831c04>);
	m_dsmap.SetAt(0x24911424, &GSRasterizer::DrawScanlineEx<0x24911424>);
	m_dsmap.SetAt(0x24911464, &GSRasterizer::DrawScanlineEx<0x24911464>);
	m_dsmap.SetAt(0x25830214, &GSRasterizer::DrawScanlineEx<0x25830214>);
	m_dsmap.SetAt(0x2a882204, &GSRasterizer::DrawScanlineEx<0x2a882204>);
	m_dsmap.SetAt(0x2ff30204, &GSRasterizer::DrawScanlineEx<0x2ff30204>);
	m_dsmap.SetAt(0x2ff30214, &GSRasterizer::DrawScanlineEx<0x2ff30214>);
	m_dsmap.SetAt(0x2ff82204, &GSRasterizer::DrawScanlineEx<0x2ff82204>);
	m_dsmap.SetAt(0x4ff02214, &GSRasterizer::DrawScanlineEx<0x4ff02214>);
	m_dsmap.SetAt(0x4ff20214, &GSRasterizer::DrawScanlineEx<0x4ff20214>);
	m_dsmap.SetAt(0x4ff4a264, &GSRasterizer::DrawScanlineEx<0x4ff4a264>);
	m_dsmap.SetAt(0x4ff4b464, &GSRasterizer::DrawScanlineEx<0x4ff4b464>);
	m_dsmap.SetAt(0x64203054, &GSRasterizer::DrawScanlineEx<0x64203054>);
	m_dsmap.SetAt(0x64402214, &GSRasterizer::DrawScanlineEx<0x64402214>);
	m_dsmap.SetAt(0x64403054, &GSRasterizer::DrawScanlineEx<0x64403054>);
	m_dsmap.SetAt(0x6445b464, &GSRasterizer::DrawScanlineEx<0x6445b464>);
	m_dsmap.SetAt(0x6445d464, &GSRasterizer::DrawScanlineEx<0x6445d464>);
	m_dsmap.SetAt(0x6485b464, &GSRasterizer::DrawScanlineEx<0x6485b464>);
	m_dsmap.SetAt(0x0ff11c04, &GSRasterizer::DrawScanlineEx<0x0ff11c04>);
	m_dsmap.SetAt(0x24410264, &GSRasterizer::DrawScanlineEx<0x24410264>);
	m_dsmap.SetAt(0x24410c14, &GSRasterizer::DrawScanlineEx<0x24410c14>);
	m_dsmap.SetAt(0x24410c54, &GSRasterizer::DrawScanlineEx<0x24410c54>);
	m_dsmap.SetAt(0x24411c04, &GSRasterizer::DrawScanlineEx<0x24411c04>);
	m_dsmap.SetAt(0x24430c54, &GSRasterizer::DrawScanlineEx<0x24430c54>);
	m_dsmap.SetAt(0x24451464, &GSRasterizer::DrawScanlineEx<0x24451464>);
	m_dsmap.SetAt(0x24811454, &GSRasterizer::DrawScanlineEx<0x24811454>);
	m_dsmap.SetAt(0x2ff02224, &GSRasterizer::DrawScanlineEx<0x2ff02224>);
	m_dsmap.SetAt(0x4ff03814, &GSRasterizer::DrawScanlineEx<0x4ff03814>);
	m_dsmap.SetAt(0x64402254, &GSRasterizer::DrawScanlineEx<0x64402254>);
	m_dsmap.SetAt(0x64403454, &GSRasterizer::DrawScanlineEx<0x64403454>);
	m_dsmap.SetAt(0x64403474, &GSRasterizer::DrawScanlineEx<0x64403474>);
	m_dsmap.SetAt(0x6441b464, &GSRasterizer::DrawScanlineEx<0x6441b464>);
	m_dsmap.SetAt(0x6445a264, &GSRasterizer::DrawScanlineEx<0x6445a264>);
	m_dsmap.SetAt(0x64602254, &GSRasterizer::DrawScanlineEx<0x64602254>);
	m_dsmap.SetAt(0x64603454, &GSRasterizer::DrawScanlineEx<0x64603454>);
	m_dsmap.SetAt(0x64803454, &GSRasterizer::DrawScanlineEx<0x64803454>);
	m_dsmap.SetAt(0x6481b464, &GSRasterizer::DrawScanlineEx<0x6481b464>);

	// kingdom hearts

	m_dsmap.SetAt(0x0ff02205, &GSRasterizer::DrawScanlineEx<0x0ff02205>);
	m_dsmap.SetAt(0x0ff03c05, &GSRasterizer::DrawScanlineEx<0x0ff03c05>);
	m_dsmap.SetAt(0x24403c04, &GSRasterizer::DrawScanlineEx<0x24403c04>);
	m_dsmap.SetAt(0x2440d434, &GSRasterizer::DrawScanlineEx<0x2440d434>);
	m_dsmap.SetAt(0x2440d474, &GSRasterizer::DrawScanlineEx<0x2440d474>);
	m_dsmap.SetAt(0x2440d824, &GSRasterizer::DrawScanlineEx<0x2440d824>);
	m_dsmap.SetAt(0x2440dc04, &GSRasterizer::DrawScanlineEx<0x2440dc04>);
	m_dsmap.SetAt(0x24445464, &GSRasterizer::DrawScanlineEx<0x24445464>);
	m_dsmap.SetAt(0x2444d464, &GSRasterizer::DrawScanlineEx<0x2444d464>);
	m_dsmap.SetAt(0x2480d434, &GSRasterizer::DrawScanlineEx<0x2480d434>);
	m_dsmap.SetAt(0x2480d474, &GSRasterizer::DrawScanlineEx<0x2480d474>);
	m_dsmap.SetAt(0x2480dc34, &GSRasterizer::DrawScanlineEx<0x2480dc34>);
	m_dsmap.SetAt(0x2484d474, &GSRasterizer::DrawScanlineEx<0x2484d474>);
	m_dsmap.SetAt(0x28102204, &GSRasterizer::DrawScanlineEx<0x28102204>);
	m_dsmap.SetAt(0x2ff03c00, &GSRasterizer::DrawScanlineEx<0x2ff03c00>);
	m_dsmap.SetAt(0x4ff4d464, &GSRasterizer::DrawScanlineEx<0x4ff4d464>);
	m_dsmap.SetAt(0x62a4d468, &GSRasterizer::DrawScanlineEx<0x62a4d468>);
	m_dsmap.SetAt(0x6420c254, &GSRasterizer::DrawScanlineEx<0x6420c254>);
	m_dsmap.SetAt(0x64402c14, &GSRasterizer::DrawScanlineEx<0x64402c14>);
	m_dsmap.SetAt(0x64403414, &GSRasterizer::DrawScanlineEx<0x64403414>);
	m_dsmap.SetAt(0x64403c14, &GSRasterizer::DrawScanlineEx<0x64403c14>);
	m_dsmap.SetAt(0x64403c54, &GSRasterizer::DrawScanlineEx<0x64403c54>);
	m_dsmap.SetAt(0x6440c214, &GSRasterizer::DrawScanlineEx<0x6440c214>);
	m_dsmap.SetAt(0x6440c254, &GSRasterizer::DrawScanlineEx<0x6440c254>);
	m_dsmap.SetAt(0x6440d464, &GSRasterizer::DrawScanlineEx<0x6440d464>);
	m_dsmap.SetAt(0x6440dc14, &GSRasterizer::DrawScanlineEx<0x6440dc14>);
	m_dsmap.SetAt(0x6440dc54, &GSRasterizer::DrawScanlineEx<0x6440dc54>);
	m_dsmap.SetAt(0x6444b464, &GSRasterizer::DrawScanlineEx<0x6444b464>);
	m_dsmap.SetAt(0x6444b468, &GSRasterizer::DrawScanlineEx<0x6444b468>);
	m_dsmap.SetAt(0x6444d464, &GSRasterizer::DrawScanlineEx<0x6444d464>);
	m_dsmap.SetAt(0x6480c254, &GSRasterizer::DrawScanlineEx<0x6480c254>);
	m_dsmap.SetAt(0x6480dc14, &GSRasterizer::DrawScanlineEx<0x6480dc14>);
	m_dsmap.SetAt(0x6480dc54, &GSRasterizer::DrawScanlineEx<0x6480dc54>);
	m_dsmap.SetAt(0x6ff02218, &GSRasterizer::DrawScanlineEx<0x6ff02218>);
	m_dsmap.SetAt(0x6ff0c228, &GSRasterizer::DrawScanlineEx<0x6ff0c228>);
	m_dsmap.SetAt(0x0ff03805, &GSRasterizer::DrawScanlineEx<0x0ff03805>);
	m_dsmap.SetAt(0x64402c54, &GSRasterizer::DrawScanlineEx<0x64402c54>);
	m_dsmap.SetAt(0x64802c14, &GSRasterizer::DrawScanlineEx<0x64802c14>);
	m_dsmap.SetAt(0x0ff02884, &GSRasterizer::DrawScanlineEx<0x0ff02884>);
	m_dsmap.SetAt(0x0ff02c04, &GSRasterizer::DrawScanlineEx<0x0ff02c04>);
	m_dsmap.SetAt(0x2420d474, &GSRasterizer::DrawScanlineEx<0x2420d474>);
	m_dsmap.SetAt(0x2420dc34, &GSRasterizer::DrawScanlineEx<0x2420dc34>);
	m_dsmap.SetAt(0x24402804, &GSRasterizer::DrawScanlineEx<0x24402804>);
	m_dsmap.SetAt(0x2440d074, &GSRasterizer::DrawScanlineEx<0x2440d074>);
	m_dsmap.SetAt(0x2440d404, &GSRasterizer::DrawScanlineEx<0x2440d404>);
	m_dsmap.SetAt(0x2440dc34, &GSRasterizer::DrawScanlineEx<0x2440dc34>);
	m_dsmap.SetAt(0x24445468, &GSRasterizer::DrawScanlineEx<0x24445468>);
	m_dsmap.SetAt(0x2480d444, &GSRasterizer::DrawScanlineEx<0x2480d444>);
	m_dsmap.SetAt(0x64403814, &GSRasterizer::DrawScanlineEx<0x64403814>);
	m_dsmap.SetAt(0x6440d414, &GSRasterizer::DrawScanlineEx<0x6440d414>);
	m_dsmap.SetAt(0x6444d468, &GSRasterizer::DrawScanlineEx<0x6444d468>);

	// Gundam Seed Destiny OMNI VS ZAFT II PLUS 

	m_dsmap.SetAt(0x0ff12205, &GSRasterizer::DrawScanlineEx<0x0ff12205>);
	m_dsmap.SetAt(0x0ff1ac35, &GSRasterizer::DrawScanlineEx<0x0ff1ac35>);
	m_dsmap.SetAt(0x24402c05, &GSRasterizer::DrawScanlineEx<0x24402c05>);
	m_dsmap.SetAt(0x24403445, &GSRasterizer::DrawScanlineEx<0x24403445>);
	m_dsmap.SetAt(0x24403c05, &GSRasterizer::DrawScanlineEx<0x24403c05>);
	m_dsmap.SetAt(0x2445b475, &GSRasterizer::DrawScanlineEx<0x2445b475>);
	m_dsmap.SetAt(0x24802205, &GSRasterizer::DrawScanlineEx<0x24802205>);
	m_dsmap.SetAt(0x24803445, &GSRasterizer::DrawScanlineEx<0x24803445>);
	m_dsmap.SetAt(0x2485b475, &GSRasterizer::DrawScanlineEx<0x2485b475>);
	m_dsmap.SetAt(0x4ff12215, &GSRasterizer::DrawScanlineEx<0x4ff12215>);
	m_dsmap.SetAt(0x4ff1b475, &GSRasterizer::DrawScanlineEx<0x4ff1b475>);
	m_dsmap.SetAt(0x4ff5b475, &GSRasterizer::DrawScanlineEx<0x4ff5b475>);
	m_dsmap.SetAt(0x6445b475, &GSRasterizer::DrawScanlineEx<0x6445b475>);

	// disgaea 2

	m_dsmap.SetAt(0x62a0d474, &GSRasterizer::DrawScanlineEx<0x62a0d474>);
	m_dsmap.SetAt(0x64402224, &GSRasterizer::DrawScanlineEx<0x64402224>);
	m_dsmap.SetAt(0x6440c224, &GSRasterizer::DrawScanlineEx<0x6440c224>);
	m_dsmap.SetAt(0x6440dc24, &GSRasterizer::DrawScanlineEx<0x6440dc24>);
	m_dsmap.SetAt(0x6440dc64, &GSRasterizer::DrawScanlineEx<0x6440dc64>);
	m_dsmap.SetAt(0x6441d064, &GSRasterizer::DrawScanlineEx<0x6441d064>);
	m_dsmap.SetAt(0x6441d464, &GSRasterizer::DrawScanlineEx<0x6441d464>);
	m_dsmap.SetAt(0x6481d464, &GSRasterizer::DrawScanlineEx<0x6481d464>);
	m_dsmap.SetAt(0x6620d464, &GSRasterizer::DrawScanlineEx<0x6620d464>);
	m_dsmap.SetAt(0x6a802224, &GSRasterizer::DrawScanlineEx<0x6a802224>);
	m_dsmap.SetAt(0x6a80c224, &GSRasterizer::DrawScanlineEx<0x6a80c224>);

	// sfex3

	m_dsmap.SetAt(0x0ff02200, &GSRasterizer::DrawScanlineEx<0x0ff02200>);
	m_dsmap.SetAt(0x0ff0280a, &GSRasterizer::DrawScanlineEx<0x0ff0280a>);
	m_dsmap.SetAt(0x0ff0380a, &GSRasterizer::DrawScanlineEx<0x0ff0380a>);
	m_dsmap.SetAt(0x0ff0c238, &GSRasterizer::DrawScanlineEx<0x0ff0c238>);
	m_dsmap.SetAt(0x0ff0c278, &GSRasterizer::DrawScanlineEx<0x0ff0c278>);
	m_dsmap.SetAt(0x0ff0d438, &GSRasterizer::DrawScanlineEx<0x0ff0d438>);
	m_dsmap.SetAt(0x0ff0dc38, &GSRasterizer::DrawScanlineEx<0x0ff0dc38>);
	m_dsmap.SetAt(0x2428e248, &GSRasterizer::DrawScanlineEx<0x2428e248>);
	m_dsmap.SetAt(0x2428e5c8, &GSRasterizer::DrawScanlineEx<0x2428e5c8>);
	m_dsmap.SetAt(0x2440280a, &GSRasterizer::DrawScanlineEx<0x2440280a>);
	m_dsmap.SetAt(0x24402c08, &GSRasterizer::DrawScanlineEx<0x24402c08>);
	m_dsmap.SetAt(0x24402cb8, &GSRasterizer::DrawScanlineEx<0x24402cb8>);
	m_dsmap.SetAt(0x24403c08, &GSRasterizer::DrawScanlineEx<0x24403c08>);
	m_dsmap.SetAt(0x24802278, &GSRasterizer::DrawScanlineEx<0x24802278>);
	m_dsmap.SetAt(0x24803438, &GSRasterizer::DrawScanlineEx<0x24803438>);
	m_dsmap.SetAt(0x2480d478, &GSRasterizer::DrawScanlineEx<0x2480d478>);
	m_dsmap.SetAt(0x2480dc38, &GSRasterizer::DrawScanlineEx<0x2480dc38>);
	m_dsmap.SetAt(0x2480e278, &GSRasterizer::DrawScanlineEx<0x2480e278>);
	m_dsmap.SetAt(0x2480fc38, &GSRasterizer::DrawScanlineEx<0x2480fc38>);
	m_dsmap.SetAt(0x2480fc78, &GSRasterizer::DrawScanlineEx<0x2480fc78>);
	m_dsmap.SetAt(0x2680c5e8, &GSRasterizer::DrawScanlineEx<0x2680c5e8>);
	m_dsmap.SetAt(0x4ff02218, &GSRasterizer::DrawScanlineEx<0x4ff02218>);
	m_dsmap.SetAt(0x4ff02258, &GSRasterizer::DrawScanlineEx<0x4ff02258>);
	m_dsmap.SetAt(0x4ff0c5e8, &GSRasterizer::DrawScanlineEx<0x4ff0c5e8>);
	m_dsmap.SetAt(0x64402218, &GSRasterizer::DrawScanlineEx<0x64402218>);
	m_dsmap.SetAt(0x64402c18, &GSRasterizer::DrawScanlineEx<0x64402c18>);
	m_dsmap.SetAt(0x64403468, &GSRasterizer::DrawScanlineEx<0x64403468>);
	m_dsmap.SetAt(0x64403c18, &GSRasterizer::DrawScanlineEx<0x64403c18>);
	m_dsmap.SetAt(0x6440c218, &GSRasterizer::DrawScanlineEx<0x6440c218>);
	m_dsmap.SetAt(0x6440c238, &GSRasterizer::DrawScanlineEx<0x6440c238>);
	m_dsmap.SetAt(0x6440c5e8, &GSRasterizer::DrawScanlineEx<0x6440c5e8>);
	m_dsmap.SetAt(0x6440cc18, &GSRasterizer::DrawScanlineEx<0x6440cc18>);
	m_dsmap.SetAt(0x6440d5f8, &GSRasterizer::DrawScanlineEx<0x6440d5f8>);
	m_dsmap.SetAt(0x6440dc18, &GSRasterizer::DrawScanlineEx<0x6440dc18>);
	m_dsmap.SetAt(0x66402258, &GSRasterizer::DrawScanlineEx<0x66402258>);
	m_dsmap.SetAt(0x6640c218, &GSRasterizer::DrawScanlineEx<0x6640c218>);
	m_dsmap.SetAt(0x66a02218, &GSRasterizer::DrawScanlineEx<0x66a02218>);
	m_dsmap.SetAt(0x6a90c238, &GSRasterizer::DrawScanlineEx<0x6a90c238>);

	// dbzbt3

	m_dsmap.SetAt(0x24203464, &GSRasterizer::DrawScanlineEx<0x24203464>);
	m_dsmap.SetAt(0x24402884, &GSRasterizer::DrawScanlineEx<0x24402884>);
	m_dsmap.SetAt(0x24402c84, &GSRasterizer::DrawScanlineEx<0x24402c84>);
	m_dsmap.SetAt(0x24402c86, &GSRasterizer::DrawScanlineEx<0x24402c86>);
	m_dsmap.SetAt(0x24402cb4, &GSRasterizer::DrawScanlineEx<0x24402cb4>);
	m_dsmap.SetAt(0x24403424, &GSRasterizer::DrawScanlineEx<0x24403424>);
	m_dsmap.SetAt(0x24403464, &GSRasterizer::DrawScanlineEx<0x24403464>);
	m_dsmap.SetAt(0x24403804, &GSRasterizer::DrawScanlineEx<0x24403804>);
	m_dsmap.SetAt(0x24403884, &GSRasterizer::DrawScanlineEx<0x24403884>);
	m_dsmap.SetAt(0x2440b464, &GSRasterizer::DrawScanlineEx<0x2440b464>);
	m_dsmap.SetAt(0x2440bc04, &GSRasterizer::DrawScanlineEx<0x2440bc04>);
	m_dsmap.SetAt(0x2440d424, &GSRasterizer::DrawScanlineEx<0x2440d424>);
	m_dsmap.SetAt(0x2440d464, &GSRasterizer::DrawScanlineEx<0x2440d464>);
	m_dsmap.SetAt(0x2440fc04, &GSRasterizer::DrawScanlineEx<0x2440fc04>);
	m_dsmap.SetAt(0x2440fc84, &GSRasterizer::DrawScanlineEx<0x2440fc84>);
	m_dsmap.SetAt(0x2443bc04, &GSRasterizer::DrawScanlineEx<0x2443bc04>);
	m_dsmap.SetAt(0x24483464, &GSRasterizer::DrawScanlineEx<0x24483464>);
	m_dsmap.SetAt(0x24483c04, &GSRasterizer::DrawScanlineEx<0x24483c04>);
	m_dsmap.SetAt(0x2448e464, &GSRasterizer::DrawScanlineEx<0x2448e464>);
	m_dsmap.SetAt(0x24802264, &GSRasterizer::DrawScanlineEx<0x24802264>);
	m_dsmap.SetAt(0x24802c84, &GSRasterizer::DrawScanlineEx<0x24802c84>);
	m_dsmap.SetAt(0x24803464, &GSRasterizer::DrawScanlineEx<0x24803464>);
	m_dsmap.SetAt(0x24882204, &GSRasterizer::DrawScanlineEx<0x24882204>);
	m_dsmap.SetAt(0x24883c04, &GSRasterizer::DrawScanlineEx<0x24883c04>);
	m_dsmap.SetAt(0x24903464, &GSRasterizer::DrawScanlineEx<0x24903464>);
	m_dsmap.SetAt(0x25202204, &GSRasterizer::DrawScanlineEx<0x25202204>);
	m_dsmap.SetAt(0x25403804, &GSRasterizer::DrawScanlineEx<0x25403804>);
	m_dsmap.SetAt(0x25803c04, &GSRasterizer::DrawScanlineEx<0x25803c04>);
	m_dsmap.SetAt(0x26202c86, &GSRasterizer::DrawScanlineEx<0x26202c86>);
	m_dsmap.SetAt(0x26402204, &GSRasterizer::DrawScanlineEx<0x26402204>);
	m_dsmap.SetAt(0x26402206, &GSRasterizer::DrawScanlineEx<0x26402206>);
	m_dsmap.SetAt(0x26402244, &GSRasterizer::DrawScanlineEx<0x26402244>);
	m_dsmap.SetAt(0x26403804, &GSRasterizer::DrawScanlineEx<0x26403804>);
	m_dsmap.SetAt(0x2640fc84, &GSRasterizer::DrawScanlineEx<0x2640fc84>);
	m_dsmap.SetAt(0x26803804, &GSRasterizer::DrawScanlineEx<0x26803804>);
	m_dsmap.SetAt(0x2ff02c86, &GSRasterizer::DrawScanlineEx<0x2ff02c86>);
	m_dsmap.SetAt(0x64403464, &GSRasterizer::DrawScanlineEx<0x64403464>);
	m_dsmap.SetAt(0x64403c24, &GSRasterizer::DrawScanlineEx<0x64403c24>);
	m_dsmap.SetAt(0x6440b464, &GSRasterizer::DrawScanlineEx<0x6440b464>);
	m_dsmap.SetAt(0x64419464, &GSRasterizer::DrawScanlineEx<0x64419464>);
	m_dsmap.SetAt(0x6540dc24, &GSRasterizer::DrawScanlineEx<0x6540dc24>);
	m_dsmap.SetAt(0x658034e4, &GSRasterizer::DrawScanlineEx<0x658034e4>);
	m_dsmap.SetAt(0x662034e4, &GSRasterizer::DrawScanlineEx<0x662034e4>);
	m_dsmap.SetAt(0x66403464, &GSRasterizer::DrawScanlineEx<0x66403464>);

	// mana khemia

	m_dsmap.SetAt(0x24402209, &GSRasterizer::DrawScanlineEx<0x24402209>);
	m_dsmap.SetAt(0x24402c09, &GSRasterizer::DrawScanlineEx<0x24402c09>);
	m_dsmap.SetAt(0x24403c09, &GSRasterizer::DrawScanlineEx<0x24403c09>);
	m_dsmap.SetAt(0x24442c09, &GSRasterizer::DrawScanlineEx<0x24442c09>);
	m_dsmap.SetAt(0x24802c09, &GSRasterizer::DrawScanlineEx<0x24802c09>);
	m_dsmap.SetAt(0x24803c09, &GSRasterizer::DrawScanlineEx<0x24803c09>);
	m_dsmap.SetAt(0x24842449, &GSRasterizer::DrawScanlineEx<0x24842449>);
	m_dsmap.SetAt(0x2485bc29, &GSRasterizer::DrawScanlineEx<0x2485bc29>);
	m_dsmap.SetAt(0x4ff02219, &GSRasterizer::DrawScanlineEx<0x4ff02219>);
	m_dsmap.SetAt(0x64402c19, &GSRasterizer::DrawScanlineEx<0x64402c19>);
	m_dsmap.SetAt(0x6445b469, &GSRasterizer::DrawScanlineEx<0x6445b469>);
	m_dsmap.SetAt(0x6445bc29, &GSRasterizer::DrawScanlineEx<0x6445bc29>);
	m_dsmap.SetAt(0x0ff02202, &GSRasterizer::DrawScanlineEx<0x0ff02202>);
	m_dsmap.SetAt(0x24403c02, &GSRasterizer::DrawScanlineEx<0x24403c02>);
	m_dsmap.SetAt(0x64403c29, &GSRasterizer::DrawScanlineEx<0x64403c29>);

	// rumble roses

	m_dsmap.SetAt(0x24410244, &GSRasterizer::DrawScanlineEx<0x24410244>);
	m_dsmap.SetAt(0x24430214, &GSRasterizer::DrawScanlineEx<0x24430214>);
	m_dsmap.SetAt(0x24843444, &GSRasterizer::DrawScanlineEx<0x24843444>);
	m_dsmap.SetAt(0x2484d464, &GSRasterizer::DrawScanlineEx<0x2484d464>);
	m_dsmap.SetAt(0x26242244, &GSRasterizer::DrawScanlineEx<0x26242244>);
	m_dsmap.SetAt(0x2ff10204, &GSRasterizer::DrawScanlineEx<0x2ff10204>);
	m_dsmap.SetAt(0x4ff0dc24, &GSRasterizer::DrawScanlineEx<0x4ff0dc24>);
	m_dsmap.SetAt(0x6440c264, &GSRasterizer::DrawScanlineEx<0x6440c264>);
	m_dsmap.SetAt(0x6440d5e4, &GSRasterizer::DrawScanlineEx<0x6440d5e4>);
	m_dsmap.SetAt(0x6440d824, &GSRasterizer::DrawScanlineEx<0x6440d824>);
	m_dsmap.SetAt(0x6443d5e4, &GSRasterizer::DrawScanlineEx<0x6443d5e4>);
	m_dsmap.SetAt(0x64443064, &GSRasterizer::DrawScanlineEx<0x64443064>);
	m_dsmap.SetAt(0x6447d464, &GSRasterizer::DrawScanlineEx<0x6447d464>);
	m_dsmap.SetAt(0x65843464, &GSRasterizer::DrawScanlineEx<0x65843464>);
	m_dsmap.SetAt(0x6640d824, &GSRasterizer::DrawScanlineEx<0x6640d824>);
	m_dsmap.SetAt(0x66443064, &GSRasterizer::DrawScanlineEx<0x66443064>);
	m_dsmap.SetAt(0x66443464, &GSRasterizer::DrawScanlineEx<0x66443464>);
	m_dsmap.SetAt(0x6644d464, &GSRasterizer::DrawScanlineEx<0x6644d464>);
	m_dsmap.SetAt(0x6645d464, &GSRasterizer::DrawScanlineEx<0x6645d464>);
	m_dsmap.SetAt(0x6a9c2274, &GSRasterizer::DrawScanlineEx<0x6a9c2274>);

	// the punisher

	m_dsmap.SetAt(0x2423d064, &GSRasterizer::DrawScanlineEx<0x2423d064>);
	m_dsmap.SetAt(0x2423d464, &GSRasterizer::DrawScanlineEx<0x2423d464>);
	m_dsmap.SetAt(0x2443c204, &GSRasterizer::DrawScanlineEx<0x2443c204>);
	m_dsmap.SetAt(0x2443c206, &GSRasterizer::DrawScanlineEx<0x2443c206>);
	m_dsmap.SetAt(0x2443cc04, &GSRasterizer::DrawScanlineEx<0x2443cc04>);
	m_dsmap.SetAt(0x2443d464, &GSRasterizer::DrawScanlineEx<0x2443d464>);
	m_dsmap.SetAt(0x2443dc04, &GSRasterizer::DrawScanlineEx<0x2443dc04>);
	m_dsmap.SetAt(0x2443dc24, &GSRasterizer::DrawScanlineEx<0x2443dc24>);
	m_dsmap.SetAt(0x2483d444, &GSRasterizer::DrawScanlineEx<0x2483d444>);
	m_dsmap.SetAt(0x2483d464, &GSRasterizer::DrawScanlineEx<0x2483d464>);
	m_dsmap.SetAt(0x2483dc24, &GSRasterizer::DrawScanlineEx<0x2483dc24>);
	m_dsmap.SetAt(0x2683c204, &GSRasterizer::DrawScanlineEx<0x2683c204>);
	m_dsmap.SetAt(0x2683d464, &GSRasterizer::DrawScanlineEx<0x2683d464>);
	m_dsmap.SetAt(0x2a83c246, &GSRasterizer::DrawScanlineEx<0x2a83c246>);
	m_dsmap.SetAt(0x2a83dc04, &GSRasterizer::DrawScanlineEx<0x2a83dc04>);
	m_dsmap.SetAt(0x2ff02204, &GSRasterizer::DrawScanlineEx<0x2ff02204>);
	m_dsmap.SetAt(0x6443d464, &GSRasterizer::DrawScanlineEx<0x6443d464>);
	m_dsmap.SetAt(0x6443d564, &GSRasterizer::DrawScanlineEx<0x6443d564>);
	m_dsmap.SetAt(0x6a83d464, &GSRasterizer::DrawScanlineEx<0x6a83d464>);
	m_dsmap.SetAt(0x6a83d564, &GSRasterizer::DrawScanlineEx<0x6a83d564>);

	// tales of abyss

	m_dsmap.SetAt(0x0ff02208, &GSRasterizer::DrawScanlineEx<0x0ff02208>);
	m_dsmap.SetAt(0x0ff03c88, &GSRasterizer::DrawScanlineEx<0x0ff03c88>);
	m_dsmap.SetAt(0x2091a008, &GSRasterizer::DrawScanlineEx<0x2091a008>);
	m_dsmap.SetAt(0x2091a048, &GSRasterizer::DrawScanlineEx<0x2091a048>);
	m_dsmap.SetAt(0x24402208, &GSRasterizer::DrawScanlineEx<0x24402208>);
	m_dsmap.SetAt(0x24402248, &GSRasterizer::DrawScanlineEx<0x24402248>);
	m_dsmap.SetAt(0x2441a448, &GSRasterizer::DrawScanlineEx<0x2441a448>);
	m_dsmap.SetAt(0x2441b408, &GSRasterizer::DrawScanlineEx<0x2441b408>);
	m_dsmap.SetAt(0x2441b428, &GSRasterizer::DrawScanlineEx<0x2441b428>);
	m_dsmap.SetAt(0x2441b448, &GSRasterizer::DrawScanlineEx<0x2441b448>);
	m_dsmap.SetAt(0x2441b468, &GSRasterizer::DrawScanlineEx<0x2441b468>);
	m_dsmap.SetAt(0x24442268, &GSRasterizer::DrawScanlineEx<0x24442268>);
	m_dsmap.SetAt(0x2445b468, &GSRasterizer::DrawScanlineEx<0x2445b468>);
	m_dsmap.SetAt(0x2481b448, &GSRasterizer::DrawScanlineEx<0x2481b448>);
	m_dsmap.SetAt(0x2481b468, &GSRasterizer::DrawScanlineEx<0x2481b468>);
	m_dsmap.SetAt(0x2484f468, &GSRasterizer::DrawScanlineEx<0x2484f468>);
	m_dsmap.SetAt(0x2485b468, &GSRasterizer::DrawScanlineEx<0x2485b468>);
	m_dsmap.SetAt(0x25403c88, &GSRasterizer::DrawScanlineEx<0x25403c88>);
	m_dsmap.SetAt(0x26403c08, &GSRasterizer::DrawScanlineEx<0x26403c08>);
	m_dsmap.SetAt(0x2680d408, &GSRasterizer::DrawScanlineEx<0x2680d408>);
	m_dsmap.SetAt(0x2680d448, &GSRasterizer::DrawScanlineEx<0x2680d448>);
	m_dsmap.SetAt(0x2a803448, &GSRasterizer::DrawScanlineEx<0x2a803448>);
	m_dsmap.SetAt(0x2a803c08, &GSRasterizer::DrawScanlineEx<0x2a803c08>);
	m_dsmap.SetAt(0x2a805408, &GSRasterizer::DrawScanlineEx<0x2a805408>);
	m_dsmap.SetAt(0x2a805448, &GSRasterizer::DrawScanlineEx<0x2a805448>);
	m_dsmap.SetAt(0x2a80d408, &GSRasterizer::DrawScanlineEx<0x2a80d408>);
	m_dsmap.SetAt(0x2a80d448, &GSRasterizer::DrawScanlineEx<0x2a80d448>);
	m_dsmap.SetAt(0x2a81b428, &GSRasterizer::DrawScanlineEx<0x2a81b428>);
	m_dsmap.SetAt(0x2a81b448, &GSRasterizer::DrawScanlineEx<0x2a81b448>);
	m_dsmap.SetAt(0x2a81b468, &GSRasterizer::DrawScanlineEx<0x2a81b468>);
	m_dsmap.SetAt(0x2ff02208, &GSRasterizer::DrawScanlineEx<0x2ff02208>);
	m_dsmap.SetAt(0x2ff02cb8, &GSRasterizer::DrawScanlineEx<0x2ff02cb8>);
	m_dsmap.SetAt(0x2ff03c88, &GSRasterizer::DrawScanlineEx<0x2ff03c88>);
	m_dsmap.SetAt(0x4ff02228, &GSRasterizer::DrawScanlineEx<0x4ff02228>);
	m_dsmap.SetAt(0x4ff02268, &GSRasterizer::DrawScanlineEx<0x4ff02268>);
	m_dsmap.SetAt(0x6441b428, &GSRasterizer::DrawScanlineEx<0x6441b428>);
	m_dsmap.SetAt(0x6441b458, &GSRasterizer::DrawScanlineEx<0x6441b458>);
	m_dsmap.SetAt(0x6441b468, &GSRasterizer::DrawScanlineEx<0x6441b468>);
	m_dsmap.SetAt(0x64443468, &GSRasterizer::DrawScanlineEx<0x64443468>);
	m_dsmap.SetAt(0x6445a468, &GSRasterizer::DrawScanlineEx<0x6445a468>);
	m_dsmap.SetAt(0x6445b458, &GSRasterizer::DrawScanlineEx<0x6445b458>);
	m_dsmap.SetAt(0x6445b468, &GSRasterizer::DrawScanlineEx<0x6445b468>);
	m_dsmap.SetAt(0x6481b468, &GSRasterizer::DrawScanlineEx<0x6481b468>);
	m_dsmap.SetAt(0x6485a468, &GSRasterizer::DrawScanlineEx<0x6485a468>);
	m_dsmap.SetAt(0x6485b458, &GSRasterizer::DrawScanlineEx<0x6485b458>);
	m_dsmap.SetAt(0x6485b468, &GSRasterizer::DrawScanlineEx<0x6485b468>);
	m_dsmap.SetAt(0x2464f468, &GSRasterizer::DrawScanlineEx<0x2464f468>);
	m_dsmap.SetAt(0x2a803048, &GSRasterizer::DrawScanlineEx<0x2a803048>);

	// 12riven

	m_dsmap.SetAt(0x24402c48, &GSRasterizer::DrawScanlineEx<0x24402c48>);

	// ffx-2

	m_dsmap.SetAt(0x20002806, &GSRasterizer::DrawScanlineEx<0x20002806>);
	m_dsmap.SetAt(0x24402805, &GSRasterizer::DrawScanlineEx<0x24402805>);
	m_dsmap.SetAt(0x2440c445, &GSRasterizer::DrawScanlineEx<0x2440c445>);
	m_dsmap.SetAt(0x2440cc05, &GSRasterizer::DrawScanlineEx<0x2440cc05>);
	m_dsmap.SetAt(0x2440d805, &GSRasterizer::DrawScanlineEx<0x2440d805>);
	m_dsmap.SetAt(0x2440dc25, &GSRasterizer::DrawScanlineEx<0x2440dc25>);
	m_dsmap.SetAt(0x24410215, &GSRasterizer::DrawScanlineEx<0x24410215>);
	m_dsmap.SetAt(0x24410815, &GSRasterizer::DrawScanlineEx<0x24410815>);
	m_dsmap.SetAt(0x24411c19, &GSRasterizer::DrawScanlineEx<0x24411c19>);
	m_dsmap.SetAt(0x24490214, &GSRasterizer::DrawScanlineEx<0x24490214>);
	m_dsmap.SetAt(0x24490814, &GSRasterizer::DrawScanlineEx<0x24490814>);
	m_dsmap.SetAt(0x24491854, &GSRasterizer::DrawScanlineEx<0x24491854>);
	m_dsmap.SetAt(0x244b1814, &GSRasterizer::DrawScanlineEx<0x244b1814>);
	m_dsmap.SetAt(0x2484d465, &GSRasterizer::DrawScanlineEx<0x2484d465>);
	m_dsmap.SetAt(0x2880d465, &GSRasterizer::DrawScanlineEx<0x2880d465>);
	m_dsmap.SetAt(0x2a902205, &GSRasterizer::DrawScanlineEx<0x2a902205>);
	m_dsmap.SetAt(0x2ff02805, &GSRasterizer::DrawScanlineEx<0x2ff02805>);
	m_dsmap.SetAt(0x2ff02c45, &GSRasterizer::DrawScanlineEx<0x2ff02c45>);
	m_dsmap.SetAt(0x2ff0c245, &GSRasterizer::DrawScanlineEx<0x2ff0c245>);
	m_dsmap.SetAt(0x2ff0cc05, &GSRasterizer::DrawScanlineEx<0x2ff0cc05>);
	m_dsmap.SetAt(0x2ff0cc45, &GSRasterizer::DrawScanlineEx<0x2ff0cc45>);
	m_dsmap.SetAt(0x4ff03829, &GSRasterizer::DrawScanlineEx<0x4ff03829>);
	m_dsmap.SetAt(0x6440d065, &GSRasterizer::DrawScanlineEx<0x6440d065>);
	m_dsmap.SetAt(0x6440d425, &GSRasterizer::DrawScanlineEx<0x6440d425>);
	m_dsmap.SetAt(0x6440d815, &GSRasterizer::DrawScanlineEx<0x6440d815>);
	m_dsmap.SetAt(0x6440e225, &GSRasterizer::DrawScanlineEx<0x6440e225>);
	m_dsmap.SetAt(0x6440ec15, &GSRasterizer::DrawScanlineEx<0x6440ec15>);
	m_dsmap.SetAt(0x6440ec25, &GSRasterizer::DrawScanlineEx<0x6440ec25>);
	m_dsmap.SetAt(0x6440fc75, &GSRasterizer::DrawScanlineEx<0x6440fc75>);
	m_dsmap.SetAt(0x64420215, &GSRasterizer::DrawScanlineEx<0x64420215>);
	m_dsmap.SetAt(0x64802219, &GSRasterizer::DrawScanlineEx<0x64802219>);
	m_dsmap.SetAt(0x64802259, &GSRasterizer::DrawScanlineEx<0x64802259>);
	m_dsmap.SetAt(0x64802269, &GSRasterizer::DrawScanlineEx<0x64802269>);
	m_dsmap.SetAt(0x6480ac15, &GSRasterizer::DrawScanlineEx<0x6480ac15>);
	m_dsmap.SetAt(0x6480c215, &GSRasterizer::DrawScanlineEx<0x6480c215>);
	m_dsmap.SetAt(0x6480ec15, &GSRasterizer::DrawScanlineEx<0x6480ec15>);
	m_dsmap.SetAt(0x6480fc15, &GSRasterizer::DrawScanlineEx<0x6480fc15>);
	m_dsmap.SetAt(0x6484d465, &GSRasterizer::DrawScanlineEx<0x6484d465>);
	m_dsmap.SetAt(0x66402215, &GSRasterizer::DrawScanlineEx<0x66402215>);
	m_dsmap.SetAt(0x66402815, &GSRasterizer::DrawScanlineEx<0x66402815>);
	m_dsmap.SetAt(0x6880c265, &GSRasterizer::DrawScanlineEx<0x6880c265>);
	m_dsmap.SetAt(0x6880d065, &GSRasterizer::DrawScanlineEx<0x6880d065>);
	m_dsmap.SetAt(0x6880d465, &GSRasterizer::DrawScanlineEx<0x6880d465>);

	// persona 4

	m_dsmap.SetAt(0x24210248, &GSRasterizer::DrawScanlineEx<0x24210248>);
	m_dsmap.SetAt(0x24251468, &GSRasterizer::DrawScanlineEx<0x24251468>);
	m_dsmap.SetAt(0x24402268, &GSRasterizer::DrawScanlineEx<0x24402268>);
	m_dsmap.SetAt(0x2441a268, &GSRasterizer::DrawScanlineEx<0x2441a268>);
	m_dsmap.SetAt(0x24803468, &GSRasterizer::DrawScanlineEx<0x24803468>);
	m_dsmap.SetAt(0x24810248, &GSRasterizer::DrawScanlineEx<0x24810248>);
	m_dsmap.SetAt(0x24811048, &GSRasterizer::DrawScanlineEx<0x24811048>);
	m_dsmap.SetAt(0x24811468, &GSRasterizer::DrawScanlineEx<0x24811468>);
	m_dsmap.SetAt(0x24842268, &GSRasterizer::DrawScanlineEx<0x24842268>);
	m_dsmap.SetAt(0x24851468, &GSRasterizer::DrawScanlineEx<0x24851468>);
	m_dsmap.SetAt(0x25231448, &GSRasterizer::DrawScanlineEx<0x25231448>);
	m_dsmap.SetAt(0x25402248, &GSRasterizer::DrawScanlineEx<0x25402248>);
	m_dsmap.SetAt(0x25403468, &GSRasterizer::DrawScanlineEx<0x25403468>);
	m_dsmap.SetAt(0x2580c268, &GSRasterizer::DrawScanlineEx<0x2580c268>);
	m_dsmap.SetAt(0x25830248, &GSRasterizer::DrawScanlineEx<0x25830248>);
	m_dsmap.SetAt(0x25831448, &GSRasterizer::DrawScanlineEx<0x25831448>);
	m_dsmap.SetAt(0x26842068, &GSRasterizer::DrawScanlineEx<0x26842068>);
	m_dsmap.SetAt(0x26843468, &GSRasterizer::DrawScanlineEx<0x26843468>);
	m_dsmap.SetAt(0x4ff42268, &GSRasterizer::DrawScanlineEx<0x4ff42268>);
	m_dsmap.SetAt(0x64402258, &GSRasterizer::DrawScanlineEx<0x64402258>);
	m_dsmap.SetAt(0x6440d068, &GSRasterizer::DrawScanlineEx<0x6440d068>);
	m_dsmap.SetAt(0x6440d468, &GSRasterizer::DrawScanlineEx<0x6440d468>);
	m_dsmap.SetAt(0x6441a468, &GSRasterizer::DrawScanlineEx<0x6441a468>);
	m_dsmap.SetAt(0x6441d468, &GSRasterizer::DrawScanlineEx<0x6441d468>);
	m_dsmap.SetAt(0x64442268, &GSRasterizer::DrawScanlineEx<0x64442268>);
	m_dsmap.SetAt(0x6447b468, &GSRasterizer::DrawScanlineEx<0x6447b468>);
	m_dsmap.SetAt(0x64802268, &GSRasterizer::DrawScanlineEx<0x64802268>);
	m_dsmap.SetAt(0x64803468, &GSRasterizer::DrawScanlineEx<0x64803468>);
	m_dsmap.SetAt(0x66842068, &GSRasterizer::DrawScanlineEx<0x66842068>);
	m_dsmap.SetAt(0x66843468, &GSRasterizer::DrawScanlineEx<0x66843468>);
	m_dsmap.SetAt(0x6ff02258, &GSRasterizer::DrawScanlineEx<0x6ff02258>);
	m_dsmap.SetAt(0x6ff02268, &GSRasterizer::DrawScanlineEx<0x6ff02268>);
	m_dsmap.SetAt(0x0ff1a248, &GSRasterizer::DrawScanlineEx<0x0ff1a248>);
	m_dsmap.SetAt(0x2ff82248, &GSRasterizer::DrawScanlineEx<0x2ff82248>);
	m_dsmap.SetAt(0x64402268, &GSRasterizer::DrawScanlineEx<0x64402268>);
	m_dsmap.SetAt(0x6445a268, &GSRasterizer::DrawScanlineEx<0x6445a268>);
	m_dsmap.SetAt(0x24411448, &GSRasterizer::DrawScanlineEx<0x24411448>);
	m_dsmap.SetAt(0x25411048, &GSRasterizer::DrawScanlineEx<0x25411048>);
	m_dsmap.SetAt(0x2541b048, &GSRasterizer::DrawScanlineEx<0x2541b048>);
	m_dsmap.SetAt(0x2541b448, &GSRasterizer::DrawScanlineEx<0x2541b448>);
	m_dsmap.SetAt(0x2541b468, &GSRasterizer::DrawScanlineEx<0x2541b468>);
	m_dsmap.SetAt(0x6443b468, &GSRasterizer::DrawScanlineEx<0x6443b468>);

	// xenosaga

	m_dsmap.SetAt(0x0ff10234, &GSRasterizer::DrawScanlineEx<0x0ff10234>);
	m_dsmap.SetAt(0x2420d424, &GSRasterizer::DrawScanlineEx<0x2420d424>);
	m_dsmap.SetAt(0x24402234, &GSRasterizer::DrawScanlineEx<0x24402234>);
	m_dsmap.SetAt(0x24402264, &GSRasterizer::DrawScanlineEx<0x24402264>);
	m_dsmap.SetAt(0x24402274, &GSRasterizer::DrawScanlineEx<0x24402274>);
	m_dsmap.SetAt(0x24402c34, &GSRasterizer::DrawScanlineEx<0x24402c34>);
	m_dsmap.SetAt(0x24403834, &GSRasterizer::DrawScanlineEx<0x24403834>);
	m_dsmap.SetAt(0x24407464, &GSRasterizer::DrawScanlineEx<0x24407464>);
	m_dsmap.SetAt(0x24430264, &GSRasterizer::DrawScanlineEx<0x24430264>);
	m_dsmap.SetAt(0x2480c264, &GSRasterizer::DrawScanlineEx<0x2480c264>);
	m_dsmap.SetAt(0x2480d424, &GSRasterizer::DrawScanlineEx<0x2480d424>);
	m_dsmap.SetAt(0x26203464, &GSRasterizer::DrawScanlineEx<0x26203464>);
	m_dsmap.SetAt(0x26403464, &GSRasterizer::DrawScanlineEx<0x26403464>);
	m_dsmap.SetAt(0x26408434, &GSRasterizer::DrawScanlineEx<0x26408434>);
	m_dsmap.SetAt(0x26802264, &GSRasterizer::DrawScanlineEx<0x26802264>);
	m_dsmap.SetAt(0x26810214, &GSRasterizer::DrawScanlineEx<0x26810214>);
	m_dsmap.SetAt(0x28911434, &GSRasterizer::DrawScanlineEx<0x28911434>);
	m_dsmap.SetAt(0x2ff10234, &GSRasterizer::DrawScanlineEx<0x2ff10234>);
	m_dsmap.SetAt(0x4ff02c34, &GSRasterizer::DrawScanlineEx<0x4ff02c34>);
	m_dsmap.SetAt(0x60003c14, &GSRasterizer::DrawScanlineEx<0x60003c14>);
	m_dsmap.SetAt(0x6000c214, &GSRasterizer::DrawScanlineEx<0x6000c214>);
	m_dsmap.SetAt(0x6000cc14, &GSRasterizer::DrawScanlineEx<0x6000cc14>);
	m_dsmap.SetAt(0x6000cc24, &GSRasterizer::DrawScanlineEx<0x6000cc24>);
	m_dsmap.SetAt(0x6000cc34, &GSRasterizer::DrawScanlineEx<0x6000cc34>);
	m_dsmap.SetAt(0x6110cc34, &GSRasterizer::DrawScanlineEx<0x6110cc34>);
	m_dsmap.SetAt(0x6290cc14, &GSRasterizer::DrawScanlineEx<0x6290cc14>);
	m_dsmap.SetAt(0x6440cc14, &GSRasterizer::DrawScanlineEx<0x6440cc14>);
	m_dsmap.SetAt(0x6440cc24, &GSRasterizer::DrawScanlineEx<0x6440cc24>);
	m_dsmap.SetAt(0x6440cc34, &GSRasterizer::DrawScanlineEx<0x6440cc34>);
	m_dsmap.SetAt(0x6440d424, &GSRasterizer::DrawScanlineEx<0x6440d424>);
	m_dsmap.SetAt(0x6600c274, &GSRasterizer::DrawScanlineEx<0x6600c274>);
	m_dsmap.SetAt(0x66803464, &GSRasterizer::DrawScanlineEx<0x66803464>);
	m_dsmap.SetAt(0x6680b464, &GSRasterizer::DrawScanlineEx<0x6680b464>);
	m_dsmap.SetAt(0x6680cc14, &GSRasterizer::DrawScanlineEx<0x6680cc14>);
	m_dsmap.SetAt(0x6a902224, &GSRasterizer::DrawScanlineEx<0x6a902224>);
	m_dsmap.SetAt(0x6ff02264, &GSRasterizer::DrawScanlineEx<0x6ff02264>);
	m_dsmap.SetAt(0x6ff03464, &GSRasterizer::DrawScanlineEx<0x6ff03464>);
	m_dsmap.SetAt(0x6ff0b464, &GSRasterizer::DrawScanlineEx<0x6ff0b464>);
	m_dsmap.SetAt(0x0ff10c14, &GSRasterizer::DrawScanlineEx<0x0ff10c14>);
	m_dsmap.SetAt(0x24410234, &GSRasterizer::DrawScanlineEx<0x24410234>);
	m_dsmap.SetAt(0x24447464, &GSRasterizer::DrawScanlineEx<0x24447464>);
	m_dsmap.SetAt(0x26430c34, &GSRasterizer::DrawScanlineEx<0x26430c34>);
	m_dsmap.SetAt(0x26431c34, &GSRasterizer::DrawScanlineEx<0x26431c34>);
	m_dsmap.SetAt(0x26842264, &GSRasterizer::DrawScanlineEx<0x26842264>);
	m_dsmap.SetAt(0x4ff0dc34, &GSRasterizer::DrawScanlineEx<0x4ff0dc34>);
	m_dsmap.SetAt(0x64803c14, &GSRasterizer::DrawScanlineEx<0x64803c14>);
	m_dsmap.SetAt(0x66403c14, &GSRasterizer::DrawScanlineEx<0x66403c14>);
	m_dsmap.SetAt(0x6ff42264, &GSRasterizer::DrawScanlineEx<0x6ff42264>);
	m_dsmap.SetAt(0x6ff43464, &GSRasterizer::DrawScanlineEx<0x6ff43464>);
	m_dsmap.SetAt(0x6ff4b464, &GSRasterizer::DrawScanlineEx<0x6ff4b464>);
	m_dsmap.SetAt(0x0ff10c04, &GSRasterizer::DrawScanlineEx<0x0ff10c04>);
	m_dsmap.SetAt(0x2420d404, &GSRasterizer::DrawScanlineEx<0x2420d404>);
	m_dsmap.SetAt(0x2480d404, &GSRasterizer::DrawScanlineEx<0x2480d404>);
	m_dsmap.SetAt(0x4ff03c34, &GSRasterizer::DrawScanlineEx<0x4ff03c34>);
	m_dsmap.SetAt(0x6440cc64, &GSRasterizer::DrawScanlineEx<0x6440cc64>);
	m_dsmap.SetAt(0x64420214, &GSRasterizer::DrawScanlineEx<0x64420214>);
	m_dsmap.SetAt(0x6ff03c34, &GSRasterizer::DrawScanlineEx<0x6ff03c34>);

	// xenosaga 2

	m_dsmap.SetAt(0x0ff10204, &GSRasterizer::DrawScanlineEx<0x0ff10204>);
	m_dsmap.SetAt(0x0ff42244, &GSRasterizer::DrawScanlineEx<0x0ff42244>);
	m_dsmap.SetAt(0x0ff43444, &GSRasterizer::DrawScanlineEx<0x0ff43444>);
	m_dsmap.SetAt(0x24405464, &GSRasterizer::DrawScanlineEx<0x24405464>);
	m_dsmap.SetAt(0x24443464, &GSRasterizer::DrawScanlineEx<0x24443464>);
	m_dsmap.SetAt(0x24a13404, &GSRasterizer::DrawScanlineEx<0x24a13404>);
	m_dsmap.SetAt(0x2520d424, &GSRasterizer::DrawScanlineEx<0x2520d424>);
	m_dsmap.SetAt(0x2540d424, &GSRasterizer::DrawScanlineEx<0x2540d424>);
	m_dsmap.SetAt(0x2580d424, &GSRasterizer::DrawScanlineEx<0x2580d424>);
	m_dsmap.SetAt(0x26803c04, &GSRasterizer::DrawScanlineEx<0x26803c04>);
	m_dsmap.SetAt(0x28903464, &GSRasterizer::DrawScanlineEx<0x28903464>);
	m_dsmap.SetAt(0x2ff03c04, &GSRasterizer::DrawScanlineEx<0x2ff03c04>);
	m_dsmap.SetAt(0x2ff90204, &GSRasterizer::DrawScanlineEx<0x2ff90204>);
	m_dsmap.SetAt(0x4ff42264, &GSRasterizer::DrawScanlineEx<0x4ff42264>);
	m_dsmap.SetAt(0x4ff43464, &GSRasterizer::DrawScanlineEx<0x4ff43464>);
	m_dsmap.SetAt(0x6000dc34, &GSRasterizer::DrawScanlineEx<0x6000dc34>);
	m_dsmap.SetAt(0x64803464, &GSRasterizer::DrawScanlineEx<0x64803464>);
	m_dsmap.SetAt(0x66402214, &GSRasterizer::DrawScanlineEx<0x66402214>);
	m_dsmap.SetAt(0x66802214, &GSRasterizer::DrawScanlineEx<0x66802214>);
	m_dsmap.SetAt(0x24402244, &GSRasterizer::DrawScanlineEx<0x24402244>);
	m_dsmap.SetAt(0x24402c04, &GSRasterizer::DrawScanlineEx<0x24402c04>);
	m_dsmap.SetAt(0x24802804, &GSRasterizer::DrawScanlineEx<0x24802804>);
	m_dsmap.SetAt(0x24802c04, &GSRasterizer::DrawScanlineEx<0x24802c04>);
	m_dsmap.SetAt(0x4ff02c14, &GSRasterizer::DrawScanlineEx<0x4ff02c14>);
	m_dsmap.SetAt(0x6440dc34, &GSRasterizer::DrawScanlineEx<0x6440dc34>);
	m_dsmap.SetAt(0x66202214, &GSRasterizer::DrawScanlineEx<0x66202214>);

	// onimusha 3

	m_dsmap.SetAt(0x0ff02c28, &GSRasterizer::DrawScanlineEx<0x0ff02c28>);
	m_dsmap.SetAt(0x0ff02c88, &GSRasterizer::DrawScanlineEx<0x0ff02c88>);
	m_dsmap.SetAt(0x0ff02c8a, &GSRasterizer::DrawScanlineEx<0x0ff02c8a>);
	m_dsmap.SetAt(0x0ff03808, &GSRasterizer::DrawScanlineEx<0x0ff03808>);
	m_dsmap.SetAt(0x0ff03c08, &GSRasterizer::DrawScanlineEx<0x0ff03c08>);
	m_dsmap.SetAt(0x0ff03c0a, &GSRasterizer::DrawScanlineEx<0x0ff03c0a>);
	m_dsmap.SetAt(0x0ff4d048, &GSRasterizer::DrawScanlineEx<0x0ff4d048>);
	m_dsmap.SetAt(0x0ff4d448, &GSRasterizer::DrawScanlineEx<0x0ff4d448>);
	m_dsmap.SetAt(0x24403468, &GSRasterizer::DrawScanlineEx<0x24403468>);
	m_dsmap.SetAt(0x2440bc28, &GSRasterizer::DrawScanlineEx<0x2440bc28>);
	m_dsmap.SetAt(0x24803c08, &GSRasterizer::DrawScanlineEx<0x24803c08>);
	m_dsmap.SetAt(0x24803c48, &GSRasterizer::DrawScanlineEx<0x24803c48>);
	m_dsmap.SetAt(0x26402c08, &GSRasterizer::DrawScanlineEx<0x26402c08>);
	m_dsmap.SetAt(0x26802228, &GSRasterizer::DrawScanlineEx<0x26802228>);
	m_dsmap.SetAt(0x26802c48, &GSRasterizer::DrawScanlineEx<0x26802c48>);
	m_dsmap.SetAt(0x26803c08, &GSRasterizer::DrawScanlineEx<0x26803c08>);
	m_dsmap.SetAt(0x26803c88, &GSRasterizer::DrawScanlineEx<0x26803c88>);
	m_dsmap.SetAt(0x28902c88, &GSRasterizer::DrawScanlineEx<0x28902c88>);
	m_dsmap.SetAt(0x2a102c88, &GSRasterizer::DrawScanlineEx<0x2a102c88>);
	m_dsmap.SetAt(0x2ff02238, &GSRasterizer::DrawScanlineEx<0x2ff02238>);
	m_dsmap.SetAt(0x2ff02c88, &GSRasterizer::DrawScanlineEx<0x2ff02c88>);
	m_dsmap.SetAt(0x2ff02c8a, &GSRasterizer::DrawScanlineEx<0x2ff02c8a>);
	m_dsmap.SetAt(0x2ff82208, &GSRasterizer::DrawScanlineEx<0x2ff82208>);
	m_dsmap.SetAt(0x2ff82c08, &GSRasterizer::DrawScanlineEx<0x2ff82c08>);
	m_dsmap.SetAt(0x4ff4b468, &GSRasterizer::DrawScanlineEx<0x4ff4b468>);
	m_dsmap.SetAt(0x4ff5d068, &GSRasterizer::DrawScanlineEx<0x4ff5d068>);
	m_dsmap.SetAt(0x6411d428, &GSRasterizer::DrawScanlineEx<0x6411d428>);
	m_dsmap.SetAt(0x6421d428, &GSRasterizer::DrawScanlineEx<0x6421d428>);
	m_dsmap.SetAt(0x64403c28, &GSRasterizer::DrawScanlineEx<0x64403c28>);
	m_dsmap.SetAt(0x6440ec28, &GSRasterizer::DrawScanlineEx<0x6440ec28>);
	m_dsmap.SetAt(0x6441d428, &GSRasterizer::DrawScanlineEx<0x6441d428>);
	m_dsmap.SetAt(0x6441dc28, &GSRasterizer::DrawScanlineEx<0x6441dc28>);
	m_dsmap.SetAt(0x6441dc68, &GSRasterizer::DrawScanlineEx<0x6441dc68>);
	m_dsmap.SetAt(0x6445d468, &GSRasterizer::DrawScanlineEx<0x6445d468>);
	m_dsmap.SetAt(0x6461d468, &GSRasterizer::DrawScanlineEx<0x6461d468>);
	m_dsmap.SetAt(0x6461dc28, &GSRasterizer::DrawScanlineEx<0x6461dc28>);
	m_dsmap.SetAt(0x64803c28, &GSRasterizer::DrawScanlineEx<0x64803c28>);
	m_dsmap.SetAt(0x6481c268, &GSRasterizer::DrawScanlineEx<0x6481c268>);
	m_dsmap.SetAt(0x6481d468, &GSRasterizer::DrawScanlineEx<0x6481d468>);
	m_dsmap.SetAt(0x6481dc28, &GSRasterizer::DrawScanlineEx<0x6481dc28>);
	m_dsmap.SetAt(0x6491d428, &GSRasterizer::DrawScanlineEx<0x6491d428>);
	m_dsmap.SetAt(0x6640bc28, &GSRasterizer::DrawScanlineEx<0x6640bc28>);
	m_dsmap.SetAt(0x6640ec28, &GSRasterizer::DrawScanlineEx<0x6640ec28>);
	m_dsmap.SetAt(0x6641c228, &GSRasterizer::DrawScanlineEx<0x6641c228>);
	m_dsmap.SetAt(0x6641cc28, &GSRasterizer::DrawScanlineEx<0x6641cc28>);
	m_dsmap.SetAt(0x6641dc28, &GSRasterizer::DrawScanlineEx<0x6641dc28>);
	m_dsmap.SetAt(0x6644b468, &GSRasterizer::DrawScanlineEx<0x6644b468>);
	m_dsmap.SetAt(0x6ff0bc28, &GSRasterizer::DrawScanlineEx<0x6ff0bc28>);
	m_dsmap.SetAt(0x6ff1c228, &GSRasterizer::DrawScanlineEx<0x6ff1c228>);
	m_dsmap.SetAt(0x6ff1dc28, &GSRasterizer::DrawScanlineEx<0x6ff1dc28>);
	m_dsmap.SetAt(0x6481d428, &GSRasterizer::DrawScanlineEx<0x6481d428>);
	m_dsmap.SetAt(0x6481dc68, &GSRasterizer::DrawScanlineEx<0x6481dc68>);
	m_dsmap.SetAt(0x6491dc28, &GSRasterizer::DrawScanlineEx<0x6491dc28>);

	// kh2

	m_dsmap.SetAt(0x0ff03c00, &GSRasterizer::DrawScanlineEx<0x0ff03c00>);
	m_dsmap.SetAt(0x2000d434, &GSRasterizer::DrawScanlineEx<0x2000d434>);
	m_dsmap.SetAt(0x2420d434, &GSRasterizer::DrawScanlineEx<0x2420d434>);
	m_dsmap.SetAt(0x24402c45, &GSRasterizer::DrawScanlineEx<0x24402c45>);
	m_dsmap.SetAt(0x24403c45, &GSRasterizer::DrawScanlineEx<0x24403c45>);
	m_dsmap.SetAt(0x2440dc00, &GSRasterizer::DrawScanlineEx<0x2440dc00>);
	m_dsmap.SetAt(0x24803c45, &GSRasterizer::DrawScanlineEx<0x24803c45>);
	m_dsmap.SetAt(0x2480c274, &GSRasterizer::DrawScanlineEx<0x2480c274>);
	m_dsmap.SetAt(0x2480dc04, &GSRasterizer::DrawScanlineEx<0x2480dc04>);
	m_dsmap.SetAt(0x2620d434, &GSRasterizer::DrawScanlineEx<0x2620d434>);
	m_dsmap.SetAt(0x26403805, &GSRasterizer::DrawScanlineEx<0x26403805>);
	m_dsmap.SetAt(0x26803805, &GSRasterizer::DrawScanlineEx<0x26803805>);
	m_dsmap.SetAt(0x2840d434, &GSRasterizer::DrawScanlineEx<0x2840d434>);
	m_dsmap.SetAt(0x2a60d434, &GSRasterizer::DrawScanlineEx<0x2a60d434>);
	m_dsmap.SetAt(0x2aa0d434, &GSRasterizer::DrawScanlineEx<0x2aa0d434>);
	m_dsmap.SetAt(0x4ff03894, &GSRasterizer::DrawScanlineEx<0x4ff03894>);
	m_dsmap.SetAt(0x62a4d464, &GSRasterizer::DrawScanlineEx<0x62a4d464>);
	m_dsmap.SetAt(0x64203c54, &GSRasterizer::DrawScanlineEx<0x64203c54>);
	m_dsmap.SetAt(0x64402814, &GSRasterizer::DrawScanlineEx<0x64402814>);
	m_dsmap.SetAt(0x64403c74, &GSRasterizer::DrawScanlineEx<0x64403c74>);
	m_dsmap.SetAt(0x6440c220, &GSRasterizer::DrawScanlineEx<0x6440c220>);
	m_dsmap.SetAt(0x64802214, &GSRasterizer::DrawScanlineEx<0x64802214>);
	m_dsmap.SetAt(0x64803c54, &GSRasterizer::DrawScanlineEx<0x64803c54>);

	// katamary damacy

	m_dsmap.SetAt(0x2440ec04, &GSRasterizer::DrawScanlineEx<0x2440ec04>);
	m_dsmap.SetAt(0x2440ec24, &GSRasterizer::DrawScanlineEx<0x2440ec24>);
	m_dsmap.SetAt(0x2440f464, &GSRasterizer::DrawScanlineEx<0x2440f464>);
	m_dsmap.SetAt(0x2440f805, &GSRasterizer::DrawScanlineEx<0x2440f805>);
	m_dsmap.SetAt(0x2440f824, &GSRasterizer::DrawScanlineEx<0x2440f824>);
	m_dsmap.SetAt(0x24410204, &GSRasterizer::DrawScanlineEx<0x24410204>);
	m_dsmap.SetAt(0x24410802, &GSRasterizer::DrawScanlineEx<0x24410802>);
	m_dsmap.SetAt(0x24410816, &GSRasterizer::DrawScanlineEx<0x24410816>);
	m_dsmap.SetAt(0x24442264, &GSRasterizer::DrawScanlineEx<0x24442264>);
	m_dsmap.SetAt(0x244cf444, &GSRasterizer::DrawScanlineEx<0x244cf444>);
	m_dsmap.SetAt(0x2480f444, &GSRasterizer::DrawScanlineEx<0x2480f444>);
	m_dsmap.SetAt(0x2480f464, &GSRasterizer::DrawScanlineEx<0x2480f464>);
	m_dsmap.SetAt(0x2484f444, &GSRasterizer::DrawScanlineEx<0x2484f444>);
	m_dsmap.SetAt(0x2810f464, &GSRasterizer::DrawScanlineEx<0x2810f464>);
	m_dsmap.SetAt(0x4ff02216, &GSRasterizer::DrawScanlineEx<0x4ff02216>);
	m_dsmap.SetAt(0x6440e214, &GSRasterizer::DrawScanlineEx<0x6440e214>);
	m_dsmap.SetAt(0x6440e224, &GSRasterizer::DrawScanlineEx<0x6440e224>);
	m_dsmap.SetAt(0x6440ec14, &GSRasterizer::DrawScanlineEx<0x6440ec14>);
	m_dsmap.SetAt(0x6440ec24, &GSRasterizer::DrawScanlineEx<0x6440ec24>);
	m_dsmap.SetAt(0x6440f464, &GSRasterizer::DrawScanlineEx<0x6440f464>);
	m_dsmap.SetAt(0x6442e214, &GSRasterizer::DrawScanlineEx<0x6442e214>);
	m_dsmap.SetAt(0x64443464, &GSRasterizer::DrawScanlineEx<0x64443464>);
	m_dsmap.SetAt(0x6444f454, &GSRasterizer::DrawScanlineEx<0x6444f454>);
	m_dsmap.SetAt(0x6444f464, &GSRasterizer::DrawScanlineEx<0x6444f464>);
	m_dsmap.SetAt(0x644a0214, &GSRasterizer::DrawScanlineEx<0x644a0214>);
	m_dsmap.SetAt(0x6480ec24, &GSRasterizer::DrawScanlineEx<0x6480ec24>);
	m_dsmap.SetAt(0x6480f464, &GSRasterizer::DrawScanlineEx<0x6480f464>);
	m_dsmap.SetAt(0x6484f064, &GSRasterizer::DrawScanlineEx<0x6484f064>);
	m_dsmap.SetAt(0x6484f464, &GSRasterizer::DrawScanlineEx<0x6484f464>);
	m_dsmap.SetAt(0x6810cc14, &GSRasterizer::DrawScanlineEx<0x6810cc14>);
	m_dsmap.SetAt(0x6ff02214, &GSRasterizer::DrawScanlineEx<0x6ff02214>);

	// culdcept

	m_dsmap.SetAt(0x0ff03880, &GSRasterizer::DrawScanlineEx<0x0ff03880>);
	m_dsmap.SetAt(0x2440e266, &GSRasterizer::DrawScanlineEx<0x2440e266>);
	m_dsmap.SetAt(0x2440e466, &GSRasterizer::DrawScanlineEx<0x2440e466>);
	m_dsmap.SetAt(0x2440ec26, &GSRasterizer::DrawScanlineEx<0x2440ec26>);
	m_dsmap.SetAt(0x2440f466, &GSRasterizer::DrawScanlineEx<0x2440f466>);
	m_dsmap.SetAt(0x2480e266, &GSRasterizer::DrawScanlineEx<0x2480e266>);
	m_dsmap.SetAt(0x2680e266, &GSRasterizer::DrawScanlineEx<0x2680e266>);
	m_dsmap.SetAt(0x2680ec26, &GSRasterizer::DrawScanlineEx<0x2680ec26>);
	m_dsmap.SetAt(0x2680f566, &GSRasterizer::DrawScanlineEx<0x2680f566>);
	m_dsmap.SetAt(0x2680fc26, &GSRasterizer::DrawScanlineEx<0x2680fc26>);
	m_dsmap.SetAt(0x2a10f566, &GSRasterizer::DrawScanlineEx<0x2a10f566>);
	m_dsmap.SetAt(0x4ff038a6, &GSRasterizer::DrawScanlineEx<0x4ff038a6>);
	m_dsmap.SetAt(0x4ff0e266, &GSRasterizer::DrawScanlineEx<0x4ff0e266>);
	m_dsmap.SetAt(0x4ff0e466, &GSRasterizer::DrawScanlineEx<0x4ff0e466>);
	m_dsmap.SetAt(0x4ff0ec26, &GSRasterizer::DrawScanlineEx<0x4ff0ec26>);
	m_dsmap.SetAt(0x4ff0f526, &GSRasterizer::DrawScanlineEx<0x4ff0f526>);
	m_dsmap.SetAt(0x4ff0f566, &GSRasterizer::DrawScanlineEx<0x4ff0f566>);
	m_dsmap.SetAt(0x4ff0fc26, &GSRasterizer::DrawScanlineEx<0x4ff0fc26>);
	m_dsmap.SetAt(0x4ff4ec26, &GSRasterizer::DrawScanlineEx<0x4ff4ec26>);
	m_dsmap.SetAt(0x4ff4f526, &GSRasterizer::DrawScanlineEx<0x4ff4f526>);
	m_dsmap.SetAt(0x4ff4f566, &GSRasterizer::DrawScanlineEx<0x4ff4f566>);
	m_dsmap.SetAt(0x4ff4fc26, &GSRasterizer::DrawScanlineEx<0x4ff4fc26>);
	m_dsmap.SetAt(0x6440ec26, &GSRasterizer::DrawScanlineEx<0x6440ec26>);
	m_dsmap.SetAt(0x6440f466, &GSRasterizer::DrawScanlineEx<0x6440f466>);
	m_dsmap.SetAt(0x6440fc26, &GSRasterizer::DrawScanlineEx<0x6440fc26>);
	m_dsmap.SetAt(0x6444fc26, &GSRasterizer::DrawScanlineEx<0x6444fc26>);

	// tomoyo after 

	m_dsmap.SetAt(0x0ff0220a, &GSRasterizer::DrawScanlineEx<0x0ff0220a>);
	m_dsmap.SetAt(0x4ff03c19, &GSRasterizer::DrawScanlineEx<0x4ff03c19>);
	m_dsmap.SetAt(0x64203c68, &GSRasterizer::DrawScanlineEx<0x64203c68>);
	m_dsmap.SetAt(0x64402c68, &GSRasterizer::DrawScanlineEx<0x64402c68>);
	m_dsmap.SetAt(0x64403c68, &GSRasterizer::DrawScanlineEx<0x64403c68>);
	m_dsmap.SetAt(0x64802c68, &GSRasterizer::DrawScanlineEx<0x64802c68>);
	m_dsmap.SetAt(0x65402c68, &GSRasterizer::DrawScanlineEx<0x65402c68>);

	// persona 3 fes

	m_dsmap.SetAt(0x0ff02248, &GSRasterizer::DrawScanlineEx<0x0ff02248>);
	m_dsmap.SetAt(0x24410448, &GSRasterizer::DrawScanlineEx<0x24410448>);
	m_dsmap.SetAt(0x24411468, &GSRasterizer::DrawScanlineEx<0x24411468>);
	m_dsmap.SetAt(0x2445a468, &GSRasterizer::DrawScanlineEx<0x2445a468>);
	m_dsmap.SetAt(0x64461468, &GSRasterizer::DrawScanlineEx<0x64461468>);

	// tokyo bus guide

	m_dsmap.SetAt(0x0ff4d470, &GSRasterizer::DrawScanlineEx<0x0ff4d470>);
	m_dsmap.SetAt(0x0ff4f470, &GSRasterizer::DrawScanlineEx<0x0ff4f470>);
	m_dsmap.SetAt(0x24411450, &GSRasterizer::DrawScanlineEx<0x24411450>);
	m_dsmap.SetAt(0x24411470, &GSRasterizer::DrawScanlineEx<0x24411470>);
	m_dsmap.SetAt(0x2444f470, &GSRasterizer::DrawScanlineEx<0x2444f470>);
	m_dsmap.SetAt(0x4ff0e230, &GSRasterizer::DrawScanlineEx<0x4ff0e230>);
	m_dsmap.SetAt(0x4ff0e270, &GSRasterizer::DrawScanlineEx<0x4ff0e270>);
	m_dsmap.SetAt(0x4ff4d470, &GSRasterizer::DrawScanlineEx<0x4ff4d470>);
	m_dsmap.SetAt(0x4ff4f470, &GSRasterizer::DrawScanlineEx<0x4ff4f470>);
	m_dsmap.SetAt(0x6440d470, &GSRasterizer::DrawScanlineEx<0x6440d470>);
	m_dsmap.SetAt(0x6440e230, &GSRasterizer::DrawScanlineEx<0x6440e230>);
	m_dsmap.SetAt(0x6440e270, &GSRasterizer::DrawScanlineEx<0x6440e270>);
	m_dsmap.SetAt(0x6440f430, &GSRasterizer::DrawScanlineEx<0x6440f430>);
	m_dsmap.SetAt(0x6440fc30, &GSRasterizer::DrawScanlineEx<0x6440fc30>);
	m_dsmap.SetAt(0x64420210, &GSRasterizer::DrawScanlineEx<0x64420210>);
	m_dsmap.SetAt(0x6444f470, &GSRasterizer::DrawScanlineEx<0x6444f470>);
	m_dsmap.SetAt(0x64453450, &GSRasterizer::DrawScanlineEx<0x64453450>);
	m_dsmap.SetAt(0x66403c20, &GSRasterizer::DrawScanlineEx<0x66403c20>);

	// grandia 3

	m_dsmap.SetAt(0x20931460, &GSRasterizer::DrawScanlineEx<0x20931460>);
	m_dsmap.SetAt(0x20a0224a, &GSRasterizer::DrawScanlineEx<0x20a0224a>);
	m_dsmap.SetAt(0x24202200, &GSRasterizer::DrawScanlineEx<0x24202200>);
	m_dsmap.SetAt(0x24271460, &GSRasterizer::DrawScanlineEx<0x24271460>);
	m_dsmap.SetAt(0x24403440, &GSRasterizer::DrawScanlineEx<0x24403440>);
	m_dsmap.SetAt(0x24431470, &GSRasterizer::DrawScanlineEx<0x24431470>);
	m_dsmap.SetAt(0x24471460, &GSRasterizer::DrawScanlineEx<0x24471460>);
	m_dsmap.SetAt(0x24471470, &GSRasterizer::DrawScanlineEx<0x24471470>);
	m_dsmap.SetAt(0x24803c00, &GSRasterizer::DrawScanlineEx<0x24803c00>);
	m_dsmap.SetAt(0x24830240, &GSRasterizer::DrawScanlineEx<0x24830240>);
	m_dsmap.SetAt(0x24830260, &GSRasterizer::DrawScanlineEx<0x24830260>);
	m_dsmap.SetAt(0x24831440, &GSRasterizer::DrawScanlineEx<0x24831440>);
	m_dsmap.SetAt(0x24831460, &GSRasterizer::DrawScanlineEx<0x24831460>);
	m_dsmap.SetAt(0x24871460, &GSRasterizer::DrawScanlineEx<0x24871460>);
	m_dsmap.SetAt(0x26402200, &GSRasterizer::DrawScanlineEx<0x26402200>);
	m_dsmap.SetAt(0x26402c00, &GSRasterizer::DrawScanlineEx<0x26402c00>);
	m_dsmap.SetAt(0x26403c00, &GSRasterizer::DrawScanlineEx<0x26403c00>);
	m_dsmap.SetAt(0x26403c20, &GSRasterizer::DrawScanlineEx<0x26403c20>);
	m_dsmap.SetAt(0x26433c00, &GSRasterizer::DrawScanlineEx<0x26433c00>);
	m_dsmap.SetAt(0x26433c20, &GSRasterizer::DrawScanlineEx<0x26433c20>);
	m_dsmap.SetAt(0x28191460, &GSRasterizer::DrawScanlineEx<0x28191460>);
	m_dsmap.SetAt(0x2ff02200, &GSRasterizer::DrawScanlineEx<0x2ff02200>);
	m_dsmap.SetAt(0x4ff0221a, &GSRasterizer::DrawScanlineEx<0x4ff0221a>);
	m_dsmap.SetAt(0x6441b460, &GSRasterizer::DrawScanlineEx<0x6441b460>);
	m_dsmap.SetAt(0x6443b460, &GSRasterizer::DrawScanlineEx<0x6443b460>);
	m_dsmap.SetAt(0x6445a260, &GSRasterizer::DrawScanlineEx<0x6445a260>);
	m_dsmap.SetAt(0x6445b460, &GSRasterizer::DrawScanlineEx<0x6445b460>);
	m_dsmap.SetAt(0x6445d460, &GSRasterizer::DrawScanlineEx<0x6445d460>);
	m_dsmap.SetAt(0x6447a260, &GSRasterizer::DrawScanlineEx<0x6447a260>);
	m_dsmap.SetAt(0x6447b460, &GSRasterizer::DrawScanlineEx<0x6447b460>);
	m_dsmap.SetAt(0x6447d460, &GSRasterizer::DrawScanlineEx<0x6447d460>);
	m_dsmap.SetAt(0x644c2270, &GSRasterizer::DrawScanlineEx<0x644c2270>);
	m_dsmap.SetAt(0x66433c20, &GSRasterizer::DrawScanlineEx<0x66433c20>);
	m_dsmap.SetAt(0x20903440, &GSRasterizer::DrawScanlineEx<0x20903440>);
	m_dsmap.SetAt(0x20931470, &GSRasterizer::DrawScanlineEx<0x20931470>);
	m_dsmap.SetAt(0x24230260, &GSRasterizer::DrawScanlineEx<0x24230260>);
	m_dsmap.SetAt(0x24231460, &GSRasterizer::DrawScanlineEx<0x24231460>);
	m_dsmap.SetAt(0x24831470, &GSRasterizer::DrawScanlineEx<0x24831470>);

	// nba 2k8

	m_dsmap.SetAt(0x0ff02c02, &GSRasterizer::DrawScanlineEx<0x0ff02c02>);
	m_dsmap.SetAt(0x20a02c02, &GSRasterizer::DrawScanlineEx<0x20a02c02>);
	m_dsmap.SetAt(0x24403446, &GSRasterizer::DrawScanlineEx<0x24403446>);
	m_dsmap.SetAt(0x24403802, &GSRasterizer::DrawScanlineEx<0x24403802>);
	m_dsmap.SetAt(0x2440b466, &GSRasterizer::DrawScanlineEx<0x2440b466>);
	m_dsmap.SetAt(0x2448d466, &GSRasterizer::DrawScanlineEx<0x2448d466>);
	m_dsmap.SetAt(0x2480b466, &GSRasterizer::DrawScanlineEx<0x2480b466>);
	m_dsmap.SetAt(0x2a803882, &GSRasterizer::DrawScanlineEx<0x2a803882>);
	m_dsmap.SetAt(0x2a80b466, &GSRasterizer::DrawScanlineEx<0x2a80b466>);
	m_dsmap.SetAt(0x60002256, &GSRasterizer::DrawScanlineEx<0x60002256>);
	m_dsmap.SetAt(0x6440b456, &GSRasterizer::DrawScanlineEx<0x6440b456>);
	m_dsmap.SetAt(0x6440b466, &GSRasterizer::DrawScanlineEx<0x6440b466>);
	m_dsmap.SetAt(0x6440d466, &GSRasterizer::DrawScanlineEx<0x6440d466>);
	m_dsmap.SetAt(0x6480b066, &GSRasterizer::DrawScanlineEx<0x6480b066>);
	m_dsmap.SetAt(0x6480b466, &GSRasterizer::DrawScanlineEx<0x6480b466>);
	m_dsmap.SetAt(0x6a80a466, &GSRasterizer::DrawScanlineEx<0x6a80a466>);
	m_dsmap.SetAt(0x6a80b066, &GSRasterizer::DrawScanlineEx<0x6a80b066>);
	m_dsmap.SetAt(0x6a80b456, &GSRasterizer::DrawScanlineEx<0x6a80b456>);
	m_dsmap.SetAt(0x6a80b466, &GSRasterizer::DrawScanlineEx<0x6a80b466>);
	m_dsmap.SetAt(0x6440a466, &GSRasterizer::DrawScanlineEx<0x6440a466>);
	m_dsmap.SetAt(0x6440c466, &GSRasterizer::DrawScanlineEx<0x6440c466>);

	// ico

	m_dsmap.SetAt(0x0ff03d00, &GSRasterizer::DrawScanlineEx<0x0ff03d00>);
	m_dsmap.SetAt(0x0ff05c00, &GSRasterizer::DrawScanlineEx<0x0ff05c00>);
	m_dsmap.SetAt(0x24402200, &GSRasterizer::DrawScanlineEx<0x24402200>);
	m_dsmap.SetAt(0x24402c20, &GSRasterizer::DrawScanlineEx<0x24402c20>);
	m_dsmap.SetAt(0x24403c00, &GSRasterizer::DrawScanlineEx<0x24403c00>);
	m_dsmap.SetAt(0x2441dc20, &GSRasterizer::DrawScanlineEx<0x2441dc20>);
	m_dsmap.SetAt(0x2448dc00, &GSRasterizer::DrawScanlineEx<0x2448dc00>);
	m_dsmap.SetAt(0x24802200, &GSRasterizer::DrawScanlineEx<0x24802200>);
	m_dsmap.SetAt(0x24802220, &GSRasterizer::DrawScanlineEx<0x24802220>);
	m_dsmap.SetAt(0x2481d460, &GSRasterizer::DrawScanlineEx<0x2481d460>);
	m_dsmap.SetAt(0x2481dc20, &GSRasterizer::DrawScanlineEx<0x2481dc20>);
	m_dsmap.SetAt(0x2621d460, &GSRasterizer::DrawScanlineEx<0x2621d460>);
	m_dsmap.SetAt(0x26802200, &GSRasterizer::DrawScanlineEx<0x26802200>);
	m_dsmap.SetAt(0x26802220, &GSRasterizer::DrawScanlineEx<0x26802220>);
	m_dsmap.SetAt(0x26803c00, &GSRasterizer::DrawScanlineEx<0x26803c00>);
	m_dsmap.SetAt(0x2681d460, &GSRasterizer::DrawScanlineEx<0x2681d460>);
	m_dsmap.SetAt(0x4ff02220, &GSRasterizer::DrawScanlineEx<0x4ff02220>);
	m_dsmap.SetAt(0x4ff1d460, &GSRasterizer::DrawScanlineEx<0x4ff1d460>);
	m_dsmap.SetAt(0x6441d460, &GSRasterizer::DrawScanlineEx<0x6441d460>);
	m_dsmap.SetAt(0x64802260, &GSRasterizer::DrawScanlineEx<0x64802260>);
	m_dsmap.SetAt(0x6481d460, &GSRasterizer::DrawScanlineEx<0x6481d460>);

	// gt4

	m_dsmap.SetAt(0x0ff02201, &GSRasterizer::DrawScanlineEx<0x0ff02201>);
	m_dsmap.SetAt(0x0ff02224, &GSRasterizer::DrawScanlineEx<0x0ff02224>);
	m_dsmap.SetAt(0x0ff02c05, &GSRasterizer::DrawScanlineEx<0x0ff02c05>);
	m_dsmap.SetAt(0x0ff03c01, &GSRasterizer::DrawScanlineEx<0x0ff03c01>);
	m_dsmap.SetAt(0x0ff03c06, &GSRasterizer::DrawScanlineEx<0x0ff03c06>);
	m_dsmap.SetAt(0x0ff03c44, &GSRasterizer::DrawScanlineEx<0x0ff03c44>);
	m_dsmap.SetAt(0x0ff0ec04, &GSRasterizer::DrawScanlineEx<0x0ff0ec04>);
	m_dsmap.SetAt(0x0ff20204, &GSRasterizer::DrawScanlineEx<0x0ff20204>);
	m_dsmap.SetAt(0x24402241, &GSRasterizer::DrawScanlineEx<0x24402241>);
	m_dsmap.SetAt(0x24402441, &GSRasterizer::DrawScanlineEx<0x24402441>);
	m_dsmap.SetAt(0x24402445, &GSRasterizer::DrawScanlineEx<0x24402445>);
	m_dsmap.SetAt(0x24403441, &GSRasterizer::DrawScanlineEx<0x24403441>);
	m_dsmap.SetAt(0x24403c41, &GSRasterizer::DrawScanlineEx<0x24403c41>);
	m_dsmap.SetAt(0x2440d445, &GSRasterizer::DrawScanlineEx<0x2440d445>);
	m_dsmap.SetAt(0x24442464, &GSRasterizer::DrawScanlineEx<0x24442464>);
	m_dsmap.SetAt(0x24443444, &GSRasterizer::DrawScanlineEx<0x24443444>);
	m_dsmap.SetAt(0x24443465, &GSRasterizer::DrawScanlineEx<0x24443465>);
	m_dsmap.SetAt(0x244435e4, &GSRasterizer::DrawScanlineEx<0x244435e4>);
	m_dsmap.SetAt(0x2444b464, &GSRasterizer::DrawScanlineEx<0x2444b464>);
	m_dsmap.SetAt(0x24802244, &GSRasterizer::DrawScanlineEx<0x24802244>);
	m_dsmap.SetAt(0x24803441, &GSRasterizer::DrawScanlineEx<0x24803441>);
	m_dsmap.SetAt(0x24843464, &GSRasterizer::DrawScanlineEx<0x24843464>);
	m_dsmap.SetAt(0x25843464, &GSRasterizer::DrawScanlineEx<0x25843464>);
	m_dsmap.SetAt(0x258c3464, &GSRasterizer::DrawScanlineEx<0x258c3464>);
	m_dsmap.SetAt(0x26402205, &GSRasterizer::DrawScanlineEx<0x26402205>);
	m_dsmap.SetAt(0x26402245, &GSRasterizer::DrawScanlineEx<0x26402245>);
	m_dsmap.SetAt(0x26402c05, &GSRasterizer::DrawScanlineEx<0x26402c05>);
	m_dsmap.SetAt(0x26403c05, &GSRasterizer::DrawScanlineEx<0x26403c05>);
	m_dsmap.SetAt(0x26802c00, &GSRasterizer::DrawScanlineEx<0x26802c00>);
	m_dsmap.SetAt(0x2a902204, &GSRasterizer::DrawScanlineEx<0x2a902204>);
	m_dsmap.SetAt(0x2ff02201, &GSRasterizer::DrawScanlineEx<0x2ff02201>);
	m_dsmap.SetAt(0x2ff02205, &GSRasterizer::DrawScanlineEx<0x2ff02205>);
	m_dsmap.SetAt(0x2ff02c00, &GSRasterizer::DrawScanlineEx<0x2ff02c00>);
	m_dsmap.SetAt(0x2ff02c04, &GSRasterizer::DrawScanlineEx<0x2ff02c04>);
	m_dsmap.SetAt(0x2ff02c81, &GSRasterizer::DrawScanlineEx<0x2ff02c81>);
	m_dsmap.SetAt(0x2ff03444, &GSRasterizer::DrawScanlineEx<0x2ff03444>);
	m_dsmap.SetAt(0x2ff03446, &GSRasterizer::DrawScanlineEx<0x2ff03446>);
	m_dsmap.SetAt(0x2ff03c06, &GSRasterizer::DrawScanlineEx<0x2ff03c06>);
	m_dsmap.SetAt(0x2ff0cc04, &GSRasterizer::DrawScanlineEx<0x2ff0cc04>);
	m_dsmap.SetAt(0x2ff42264, &GSRasterizer::DrawScanlineEx<0x2ff42264>);
	m_dsmap.SetAt(0x2ff43464, &GSRasterizer::DrawScanlineEx<0x2ff43464>);
	m_dsmap.SetAt(0x4ff4c464, &GSRasterizer::DrawScanlineEx<0x4ff4c464>);
	m_dsmap.SetAt(0x64442264, &GSRasterizer::DrawScanlineEx<0x64442264>);
	m_dsmap.SetAt(0x66402c14, &GSRasterizer::DrawScanlineEx<0x66402c14>);

	// svr2k8

	m_dsmap.SetAt(0x24412244, &GSRasterizer::DrawScanlineEx<0x24412244>);
	m_dsmap.SetAt(0x2441c244, &GSRasterizer::DrawScanlineEx<0x2441c244>);
	m_dsmap.SetAt(0x2441d044, &GSRasterizer::DrawScanlineEx<0x2441d044>);
	m_dsmap.SetAt(0x2441d444, &GSRasterizer::DrawScanlineEx<0x2441d444>);
	m_dsmap.SetAt(0x2441dc04, &GSRasterizer::DrawScanlineEx<0x2441dc04>);
	m_dsmap.SetAt(0x244c3474, &GSRasterizer::DrawScanlineEx<0x244c3474>);
	m_dsmap.SetAt(0x2481a244, &GSRasterizer::DrawScanlineEx<0x2481a244>);
	m_dsmap.SetAt(0x2481b444, &GSRasterizer::DrawScanlineEx<0x2481b444>);
	m_dsmap.SetAt(0x2481b464, &GSRasterizer::DrawScanlineEx<0x2481b464>);
	m_dsmap.SetAt(0x2485d464, &GSRasterizer::DrawScanlineEx<0x2485d464>);
	m_dsmap.SetAt(0x2a910214, &GSRasterizer::DrawScanlineEx<0x2a910214>);
	m_dsmap.SetAt(0x2ff31815, &GSRasterizer::DrawScanlineEx<0x2ff31815>);
	m_dsmap.SetAt(0x4ff0dc26, &GSRasterizer::DrawScanlineEx<0x4ff0dc26>);
	m_dsmap.SetAt(0x64403064, &GSRasterizer::DrawScanlineEx<0x64403064>);
	m_dsmap.SetAt(0x6440d454, &GSRasterizer::DrawScanlineEx<0x6440d454>);
	m_dsmap.SetAt(0x6441a214, &GSRasterizer::DrawScanlineEx<0x6441a214>);
	m_dsmap.SetAt(0x6441b5e4, &GSRasterizer::DrawScanlineEx<0x6441b5e4>);
	m_dsmap.SetAt(0x6441bc14, &GSRasterizer::DrawScanlineEx<0x6441bc14>);
	m_dsmap.SetAt(0x6444d064, &GSRasterizer::DrawScanlineEx<0x6444d064>);
	m_dsmap.SetAt(0x6484d464, &GSRasterizer::DrawScanlineEx<0x6484d464>);
	m_dsmap.SetAt(0x664c3474, &GSRasterizer::DrawScanlineEx<0x664c3474>);

	// dq8

	m_dsmap.SetAt(0x0ff02c85, &GSRasterizer::DrawScanlineEx<0x0ff02c85>);
	m_dsmap.SetAt(0x0ff0dc04, &GSRasterizer::DrawScanlineEx<0x0ff0dc04>);
	m_dsmap.SetAt(0x24202204, &GSRasterizer::DrawScanlineEx<0x24202204>);
	m_dsmap.SetAt(0x2420a264, &GSRasterizer::DrawScanlineEx<0x2420a264>);
	m_dsmap.SetAt(0x24603c04, &GSRasterizer::DrawScanlineEx<0x24603c04>);
	m_dsmap.SetAt(0x24803c04, &GSRasterizer::DrawScanlineEx<0x24803c04>);
	m_dsmap.SetAt(0x24803c24, &GSRasterizer::DrawScanlineEx<0x24803c24>);
	m_dsmap.SetAt(0x2480a264, &GSRasterizer::DrawScanlineEx<0x2480a264>);
	m_dsmap.SetAt(0x26203c04, &GSRasterizer::DrawScanlineEx<0x26203c04>);
	m_dsmap.SetAt(0x28803c04, &GSRasterizer::DrawScanlineEx<0x28803c04>);
	m_dsmap.SetAt(0x28803d84, &GSRasterizer::DrawScanlineEx<0x28803d84>);
	m_dsmap.SetAt(0x6444b064, &GSRasterizer::DrawScanlineEx<0x6444b064>);
	m_dsmap.SetAt(0x6484b464, &GSRasterizer::DrawScanlineEx<0x6484b464>);
	m_dsmap.SetAt(0x22a02c04, &GSRasterizer::DrawScanlineEx<0x22a02c04>);
	m_dsmap.SetAt(0x2640bc04, &GSRasterizer::DrawScanlineEx<0x2640bc04>);
	m_dsmap.SetAt(0x6440a264, &GSRasterizer::DrawScanlineEx<0x6440a264>);

	// resident evil 4

	m_dsmap.SetAt(0x0ff02c24, &GSRasterizer::DrawScanlineEx<0x0ff02c24>);
	m_dsmap.SetAt(0x0ff02c84, &GSRasterizer::DrawScanlineEx<0x0ff02c84>);
	m_dsmap.SetAt(0x0ff03884, &GSRasterizer::DrawScanlineEx<0x0ff03884>);
	m_dsmap.SetAt(0x0ff03c84, &GSRasterizer::DrawScanlineEx<0x0ff03c84>);
	m_dsmap.SetAt(0x0ff11c94, &GSRasterizer::DrawScanlineEx<0x0ff11c94>);
	m_dsmap.SetAt(0x22a02c84, &GSRasterizer::DrawScanlineEx<0x22a02c84>);
	m_dsmap.SetAt(0x22a03c84, &GSRasterizer::DrawScanlineEx<0x22a03c84>);
	m_dsmap.SetAt(0x2440c204, &GSRasterizer::DrawScanlineEx<0x2440c204>);
	m_dsmap.SetAt(0x2440d444, &GSRasterizer::DrawScanlineEx<0x2440d444>);
	m_dsmap.SetAt(0x24802204, &GSRasterizer::DrawScanlineEx<0x24802204>);
	m_dsmap.SetAt(0x2480c224, &GSRasterizer::DrawScanlineEx<0x2480c224>);
	m_dsmap.SetAt(0x2480d464, &GSRasterizer::DrawScanlineEx<0x2480d464>);
	m_dsmap.SetAt(0x2481c264, &GSRasterizer::DrawScanlineEx<0x2481c264>);
	m_dsmap.SetAt(0x26403c84, &GSRasterizer::DrawScanlineEx<0x26403c84>);
	m_dsmap.SetAt(0x2640c224, &GSRasterizer::DrawScanlineEx<0x2640c224>);
	m_dsmap.SetAt(0x26802224, &GSRasterizer::DrawScanlineEx<0x26802224>);
	m_dsmap.SetAt(0x26803c84, &GSRasterizer::DrawScanlineEx<0x26803c84>);
	m_dsmap.SetAt(0x2680d444, &GSRasterizer::DrawScanlineEx<0x2680d444>);
	m_dsmap.SetAt(0x28902204, &GSRasterizer::DrawScanlineEx<0x28902204>);
	m_dsmap.SetAt(0x2a403c84, &GSRasterizer::DrawScanlineEx<0x2a403c84>);
	m_dsmap.SetAt(0x2ff02c84, &GSRasterizer::DrawScanlineEx<0x2ff02c84>);
	m_dsmap.SetAt(0x2ff03484, &GSRasterizer::DrawScanlineEx<0x2ff03484>);
	m_dsmap.SetAt(0x6580d464, &GSRasterizer::DrawScanlineEx<0x6580d464>);
	m_dsmap.SetAt(0x6640d464, &GSRasterizer::DrawScanlineEx<0x6640d464>);
	m_dsmap.SetAt(0x66420214, &GSRasterizer::DrawScanlineEx<0x66420214>);

	// shadow of the colossus

	m_dsmap.SetAt(0x0ff02805, &GSRasterizer::DrawScanlineEx<0x0ff02805>);
	m_dsmap.SetAt(0x0ff10214, &GSRasterizer::DrawScanlineEx<0x0ff10214>);
	m_dsmap.SetAt(0x0ff10224, &GSRasterizer::DrawScanlineEx<0x0ff10224>);
	m_dsmap.SetAt(0x0ff10254, &GSRasterizer::DrawScanlineEx<0x0ff10254>);
	m_dsmap.SetAt(0x0ff10264, &GSRasterizer::DrawScanlineEx<0x0ff10264>);
	m_dsmap.SetAt(0x0ff11c15, &GSRasterizer::DrawScanlineEx<0x0ff11c15>);
	m_dsmap.SetAt(0x24251464, &GSRasterizer::DrawScanlineEx<0x24251464>);
	m_dsmap.SetAt(0x24411814, &GSRasterizer::DrawScanlineEx<0x24411814>);
	m_dsmap.SetAt(0x24411c64, &GSRasterizer::DrawScanlineEx<0x24411c64>);
	m_dsmap.SetAt(0x24451064, &GSRasterizer::DrawScanlineEx<0x24451064>);
	m_dsmap.SetAt(0x24491c24, &GSRasterizer::DrawScanlineEx<0x24491c24>);
	m_dsmap.SetAt(0x24810214, &GSRasterizer::DrawScanlineEx<0x24810214>);
	m_dsmap.SetAt(0x24810254, &GSRasterizer::DrawScanlineEx<0x24810254>);
	m_dsmap.SetAt(0x24811c14, &GSRasterizer::DrawScanlineEx<0x24811c14>);
	m_dsmap.SetAt(0x24851464, &GSRasterizer::DrawScanlineEx<0x24851464>);
	m_dsmap.SetAt(0x25411814, &GSRasterizer::DrawScanlineEx<0x25411814>);
	m_dsmap.SetAt(0x26410214, &GSRasterizer::DrawScanlineEx<0x26410214>);
	m_dsmap.SetAt(0x26810224, &GSRasterizer::DrawScanlineEx<0x26810224>);
	m_dsmap.SetAt(0x34451464, &GSRasterizer::DrawScanlineEx<0x34451464>);
	m_dsmap.SetAt(0x4ff0c224, &GSRasterizer::DrawScanlineEx<0x4ff0c224>);
	m_dsmap.SetAt(0x6441cc64, &GSRasterizer::DrawScanlineEx<0x6441cc64>);
	m_dsmap.SetAt(0x6441dc24, &GSRasterizer::DrawScanlineEx<0x6441dc24>);
	m_dsmap.SetAt(0x6441dc64, &GSRasterizer::DrawScanlineEx<0x6441dc64>);
	m_dsmap.SetAt(0x64453464, &GSRasterizer::DrawScanlineEx<0x64453464>);
	m_dsmap.SetAt(0x6445a464, &GSRasterizer::DrawScanlineEx<0x6445a464>);
	m_dsmap.SetAt(0x6485a464, &GSRasterizer::DrawScanlineEx<0x6485a464>);
	m_dsmap.SetAt(0x6441b454, &GSRasterizer::DrawScanlineEx<0x6441b454>);
	m_dsmap.SetAt(0x6481b454, &GSRasterizer::DrawScanlineEx<0x6481b454>);

	// bully

	m_dsmap.SetAt(0x2441b424, &GSRasterizer::DrawScanlineEx<0x2441b424>);
	m_dsmap.SetAt(0x2441b464, &GSRasterizer::DrawScanlineEx<0x2441b464>);
	m_dsmap.SetAt(0x2441bc04, &GSRasterizer::DrawScanlineEx<0x2441bc04>);
	m_dsmap.SetAt(0x2441bc24, &GSRasterizer::DrawScanlineEx<0x2441bc24>);
	m_dsmap.SetAt(0x2441bc44, &GSRasterizer::DrawScanlineEx<0x2441bc44>);
	m_dsmap.SetAt(0x2445b464, &GSRasterizer::DrawScanlineEx<0x2445b464>);
	m_dsmap.SetAt(0x25403c84, &GSRasterizer::DrawScanlineEx<0x25403c84>);
	m_dsmap.SetAt(0x26102204, &GSRasterizer::DrawScanlineEx<0x26102204>);
	m_dsmap.SetAt(0x26803884, &GSRasterizer::DrawScanlineEx<0x26803884>);
	m_dsmap.SetAt(0x2681b464, &GSRasterizer::DrawScanlineEx<0x2681b464>);
	m_dsmap.SetAt(0x2681bc04, &GSRasterizer::DrawScanlineEx<0x2681bc04>);
	m_dsmap.SetAt(0x2681bc24, &GSRasterizer::DrawScanlineEx<0x2681bc24>);
	m_dsmap.SetAt(0x2681bc44, &GSRasterizer::DrawScanlineEx<0x2681bc44>);
	m_dsmap.SetAt(0x2681bc64, &GSRasterizer::DrawScanlineEx<0x2681bc64>);
	m_dsmap.SetAt(0x2a402205, &GSRasterizer::DrawScanlineEx<0x2a402205>);
	m_dsmap.SetAt(0x2a802c86, &GSRasterizer::DrawScanlineEx<0x2a802c86>);
	m_dsmap.SetAt(0x2ff03884, &GSRasterizer::DrawScanlineEx<0x2ff03884>);
	m_dsmap.SetAt(0x4ff02264, &GSRasterizer::DrawScanlineEx<0x4ff02264>);
	m_dsmap.SetAt(0x64402264, &GSRasterizer::DrawScanlineEx<0x64402264>);
	m_dsmap.SetAt(0x64a02214, &GSRasterizer::DrawScanlineEx<0x64a02214>);
	m_dsmap.SetAt(0x64a53464, &GSRasterizer::DrawScanlineEx<0x64a53464>);
	m_dsmap.SetAt(0x6884d464, &GSRasterizer::DrawScanlineEx<0x6884d464>);
	m_dsmap.SetAt(0x6ff20235, &GSRasterizer::DrawScanlineEx<0x6ff20235>);

	// suikoden 5

	m_dsmap.SetAt(0x20a02248, &GSRasterizer::DrawScanlineEx<0x20a02248>);
	m_dsmap.SetAt(0x20a1b448, &GSRasterizer::DrawScanlineEx<0x20a1b448>);
	m_dsmap.SetAt(0x2421b448, &GSRasterizer::DrawScanlineEx<0x2421b448>);
	m_dsmap.SetAt(0x26803448, &GSRasterizer::DrawScanlineEx<0x26803448>);
	m_dsmap.SetAt(0x60203468, &GSRasterizer::DrawScanlineEx<0x60203468>);
	m_dsmap.SetAt(0x60243468, &GSRasterizer::DrawScanlineEx<0x60243468>);
	m_dsmap.SetAt(0x6441a428, &GSRasterizer::DrawScanlineEx<0x6441a428>);
	m_dsmap.SetAt(0x64843468, &GSRasterizer::DrawScanlineEx<0x64843468>);
	m_dsmap.SetAt(0x66803468, &GSRasterizer::DrawScanlineEx<0x66803468>);

	// okami

	m_dsmap.SetAt(0x24403c88, &GSRasterizer::DrawScanlineEx<0x24403c88>);
	m_dsmap.SetAt(0x2440d428, &GSRasterizer::DrawScanlineEx<0x2440d428>);
	m_dsmap.SetAt(0x2480d428, &GSRasterizer::DrawScanlineEx<0x2480d428>);
	m_dsmap.SetAt(0x25403c08, &GSRasterizer::DrawScanlineEx<0x25403c08>);
	m_dsmap.SetAt(0x26203c88, &GSRasterizer::DrawScanlineEx<0x26203c88>);
	m_dsmap.SetAt(0x26403888, &GSRasterizer::DrawScanlineEx<0x26403888>);
	m_dsmap.SetAt(0x26403c88, &GSRasterizer::DrawScanlineEx<0x26403c88>);
	m_dsmap.SetAt(0x26803888, &GSRasterizer::DrawScanlineEx<0x26803888>);
	m_dsmap.SetAt(0x28903c88, &GSRasterizer::DrawScanlineEx<0x28903c88>);
	m_dsmap.SetAt(0x2ff02ca8, &GSRasterizer::DrawScanlineEx<0x2ff02ca8>);
	m_dsmap.SetAt(0x2ff03888, &GSRasterizer::DrawScanlineEx<0x2ff03888>);
	m_dsmap.SetAt(0x4ff02c18, &GSRasterizer::DrawScanlineEx<0x4ff02c18>);
	m_dsmap.SetAt(0x62902c18, &GSRasterizer::DrawScanlineEx<0x62902c18>);
	m_dsmap.SetAt(0x64403418, &GSRasterizer::DrawScanlineEx<0x64403418>);
	m_dsmap.SetAt(0x6440d418, &GSRasterizer::DrawScanlineEx<0x6440d418>);
	m_dsmap.SetAt(0x6440d428, &GSRasterizer::DrawScanlineEx<0x6440d428>);
	m_dsmap.SetAt(0x6440dc28, &GSRasterizer::DrawScanlineEx<0x6440dc28>);
	m_dsmap.SetAt(0x6444c268, &GSRasterizer::DrawScanlineEx<0x6444c268>);

	// guitar hero

	m_dsmap.SetAt(0x0ff038aa, &GSRasterizer::DrawScanlineEx<0x0ff038aa>);
	m_dsmap.SetAt(0x2440d46a, &GSRasterizer::DrawScanlineEx<0x2440d46a>);
	m_dsmap.SetAt(0x2442c24a, &GSRasterizer::DrawScanlineEx<0x2442c24a>);
	m_dsmap.SetAt(0x2442c26a, &GSRasterizer::DrawScanlineEx<0x2442c26a>);
	m_dsmap.SetAt(0x2442d44a, &GSRasterizer::DrawScanlineEx<0x2442d44a>);
	m_dsmap.SetAt(0x2442d46a, &GSRasterizer::DrawScanlineEx<0x2442d46a>);
	m_dsmap.SetAt(0x2446d46a, &GSRasterizer::DrawScanlineEx<0x2446d46a>);
	m_dsmap.SetAt(0x2480226a, &GSRasterizer::DrawScanlineEx<0x2480226a>);
	m_dsmap.SetAt(0x2480344a, &GSRasterizer::DrawScanlineEx<0x2480344a>);
	m_dsmap.SetAt(0x2480346a, &GSRasterizer::DrawScanlineEx<0x2480346a>);
	m_dsmap.SetAt(0x248034ea, &GSRasterizer::DrawScanlineEx<0x248034ea>);
	m_dsmap.SetAt(0x24803c0a, &GSRasterizer::DrawScanlineEx<0x24803c0a>);
	m_dsmap.SetAt(0x2480d46a, &GSRasterizer::DrawScanlineEx<0x2480d46a>);
	m_dsmap.SetAt(0x2680224a, &GSRasterizer::DrawScanlineEx<0x2680224a>);
	m_dsmap.SetAt(0x2680226a, &GSRasterizer::DrawScanlineEx<0x2680226a>);
	m_dsmap.SetAt(0x2680344a, &GSRasterizer::DrawScanlineEx<0x2680344a>);
	m_dsmap.SetAt(0x268034ea, &GSRasterizer::DrawScanlineEx<0x268034ea>);
	m_dsmap.SetAt(0x2680356a, &GSRasterizer::DrawScanlineEx<0x2680356a>);
	m_dsmap.SetAt(0x2680390a, &GSRasterizer::DrawScanlineEx<0x2680390a>);
	m_dsmap.SetAt(0x26803c2a, &GSRasterizer::DrawScanlineEx<0x26803c2a>);
	m_dsmap.SetAt(0x2680d44a, &GSRasterizer::DrawScanlineEx<0x2680d44a>);
	m_dsmap.SetAt(0x2684356a, &GSRasterizer::DrawScanlineEx<0x2684356a>);
	m_dsmap.SetAt(0x2688356a, &GSRasterizer::DrawScanlineEx<0x2688356a>);
	m_dsmap.SetAt(0x2a80390a, &GSRasterizer::DrawScanlineEx<0x2a80390a>);
	m_dsmap.SetAt(0x4ff0347a, &GSRasterizer::DrawScanlineEx<0x4ff0347a>);
	m_dsmap.SetAt(0x4ff034ea, &GSRasterizer::DrawScanlineEx<0x4ff034ea>);
	m_dsmap.SetAt(0x4ff034fa, &GSRasterizer::DrawScanlineEx<0x4ff034fa>);
	m_dsmap.SetAt(0x4ff03c9a, &GSRasterizer::DrawScanlineEx<0x4ff03c9a>);
	m_dsmap.SetAt(0x4ff0d47a, &GSRasterizer::DrawScanlineEx<0x4ff0d47a>);
	m_dsmap.SetAt(0x6440d45a, &GSRasterizer::DrawScanlineEx<0x6440d45a>);
	m_dsmap.SetAt(0x6442c27a, &GSRasterizer::DrawScanlineEx<0x6442c27a>);
	m_dsmap.SetAt(0x6442d45a, &GSRasterizer::DrawScanlineEx<0x6442d45a>);
	m_dsmap.SetAt(0x6442d46a, &GSRasterizer::DrawScanlineEx<0x6442d46a>);
	m_dsmap.SetAt(0x6442d47a, &GSRasterizer::DrawScanlineEx<0x6442d47a>);
	m_dsmap.SetAt(0x6444d47a, &GSRasterizer::DrawScanlineEx<0x6444d47a>);
	m_dsmap.SetAt(0x6446c27a, &GSRasterizer::DrawScanlineEx<0x6446c27a>);
	m_dsmap.SetAt(0x6446d47a, &GSRasterizer::DrawScanlineEx<0x6446d47a>);
	m_dsmap.SetAt(0x644ad47a, &GSRasterizer::DrawScanlineEx<0x644ad47a>);
	m_dsmap.SetAt(0x6480345a, &GSRasterizer::DrawScanlineEx<0x6480345a>);

	// virtual tennis 2

	m_dsmap.SetAt(0x0ff10215, &GSRasterizer::DrawScanlineEx<0x0ff10215>);
	m_dsmap.SetAt(0x24231065, &GSRasterizer::DrawScanlineEx<0x24231065>);
	m_dsmap.SetAt(0x2680f445, &GSRasterizer::DrawScanlineEx<0x2680f445>);
	m_dsmap.SetAt(0x28830215, &GSRasterizer::DrawScanlineEx<0x28830215>);
	m_dsmap.SetAt(0x2aa30215, &GSRasterizer::DrawScanlineEx<0x2aa30215>);
	m_dsmap.SetAt(0x4ff0e265, &GSRasterizer::DrawScanlineEx<0x4ff0e265>);
	m_dsmap.SetAt(0x4ff0f475, &GSRasterizer::DrawScanlineEx<0x4ff0f475>);
	m_dsmap.SetAt(0x4ff20215, &GSRasterizer::DrawScanlineEx<0x4ff20215>);
	m_dsmap.SetAt(0x6440e265, &GSRasterizer::DrawScanlineEx<0x6440e265>);
	m_dsmap.SetAt(0x6440ec65, &GSRasterizer::DrawScanlineEx<0x6440ec65>);
	m_dsmap.SetAt(0x6440ede5, &GSRasterizer::DrawScanlineEx<0x6440ede5>);
	m_dsmap.SetAt(0x6440f465, &GSRasterizer::DrawScanlineEx<0x6440f465>);
	m_dsmap.SetAt(0x6440f475, &GSRasterizer::DrawScanlineEx<0x6440f475>);
	m_dsmap.SetAt(0x6480e265, &GSRasterizer::DrawScanlineEx<0x6480e265>);
	m_dsmap.SetAt(0x66402c15, &GSRasterizer::DrawScanlineEx<0x66402c15>);
	m_dsmap.SetAt(0x6640f475, &GSRasterizer::DrawScanlineEx<0x6640f475>);
	m_dsmap.SetAt(0x6680f475, &GSRasterizer::DrawScanlineEx<0x6680f475>);

	// one piece grand battle 3

	m_dsmap.SetAt(0x0ff02c86, &GSRasterizer::DrawScanlineEx<0x0ff02c86>);
	m_dsmap.SetAt(0x24403444, &GSRasterizer::DrawScanlineEx<0x24403444>);
	m_dsmap.SetAt(0x2440c264, &GSRasterizer::DrawScanlineEx<0x2440c264>);
	m_dsmap.SetAt(0x25602204, &GSRasterizer::DrawScanlineEx<0x25602204>);
	m_dsmap.SetAt(0x26202c84, &GSRasterizer::DrawScanlineEx<0x26202c84>);
	m_dsmap.SetAt(0x26204c84, &GSRasterizer::DrawScanlineEx<0x26204c84>);
	m_dsmap.SetAt(0x2990cc84, &GSRasterizer::DrawScanlineEx<0x2990cc84>);
	m_dsmap.SetAt(0x60a03464, &GSRasterizer::DrawScanlineEx<0x60a03464>);
	m_dsmap.SetAt(0x64482254, &GSRasterizer::DrawScanlineEx<0x64482254>);
	m_dsmap.SetAt(0x64483454, &GSRasterizer::DrawScanlineEx<0x64483454>);
	m_dsmap.SetAt(0x66220214, &GSRasterizer::DrawScanlineEx<0x66220214>);
	m_dsmap.SetAt(0x6ff03454, &GSRasterizer::DrawScanlineEx<0x6ff03454>);
	m_dsmap.SetAt(0x65402254, &GSRasterizer::DrawScanlineEx<0x65402254>);
	m_dsmap.SetAt(0x6ff02254, &GSRasterizer::DrawScanlineEx<0x6ff02254>);
/*

	// dmc (fixme)

	// mgs3s1

	// nfs mw

	// wild arms 5

	// rouge galaxy

	// God of War

	// dbzbt2

*/
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
	const DWORD abe = (sel >> 20) & 255;
	const DWORD abea = (sel >> 20) & 3;
	const DWORD abeb = (sel >> 22) & 3;
	const DWORD abec = (sel >> 24) & 3;
	const DWORD abed = (sel >> 26) & 3;
	const DWORD pabe = (sel >> 28) & 1;
	const DWORD rfb = (sel >> 29) & 1;
	const DWORD wzb = (sel >> 30) & 1;

	GSVector4i fa_base = m_slenv.fbr[top];
	GSVector4i* fa_offset = (GSVector4i*)&m_slenv.fbc[left & 3][left];

	GSVector4i za_base = m_slenv.zbr[top];
	GSVector4i* za_offset = (GSVector4i*)&m_slenv.zbc[left & 3][left];

	GSVector4 ps0123 = GSVector4::ps0123();

	GSVector4 vp = v.p;
	GSVector4 dp = m_slenv.dp;
	GSVector4 z = vp.zzzz(); z += dp.zzzz() * ps0123;
	GSVector4 f = vp.wwww(); f += dp.wwww() * ps0123;

	GSVector4 vt = v.t;
	GSVector4 dt = m_slenv.dt;
	GSVector4 s = vt.xxxx(); s += dt.xxxx() * ps0123;
	GSVector4 t = vt.yyyy(); t += dt.yyyy() * ps0123;
	GSVector4 q = vt.zzzz(); q += dt.zzzz() * ps0123;

	GSVector4 vc = v.c;
	GSVector4 dc = m_slenv.dc;
	GSVector4 r = vc.xxxx(); if(iip) r += dc.xxxx() * ps0123;
	GSVector4 g = vc.yyyy(); if(iip) g += dc.yyyy() * ps0123;
	GSVector4 b = vc.zzzz(); if(iip) b += dc.zzzz() * ps0123;
	GSVector4 a = vc.wwww(); if(iip) a += dc.wwww() * ps0123;

	int steps = right - left;

	m_slenv.steps += steps;

	while(1)
	{
		do
		{

		GSVector4i za = za_base + GSVector4i::load<true>(za_offset);
		
		GSVector4i zs = (GSVector4i(z * 0.5f) << 1) | (GSVector4i(z) & GSVector4i::one(za));

		GSVector4i test;

		if(!TestZ(zpsm, ztst, zs, za, test))
		{
			continue;
		}

		// DWORD mask = (DWORD)(((int)steps - 4) >> 31);
		// int pixels = (steps & mask) | (4 & ~mask);

		int pixels = GSVector4i::store(GSVector4i::load(steps).min_i16(GSVector4i::load(4)));

		GSVector4 c[12];

		if(tfx != TFX_NONE)
		{
			GSVector4 u = s;
			GSVector4 v = t;

			if(!fst)
			{
				GSVector4 w = q.rcp();

				u *= w;
				v *= w;

				if(ltf)
				{
					u -= 0.5f;
					v -= 0.5f;
				}
			}

			SampleTexture(ztst, test, pixels, ltf, u, v, c);
		}

		AlphaTFX(tfx, tcc, a, c[3]);

		GSVector4i fm = m_slenv.fm;
		GSVector4i zm = m_slenv.zm;

		if(!TestAlpha(atst, afail, c[3], fm, zm, test))
		{
			continue;
		}

		ColorTFX(tfx, r, g, b, a, c[0], c[1], c[2]);

		if(fge)
		{
			Fog(f, c[0], c[1], c[2]);
		}

		GSVector4i fa = fa_base + GSVector4i::load<true>(fa_offset);

		GSVector4i d = GSVector4i::zero();

		if(rfb)
		{
			d = m_state->m_mem.ReadFrameX(fpsm == 1 ? 0 : fpsm, fa);

			if(fpsm != 1 && date)
			{
				test |= (d ^ m_slenv.datm).sra32(31);

				if(test.alltrue())
				{
					continue;
				}
			}
		}

		fm |= test;
		zm |= test;

		if(abe != 255)
		{
//			GSVector4::expand(d, c[4], c[5], c[6], c[7]);

			c[4] = (d << 24) >> 24;
			c[5] = (d << 16) >> 24;
			c[6] = (d <<  8) >> 24;
			c[7] = (d >> 24);

			if(fpsm == 1)
			{
				c[7] = GSVector4(128.0f);
			}

			c[8] = GSVector4::zero();
			c[9] = GSVector4::zero();
			c[10] = GSVector4::zero();
			c[11] = m_slenv.afix;

			/*
			GSVector4 r = (c[abea*4 + 0] - c[abeb*4 + 0]).mod2x(c[abec*4 + 3]) + c[abed*4 + 0];
			GSVector4 g = (c[abea*4 + 1] - c[abeb*4 + 1]).mod2x(c[abec*4 + 3]) + c[abed*4 + 1];
			GSVector4 b = (c[abea*4 + 2] - c[abeb*4 + 2]).mod2x(c[abec*4 + 3]) + c[abed*4 + 2];
			*/

			GSVector4 r, g, b; 

			if(abea != abeb)
			{
				r = c[abea*4 + 0];
				g = c[abea*4 + 1];
				b = c[abea*4 + 2];

				if(abeb != 2)
				{
					r -= c[abeb*4 + 0];
					g -= c[abeb*4 + 1];
					b -= c[abeb*4 + 2];
				}

				if(!(fpsm == 1 && abec == 1))
				{
					if(abec == 2)
					{
						r *= m_slenv.afix2;
						g *= m_slenv.afix2;
						b *= m_slenv.afix2;
					}
					else
					{
						r = r.mod2x(c[abec*4 + 3]);
						g = g.mod2x(c[abec*4 + 3]);
						b = b.mod2x(c[abec*4 + 3]);
					}
				}

				if(abed < 2)
				{
					r += c[abed*4 + 0];
					g += c[abed*4 + 1];
					b += c[abed*4 + 2];
				}
			}
			else
			{
				r = c[abed*4 + 0];
				g = c[abed*4 + 1];
				b = c[abed*4 + 2];
			}

			if(pabe)
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
		
		GSVector4i s = rg.upl32(ba).pu16(rg.uph32(ba));

		if(fpsm != 1)
		{
			s |= m_slenv.fba;
		}

		if(rfb)
		{
			s = s.blend(d, fm);
		}

		m_state->m_mem.WriteFrameAndZBufX(fpsm == 1 && rfb ? 0 : fpsm, fa, fm, s, wzb ? zpsm : 3, za, zm, zs, pixels);

		}
		while(0);

		if(steps <= 4) break;

		steps -= 4;

		fa_offset++;
		za_offset++;

		GSVector4 dp4 = m_slenv.dp4;

		z += dp4.zzzz();
		f += dp4.wwww();

		GSVector4 dt4 = m_slenv.dt4;

		s += dt4.xxxx();
		t += dt4.yyyy();
		q += dt4.zzzz();

		GSVector4 dc4 = m_slenv.dc4;

		if(iip) r += dc4.xxxx();
		if(iip) g += dc4.yyyy();
		if(iip) b += dc4.zzzz();
		if(iip) a += dc4.wwww();
	}
}
