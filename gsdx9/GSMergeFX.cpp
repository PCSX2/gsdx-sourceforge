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
#include "GSMergeFX.h"
#include "GSDeviceDX9.h"
#include "resource.h"

GSMergeFX::GSMergeFX()
	: m_dev(NULL)
{
}

bool GSMergeFX::Create(GSDeviceDX9* dev)
{
	m_dev = dev;

	return true;
}

void GSMergeFX::Draw(GSTextureDX9* st, GSVector4* sr, GSTextureDX9& dt, PSSelector sel, PSConstantBuffer& cb)
{
	HRESULT hr;

	m_dev->BeginScene();

	// om

	m_dev->OMSetDepthStencilState(&m_dev->m_convert.dss, 0);
	m_dev->OMSetBlendState(&m_dev->m_convert.bs, 0);
	m_dev->OMSetRenderTargets(dt, NULL);
/*
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
*/
	m_dev->VSSetShader(NULL, NULL, 0); // TODO

	// ps

	CComPtr<IDirect3DPixelShader9> ps;

	if(!(ps = m_ps.Lookup(sel)))
	{
		CStringA str[4];

		str[0].Format("%d", sel.en1);
		str[1].Format("%d", sel.en2);
		str[2].Format("%d", sel.slbg);
		str[3].Format("%d", sel.mmod);

		D3DXMACRO macro[] =
		{
			{"EN1", str[0]},
			{"EN2", str[1]},
			{"SLBG", str[2]},
			{"MMOD", str[3]},
			{NULL, NULL},
		};

		hr = m_dev->CompileShader(IDR_MERGE_FX, "ps_main", macro, &ps);

		ASSERT(SUCCEEDED(hr));

		m_ps.Add(sel, ps);
	}

	m_dev->PSSetShader(ps, (const float*)&cb, sizeof(cb) / sizeof(GSVector4));
	m_dev->PSSetSamplerState(&m_dev->m_convert.ln);
	m_dev->PSSetShaderResources(st[0], st[1]);

	// rs

	m_dev->RSSet(dt.GetWidth(), dt.GetHeight());

	//

	float w = (float)dt.GetWidth();
	float h = (float)dt.GetHeight();

	GSVertexPT2 vertices[] =
	{
		{GSVector4(0, 0), GSVector2(sr[0].x, sr[0].y), GSVector2(sr[1].x, sr[1].y)},
		{GSVector4(w, 0), GSVector2(sr[0].z, sr[0].y), GSVector2(sr[1].z, sr[1].y)},
		{GSVector4(w, h), GSVector2(sr[0].z, sr[0].w), GSVector2(sr[1].z, sr[1].w)},
		{GSVector4(0, h), GSVector2(sr[0].x, sr[0].w), GSVector2(sr[1].x, sr[1].w)},
	};

	for(int i = 0; i < countof(vertices); i++)
	{
		vertices[i].p.x -= 0.5f;
		vertices[i].p.y -= 0.5f;
	}

	(*m_dev)->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX2);

	(*m_dev)->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(vertices[0]));

	//

	m_dev->EndScene();
}
