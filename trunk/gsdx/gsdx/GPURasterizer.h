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

#include "GPUState.h"
#include "GPUVertexSW.h"
#include "GPUTextureCacheSW.h"
#include "GSAlignedClass.h"

class GPURasterizer : public GSAlignedClass<16>
{
protected:
	typedef GPUVertexSW Vertex;

	GPUState* m_state;
	int m_id;
	int m_threads;

private:

	union ScanlineSelector
	{
		struct
		{
			DWORD iip:1; // 0
			DWORD me:1; // 1
			DWORD abe:1; // 2
			DWORD abr:2; // 3
			DWORD tge:1; // 5
			DWORD tme:1; // 6
			DWORD tlu:1; // 7
			DWORD twin:1; // 8
			DWORD dtd:1; // 9
			DWORD ltf:1; // 10
			// DWORD dte:1: // 11
		};

		struct
		{
			DWORD _pad1:1; // 0
			DWORD rfb:2; // 1
			DWORD _pad2:2; // 3
			DWORD tfx:2; // 5
		};

		DWORD dw;

		operator DWORD() {return dw & 0x1ff;}
	};
	
	__declspec(align(16)) struct ScanlineEnvironment
	{
		int steps;

		WORD* vm;

		const void* tex;
		const WORD* clut;

		GSVector4i u[2];
		GSVector4i v[2];

		GSVector4i a;
		GSVector4i md; // similar to gs fba

		GSVector4 dp, dp8;
		GSVector4 dc, dc8;
	};

	GSVector4i m_scissor;
	ScanlineSelector m_sel;
	ScanlineEnvironment m_slenv;

	template<bool pos, bool col> 
	__forceinline void SetupScanline(const Vertex& dv);

	typedef void (GPURasterizer::*DrawScanlinePtr)(int top, int left, int right, const Vertex& v);

	DrawScanlinePtr m_ds[512], m_dsf;

	void DrawScanline(int top, int left, int right, const Vertex& v);

	template<DWORD sel> 
	void DrawScanlineEx(int top, int left, int right, const Vertex& v);

	__forceinline void SampleTexture(int pixels, DWORD ltf, DWORD tlu, DWORD twin, GSVector4i& test, const GSVector4* s, const GSVector4* t, GSVector4i* c);
	__forceinline void ColorTFX(DWORD tfx, const GSVector4* r, const GSVector4* g, const GSVector4* b, GSVector4i* c);
	__forceinline void AlphaBlend(UINT32 abr, UINT32 tme, const GSVector4i& d, GSVector4i* c);
	__forceinline void WriteFrame(WORD* RESTRICT fb, const GSVector4i& test, const GSVector4i* c, int pixels);

	void DrawPoint(Vertex* v);
	void DrawLine(Vertex* v);
	void DrawTriangle(Vertex* v);
	void DrawTriangleTop(Vertex* v);
	void DrawTriangleBottom(Vertex* v);
	void DrawTriangleTopBottom(Vertex* v);
	void DrawSprite(Vertex* v);

	__forceinline void DrawTriangleSection(int top, int bottom, Vertex& l, const Vertex& dl, GSVector4& r, const GSVector4& dr, const Vertex& dscan);

public:
	GPURasterizer(GPUState* state, int id = 0, int threads = 0);
	virtual ~GPURasterizer();

	int Draw(Vertex* v, int count, const void* texture);
};
