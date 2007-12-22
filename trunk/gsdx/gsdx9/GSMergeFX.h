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

// FIXME
class GSDeviceDX9;
class GSTextureDX9;

class GSMergeFX
{
public:
	#pragma pack(push, 1)

	struct PSConstantBuffer
	{
		GSVector4 BGColor;
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
	GSDeviceDX9* m_dev;
	CComPtr<IDirect3DVertexDeclaration9> m_il;
	CComPtr<IDirect3DVertexShader9> m_vs;
	CSimpleMap<DWORD, CComPtr<IDirect3DPixelShader9> > m_ps;

public:
	GSMergeFX();

	bool Create(GSDeviceDX9* dev);
	void Draw(GSTextureDX9* st, GSVector4* sr, GSTextureDX9& dt, PSSelector sel, PSConstantBuffer& cb);
};
