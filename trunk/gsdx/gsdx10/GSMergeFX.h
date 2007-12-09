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

#include "GSDevice.h"

class GSMergeFX
{
public:
	#pragma pack(push, 1)

	struct PSConstantBuffer
	{
		D3DXVECTOR4 BGColor;
	};

	union PSSelector
	{
		struct
		{
			DWORD en1:1;
			DWORD en2:1;
			DWORD slbg:1;
			DWORD mmod:1;
		};

		DWORD dw;

		operator DWORD() {return dw & 0xf;}
	};

	#pragma pack(pop)

private:
	GSDevice* m_dev;
	CComPtr<ID3D10Buffer> m_vb;
	CComPtr<ID3D10InputLayout> m_il;
	CComPtr<ID3D10VertexShader> m_vs;
	CSimpleMap<DWORD, CComPtr<ID3D10PixelShader> > m_ps;
	CComPtr<ID3D10Buffer> m_ps_cb;

public:
	GSMergeFX();

	bool Create(GSDevice* dev);

	void Draw(GSTexture2D* st, D3DXVECTOR4* sr, GSTexture2D& dt, PSSelector sel, PSConstantBuffer& cb);
};
