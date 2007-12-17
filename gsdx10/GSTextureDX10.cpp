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
#include "GSTextureDX10.h"

GSTextureDX10::GSTextureDX10()
{
	memset(&m_desc, 0, sizeof(m_desc));
}

GSTextureDX10::GSTextureDX10(ID3D10Texture2D* texture)
	: m_texture(texture)
{
	ASSERT(m_texture);

	m_texture->GetDevice(&m_dev);
	m_texture->GetDesc(&m_desc);
}

GSTextureDX10::~GSTextureDX10()
{
}

GSTextureDX10::operator bool()
{
	return !!m_texture;
}

DWORD GSTextureDX10::GetType() const
{
	if(m_desc.BindFlags & D3D10_BIND_RENDER_TARGET) return GSTexture::RenderTarget;
	if(m_desc.BindFlags & D3D10_BIND_DEPTH_STENCIL) return GSTexture::DepthStencil;
	if(m_desc.BindFlags & D3D10_BIND_SHADER_RESOURCE) return GSTexture::Texture;
	if(m_desc.Usage == D3D10_USAGE_STAGING) return GSTexture::Offscreen;
	return GSTexture::None;
}

DWORD GSTextureDX10::GetWidth() const 
{
	return m_desc.Width;
}

DWORD GSTextureDX10::GetHeight() const 
{
	return m_desc.Height;
}

DWORD GSTextureDX10::GetFormat() const 
{
	return m_desc.Format;
}

ID3D10Texture2D* GSTextureDX10::operator->()
{
	return m_texture;
}

GSTextureDX10::operator ID3D10Texture2D*()
{
	return m_texture;
}

GSTextureDX10::operator ID3D10ShaderResourceView*()
{
	if(!m_srv && m_dev && m_texture)
	{
		m_dev->CreateShaderResourceView(m_texture, NULL, &m_srv);
	}

	return m_srv;
}

GSTextureDX10::operator ID3D10RenderTargetView*()
{
	ASSERT(m_dev);

	if(!m_rtv && m_dev && m_texture)
	{
		m_dev->CreateRenderTargetView(m_texture, NULL, &m_rtv);
	}

	return m_rtv;
}

GSTextureDX10::operator ID3D10DepthStencilView*()
{
	if(!m_dsv && m_dev && m_texture)
	{
		m_dev->CreateDepthStencilView(m_texture, NULL, &m_dsv);
	}

	return m_dsv;
}
