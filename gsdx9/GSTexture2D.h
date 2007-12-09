#pragma once

class GSTexture2D
{
public:
	CComPtr<IDirect3DDevice9> m_dev;
	CComPtr<IDirect3DSurface9> m_surface;
	CComPtr<IDirect3DTexture9> m_texture; 
	D3DSURFACE_DESC m_desc;

	GSTexture2D();
	explicit GSTexture2D(IDirect3DSurface9* texture);
	explicit GSTexture2D(IDirect3DTexture9* texture);
	virtual ~GSTexture2D();

	operator bool();

	bool IsManagedTexture() const;
	bool IsOffscreenPlainTexture() const;
	bool IsRenderTarget() const;
	bool IsDepthStencil() const;

	IDirect3DTexture9* operator->();

	operator IDirect3DSurface9*();
	operator IDirect3DTexture9*();
};
