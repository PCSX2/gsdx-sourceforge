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

#include "stdafx.h"
#include "GSTextureFX.h"
#include "resource.h"

GSTextureFX::GSTextureFX()
	: m_dev(NULL)
{
	memset(m_vb_max, 0, sizeof(m_vb_max));
	m_vb_cur = 0;
	memset(&m_vs_cb_cache, 0, sizeof(m_vs_cb_cache));
	memset(&m_ps_cb_cache, 0, sizeof(m_ps_cb_cache));
}

bool GSTextureFX::Create(GSDevice* dev)
{
	m_dev = dev;

	// shaders

	HRESULT hr;

	D3D10_INPUT_ELEMENT_DESC il[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 16, D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 1, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 20, D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D10_INPUT_PER_VERTEX_DATA, 0},
	};

	hr = m_dev->CompileShader(&m_vs, IDR_TFX_FX, "vs_main", il, countof(il), &m_il);

	if(FAILED(hr)) return false;

	// buffers

	D3D10_BUFFER_DESC bd;

	memset(&bd, 0, sizeof(bd));

	bd.ByteWidth = sizeof(VSConstantBuffer);
	bd.Usage = D3D10_USAGE_DEFAULT;
	bd.BindFlags = D3D10_BIND_CONSTANT_BUFFER;

	hr = (*m_dev)->CreateBuffer(&bd, NULL, &m_vs_cb);

	if(FAILED(hr)) return false;

	memset(&bd, 0, sizeof(bd));

	bd.ByteWidth = sizeof(PSConstantBuffer);
	bd.Usage = D3D10_USAGE_DEFAULT;
	bd.BindFlags = D3D10_BIND_CONSTANT_BUFFER;

	hr = (*m_dev)->CreateBuffer(&bd, NULL, &m_ps_cb);

	if(FAILED(hr)) return false;

	return true;
}

bool GSTextureFX::SetupIA(const GSVertexHW* vertices, UINT count, D3D10_PRIMITIVE_TOPOLOGY prim)
{
	HRESULT hr;

	int i = m_vb_cur;

	m_vb_cur = (m_vb_cur + 1) % countof(m_vb);

	if(m_vb[i])
	{
		if(m_vb_max[i] < max(count, 100000))
		{
			(*m_dev)->Flush();

			m_vb[i] = NULL;
		}
	}

	if(!m_vb[i])
	{
		m_vb_max[i] = max(count, 100000);

		D3D10_BUFFER_DESC bd;

		memset(&bd, 0, sizeof(bd));

		bd.Usage = D3D10_USAGE_DEFAULT;
		bd.ByteWidth = m_vb_max[i] * sizeof(GSVertexHW);
		bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;

		hr = (*m_dev)->CreateBuffer(&bd, NULL, &m_vb[i]);

		if(FAILED(hr)) return false;
	}

	m_dev->IASet(m_vb[i], count, vertices, m_il, prim);

	return true;
}

bool GSTextureFX::SetupVS(const VSConstantBuffer* cb)
{
	if(memcmp(&m_vs_cb_cache, cb, sizeof(*cb)))
	{
		(*m_dev)->UpdateSubresource(m_vs_cb, 0, NULL, cb, 0, 0);

		memcpy(&m_vs_cb_cache, cb, sizeof(*cb));
	}

	m_dev->VSSet(m_vs, m_vs_cb);

	return true;
}

bool GSTextureFX::SetupGS(GSSelector sel)
{
	HRESULT hr;

	CComPtr<ID3D10GeometryShader> gs;

	if(sel.prim > 0 && (sel.iip == 0 || sel.prim == 3)) // geometry shader works in every case, but not needed
	{
		if(!(gs = m_gs.Lookup(sel)))
		{
			CStringA str[2];

			str[0].Format("%d", sel.iip);
			str[1].Format("%d", sel.prim);

			D3D10_SHADER_MACRO macro[] =
			{
				{"IIP", str[0]},
				{"PRIM", str[1]},
				{NULL, NULL},
			};

			hr = m_dev->CompileShader(&gs, IDR_TFX_FX, "gs_main", macro);

			m_gs.Add(sel, gs);
		}
	}

	m_dev->GSSet(gs);

	return true;
}

bool GSTextureFX::SetupPS(PSSelector sel, const PSConstantBuffer* cb, PSSamplerSelector ssel, ID3D10ShaderResourceView* srv, ID3D10ShaderResourceView* pal)
{
	if(memcmp(&m_ps_cb_cache, cb, sizeof(*cb)))
	{
		(*m_dev)->UpdateSubresource(m_ps_cb, 0, NULL, cb, 0, 0);

		memcpy(&m_ps_cb_cache, cb, sizeof(*cb));
	}

	(*m_dev)->PSSetConstantBuffers(0, 1, &m_ps_cb.p);

	m_dev->PSSetShaderResources(srv, pal);

	UpdatePS(sel, ssel);

	return true;
}

void GSTextureFX::UpdatePS(PSSelector sel, PSSamplerSelector ssel)
{
	HRESULT hr;

	CComPtr<ID3D10PixelShader> ps;

	if(!(ps = m_ps.Lookup(sel)))
	{
		CStringA str[12];

		str[0].Format("%d", sel.fst);
		str[1].Format("%d", sel.clamp);
		str[2].Format("%d", sel.bpp);
		str[3].Format("%d", sel.aem);
		str[4].Format("%d", sel.tfx);
		str[5].Format("%d", sel.tcc);
		str[6].Format("%d", sel.ate);
		str[7].Format("%d", sel.atst);
		str[8].Format("%d", sel.fog);
		str[9].Format("%d", sel.clr1);
		str[10].Format("%d", sel.fba);
		str[11].Format("%d", sel.aout);

		D3D10_SHADER_MACRO macro[] =
		{
			{"FST", str[0]},
			{"CLAMP", str[1]},
			{"BPP", str[2]},
			{"AEM", str[3]},
			{"TFX", str[4]},
			{"TCC", str[5]},
			{"ATE", str[6]},
			{"ATST", str[7]},
			{"FOG", str[8]},
			{"CLR1", str[9]},
			{"FBA", str[10]},
			{"AOUT", str[11]},
			{NULL, NULL},
		};

		hr = m_dev->CompileShader(&ps, IDR_TFX_FX, "ps_main", macro);

		ASSERT(SUCCEEDED(hr));

		m_ps.Add(sel, ps);
	}

	CComPtr<ID3D10SamplerState> ss;

	if(sel.tfx != 4)
	{
		if(sel.bpp >= 3) ssel.min = ssel.mag = 0;

		if(!(ss = m_ps_ss.Lookup(ssel)))
		{
			D3D10_SAMPLER_DESC sd;

			memset(&sd, 0, sizeof(sd));

			sd.AddressU = ssel.tau ? D3D10_TEXTURE_ADDRESS_WRAP : D3D10_TEXTURE_ADDRESS_CLAMP;
			sd.AddressV = ssel.tav ? D3D10_TEXTURE_ADDRESS_WRAP : D3D10_TEXTURE_ADDRESS_CLAMP;
			sd.AddressW = D3D10_TEXTURE_ADDRESS_CLAMP;

			sd.Filter = D3D10_ENCODE_BASIC_FILTER(
				(ssel.min ? D3D10_FILTER_TYPE_LINEAR : D3D10_FILTER_TYPE_POINT),
				(ssel.mag ? D3D10_FILTER_TYPE_LINEAR : D3D10_FILTER_TYPE_POINT),
				D3D10_FILTER_TYPE_POINT,
				false);

			sd.MaxLOD = FLT_MAX;
			sd.MaxAnisotropy = 16; 
			sd.ComparisonFunc = D3D10_COMPARISON_NEVER;

			hr = (*m_dev)->CreateSamplerState(&sd, &ss);

			m_ps_ss.Add(ssel, ss);
		}
	}

	m_dev->PSSet(ps, ss);
}

void GSTextureFX::SetupRS(UINT w, UINT h, const RECT& scissor)
{
	m_dev->RSSet(w, h, &scissor);
}

void GSTextureFX::SetupOM(OMDepthStencilSelector dssel, OMBlendSelector bsel, float bf, ID3D10RenderTargetView* rtv, ID3D10DepthStencilView* dsv)
{
	UpdateOM(dssel, bsel, bf);

	m_dev->OMSetRenderTargets(rtv, dsv);
}

void GSTextureFX::UpdateOM(OMDepthStencilSelector dssel, OMBlendSelector bsel, float bf)
{
	HRESULT hr;

	CComPtr<ID3D10DepthStencilState> dss;

	if(!(dss = m_om_dss.Lookup(dssel)))
	{
		D3D10_DEPTH_STENCIL_DESC dsd;

		memset(&dsd, 0, sizeof(dsd));

		if(dssel.date)
		{
			dsd.StencilEnable = true;
			dsd.StencilReadMask = 1;
			dsd.StencilWriteMask = 1;
			dsd.FrontFace.StencilFunc = D3D10_COMPARISON_EQUAL;
			dsd.FrontFace.StencilPassOp = D3D10_STENCIL_OP_KEEP;
			dsd.FrontFace.StencilFailOp = D3D10_STENCIL_OP_KEEP;
			dsd.FrontFace.StencilDepthFailOp = D3D10_STENCIL_OP_KEEP;
			dsd.BackFace.StencilFunc = D3D10_COMPARISON_EQUAL;
			dsd.BackFace.StencilPassOp = D3D10_STENCIL_OP_KEEP;
			dsd.BackFace.StencilFailOp = D3D10_STENCIL_OP_KEEP;
			dsd.BackFace.StencilDepthFailOp = D3D10_STENCIL_OP_KEEP;
		}

		if(!(dssel.zte && dssel.ztst == 1 && !dssel.zwe))
		{
			static const D3D10_COMPARISON_FUNC ztst[] = 
			{
				D3D10_COMPARISON_NEVER, 
				D3D10_COMPARISON_ALWAYS, 
				D3D10_COMPARISON_GREATER_EQUAL, 
				D3D10_COMPARISON_GREATER
			};

			dsd.DepthEnable = dssel.zte;
			dsd.DepthWriteMask = dssel.zwe ? D3D10_DEPTH_WRITE_MASK_ALL : D3D10_DEPTH_WRITE_MASK_ZERO;
			dsd.DepthFunc = ztst[dssel.ztst];
		}

		hr = (*m_dev)->CreateDepthStencilState(&dsd, &dss);

		m_om_dss.Add(dssel, dss);
	}

	CComPtr<ID3D10BlendState> bs;

	if(!(bs = m_om_bs.Lookup(bsel)))
	{
		D3D10_BLEND_DESC bd;

		memset(&bd, 0, sizeof(bd));

		bd.BlendEnable[0] = bsel.abe;

		if(bsel.abe)
		{
			// (A:Cs/Cd/0 - B:Cs/Cd/0) * C:As/Ad/FIX + D:Cs/Cd/0

			static const struct {int bogus; D3D10_BLEND_OP op; D3D10_BLEND src, dst;} map[3*3*3*3] =
			{
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ONE, D3D10_BLEND_ZERO},							// 0000: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cs ==> Cs
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ZERO, D3D10_BLEND_ONE},							// 0001: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cd ==> Cd
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ZERO, D3D10_BLEND_ZERO},						// 0002: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + 0 ==> 0
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ONE, D3D10_BLEND_ZERO},							// 0010: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cs ==> Cs
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ZERO, D3D10_BLEND_ONE},							// 0011: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cd ==> Cd
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ZERO, D3D10_BLEND_ZERO},						// 0012: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + 0 ==> 0
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ONE, D3D10_BLEND_ZERO},							// 0020: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cs ==> Cs
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ZERO, D3D10_BLEND_ONE},							// 0021: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cd ==> Cd
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ZERO, D3D10_BLEND_ZERO},						// 0022: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + 0 ==> 0
				{1, D3D10_BLEND_OP_SUBTRACT, D3D10_BLEND_SRC1_ALPHA, D3D10_BLEND_SRC1_ALPHA},		// * 0100: (Cs - Cd)*As + Cs ==> Cs*(As + 1) - Cd*As
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_SRC1_ALPHA, D3D10_BLEND_INV_SRC1_ALPHA},		// 0101: (Cs - Cd)*As + Cd ==> Cs*As + Cd*(1 - As)
				{0, D3D10_BLEND_OP_SUBTRACT, D3D10_BLEND_SRC1_ALPHA, D3D10_BLEND_SRC1_ALPHA},		// 0102: (Cs - Cd)*As + 0 ==> Cs*As - Cd*As
				{1, D3D10_BLEND_OP_SUBTRACT, D3D10_BLEND_DEST_ALPHA, D3D10_BLEND_DEST_ALPHA},		// * 0110: (Cs - Cd)*Ad + Cs ==> Cs*(Ad + 1) - Cd*Ad
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_DEST_ALPHA, D3D10_BLEND_INV_DEST_ALPHA},		// 0111: (Cs - Cd)*Ad + Cd ==> Cs*Ad + Cd*(1 - Ad)
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_DEST_ALPHA, D3D10_BLEND_DEST_ALPHA},			// 0112: (Cs - Cd)*Ad + 0 ==> Cs*Ad - Cd*Ad
				{1, D3D10_BLEND_OP_SUBTRACT, D3D10_BLEND_BLEND_FACTOR, D3D10_BLEND_BLEND_FACTOR},	// * 0120: (Cs - Cd)*F + Cs ==> Cs*(F + 1) - Cd*F
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_BLEND_FACTOR, D3D10_BLEND_INV_BLEND_FACTOR},	// 0121: (Cs - Cd)*F + Cd ==> Cs*F + Cd*(1 - F)
				{0, D3D10_BLEND_OP_SUBTRACT, D3D10_BLEND_BLEND_FACTOR, D3D10_BLEND_BLEND_FACTOR},	// 0122: (Cs - Cd)*F + 0 ==> Cs*F - Cd*F
				{1, D3D10_BLEND_OP_ADD, D3D10_BLEND_SRC1_ALPHA, D3D10_BLEND_ZERO},					// * 0200: (Cs - 0)*As + Cs ==> Cs*(As + 1)
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_SRC1_ALPHA, D3D10_BLEND_ONE},					// 0201: (Cs - 0)*As + Cd ==> Cs*As + Cd
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_SRC1_ALPHA, D3D10_BLEND_ZERO},					// 0202: (Cs - 0)*As + 0 ==> Cs*As
				{1, D3D10_BLEND_OP_ADD, D3D10_BLEND_SRC1_ALPHA, D3D10_BLEND_ZERO},					// * 0210: (Cs - 0)*Ad + Cs ==> Cs*(Ad + 1)
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_DEST_ALPHA, D3D10_BLEND_ONE},					// 0211: (Cs - 0)*Ad + Cd ==> Cs*Ad + Cd
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_DEST_ALPHA, D3D10_BLEND_ZERO},					// 0212: (Cs - 0)*Ad + 0 ==> Cs*Ad
				{1, D3D10_BLEND_OP_ADD, D3D10_BLEND_BLEND_FACTOR, D3D10_BLEND_ZERO},				// * 0220: (Cs - 0)*F + Cs ==> Cs*(F + 1)
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_BLEND_FACTOR, D3D10_BLEND_ONE},					// 0221: (Cs - 0)*F + Cd ==> Cs*F + Cd
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_BLEND_FACTOR, D3D10_BLEND_ZERO},				// 0222: (Cs - 0)*F + 0 ==> Cs*F
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_INV_SRC1_ALPHA, D3D10_BLEND_SRC1_ALPHA},		// 1000: (Cd - Cs)*As + Cs ==> Cd*As + Cs*(1 - As)
				{1, D3D10_BLEND_OP_REV_SUBTRACT, D3D10_BLEND_SRC1_ALPHA, D3D10_BLEND_SRC1_ALPHA},	// * 1001: (Cd - Cs)*As + Cd ==> Cd*(As + 1) - Cs*As
				{0, D3D10_BLEND_OP_REV_SUBTRACT, D3D10_BLEND_SRC1_ALPHA, D3D10_BLEND_SRC1_ALPHA},	// 1002: (Cd - Cs)*As + 0 ==> Cd*As - Cs*As
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_INV_DEST_ALPHA, D3D10_BLEND_DEST_ALPHA},		// 1010: (Cd - Cs)*Ad + Cs ==> Cd*Ad + Cs*(1 - Ad)
				{1, D3D10_BLEND_OP_REV_SUBTRACT, D3D10_BLEND_DEST_ALPHA, D3D10_BLEND_DEST_ALPHA},	// * 1011: (Cd - Cs)*Ad + Cd ==> Cd*(Ad + 1) - Cs*Ad
				{0, D3D10_BLEND_OP_REV_SUBTRACT, D3D10_BLEND_DEST_ALPHA, D3D10_BLEND_DEST_ALPHA},	// 1012: (Cd - Cs)*Ad + 0 ==> Cd*Ad - Cs*Ad
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_INV_BLEND_FACTOR, D3D10_BLEND_BLEND_FACTOR},	// 1020: (Cd - Cs)*F + Cs ==> Cd*F + Cs*(1 - F)
				{1, D3D10_BLEND_OP_REV_SUBTRACT, D3D10_BLEND_BLEND_FACTOR, D3D10_BLEND_BLEND_FACTOR},// * 1021: (Cd - Cs)*F + Cd ==> Cd*(F + 1) - Cs*F
				{0, D3D10_BLEND_OP_REV_SUBTRACT, D3D10_BLEND_BLEND_FACTOR, D3D10_BLEND_BLEND_FACTOR},// 1022: (Cd - Cs)*F + 0 ==> Cd*F - Cs*F
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ONE, D3D10_BLEND_ZERO},							// 1100: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cs ==> Cs
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ZERO, D3D10_BLEND_ONE},							// 1101: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cd ==> Cd
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ZERO, D3D10_BLEND_ZERO},						// 1102: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + 0 ==> 0
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ONE, D3D10_BLEND_ZERO},							// 1110: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cs ==> Cs
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ZERO, D3D10_BLEND_ONE},							// 1111: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cd ==> Cd
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ZERO, D3D10_BLEND_ZERO},						// 1112: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + 0 ==> 0
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ONE, D3D10_BLEND_ZERO},							// 1120: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cs ==> Cs
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ZERO, D3D10_BLEND_ONE},							// 1121: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cd ==> Cd
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ZERO, D3D10_BLEND_ZERO},						// 1122: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + 0 ==> 0
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ONE, D3D10_BLEND_SRC1_ALPHA},					// 1200: (Cd - 0)*As + Cs ==> Cs + Cd*As
				{2, D3D10_BLEND_OP_ADD, D3D10_BLEND_DEST_COLOR, D3D10_BLEND_SRC1_ALPHA},			// ** 1201: (Cd - 0)*As + Cd ==> Cd*(1 + As)  // ffxii main menu background glow effect
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ZERO, D3D10_BLEND_SRC1_ALPHA},					// 1202: (Cd - 0)*As + 0 ==> Cd*As
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ONE, D3D10_BLEND_DEST_ALPHA},					// 1210: (Cd - 0)*Ad + Cs ==> Cs + Cd*Ad
				{2, D3D10_BLEND_OP_ADD, D3D10_BLEND_DEST_COLOR, D3D10_BLEND_DEST_ALPHA},			// ** 1211: (Cd - 0)*Ad + Cd ==> Cd*(1 + Ad)
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ZERO, D3D10_BLEND_DEST_ALPHA},					// 1212: (Cd - 0)*Ad + 0 ==> Cd*Ad
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ONE, D3D10_BLEND_BLEND_FACTOR},					// 1220: (Cd - 0)*F + Cs ==> Cs + Cd*F
				{2, D3D10_BLEND_OP_ADD, D3D10_BLEND_DEST_COLOR, D3D10_BLEND_BLEND_FACTOR},			// ** 1221: (Cd - 0)*F + Cd ==> Cd*(1 + F)
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ZERO, D3D10_BLEND_BLEND_FACTOR},				// 1222: (Cd - 0)*F + 0 ==> Cd*F
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_INV_SRC1_ALPHA, D3D10_BLEND_ZERO},				// 2000: (0 - Cs)*As + Cs ==> Cs*(1 - As)
				{0, D3D10_BLEND_OP_REV_SUBTRACT, D3D10_BLEND_SRC1_ALPHA, D3D10_BLEND_ONE},			// 2001: (0 - Cs)*As + Cd ==> Cd - Cs*As
				{0, D3D10_BLEND_OP_REV_SUBTRACT, D3D10_BLEND_SRC1_ALPHA, D3D10_BLEND_ZERO},			// 2002: (0 - Cs)*As + 0 ==> 0 - Cs*As
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_INV_DEST_ALPHA, D3D10_BLEND_ZERO},				// 2010: (0 - Cs)*Ad + Cs ==> Cs*(1 - Ad)
				{0, D3D10_BLEND_OP_REV_SUBTRACT, D3D10_BLEND_DEST_ALPHA, D3D10_BLEND_ONE},			// 2011: (0 - Cs)*Ad + Cd ==> Cd - Cs*Ad
				{0, D3D10_BLEND_OP_REV_SUBTRACT, D3D10_BLEND_DEST_ALPHA, D3D10_BLEND_ZERO},			// 2012: (0 - Cs)*Ad + 0 ==> 0 - Cs*Ad
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_INV_BLEND_FACTOR, D3D10_BLEND_ZERO},			// 2020: (0 - Cs)*F + Cs ==> Cs*(1 - F)
				{0, D3D10_BLEND_OP_REV_SUBTRACT, D3D10_BLEND_BLEND_FACTOR, D3D10_BLEND_ONE},		// 2021: (0 - Cs)*F + Cd ==> Cd - Cs*F
				{0, D3D10_BLEND_OP_REV_SUBTRACT, D3D10_BLEND_BLEND_FACTOR, D3D10_BLEND_ZERO},		// 2022: (0 - Cs)*F + 0 ==> 0 - Cs*F
				{0, D3D10_BLEND_OP_SUBTRACT, D3D10_BLEND_ONE, D3D10_BLEND_SRC1_ALPHA},				// 2100: (0 - Cd)*As + Cs ==> Cs - Cd*As
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ZERO, D3D10_BLEND_INV_SRC1_ALPHA},				// 2101: (0 - Cd)*As + Cd ==> Cd*(1 - As)
				{0, D3D10_BLEND_OP_SUBTRACT, D3D10_BLEND_ZERO, D3D10_BLEND_SRC1_ALPHA},				// 2102: (0 - Cd)*As + 0 ==> 0 - Cd*As
				{0, D3D10_BLEND_OP_SUBTRACT, D3D10_BLEND_ONE, D3D10_BLEND_DEST_ALPHA},				// 2110: (0 - Cd)*Ad + Cs ==> Cs - Cd*Ad
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ZERO, D3D10_BLEND_INV_DEST_ALPHA},				// 2111: (0 - Cd)*Ad + Cd ==> Cd*(1 - Ad)
				{0, D3D10_BLEND_OP_SUBTRACT, D3D10_BLEND_ONE, D3D10_BLEND_DEST_ALPHA},				// 2112: (0 - Cd)*Ad + 0 ==> 0 - Cd*Ad
				{0, D3D10_BLEND_OP_SUBTRACT, D3D10_BLEND_ONE, D3D10_BLEND_BLEND_FACTOR},			// 2120: (0 - Cd)*F + Cs ==> Cs - Cd*F
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ZERO, D3D10_BLEND_INV_BLEND_FACTOR},			// 2121: (0 - Cd)*F + Cd ==> Cd*(1 - F)
				{0, D3D10_BLEND_OP_SUBTRACT, D3D10_BLEND_ONE, D3D10_BLEND_BLEND_FACTOR},			// 2122: (0 - Cd)*F + 0 ==> 0 - Cd*F
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ONE, D3D10_BLEND_ZERO},							// 2200: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cs ==> Cs
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ZERO, D3D10_BLEND_ONE},							// 2201: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cd ==> Cd
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ZERO, D3D10_BLEND_ZERO},						// 2202: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + 0 ==> 0
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ONE, D3D10_BLEND_ZERO},							// 2210: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cs ==> Cs
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ZERO, D3D10_BLEND_ONE},							// 2211: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cd ==> Cd
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ZERO, D3D10_BLEND_ZERO},						// 2212: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + 0 ==> 0
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ONE, D3D10_BLEND_ZERO},							// 2220: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cs ==> Cs
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ZERO, D3D10_BLEND_ONE},							// 2221: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cd ==> Cd
				{0, D3D10_BLEND_OP_ADD, D3D10_BLEND_ZERO, D3D10_BLEND_ZERO},						// 2222: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + 0 ==> 0
			};

			// bogus: 0100, 0110, 0120, 0200, 0210, 0220, 1001, 1011, 1021

			// tricky: 1201, 1211, 1221
			//
			// Source.rgb = float3(1, 1, 1);
			// 1201 Cd*(1 + As) => Source * Dest color + Dest * Source1 alpha
			// 1211 Cd*(1 + Ad) => Source * Dest color + Dest * Dest alpha
			// 1221 Cd*(1 + F) => Source * Dest color + Dest * Factor

			int i = (((bsel.a & 3) * 3 + (bsel.b & 3)) * 3 + (bsel.c & 3)) * 3 + (bsel.d & 3);

			ASSERT(bsel.a != 3);
			ASSERT(bsel.b != 3);
			ASSERT(bsel.c != 3);
			ASSERT(bsel.d != 3);

			bd.BlendOp = map[i].op;
			bd.SrcBlend = map[i].src;
			bd.DestBlend = map[i].dst;
			bd.BlendOpAlpha = D3D10_BLEND_OP_ADD;
			bd.SrcBlendAlpha = D3D10_BLEND_ONE;
			bd.DestBlendAlpha = D3D10_BLEND_ZERO;

			if(map[i].bogus == 1)
			{
				ASSERT(0);

				(bsel.a == 0 ? bd.SrcBlend : bd.DestBlend) = D3D10_BLEND_ONE;
			}
		}

		if(bsel.wr) bd.RenderTargetWriteMask[0] |= D3D10_COLOR_WRITE_ENABLE_RED;
		if(bsel.wg) bd.RenderTargetWriteMask[0] |= D3D10_COLOR_WRITE_ENABLE_GREEN;
		if(bsel.wb) bd.RenderTargetWriteMask[0] |= D3D10_COLOR_WRITE_ENABLE_BLUE;
		if(bsel.wa) bd.RenderTargetWriteMask[0] |= D3D10_COLOR_WRITE_ENABLE_ALPHA;

		hr = (*m_dev)->CreateBlendState(&bd, &bs);

		m_om_bs.Add(bsel, bs);
	}

	m_dev->OMSet(dss, 1, bs, bf);
}
