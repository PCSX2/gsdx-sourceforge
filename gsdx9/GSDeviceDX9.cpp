#include "stdafx.h"
#include "GSDeviceDX9.h"
#include "resource.h"

GSDeviceDX9::GSDeviceDX9() 
	: m_vb(NULL)
	, m_vb_stride(0)
	, m_layout(NULL)
	, m_vs(NULL)
	, m_vs_cb(NULL)
	, m_vs_cb_len(0)
	, m_ps(NULL)
	, m_ps_cb(NULL)
	, m_ps_cb_len(0)
	, m_ps_ss(NULL)
	, m_scissor(0, 0, 0, 0)
	, m_dss(NULL)
	, m_sref(0)
	, m_bs(NULL)
	, m_bf(-1)
	, m_rtv(NULL)
	, m_dsv(NULL)
{
	memset(&m_pp, 0, sizeof(m_pp));
	memset(&m_ddcaps, 0, sizeof(m_ddcaps));
	memset(&m_d3dcaps, 0, sizeof(m_d3dcaps));
	memset(m_ps_srvs, 0, sizeof(m_ps_srvs));
}

GSDeviceDX9::~GSDeviceDX9()
{
	delete [] m_vs_cb;
	delete [] m_ps_cb;
}

bool GSDeviceDX9::Create(HWND hWnd)
{
	HRESULT hr;

	m_hWnd = hWnd;

	// dd

	CComPtr<IDirectDraw7> dd; 

	hr = DirectDrawCreateEx(0, (void**)&dd, IID_IDirectDraw7, 0);

	if(FAILED(hr)) return false;

	memset(&m_ddcaps, 0, sizeof(m_ddcaps));

	m_ddcaps.dwSize = sizeof(DDCAPS); 

	hr = dd->GetCaps(&m_ddcaps, NULL);

	if(FAILED(hr)) return false;

	dd = NULL;

	// d3d

	m_d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if(!m_d3d) return false;

	hr = m_d3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24S8);
		
	if(FAILED(hr)) return false;

	hr = m_d3d->CheckDepthStencilMatch(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, D3DFMT_X8R8G8B8, D3DFMT_D24S8);

	if(FAILED(hr)) return false;

	memset(&m_d3dcaps, 0, sizeof(m_d3dcaps));

	m_d3d->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &m_d3dcaps);

	bool fs = AfxGetApp()->GetProfileInt(_T("Settings"), _T("ModeWidth"), 0) > 0;

	if(!Reset(1, 1, fs)) return false;

	m_dev->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.0f, 0);

	// shaders

	DWORD psver = AfxGetApp()->GetProfileInt(_T("Settings"), _T("PixelShaderVersion2"), D3DPS_VERSION(2, 0));

	if(psver > m_d3dcaps.PixelShaderVersion)
	{
		CString str;

		str.Format(_T("Supported pixel shader version is too low!\n\nSupported: %d.%d\nSelected: %d.%d"),
			D3DSHADER_VERSION_MAJOR(m_d3dcaps.PixelShaderVersion), D3DSHADER_VERSION_MINOR(m_d3dcaps.PixelShaderVersion),
			D3DSHADER_VERSION_MAJOR(psver), D3DSHADER_VERSION_MINOR(psver));

		AfxMessageBox(str);

		return false;
	}

	m_d3dcaps.PixelShaderVersion = min(psver, m_d3dcaps.PixelShaderVersion);
	m_d3dcaps.VertexShaderVersion = m_d3dcaps.PixelShaderVersion & ~0x10000;

	for(int i = 0; i < countof(m_interlace.ps); i++)
	{
		CStringA main;
		main.Format("ps_main%d", i);
		CompileShader(IDR_INTERLACE_FX, main, NULL, &m_interlace.ps[i]);
	}

	for(int i = 0; i < countof(m_convert.ps); i++)
	{
		CStringA main;
		main.Format("ps_main%d", i);
		CompileShader(IDR_CONVERT_FX, main, NULL, &m_convert.ps[i]);
	}

	m_convert.dss.DepthEnable = false;
	m_convert.dss.StencilEnable = false;

	m_convert.bs.BlendEnable = false;
	m_convert.bs.RenderTargetWriteMask = D3DCOLORWRITEENABLE_RGBA;

	m_convert.ln.FilterMin[0] = D3DTEXF_LINEAR;
	m_convert.ln.FilterMag[0] = D3DTEXF_LINEAR;
	m_convert.ln.FilterMin[1] = D3DTEXF_LINEAR;
	m_convert.ln.FilterMag[1] = D3DTEXF_LINEAR;
	m_convert.ln.AddressU = D3DTADDRESS_CLAMP;
	m_convert.ln.AddressV = D3DTADDRESS_CLAMP;

	m_convert.pt.FilterMin[0] = D3DTEXF_POINT;
	m_convert.pt.FilterMag[0] = D3DTEXF_POINT;
	m_convert.pt.FilterMin[1] = D3DTEXF_POINT;
	m_convert.pt.FilterMag[1] = D3DTEXF_POINT;
	m_convert.pt.AddressU = D3DTADDRESS_CLAMP;
	m_convert.pt.AddressV = D3DTADDRESS_CLAMP;

	return true;
}

bool GSDeviceDX9::Reset(int w, int h, bool fs)
{
	if(!__super::Reset(w, h, fs))
		return false;

	HRESULT hr;

	if(!m_d3d) return false;

	if(m_swapchain && !fs && m_pp.Windowed)
	{
		m_swapchain = NULL;
		m_backbuffer = GSTextureDX9();

		m_pp.BackBufferWidth = w;
		m_pp.BackBufferHeight = h;

		hr = m_dev->CreateAdditionalSwapChain(&m_pp, &m_swapchain);

		if(FAILED(hr)) return false;

		CComPtr<IDirect3DSurface9> backbuffer;
		hr = m_swapchain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);
		m_backbuffer = GSTextureDX9(backbuffer);

		return true;
	}

	m_swapchain = NULL;
	m_backbuffer = GSTextureDX9();
	m_tex_current = GSTextureDX9();
	m_tex_merge = GSTextureDX9();
	m_tex_1x1 = GSTextureDX9();
	m_font = NULL;

	delete [] m_vs_cb;
	delete [] m_ps_cb;

	m_vb = NULL;
	m_vb_stride = 0;
	m_layout = NULL;
	m_vs = NULL;
	m_vs_cb = NULL;
	m_vs_cb_len = 0;
	m_ps = NULL;
	m_ps_cb = NULL;
	m_ps_cb_len = 0;
	m_ps_ss = NULL;
	m_scissor = CRect(0, 0, 0, 0);
	m_dss = NULL;
	m_sref = 0;
	m_bs = NULL;
	m_bf = -1;
	m_rtv = NULL;
	m_dsv = NULL;

	memset(&m_pp, 0, sizeof(m_pp));

	m_pp.Windowed = TRUE;
	m_pp.hDeviceWindow = m_hWnd;
	m_pp.SwapEffect = D3DSWAPEFFECT_FLIP;
	m_pp.BackBufferFormat = D3DFMT_X8R8G8B8;
	m_pp.BackBufferWidth = 1;
	m_pp.BackBufferHeight = 1;
	m_pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	if(!!AfxGetApp()->GetProfileInt(_T("Settings"), _T("vsync"), FALSE))
	{
		m_pp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	}

	if(!!AfxGetApp()->GetProfileInt(_T("Settings"), _T("tvout"), FALSE))
	{
		m_pp.Flags |= D3DPRESENTFLAG_VIDEO;
	}

	int mw = AfxGetApp()->GetProfileInt(_T("Settings"), _T("ModeWidth"), 0);
	int mh = AfxGetApp()->GetProfileInt(_T("Settings"), _T("ModeHeight"), 0);
	int mrr = AfxGetApp()->GetProfileInt(_T("Settings"), _T("ModeRefreshRate"), 0);

	if(fs && mw > 0 && mh > 0 && mrr >= 0)
	{
		m_pp.Windowed = FALSE;
		m_pp.BackBufferWidth = mw;
		m_pp.BackBufferHeight = mh;
		// m_pp.FullScreen_RefreshRateInHz = mrr;

		::SetWindowLong(m_hWnd, GWL_STYLE, ::GetWindowLong(m_hWnd, GWL_STYLE) & ~(WS_CAPTION|WS_THICKFRAME));
		::SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		::SetMenu(m_hWnd, NULL);
	}

	if(!m_dev)
	{
		UINT flags = D3DCREATE_MULTITHREADED | (m_d3dcaps.VertexProcessingCaps ? D3DCREATE_HARDWARE_VERTEXPROCESSING : D3DCREATE_SOFTWARE_VERTEXPROCESSING);

		hr = m_d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd, flags, &m_pp, &m_dev);

		if(FAILED(hr)) return false;
	}
	else
	{
		hr = m_dev->Reset(&m_pp);

		if(FAILED(hr))
		{
			if(D3DERR_DEVICELOST == hr)
			{
				Sleep(1000);

				hr = m_dev->Reset(&m_pp);
			}

			if(FAILED(hr)) return false;
		}
	}

	if(m_pp.Windowed)
	{
		m_pp.BackBufferWidth = 1;
		m_pp.BackBufferHeight = 1;

		hr = m_dev->CreateAdditionalSwapChain(&m_pp, &m_swapchain);

		if(FAILED(hr)) return false;
	}

	CComPtr<IDirect3DSurface9> backbuffer;

	if(m_swapchain)
	{
		hr = m_swapchain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);
	}
	else
	{
		hr = m_dev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);
	}

	m_backbuffer = GSTextureDX9(backbuffer);

	CreateTexture(m_tex_1x1, 1, 1);

	D3DXFONT_DESC fd;
	memset(&fd, 0, sizeof(fd));
	_tcscpy(fd.FaceName, _T("Arial"));
	fd.Height = 20;
	D3DXCreateFontIndirect(m_dev, &fd, &m_font);

	m_dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_dev->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_dev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	m_dev->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);

	return true;
}

bool GSDeviceDX9::IsLost()
{
	HRESULT hr = m_dev->TestCooperativeLevel();
	
	return hr == D3DERR_DEVICELOST || hr == D3DERR_DEVICENOTRESET;
}

void GSDeviceDX9::Present(int arx, int ary)
{
	CRect cr;

	GetClientRect(m_hWnd, &cr);

	if(m_backbuffer.GetWidth() != cr.Width() || m_backbuffer.GetHeight() != cr.Height())
	{
		Reset(cr.Width(), cr.Height(), false);
	}

	OMSetRenderTargets(m_backbuffer, NULL);

	m_dev->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.0f, 0);

	if(m_tex_current)
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

		GSVector4 dr(r.left, r.top, r.right, r.bottom);

		StretchRect(m_tex_current, m_backbuffer, dr);
	}

	if(m_swapchain)
	{
		m_swapchain->Present(NULL, NULL, NULL, NULL, 0);
	}
	else
	{
		m_dev->Present(NULL, NULL, NULL, NULL);
	}
}

void GSDeviceDX9::BeginScene()
{
	m_dev->BeginScene();
}

void GSDeviceDX9::EndScene()
{
	m_dev->EndScene();
}

bool GSDeviceDX9::Create(int type, GSTextureDX9& t, int w, int h, int format)
{
	HRESULT hr;

	CComPtr<IDirect3DTexture9> texture;
	CComPtr<IDirect3DSurface9> surface;

	if(type == GSTexture::RenderTarget)
	{
		hr = m_dev->CreateTexture(w, h, 1, D3DUSAGE_RENDERTARGET, (D3DFORMAT)format, D3DPOOL_DEFAULT, &texture, NULL);
	}

	if(type == GSTexture::DepthStencil)
	{
		hr = m_dev->CreateDepthStencilSurface(w, h, (D3DFORMAT)format, D3DMULTISAMPLE_NONE, 0, FALSE, &surface, NULL);
	}

	if(type == GSTexture::Texture)
	{
		hr = m_dev->CreateTexture(w, h, 1, 0, (D3DFORMAT)format, D3DPOOL_MANAGED, &texture, NULL);
	}

	if(type == GSTexture::Offscreen)
	{
		hr = m_dev->CreateOffscreenPlainSurface(w, h, (D3DFORMAT)format, D3DPOOL_SYSTEMMEM, &surface, NULL);
	}

	if(surface)
	{
		t = GSTextureDX9(surface);

		return true;
	}

	if(texture)
	{
		t = GSTextureDX9(texture);

		return true;
	}

	return false;
}

bool GSDeviceDX9::CreateRenderTarget(GSTextureDX9& t, int w, int h, int format)
{
	return __super::CreateRenderTarget(t, w, h, format ? format : D3DFMT_A8R8G8B8);
}

bool GSDeviceDX9::CreateDepthStencil(GSTextureDX9& t, int w, int h, int format)
{
	return __super::CreateDepthStencil(t, w, h, format ? format : D3DFMT_D24S8);
}

bool GSDeviceDX9::CreateTexture(GSTextureDX9& t, int w, int h, int format)
{
	return __super::CreateTexture(t, w, h, format ? format : D3DFMT_A8R8G8B8);
}

bool GSDeviceDX9::CreateOffscreen(GSTextureDX9& t, int w, int h, int format)
{
	return __super::CreateOffscreen(t, w, h, format ? format : D3DFMT_A8R8G8B8);
}

void GSDeviceDX9::Deinterlace(GSTextureDX9& st, GSTextureDX9& dt, int shader, bool linear, float yoffset)
{
	GSVector4 sr(0, 0, 1, 1);
	GSVector4 dr(0, yoffset, (float)dt.m_desc.Width, (float)dt.m_desc.Height + yoffset);

	const float cb[] = {0, 1.0f / dt.m_desc.Height, 0, (float)dt.m_desc.Height / 2};

	StretchRect(st, sr, dt, dr, m_interlace.ps[shader], cb, 1, linear);
}

void GSDeviceDX9::IASetVertexBuffer(IDirect3DVertexBuffer9* vb, UINT count, const void* vertices, UINT stride)
{
	void* data = NULL;

	if(SUCCEEDED(vb->Lock(0, count * stride, &data, D3DLOCK_DISCARD)))
	{
		memcpy(data, vertices, count * stride);

		vb->Unlock();
	}

	if(m_vb != vb || m_vb_stride != stride)
	{
		m_dev->SetStreamSource(0, vb, 0, stride);

		m_vb = vb;
		m_vb_stride = stride;
	}
}

void GSDeviceDX9::IASetInputLayout(IDirect3DVertexDeclaration9* layout)
{
	// if(m_layout != layout)
	{
		m_dev->SetVertexDeclaration(layout);

		// m_layout = layout;
	}
}

void GSDeviceDX9::VSSetShader(IDirect3DVertexShader9* vs, const float* vs_cb, int vs_cb_len)
{
	if(m_vs != vs)
	{
		m_dev->SetVertexShader(vs);

		m_vs = vs;
	}

	if(vs_cb && vs_cb_len > 0)
	{
		int size = vs_cb_len * sizeof(float) * 4;
		
		if(m_vs_cb_len != vs_cb_len || m_vs_cb == NULL || memcmp(m_vs_cb, vs_cb, size))
		{
			if(m_vs_cb == NULL || m_vs_cb_len < vs_cb_len)
			{
				delete [] m_vs_cb;

				m_vs_cb = (float*)new BYTE[size];
			}

			memcpy(m_vs_cb, vs_cb, size);

			m_dev->SetVertexShaderConstantF(0, vs_cb, vs_cb_len);

			m_vs_cb_len = vs_cb_len;
		}
	}
}

void GSDeviceDX9::PSSetShaderResources(IDirect3DTexture9* srv0, IDirect3DTexture9* srv1)
{
	if(m_ps_srvs[0] != srv0 || m_ps_srvs[1] != srv1)
	{
		m_dev->SetTexture(0, srv0);
		m_dev->SetTexture(1, srv1);

		m_ps_srvs[0] = srv0;
		m_ps_srvs[1] = srv1;
	}
}

void GSDeviceDX9::PSSetShader(IDirect3DPixelShader9* ps, const float* ps_cb, int ps_cb_len)
{
	if(m_ps != ps)
	{
		m_dev->SetPixelShader(ps);

		m_ps = ps;
	}
	
	if(ps_cb && ps_cb_len > 0)
	{
		int size = ps_cb_len * sizeof(float) * 4;
		
		if(m_ps_cb_len != ps_cb_len || m_ps_cb == NULL || memcmp(m_ps_cb, ps_cb, size))
		{
			if(m_ps_cb == NULL || m_ps_cb_len < ps_cb_len)
			{
				delete [] m_ps_cb;

				m_ps_cb = (float*)new BYTE[size];
			}

			memcpy(m_ps_cb, ps_cb, size);

			m_dev->SetPixelShaderConstantF(0, ps_cb, ps_cb_len);

			m_ps_cb_len = ps_cb_len;
		}
	}
}

void GSDeviceDX9::PSSetSamplerState(Direct3DSamplerState9* ss)
{
	if(ss && m_ps_ss != ss)
	{
		m_dev->SetSamplerState(0, D3DSAMP_ADDRESSU, ss->AddressU);
		m_dev->SetSamplerState(0, D3DSAMP_ADDRESSV, ss->AddressV);
		m_dev->SetSamplerState(1, D3DSAMP_ADDRESSU, ss->AddressU);
		m_dev->SetSamplerState(1, D3DSAMP_ADDRESSV, ss->AddressV);
		m_dev->SetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		m_dev->SetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		m_dev->SetSamplerState(3, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		m_dev->SetSamplerState(3, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		m_dev->SetSamplerState(0, D3DSAMP_MINFILTER, ss->FilterMin[0]);
		m_dev->SetSamplerState(0, D3DSAMP_MAGFILTER, ss->FilterMag[0]);
		m_dev->SetSamplerState(1, D3DSAMP_MINFILTER, ss->FilterMin[1]);
		m_dev->SetSamplerState(1, D3DSAMP_MAGFILTER, ss->FilterMag[1]);
		m_dev->SetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		m_dev->SetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		m_dev->SetSamplerState(3, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		m_dev->SetSamplerState(3, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		
		m_ps_ss = ss;
	}
}

void GSDeviceDX9::RSSet(int width, int height, const RECT* scissor)
{
	CRect r = scissor ? *scissor : CRect(0, 0, width, height);

	if(m_scissor != r)
	{
		m_dev->SetScissorRect(&r);

		m_scissor = r;
	}
}

void GSDeviceDX9::OMSetDepthStencilState(Direct3DDepthStencilState9* dss, UINT sref)
{
	if(m_dss != dss || m_sref != sref)
	{
		m_dev->SetRenderState(D3DRS_ZENABLE, dss->DepthEnable);
		m_dev->SetRenderState(D3DRS_ZWRITEENABLE, dss->DepthWriteMask);
		
		if(dss->DepthEnable)
		{
			m_dev->SetRenderState(D3DRS_ZFUNC, dss->DepthFunc);
		}

		m_dev->SetRenderState(D3DRS_STENCILENABLE, dss->StencilEnable);

		if(dss->StencilEnable)
		{
			m_dev->SetRenderState(D3DRS_STENCILMASK, dss->StencilReadMask);
			m_dev->SetRenderState(D3DRS_STENCILWRITEMASK, dss->StencilWriteMask);	
			m_dev->SetRenderState(D3DRS_STENCILFUNC, dss->StencilFunc);
			m_dev->SetRenderState(D3DRS_STENCILPASS, dss->StencilPassOp);
			m_dev->SetRenderState(D3DRS_STENCILFAIL, dss->StencilFailOp);
			m_dev->SetRenderState(D3DRS_STENCILZFAIL, dss->StencilDepthFailOp);
			m_dev->SetRenderState(D3DRS_STENCILREF, sref);
		}

		m_dss = dss;
		m_sref = sref;
	}
}

void GSDeviceDX9::OMSetBlendState(Direct3DBlendState9* bs, DWORD bf)
{
	if(m_bs != bs || m_bf != bf)
	{
		m_dev->SetRenderState(D3DRS_ALPHABLENDENABLE, bs->BlendEnable);

		if(bs->BlendEnable)
		{
			m_dev->SetRenderState(D3DRS_BLENDOP, bs->BlendOp);
			m_dev->SetRenderState(D3DRS_SRCBLEND, bs->SrcBlend);
			m_dev->SetRenderState(D3DRS_DESTBLEND, bs->DestBlend);
			m_dev->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
			m_dev->SetRenderState(D3DRS_BLENDOPALPHA, bs->BlendOpAlpha);
			m_dev->SetRenderState(D3DRS_SRCBLENDALPHA, bs->SrcBlendAlpha);
			m_dev->SetRenderState(D3DRS_DESTBLENDALPHA, bs->DestBlendAlpha);
			m_dev->SetRenderState(D3DRS_BLENDFACTOR, bf);
		}

		m_dev->SetRenderState(D3DRS_COLORWRITEENABLE, bs->RenderTargetWriteMask);

		m_bs = bs;
		m_bf = bf;
	}
}

void GSDeviceDX9::OMSetRenderTargets(IDirect3DSurface9* rtv, IDirect3DSurface9* dsv)
{
	if(m_rtv != rtv || m_dsv != dsv)
	{
		m_dev->SetRenderTarget(0, rtv);
		m_dev->SetDepthStencilSurface(dsv);

		m_rtv = rtv;
		m_dsv = dsv;
	}
}

void GSDeviceDX9::Draw(LPCTSTR str)
{
	if(!m_pp.Windowed)
	{
		BeginScene();

		OMSetRenderTargets(m_backbuffer, NULL);

		CRect r(0, 0, m_backbuffer.GetWidth(), m_backbuffer.GetHeight());

		D3DCOLOR c = D3DCOLOR_ARGB(255, 0, 255, 0);

		if(m_font->DrawText(NULL, str, -1, &r, DT_CALCRECT|DT_LEFT|DT_WORDBREAK, c))
		{
			m_font->DrawText(NULL, str, -1, &r, DT_LEFT|DT_WORDBREAK, c);
		}

		EndScene();
	}
}

bool GSDeviceDX9::SaveCurrent(LPCTSTR fn)
{
	return SUCCEEDED(D3DXSaveTextureToFile(fn, D3DXIFF_BMP, m_tex_current, NULL));
}

bool GSDeviceDX9::SaveToFileD24S8(IDirect3DSurface9* ds, LPCTSTR fn)
{
	HRESULT hr;

	D3DSURFACE_DESC desc;

	ds->GetDesc(&desc);

	if(desc.Format != D3DFMT_D32F_LOCKABLE)
		return false;

	CComPtr<IDirect3DSurface9> surface;
	
	hr = m_dev->CreateOffscreenPlainSurface(desc.Width, desc.Height, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &surface, NULL);

	D3DLOCKED_RECT slr, dlr;

	hr = ds->LockRect(&slr, NULL, 0);;
	hr = surface->LockRect(&dlr, NULL, 0);

	BYTE* s = (BYTE*)slr.pBits;
	BYTE* d = (BYTE*)dlr.pBits;

	for(int y = 0; y < desc.Height; y++, s += slr.Pitch, d += dlr.Pitch)
	{
		for(int x = 0; x < desc.Width; x++)
		{
			((float*)d)[x] = ((float*)s)[x];
		}
	}

	ds->UnlockRect();
	surface->UnlockRect();

	return SUCCEEDED(D3DXSaveSurfaceToFile(fn, D3DXIFF_BMP, surface, NULL, NULL));
}

void GSDeviceDX9::StretchRect(GSTextureDX9& st, GSTextureDX9& dt, const GSVector4& dr, bool linear)
{
	StretchRect(st, GSVector4(0, 0, 1, 1), dt, dr, m_convert.ps[0], NULL, 0, linear);
}

void GSDeviceDX9::StretchRect(GSTextureDX9& st, const GSVector4& sr, GSTextureDX9& dt, const GSVector4& dr, bool linear)
{
	StretchRect(st, sr, dt, dr, m_convert.ps[0], NULL, 0, linear);
}

void GSDeviceDX9::StretchRect(GSTextureDX9& st, const GSVector4& sr, GSTextureDX9& dt, const GSVector4& dr, IDirect3DPixelShader9* ps, const float* ps_cb, int ps_cb_len, bool linear)
{
	BeginScene();

	// om

	OMSetDepthStencilState(&m_convert.dss, 0);
	OMSetBlendState(&m_convert.bs, 0);
	OMSetRenderTargets(dt, NULL);

	// ia
/*
	float left = dr.x * 2 / dt.m_desc.Width - 1.0f;
	float top = 1.0f - dr.y * 2 / dt.m_desc.Height;
	float right = dr.z * 2 / dt.m_desc.Width - 1.0f;
	float bottom = 1.0f - dr.w * 2 / dt.m_desc.Height;

	GSVertexPT1 vertices[] =
	{
		{GSVector4(left, top), GSVector2(sr.x, sr.y)},
		{GSVector4(right, top), GSVector2(sr.z, sr.y)},
		{GSVector4(left, bottom), GSVector2(sr.x, sr.w)},
		{GSVector4(right, bottom), GSVector2(sr.z, sr.w)},
	};

	IASetInputLayout(m_convert.il);

	// vs

	VSSetShader(m_convert.vs, NULL);
*/
	VSSetShader(NULL, NULL, 0); // TODO

	// ps

	PSSetShader(ps, ps_cb, ps_cb_len);
	PSSetSamplerState(linear ? &m_convert.ln : &m_convert.pt);
	PSSetShaderResources(st, NULL);

	// rs

	RSSet(dt.m_desc.Width, dt.m_desc.Height);

	//

	GSVertexPT1 vertices[] =
	{
		{GSVector4(dr.x, dr.y), GSVector2(sr.x, sr.y)},
		{GSVector4(dr.z, dr.y), GSVector2(sr.z, sr.y)},
		{GSVector4(dr.z, dr.w), GSVector2(sr.z, sr.w)},
		{GSVector4(dr.x, dr.w), GSVector2(sr.x, sr.w)},
	};

	for(int i = 0; i < countof(vertices); i++)
	{
		vertices[i].p.x -= 0.5f;
		vertices[i].p.y -= 0.5f;
	}

	m_dev->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);

	m_dev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(vertices[0]));

	//

	EndScene();
}

HRESULT GSDeviceDX9::CompileShader(UINT id, LPCSTR entry, const D3DXMACRO* macro, IDirect3DVertexShader9** vs, ID3DXConstantTable** ct)
{
	LPCSTR target;

	if(m_d3dcaps.VertexShaderVersion >= D3DVS_VERSION(3, 0))
	{
		target = "vs_3_0";
	}
	else if(m_d3dcaps.VertexShaderVersion >= D3DVS_VERSION(2, 0))
	{
		target = "vs_2_0";
	}
	else
	{
		return E_FAIL;
	}

	CComPtr<ID3DXBuffer> shader, error;

	HRESULT hr = D3DXCompileShaderFromResource(AfxGetResourceHandle(), MAKEINTRESOURCE(id), macro, NULL, entry, target, 0, &shader, &error, ct);

	if(SUCCEEDED(hr))
	{
		hr = m_dev->CreateVertexShader((DWORD*)shader->GetBufferPointer(), vs);
	}
	else if(error)
	{
		LPCSTR msg = (LPCSTR)error->GetBufferPointer();

		TRACE(_T("%s\n"), CString(msg));
	}

	ASSERT(SUCCEEDED(hr));

	return hr;
}

HRESULT GSDeviceDX9::CompileShader(UINT id, LPCSTR entry, const D3DXMACRO* macro, IDirect3DPixelShader9** ps, ID3DXConstantTable** ct)
{
	LPCSTR target = NULL;
	UINT flags = 0;

	if(m_d3dcaps.PixelShaderVersion >= D3DPS_VERSION(3, 0))
	{
		target = "ps_3_0";
		flags |= D3DXSHADER_AVOID_FLOW_CONTROL;
	}
	else if(m_d3dcaps.PixelShaderVersion >= D3DPS_VERSION(2, 0))
	{
		target = "ps_2_0";
	}
	else 
	{
		return false;
	}

	CComPtr<ID3DXBuffer> shader, error;

	HRESULT hr = D3DXCompileShaderFromResource(AfxGetResourceHandle(), MAKEINTRESOURCE(id), macro, NULL, entry, target, flags, &shader, &error, ct);

	if(SUCCEEDED(hr))
	{
		hr = m_dev->CreatePixelShader((DWORD*)shader->GetBufferPointer(), ps);

		ASSERT(SUCCEEDED(hr));
	}
	else if(error)
	{
		LPCSTR msg = (LPCSTR)error->GetBufferPointer();

		TRACE(_T("%s\n"), CString(msg));
	}

	ASSERT(SUCCEEDED(hr));

	return hr;
}
