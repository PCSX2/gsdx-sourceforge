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
#include "GPURasterizer.h"

GPURasterizer::GPURasterizer(GPUState* state, int id, int threads)
	: m_state(state)
	, m_id(id)
	, m_threads(threads)
{
	memset(m_ds, 0, sizeof(m_ds));

	for(int i = 0; i < countof(m_ds); i++)
	{
		m_ds[i] = &GPURasterizer::DrawScanline;
	}

	m_ds[0x00] = &GPURasterizer::DrawScanlineEx<0x00>;
	m_ds[0x01] = &GPURasterizer::DrawScanlineEx<0x01>;
	m_ds[0x02] = &GPURasterizer::DrawScanlineEx<0x02>;
	m_ds[0x03] = &GPURasterizer::DrawScanlineEx<0x03>;
	m_ds[0x04] = &GPURasterizer::DrawScanlineEx<0x04>;
	m_ds[0x05] = &GPURasterizer::DrawScanlineEx<0x05>;
	m_ds[0x06] = &GPURasterizer::DrawScanlineEx<0x06>;
	m_ds[0x07] = &GPURasterizer::DrawScanlineEx<0x07>;
	m_ds[0x08] = &GPURasterizer::DrawScanlineEx<0x08>;
	m_ds[0x09] = &GPURasterizer::DrawScanlineEx<0x09>;
	m_ds[0x0a] = &GPURasterizer::DrawScanlineEx<0x0a>;
	m_ds[0x0b] = &GPURasterizer::DrawScanlineEx<0x0b>;
	m_ds[0x0c] = &GPURasterizer::DrawScanlineEx<0x0c>;
	m_ds[0x0d] = &GPURasterizer::DrawScanlineEx<0x0d>;
	m_ds[0x0e] = &GPURasterizer::DrawScanlineEx<0x0e>;
	m_ds[0x0f] = &GPURasterizer::DrawScanlineEx<0x0f>;
	m_ds[0x10] = &GPURasterizer::DrawScanlineEx<0x10>;
	m_ds[0x11] = &GPURasterizer::DrawScanlineEx<0x11>;
	m_ds[0x12] = &GPURasterizer::DrawScanlineEx<0x12>;
	m_ds[0x13] = &GPURasterizer::DrawScanlineEx<0x13>;
	m_ds[0x14] = &GPURasterizer::DrawScanlineEx<0x14>;
	m_ds[0x15] = &GPURasterizer::DrawScanlineEx<0x15>;
	m_ds[0x16] = &GPURasterizer::DrawScanlineEx<0x16>;
	m_ds[0x17] = &GPURasterizer::DrawScanlineEx<0x17>;
	m_ds[0x18] = &GPURasterizer::DrawScanlineEx<0x18>;
	m_ds[0x19] = &GPURasterizer::DrawScanlineEx<0x19>;
	m_ds[0x1a] = &GPURasterizer::DrawScanlineEx<0x1a>;
	m_ds[0x1b] = &GPURasterizer::DrawScanlineEx<0x1b>;
	m_ds[0x1c] = &GPURasterizer::DrawScanlineEx<0x1c>;
	m_ds[0x1d] = &GPURasterizer::DrawScanlineEx<0x1d>;
	m_ds[0x1e] = &GPURasterizer::DrawScanlineEx<0x1e>;
	m_ds[0x1f] = &GPURasterizer::DrawScanlineEx<0x1f>;
	m_ds[0x20] = &GPURasterizer::DrawScanlineEx<0x20>;
	m_ds[0x21] = &GPURasterizer::DrawScanlineEx<0x21>;
	m_ds[0x22] = &GPURasterizer::DrawScanlineEx<0x22>;
	m_ds[0x23] = &GPURasterizer::DrawScanlineEx<0x23>;
	m_ds[0x24] = &GPURasterizer::DrawScanlineEx<0x24>;
	m_ds[0x25] = &GPURasterizer::DrawScanlineEx<0x25>;
	m_ds[0x26] = &GPURasterizer::DrawScanlineEx<0x26>;
	m_ds[0x27] = &GPURasterizer::DrawScanlineEx<0x27>;
	m_ds[0x28] = &GPURasterizer::DrawScanlineEx<0x28>;
	m_ds[0x29] = &GPURasterizer::DrawScanlineEx<0x29>;
	m_ds[0x2a] = &GPURasterizer::DrawScanlineEx<0x2a>;
	m_ds[0x2b] = &GPURasterizer::DrawScanlineEx<0x2b>;
	m_ds[0x2c] = &GPURasterizer::DrawScanlineEx<0x2c>;
	m_ds[0x2d] = &GPURasterizer::DrawScanlineEx<0x2d>;
	m_ds[0x2e] = &GPURasterizer::DrawScanlineEx<0x2e>;
	m_ds[0x2f] = &GPURasterizer::DrawScanlineEx<0x2f>;
	m_ds[0x30] = &GPURasterizer::DrawScanlineEx<0x30>;
	m_ds[0x31] = &GPURasterizer::DrawScanlineEx<0x31>;
	m_ds[0x32] = &GPURasterizer::DrawScanlineEx<0x32>;
	m_ds[0x33] = &GPURasterizer::DrawScanlineEx<0x33>;
	m_ds[0x34] = &GPURasterizer::DrawScanlineEx<0x34>;
	m_ds[0x35] = &GPURasterizer::DrawScanlineEx<0x35>;
	m_ds[0x36] = &GPURasterizer::DrawScanlineEx<0x36>;
	m_ds[0x37] = &GPURasterizer::DrawScanlineEx<0x37>;
	m_ds[0x38] = &GPURasterizer::DrawScanlineEx<0x38>;
	m_ds[0x39] = &GPURasterizer::DrawScanlineEx<0x39>;
	m_ds[0x3a] = &GPURasterizer::DrawScanlineEx<0x3a>;
	m_ds[0x3b] = &GPURasterizer::DrawScanlineEx<0x3b>;
	m_ds[0x3c] = &GPURasterizer::DrawScanlineEx<0x3c>;
	m_ds[0x3d] = &GPURasterizer::DrawScanlineEx<0x3d>;
	m_ds[0x3e] = &GPURasterizer::DrawScanlineEx<0x3e>;
	m_ds[0x3f] = &GPURasterizer::DrawScanlineEx<0x3f>;
	m_ds[0x40] = &GPURasterizer::DrawScanlineEx<0x40>;
	m_ds[0x41] = &GPURasterizer::DrawScanlineEx<0x41>;
	m_ds[0x42] = &GPURasterizer::DrawScanlineEx<0x42>;
	m_ds[0x43] = &GPURasterizer::DrawScanlineEx<0x43>;
	m_ds[0x44] = &GPURasterizer::DrawScanlineEx<0x44>;
	m_ds[0x45] = &GPURasterizer::DrawScanlineEx<0x45>;
	m_ds[0x46] = &GPURasterizer::DrawScanlineEx<0x46>;
	m_ds[0x47] = &GPURasterizer::DrawScanlineEx<0x47>;
	m_ds[0x48] = &GPURasterizer::DrawScanlineEx<0x48>;
	m_ds[0x49] = &GPURasterizer::DrawScanlineEx<0x49>;
	m_ds[0x4a] = &GPURasterizer::DrawScanlineEx<0x4a>;
	m_ds[0x4b] = &GPURasterizer::DrawScanlineEx<0x4b>;
	m_ds[0x4c] = &GPURasterizer::DrawScanlineEx<0x4c>;
	m_ds[0x4d] = &GPURasterizer::DrawScanlineEx<0x4d>;
	m_ds[0x4e] = &GPURasterizer::DrawScanlineEx<0x4e>;
	m_ds[0x4f] = &GPURasterizer::DrawScanlineEx<0x4f>;
	m_ds[0x50] = &GPURasterizer::DrawScanlineEx<0x50>;
	m_ds[0x51] = &GPURasterizer::DrawScanlineEx<0x51>;
	m_ds[0x52] = &GPURasterizer::DrawScanlineEx<0x52>;
	m_ds[0x53] = &GPURasterizer::DrawScanlineEx<0x53>;
	m_ds[0x54] = &GPURasterizer::DrawScanlineEx<0x54>;
	m_ds[0x55] = &GPURasterizer::DrawScanlineEx<0x55>;
	m_ds[0x56] = &GPURasterizer::DrawScanlineEx<0x56>;
	m_ds[0x57] = &GPURasterizer::DrawScanlineEx<0x57>;
	m_ds[0x58] = &GPURasterizer::DrawScanlineEx<0x58>;
	m_ds[0x59] = &GPURasterizer::DrawScanlineEx<0x59>;
	m_ds[0x5a] = &GPURasterizer::DrawScanlineEx<0x5a>;
	m_ds[0x5b] = &GPURasterizer::DrawScanlineEx<0x5b>;
	m_ds[0x5c] = &GPURasterizer::DrawScanlineEx<0x5c>;
	m_ds[0x5d] = &GPURasterizer::DrawScanlineEx<0x5d>;
	m_ds[0x5e] = &GPURasterizer::DrawScanlineEx<0x5e>;
	m_ds[0x5f] = &GPURasterizer::DrawScanlineEx<0x5f>;
	m_ds[0x60] = &GPURasterizer::DrawScanlineEx<0x60>;
	m_ds[0x61] = &GPURasterizer::DrawScanlineEx<0x61>;
	m_ds[0x62] = &GPURasterizer::DrawScanlineEx<0x62>;
	m_ds[0x63] = &GPURasterizer::DrawScanlineEx<0x63>;
	m_ds[0x64] = &GPURasterizer::DrawScanlineEx<0x64>;
	m_ds[0x65] = &GPURasterizer::DrawScanlineEx<0x65>;
	m_ds[0x66] = &GPURasterizer::DrawScanlineEx<0x66>;
	m_ds[0x67] = &GPURasterizer::DrawScanlineEx<0x67>;
	m_ds[0x68] = &GPURasterizer::DrawScanlineEx<0x68>;
	m_ds[0x69] = &GPURasterizer::DrawScanlineEx<0x69>;
	m_ds[0x6a] = &GPURasterizer::DrawScanlineEx<0x6a>;
	m_ds[0x6b] = &GPURasterizer::DrawScanlineEx<0x6b>;
	m_ds[0x6c] = &GPURasterizer::DrawScanlineEx<0x6c>;
	m_ds[0x6d] = &GPURasterizer::DrawScanlineEx<0x6d>;
	m_ds[0x6e] = &GPURasterizer::DrawScanlineEx<0x6e>;
	m_ds[0x6f] = &GPURasterizer::DrawScanlineEx<0x6f>;
	m_ds[0x70] = &GPURasterizer::DrawScanlineEx<0x70>;
	m_ds[0x71] = &GPURasterizer::DrawScanlineEx<0x71>;
	m_ds[0x72] = &GPURasterizer::DrawScanlineEx<0x72>;
	m_ds[0x73] = &GPURasterizer::DrawScanlineEx<0x73>;
	m_ds[0x74] = &GPURasterizer::DrawScanlineEx<0x74>;
	m_ds[0x75] = &GPURasterizer::DrawScanlineEx<0x75>;
	m_ds[0x76] = &GPURasterizer::DrawScanlineEx<0x76>;
	m_ds[0x77] = &GPURasterizer::DrawScanlineEx<0x77>;
	m_ds[0x78] = &GPURasterizer::DrawScanlineEx<0x78>;
	m_ds[0x79] = &GPURasterizer::DrawScanlineEx<0x79>;
	m_ds[0x7a] = &GPURasterizer::DrawScanlineEx<0x7a>;
	m_ds[0x7b] = &GPURasterizer::DrawScanlineEx<0x7b>;
	m_ds[0x7c] = &GPURasterizer::DrawScanlineEx<0x7c>;
	m_ds[0x7d] = &GPURasterizer::DrawScanlineEx<0x7d>;
	m_ds[0x7e] = &GPURasterizer::DrawScanlineEx<0x7e>;
	m_ds[0x7f] = &GPURasterizer::DrawScanlineEx<0x7f>;
	m_ds[0x80] = &GPURasterizer::DrawScanlineEx<0x80>;
	m_ds[0x81] = &GPURasterizer::DrawScanlineEx<0x81>;
	m_ds[0x82] = &GPURasterizer::DrawScanlineEx<0x82>;
	m_ds[0x83] = &GPURasterizer::DrawScanlineEx<0x83>;
	m_ds[0x84] = &GPURasterizer::DrawScanlineEx<0x84>;
	m_ds[0x85] = &GPURasterizer::DrawScanlineEx<0x85>;
	m_ds[0x86] = &GPURasterizer::DrawScanlineEx<0x86>;
	m_ds[0x87] = &GPURasterizer::DrawScanlineEx<0x87>;
	m_ds[0x88] = &GPURasterizer::DrawScanlineEx<0x88>;
	m_ds[0x89] = &GPURasterizer::DrawScanlineEx<0x89>;
	m_ds[0x8a] = &GPURasterizer::DrawScanlineEx<0x8a>;
	m_ds[0x8b] = &GPURasterizer::DrawScanlineEx<0x8b>;
	m_ds[0x8c] = &GPURasterizer::DrawScanlineEx<0x8c>;
	m_ds[0x8d] = &GPURasterizer::DrawScanlineEx<0x8d>;
	m_ds[0x8e] = &GPURasterizer::DrawScanlineEx<0x8e>;
	m_ds[0x8f] = &GPURasterizer::DrawScanlineEx<0x8f>;
	m_ds[0x90] = &GPURasterizer::DrawScanlineEx<0x90>;
	m_ds[0x91] = &GPURasterizer::DrawScanlineEx<0x91>;
	m_ds[0x92] = &GPURasterizer::DrawScanlineEx<0x92>;
	m_ds[0x93] = &GPURasterizer::DrawScanlineEx<0x93>;
	m_ds[0x94] = &GPURasterizer::DrawScanlineEx<0x94>;
	m_ds[0x95] = &GPURasterizer::DrawScanlineEx<0x95>;
	m_ds[0x96] = &GPURasterizer::DrawScanlineEx<0x96>;
	m_ds[0x97] = &GPURasterizer::DrawScanlineEx<0x97>;
	m_ds[0x98] = &GPURasterizer::DrawScanlineEx<0x98>;
	m_ds[0x99] = &GPURasterizer::DrawScanlineEx<0x99>;
	m_ds[0x9a] = &GPURasterizer::DrawScanlineEx<0x9a>;
	m_ds[0x9b] = &GPURasterizer::DrawScanlineEx<0x9b>;
	m_ds[0x9c] = &GPURasterizer::DrawScanlineEx<0x9c>;
	m_ds[0x9d] = &GPURasterizer::DrawScanlineEx<0x9d>;
	m_ds[0x9e] = &GPURasterizer::DrawScanlineEx<0x9e>;
	m_ds[0x9f] = &GPURasterizer::DrawScanlineEx<0x9f>;
	m_ds[0xa0] = &GPURasterizer::DrawScanlineEx<0xa0>;
	m_ds[0xa1] = &GPURasterizer::DrawScanlineEx<0xa1>;
	m_ds[0xa2] = &GPURasterizer::DrawScanlineEx<0xa2>;
	m_ds[0xa3] = &GPURasterizer::DrawScanlineEx<0xa3>;
	m_ds[0xa4] = &GPURasterizer::DrawScanlineEx<0xa4>;
	m_ds[0xa5] = &GPURasterizer::DrawScanlineEx<0xa5>;
	m_ds[0xa6] = &GPURasterizer::DrawScanlineEx<0xa6>;
	m_ds[0xa7] = &GPURasterizer::DrawScanlineEx<0xa7>;
	m_ds[0xa8] = &GPURasterizer::DrawScanlineEx<0xa8>;
	m_ds[0xa9] = &GPURasterizer::DrawScanlineEx<0xa9>;
	m_ds[0xaa] = &GPURasterizer::DrawScanlineEx<0xaa>;
	m_ds[0xab] = &GPURasterizer::DrawScanlineEx<0xab>;
	m_ds[0xac] = &GPURasterizer::DrawScanlineEx<0xac>;
	m_ds[0xad] = &GPURasterizer::DrawScanlineEx<0xad>;
	m_ds[0xae] = &GPURasterizer::DrawScanlineEx<0xae>;
	m_ds[0xaf] = &GPURasterizer::DrawScanlineEx<0xaf>;
	m_ds[0xb0] = &GPURasterizer::DrawScanlineEx<0xb0>;
	m_ds[0xb1] = &GPURasterizer::DrawScanlineEx<0xb1>;
	m_ds[0xb2] = &GPURasterizer::DrawScanlineEx<0xb2>;
	m_ds[0xb3] = &GPURasterizer::DrawScanlineEx<0xb3>;
	m_ds[0xb4] = &GPURasterizer::DrawScanlineEx<0xb4>;
	m_ds[0xb5] = &GPURasterizer::DrawScanlineEx<0xb5>;
	m_ds[0xb6] = &GPURasterizer::DrawScanlineEx<0xb6>;
	m_ds[0xb7] = &GPURasterizer::DrawScanlineEx<0xb7>;
	m_ds[0xb8] = &GPURasterizer::DrawScanlineEx<0xb8>;
	m_ds[0xb9] = &GPURasterizer::DrawScanlineEx<0xb9>;
	m_ds[0xba] = &GPURasterizer::DrawScanlineEx<0xba>;
	m_ds[0xbb] = &GPURasterizer::DrawScanlineEx<0xbb>;
	m_ds[0xbc] = &GPURasterizer::DrawScanlineEx<0xbc>;
	m_ds[0xbd] = &GPURasterizer::DrawScanlineEx<0xbd>;
	m_ds[0xbe] = &GPURasterizer::DrawScanlineEx<0xbe>;
	m_ds[0xbf] = &GPURasterizer::DrawScanlineEx<0xbf>;
	m_ds[0xc0] = &GPURasterizer::DrawScanlineEx<0xc0>;
	m_ds[0xc1] = &GPURasterizer::DrawScanlineEx<0xc1>;
	m_ds[0xc2] = &GPURasterizer::DrawScanlineEx<0xc2>;
	m_ds[0xc3] = &GPURasterizer::DrawScanlineEx<0xc3>;
	m_ds[0xc4] = &GPURasterizer::DrawScanlineEx<0xc4>;
	m_ds[0xc5] = &GPURasterizer::DrawScanlineEx<0xc5>;
	m_ds[0xc6] = &GPURasterizer::DrawScanlineEx<0xc6>;
	m_ds[0xc7] = &GPURasterizer::DrawScanlineEx<0xc7>;
	m_ds[0xc8] = &GPURasterizer::DrawScanlineEx<0xc8>;
	m_ds[0xc9] = &GPURasterizer::DrawScanlineEx<0xc9>;
	m_ds[0xca] = &GPURasterizer::DrawScanlineEx<0xca>;
	m_ds[0xcb] = &GPURasterizer::DrawScanlineEx<0xcb>;
	m_ds[0xcc] = &GPURasterizer::DrawScanlineEx<0xcc>;
	m_ds[0xcd] = &GPURasterizer::DrawScanlineEx<0xcd>;
	m_ds[0xce] = &GPURasterizer::DrawScanlineEx<0xce>;
	m_ds[0xcf] = &GPURasterizer::DrawScanlineEx<0xcf>;
	m_ds[0xd0] = &GPURasterizer::DrawScanlineEx<0xd0>;
	m_ds[0xd1] = &GPURasterizer::DrawScanlineEx<0xd1>;
	m_ds[0xd2] = &GPURasterizer::DrawScanlineEx<0xd2>;
	m_ds[0xd3] = &GPURasterizer::DrawScanlineEx<0xd3>;
	m_ds[0xd4] = &GPURasterizer::DrawScanlineEx<0xd4>;
	m_ds[0xd5] = &GPURasterizer::DrawScanlineEx<0xd5>;
	m_ds[0xd6] = &GPURasterizer::DrawScanlineEx<0xd6>;
	m_ds[0xd7] = &GPURasterizer::DrawScanlineEx<0xd7>;
	m_ds[0xd8] = &GPURasterizer::DrawScanlineEx<0xd8>;
	m_ds[0xd9] = &GPURasterizer::DrawScanlineEx<0xd9>;
	m_ds[0xda] = &GPURasterizer::DrawScanlineEx<0xda>;
	m_ds[0xdb] = &GPURasterizer::DrawScanlineEx<0xdb>;
	m_ds[0xdc] = &GPURasterizer::DrawScanlineEx<0xdc>;
	m_ds[0xdd] = &GPURasterizer::DrawScanlineEx<0xdd>;
	m_ds[0xde] = &GPURasterizer::DrawScanlineEx<0xde>;
	m_ds[0xdf] = &GPURasterizer::DrawScanlineEx<0xdf>;
	m_ds[0xe0] = &GPURasterizer::DrawScanlineEx<0xe0>;
	m_ds[0xe1] = &GPURasterizer::DrawScanlineEx<0xe1>;
	m_ds[0xe2] = &GPURasterizer::DrawScanlineEx<0xe2>;
	m_ds[0xe3] = &GPURasterizer::DrawScanlineEx<0xe3>;
	m_ds[0xe4] = &GPURasterizer::DrawScanlineEx<0xe4>;
	m_ds[0xe5] = &GPURasterizer::DrawScanlineEx<0xe5>;
	m_ds[0xe6] = &GPURasterizer::DrawScanlineEx<0xe6>;
	m_ds[0xe7] = &GPURasterizer::DrawScanlineEx<0xe7>;
	m_ds[0xe8] = &GPURasterizer::DrawScanlineEx<0xe8>;
	m_ds[0xe9] = &GPURasterizer::DrawScanlineEx<0xe9>;
	m_ds[0xea] = &GPURasterizer::DrawScanlineEx<0xea>;
	m_ds[0xeb] = &GPURasterizer::DrawScanlineEx<0xeb>;
	m_ds[0xec] = &GPURasterizer::DrawScanlineEx<0xec>;
	m_ds[0xed] = &GPURasterizer::DrawScanlineEx<0xed>;
	m_ds[0xee] = &GPURasterizer::DrawScanlineEx<0xee>;
	m_ds[0xef] = &GPURasterizer::DrawScanlineEx<0xef>;
	m_ds[0xf0] = &GPURasterizer::DrawScanlineEx<0xf0>;
	m_ds[0xf1] = &GPURasterizer::DrawScanlineEx<0xf1>;
	m_ds[0xf2] = &GPURasterizer::DrawScanlineEx<0xf2>;
	m_ds[0xf3] = &GPURasterizer::DrawScanlineEx<0xf3>;
	m_ds[0xf4] = &GPURasterizer::DrawScanlineEx<0xf4>;
	m_ds[0xf5] = &GPURasterizer::DrawScanlineEx<0xf5>;
	m_ds[0xf6] = &GPURasterizer::DrawScanlineEx<0xf6>;
	m_ds[0xf7] = &GPURasterizer::DrawScanlineEx<0xf7>;
	m_ds[0xf8] = &GPURasterizer::DrawScanlineEx<0xf8>;
	m_ds[0xf9] = &GPURasterizer::DrawScanlineEx<0xf9>;
	m_ds[0xfa] = &GPURasterizer::DrawScanlineEx<0xfa>;
	m_ds[0xfb] = &GPURasterizer::DrawScanlineEx<0xfb>;
	m_ds[0xfc] = &GPURasterizer::DrawScanlineEx<0xfc>;
	m_ds[0xfd] = &GPURasterizer::DrawScanlineEx<0xfd>;
	m_ds[0xfe] = &GPURasterizer::DrawScanlineEx<0xfe>;
	m_ds[0xff] = &GPURasterizer::DrawScanlineEx<0xff>;
	m_ds[0x100] = &GPURasterizer::DrawScanlineEx<0x100>;
	m_ds[0x101] = &GPURasterizer::DrawScanlineEx<0x101>;
	m_ds[0x102] = &GPURasterizer::DrawScanlineEx<0x102>;
	m_ds[0x103] = &GPURasterizer::DrawScanlineEx<0x103>;
	m_ds[0x104] = &GPURasterizer::DrawScanlineEx<0x104>;
	m_ds[0x105] = &GPURasterizer::DrawScanlineEx<0x105>;
	m_ds[0x106] = &GPURasterizer::DrawScanlineEx<0x106>;
	m_ds[0x107] = &GPURasterizer::DrawScanlineEx<0x107>;
	m_ds[0x108] = &GPURasterizer::DrawScanlineEx<0x108>;
	m_ds[0x109] = &GPURasterizer::DrawScanlineEx<0x109>;
	m_ds[0x10a] = &GPURasterizer::DrawScanlineEx<0x10a>;
	m_ds[0x10b] = &GPURasterizer::DrawScanlineEx<0x10b>;
	m_ds[0x10c] = &GPURasterizer::DrawScanlineEx<0x10c>;
	m_ds[0x10d] = &GPURasterizer::DrawScanlineEx<0x10d>;
	m_ds[0x10e] = &GPURasterizer::DrawScanlineEx<0x10e>;
	m_ds[0x10f] = &GPURasterizer::DrawScanlineEx<0x10f>;
	m_ds[0x110] = &GPURasterizer::DrawScanlineEx<0x110>;
	m_ds[0x111] = &GPURasterizer::DrawScanlineEx<0x111>;
	m_ds[0x112] = &GPURasterizer::DrawScanlineEx<0x112>;
	m_ds[0x113] = &GPURasterizer::DrawScanlineEx<0x113>;
	m_ds[0x114] = &GPURasterizer::DrawScanlineEx<0x114>;
	m_ds[0x115] = &GPURasterizer::DrawScanlineEx<0x115>;
	m_ds[0x116] = &GPURasterizer::DrawScanlineEx<0x116>;
	m_ds[0x117] = &GPURasterizer::DrawScanlineEx<0x117>;
	m_ds[0x118] = &GPURasterizer::DrawScanlineEx<0x118>;
	m_ds[0x119] = &GPURasterizer::DrawScanlineEx<0x119>;
	m_ds[0x11a] = &GPURasterizer::DrawScanlineEx<0x11a>;
	m_ds[0x11b] = &GPURasterizer::DrawScanlineEx<0x11b>;
	m_ds[0x11c] = &GPURasterizer::DrawScanlineEx<0x11c>;
	m_ds[0x11d] = &GPURasterizer::DrawScanlineEx<0x11d>;
	m_ds[0x11e] = &GPURasterizer::DrawScanlineEx<0x11e>;
	m_ds[0x11f] = &GPURasterizer::DrawScanlineEx<0x11f>;
	m_ds[0x120] = &GPURasterizer::DrawScanlineEx<0x120>;
	m_ds[0x121] = &GPURasterizer::DrawScanlineEx<0x121>;
	m_ds[0x122] = &GPURasterizer::DrawScanlineEx<0x122>;
	m_ds[0x123] = &GPURasterizer::DrawScanlineEx<0x123>;
	m_ds[0x124] = &GPURasterizer::DrawScanlineEx<0x124>;
	m_ds[0x125] = &GPURasterizer::DrawScanlineEx<0x125>;
	m_ds[0x126] = &GPURasterizer::DrawScanlineEx<0x126>;
	m_ds[0x127] = &GPURasterizer::DrawScanlineEx<0x127>;
	m_ds[0x128] = &GPURasterizer::DrawScanlineEx<0x128>;
	m_ds[0x129] = &GPURasterizer::DrawScanlineEx<0x129>;
	m_ds[0x12a] = &GPURasterizer::DrawScanlineEx<0x12a>;
	m_ds[0x12b] = &GPURasterizer::DrawScanlineEx<0x12b>;
	m_ds[0x12c] = &GPURasterizer::DrawScanlineEx<0x12c>;
	m_ds[0x12d] = &GPURasterizer::DrawScanlineEx<0x12d>;
	m_ds[0x12e] = &GPURasterizer::DrawScanlineEx<0x12e>;
	m_ds[0x12f] = &GPURasterizer::DrawScanlineEx<0x12f>;
	m_ds[0x130] = &GPURasterizer::DrawScanlineEx<0x130>;
	m_ds[0x131] = &GPURasterizer::DrawScanlineEx<0x131>;
	m_ds[0x132] = &GPURasterizer::DrawScanlineEx<0x132>;
	m_ds[0x133] = &GPURasterizer::DrawScanlineEx<0x133>;
	m_ds[0x134] = &GPURasterizer::DrawScanlineEx<0x134>;
	m_ds[0x135] = &GPURasterizer::DrawScanlineEx<0x135>;
	m_ds[0x136] = &GPURasterizer::DrawScanlineEx<0x136>;
	m_ds[0x137] = &GPURasterizer::DrawScanlineEx<0x137>;
	m_ds[0x138] = &GPURasterizer::DrawScanlineEx<0x138>;
	m_ds[0x139] = &GPURasterizer::DrawScanlineEx<0x139>;
	m_ds[0x13a] = &GPURasterizer::DrawScanlineEx<0x13a>;
	m_ds[0x13b] = &GPURasterizer::DrawScanlineEx<0x13b>;
	m_ds[0x13c] = &GPURasterizer::DrawScanlineEx<0x13c>;
	m_ds[0x13d] = &GPURasterizer::DrawScanlineEx<0x13d>;
	m_ds[0x13e] = &GPURasterizer::DrawScanlineEx<0x13e>;
	m_ds[0x13f] = &GPURasterizer::DrawScanlineEx<0x13f>;
	m_ds[0x140] = &GPURasterizer::DrawScanlineEx<0x140>;
	m_ds[0x141] = &GPURasterizer::DrawScanlineEx<0x141>;
	m_ds[0x142] = &GPURasterizer::DrawScanlineEx<0x142>;
	m_ds[0x143] = &GPURasterizer::DrawScanlineEx<0x143>;
	m_ds[0x144] = &GPURasterizer::DrawScanlineEx<0x144>;
	m_ds[0x145] = &GPURasterizer::DrawScanlineEx<0x145>;
	m_ds[0x146] = &GPURasterizer::DrawScanlineEx<0x146>;
	m_ds[0x147] = &GPURasterizer::DrawScanlineEx<0x147>;
	m_ds[0x148] = &GPURasterizer::DrawScanlineEx<0x148>;
	m_ds[0x149] = &GPURasterizer::DrawScanlineEx<0x149>;
	m_ds[0x14a] = &GPURasterizer::DrawScanlineEx<0x14a>;
	m_ds[0x14b] = &GPURasterizer::DrawScanlineEx<0x14b>;
	m_ds[0x14c] = &GPURasterizer::DrawScanlineEx<0x14c>;
	m_ds[0x14d] = &GPURasterizer::DrawScanlineEx<0x14d>;
	m_ds[0x14e] = &GPURasterizer::DrawScanlineEx<0x14e>;
	m_ds[0x14f] = &GPURasterizer::DrawScanlineEx<0x14f>;
	m_ds[0x150] = &GPURasterizer::DrawScanlineEx<0x150>;
	m_ds[0x151] = &GPURasterizer::DrawScanlineEx<0x151>;
	m_ds[0x152] = &GPURasterizer::DrawScanlineEx<0x152>;
	m_ds[0x153] = &GPURasterizer::DrawScanlineEx<0x153>;
	m_ds[0x154] = &GPURasterizer::DrawScanlineEx<0x154>;
	m_ds[0x155] = &GPURasterizer::DrawScanlineEx<0x155>;
	m_ds[0x156] = &GPURasterizer::DrawScanlineEx<0x156>;
	m_ds[0x157] = &GPURasterizer::DrawScanlineEx<0x157>;
	m_ds[0x158] = &GPURasterizer::DrawScanlineEx<0x158>;
	m_ds[0x159] = &GPURasterizer::DrawScanlineEx<0x159>;
	m_ds[0x15a] = &GPURasterizer::DrawScanlineEx<0x15a>;
	m_ds[0x15b] = &GPURasterizer::DrawScanlineEx<0x15b>;
	m_ds[0x15c] = &GPURasterizer::DrawScanlineEx<0x15c>;
	m_ds[0x15d] = &GPURasterizer::DrawScanlineEx<0x15d>;
	m_ds[0x15e] = &GPURasterizer::DrawScanlineEx<0x15e>;
	m_ds[0x15f] = &GPURasterizer::DrawScanlineEx<0x15f>;
	m_ds[0x160] = &GPURasterizer::DrawScanlineEx<0x160>;
	m_ds[0x161] = &GPURasterizer::DrawScanlineEx<0x161>;
	m_ds[0x162] = &GPURasterizer::DrawScanlineEx<0x162>;
	m_ds[0x163] = &GPURasterizer::DrawScanlineEx<0x163>;
	m_ds[0x164] = &GPURasterizer::DrawScanlineEx<0x164>;
	m_ds[0x165] = &GPURasterizer::DrawScanlineEx<0x165>;
	m_ds[0x166] = &GPURasterizer::DrawScanlineEx<0x166>;
	m_ds[0x167] = &GPURasterizer::DrawScanlineEx<0x167>;
	m_ds[0x168] = &GPURasterizer::DrawScanlineEx<0x168>;
	m_ds[0x169] = &GPURasterizer::DrawScanlineEx<0x169>;
	m_ds[0x16a] = &GPURasterizer::DrawScanlineEx<0x16a>;
	m_ds[0x16b] = &GPURasterizer::DrawScanlineEx<0x16b>;
	m_ds[0x16c] = &GPURasterizer::DrawScanlineEx<0x16c>;
	m_ds[0x16d] = &GPURasterizer::DrawScanlineEx<0x16d>;
	m_ds[0x16e] = &GPURasterizer::DrawScanlineEx<0x16e>;
	m_ds[0x16f] = &GPURasterizer::DrawScanlineEx<0x16f>;
	m_ds[0x170] = &GPURasterizer::DrawScanlineEx<0x170>;
	m_ds[0x171] = &GPURasterizer::DrawScanlineEx<0x171>;
	m_ds[0x172] = &GPURasterizer::DrawScanlineEx<0x172>;
	m_ds[0x173] = &GPURasterizer::DrawScanlineEx<0x173>;
	m_ds[0x174] = &GPURasterizer::DrawScanlineEx<0x174>;
	m_ds[0x175] = &GPURasterizer::DrawScanlineEx<0x175>;
	m_ds[0x176] = &GPURasterizer::DrawScanlineEx<0x176>;
	m_ds[0x177] = &GPURasterizer::DrawScanlineEx<0x177>;
	m_ds[0x178] = &GPURasterizer::DrawScanlineEx<0x178>;
	m_ds[0x179] = &GPURasterizer::DrawScanlineEx<0x179>;
	m_ds[0x17a] = &GPURasterizer::DrawScanlineEx<0x17a>;
	m_ds[0x17b] = &GPURasterizer::DrawScanlineEx<0x17b>;
	m_ds[0x17c] = &GPURasterizer::DrawScanlineEx<0x17c>;
	m_ds[0x17d] = &GPURasterizer::DrawScanlineEx<0x17d>;
	m_ds[0x17e] = &GPURasterizer::DrawScanlineEx<0x17e>;
	m_ds[0x17f] = &GPURasterizer::DrawScanlineEx<0x17f>;
	m_ds[0x180] = &GPURasterizer::DrawScanlineEx<0x180>;
	m_ds[0x181] = &GPURasterizer::DrawScanlineEx<0x181>;
	m_ds[0x182] = &GPURasterizer::DrawScanlineEx<0x182>;
	m_ds[0x183] = &GPURasterizer::DrawScanlineEx<0x183>;
	m_ds[0x184] = &GPURasterizer::DrawScanlineEx<0x184>;
	m_ds[0x185] = &GPURasterizer::DrawScanlineEx<0x185>;
	m_ds[0x186] = &GPURasterizer::DrawScanlineEx<0x186>;
	m_ds[0x187] = &GPURasterizer::DrawScanlineEx<0x187>;
	m_ds[0x188] = &GPURasterizer::DrawScanlineEx<0x188>;
	m_ds[0x189] = &GPURasterizer::DrawScanlineEx<0x189>;
	m_ds[0x18a] = &GPURasterizer::DrawScanlineEx<0x18a>;
	m_ds[0x18b] = &GPURasterizer::DrawScanlineEx<0x18b>;
	m_ds[0x18c] = &GPURasterizer::DrawScanlineEx<0x18c>;
	m_ds[0x18d] = &GPURasterizer::DrawScanlineEx<0x18d>;
	m_ds[0x18e] = &GPURasterizer::DrawScanlineEx<0x18e>;
	m_ds[0x18f] = &GPURasterizer::DrawScanlineEx<0x18f>;
	m_ds[0x190] = &GPURasterizer::DrawScanlineEx<0x190>;
	m_ds[0x191] = &GPURasterizer::DrawScanlineEx<0x191>;
	m_ds[0x192] = &GPURasterizer::DrawScanlineEx<0x192>;
	m_ds[0x193] = &GPURasterizer::DrawScanlineEx<0x193>;
	m_ds[0x194] = &GPURasterizer::DrawScanlineEx<0x194>;
	m_ds[0x195] = &GPURasterizer::DrawScanlineEx<0x195>;
	m_ds[0x196] = &GPURasterizer::DrawScanlineEx<0x196>;
	m_ds[0x197] = &GPURasterizer::DrawScanlineEx<0x197>;
	m_ds[0x198] = &GPURasterizer::DrawScanlineEx<0x198>;
	m_ds[0x199] = &GPURasterizer::DrawScanlineEx<0x199>;
	m_ds[0x19a] = &GPURasterizer::DrawScanlineEx<0x19a>;
	m_ds[0x19b] = &GPURasterizer::DrawScanlineEx<0x19b>;
	m_ds[0x19c] = &GPURasterizer::DrawScanlineEx<0x19c>;
	m_ds[0x19d] = &GPURasterizer::DrawScanlineEx<0x19d>;
	m_ds[0x19e] = &GPURasterizer::DrawScanlineEx<0x19e>;
	m_ds[0x19f] = &GPURasterizer::DrawScanlineEx<0x19f>;
	m_ds[0x1a0] = &GPURasterizer::DrawScanlineEx<0x1a0>;
	m_ds[0x1a1] = &GPURasterizer::DrawScanlineEx<0x1a1>;
	m_ds[0x1a2] = &GPURasterizer::DrawScanlineEx<0x1a2>;
	m_ds[0x1a3] = &GPURasterizer::DrawScanlineEx<0x1a3>;
	m_ds[0x1a4] = &GPURasterizer::DrawScanlineEx<0x1a4>;
	m_ds[0x1a5] = &GPURasterizer::DrawScanlineEx<0x1a5>;
	m_ds[0x1a6] = &GPURasterizer::DrawScanlineEx<0x1a6>;
	m_ds[0x1a7] = &GPURasterizer::DrawScanlineEx<0x1a7>;
	m_ds[0x1a8] = &GPURasterizer::DrawScanlineEx<0x1a8>;
	m_ds[0x1a9] = &GPURasterizer::DrawScanlineEx<0x1a9>;
	m_ds[0x1aa] = &GPURasterizer::DrawScanlineEx<0x1aa>;
	m_ds[0x1ab] = &GPURasterizer::DrawScanlineEx<0x1ab>;
	m_ds[0x1ac] = &GPURasterizer::DrawScanlineEx<0x1ac>;
	m_ds[0x1ad] = &GPURasterizer::DrawScanlineEx<0x1ad>;
	m_ds[0x1ae] = &GPURasterizer::DrawScanlineEx<0x1ae>;
	m_ds[0x1af] = &GPURasterizer::DrawScanlineEx<0x1af>;
	m_ds[0x1b0] = &GPURasterizer::DrawScanlineEx<0x1b0>;
	m_ds[0x1b1] = &GPURasterizer::DrawScanlineEx<0x1b1>;
	m_ds[0x1b2] = &GPURasterizer::DrawScanlineEx<0x1b2>;
	m_ds[0x1b3] = &GPURasterizer::DrawScanlineEx<0x1b3>;
	m_ds[0x1b4] = &GPURasterizer::DrawScanlineEx<0x1b4>;
	m_ds[0x1b5] = &GPURasterizer::DrawScanlineEx<0x1b5>;
	m_ds[0x1b6] = &GPURasterizer::DrawScanlineEx<0x1b6>;
	m_ds[0x1b7] = &GPURasterizer::DrawScanlineEx<0x1b7>;
	m_ds[0x1b8] = &GPURasterizer::DrawScanlineEx<0x1b8>;
	m_ds[0x1b9] = &GPURasterizer::DrawScanlineEx<0x1b9>;
	m_ds[0x1ba] = &GPURasterizer::DrawScanlineEx<0x1ba>;
	m_ds[0x1bb] = &GPURasterizer::DrawScanlineEx<0x1bb>;
	m_ds[0x1bc] = &GPURasterizer::DrawScanlineEx<0x1bc>;
	m_ds[0x1bd] = &GPURasterizer::DrawScanlineEx<0x1bd>;
	m_ds[0x1be] = &GPURasterizer::DrawScanlineEx<0x1be>;
	m_ds[0x1bf] = &GPURasterizer::DrawScanlineEx<0x1bf>;
	m_ds[0x1c0] = &GPURasterizer::DrawScanlineEx<0x1c0>;
	m_ds[0x1c1] = &GPURasterizer::DrawScanlineEx<0x1c1>;
	m_ds[0x1c2] = &GPURasterizer::DrawScanlineEx<0x1c2>;
	m_ds[0x1c3] = &GPURasterizer::DrawScanlineEx<0x1c3>;
	m_ds[0x1c4] = &GPURasterizer::DrawScanlineEx<0x1c4>;
	m_ds[0x1c5] = &GPURasterizer::DrawScanlineEx<0x1c5>;
	m_ds[0x1c6] = &GPURasterizer::DrawScanlineEx<0x1c6>;
	m_ds[0x1c7] = &GPURasterizer::DrawScanlineEx<0x1c7>;
	m_ds[0x1c8] = &GPURasterizer::DrawScanlineEx<0x1c8>;
	m_ds[0x1c9] = &GPURasterizer::DrawScanlineEx<0x1c9>;
	m_ds[0x1ca] = &GPURasterizer::DrawScanlineEx<0x1ca>;
	m_ds[0x1cb] = &GPURasterizer::DrawScanlineEx<0x1cb>;
	m_ds[0x1cc] = &GPURasterizer::DrawScanlineEx<0x1cc>;
	m_ds[0x1cd] = &GPURasterizer::DrawScanlineEx<0x1cd>;
	m_ds[0x1ce] = &GPURasterizer::DrawScanlineEx<0x1ce>;
	m_ds[0x1cf] = &GPURasterizer::DrawScanlineEx<0x1cf>;
	m_ds[0x1d0] = &GPURasterizer::DrawScanlineEx<0x1d0>;
	m_ds[0x1d1] = &GPURasterizer::DrawScanlineEx<0x1d1>;
	m_ds[0x1d2] = &GPURasterizer::DrawScanlineEx<0x1d2>;
	m_ds[0x1d3] = &GPURasterizer::DrawScanlineEx<0x1d3>;
	m_ds[0x1d4] = &GPURasterizer::DrawScanlineEx<0x1d4>;
	m_ds[0x1d5] = &GPURasterizer::DrawScanlineEx<0x1d5>;
	m_ds[0x1d6] = &GPURasterizer::DrawScanlineEx<0x1d6>;
	m_ds[0x1d7] = &GPURasterizer::DrawScanlineEx<0x1d7>;
	m_ds[0x1d8] = &GPURasterizer::DrawScanlineEx<0x1d8>;
	m_ds[0x1d9] = &GPURasterizer::DrawScanlineEx<0x1d9>;
	m_ds[0x1da] = &GPURasterizer::DrawScanlineEx<0x1da>;
	m_ds[0x1db] = &GPURasterizer::DrawScanlineEx<0x1db>;
	m_ds[0x1dc] = &GPURasterizer::DrawScanlineEx<0x1dc>;
	m_ds[0x1dd] = &GPURasterizer::DrawScanlineEx<0x1dd>;
	m_ds[0x1de] = &GPURasterizer::DrawScanlineEx<0x1de>;
	m_ds[0x1df] = &GPURasterizer::DrawScanlineEx<0x1df>;
	m_ds[0x1e0] = &GPURasterizer::DrawScanlineEx<0x1e0>;
	m_ds[0x1e1] = &GPURasterizer::DrawScanlineEx<0x1e1>;
	m_ds[0x1e2] = &GPURasterizer::DrawScanlineEx<0x1e2>;
	m_ds[0x1e3] = &GPURasterizer::DrawScanlineEx<0x1e3>;
	m_ds[0x1e4] = &GPURasterizer::DrawScanlineEx<0x1e4>;
	m_ds[0x1e5] = &GPURasterizer::DrawScanlineEx<0x1e5>;
	m_ds[0x1e6] = &GPURasterizer::DrawScanlineEx<0x1e6>;
	m_ds[0x1e7] = &GPURasterizer::DrawScanlineEx<0x1e7>;
	m_ds[0x1e8] = &GPURasterizer::DrawScanlineEx<0x1e8>;
	m_ds[0x1e9] = &GPURasterizer::DrawScanlineEx<0x1e9>;
	m_ds[0x1ea] = &GPURasterizer::DrawScanlineEx<0x1ea>;
	m_ds[0x1eb] = &GPURasterizer::DrawScanlineEx<0x1eb>;
	m_ds[0x1ec] = &GPURasterizer::DrawScanlineEx<0x1ec>;
	m_ds[0x1ed] = &GPURasterizer::DrawScanlineEx<0x1ed>;
	m_ds[0x1ee] = &GPURasterizer::DrawScanlineEx<0x1ee>;
	m_ds[0x1ef] = &GPURasterizer::DrawScanlineEx<0x1ef>;
	m_ds[0x1f0] = &GPURasterizer::DrawScanlineEx<0x1f0>;
	m_ds[0x1f1] = &GPURasterizer::DrawScanlineEx<0x1f1>;
	m_ds[0x1f2] = &GPURasterizer::DrawScanlineEx<0x1f2>;
	m_ds[0x1f3] = &GPURasterizer::DrawScanlineEx<0x1f3>;
	m_ds[0x1f4] = &GPURasterizer::DrawScanlineEx<0x1f4>;
	m_ds[0x1f5] = &GPURasterizer::DrawScanlineEx<0x1f5>;
	m_ds[0x1f6] = &GPURasterizer::DrawScanlineEx<0x1f6>;
	m_ds[0x1f7] = &GPURasterizer::DrawScanlineEx<0x1f7>;
	m_ds[0x1f8] = &GPURasterizer::DrawScanlineEx<0x1f8>;
	m_ds[0x1f9] = &GPURasterizer::DrawScanlineEx<0x1f9>;
	m_ds[0x1fa] = &GPURasterizer::DrawScanlineEx<0x1fa>;
	m_ds[0x1fb] = &GPURasterizer::DrawScanlineEx<0x1fb>;
	m_ds[0x1fc] = &GPURasterizer::DrawScanlineEx<0x1fc>;
	m_ds[0x1fd] = &GPURasterizer::DrawScanlineEx<0x1fd>;
	m_ds[0x1fe] = &GPURasterizer::DrawScanlineEx<0x1fe>;
	m_ds[0x1ff] = &GPURasterizer::DrawScanlineEx<0x1ff>;
}

GPURasterizer::~GPURasterizer()
{
}

int GPURasterizer::Draw(Vertex* vertices, int count, const void* texture)
{
	GPUDrawingEnvironment& env = m_state->m_env;

	// m_scissor

	m_scissor.x = env.DRAREATL.X;
	m_scissor.y = env.DRAREATL.Y;
	m_scissor.z = min(env.DRAREABR.X + 1, 1024);
	m_scissor.w = min(env.DRAREABR.Y + 1, 512);

	if(m_scissor.x >= m_scissor.z || m_scissor.y >= m_scissor.w)
	{
		ASSERT(0);
		return 0;
	}

	// m_sel

	m_sel.dw = 0;
	m_sel.iip = env.PRIM.IIP;
	m_sel.me = env.STATUS.ME;
	m_sel.abe = env.PRIM.ABE;
	m_sel.abr = env.STATUS.ABR;
	m_sel.tge = env.PRIM.TGE;
	m_sel.tme = env.PRIM.TME;
	m_sel.tlu = env.STATUS.TP < 2;
	m_sel.twin = (env.TWIN.ai32 & 0xfffff) != 0;
	m_sel.ltf = 1; // TODO

	m_dsf = m_ds[m_sel];

	// m_slenv

	m_slenv.steps = 0;

	m_slenv.vm = m_state->m_mem.m_vm16;

	if(m_sel.tme)
	{
		m_slenv.tex = texture;
		m_slenv.clut = m_state->GetCLUT();

		if(m_sel.twin)
		{
			DWORD u, v;

			u = ~(env.TWIN.TWW << 3) & 0xff;
			v = ~(env.TWIN.TWH << 3) & 0xff;

			m_slenv.u[0] = GSVector4i((u << 16) | u);
			m_slenv.v[0] = GSVector4i((v << 16) | v);

			u = env.TWIN.TWX << 3;
			v = env.TWIN.TWY << 3;
			
			m_slenv.u[1] = GSVector4i((u << 16) | u);
			m_slenv.v[1] = GSVector4i((v << 16) | v);
		}
	}

	m_slenv.a = GSVector4i(env.PRIM.ABE ? 0xffffffff : 0);
	m_slenv.md = GSVector4i(env.STATUS.MD ? 0x80008000 : 0);

	switch(env.PRIM.TYPE)
	{
	case GPU_POLYGON:
		ASSERT(!(count % 3));
		count = count / 3;
		for(int i = 0; i < count; i++, vertices += 3) DrawTriangle(vertices);
		break;
	case GPU_LINE:
		ASSERT(!(count & 1));
		count = count / 2;
		for(int i = 0; i < count; i++, vertices += 2) DrawLine(vertices);
		break;
	case GPU_SPRITE:
		ASSERT(!(count & 1));
		count = count / 2;
		for(int i = 0; i < count; i++, vertices += 2) DrawSprite(vertices);
		break;
	default:
		__assume(0);
	}

	m_state->m_perfmon.Put(GSPerfMon::Fillrate, m_slenv.steps); // TODO: move this to the renderer, not thread safe here

	return count;
}

void GPURasterizer::DrawPoint(Vertex* v)
{
	// TODO: round to closest for point, prestep for line

	GSVector4i p(v->p);

	if(m_scissor.x <= p.x && p.x < m_scissor.z && m_scissor.y <= p.y && p.y < m_scissor.w)
	{
		if((p.y % m_threads) == m_id) 
		{
			(this->*m_dsf)(p.y, p.x, p.x + 1, *v);
		}
	}
}

void GPURasterizer::DrawLine(Vertex* v)
{
	Vertex dv = v[1] - v[0];

	GSVector4 dp = dv.p.abs();
	GSVector4i dpi(dp);

	if(dpi.x == 0 && dpi.y == 0) return;

	int i = dpi.x > dpi.y ? 0 : 1;

	Vertex edge = v[0];
	Vertex dedge = dv / dp.v[i];

	// TODO: prestep + clip with the scissor

	int steps = dpi.v[i];

	while(steps-- > 0)
	{
		DrawPoint(&edge);

		edge += dedge;
	}
}

static const int s_abc[8][4] = 
{
	{0, 1, 2, 0},
	{1, 0, 2, 0},
	{0, 0, 0, 0},
	{1, 2, 0, 0},
	{0, 2, 1, 0},
	{0, 0, 0, 0},
	{2, 0, 1, 0},
	{2, 1, 0, 0},
};

void GPURasterizer::DrawTriangle(Vertex* vertices)
{
	Vertex v[3];

	GSVector4 aabb = vertices[0].p.yyyy(vertices[1].p);
	GSVector4 bccb = vertices[1].p.yyyy(vertices[2].p).xzzx();

	int i = (aabb > bccb).mask() & 7;

	v[0] = vertices[s_abc[i][0]];
	v[1] = vertices[s_abc[i][1]];
	v[2] = vertices[s_abc[i][2]];

	aabb = v[0].p.yyyy(v[1].p);
	bccb = v[1].p.yyyy(v[2].p).xzzx();

	i = (aabb == bccb).mask() & 7;

	switch(i)
	{
	case 0: // a < b < c
		DrawTriangleTopBottom(v);
		break;
	case 1: // a == b < c
		DrawTriangleBottom(v);
		break;
	case 4: // a < b == c
		DrawTriangleTop(v);
		break;
	case 7: // a == b == c
		break;
	default:
		__assume(0);
	}
}

void GPURasterizer::DrawTriangleTop(Vertex* v)
{
	Vertex longest = v[2] - v[1];
	
	if((longest.p == GSVector4::zero()).mask() & 1)
	{
		return;
	}

	Vertex dscan = longest * longest.p.xxxx().rcp();

	SetupScanline<true, true>(dscan);

	int i = (longest.p > GSVector4::zero()).mask() & 1;

	Vertex& l = v[0];
	GSVector4 r = v[0].p;

	Vertex vl = v[2 - i] - l;
	GSVector4 vr = v[1 + i].p - r;

	Vertex dl = vl / vl.p.yyyy();
	GSVector4 dr = vr / vr.yyyy();

	GSVector4i tb(l.p.xyxy(v[2].p).ceil());

	int top = tb.y;
	int bottom = tb.w;

	if(top < m_scissor.y) top = m_scissor.y;
	if(bottom > m_scissor.w) bottom = m_scissor.w;
			
	if(top < bottom)
	{
		float py = (float)top - l.p.y;

		if(py > 0)
		{
			GSVector4 dy(py);

			l += dl * dy;
			r += dr * dy;
		}

		DrawTriangleSection(top, bottom, l, dl, r, dr, dscan);
	}
}

void GPURasterizer::DrawTriangleBottom(Vertex* v)
{
	Vertex longest = v[1] - v[0];
	
	if((longest.p == GSVector4::zero()).mask() & 1)
	{
		return;
	}
	
	Vertex dscan = longest * longest.p.xxxx().rcp();

	SetupScanline<true, true>(dscan);

	int i = (longest.p > GSVector4::zero()).mask() & 1;

	Vertex& l = v[1 - i];
	GSVector4& r = v[i].p;

	Vertex vl = v[2] - l;
	GSVector4 vr = v[2].p - r;

	Vertex dl = vl / vl.p.yyyy();
	GSVector4 dr = vr / vr.yyyy();

	GSVector4i tb(l.p.xyxy(v[2].p).ceil());

	int top = tb.y;
	int bottom = tb.w;

	if(top < m_scissor.y) top = m_scissor.y;
	if(bottom > m_scissor.w) bottom = m_scissor.w;
	
	if(top < bottom)
	{
		float py = (float)top - l.p.y;

		if(py > 0)
		{
			GSVector4 dy(py);

			l += dl * dy;
			r += dr * dy;
		}

		DrawTriangleSection(top, bottom, l, dl, r, dr, dscan);
	}
}

void GPURasterizer::DrawTriangleTopBottom(Vertex* v)
{
	Vertex v01, v02, v12;

	v01 = v[1] - v[0];
	v02 = v[2] - v[0];

	Vertex longest = v[0] + v02 * (v01.p / v02.p).yyyy() - v[1];

	if((longest.p == GSVector4::zero()).mask() & 1)
	{
		return;
	}

	Vertex dscan = longest * longest.p.xxxx().rcp();

	SetupScanline<true, true>(dscan);

	Vertex& l = v[0];
	GSVector4 r = v[0].p;

	Vertex dl;
	GSVector4 dr;

	bool b = (longest.p > GSVector4::zero()).mask() & 1;
	
	if(b)
	{
		dl = v01 / v01.p.yyyy();
		dr = v02.p / v02.p.yyyy();
	}
	else
	{
		dl = v02 / v02.p.yyyy();
		dr = v01.p / v01.p.yyyy();
	}

	GSVector4i tb(v[0].p.yyyy(v[1].p).xzyy(v[2].p).ceil());

	int top = tb.x;
	int bottom = tb.y;

	if(top < m_scissor.y) top = m_scissor.y;
	if(bottom > m_scissor.w) bottom = m_scissor.w;

	float py = (float)top - l.p.y;

	if(py > 0)
	{
		GSVector4 dy(py);

		l += dl * dy;
		r += dr * dy;
	}

	if(top < bottom)
	{
		DrawTriangleSection(top, bottom, l, dl, r, dr, dscan);
	}

	if(b)
	{
		v12 = v[2] - v[1];

		l = v[1];

		dl = v12 / v12.p.yyyy();
	}
	else
	{
		v12.p = v[2].p - v[1].p;

		r = v[1].p;

		dr = v12.p / v12.p.yyyy();
	}

	top = tb.y;
	bottom = tb.z;

	if(top < m_scissor.y) top = m_scissor.y;
	if(bottom > m_scissor.w) bottom = m_scissor.w;

	if(top < bottom)
	{
		py = (float)top - l.p.y;

		if(py > 0) l += dl * py;

		py = (float)top - r.y;

		if(py > 0) r += dr * py;

		DrawTriangleSection(top, bottom, l, dl, r, dr, dscan);
	}
}

void GPURasterizer::DrawTriangleSection(int top, int bottom, Vertex& l, const Vertex& dl, GSVector4& r, const GSVector4& dr, const Vertex& dscan)
{
	ASSERT(top < bottom);

	while(1)
	{
		do
		{
			if((top % m_threads) == m_id) 
			{
				GSVector4i lr(l.p.xyxy(r).ceil());

				int& left = lr.x;
				int& right = lr.z;

				if(left < m_scissor.x) left = m_scissor.x;
				if(right > m_scissor.z) right = m_scissor.z;

				if(left < right)
				{
					Vertex scan = l;

					float px = (float)left - l.p.x;

					if(px > 0) scan += dscan * px;

					(this->*m_dsf)(top, left, right, scan);
				}
			}
		}
		while(0);

		if(++top >= bottom) break;

		l += dl;
		r += dr;
	}
}

void GPURasterizer::DrawSprite(Vertex* v)
{
	GSVector4i r(v[0].p.xyxy(v[1].p).ceil());

	int& top = r.y;
	int& bottom = r.w;

	int& left = r.x;
	int& right = r.z;

	#if _M_SSE >= 0x401

	r = r.sat_i32(m_scissor);
	
	if((r < r.zwzw()).mask() != 0x00ff) return;

	#else

	if(top < m_scissor.y) top = m_scissor.y;
	if(bottom > m_scissor.w) bottom = m_scissor.w;
	if(top >= bottom) return;

	if(left < m_scissor.x) left = m_scissor.x;
	if(right > m_scissor.z) right = m_scissor.z;
	if(left >= right) return;

	#endif

	Vertex scan = v[0];

	// TODO: solid rect

	if(m_sel.tme)
	{
		Vertex dedge, dscan;

		GSVector4 one = GSVector4(1.0f).xyxy(GSVector4::zero());

		dscan.p = one.wwxw();
		dedge.p = one.wwwy();

		if(scan.p.y < (float)top) scan.p += dedge.p * ((float)top - scan.p.y);
		if(scan.p.x < (float)left) scan.p += dscan.p * ((float)left - scan.p.x);

		SetupScanline<true, false>(dscan);

		for(; top < bottom; top++, scan.p += dedge.p)
		{
			if((top % m_threads) == m_id) 
			{
				(this->*m_dsf)(top, left, right, scan);
			}
		}
	}
	else
	{
		for(; top < bottom; top++)
		{
			if((top % m_threads) == m_id) 
			{
				(this->*m_dsf)(top, left, right, scan);
			}
		}
	}
}

template<bool pos, bool col> 
void GPURasterizer::SetupScanline(const Vertex& dv)
{
	if(pos)
	{
		GSVector4 dp = dv.p;

		m_slenv.dp = dp;
		m_slenv.dp8 = dp * 8.0f;
	}

	if(col)
	{
		GSVector4 dc = dv.c;

		m_slenv.dc = dc;
		m_slenv.dc8 = dc * 8.0f;
	}
}

void GPURasterizer::DrawScanline(int top, int left, int right, const Vertex& v)	
{
	GSVector4 ps0123 = GSVector4::ps0123();
	GSVector4 ps4567 = GSVector4::ps4567();

	GSVector4 s[2], t[2]; 
	
	GSVector4 vp = v.p;

	s[0] = vp.zzzz(); s[1] = s[0];
	t[0] = vp.wwww(); t[1] = t[0];
	
	if(m_sel.tme)
	{
		GSVector4 dp = m_slenv.dp;

		s[0] += dp.zzzz() * ps0123;
		t[0] += dp.wwww() * ps0123;
		s[1] += dp.zzzz() * ps4567;
		t[1] += dp.wwww() * ps4567;
	}

	GSVector4 r[2], g[2], b[2];
	
	GSVector4 vc = v.c;

	r[0] = vc.xxxx(); r[1] = r[0];
	g[0] = vc.yyyy(); g[1] = g[0];
	b[0] = vc.zzzz(); b[1] = b[0];

	if(m_sel.iip)
	{
		GSVector4 dc = m_slenv.dc;

		r[0] += dc.xxxx() * ps0123;
		g[0] += dc.yyyy() * ps0123;
		b[0] += dc.zzzz() * ps0123;
		r[1] += dc.xxxx() * ps4567;
		g[1] += dc.yyyy() * ps4567;
		b[1] += dc.zzzz() * ps4567;
	}

	int steps = right - left;

	m_slenv.steps += steps;

	WORD* fb = &m_slenv.vm[(top << 10) + left];

	while(1)
	{
		do
		{
			int pixels = GSVector4i::store(GSVector4i::load(steps).min_i16(GSVector4i::load(8)));

			GSVector4i test = GSVector4i::zero();

			GSVector4i d = GSVector4i::zero();

			if(m_sel.rfb) // me | abe
			{
				d = GSVector4i::load<false>(fb);

				if(m_sel.me)
				{
					test = d.sra16(15);

					if(test.alltrue())
					{
						continue;
					}
				}
			}

			GSVector4i c[4];

			if(m_sel.tme)
			{
				SampleTexture(pixels, m_sel.ltf, m_sel.tlu, m_sel.twin, test, s, t, c);
			}

			ColorTFX(m_sel.tfx, r, g, b, c);

			if(m_sel.abe)
			{
				AlphaBlend(m_sel.abr, m_sel.tme, d, c);
			}

			WriteFrame(fb, test, c, pixels);
		}
		while(0);

		if(steps <= 8) break;

		steps -= 8;

		fb += 8;

		if(m_sel.tme)
		{
			GSVector4 dp8 = m_slenv.dp8;

			s[0] += dp8.zzzz();
			t[0] += dp8.wwww();
			s[1] += dp8.zzzz();
			t[1] += dp8.wwww();
		}

		if(m_sel.iip)
		{
			GSVector4 dc8 = m_slenv.dc8;

			r[0] += dc8.xxxx();
			g[0] += dc8.yyyy();
			b[0] += dc8.zzzz();
			r[1] += dc8.xxxx();
			g[1] += dc8.yyyy();
			b[1] += dc8.zzzz();
		}
	}
}

template<DWORD sel> 
void GPURasterizer::DrawScanlineEx(int top, int left, int right, const Vertex& v)	
{
	DWORD iip = (sel >> 0) & 1;
	DWORD me = (sel >> 1) & 1;
	DWORD abe = (sel >> 2) & 1;
	DWORD abr = (sel >> 3) & 3;
	DWORD tge = (sel >> 5) & 1;
	DWORD tme = (sel >> 6) & 1;
	DWORD tlu = (sel >> 7) & 1;
	DWORD twin = (sel >> 8) & 1;
	DWORD rfb = (sel >> 1) & 3;
	DWORD tfx = (sel >> 5) & 3;

	GSVector4 ps0123 = GSVector4::ps0123();
	GSVector4 ps4567 = GSVector4::ps4567();

	GSVector4 s[2], t[2]; 
	
	GSVector4 vp = v.p;

	s[0] = vp.zzzz(); s[1] = s[0];
	t[0] = vp.wwww(); t[1] = t[0];
	
	if(tme)
	{
		GSVector4 dp = m_slenv.dp;

		s[0] += dp.zzzz() * ps0123;
		t[0] += dp.wwww() * ps0123;
		s[1] += dp.zzzz() * ps4567;
		t[1] += dp.wwww() * ps4567;
	}

	GSVector4 r[2], g[2], b[2];
	
	GSVector4 vc = v.c;

	r[0] = vc.xxxx(); r[1] = r[0];
	g[0] = vc.yyyy(); g[1] = g[0];
	b[0] = vc.zzzz(); b[1] = b[0];

	if(iip)
	{
		GSVector4 dc = m_slenv.dc;

		r[0] += dc.xxxx() * ps0123;
		g[0] += dc.yyyy() * ps0123;
		b[0] += dc.zzzz() * ps0123;
		r[1] += dc.xxxx() * ps4567;
		g[1] += dc.yyyy() * ps4567;
		b[1] += dc.zzzz() * ps4567;
	}

	int steps = right - left;

	m_slenv.steps += steps;

	WORD* fb = &m_slenv.vm[(top << 10) + left];

	while(1)
	{
		do
		{
			int pixels = GSVector4i::store(GSVector4i::load(steps).min_i16(GSVector4i::load(8)));

			GSVector4i test = GSVector4i::zero();

			GSVector4i d = GSVector4i::zero();

			if(rfb) // me | abe
			{
				d = GSVector4i::load<false>(fb);

				if(me)
				{
					test = d.sra16(15);

					if(test.alltrue())
					{
						continue;
					}
				}
			}

			GSVector4i c[4];

			if(tme)
			{
				SampleTexture(pixels, m_sel.ltf, tlu, twin, test, s, t, c); // TODO: ltf
			}

			ColorTFX(tfx, r, g, b, c);

			if(abe)
			{
				AlphaBlend(abr, tme, d, c);
			}

			WriteFrame(fb, test, c, pixels);
		}
		while(0);

		if(steps <= 8) break;

		steps -= 8;

		fb += 8;

		if(tme)
		{
			GSVector4 dp8 = m_slenv.dp8;

			s[0] += dp8.zzzz();
			t[0] += dp8.wwww();
			s[1] += dp8.zzzz();
			t[1] += dp8.wwww();
		}

		if(iip)
		{
			GSVector4 dc8 = m_slenv.dc8;

			r[0] += dc8.xxxx();
			g[0] += dc8.yyyy();
			b[0] += dc8.zzzz();
			r[1] += dc8.xxxx();
			g[1] += dc8.yyyy();
			b[1] += dc8.zzzz();
		}
	}
}


void GPURasterizer::SampleTexture(int pixels, DWORD ltf, DWORD tlu, DWORD twin, GSVector4i& test, const GSVector4* s, const GSVector4* t, GSVector4i* c)
{
	const void* RESTRICT tex = m_slenv.tex;
	const WORD* RESTRICT clut = m_slenv.clut;

	if(ltf)
	{
		GSVector4i cc[8];

		for(int j = 0; j < 2; j++)
		{
			GSVector4 ss = s[j] - 0.5f;
			GSVector4 tt = t[j] - 0.5f;

			GSVector4 uf = ss.floor();
			GSVector4 vf = tt.floor();

			GSVector4 uff = ss - uf;
			GSVector4 vff = tt - vf;

			GSVector4i u = GSVector4i(uf);
			GSVector4i v = GSVector4i(vf);
		
			GSVector4i u01 = GSVector4i(u).ps32(u + GSVector4i::x00000001());
			GSVector4i v01 = GSVector4i(v).ps32(v + GSVector4i::x00000001());

			if(twin)
			{
				u01 = (u01 & m_slenv.u[0]).add16(m_slenv.u[1]);
				v01 = (v01 & m_slenv.v[0]).add16(m_slenv.v[1]);
			}

			GSVector4i uv01 = u01.pu16(v01);

			GSVector4i addr0011 = uv01.upl8(uv01.zwxy());
			GSVector4i addr0110 = uv01.upl8(uv01.wzyx());

			GSVector4i c0011, c0110;

			#if _M_SSE >= 0x401

			if(tlu)
			{
				c0011 = addr0011.gather16_16((const BYTE*)tex).gather16_16(clut);
				c0110 = addr0110.gather16_16((const BYTE*)tex).gather16_16(clut);
			}
			else
			{
				c0011 = addr0011.gather16_16((const WORD*)tex);
				c0110 = addr0110.gather16_16((const WORD*)tex);
			}

			#else

			int i = 0;

			if(tlu)
			{
				do
				{
					c0011.u16[i] = clut[((const BYTE*)tex)[addr0011.u16[i]]];
					c0110.u16[i] = clut[((const BYTE*)tex)[addr0110.u16[i]]];
				}
				while(++i < 8);
			}
			else
			{
				do
				{
					c0011.u16[i] = ((const WORD*)tex)[addr0011.u16[i]];
					c0110.u16[i] = ((const WORD*)tex)[addr0110.u16[i]];
				}
				while(++i < 8);
			}

			#endif

			GSVector4i r0011 = GSVector4i(c0011 & 0x001f001f) << 3;
			GSVector4i g0011 = GSVector4i(c0011 & 0x03e003e0) >> 2;
			GSVector4i b0011 = GSVector4i(c0011 & 0x7c007c00) >> 7;
			GSVector4i a0011 = GSVector4i(c0011 & 0x80008000);

			GSVector4i r0110 = GSVector4i(c0110 & 0x001f001f) << 3;
			GSVector4i g0110 = GSVector4i(c0110 & 0x03e003e0) >> 2;
			GSVector4i b0110 = GSVector4i(c0110 & 0x7c007c00) >> 7;
			GSVector4i a0110 = GSVector4i(c0110 & 0x80008000);

			GSVector4 r00 = GSVector4(r0011.upl16());
			GSVector4 g00 = GSVector4(g0011.upl16());
			GSVector4 b00 = GSVector4(b0011.upl16());
			GSVector4 a00 = GSVector4(a0011.upl16());

			GSVector4 r01 = GSVector4(r0110.upl16());
			GSVector4 g01 = GSVector4(g0110.upl16());
			GSVector4 b01 = GSVector4(b0110.upl16());
			GSVector4 a01 = GSVector4(a0110.upl16());

			GSVector4 r10 = GSVector4(r0110.uph16());
			GSVector4 g10 = GSVector4(g0110.uph16());
			GSVector4 b10 = GSVector4(b0110.uph16());
			GSVector4 a10 = GSVector4(a0110.uph16());

			GSVector4 r11 = GSVector4(r0011.uph16());
			GSVector4 g11 = GSVector4(g0011.uph16());
			GSVector4 b11 = GSVector4(b0011.uph16());
			GSVector4 a11 = GSVector4(a0011.uph16());

			r00 = r00.lerp(r01, vff);
			r10 = r10.lerp(r11, vff);
			r00 = r00.lerp(r10, uff);

			cc[j * 4 + 0] = GSVector4i(r00);

			g00 = g00.lerp(g01, vff);
			g10 = g10.lerp(g11, vff);
			g00 = g00.lerp(g10, uff);

			cc[j * 4 + 1] = GSVector4i(g00);

			b00 = b00.lerp(b01, vff);
			b10 = b10.lerp(b11, vff);
			b00 = b00.lerp(b10, uff);

			cc[j * 4 + 2] = GSVector4i(b00);

			a00 = a00.lerp(a01, vff);
			a10 = a10.lerp(a11, vff);
			a00 = a00.lerp(a10, uff);

			cc[j * 4 + 3] = GSVector4i(a00);
		}

		c[0] = cc[0].ps32(cc[4]);
		c[1] = cc[1].ps32(cc[5]);
		c[2] = cc[2].ps32(cc[6]);
		c[3] = cc[3].ps32(cc[7]).gt16(GSVector4i::zero());

		// mask out blank pixels (not perfect)

		test |= 
			c[0].eq16(GSVector4i::zero()) & 
			c[1].eq16(GSVector4i::zero()) &
			c[2].eq16(GSVector4i::zero()) & 
			c[3].eq16(GSVector4i::zero());
	}
	else
	{
		GSVector4i u, v;
		
		u = GSVector4i(s[0]).ps32(GSVector4i(s[1]));
		v = GSVector4i(t[0]).ps32(GSVector4i(t[1]));

		if(twin)
		{
			u = (u & m_slenv.u[0]).add16(m_slenv.u[1]);
			v = (v & m_slenv.v[0]).add16(m_slenv.v[1]);
		}

		GSVector4i uv = u.pu16(v);

		GSVector4i addr = uv.upl8(uv.zwxy());

		GSVector4i c00;

		#if _M_SSE >= 0x401

		if(tlu)
		{
			c00 = addr.gather16_16((const BYTE*)tex).gather16_16(clut);
		}
		else
		{
			c00 = addr.gather16_16((const WORD*)tex);
		}

		#else

		int i = 0;

		if(tlu)
		{
			do
			{
				if(test.u16[i]) // me && 
				{
					continue;
				}

				c00.u16[i] = clut[((const BYTE*)tex)[addr.u16[i]]];
			}
			while(++i < pixels);
		}
		else
		{
			do
			{
				if(test.u16[i]) // me && 
				{
					continue;
				}

				c00.u16[i] = ((const WORD*)tex)[addr.u16[i]];
			}
			while(++i < pixels);
		}

		#endif

		test |= c00.eq16(GSVector4i::zero()); // mask out blank pixels

		c[0] = (c00 & 0x001f001f) << 3;
		c[1] = (c00 & 0x03e003e0) >> 2;
		c[2] = (c00 & 0x7c007c00) >> 7;
		c[3] = c00.sra16(15);
	}
}

void GPURasterizer::ColorTFX(DWORD tfx, const GSVector4* r, const GSVector4* g, const GSVector4* b, GSVector4i* c)
{
	GSVector4i ri, gi, bi;

	switch(tfx)
	{
	case 0: // none (tfx = 0)
	case 1: // none (tfx = tge)
		ri = GSVector4i(r[0]).ps32(GSVector4i(r[1]));
		gi = GSVector4i(g[0]).ps32(GSVector4i(g[1]));
		bi = GSVector4i(b[0]).ps32(GSVector4i(b[1]));
		c[0] = ri;
		c[1] = gi;
		c[2] = bi;
		break;
	case 2: // modulate (tfx = tme | tge)
		ri = GSVector4i(r[0]).ps32(GSVector4i(r[1]));
		gi = GSVector4i(g[0]).ps32(GSVector4i(g[1]));
		bi = GSVector4i(b[0]).ps32(GSVector4i(b[1]));
		c[0] = c[0].mul16l(ri).srl16(7);
		c[1] = c[1].mul16l(gi).srl16(7);
		c[2] = c[2].mul16l(bi).srl16(7);
		c[0] = c[0].pu16().upl8();
		c[1] = c[1].pu16().upl8();
		c[2] = c[2].pu16().upl8();
		break;
	case 3: // decal (tfx = tme)
		break;
	default:
		__assume(0);
	}
}
void GPURasterizer::AlphaBlend(UINT32 abr, UINT32 tme, const GSVector4i& d, GSVector4i* c)
{
	GSVector4i r = (d & 0x001f001f) << 3;
	GSVector4i g = (d & 0x03e003e0) >> 2;
	GSVector4i b = (d & 0x7c007c00) >> 7;

	switch(abr)
	{
	case 0:
		r = r.add16(c[0]).srl16(1).min_i16(GSVector4i::x00ff());
		g = g.add16(c[1]).srl16(1).min_i16(GSVector4i::x00ff());
		b = b.add16(c[2]).srl16(1).min_i16(GSVector4i::x00ff());
		break;
	case 1:
		r = r.add16(c[0]).min_i16(GSVector4i::x00ff());
		g = g.add16(c[1]).min_i16(GSVector4i::x00ff());
		b = b.add16(c[2]).min_i16(GSVector4i::x00ff());
		break;
	case 2:
		r = r.sub16(c[0]).max_i16(GSVector4i::zero());
		g = g.sub16(c[1]).max_i16(GSVector4i::zero());
		b = b.sub16(c[2]).max_i16(GSVector4i::zero());
		break;
	case 3:
		r = r.add16(c[0].srl16(2)).min_i16(GSVector4i::x00ff());
		g = g.add16(c[1].srl16(2)).min_i16(GSVector4i::x00ff());
		b = b.add16(c[2].srl16(2)).min_i16(GSVector4i::x00ff());
		break;
	default:
		__assume(0);
	}

	if(tme) // per pixel
	{
		c[0] = c[0].blend8(r, c[3]);
		c[1] = c[1].blend8(g, c[3]);
		c[2] = c[2].blend8(b, c[3]);
	}
	else
	{
		c[0] = r;
		c[1] = g;
		c[2] = b;
		c[3] = GSVector4i::zero();
	}
}

void GPURasterizer::WriteFrame(WORD* RESTRICT fb, const GSVector4i& test, const GSVector4i* c, int pixels)
{
	GSVector4i r = (c[0] & 0x00f800f8) >> 3;
	GSVector4i g = (c[1] & 0x00f800f8) << 2;
	GSVector4i b = (c[2] & 0x00f800f8) << 7;
	GSVector4i a = (c[3] & 0x00800080) << 8;

	GSVector4i s = r | g | b | a | m_slenv.md;

	int i = 0;

	do
	{
		if(test.u16[i] == 0)
		{
			fb[i] = s.u16[i];
		}
	}
	while(++i < pixels);
}
