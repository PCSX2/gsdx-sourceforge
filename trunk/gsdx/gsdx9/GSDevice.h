#pragma once

#include "GSTexture2D.h"

struct VertexPT1
{
	float x, y, z, w;
	float tu, tv;
};

struct VertexPT2
{
	float x, y, z, w;
	float tu1, tv1;
	float tu2, tv2;
};

class GSDevice
{
	CAtlList<GSTexture2D> m_pool;

	void Interlace(GSTexture2D& st, GSTexture2D& dt, int shader, bool linear, float yoffset = 0);

public: // TODO
	HWND m_hWnd;
	D3DPRESENT_PARAMETERS m_pp;
	DDCAPS m_ddcaps;
	D3DCAPS9 m_d3dcaps;
	CComPtr<IDirect3D9> m_d3d;
	CComPtr<IDirect3DDevice9> m_dev;
	CComPtr<IDirect3DSwapChain9> m_swapchain;
	CComPtr<IDirect3DSurface9> m_backbuffer;
	CComPtr<IDirect3DTexture9> m_tex_current;

	GSTexture2D m_tex_merge;
	GSTexture2D m_tex_interlace;
	GSTexture2D m_tex_deinterlace;
	GSTexture2D m_tex_1x1;

	CComPtr<IDirect3DPixelShader9> m_ps_convert[4];
	CComPtr<IDirect3DPixelShader9> m_ps_interlace[4];

public:
	GSDevice();
	virtual ~GSDevice();

	bool Create(HWND hWnd);

	IDirect3DDevice9* operator->() {return m_dev;}
	operator IDirect3DDevice9*() {return m_dev;}

	bool ResetDevice(int w, int h, bool fs = false);
	void Present();

	HRESULT CreateRenderTarget(GSTexture2D& t, int w, int h, D3DFORMAT format = D3DFMT_A8R8G8B8);
	HRESULT CreateDepthStencil(GSTexture2D& t, int w, int h, D3DFORMAT format = D3DFMT_D24S8);
	HRESULT CreateTexture(GSTexture2D& t, int w, int h, D3DFORMAT format = D3DFMT_A8R8G8B8);
	HRESULT CreateOffscreenPlainSurface(GSTexture2D& t, int w, int h, D3DFORMAT format = D3DFMT_A8R8G8B8);

	void Recycle(GSTexture2D& t);

	bool SaveCurrent(LPCTSTR fn);

	void StretchRect(GSTexture2D& st, GSTexture2D& dt, const D3DXVECTOR4& dr, bool linear = true);
	void StretchRect(GSTexture2D& st, const D3DXVECTOR4& sr, GSTexture2D& dt, const D3DXVECTOR4& dr, bool linear = true);
	void StretchRect(GSTexture2D& st, const D3DXVECTOR4& sr, GSTexture2D& dt, const D3DXVECTOR4& dr, IDirect3DPixelShader9* ps, bool linear = true);

	IDirect3DTexture9* Interlace(GSTexture2D& st, CSize ds, int field, int mode, float yoffset);

	HRESULT CompileShader(UINT id, LPCSTR entry, const D3DXMACRO* macro, IDirect3DVertexShader9** vs, ID3DXConstantTable** ct = NULL);
	HRESULT CompileShader(UINT id, LPCSTR entry, const D3DXMACRO* macro, IDirect3DPixelShader9** ps, ID3DXConstantTable** ct = NULL);
};
