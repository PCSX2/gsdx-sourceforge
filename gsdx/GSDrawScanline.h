/* 
 *	Copyright (C) 2007-2009 Gabest
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

union GSScanlineSelector
{
	struct
	{
		DWORD fpsm:2; // 0
		DWORD zpsm:2; // 2
		DWORD ztst:2; // 4 (0: off, 1: write, 2: test (ge), 3: test (g))
		DWORD atst:3; // 6
		DWORD afail:2; // 9
		DWORD iip:1; // 11
		DWORD tfx:3; // 12
		DWORD tcc:1; // 15
		DWORD fst:1; // 16
		DWORD ltf:1; // 17
		DWORD tlu:1; // 18
		DWORD fge:1; // 19
		DWORD date:1; // 20
		DWORD abea:2; // 21
		DWORD abeb:2; // 23
		DWORD abec:2; // 25
		DWORD abed:2; // 27
		DWORD pabe:1; // 29
		DWORD rfb:1; // 30
	};

	struct
	{
		DWORD _pad1:21;
		DWORD abe:8;
		DWORD _pad2:3;
	};

	DWORD dw;

	operator DWORD() {return dw & 0x7fffffff;}
};

__declspec(align(16)) struct GSScanlineEnvironment
{
	GSScanlineSelector sel;

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
	GSVector4i afix, afix2;
	GSVector4i frb, fga;

	GSVector4 dz, dz4;
	GSVector4i df, df4;
	GSVector4 dt, dt4;
	GSVector4i drb, dga, dc4;
};

__declspec(align(16)) struct GSScanlineParam
{
	GSScanlineSelector sel;

	void* vm;
	const void* tex;
	const DWORD* clut;
	DWORD tw;

	GSLocalMemory::Offset* fbo;
	GSLocalMemory::Offset* zbo;

	DWORD fm, zm;
};

class GSDrawScanline : public GSAlignedClass<16>, public IDrawScanline
{
	struct ActiveDrawScanlinePtr
	{
		UINT64 frame;
		UINT64 frames;
		__int64 ticks;
		__int64 pixels;
		DrawScanlinePtr dsf;
	};

	GSScanlineEnvironment m_env;

	DrawScanlinePtr m_ds[4][4][4][2];
	CRBMap<DWORD, DrawScanlinePtr> m_dsmap;
	CRBMap<DWORD, ActiveDrawScanlinePtr*> m_dsmap_active;
	ActiveDrawScanlinePtr* m_dsf;

	void Init();

	template<DWORD fpsm, DWORD zpsm, DWORD ztst, DWORD iip>
	void DrawScanlineT(int top, int left, int right, const GSVertexSW& v);

	template<DWORD sel> 
	void DrawScanlineExT(int top, int left, int right, const GSVertexSW& v);

	__forceinline GSVector4i Wrap(const GSVector4i& t);

	__forceinline void SampleTexture(int pixels, DWORD ztst, DWORD fst, DWORD ltf, DWORD tlu, const GSVector4i& test, const GSVector4& s, const GSVector4& t, const GSVector4& q, GSVector4i* c);
	__forceinline void ColorTFX(DWORD tfx, const GSVector4i& rbf, const GSVector4i& gaf, GSVector4i& rbt, GSVector4i& gat);
	__forceinline void AlphaTFX(DWORD tfx, DWORD tcc, const GSVector4i& gaf, GSVector4i& gat);
	__forceinline void Fog(DWORD fge, const GSVector4i& f, GSVector4i& rb, GSVector4i& ga);
	__forceinline bool TestZ(DWORD zpsm, DWORD ztst, const GSVector4i& zs, const GSVector4i& za, GSVector4i& test);
	__forceinline bool TestAlpha(DWORD atst, DWORD afail, const GSVector4i& ga, GSVector4i& fm, GSVector4i& zm, GSVector4i& test);
	__forceinline bool TestDestAlpha(DWORD fpsm, DWORD date, const GSVector4i& d, GSVector4i& test);

	__forceinline static DWORD ReadPixel32(DWORD* RESTRICT vm, DWORD addr);
	__forceinline static DWORD ReadPixel24(DWORD* RESTRICT vm, DWORD addr);
	__forceinline static DWORD ReadPixel16(WORD* RESTRICT vm, DWORD addr);
	__forceinline static void WritePixel32(DWORD* RESTRICT vm, DWORD addr, DWORD c);
	__forceinline static void WritePixel24(DWORD* RESTRICT vm, DWORD addr, DWORD c);
	__forceinline static void WritePixel16(WORD* RESTRICT vm, DWORD addr, DWORD c);

	__forceinline GSVector4i ReadFrameX(int psm, const GSVector4i& addr) const;
	__forceinline GSVector4i ReadZBufX(int psm, const GSVector4i& addr) const;
	__forceinline void WriteFrameAndZBufX(int fpsm, const GSVector4i& fa, const GSVector4i& fm, const GSVector4i& f, int zpsm, const GSVector4i& za, const GSVector4i& zm, const GSVector4i& z, int pixels);

	template<class T, bool masked> 
	void DrawSolidRectT(const GSVector4i* row, int* col, const GSVector4i& r, DWORD c, DWORD m);

	template<class T, bool masked> 
	__forceinline void FillRect(const GSVector4i* row, int* col, const GSVector4i& r, const GSVector4i& c, const GSVector4i& m);

	template<class T, bool masked> 
	__forceinline void FillBlock(const GSVector4i* row, int* col, const GSVector4i& r, const GSVector4i& c, const GSVector4i& m);

protected:
	GSState* m_state;
	int m_id;

public:
	GSDrawScanline(GSState* state, int id);
	virtual ~GSDrawScanline();

	// IDrawScanline

	bool BeginDraw(const GSRasterizerData* data);
	void EndDraw(const GSRasterizerStats& stats);
	void SetupPrim(GS_PRIM_CLASS primclass, const GSVertexSW* vertices, const GSVertexSW& dscan);
	void DrawScanline(int top, int left, int right, const GSVertexSW& v);
	void DrawSolidRect(const GSVector4i& r, const GSVertexSW& v);
	DrawScanlinePtr GetDrawScanlinePtr();

	void PrintStats();
};
