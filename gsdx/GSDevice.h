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

#include "GSTexture.h"

template<class Texture> class GSDevice
{
	CAtlList<Texture> m_pool;

protected:
	bool Fetch(DWORD type, Texture& t, DWORD w, DWORD h, DWORD format)
	{
		Recycle(t);

		for(POSITION pos = m_pool.GetHeadPosition(); pos; m_pool.GetNext(pos))
		{
			const Texture& t2 = m_pool.GetAt(pos);

			if(t2.GetType() == type && t2.GetWidth() == w && t2.GetHeight() == h && t2.GetFormat() == format)
			{
				t = t2;

				m_pool.RemoveAt(pos);

				return true;
			}
		}

		return Create(type, t, w, h, format);
	}

	virtual bool Create(DWORD type, Texture& t, DWORD w, DWORD h, DWORD format) = 0;

public:
	GSDevice() 
	{
	}

	virtual ~GSDevice() 
	{
	}

	virtual bool Create(HWND hWnd)
	{
		return true;
	}
	
	virtual bool Reset(DWORD w, DWORD h, bool fs)
	{
		m_pool.RemoveAll();

		return true;
	}

	bool CreateRenderTarget(Texture& t, DWORD w, DWORD h, DWORD format)
	{
		return Fetch(GSTexture::RenderTarget, t, w, h, format);
	}

	bool CreateDepthStencil(Texture& t, DWORD w, DWORD h, DWORD format)
	{
		return Fetch(GSTexture::DepthStencil, t, w, h, format);
	}

	bool CreateTexture(Texture& t, DWORD w, DWORD h, DWORD format)
	{
		return Fetch(GSTexture::Texture, t, w, h, format);
	}

	bool CreateOffscreen(Texture& t, DWORD w, DWORD h, DWORD format)
	{
		return Fetch(GSTexture::Offscreen, t, w, h, format);
	}

	void Recycle(Texture& t)
	{
		if(t)
		{
			m_pool.AddHead(t);

			while(m_pool.GetCount() > 200)
			{
				m_pool.RemoveTail();
			}

			t = Texture();
		}
	}
};
