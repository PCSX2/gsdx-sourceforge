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

// doesn't seems to make any difference in speed, but in the shader the implementation is closer to gsdx10

#define PS_ALPHA_TEST

// this can be painfully slow with ps 2.0/3.0, only helps in a few cases (soul calibur 2)

// #define PS_REGION_REPEAT

#include "GSTexture2D.h"

struct VertexP
{
	float x, y, z, w;
};

struct VertexPC
{
	float x, y, z, w;
	DWORD c;
};

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
	CComPtr<ID3DXFont> m_font;

	GSTexture2D m_tex_merge;
	GSTexture2D m_tex_interlace;
	GSTexture2D m_tex_deinterlace;
	GSTexture2D m_tex_1x1;

	CComPtr<IDirect3DPixelShader9> m_ps_convert[3];
	CComPtr<IDirect3DPixelShader9> m_ps_interlace[4];

public:
	GSDevice();
	virtual ~GSDevice();

	bool Create(HWND hWnd);

	IDirect3DDevice9* operator->() {return m_dev;}
	operator IDirect3DDevice9*() {return m_dev;}

	bool ResetDevice(int w, int h, bool fs = false);
	void Draw(LPCTSTR str);
	void Present();

	HRESULT CreateRenderTarget(GSTexture2D& t, int w, int h, D3DFORMAT format = D3DFMT_A8R8G8B8);
	HRESULT CreateDepthStencil(GSTexture2D& t, int w, int h, D3DFORMAT format = D3DFMT_D24S8/*D3DFMT_D32F_LOCKABLE*/);
	HRESULT CreateTexture(GSTexture2D& t, int w, int h, D3DFORMAT format = D3DFMT_A8R8G8B8);
	HRESULT CreateOffscreenPlainSurface(GSTexture2D& t, int w, int h, D3DFORMAT format = D3DFMT_A8R8G8B8);

	void Recycle(GSTexture2D& t);

	bool SaveCurrent(LPCTSTR fn);
	bool SaveToFileD24S8(IDirect3DSurface9* ds, LPCTSTR fn);

	void StretchRect(GSTexture2D& st, GSTexture2D& dt, const GSVector4& dr, bool linear = true);
	void StretchRect(GSTexture2D& st, const GSVector4& sr, GSTexture2D& dt, const GSVector4& dr, bool linear = true);
	void StretchRect(GSTexture2D& st, const GSVector4& sr, GSTexture2D& dt, const GSVector4& dr, IDirect3DPixelShader9* ps, bool linear = true);

	IDirect3DTexture9* Interlace(GSTexture2D& st, CSize ds, int field, int mode, float yoffset);

	HRESULT CompileShader(UINT id, LPCSTR entry, const D3DXMACRO* macro, IDirect3DVertexShader9** vs, ID3DXConstantTable** ct = NULL);
	HRESULT CompileShader(UINT id, LPCSTR entry, const D3DXMACRO* macro, IDirect3DPixelShader9** ps, ID3DXConstantTable** ct = NULL);
};
