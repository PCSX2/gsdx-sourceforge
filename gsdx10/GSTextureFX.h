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

#include "GSDeviceDX10.h"

class GSTextureFX
{
public:
	#pragma pack(push, 1)

	struct VSConstantBuffer
	{
		GSVector4 VertexScale;
		GSVector4 VertexOffset;
		GSVector2 TextureScale;
		float _pad[2];

		struct VSConstantBuffer() {memset(this, 0, sizeof(*this));}
	};

	struct PSConstantBuffer
	{
		GSVector4 FogColor;
		float MINU;
		float MAXU;
		float MINV;
		float MAXV;
		DWORD UMSK;
		DWORD UFIX;
		DWORD VMSK;
		DWORD VFIX;
		float TA0;
		float TA1;
		float AREF;
		float _pad[1];
		GSVector2 WH;
		GSVector2 rWrH;
		GSVector2 rWZ;
		GSVector2 ZrH;

		struct PSConstantBuffer() {memset(this, 0, sizeof(*this));}
	};

	union PSSelector
	{
		struct
		{
			DWORD fst:1;
			DWORD wms:2;
			DWORD wmt:2;
			DWORD bpp:3;
			DWORD aem:1;
			DWORD tfx:3;
			DWORD tcc:1;
			DWORD ate:1;
			DWORD atst:3;
			DWORD fog:1;
			DWORD clr1:1;
			DWORD fba:1;
			DWORD aout:1;
		};

		DWORD dw;

		operator DWORD() {return dw & 0x1fffff;}
	};

	union GSSelector
	{
		struct
		{
			DWORD iip:1;
			DWORD prim:2;
		};

		DWORD dw;

		operator DWORD() {return dw & 0x7;}
	};

	union PSSamplerSelector
	{
		struct
		{
			DWORD tau:1;
			DWORD tav:1;
			DWORD min:1;
			DWORD mag:1;
		};

		DWORD dw;

		operator DWORD() {return dw & 0xf;}
	};

	union OMDepthStencilSelector
	{
		struct
		{
			DWORD zte:1;
			DWORD ztst:2;
			DWORD zwe:1;
			DWORD date:1;
		};

		DWORD dw;

		operator DWORD() {return dw & 0x1f;}
	};

	union OMBlendSelector
	{
		struct
		{
			DWORD abe:1;
			DWORD a:2;
			DWORD b:2;
			DWORD c:2;
			DWORD d:2;
			DWORD wr:1;
			DWORD wg:1;
			DWORD wb:1;
			DWORD wa:1;
		};

		DWORD dw;

		operator DWORD() {return dw & 0x1fff;}
	};

	#pragma pack(pop)

private:
	GSDeviceDX10* m_dev;
	CComPtr<ID3D10InputLayout> m_il;
	CComPtr<ID3D10VertexShader> m_vs;
	CComPtr<ID3D10Buffer> m_vs_cb;
	CSimpleMap<DWORD, CComPtr<ID3D10GeometryShader> > m_gs;
	CSimpleMap<DWORD, CComPtr<ID3D10PixelShader> > m_ps;
	CComPtr<ID3D10Buffer> m_ps_cb;
	CSimpleMap<DWORD, CComPtr<ID3D10SamplerState> > m_ps_ss;
	CSimpleMap<DWORD, CComPtr<ID3D10DepthStencilState> > m_om_dss;	
	CSimpleMap<DWORD, CComPtr<ID3D10BlendState> > m_om_bs;	

	CComPtr<ID3D10Buffer> m_vb;
	int m_vb_max;

	VSConstantBuffer m_vs_cb_cache;
	PSConstantBuffer m_ps_cb_cache;
	
public:
	GSTextureFX();

	bool Create(GSDeviceDX10* dev);
	
	bool SetupIA(const GSVertexHW* vertices, int count, D3D10_PRIMITIVE_TOPOLOGY prim);
	bool SetupVS(const VSConstantBuffer* cb);
	bool SetupGS(GSSelector sel);
	bool SetupPS(PSSelector sel, const PSConstantBuffer* cb, PSSamplerSelector ssel, ID3D10ShaderResourceView* tex, ID3D10ShaderResourceView* pal);
	void UpdatePS(PSSelector sel, const PSConstantBuffer* cb, PSSamplerSelector ssel);
	void SetupRS(UINT w, UINT h, const RECT& scissor);
	void SetupOM(OMDepthStencilSelector dssel, OMBlendSelector bsel, float bf, ID3D10RenderTargetView* rtv, ID3D10DepthStencilView* dsv);
	void UpdateOM(OMDepthStencilSelector dssel, OMBlendSelector bsel, float bf);
};
