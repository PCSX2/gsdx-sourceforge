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
#include "GSMergeFX10.h"
#include "GSDevice10.h"
#include "resource.h"

GSMergeFX10::GSMergeFX10()
	: m_dev(NULL)
{
}

bool GSMergeFX10::Create(GSDevice10* dev)
{
	m_dev = dev;

	HRESULT hr;

	D3D10_INPUT_ELEMENT_DESC il[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D10_INPUT_PER_VERTEX_DATA, 0},
	};

	hr = m_dev->CompileShader(IDR_MERGE10_FX, "vs_main", NULL, &m_vs, il, countof(il), &m_il);

	if(FAILED(hr)) return false;

	D3D10_BUFFER_DESC bd;

	memset(&bd, 0, sizeof(bd));

	bd.ByteWidth = sizeof(PSConstantBuffer);
	bd.Usage = D3D10_USAGE_DEFAULT;
	bd.BindFlags = D3D10_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	hr = (*m_dev)->CreateBuffer(&bd, NULL, &m_ps_cb);

	if(FAILED(hr)) return false;

	memset(&bd, 0, sizeof(bd));

	bd.Usage = D3D10_USAGE_DEFAULT;
	bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.ByteWidth = 4 * sizeof(GSVertexPT2);

	hr = (*m_dev)->CreateBuffer(&bd, NULL, &m_vb);

	if(FAILED(hr)) return false;

	return true;
}

void GSMergeFX10::Draw(GSTexture10* st, GSVector4* sr, GSTexture10& dt, PSSelector sel, PSConstantBuffer& cb)
{
	HRESULT hr;

	m_dev->BeginScene();

	// om

	m_dev->OMSetDepthStencilState(m_dev->m_convert.dss, 0);
	m_dev->OMSetBlendState(m_dev->m_convert.bs, 0);
	m_dev->OMSetRenderTargets(dt, NULL);

	// ia

	GSVertexPT2 vertices[] =
	{
		{GSVector4(-1, +1), GSVector2(sr[0].x, sr[0].y), GSVector2(sr[1].x, sr[1].y)},
		{GSVector4(+1, +1), GSVector2(sr[0].z, sr[0].y), GSVector2(sr[1].z, sr[1].y)},
		{GSVector4(-1, -1), GSVector2(sr[0].x, sr[0].w), GSVector2(sr[1].x, sr[1].w)},
		{GSVector4(+1, -1), GSVector2(sr[0].z, sr[0].w), GSVector2(sr[1].z, sr[1].w)},
	};

	m_dev->IASetVertexBuffer(m_vb, 4, vertices);
	m_dev->IASetInputLayout(m_il);
	m_dev->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// vs

	m_dev->VSSetShader(m_vs, NULL);

	// gs

	m_dev->GSSetShader(NULL);

	// ps

	CComPtr<ID3D10PixelShader> ps;

	if(!(ps = m_ps.Lookup(sel)))
	{
		CStringA str[4];

		str[0].Format("%d", sel.en1);
		str[1].Format("%d", sel.en2);
		str[2].Format("%d", sel.slbg);
		str[3].Format("%d", sel.mmod);

		D3D10_SHADER_MACRO macro[] =
		{
			{"EN1", str[0]},
			{"EN2", str[1]},
			{"SLBG", str[2]},
			{"MMOD", str[3]},
			{NULL, NULL},
		};

		hr = m_dev->CompileShader(IDR_MERGE10_FX, "ps_main", macro, &ps);

		ASSERT(SUCCEEDED(hr));

		m_ps.Add(sel, ps);
	}

	(*m_dev)->UpdateSubresource(m_ps_cb, 0, NULL, &cb, 0, 0);

	m_dev->PSSetShader(ps, m_ps_cb);
	m_dev->PSSetSamplerState(m_dev->m_convert.ln);
	m_dev->PSSetShaderResources(st[0], st[1]);

	// rs

	m_dev->RSSet(dt.GetWidth(), dt.GetHeight());

	//

	m_dev->DrawPrimitive();

	//

	m_dev->EndScene();
}
