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
#include "GSTextureCache.h"
#include "GSRendererHW.h"

GSTextureCache::GSDepthStencil::GSDepthStencil(GSTextureCache* tc)
	: GSSurface(tc)
	, m_used(false)
{
}

bool GSTextureCache::GSDepthStencil::Create(int w, int h)
{
	if(!m_tc->m_renderer->m_dev.CreateDepthStencil(m_texture, w, h))
	{
		return false;
	}

	// FIXME: initial data should be unswizzled from local mem in Update() if dirty

	CComPtr<IDirect3DSurface9> surface;

	m_tc->m_renderer->m_dev->GetDepthStencilSurface(&surface);
	m_tc->m_renderer->m_dev->SetDepthStencilSurface(m_texture);
	m_tc->m_renderer->m_dev->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0, 0, 0);
	m_tc->m_renderer->m_dev->SetDepthStencilSurface(surface);

	return true;
}

void GSTextureCache::GSDepthStencil::Update()
{
	__super::Update();

	// TODO
}

