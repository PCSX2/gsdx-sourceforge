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
#include "GSVertexSW.h"

// - texture cache size should be the size of the page (depends on psm, ttbl->pgs), 
// but the min fetchable 4 bpp block size (32x16) is the fastest and 99% ok
// - there should be only one page in the cache, addressing another is equal to a TEXFLUSH

#define TEXTURE_CACHE_WIDTH 5
#define TEXTURE_CACHE_HEIGHT 4

// FIXME: the fog effect in re4 needs even bigger cache (128x128) to leave no black lines at the edges (might be the bilinear filter)

class GSRasterizer
{
protected:
	typedef GSVertexSW Vertex;

	GSState* m_state;
	int m_id;
	int m_threads;

private:
	struct ColumnOffset
	{
		GSVector4i addr[1024]; 
		DWORD hash;
	};

	struct ScanlineEnvironment
	{
		int** fo;
		int** zo;

		GSVector4i fm, zm;
		struct {GSVector4i min, max, mask;} t; // [u] x 4 [v] x 4
		GSVector4i datm;
		GSVector4i colclamp;
		GSVector4i fba;
		GSVector4i aref;
		GSVector4 afix;
		struct {GSVector4 r, g, b;} f;

		GSVector4 dz0123, dz;
		GSVector4 df0123, df;
		GSVector4 dt0123, dt;
		GSVector4 ds0123, ds;
		GSVector4 dq0123, dq;
		GSVector4 dr0123, dr;
		GSVector4 dg0123, dg;
		GSVector4 db0123, db;
		GSVector4 da0123, da;
	};

	union ScanlineSelector
	{
		struct
		{
			DWORD fpsm:3; // 0
			DWORD zpsm:2; // 3
			DWORD ztst:2; // 5 (0: off, 1: write, 2: write + test (ge), 3: write + test (g))
			DWORD iip:1; // 7
			DWORD tfx:3; // 8
			DWORD tcc:1; // 11
			DWORD fst:1; // 12
			DWORD ltf:1; // 13
			DWORD atst:3; // 14
			DWORD afail:2; // 17
			DWORD fge:1; // 19
			DWORD rfb:1; // 20
			DWORD date:1; // 21
			DWORD abe:2; // 22
			DWORD abea:2; // 24
			DWORD abeb:2; // 26
			DWORD abec:2; // 28
			DWORD abed:2; // 30
		};

		DWORD dw;

		operator DWORD() {return dw & 0xffffffff;}
	};

	CRect m_scissor;
	DWORD* m_cache;
	DWORD m_page[(1024 / (1 << TEXTURE_CACHE_HEIGHT)) * (1024 / (1 << TEXTURE_CACHE_WIDTH))];
	DWORD m_pagehash;
	DWORD m_pagedirty;
	CSimpleMap<DWORD, ColumnOffset*> m_comap;
	ColumnOffset* m_fbco;
	ColumnOffset* m_zbco;
	ScanlineSelector m_sel;
	ScanlineEnvironment* m_slenv;
	bool m_solidrect;

	void SetupColumnOffset();

	template<bool pos, bool tex, bool col> 
	__forceinline void SetupScanline(const Vertex& dv);

	typedef void (GSRasterizer::*DrawScanlinePtr)(int top, int left, int right, const Vertex& v);

	DrawScanlinePtr m_ds[8][4][4][2], m_dsf;
	CAtlMap<DWORD, DrawScanlinePtr> m_dsmap, m_dsmap2;

	template<int iFPSM, int iZPSM, int iZTST, int iIIP>
	void DrawScanline(int top, int left, int right, const Vertex& v);

	void InitEx();

	template<DWORD sel> 
	void DrawScanlineEx(int top, int left, int right, const Vertex& v);

	void FetchTexture(int x, int y);

	__forceinline void FetchTexel(int x, int y)
	{
		int i = x >> TEXTURE_CACHE_WIDTH;
		int j = y & ~((1 << TEXTURE_CACHE_HEIGHT) - 1);
		int k = 10 - (TEXTURE_CACHE_WIDTH + TEXTURE_CACHE_HEIGHT);

		if(k > 0) i += j << k;
		else if(k < 0) i += j >> -k;
		else i += j;

		if(m_page[i] != m_pagehash)
		{
			m_page[i] = m_pagehash;
			m_pagedirty = true;

			FetchTexture(x, y);
		}
	}

	__forceinline DWORD ReadTexelNoFetch(int x, int y)
	{
		return m_cache[y * 1024 + x];
	}

	__forceinline DWORD ReadTexel(int x, int y)
	{
		FetchTexel(x, y);

		return ReadTexelNoFetch(x, y);
	}

	__forceinline GSVector4i Wrap(const GSVector4i& t)
	{
		GSVector4i clamp = t.sat_i16(m_slenv->t.min, m_slenv->t.max);
		GSVector4i repeat = (t & m_slenv->t.min) | m_slenv->t.max;

 		return clamp.blend8(repeat, m_slenv->t.mask);
	}

	void DrawPoint(Vertex* v);
	void DrawLine(Vertex* v);
	void DrawTriangle(Vertex* v);
	void DrawSprite(Vertex* v);
	bool DrawSolidRect(int left, int top, int right, int bottom, const Vertex& v);

	__forceinline void DrawTriangleSection(Vertex& l, const Vertex& dl, GSVector4& r, const GSVector4& dr, const GSVector4& b, const Vertex& dscan);

public:
	GSRasterizer(GSState* state, int id = 0, int threads = 0);
	virtual ~GSRasterizer();

	int Draw(Vertex* v, int count);
	void InvalidateTextureCache();
};

class GSRasterizerMT : public GSRasterizer
{
	Vertex* m_vertices;
	int m_count;
	long* m_sync;
	bool m_exit;
    DWORD m_ThreadId;
    HANDLE m_hThread;

	static DWORD WINAPI StaticThreadProc(LPVOID lpParam);

	DWORD ThreadProc();

public:
	GSRasterizerMT(GSState* state, int id, int threads, long* sync);
	virtual ~GSRasterizerMT();

	void BeginDraw(Vertex* vertices, int count);
};