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

// this can be painfully slow with ps 2.0/3.0, only helps in a few cases (soul calibur 2)

// #define PS_REGION_REPEAT

#include "GSTextureDX9.h"

struct Direct3DSamplerState9
{
    D3DTEXTUREFILTERTYPE FilterMin[2];
    D3DTEXTUREFILTERTYPE FilterMag[2];
    D3DTEXTUREADDRESS AddressU;
    D3DTEXTUREADDRESS AddressV;
};

struct Direct3DDepthStencilState9
{
    BOOL DepthEnable;
    BOOL DepthWriteMask;
    D3DCMPFUNC DepthFunc;
    BOOL StencilEnable;
    UINT8 StencilReadMask;
    UINT8 StencilWriteMask;
    D3DSTENCILOP StencilFailOp;
    D3DSTENCILOP StencilDepthFailOp;
    D3DSTENCILOP StencilPassOp;
    D3DCMPFUNC StencilFunc;
};

struct Direct3DBlendState9
{
    BOOL BlendEnable;
    D3DBLEND SrcBlend;
    D3DBLEND DestBlend;
    D3DBLENDOP BlendOp;
    D3DBLEND SrcBlendAlpha;
    D3DBLEND DestBlendAlpha;
    D3DBLENDOP BlendOpAlpha;
    UINT8 RenderTargetWriteMask;
};

class GSDeviceDX9 : public GSDevice<GSTextureDX9>
{
	// state cache

	IDirect3DVertexBuffer9* m_vb;
	UINT m_vb_stride;
	IDirect3DVertexDeclaration9* m_layout;
	IDirect3DVertexShader9* m_vs;
	float* m_vs_cb;
	int m_vs_cb_len;
	IDirect3DTexture9* m_ps_srvs[2];
	IDirect3DPixelShader9* m_ps;
	float* m_ps_cb;
	int m_ps_cb_len;
	Direct3DSamplerState9* m_ps_ss;
	CRect m_scissor;
	Direct3DDepthStencilState9* m_dss;
	UINT m_sref;
	Direct3DBlendState9* m_bs;
	DWORD m_bf;
	IDirect3DSurface9* m_rtv;
	IDirect3DSurface9* m_dsv;

	//

	bool Create(int type, GSTextureDX9& t, int w, int h, int format);
	void Deinterlace(GSTextureDX9& st, GSTextureDX9& dt, int shader, bool linear, float yoffset = 0);

private:
	HWND m_hWnd;
	DDCAPS m_ddcaps;
	D3DCAPS9 m_d3dcaps;
	CComPtr<IDirect3D9> m_d3d;
	CComPtr<IDirect3DDevice9> m_dev;
	CComPtr<IDirect3DSwapChain9> m_swapchain;
	GSTextureDX9 m_backbuffer;

public: // TODO
	D3DPRESENT_PARAMETERS m_pp;
	CComPtr<ID3DXFont> m_font;

	GSTextureDX9 m_tex_current;
	GSTextureDX9 m_tex_merge;
	GSTextureDX9 m_tex_1x1;

	struct
	{
		CComPtr<IDirect3DPixelShader9> ps[5];
		Direct3DSamplerState9 ln;
		Direct3DSamplerState9 pt;
		Direct3DDepthStencilState9 dss;
		Direct3DBlendState9 bs;
	} m_convert;

	struct
	{
		CComPtr<IDirect3DPixelShader9> ps[4];
	} m_interlace;

public:
	GSDeviceDX9();
	virtual ~GSDeviceDX9();

	bool Create(HWND hWnd);
	bool Reset(int w, int h, bool fs);
	bool IsLost();
	void Present(int arx, int ary);
	void BeginScene();
	void EndScene();

	bool CreateRenderTarget(GSTextureDX9& t, int w, int h, int format = 0);
	bool CreateDepthStencil(GSTextureDX9& t, int w, int h, int format = 0);
	bool CreateTexture(GSTextureDX9& t, int w, int h, int format = 0);
	bool CreateOffscreen(GSTextureDX9& t, int w, int h, int format = 0);

	IDirect3DDevice9* operator->() {return m_dev;}
	operator IDirect3DDevice9*() {return m_dev;}

	void IASetVertexBuffer(IDirect3DVertexBuffer9* vb, UINT count, const void* vertices, UINT stride);
	void IASetInputLayout(IDirect3DVertexDeclaration9* layout);
	// void IASetPrimitiveTopology(D3DPRIMITIVETYPE topology);
	void VSSetShader(IDirect3DVertexShader9* vs, const float* vs_cb, int vs_cb_len);
	void PSSetShaderResources(IDirect3DTexture9* srv0, IDirect3DTexture9* srv1);
	void PSSetShader(IDirect3DPixelShader9* ps, const float* ps_cb, int ps_cb_len);
	void PSSetSamplerState(Direct3DSamplerState9* ss);
	void RSSet(int width, int height, const RECT* scissor = NULL);
	void OMSetDepthStencilState(Direct3DDepthStencilState9* dss, UINT sref);
	void OMSetBlendState(Direct3DBlendState9* bs, DWORD bf);
	void OMSetRenderTargets(IDirect3DSurface9* rtv, IDirect3DSurface9* dsv);

	void Draw(LPCTSTR str);

	bool SaveCurrent(LPCTSTR fn);
	bool SaveToFileD24S8(IDirect3DSurface9* ds, LPCTSTR fn);

	void StretchRect(GSTextureDX9& st, GSTextureDX9& dt, const GSVector4& dr, bool linear = true);
	void StretchRect(GSTextureDX9& st, const GSVector4& sr, GSTextureDX9& dt, const GSVector4& dr, bool linear = true);
	void StretchRect(GSTextureDX9& st, const GSVector4& sr, GSTextureDX9& dt, const GSVector4& dr, IDirect3DPixelShader9* ps, const float* ps_cb, int ps_cb_len, bool linear = true);

	HRESULT CompileShader(UINT id, LPCSTR entry, const D3DXMACRO* macro, IDirect3DVertexShader9** vs, ID3DXConstantTable** ct = NULL);
	HRESULT CompileShader(UINT id, LPCSTR entry, const D3DXMACRO* macro, IDirect3DPixelShader9** ps, ID3DXConstantTable** ct = NULL);
};
