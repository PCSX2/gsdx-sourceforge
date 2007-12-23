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
#include "GSDeviceDX10.h"
#include "resource.h"

GSDeviceDX10::GSDeviceDX10()
	: m_vb(NULL)
	, m_vb_count(0)
	, m_vb_stride(0)
	, m_layout(NULL)
	, m_topology(D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED)
	, m_vs(NULL)
	, m_vs_cb(NULL)
	, m_gs(NULL)
	, m_ps(NULL)
	, m_ps_cb(NULL)
	, m_ps_ss(NULL)
	, m_scissor(0, 0, 0, 0)
	, m_viewport(0, 0)
	, m_dss(NULL)
	, m_sref(0)
	, m_bs(NULL)
	, m_bf(-1)
	, m_rtv(NULL)
	, m_dsv(NULL)
{
	memset(m_ps_srvs, 0, sizeof(m_ps_srvs));
}

GSDeviceDX10::~GSDeviceDX10()
{
}

bool GSDeviceDX10::Create(HWND hWnd)
{
	if(!__super::Create(hWnd))
	{
		return false;
	}

	HRESULT hr;

	DXGI_SWAP_CHAIN_DESC scd;
	D3D10_BUFFER_DESC bd;
	D3D10_SAMPLER_DESC sd;
	D3D10_DEPTH_STENCIL_DESC dsd;
    D3D10_RASTERIZER_DESC rd;
	D3D10_BLEND_DESC bsd;

	memset(&scd, 0, sizeof(scd));

    scd.BufferCount = 2;
    scd.BufferDesc.Width = 1;
    scd.BufferDesc.Height = 1;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    // scd.BufferDesc.RefreshRate.Numerator = 60;
    // scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hWnd;
    scd.SampleDesc.Count = 1;
    scd.SampleDesc.Quality = 0;
    scd.Windowed = TRUE;

	UINT flags = 0;

#ifdef DEBUG
	flags |= D3D10_CREATE_DEVICE_DEBUG;
#endif

	hr = D3D10CreateDeviceAndSwapChain(NULL, D3D10_DRIVER_TYPE_HARDWARE, NULL, flags, D3D10_SDK_VERSION, &scd, &m_swapchain, &m_dev);

	if(FAILED(hr)) return false;


	// convert

	D3D10_INPUT_ELEMENT_DESC il_convert[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D10_INPUT_PER_VERTEX_DATA, 0},
	};

	hr = CompileShader(IDR_CONVERT_FX, "vs_main", NULL, &m_convert.vs, il_convert, countof(il_convert), &m_convert.il);

	for(int i = 0; i < countof(m_convert.ps); i++)
	{
		CStringA main;
		main.Format("ps_main%d", i);
		hr = CompileShader(IDR_CONVERT_FX, main, NULL, &m_convert.ps[i]);
	}

	memset(&bd, 0, sizeof(bd));

	bd.Usage = D3D10_USAGE_DEFAULT;
	bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.ByteWidth = 4 * sizeof(GSVertexPT1);

	hr = m_dev->CreateBuffer(&bd, NULL, &m_convert.vb);

	memset(&dsd, 0, sizeof(dsd));

	dsd.DepthEnable = false;
	dsd.StencilEnable = false;

	hr = m_dev->CreateDepthStencilState(&dsd, &m_convert.dss);

	memset(&bsd, 0, sizeof(bsd));

	bsd.BlendEnable[0] = false;
	bsd.RenderTargetWriteMask[0] = D3D10_COLOR_WRITE_ENABLE_ALL;

	hr = m_dev->CreateBlendState(&bsd, &m_convert.bs);

	// interlace

	memset(&bd, 0, sizeof(bd));

    bd.ByteWidth = sizeof(InterlaceConstantBuffer);
    bd.Usage = D3D10_USAGE_DEFAULT;
    bd.BindFlags = D3D10_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;

    hr = m_dev->CreateBuffer(&bd, NULL, &m_interlace.cb);

	for(int i = 0; i < countof(m_interlace.ps); i++)
	{
		CStringA main;
		main.Format("ps_main%d", i);
		hr = CompileShader(IDR_INTERLACE_FX, main, NULL, &m_interlace.ps[i]);
	}

	//

	memset(&rd, 0, sizeof(rd));

	rd.FillMode = D3D10_FILL_SOLID;
	rd.CullMode = D3D10_CULL_NONE;
	rd.FrontCounterClockwise = false;
	rd.DepthBias = false;
	rd.DepthBiasClamp = 0;
	rd.SlopeScaledDepthBias = 0;
	rd.DepthClipEnable = false; // ???
	rd.ScissorEnable = true;
	rd.MultisampleEnable = false;
	rd.AntialiasedLineEnable = false;

	hr = m_dev->CreateRasterizerState(&rd, &m_rs);

	m_dev->RSSetState(m_rs);

	//

	memset(&sd, 0, sizeof(sd));

	sd.Filter = D3D10_FILTER_MIN_MAG_MIP_LINEAR;
	sd.AddressU = D3D10_TEXTURE_ADDRESS_CLAMP;
	sd.AddressV = D3D10_TEXTURE_ADDRESS_CLAMP;
	sd.AddressW = D3D10_TEXTURE_ADDRESS_CLAMP;
	sd.MaxLOD = FLT_MAX;
	sd.MaxAnisotropy = 16; 
	sd.ComparisonFunc = D3D10_COMPARISON_NEVER;

	hr = m_dev->CreateSamplerState(&sd, &m_convert.ln);

	sd.Filter = D3D10_FILTER_MIN_MAG_MIP_POINT;

	hr = m_dev->CreateSamplerState(&sd, &m_convert.pt);

	//

	Reset(1, 1, true);

	//

	if(!m_mergefx.Create(this))
	{
		return false;
	}

	//

	return true;
}

bool GSDeviceDX10::Reset(int w, int h, bool fs)
{
	if(!__super::Reset(w, h, fs))
		return false;

	DXGI_SWAP_CHAIN_DESC scd;
	memset(&scd, 0, sizeof(scd));
	m_swapchain->GetDesc(&scd);
	m_swapchain->ResizeBuffers(scd.BufferCount, w, h, scd.BufferDesc.Format, 0);
	
	CComPtr<ID3D10Texture2D> backbuffer;
	m_swapchain->GetBuffer(0, __uuidof(ID3D10Texture2D), (void**)&backbuffer);
	m_backbuffer = GSTextureDX10(backbuffer);

	return true;
}

void GSDeviceDX10::Present(int arx, int ary)
{
	CRect cr;

	GetClientRect(m_hWnd, &cr);

	if(m_backbuffer.GetWidth() != cr.Width() || m_backbuffer.GetHeight() != cr.Height())
	{
		Reset(cr.Width(), cr.Height(), false);		
	}

	float color[4] = {0, 0, 0, 0};

	m_dev->ClearRenderTargetView(m_backbuffer, color);

	if(m_current)
	{
		CRect r = cr;

		if(arx > 0 && ary > 0)
		{
			if(r.Width() * ary > r.Height() * arx)
			{
				int w = r.Height() * arx / ary;
				r.left = r.CenterPoint().x - w / 2;
				if(r.left & 1) r.left++;
				r.right = r.left + w;
			}
			else
			{
				int h = r.Width() * ary / arx;
				r.top = r.CenterPoint().y - h / 2;
				if(r.top & 1) r.top++;
				r.bottom = r.top + h;
			}
		}

		r &= cr;

		StretchRect(m_current, m_backbuffer, GSVector4(r));
	}

	m_swapchain->Present(0, 0);
}

void GSDeviceDX10::BeginScene()
{
}

void GSDeviceDX10::EndScene()
{
	PSSetShaderResources(NULL, NULL);

	OMSetRenderTargets(NULL, NULL);
}

bool GSDeviceDX10::Create(int type, GSTextureDX10& t, int w, int h, int format)
{
	HRESULT hr;

	D3D10_TEXTURE2D_DESC desc;

	memset(&desc, 0, sizeof(desc));

	desc.Width = w;
	desc.Height = h;
	desc.Format = (DXGI_FORMAT)format;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D10_USAGE_DEFAULT;

	switch(type)
	{
	case GSTexture::RenderTarget:
		desc.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
		break;
	case GSTexture::DepthStencil:
		desc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
		break;
	case GSTexture::Texture:
		desc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
		break;
	case GSTexture::Offscreen:
		desc.Usage = D3D10_USAGE_STAGING;
		desc.CPUAccessFlags |= D3D10_CPU_ACCESS_READ | D3D10_CPU_ACCESS_WRITE;
		break;
	}

	CComPtr<ID3D10Texture2D> texture;

	hr = m_dev->CreateTexture2D(&desc, NULL, &texture);

	if(SUCCEEDED(hr))
	{
		t = GSTextureDX10(texture);

		float color[4] = {0, 0, 0, 0};

		switch(type)
		{
		case GSTexture::RenderTarget:
			m_dev->ClearRenderTargetView(t, color);
			break;
		case GSTexture::DepthStencil:
			m_dev->ClearDepthStencilView(t, D3D10_CLEAR_DEPTH, 0, 0);
			break;
		}

		return true;
	}

	return false;
}

bool GSDeviceDX10::CreateRenderTarget(GSTextureDX10& t, int w, int h, int format)
{
	return __super::CreateRenderTarget(t, w, h, format ? format : DXGI_FORMAT_R8G8B8A8_UNORM);
}

bool GSDeviceDX10::CreateDepthStencil(GSTextureDX10& t, int w, int h, int format)
{
	return __super::CreateDepthStencil(t, w, h, format ? format : DXGI_FORMAT_D32_FLOAT_S8X24_UINT);
}

bool GSDeviceDX10::CreateTexture(GSTextureDX10& t, int w, int h, int format)
{
	return __super::CreateTexture(t, w, h, format ? format : DXGI_FORMAT_R8G8B8A8_UNORM);
}

bool GSDeviceDX10::CreateOffscreen(GSTextureDX10& t, int w, int h, int format)
{
	return __super::CreateOffscreen(t, w, h, format ? format : DXGI_FORMAT_R8G8B8A8_UNORM);
}

void GSDeviceDX10::DoMerge(GSTextureDX10* st, GSVector4* sr, GSTextureDX10& dt, bool en1, bool en2, bool slbg, bool mmod, GSVector4& c)
{
	GSMergeFX::PSSelector sel;

	sel.en1 = en1;
	sel.en2 = en2;
	sel.slbg = slbg;
	sel.mmod = mmod;

	GSMergeFX::PSConstantBuffer cb;

	cb.BGColor = c;

	m_mergefx.Draw(st, sr, dt, sel, cb);
}

void GSDeviceDX10::DoInterlace(GSTextureDX10& st, GSTextureDX10& dt, int shader, bool linear, float yoffset)
{
	GSVector4 sr(0, 0, 1, 1);
	GSVector4 dr(0, yoffset, (float)dt.GetWidth(), (float)dt.GetHeight() + yoffset);

	InterlaceConstantBuffer cb;

	cb.ZrH = GSVector2(0, 1.0f / dt.GetHeight());
	cb.hH = (float)dt.GetHeight() / 2;

	m_dev->UpdateSubresource(m_interlace.cb, 0, NULL, &cb, 0, 0);

	StretchRect(st, sr, dt, dr, m_interlace.ps[shader], m_interlace.cb, linear);
}

void GSDeviceDX10::IASetVertexBuffer(ID3D10Buffer* vb, UINT count, const void* vertices, UINT stride)
{
	D3D10_BOX box = {0, 0, 0, count * stride, 1, 1};

	m_dev->UpdateSubresource(vb, 0, &box, vertices, 0, 0);

	if(m_vb != vb || m_vb_stride != stride)
	{
		UINT offset = 0;

		m_dev->IASetVertexBuffers(0, 1, &vb, &stride, &offset);

		m_vb = vb;
		m_vb_stride = stride;
	}

	m_vb_count = count;
}

void GSDeviceDX10::IASetInputLayout(ID3D10InputLayout* layout)
{
	if(m_layout != layout)
	{
		m_dev->IASetInputLayout(layout);

		m_layout = layout;
	}
}

void GSDeviceDX10::IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY topology)
{
	if(m_topology != topology)
	{
		m_dev->IASetPrimitiveTopology(topology);

		m_topology = topology;
	}
}

void GSDeviceDX10::VSSetShader(ID3D10VertexShader* vs, ID3D10Buffer* vs_cb)
{
	if(m_vs != vs)
	{
		m_dev->VSSetShader(vs);

		m_vs = vs;
	}
	
	if(m_vs_cb != vs_cb)
	{
		m_dev->VSSetConstantBuffers(0, 1, &vs_cb);

		m_vs_cb = vs_cb;
	}
}

void GSDeviceDX10::GSSetShader(ID3D10GeometryShader* gs)
{
	if(m_gs != gs)
	{
		m_dev->GSSetShader(gs);

		m_gs = gs;
	}
}

void GSDeviceDX10::PSSetShaderResources(ID3D10ShaderResourceView* srv0, ID3D10ShaderResourceView* srv1)
{
	if(m_ps_srvs[0] != srv0 || m_ps_srvs[1] != srv1)
	{
		ID3D10ShaderResourceView* srvs[] = {srv0, srv1};
	
		m_dev->PSSetShaderResources(0, 2, srvs);

		m_ps_srvs[0] = srv0;
		m_ps_srvs[1] = srv1;
	}
}

void GSDeviceDX10::PSSetShader(ID3D10PixelShader* ps, ID3D10Buffer* ps_cb)
{
	if(m_ps != ps)
	{
		m_dev->PSSetShader(ps);

		m_ps = ps;
	}
	
	if(m_ps_cb != ps_cb)
	{
		m_dev->PSSetConstantBuffers(0, 1, &ps_cb);

		m_ps_cb = ps_cb;
	}
}

void GSDeviceDX10::PSSetSamplerState(ID3D10SamplerState* ss)
{
	if(m_ps_ss != ss)
	{
		m_dev->PSSetSamplers(0, 1, &ss);

		m_ps_ss = ss;
	}
}

void GSDeviceDX10::RSSet(int width, int height, const RECT* scissor)
{
	if(m_viewport.cx != width || m_viewport.cy != height)
	{
		D3D10_VIEWPORT vp;

		memset(&vp, 0, sizeof(vp));
		
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		vp.Width = width;
		vp.Height = height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;

		m_dev->RSSetViewports(1, &vp);

		m_viewport = CSize(width, height);
	}

	CRect r = scissor ? *scissor : CRect(0, 0, width, height);

	if(m_scissor != r)
	{
		m_dev->RSSetScissorRects(1, &r);

		m_scissor = r;
	}
}

void GSDeviceDX10::OMSetDepthStencilState(ID3D10DepthStencilState* dss, UINT sref)
{
	if(m_dss != dss || m_sref != sref)
	{
		m_dev->OMSetDepthStencilState(dss, sref);

		m_dss = dss;
		m_sref = sref;
	}
}

void GSDeviceDX10::OMSetBlendState(ID3D10BlendState* bs, float bf)
{
	if(m_bs != bs || m_bf != bf)
	{
		float BlendFactor[] = {bf, bf, bf, 0};

		m_dev->OMSetBlendState(bs, BlendFactor, 0xffffffff);

		m_bs = bs;
		m_bf = bf;
	}
}

void GSDeviceDX10::OMSetRenderTargets(ID3D10RenderTargetView* rtv, ID3D10DepthStencilView* dsv)
{
	if(m_rtv != rtv || m_dsv != dsv)
	{
		m_dev->OMSetRenderTargets(1, &rtv, dsv);

		m_rtv = rtv;
		m_dsv = dsv;
	}
}

void GSDeviceDX10::DrawPrimitive()
{
	m_dev->Draw(m_vb_count, 0);
}

void GSDeviceDX10::StretchRect(GSTextureDX10& st, GSTextureDX10& dt, const GSVector4& dr, bool linear)
{
	StretchRect(st, GSVector4(0, 0, 1, 1), dt, dr, m_convert.ps[0], NULL, linear);
}

void GSDeviceDX10::StretchRect(GSTextureDX10& st, const GSVector4& sr, GSTextureDX10& dt, const GSVector4& dr, bool linear)
{
	StretchRect(st, sr, dt, dr, m_convert.ps[0], NULL, linear);
}

void GSDeviceDX10::StretchRect(GSTextureDX10& st, const GSVector4& sr, GSTextureDX10& dt, const GSVector4& dr, ID3D10PixelShader* ps, ID3D10Buffer* ps_cb, bool linear)
{
	BeginScene();

	// om

	OMSetDepthStencilState(m_convert.dss, 0);
	OMSetBlendState(m_convert.bs, 0);
	OMSetRenderTargets(dt, NULL);

	// ia

	float left = dr.x * 2 / dt.GetWidth() - 1.0f;
	float top = 1.0f - dr.y * 2 / dt.GetHeight();
	float right = dr.z * 2 / dt.GetWidth() - 1.0f;
	float bottom = 1.0f - dr.w * 2 / dt.GetHeight();

	GSVertexPT1 vertices[] =
	{
		{GSVector4(left, top), GSVector2(sr.x, sr.y)},
		{GSVector4(right, top), GSVector2(sr.z, sr.y)},
		{GSVector4(left, bottom), GSVector2(sr.x, sr.w)},
		{GSVector4(right, bottom), GSVector2(sr.z, sr.w)},
	};

	IASetVertexBuffer(m_convert.vb, 4, vertices);
	IASetInputLayout(m_convert.il);
	IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// vs

	VSSetShader(m_convert.vs, NULL);

	// gs

	GSSetShader(NULL);

	// ps

	PSSetShader(ps, ps_cb);
	PSSetSamplerState(linear ? m_convert.ln : m_convert.pt);
	PSSetShaderResources(st, NULL);

	// rs

	RSSet(dt.GetWidth(), dt.GetHeight());

	//

	DrawPrimitive();

	//

	EndScene();
}

HRESULT GSDeviceDX10::CompileShader(UINT id, LPCSTR entry, D3D10_SHADER_MACRO* macro, ID3D10VertexShader** ps, D3D10_INPUT_ELEMENT_DESC* layout, int count, ID3D10InputLayout** il)
{
	HRESULT hr;

	CComPtr<ID3D10Blob> shader, error;

    hr = D3DX10CompileFromResource(AfxGetInstanceHandle(), MAKEINTRESOURCE(id), NULL, macro, NULL, entry, "vs_4_0", 0, 0, NULL, &shader, &error, NULL);
	
	if(error)
	{
		TRACE(_T("%s\n"), CString((LPCSTR)error->GetBufferPointer()));
	}

	if(FAILED(hr))
	{
		return hr;
	}

	hr = m_dev->CreateVertexShader((DWORD*)shader->GetBufferPointer(), shader->GetBufferSize(), ps);

	if(FAILED(hr))
	{
		return hr;
	}

	hr = m_dev->CreateInputLayout(layout, count, shader->GetBufferPointer(), shader->GetBufferSize(), il);

	if(FAILED(hr))
	{
		return hr;
	}

	return hr;
}

HRESULT GSDeviceDX10::CompileShader(UINT id, LPCSTR entry, D3D10_SHADER_MACRO* macro, ID3D10GeometryShader** gs)
{
	HRESULT hr;

	CComPtr<ID3D10Blob> shader, error;

    hr = D3DX10CompileFromResource(AfxGetInstanceHandle(), MAKEINTRESOURCE(id), NULL, macro, NULL, entry, "gs_4_0", 0, 0, NULL, &shader, &error, NULL);
	
	if(error)
	{
		TRACE(_T("%s\n"), CString((LPCSTR)error->GetBufferPointer()));
	}

	if(FAILED(hr))
	{
		return hr;
	}

	hr = m_dev->CreateGeometryShader((DWORD*)shader->GetBufferPointer(), shader->GetBufferSize(), gs);

	if(FAILED(hr))
	{
		return hr;
	}

	return hr;
}

HRESULT GSDeviceDX10::CompileShader(UINT id, LPCSTR entry, D3D10_SHADER_MACRO* macro, ID3D10PixelShader** ps)
{
	HRESULT hr;

	CComPtr<ID3D10Blob> shader, error;

    hr = D3DX10CompileFromResource(AfxGetInstanceHandle(), MAKEINTRESOURCE(id), NULL, macro, NULL, entry, "ps_4_0", 0, 0, NULL, &shader, &error, NULL);
	
	if(error)
	{
		TRACE(_T("%s\n"), CString((LPCSTR)error->GetBufferPointer()));
	}

	if(FAILED(hr))
	{
		return hr;
	}

	hr = m_dev->CreatePixelShader((DWORD*)shader->GetBufferPointer(), shader->GetBufferSize(), ps);

	if(FAILED(hr))
	{
		return hr;
	}

	return hr;
}

bool GSDeviceDX10::SaveToFileD32S8X24(ID3D10Texture2D* ds, LPCTSTR fn)
{
	HRESULT hr;

	D3D10_TEXTURE2D_DESC desc;

	memset(&desc, 0, sizeof(desc));

	ds->GetDesc(&desc);

	desc.Usage = D3D10_USAGE_STAGING;
	desc.BindFlags = 0;
	desc.CPUAccessFlags = D3D10_CPU_ACCESS_READ;

	CComPtr<ID3D10Texture2D> src, dst;

	hr = m_dev->CreateTexture2D(&desc, NULL, &src);

	m_dev->CopyResource(src, ds);

	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;

	hr = m_dev->CreateTexture2D(&desc, NULL, &dst);

	D3D10_MAPPED_TEXTURE2D sm, dm;

	hr = src->Map(0, D3D10_MAP_READ, 0, &sm);
	hr = dst->Map(0, D3D10_MAP_WRITE, 0, &dm);

	BYTE* s = (BYTE*)sm.pData;
	BYTE* d = (BYTE*)dm.pData;

	for(int y = 0; y < desc.Height; y++, s += sm.RowPitch, d += dm.RowPitch)
	{
		for(int x = 0; x < desc.Width; x++)
		{
			((float*)d)[x] = ((float*)s)[x*2];
		}
	}

	src->Unmap(0);
	dst->Unmap(0);

	return SUCCEEDED(D3DX10SaveTextureToFile(dst, D3DX10_IFF_BMP, fn));
}