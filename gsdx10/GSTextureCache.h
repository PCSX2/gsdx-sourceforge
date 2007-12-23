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

#include "GSDeviceDX10.h"

class GSTextureCacheHWDX10 : public GSTextureCache<GSDeviceDX10>
{
	class GSRenderTargetHWDX10 : public GSRenderTarget
	{
	public:
		explicit GSRenderTargetHWDX10(GSRenderer<GSDeviceDX10>* renderer) : GSRenderTarget(renderer) {}

		void Update();
		void Read(CRect r);
	};

	class GSDepthStencilHWDX10 : public GSDepthStencil
	{
	public:
		explicit GSDepthStencilHWDX10(GSRenderer<GSDeviceDX10>* renderer) : GSDepthStencil(renderer) {}

		void Update();
	};

	class GSTextureHWDX10 : public GSTexture
	{
	public:
		explicit GSTextureHWDX10(GSRenderer<GSDeviceDX10>* renderer) : GSTexture(renderer) {}

		bool Create();
		bool Create(GSRenderTarget* rt);
		bool Create(GSDepthStencil* ds);
		void Update();
	};

protected:
	GSRenderTarget* CreateRenderTarget() {return new GSRenderTargetHWDX10(m_renderer);}
	GSDepthStencil* CreateDepthStencil() {return new GSDepthStencilHWDX10(m_renderer);}
	GSTexture* CreateTexture() {return new GSTextureHWDX10(m_renderer);}

public:
	GSTextureCacheHWDX10(GSRenderer<GSDeviceDX10>* renderer, bool nativeres);
};
