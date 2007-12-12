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
	HRESULT hr;

	hr = m_tc->m_renderer->m_dev.CreateDepthStencil(m_texture, w, h);

	// FIXME: initial data should be unswizzled from local mem in Update() if dirty

	m_tc->m_renderer->m_dev->ClearDepthStencilView(m_texture, D3D10_CLEAR_DEPTH, 0, 0);

	return SUCCEEDED(hr);
}

void GSTextureCache::GSDepthStencil::Update()
{
	__super::Update();

	// TODO: dx 10.1 could update ds directly
}

