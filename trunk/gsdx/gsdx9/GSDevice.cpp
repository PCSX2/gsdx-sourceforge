#include "stdafx.h"
#include "GSDevice.h"
#include "resource.h"

GSDevice::GSDevice()
{
	memset(&m_pp, 0, sizeof(m_pp));
	memset(&m_ddcaps, 0, sizeof(m_ddcaps));
	memset(&m_d3dcaps, 0, sizeof(m_d3dcaps));
}

GSDevice::~GSDevice()
{
}

bool GSDevice::Create(HWND hWnd)
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

	if(!ResetDevice(1, 1, fs)) return false;

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

	for(int i = 0; i < countof(m_ps_interlace); i++)
	{
		CStringA main;
		main.Format("ps_main%d", i);
		CompileShader(IDR_INTERLACE_FX, main, NULL, &m_ps_interlace[i]);
	}

	for(int i = 0; i < countof(m_ps_convert); i++)
	{
		CStringA main;
		main.Format("ps_main%d", i);
		CompileShader(IDR_CONVERT_FX, main, NULL, &m_ps_convert[i]);
	}

	return true;
}

bool GSDevice::ResetDevice(int w, int h, bool fs)
{
	HRESULT hr;

	if(!m_d3d) return false;

	if(m_swapchain && !fs && m_pp.Windowed)
	{
		m_swapchain = NULL;
		m_backbuffer = NULL;

		m_pp.BackBufferWidth = w;
		m_pp.BackBufferHeight = h;

		hr = m_dev->CreateAdditionalSwapChain(&m_pp, &m_swapchain);

		if(FAILED(hr)) return false;

		hr = m_swapchain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &m_backbuffer);

		return true;
	}

	m_swapchain = NULL;
	m_backbuffer = NULL;
	m_tex_current = NULL;
	m_tex_merge = GSTexture2D();
	m_tex_interlace = GSTexture2D();
	m_tex_deinterlace = GSTexture2D();
	m_tex_1x1 = GSTexture2D();
	m_pool.RemoveAll();
	m_font = NULL;

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

	if(m_swapchain)
	{
		hr = m_swapchain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &m_backbuffer);
	}
	else
	{
		hr = m_dev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &m_backbuffer);
	}

	hr = m_dev->SetRenderTarget(0, m_backbuffer);

	hr = m_dev->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.0f, 0);

    hr = m_dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    hr = m_dev->SetRenderState(D3DRS_LIGHTING, FALSE);

	for(int i = 0; i < 8; i++)
	{
		hr = m_dev->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		hr = m_dev->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		hr = m_dev->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
		hr = m_dev->SetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		hr = m_dev->SetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	}

	hr = m_dev->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
	hr = m_dev->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
	hr = m_dev->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
	hr = m_dev->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ZERO);

	CreateTexture(m_tex_1x1, 1, 1);

	D3DXFONT_DESC fd;
	memset(&fd, 0, sizeof(fd));
	_tcscpy(fd.FaceName, _T("Arial"));
	fd.Height = 20;
	D3DXCreateFontIndirect(m_dev, &fd, &m_font);

	return true;
}

void GSDevice::Draw(LPCTSTR str)
{
	if(!m_pp.Windowed)
	{
		m_dev->BeginScene();

		m_dev->SetRenderTarget(0, m_backbuffer);
		m_dev->SetDepthStencilSurface(NULL);

		D3DSURFACE_DESC desc;
		m_backbuffer->GetDesc(&desc);
		CRect r(0, 0, desc.Width, desc.Height);

		D3DCOLOR c = D3DCOLOR_ARGB(255, 0, 255, 0);

		if(m_font->DrawText(NULL, str, -1, &r, DT_CALCRECT|DT_LEFT|DT_WORDBREAK, c))
		{
			m_font->DrawText(NULL, str, -1, &r, DT_LEFT|DT_WORDBREAK, c);
		}

		m_dev->EndScene();
	}
}

void GSDevice::Present()
{
	if(m_swapchain)
	{
		m_swapchain->Present(NULL, NULL, NULL, NULL, 0);
	}
	else
	{
		m_dev->Present(NULL, NULL, NULL, NULL);
	}
}

bool GSDevice::CreateRenderTarget(GSTexture2D& t, int w, int h, DWORD format)
{
	HRESULT hr;

	Recycle(t);

	for(POSITION pos = m_pool.GetHeadPosition(); pos; m_pool.GetNext(pos))
	{
		const GSTexture2D& t2 = m_pool.GetAt(pos);

		if(t2.IsRenderTarget() && t2.m_desc.Width == w && t2.m_desc.Height == h && t2.m_desc.Format == format)
		{
			t = t2;

			m_pool.RemoveAt(pos);

			return true;
		}
	}

	CComPtr<IDirect3DTexture9> texture;
	
	hr = m_dev->CreateTexture(w, h, 1, D3DUSAGE_RENDERTARGET, (D3DFORMAT)format, D3DPOOL_DEFAULT, &texture, NULL);

	if(SUCCEEDED(hr))
	{
		t = GSTexture2D(texture);

		return true;
	}

	return false;
}

bool GSDevice::CreateDepthStencil(GSTexture2D& t, int w, int h, DWORD format)
{
	HRESULT hr;

	Recycle(t);

	for(POSITION pos = m_pool.GetHeadPosition(); pos; m_pool.GetNext(pos))
	{
		const GSTexture2D& t2 = m_pool.GetAt(pos);

		if(t2.IsDepthStencil() && t2.m_desc.Width == w && t2.m_desc.Height == h && t2.m_desc.Format == format)
		{
			t = t2;

			m_pool.RemoveAt(pos);

			return true;
		}
	}

	CComPtr<IDirect3DSurface9> surface;
	
	hr = m_dev->CreateDepthStencilSurface(w, h, (D3DFORMAT)format, D3DMULTISAMPLE_NONE, 0, FALSE, &surface, NULL);

	if(SUCCEEDED(hr))
	{
		t = GSTexture2D(surface);

		return true;
	}

	return false;
}

bool GSDevice::CreateTexture(GSTexture2D& t, int w, int h, DWORD format)
{
	HRESULT hr;

	Recycle(t);

	for(POSITION pos = m_pool.GetHeadPosition(); pos; m_pool.GetNext(pos))
	{
		const GSTexture2D& t2 = m_pool.GetAt(pos);

		if(t2.IsManagedTexture() && t2.m_desc.Width == w && t2.m_desc.Height == h && t2.m_desc.Format == format)
		{
			t = t2;

			m_pool.RemoveAt(pos);

			return true;
		}
	}

	CComPtr<IDirect3DTexture9> texture;
	
	hr = m_dev->CreateTexture(w, h, 1, 0, (D3DFORMAT)format, D3DPOOL_MANAGED, &texture, NULL);

	if(SUCCEEDED(hr))
	{
		t = GSTexture2D(texture);

		return true;
	}

	return false;
}

bool GSDevice::CreateOffscreen(GSTexture2D& t, int w, int h, DWORD format)
{
	HRESULT hr;

	Recycle(t);

	for(POSITION pos = m_pool.GetHeadPosition(); pos; m_pool.GetNext(pos))
	{
		const GSTexture2D& t2 = m_pool.GetAt(pos);

		if(t2.IsOffscreenPlainTexture() && t2.m_desc.Width == w && t2.m_desc.Height == h && t2.m_desc.Format == format)
		{
			t = t2;

			m_pool.RemoveAt(pos);

			return true;
		}
	}

	CComPtr<IDirect3DSurface9> surface;
	
	hr = m_dev->CreateOffscreenPlainSurface(w, h, (D3DFORMAT)format, D3DPOOL_SYSTEMMEM, &surface, NULL);

	if(SUCCEEDED(hr))
	{
		t = GSTexture2D(surface);

		return true;
	}

	return false;
}

bool GSDevice::SaveCurrent(LPCTSTR fn)
{
	return SUCCEEDED(D3DXSaveTextureToFile(fn, D3DXIFF_BMP, m_tex_current, NULL));
}

bool GSDevice::SaveToFileD24S8(IDirect3DSurface9* ds, LPCTSTR fn)
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

void GSDevice::StretchRect(GSTexture2D& st, GSTexture2D& dt, const GSVector4& dr, bool linear)
{
	StretchRect(st, GSVector4(0, 0, 1, 1), dt, dr, m_ps_convert[0], linear);
}

void GSDevice::StretchRect(GSTexture2D& st, const GSVector4& sr, GSTexture2D& dt, const GSVector4& dr, bool linear)
{
	StretchRect(st, sr, dt, dr, m_ps_convert[0], linear);
}

void GSDevice::StretchRect(GSTexture2D& st, const GSVector4& sr, GSTexture2D& dt, const GSVector4& dr, IDirect3DPixelShader9* ps, bool linear)
{
	HRESULT hr;

	hr = m_dev->BeginScene();

	hr = m_dev->SetRenderTarget(0, dt);
	hr = m_dev->SetDepthStencilSurface(NULL);

	hr = m_dev->SetTexture(0, st);
	hr = m_dev->SetTexture(1, NULL);
	
    hr = m_dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    hr = m_dev->SetRenderState(D3DRS_LIGHTING, FALSE);
	hr = m_dev->SetRenderState(D3DRS_ZENABLE, FALSE);
	hr = m_dev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	hr = m_dev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE); 
	hr = m_dev->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
	hr = m_dev->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RGBA);

	hr = m_dev->SetSamplerState(0, D3DSAMP_MAGFILTER, linear ? D3DTEXF_LINEAR : D3DTEXF_POINT);
	hr = m_dev->SetSamplerState(0, D3DSAMP_MINFILTER, linear ? D3DTEXF_LINEAR : D3DTEXF_POINT);

	hr = m_dev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	hr = m_dev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	hr = m_dev->SetVertexShader(NULL);
	hr = m_dev->SetPixelShader(ps);

	VertexPT1 vertices[] =
	{
		{dr.x, dr.y, 0.5f, 1.0f, sr.x, sr.y},
		{dr.z, dr.y, 0.5f, 1.0f, sr.z, sr.y},
		{dr.z, dr.w, 0.5f, 1.0f, sr.z, sr.w},
		{dr.x, dr.w, 0.5f, 1.0f, sr.x, sr.w},
	};

	for(int i = 0; i < countof(vertices); i++)
	{
		vertices[i].x -= 0.5f;
		vertices[i].y -= 0.5f;
	}

	hr = m_dev->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);

	hr = m_dev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(vertices[0]));

	hr = m_dev->EndScene();
}

void GSDevice::Interlace(GSTexture2D& st, GSTexture2D& dt, int shader, bool linear, float yoffset)
{
	const float c[] = {0, 1.0f / dt.m_desc.Height, 0, (float)dt.m_desc.Height / 2};

	m_dev->SetPixelShaderConstantF(0, c, countof(c) / 4);

	GSVector4 sr(0, 0, 1, 1);
	GSVector4 dr(0, yoffset, (float)dt.m_desc.Width, (float)dt.m_desc.Height + yoffset);

	StretchRect(st, sr, dt, dr, m_ps_interlace[shader], linear);
}

IDirect3DTexture9* GSDevice::Interlace(GSTexture2D& st, CSize ds, int field, int mode, float yoffset)
{
	IDirect3DTexture9* t = st;

	if(!m_tex_interlace || m_tex_interlace.m_desc.Width != ds.cx || m_tex_interlace.m_desc.Height != ds.cy)
	{
		CreateRenderTarget(m_tex_interlace, ds.cx, ds.cy);
	}

	if(mode == 0 || mode == 2) // weave or blend
	{
		// weave first

		Interlace(m_tex_merge, m_tex_interlace, field, false);

		t = m_tex_interlace;

		if(mode == 2)
		{
			// blend

			if(!m_tex_deinterlace || m_tex_deinterlace.m_desc.Width != ds.cx || m_tex_deinterlace.m_desc.Height != ds.cy)
			{
				CreateRenderTarget(m_tex_deinterlace, ds.cx, ds.cy);
			}

			if(field == 0) return NULL;

			Interlace(m_tex_interlace, m_tex_deinterlace, 2, false);

			t = m_tex_deinterlace;
		}
	}
	else if(mode == 1) // bob
	{
		Interlace(m_tex_merge, m_tex_interlace, 3, true, yoffset * field);

		t = m_tex_interlace;
	}

	return t;
}

HRESULT GSDevice::CompileShader(UINT id, LPCSTR entry, const D3DXMACRO* macro, IDirect3DVertexShader9** vs, ID3DXConstantTable** ct)
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

HRESULT GSDevice::CompileShader(UINT id, LPCSTR entry, const D3DXMACRO* macro, IDirect3DPixelShader9** ps, ID3DXConstantTable** ct)
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
