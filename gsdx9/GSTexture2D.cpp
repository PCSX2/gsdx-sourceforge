#include "stdafx.h"
#include "GSTexture2D.h"

GSTexture2D::GSTexture2D()
{
	memset(&m_desc, 0, sizeof(m_desc));
}

GSTexture2D::GSTexture2D(IDirect3DSurface9* surface)
{
	m_surface = surface;

	surface->GetDevice(&m_dev);
	surface->GetDesc(&m_desc);
	
	if(m_desc.Type != D3DRTYPE_SURFACE)
	{
		HRESULT hr = surface->GetContainer(__uuidof(IDirect3DTexture9), (void**)&m_texture);
		ASSERT(SUCCEEDED(hr));
	}
}

GSTexture2D::GSTexture2D(IDirect3DTexture9* texture)
{
	m_texture = texture;

	texture->GetDevice(&m_dev);
	texture->GetLevelDesc(0, &m_desc);
	texture->GetSurfaceLevel(0, &m_surface);
}

GSTexture2D::~GSTexture2D()
{
}

GSTexture2D::operator bool()
{
	return !!m_surface;
}

bool GSTexture2D::IsManagedTexture() const
{
	return m_desc.Pool == D3DPOOL_MANAGED;
}

bool GSTexture2D::IsRenderTarget() const
{
	return !!(m_desc.Usage & D3DUSAGE_RENDERTARGET);
}

bool GSTexture2D::IsDepthStencil() const
{
	return !!(m_desc.Usage & D3DUSAGE_DEPTHSTENCIL);
}

IDirect3DTexture9* GSTexture2D::operator->()
{
	return m_texture;
}

GSTexture2D::operator IDirect3DSurface9*()
{
	if(m_texture && !m_surface)
	{
		m_texture->GetSurfaceLevel(0, &m_surface);
	}

	return m_surface;
}

GSTexture2D::operator IDirect3DTexture9*()
{
	if(m_surface && !m_texture)
	{
		m_surface->GetContainer(__uuidof(IDirect3DTexture9), (void**)&m_texture);

		ASSERT(m_texture);
	}

	return m_texture;
}
