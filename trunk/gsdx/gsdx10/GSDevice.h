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

#include "GSTexture2D.h"

#pragma pack(push, 1)

struct InterlaceCB
{
	D3DXVECTOR2 ZrH;
	float hH;
	float _pad[1];
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

#pragma pack(pop)

class GSDevice
{
	// texture cache

	CAtlList<GSTexture2D> m_pool;

	// state cache

	ID3D10Buffer* m_vb;
	UINT m_vb_stride;

	ID3D10InputLayout* m_layout;
	D3D10_PRIMITIVE_TOPOLOGY m_topology;

	ID3D10VertexShader* m_vs;
	ID3D10Buffer* m_vs_cb;

	ID3D10GeometryShader* m_gs;

	ID3D10ShaderResourceView* m_ps_srvs[2];

	ID3D10PixelShader* m_ps;
	ID3D10SamplerState* m_ps_ss;

	CSize m_viewport;
	CRect m_scissor;

	ID3D10DepthStencilState* m_dss;
	UINT m_sref;
	ID3D10BlendState* m_bs;
	float m_bf;
	ID3D10RenderTargetView* m_rtv;
	ID3D10DepthStencilView* m_dsv;

	//

	void Interlace(GSTexture2D& st, GSTexture2D& dt, int shader, bool linear, float yoffset = 0);

public: // TODO
	CComPtr<ID3D10Device> m_dev;
	CComPtr<IDXGISwapChain> m_swapchain;
	CComPtr<ID3D10Texture2D> m_backbuffer;
	CComPtr<ID3D10Texture2D> m_tex_current;

	GSTexture2D m_tex_merge;
	GSTexture2D m_tex_interlace;
	GSTexture2D m_tex_deinterlace;
	GSTexture2D m_tex_1x1;

	CComPtr<ID3D10SamplerState> m_ss_linear;
	CComPtr<ID3D10SamplerState> m_ss_point;

	CComPtr<ID3D10RasterizerState> m_rs;

	struct
	{
		CComPtr<ID3D10Buffer> vb;
		CComPtr<ID3D10InputLayout> il;
		CComPtr<ID3D10VertexShader> vs;
		CComPtr<ID3D10PixelShader> ps[5];
		CComPtr<ID3D10DepthStencilState> dss;
		CComPtr<ID3D10BlendState> bs;
	} m_convert;

	struct
	{
		CComPtr<ID3D10PixelShader> ps[4];
		CComPtr<ID3D10Buffer> cb;
	} m_interlace;

public:
	GSDevice();
	virtual ~GSDevice();

	bool Create(HWND hWnd);

	ID3D10Device* operator->() {return m_dev;}
	operator ID3D10Device*() {return m_dev;}

	void ResetDevice(int w, int h);
	void EndScene();
	void Present();

	void IASet(ID3D10Buffer* vb, UINT count, const void* vertices, UINT stride, ID3D10InputLayout* layout, D3D10_PRIMITIVE_TOPOLOGY topology);
	void VSSet(ID3D10VertexShader* vs, ID3D10Buffer* vs_cb);
	void GSSet(ID3D10GeometryShader* gs);
	void PSSetShaderResources(ID3D10ShaderResourceView* srv0, ID3D10ShaderResourceView* srv1);
	void PSSet(ID3D10PixelShader* ps, ID3D10SamplerState* ss);
	void RSSet(int width, int height, const RECT* scissor = NULL);
	void OMSet(ID3D10DepthStencilState* dss, UINT sref, ID3D10BlendState* bs, float bf);
	void OMSetRenderTargets(ID3D10RenderTargetView* rtv, ID3D10DepthStencilView* dsv);

	template<class T> void IASet(ID3D10Buffer* vb, UINT count, T* vertices, ID3D10InputLayout* layout, D3D10_PRIMITIVE_TOPOLOGY topology)
	{
		IASet(vb, count, vertices, sizeof(T), layout, topology);
	}

	HRESULT CreateRenderTarget(GSTexture2D& t, int w, int h, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);
	HRESULT CreateDepthStencil(GSTexture2D& t, int w, int h, DXGI_FORMAT format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT);
	HRESULT CreateTexture(GSTexture2D& t, int w, int h, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);
	HRESULT CreateOffscreenPlainSurface(GSTexture2D& t, int w, int h, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);
	HRESULT Create(GSTexture2D& t, int w, int h, DXGI_FORMAT format, D3D10_USAGE usage, UINT bind);	

	void Recycle(GSTexture2D& t);

	bool SaveCurrent(LPCTSTR fn);
	bool SaveToFileD32S8X24(ID3D10Texture2D* ds, LPCTSTR fn);

	void StretchRect(GSTexture2D& st, GSTexture2D& dt, const D3DXVECTOR4& dr, bool linear = true);
	void StretchRect(GSTexture2D& st, const D3DXVECTOR4& sr, GSTexture2D& dt, const D3DXVECTOR4& dr, bool linear = true);
	void StretchRect(GSTexture2D& st, const D3DXVECTOR4& sr, GSTexture2D& dt, const D3DXVECTOR4& dr, ID3D10PixelShader* ps, bool linear = true);

	ID3D10Texture2D* Interlace(GSTexture2D& st, CSize ds, int field, int mode, float yoffset);

	HRESULT CompileShader(ID3D10VertexShader** vs, UINT id, LPCSTR entry, D3D10_INPUT_ELEMENT_DESC* layout, int count, ID3D10InputLayout** pl, D3D10_SHADER_MACRO* macro = NULL);
	HRESULT CompileShader(ID3D10GeometryShader** gs, UINT id, LPCSTR entry, D3D10_SHADER_MACRO* macro = NULL);
	HRESULT CompileShader(ID3D10PixelShader** ps, UINT id, LPCSTR entry, D3D10_SHADER_MACRO* macro = NULL);
};
