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

GSTextureCache::GSTexture::GSTexture(GSTextureCache* tc) 
	: GSSurface(tc)
	, m_valid(0, 0, 0, 0)
	, m_bpp(0)
	, m_bpp2(0)
	, m_rendered(false)
{
	memset(m_clut, 0, sizeof(m_clut));
}

bool GSTextureCache::GSTexture::Create()
{
	// m_tc->m_renderer->m_perfmon.Put(GSPerfMon::WriteTexture, 1);

	HRESULT hr;

	m_TEX0 = m_tc->m_renderer->m_context->TEX0;
	m_CLAMP = m_tc->m_renderer->m_context->CLAMP;

	DWORD psm = m_TEX0.PSM;

	switch(psm)
	{
	case PSM_PSMT8:
	case PSM_PSMT8H:
	case PSM_PSMT4:
	case PSM_PSMT4HL:
	case PSM_PSMT4HH:
		psm = m_TEX0.CPSM;
		break;
	}

	DXGI_FORMAT format;

	switch(psm)
	{
	default:
		TRACE(_T("Invalid TEX0.PSM/CPSM (%I64d, %I64d)\n"), m_TEX0.PSM, m_TEX0.CPSM);
	case PSM_PSMCT32:
		m_bpp = 32;
		m_bpp2 = 0;
		format = DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case PSM_PSMCT24:
		m_bpp = 32;
		m_bpp2 = 1;
		format = DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case PSM_PSMCT16:
	case PSM_PSMCT16S:
		m_bpp = 16;
		m_bpp2 = 3;
		format = DXGI_FORMAT_R16_UNORM;
		break;
	}

	int w = 1 << m_TEX0.TW;
	int h = 1 << m_TEX0.TH;

	hr = m_tc->m_renderer->m_dev.CreateTexture(m_texture, w, h, format);

	return SUCCEEDED(hr);
}

bool GSTextureCache::GSTexture::Create(GSRenderTarget* rt)
{
	rt->Update();

	// m_tc->m_renderer->m_perfmon.Put(GSPerfMon::ConvertRT2T, 1);

	HRESULT hr;

	m_scale = rt->m_scale;
	m_TEX0 = m_tc->m_renderer->m_context->TEX0;
	m_CLAMP = m_tc->m_renderer->m_context->CLAMP;
	m_rendered = true;

	int tw = 1 << m_TEX0.TW;
	int th = 1 << m_TEX0.TH;
	int tp = (int)m_TEX0.TW << 6;

	int w = (int)(m_scale.x * tw + 0.5f);
	int h = (int)(m_scale.y * th + 0.5f);

	// pitch conversion

	if(rt->m_TEX0.TBW != m_TEX0.TBW) // && rt->m_TEX0.PSM == m_TEX0.PSM
	{
		// sfex3 uses this trick (bw: 10 -> 5, wraps the right side below the left)

		// ASSERT(rt->m_TEX0.TBW > m_TEX0.TBW); // otherwise scale.x need to be reduced to make the larger texture fit (TODO)

		hr = m_tc->m_renderer->m_dev.CreateRenderTarget(m_texture, rt->m_texture.m_desc.Width, rt->m_texture.m_desc.Height);

		int bw = 64;
		int bh = m_TEX0.PSM == PSM_PSMCT32 || m_TEX0.PSM == PSM_PSMCT24 ? 32 : 64;

		int sw = (int)rt->m_TEX0.TBW << 6;

		int dw = (int)m_TEX0.TBW << 6;
		int dh = 1 << m_TEX0.TH;

		for(int dy = 0; dy < dh; dy += bh)
		{
			for(int dx = 0; dx < dw; dx += bw)
			{
				int o = dy * dw / bh + dx;

				int sx = o % sw;
				int sy = o / sw;

				D3DXVECTOR4 src, dst;

				src.x = m_scale.x * sx / rt->m_texture.m_desc.Width;
				src.y = m_scale.y * sy / rt->m_texture.m_desc.Height;
				src.z = m_scale.x * (sx + bw) / rt->m_texture.m_desc.Width;
				src.w = m_scale.y * (sy + bh) / rt->m_texture.m_desc.Height;

				dst.x = m_scale.x * dx;
				dst.y = m_scale.y * dy;
				dst.z = m_scale.x * (dx + bw);
				dst.w = m_scale.y * (dy + bh);

				m_tc->m_renderer->m_dev.StretchRect(rt->m_texture, src, m_texture, dst);

				// TODO: this is quite a lot of StretchRect, do it with one Draw
			}
		}
	}
	else if(tw < tp)
	{
		// FIXME: timesplitters blurs the render target by blending itself over a couple of times

		if(tw == 256 && th == 128 && tp == 512 && (m_TEX0.TBP0 == 0 || m_TEX0.TBP0 == 0x00e00))
		{
			return false;
		}

		// TODO
	}

	// width/height conversion

	if(w != rt->m_texture.m_desc.Width || h != rt->m_texture.m_desc.Height)
	{
		D3DXVECTOR4 dst(0, 0, w, h);
		
		if(w > rt->m_texture.m_desc.Width) 
		{
			float scale = m_scale.x;
			m_scale.x = (float)rt->m_texture.m_desc.Width / tw;
			dst.z = (float)rt->m_texture.m_desc.Width * m_scale.x / scale;
			w = rt->m_texture.m_desc.Width;
		}
		
		if(h > rt->m_texture.m_desc.Height) 
		{
			float scale = m_scale.y;
			m_scale.y = (float)rt->m_texture.m_desc.Height / th;
			dst.w = (float)rt->m_texture.m_desc.Height * m_scale.y / scale;
			h = rt->m_texture.m_desc.Height;
		}

		D3DXVECTOR4 src(0, 0, w, h);

		GSTexture2D* st;
		GSTexture2D* dt;
		GSTexture2D tmp;

		if(!m_texture)
		{
			st = &rt->m_texture;
			dt = &m_texture;
		}
		else
		{
			st = &m_texture;
			dt = &tmp;
		}

		hr = m_tc->m_renderer->m_dev.CreateRenderTarget(*dt, w, h);

		if(src == dst)
		{
			D3D10_BOX box = {0, 0, 0, w, h, 1};

			m_tc->m_renderer->m_dev->CopySubresourceRegion(*dt, 0, 0, 0, 0, *st, 0, &box);
		}
		else
		{
			src.z /= st->m_desc.Width;
			src.w /= st->m_desc.Height;

			m_tc->m_renderer->m_dev.StretchRect(*st, src, *dt, dst);
		}

		if(tmp)
		{
			m_tc->m_renderer->m_dev.Recycle(m_texture);

			m_texture = tmp;
		}
	}

	if(!m_texture)
	{
		hr = m_tc->m_renderer->m_dev.CreateTexture(m_texture, rt->m_texture.m_desc.Width, rt->m_texture.m_desc.Height);

		m_tc->m_renderer->m_dev->CopyResource(m_texture, rt->m_texture);
	}

	switch(m_TEX0.PSM)
	{
	case PSM_PSMCT32:
		m_bpp2 = 0;
		break;
	case PSM_PSMCT24:
		m_bpp2 = 1;
		break;
	case PSM_PSMCT16:
	case PSM_PSMCT16S:
		m_bpp2 = 2;
		break;
	case PSM_PSMT8H:
		m_bpp2 = 4;
		hr = m_tc->m_renderer->m_dev.CreateTexture(m_palette, 256, 1, m_TEX0.CPSM == PSM_PSMCT32 ? DXGI_FORMAT_R8G8B8A8_UNORM : DXGI_FORMAT_R16_UNORM); // 
		break;
	case PSM_PSMT4HL:
	case PSM_PSMT4HH:
		ASSERT(0); // TODO
		break;
	}

	return true;
}

bool GSTextureCache::GSTexture::Create(GSDepthStencil* ds)
{
	m_rendered = true;

	// TODO

	return false;
}

void GSTextureCache::GSTexture::Update(GSLocalMemory::readTexture rt)
{
	__super::Update();

	if(m_rendered)
	{
		return;
	}

	CRect r;

	if(!GetDirtyRect(r))
	{
		return;
	}

	static BYTE* buff = (BYTE*)::_aligned_malloc(1024 * 1024 * 4, 16);

	int pitch = 1024 * m_bpp >> 3;

	BYTE* bits = buff + pitch * r.top + (r.left * m_bpp >> 3);

	(m_tc->m_renderer->m_mem.*rt)(r, bits, pitch, m_tc->m_renderer->m_context->TEX0, m_tc->m_renderer->m_env.TEXA, m_tc->m_renderer->m_context->CLAMP);

	D3D10_BOX box = {r.left, r.top, 0, r.right, r.bottom, 1};

	m_tc->m_renderer->m_dev->UpdateSubresource(m_texture, 0, &box, bits, pitch, 0); 

	// m_tc->m_renderer->m_perfmon.Put(GSPerfMon::Unswizzle, r.Width() * r.Height() * m_bpp >> 3);

	CRect r2 = m_valid & r;

	if(!r2.IsRectEmpty())
	{
		// m_tc->m_renderer->m_perfmon.Put(GSPerfMon::Unswizzle2, r2.Width() * r2.Height() * m_bpp >> 3);
	}

	m_valid |= r;
	m_dirty.RemoveAll();

	// m_tc->m_renderer->m_perfmon.Put(GSPerfMon::Texture, r.Width() * r.Height() * m_bpp >> 3);
}

bool GSTextureCache::GSTexture::GetDirtyRect(CRect& r)
{
	int w = 1 << m_TEX0.TW;
	int h = 1 << m_TEX0.TH;

	r.SetRect(0, 0, w, h);

	m_tc->m_renderer->MinMaxUV(w, h, r);

	CRect dirty = m_dirty.GetDirtyRect(m_TEX0);
	CRect valid = m_valid;

	dirty &= CRect(0, 0, m_texture.m_desc.Width, m_texture.m_desc.Height);

	if(IsRectInRect(r, valid))
	{
		if(dirty.IsRectEmpty()) return false;
		else if(IsRectInRect(dirty, r)) r = dirty;
		else if(IsRectInRect(dirty, valid)) r |= dirty;
		else r = valid & dirty;
	}
	else if(IsRectInRectH(r, valid) && (r.left >= valid.left || r.right <= valid.right))
	{
		r.top = valid.top;
		r.bottom = valid.bottom;
		if(r.left < valid.left) r.right = valid.left;
		else /*if(r.right > valid.right)*/ r.left = valid.right;
	}
	else if(IsRectInRectV(r, valid) && (r.top >= valid.top || r.bottom <= valid.bottom))
	{
		r.left = valid.left;
		r.right = valid.right;
		if(r.top < valid.top) r.bottom = valid.top;
		else /*if(r.bottom > valid.bottom)*/ r.top = valid.bottom;
	}
	else
	{
		r |= valid;
	}

	return !r.IsRectEmpty();
}
