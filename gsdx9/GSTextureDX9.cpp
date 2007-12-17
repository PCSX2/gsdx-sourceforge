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
#include "GSTextureDX9.h"

GSTextureDX9::GSTextureDX9()
{
	memset(&m_desc, 0, sizeof(m_desc));
}

GSTextureDX9::GSTextureDX9(IDirect3DSurface9* surface)
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

GSTextureDX9::GSTextureDX9(IDirect3DTexture9* texture)
{
	m_texture = texture;

	texture->GetDevice(&m_dev);
	texture->GetLevelDesc(0, &m_desc);
	texture->GetSurfaceLevel(0, &m_surface);
}

GSTextureDX9::~GSTextureDX9()
{
}

GSTextureDX9::operator bool()
{
	return !!m_surface;
}

int GSTextureDX9::GetType() const
{
	if(m_desc.Usage & D3DUSAGE_RENDERTARGET) return GSTexture::RenderTarget;
	if(m_desc.Usage & D3DUSAGE_DEPTHSTENCIL) return GSTexture::DepthStencil;
	if(m_desc.Pool == D3DPOOL_MANAGED) return GSTexture::Texture;
	if(m_desc.Pool == D3DPOOL_SYSTEMMEM) return GSTexture::Offscreen;
	return GSTexture::None;
}

DWORD GSTextureDX9::GetWidth() const 
{
	return m_desc.Width;
}

DWORD GSTextureDX9::GetHeight() const 
{
	return m_desc.Height;
}

DWORD GSTextureDX9::GetFormat() const 
{
	return m_desc.Format;
}

IDirect3DTexture9* GSTextureDX9::operator->()
{
	return m_texture;
}

GSTextureDX9::operator IDirect3DSurface9*()
{
	if(m_texture && !m_surface)
	{
		m_texture->GetSurfaceLevel(0, &m_surface);
	}

	return m_surface;
}

GSTextureDX9::operator IDirect3DTexture9*()
{
	if(m_surface && !m_texture)
	{
		m_surface->GetContainer(__uuidof(IDirect3DTexture9), (void**)&m_texture);

		ASSERT(m_texture);
	}

	return m_texture;
}
