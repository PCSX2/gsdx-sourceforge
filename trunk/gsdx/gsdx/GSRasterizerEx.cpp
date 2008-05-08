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
	m_dsmap[0x00204408] = &GSRasterizer::DrawScanlineEx<0x00204408>;
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

	ScanlineEnvironment* slenv = m_slenv;

	int fpsm = GSLocalMemory::DecodeFPSM(((sel >> 0) & 7));
	int zpsm = GSLocalMemory::DecodeZPSM(((sel >> 3) & 3));

	__m128i fa_base = m_fbco->addr[top];
	__m128i* fa_offset = (__m128i*)&m_state->m_context->ftbl->rowOffset[top & 7][left];

	__m128i za_base = m_zbco->addr[top];
	__m128i* za_offset = (__m128i*)&m_state->m_context->ztbl->rowOffset[top & 7][left];

	Vector vp = v.p;
	Vector z = vp.zzzz(); z += slenv->dz0123;
	Vector f = vp.wwww(); f += slenv->df0123;

	Vector vt = v.t;
	Vector s = vt.xxxx(); s += slenv->ds0123;
	Vector t = vt.yyyy(); t += slenv->dt0123;
	Vector q = vt.zzzz(); q += slenv->dq0123;

	Vector vc = v.c;
	Vector r = vc.xxxx(); if(iip) r += slenv->dr0123;
	Vector g = vc.yyyy(); if(iip) g += slenv->dg0123;
	Vector b = vc.zzzz(); if(iip) b += slenv->db0123;
	Vector a = vc.wwww(); if(iip) a += slenv->da0123;

	for(int steps = right - left; steps > 0; steps -= 4)
	{
		do
		{

		int pixels = min(steps, 4);

		__m128i fa = _mm_add_epi32(fa_base, _mm_loadu_si128(fa_offset));
		__m128i za = _mm_add_epi32(za_base, _mm_loadu_si128(za_offset));
		
		__m128i fm =  slenv->fm;
		__m128i zm =  slenv->zm;
		__m128i test = _mm_setzero_si128();
		
		__m128i zi = _mm_or_si128(_mm_slli_epi32(z * 0.5f, 1), _mm_and_si128(z, _mm_set1_epi32(1)));

		if(ztst > 1)
		{
			__m128i zd0123;

			// TODO: benchmark this, it may not be faster, but compiles to only 3 instructions per line for a 32 bit z buffer (extract, mov, insert)

			#if 0 // _M_SSE >= 0x400

			zd0123 = _mm_insert_epi32(zd0123, m_state->m_mem.readPixelX(zpsm, _mm_extract_epi32(za, 0)), 0);
			zd0123 = _mm_insert_epi32(zd0123, m_state->m_mem.readPixelX(zpsm, _mm_extract_epi32(za, 1)), 1);
			zd0123 = _mm_insert_epi32(zd0123, m_state->m_mem.readPixelX(zpsm, _mm_extract_epi32(za, 2)), 2);
			zd0123 = _mm_insert_epi32(zd0123, m_state->m_mem.readPixelX(zpsm, _mm_extract_epi32(za, 3)), 3);

			#else

			for(int i = 0; i < pixels; i++)
			{
				zd0123.m128i_u32[i] = m_state->m_mem.readPixelX(zpsm, za.m128i_u32[i]);
			}

			#endif

			__m128i zs = _mm_sub_epi32(zi, _mm_set1_epi32(0x80000000));
			__m128i zd = _mm_sub_epi32(zd0123, _mm_set1_epi32(0x80000000));

			switch(ztst)
			{
			case 2: test = _mm_cmplt_epi32(zs, zd); break; // ge
			case 3: test = _mm_or_si128(_mm_cmplt_epi32(zs, zd), _mm_cmpeq_epi32(zs, zd)); break; // g
			default: __assume(0);
			}

			if(_mm_movemask_epi8(test) == 0xffff)
			{
				continue;
			}
		}

		Vector c[12];

		if(tfx < 4)
		{
			Vector u = s;
			Vector v = t;

			if(!fst)
			{
				Vector w = q.rcp();

				u *= w;
				v *= w;
			}

			if(ltf)
			{
				u -= 0.5f;
				v -= 0.5f;

				Vector uf = u.floor();
				Vector vf = v.floor();
				
				Vector uff = u - uf;
				Vector vff = v - vf;

				__m128i uv = _mm_packs_epi32(uf, vf);
				__m128i uv0 = Wrap(uv);
				__m128i uv1 = Wrap(_mm_add_epi16(uv, _mm_set1_epi32(0x00010001)));

				for(int i = 0; i < pixels; i++)
				{
					if(ztst > 1 && test.m128i_u32[i])
					{
						continue;
					}

					FetchTexel(uv0.m128i_u16[i], uv0.m128i_u16[i + 4]);
					FetchTexel(uv1.m128i_u16[i], uv0.m128i_u16[i + 4]);
					FetchTexel(uv0.m128i_u16[i], uv1.m128i_u16[i + 4]);
					FetchTexel(uv1.m128i_u16[i], uv1.m128i_u16[i + 4]);

					Vector c00(ReadTexelNoFetch(uv0.m128i_u16[i], uv0.m128i_u16[i + 4]));
					Vector c01(ReadTexelNoFetch(uv1.m128i_u16[i], uv0.m128i_u16[i + 4]));
					Vector c10(ReadTexelNoFetch(uv0.m128i_u16[i], uv1.m128i_u16[i + 4]));
					Vector c11(ReadTexelNoFetch(uv1.m128i_u16[i], uv1.m128i_u16[i + 4]));

					c00 = c00.lerp(c01, uff.v[i]);
					c10 = c10.lerp(c11, uff.v[i]);
					c00 = c00.lerp(c10, vff.v[i]);

					c[i] = c00;
				}

				Vector::transpose(c[0], c[1], c[2], c[3]);
			}
			else
			{
				__m128i uv = _mm_packs_epi32(u, v);
				__m128i uv0 = Wrap(uv);

				__m128i c00;

				for(int i = 0; i < pixels; i++)
				{
					if(ztst > 1 && test.m128i_u32[i])
					{
						continue;
					}

					c00.m128i_u32[i] = ReadTexel(uv0.m128i_u16[i], uv0.m128i_u16[i + 4]);
				}

				Vector::expand(c00, c[0], c[1], c[2], c[3]);
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
			__m128i t;

			switch(atst)
			{
			case 0: t = _mm_set1_epi32(0xffffffff); break; // never 
			case 1: t = _mm_setzero_si128(); break; // always
			case 2: case 3: t = _mm_cmpgt_epi32(c[3], slenv->aref); break; // l, le
			case 4: t = _mm_xor_si128(_mm_cmpeq_epi32(c[3], slenv->aref), _mm_set1_epi32(0xffffffff)); break; // e
			case 5: case 6: t = _mm_cmplt_epi32(c[3], slenv->aref); break; // ge, g
			case 7: t = _mm_cmpeq_epi32(c[3], slenv->aref); break; // ne 
			default: __assume(0);
			}		

			switch(afail)
			{
			case 0:
				fm = _mm_or_si128(fm, t);
				zm = _mm_or_si128(zm, t);
				test = _mm_or_si128(test, t);
				if(_mm_movemask_epi8(test) == 0xffff) continue;
				break;
			case 1:
				zm = _mm_or_si128(zm, t);
				break;
			case 2:
				fm = _mm_or_si128(fm, t);
				break;
			case 3: 
				fm = _mm_or_si128(fm, _mm_and_si128(t, _mm_set1_epi32(0xff000000)));
				zm = _mm_or_si128(zm, t);
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
			c[0] = slenv->f.r.lerp(c[0], f);
			c[1] = slenv->f.g.lerp(c[1], f);
			c[2] = slenv->f.b.lerp(c[2], f);
		}

		__m128i d = _mm_setzero_si128();

		if(rfb)
		{
			for(int i = 0; i < pixels; i++)
			{
				if(test.m128i_u32[i])
				{
					continue;
				}

				d.m128i_u32[i] = m_state->m_mem.readFrameX(fpsm, fa.m128i_u32[i]);
			}
		}

		if(date)
		{
			test = _mm_or_si128(test, _mm_srai_epi32(_mm_xor_si128(d, slenv->datm), 31));

			if(_mm_movemask_epi8(test) == 0xffff)
			{
				continue;
			}
		}

		fm = _mm_or_si128(fm, test);
		zm = _mm_or_si128(zm, test);

		if(abe)
		{
			Vector::expand(d, c[4], c[5], c[6], c[7]);

			c[8] = Vector::zero();
			c[9] = Vector::zero();
			c[10] = Vector::zero();
			c[11] = slenv->afix;

			Vector r = (c[abea*4 + 0] - c[abeb*4 + 0]).mod2x(c[abec*4 + 3]) + c[abed*4 + 0];
			Vector g = (c[abea*4 + 1] - c[abeb*4 + 1]).mod2x(c[abec*4 + 3]) + c[abed*4 + 1];
			Vector b = (c[abea*4 + 2] - c[abeb*4 + 2]).mod2x(c[abec*4 + 3]) + c[abed*4 + 2];

			if(abe == 2)
			{
				Vector mask = c[3] >= Vector(128.0f);

				c[0] = c[0].blend(r, mask);
				c[1] = c[1].blend(g, mask);
				c[2] = c[2].blend(b, mask);
			}
			else
			{
				c[0] = r;
				c[1] = g;
				c[2] = b;
			}
		}

		__m128i rb = _mm_packs_epi32(c[0], c[2]);
		__m128i ga = _mm_packs_epi32(c[1], c[3]);
		
		__m128i rg = _mm_and_si128(_mm_unpacklo_epi16(rb, ga), slenv->colclamp);
		__m128i ba = _mm_and_si128(_mm_unpackhi_epi16(rb, ga), slenv->colclamp);
		
		__m128i s = _mm_or_si128(_mm_packus_epi16(_mm_unpacklo_epi32(rg, ba), _mm_unpackhi_epi32(rg, ba)), slenv->fba);

		s = _mm_or_si128(_mm_andnot_si128(fm, s), _mm_and_si128(fm, d));

		for(int i = 0; i < pixels; i++)
		{
			if(fm.m128i_u32[i] != 0xffffffff)
			{
				m_state->m_mem.writeFrameX(fpsm, fa.m128i_u32[i], s.m128i_u32[i]);
			}

			if(ztst > 0 && zm.m128i_u32[i] != 0xffffffff)
			{
				m_state->m_mem.writePixelX(zpsm, za.m128i_u32[i], zi.m128i_u32[i]);
			}
		}

		}
		while(0);

		fa_offset++;
		za_offset++;
		z += slenv->dz;
		f += slenv->df;
		s += slenv->ds;
		t += slenv->dt;
		q += slenv->dq;
		if(iip) r += slenv->dr;
		if(iip) g += slenv->dg;
		if(iip) b += slenv->db;
		if(iip) a += slenv->da;
	}
}
