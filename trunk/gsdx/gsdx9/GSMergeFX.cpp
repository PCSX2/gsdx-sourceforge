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

	hr = (*m_dev)->SetRenderTarget(0, dt);
	hr = (*m_dev)->SetDepthStencilSurface(NULL);

	hr = (*m_dev)->SetTexture(0, st[0]);
	hr = (*m_dev)->SetTexture(1, st[1]);

    hr = (*m_dev)->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    hr = (*m_dev)->SetRenderState(D3DRS_LIGHTING, FALSE);
	hr = (*m_dev)->SetRenderState(D3DRS_ZENABLE, FALSE);
	hr = (*m_dev)->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	hr = (*m_dev)->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE); 
	hr = (*m_dev)->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
	hr = (*m_dev)->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RGBA);

	hr = (*m_dev)->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	hr = (*m_dev)->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	hr = (*m_dev)->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	hr = (*m_dev)->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

	hr = (*m_dev)->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	hr = (*m_dev)->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	hr = (*m_dev)->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	hr = (*m_dev)->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	hr = (*m_dev)->SetVertexShader(NULL);
	
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
	
	(*m_dev)->SetPixelShader(ps);

	(*m_dev)->SetPixelShaderConstantF(0, (const float*)&cb, sizeof(cb) / sizeof(GSVector4));

	int w = dt.m_desc.Width;
	int h = dt.m_desc.Height;

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

	(*m_dev)->BeginScene();
	(*m_dev)->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX2);
	(*m_dev)->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(vertices[0]));
	(*m_dev)->EndScene();
}
