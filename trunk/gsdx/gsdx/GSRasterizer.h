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

class GSRasterizer
{
	typedef GSVertexSW Vertex;
	typedef GSVertexSW::Scalar Scalar;
	typedef GSVertexSW::Vector Vector;

	struct ColumnOffset
	{
		__m128i addr[1024]; 
		DWORD hash;
	};

	struct Scanline
	{
		__m128i fa, fm, t, za, zm, z;
		__m128 u, v, q, r[3], g[3], b[3], a[3], f;
	};

	struct ScanlineEnv
	{
		__m128i fm, zm;
		struct {__m128 r, g, b;} f;
		struct {union {struct {__m128i min, max;}; struct {__m128i and, or;};}; __m128i mask;} t; // [u] x 4 [v] x 4
		__m128 afix, aref;
		__m128i datm;
	};

protected:
	GSState* m_state;
	CRect m_scissor;
	DWORD* m_cache;
	DWORD m_page[(1024 / 32) * (1024 / 64)];
	DWORD m_pagehash;
	CSimpleMap<DWORD, ColumnOffset*> m_comap;
	ColumnOffset* m_fbco;
	ColumnOffset* m_zbco;
	Scanline* m_sl;
	ScanlineEnv* m_slenv;
	bool m_solidrect;

	void SetupTexture();
	void SetupColumnOffset();

	typedef void (GSRasterizer::*PrepareScanlinePtr)(int x, int y, int steps, const Vertex& v, const Vertex& dv);
	typedef void (GSRasterizer::*DrawScanlinePtr)(int top, int left, int right, Vertex& v, const Vertex& dv);
	typedef void (GSRasterizer::*DrawScanlineZPtr)(int steps);
	typedef void (GSRasterizer::*DrawScanlineTPtr)(int steps);
	typedef void (GSRasterizer::*DrawScanlineOMPtr)(int steps);

	PrepareScanlinePtr m_ps1[5][2][2][2], m_ps1f;
	PrepareScanlinePtr m_ps4[5][2][2][2], m_ps4f;
	DrawScanlinePtr m_ds[3][4][2], m_dsf;
	DrawScanlineZPtr m_dsz1[4], m_dsz1f;
	DrawScanlineZPtr m_dsz4[2][4], m_dsz4f;
	DrawScanlineTPtr m_dst[5][2][2][2], m_dstf;
	DrawScanlineOMPtr m_dsom[8][4][2][2][3], m_dsomf;

	template<int iTFX, int bFST, int bFGE, int bZRW>
	void PrepareScanline1(int x, int y, int steps, const Vertex& v, const Vertex& dv);
	template<int iTFX, int bFST, int bFGE, int bZRW>
	void PrepareScanline4(int x, int y, int steps, const Vertex& v, const Vertex& dv);
	template<int iZTST, int iZPSM, int bTAF>
	void DrawScanline(int top, int left, int right, Vertex& v, const Vertex& dv);
	template<int iTFX, bool bTCC, int bLTF, int bFST>
	void DrawScanlineT(int steps);
	template<int iFPSM, int iZPSM, int bRFB, int bDATE, int iABE>
	void DrawScanlineOM(int steps);

	void FetchTexture(int x, int y);

	__forceinline void FetchTexel(int x, int y)
	{
		// - page size depends on psm, this is just the smallest for 32 bit
		// - seems to be just enough to avoid recursive rendering errors
		// - to be fully correct we need to fetch ttbl->pgs sized pages

		int i = ((y & ~31) >> 1) + (x >> 6);

		if(m_page[i] != m_pagehash)
		{
			m_page[i] = m_pagehash;

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

	__forceinline __m128i Wrap(__m128i t)
	{
		__m128i region = _mm_or_si128(_mm_and_si128(t, m_slenv->t.and), m_slenv->t.or);
		__m128i clamp = _mm_min_epi16(_mm_max_epi16(t, m_slenv->t.min), m_slenv->t.max);
		return _mm_or_si128(_mm_and_si128(region, m_slenv->t.mask), _mm_andnot_si128(m_slenv->t.mask, clamp));
	}

	__forceinline __m128 Unpack(DWORD c)
	{
		__m128i zero = _mm_setzero_si128(); 
		__m128i r0 = _mm_cvtsi32_si128(c);
		r0 = _mm_unpacklo_epi16(_mm_unpacklo_epi8(r0, zero), zero);
		return _mm_cvtepi32_ps(r0);
	}

	__forceinline __m128 Modulate(__m128 c, __m128 f)
	{
		return _mm_mul_ps(_mm_mul_ps(c, f), _mm_set1_ps(2.0f / 256));
	}

	__forceinline __m128 Blend(__m128 c0, __m128 c1, __m128 f)
	{
		return _mm_add_ps(c0, _mm_mul_ps(_mm_sub_ps(c1, c0), f));
	}

	__forceinline __m128 Saturate(__m128 c)
	{
		return _mm_min_ps(_mm_max_ps(c, _mm_setzero_ps()), _mm_set1_ps(255));
	}

public:
	GSRasterizer(GSState* state);
	virtual ~GSRasterizer();

	void InvalidateTextureCache();

	void BeginDraw();
	void DrawPoint(Vertex* v);
	void DrawLine(Vertex* v);
	void DrawTriangle(Vertex* v);
	void DrawSprite(Vertex* v);
	bool DrawSolidRect(int left, int top, int right, int bottom, const Vertex& v);
};
