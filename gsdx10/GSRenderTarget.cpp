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

GSTextureCache::GSRenderTarget::GSRenderTarget(GSRendererHWDX10* renderer)
	: GSSurface(renderer)
	, m_used(true)
{
}

bool GSTextureCache::GSRenderTarget::Create(int w, int h)
{
	// FIXME: initial data should be unswizzled from local mem in Update() if dirty

	return m_renderer->m_dev.CreateRenderTarget(m_texture, w, h);
}

void GSTextureCache::GSRenderTarget::Update()
{
	__super::Update();

}

void GSTextureCache::GSRenderTarget::Read(CRect r)
{
	if(m_TEX0.PSM != PSM_PSMCT32 
	&& m_TEX0.PSM != PSM_PSMCT24
	&& m_TEX0.PSM != PSM_PSMCT16
	&& m_TEX0.PSM != PSM_PSMCT16S)
	{
		//ASSERT(0);
		return;
	}

	TRACE(_T("GSRenderTarget::Read %d,%d - %d,%d (%08x)\n"), r.left, r.top, r.right, r.bottom, m_TEX0.TBP0);

	// m_renderer->m_perfmon.Put(GSPerfMon::ReadRT, 1);

	//

	float left = m_scale.x * r.left / m_texture.GetWidth();
	float top = m_scale.y * r.top / m_texture.GetHeight();
	float right = m_scale.x * r.right / m_texture.GetWidth();
	float bottom = m_scale.y * r.bottom / m_texture.GetHeight();

	GSVector4 src(left, top, right, bottom);
	GSVector4 dst(0, 0, r.Width(), r.Height());
	
	DXGI_FORMAT format = m_TEX0.PSM == PSM_PSMCT16 || m_TEX0.PSM == PSM_PSMCT16S ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R8G8B8A8_UNORM;

	int shader = m_TEX0.PSM == PSM_PSMCT16 || m_TEX0.PSM == PSM_PSMCT16S ? 1 : 0;

	GSTextureDX10 rt;

	m_renderer->m_dev.CreateRenderTarget(rt, r.Width(), r.Height(), format);

	m_renderer->m_dev.StretchRect(m_texture, src, rt, dst, m_renderer->m_dev.m_convert.ps[shader], NULL);

	GSTextureDX10 offscreen;

	m_renderer->m_dev.CreateOffscreen(offscreen, r.Width(), r.Height(), format);

	m_renderer->m_dev->CopyResource(offscreen, rt);

	m_renderer->m_dev.Recycle(rt);

	BYTE* bits;
	int pitch;

	if(offscreen.Map(&bits, pitch))
	{
		// TODO: block level write

		DWORD bp = m_TEX0.TBP0;
		DWORD bw = m_TEX0.TBW;

		GSLocalMemory::pixelAddress pa = GSLocalMemory::m_psm[m_TEX0.PSM].pa;

		if(m_TEX0.PSM == PSM_PSMCT32)
		{
			for(int y = r.top; y < r.bottom; y++, bits += pitch)
			{
				DWORD addr = pa(0, y, bp, bw);
				int* offset = GSLocalMemory::m_psm[m_TEX0.PSM].rowOffset[y & 7];

				for(int x = r.left, i = 0; x < r.right; x++, i++)
				{
					m_renderer->m_mem.writePixel32(addr + offset[x], ((DWORD*)bits)[i]);
				}
			}
		}
		else if(m_TEX0.PSM == PSM_PSMCT24)
		{
			for(int y = r.top; y < r.bottom; y++, bits += pitch)
			{
				DWORD addr = pa(0, y, bp, bw);
				int* offset = GSLocalMemory::m_psm[m_TEX0.PSM].rowOffset[y & 7];

				for(int x = r.left, i = 0; x < r.right; x++, i++)
				{
					m_renderer->m_mem.writePixel24(addr + offset[x], ((DWORD*)bits)[i]);
				}
			}
		}
		else if(m_TEX0.PSM == PSM_PSMCT16)
		{
			for(int y = r.top; y < r.bottom; y++, bits += pitch)
			{
				DWORD addr = pa(0, y, bp, bw);
				int* offset = GSLocalMemory::m_psm[m_TEX0.PSM].rowOffset[y & 7];

				for(int x = r.left, i = 0; x < r.right; x++, i++)
				{
					m_renderer->m_mem.writePixel16(addr + offset[x], ((WORD*)bits)[i]);
				}
			}
		}
		else if(m_TEX0.PSM == PSM_PSMCT16S)
		{
			for(int y = r.top; y < r.bottom; y++, bits += pitch)
			{
				DWORD addr = pa(0, y, bp, bw);
				int* offset = GSLocalMemory::m_psm[m_TEX0.PSM].rowOffset[y & 7];

				for(int x = r.left, i = 0; x < r.right; x++, i++)
				{
					m_renderer->m_mem.writePixel16S(addr + offset[x], ((WORD*)bits)[i]);
				}
			}
		}
		else
		{
			ASSERT(0);
		}

		offscreen.Unmap();
	}

	m_renderer->m_dev.Recycle(offscreen);
}
