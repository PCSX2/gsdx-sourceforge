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

#include "GSState.h"
#include "GSRasterizer.h"
#include "GSAlignedClass.h"

class GSDrawScanline : public GSAlignedClass<16>, public IDrawScanline
{
	union ScanlineSelector
	{
		struct
		{
			DWORD fpsm:2; // 0
			DWORD zpsm:2; // 2
			DWORD ztst:2; // 4 (0: off, 1: write, 2: test (ge), 3: test (g))
			DWORD iip:1; // 6
			DWORD tfx:3; // 7
			DWORD tcc:1; // 10
			DWORD fst:1; // 11
			DWORD ltf:1; // 12
			DWORD atst:3; // 13
			DWORD afail:2; // 16
			DWORD fge:1; // 18
			DWORD date:1; // 19
			DWORD abea:2; // 20
			DWORD abeb:2; // 22
			DWORD abec:2; // 24
			DWORD abed:2; // 26
			DWORD pabe:1; // 28
			DWORD rfb:1; // 29
			DWORD wzb:1; // 30
			DWORD tlu:1; // 31
		};

		struct
		{
			DWORD _pad1:20;
			DWORD abe:8;
			DWORD _pad2:4;
		};

		DWORD dw;

		operator DWORD() {return dw;}// & 0x7fffffff;}
	};

	__declspec(align(16)) struct ScanlineEnvironment
	{
		void* vm;

		const void* tex;
		const DWORD* clut;
		DWORD tw;

		GSVector4i* fbr;
		GSVector4i* zbr;
		int** fbc;
		int** zbc;

		GSVector4i fm, zm;
		struct {GSVector4i min, max, mask;} t; // [u] x 4 [v] x 4
		GSVector4i datm;
		GSVector4i colclamp;
		GSVector4i fba;
		GSVector4i aref;
		GSVector4 afix;
		GSVector4 afix2;
		GSVector4 fc;

		GSVector4 dp, dp4;
		GSVector4 dt, dt4;
		GSVector4 dc, dc4;
	};

	struct Offset
	{
		GSVector4i row[1024];
		int* col[4];
		DWORD hash;
	};

	ScanlineSelector m_sel;
	ScanlineEnvironment m_slenv;

	CRBMapC<DWORD, Offset*> m_omap;
	Offset* m_fbo;
	Offset* m_zbo;

	void SetupOffset(Offset*& co, DWORD bp, DWORD bw, DWORD psm);
	void FreeOffsets();

	DrawScanlinePtr m_ds[4][4][4][2], m_dsf;
	CRBMap<DWORD, DrawScanlinePtr> m_dsmap, m_dsmap2;

	void Init();

	template<DWORD fpsm, DWORD zpsm, DWORD ztst, DWORD iip>
	void DrawScanlineT(int top, int left, int right, const Vertex& v);

	template<DWORD sel> 
	void DrawScanlineExT(int top, int left, int right, const Vertex& v);

	__forceinline GSVector4i Wrap(const GSVector4i& t);

	__forceinline void SampleTexture(int pixels, DWORD ztst, DWORD ltf, DWORD pal, const GSVector4i& test, const GSVector4& u, const GSVector4& v, GSVector4* c);
	__forceinline void ColorTFX(DWORD tfx, const GSVector4& rf, const GSVector4& gf, const GSVector4& bf, const GSVector4& af, GSVector4& rt, GSVector4& gt, GSVector4& bt);
	__forceinline void AlphaTFX(DWORD tfx, DWORD tcc, const GSVector4& af, GSVector4& at);
	__forceinline void Fog(const GSVector4& f, GSVector4& r, GSVector4& g, GSVector4& b);
	__forceinline bool TestZ(DWORD zpsm, DWORD ztst, const GSVector4i& zs, const GSVector4i& za, GSVector4i& test);
	__forceinline bool TestAlpha(DWORD atst, DWORD afail, const GSVector4& a, GSVector4i& fm, GSVector4i& zm, GSVector4i& test);

	__forceinline static DWORD ReadPixel32(DWORD* RESTRICT vm, DWORD addr);
	__forceinline static DWORD ReadPixel24(DWORD* RESTRICT vm, DWORD addr);
	__forceinline static DWORD ReadPixel16(WORD* RESTRICT vm, DWORD addr);
	__forceinline static void WritePixel32(DWORD* RESTRICT vm, DWORD addr, DWORD c);
	__forceinline static void WritePixel24(DWORD* RESTRICT vm, DWORD addr, DWORD c);
	__forceinline static void WritePixel16(WORD* RESTRICT vm, DWORD addr, DWORD c);

	__forceinline GSVector4i ReadFrameX(int psm, const GSVector4i& addr) const;
	__forceinline GSVector4i ReadZBufX(int psm, const GSVector4i& addr) const;
	__forceinline void WriteFrameAndZBufX(int fpsm, const GSVector4i& fa, const GSVector4i& fm, const GSVector4i& f, int zpsm, const GSVector4i& za, const GSVector4i& zm, const GSVector4i& z, int pixels);

protected:
	GSState* m_state;

public:
	GSDrawScanline(GSState* state);
	virtual ~GSDrawScanline();

	// IDrawScanline

	void SetupDraw(Vertex* vertices, int count, const void* texture);
	void SetupScanline(const Vertex& dv);
	void DrawScanline(int top, int left, int right, const Vertex& v);
	void FillRect(const GSVector4i& r, const Vertex& v);
	DrawScanlinePtr GetDrawScanlinePtr();
};
