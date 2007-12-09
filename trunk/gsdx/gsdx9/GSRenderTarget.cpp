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

GSTextureCache::GSRenderTarget::GSRenderTarget(GSTextureCache* tc)
	: GSSurface(tc)
	, m_used(true)
{
}

bool GSTextureCache::GSRenderTarget::Create(int w, int h)
{
	HRESULT hr;

	hr = m_tc->m_renderer->m_dev.CreateRenderTarget(m_texture, w, h);
	
	if(FAILED(hr)) return false;

	// TODO: clear

	return true;
}

void GSTextureCache::GSRenderTarget::Update()
{
	__super::Update();

	// FIXME: the union of the rects may also update wrong parts of the render target (but a lot faster :)

	CRect r = m_dirty.GetDirtyRect(m_TEX0);

	m_dirty.RemoveAll();

	if(r.IsRectEmpty()) return;

	HRESULT hr;

	if(r.right > 1024) {ASSERT(0); r.right = 1024;}
	if(r.bottom > 1024) {ASSERT(0); r.bottom = 1024;}

	int w = r.Width();
	int h = r.Height();

	GSTexture2D texture;

	hr = m_tc->m_renderer->m_dev.CreateTexture(texture, w, h);

	if(FAILED(hr)) return;

	D3DLOCKED_RECT lr;

	if(SUCCEEDED(texture->LockRect(0, &lr, NULL, 0)))
	{
		GIFRegTEXA TEXA;

		TEXA.AEM = 1;
		TEXA.TA0 = 0;
		TEXA.TA1 = 0x80;

		GIFRegCLAMP CLAMP;

		CLAMP.WMS = 0;
		CLAMP.WMT = 0;

		m_tc->m_renderer->m_mem.ReadTexture(r, (BYTE*)lr.pBits, lr.Pitch, m_TEX0, TEXA, CLAMP);
		
		// m_tc->m_renderer->m_perfmon.Put(GSPerfMon::Unswizzle, r.Width() * r.Height() * 4);

		texture->UnlockRect(0);

		D3DXVECTOR4 dr(m_scale.x * r.left, m_scale.y * r.top, m_scale.x * r.right, m_scale.y * r.bottom);

		m_tc->m_renderer->m_dev.StretchRect(texture, m_texture, dr);
	}

	m_tc->m_renderer->m_dev.Recycle(texture);
}

void GSTextureCache::GSRenderTarget::Read(CRect r)
{
	HRESULT hr;

	if(m_TEX0.PSM != PSM_PSMCT32 
	&& m_TEX0.PSM != PSM_PSMCT24
	&& m_TEX0.PSM != PSM_PSMCT16
	&& m_TEX0.PSM != PSM_PSMCT16S)
	{
		//ASSERT(0);
		return;
	}

	TRACE(_T("GSRenderTarget::Read %d,%d - %d,%d (%08x)\n"), r.left, r.top, r.right, r.bottom, m_TEX0.TBP0);

	// m_tc->m_renderer->m_perfmon.Put(GSPerfMon::ReadRT, 1);

	//

	float left = m_scale.x * r.left / m_texture.m_desc.Width;
	float top = m_scale.y * r.top / m_texture.m_desc.Height;
	float right = m_scale.x * r.right / m_texture.m_desc.Width;
	float bottom = m_scale.y * r.bottom / m_texture.m_desc.Height;

	D3DXVECTOR4 src(left, top, right, bottom);
	D3DXVECTOR4 dst(0, 0, r.Width(), r.Height());
	
	GSTexture2D rt;

	hr = m_tc->m_renderer->m_dev.CreateRenderTarget(rt, r.Width(), r.Height());

	m_tc->m_renderer->m_dev.StretchRect(m_texture, src, rt, dst, m_tc->m_renderer->m_dev.m_ps_convert[1]);

	GSTexture2D offscreen;

	hr = m_tc->m_renderer->m_dev.CreateOffscreenPlainSurface(offscreen, r.Width(), r.Height());

	hr = m_tc->m_renderer->m_dev->GetRenderTargetData(rt, offscreen);

	m_tc->m_renderer->m_dev.Recycle(rt);

	D3DLOCKED_RECT lr;

	if(SUCCEEDED(((IDirect3DSurface9*)offscreen)->LockRect(&lr, NULL, D3DLOCK_READONLY)))
	{
		// TODO: block level write

		DWORD bp = m_TEX0.TBP0;
		DWORD bw = m_TEX0.TBW;

		GSLocalMemory::pixelAddress pa = GSLocalMemory::m_psm[m_TEX0.PSM].pa;

		BYTE* bits = (BYTE*)lr.pBits;

		if(m_TEX0.PSM == PSM_PSMCT32)
		{
			for(int y = r.top; y < r.bottom; y++, bits += lr.Pitch)
			{
				DWORD addr = pa(0, y, bp, bw);
				int* offset = GSLocalMemory::m_psm[m_TEX0.PSM].rowOffset[y & 7];

				for(int x = r.left, i = 0; x < r.right; x++, i++)
				{
					m_tc->m_renderer->m_mem.writePixel32(addr + offset[x], ((DWORD*)bits)[i]);
				}
			}
		}
		else if(m_TEX0.PSM == PSM_PSMCT24)
		{
			for(int y = r.top; y < r.bottom; y++, bits += lr.Pitch)
			{
				DWORD addr = pa(0, y, bp, bw);
				int* offset = GSLocalMemory::m_psm[m_TEX0.PSM].rowOffset[y & 7];

				for(int x = r.left, i = 0; x < r.right; x++, i++)
				{
					m_tc->m_renderer->m_mem.writePixel24(addr + offset[x], ((DWORD*)bits)[i]);
				}
			}
		}
		else
		{
			GSLocalMemory::writeFrameAddr wfa = GSLocalMemory::m_psm[m_TEX0.PSM].wfa;

			for(int y = r.top; y < r.bottom; y++, bits += lr.Pitch)
			{
				DWORD addr = pa(0, y, bp, bw);
				int* offset = GSLocalMemory::m_psm[m_TEX0.PSM].rowOffset[y & 7];

				for(int x = r.left, i = 0; x < r.right; x++, i++)
				{
					(m_tc->m_renderer->m_mem.*wfa)(addr + offset[x], ((DWORD*)bits)[i]);
				}
			}
		}

		((IDirect3DSurface9*)offscreen)->UnlockRect();
	}

	m_tc->m_renderer->m_dev.Recycle(offscreen);
}
