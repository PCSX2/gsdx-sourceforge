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
