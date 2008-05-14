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
#include "GSTables.h"
#include "x86.h"

// unswizzling

void __fastcall unSwizzleBlock32_c(BYTE* src, BYTE* dst, int dstpitch)
{
	const DWORD* s = &columnTable32[0][0];

	for(int j = 0; j < 8; j++, s += 8, dst += dstpitch)
		for(int i = 0; i < 8; i++)
			((DWORD*)dst)[i] = ((DWORD*)src)[s[i]];
}

void __fastcall unSwizzleBlock16_c(BYTE* src, BYTE* dst, int dstpitch)
{
	const DWORD* s = &columnTable16[0][0];

	for(int j = 0; j < 8; j++, s += 16, dst += dstpitch)
		for(int i = 0; i < 16; i++)
			((WORD*)dst)[i] = ((WORD*)src)[s[i]];
}

void __fastcall unSwizzleBlock8_c(BYTE* src, BYTE* dst, int dstpitch)
{
	const DWORD* s = &columnTable8[0][0];

	for(int j = 0; j < 16; j++, s += 16, dst += dstpitch)
		for(int i = 0; i < 16; i++)
			dst[i] = src[s[i]];
}

void __fastcall unSwizzleBlock4_c(BYTE* src, BYTE* dst, int dstpitch)
{
	const DWORD* s = &columnTable4[0][0];

	for(int j = 0; j < 16; j++, s += 32, dst += dstpitch)
	{
		for(int i = 0; i < 32; i++)
		{
			DWORD addr = s[i];
			BYTE c = (src[addr>>1] >> ((addr&1) << 2)) & 0x0f;
			int shift = (i&1) << 2;
			dst[i >> 1] = (dst[i >> 1] & (0xf0 >> shift)) | (c << shift);
		}
	}
}

void __fastcall unSwizzleBlock8HP_c(BYTE* src, BYTE* dst, int dstpitch)
{
	const DWORD* s = &columnTable32[0][0];

	for(int j = 0; j < 8; j++, s += 8, dst += dstpitch)
		for(int i = 0; i < 8; i++)
			dst[i] = (BYTE)(((DWORD*)src)[s[i]]>>24);
}

void __fastcall unSwizzleBlock4HLP_c(BYTE* src, BYTE* dst, int dstpitch)
{
	const DWORD* s = &columnTable32[0][0];

	for(int j = 0; j < 8; j++, s += 8, dst += dstpitch)
		for(int i = 0; i < 8; i++)
			dst[i] = (BYTE)(((DWORD*)src)[s[i]]>>24)&0xf;
}

void __fastcall unSwizzleBlock4HHP_c(BYTE* src, BYTE* dst, int dstpitch)
{
	const DWORD* s = &columnTable32[0][0];

	for(int j = 0; j < 8; j++, s += 8, dst += dstpitch)
		for(int i = 0; i < 8; i++)
			dst[i] = (BYTE)(((DWORD*)src)[s[i]]>>28);
}

void __fastcall unSwizzleBlock4P_c(BYTE* src, BYTE* dst, int dstpitch)
{
	const DWORD* s = &columnTable4[0][0];

	for(int j = 0; j < 16; j++, s += 32, dst += dstpitch)
	{
		for(int i = 0; i < 32; i++)
		{
			DWORD addr = s[i];
			dst[i] = (src[addr>>1] >> ((addr&1) << 2)) & 0x0f;
		}
	}
}

// swizzling

void __fastcall SwizzleBlock32_c(BYTE* dst, BYTE* src, int srcpitch, DWORD WriteMask)
{
	const DWORD* d = &columnTable32[0][0];

	if(WriteMask == 0xffffffff)
	{
		for(int j = 0; j < 8; j++, d += 8, src += srcpitch)
			for(int i = 0; i < 8; i++)
				((DWORD*)dst)[d[i]] = ((DWORD*)src)[i];
	}
	else
	{
		for(int j = 0; j < 8; j++, d += 8, src += srcpitch)
			for(int i = 0; i < 8; i++)
				((DWORD*)dst)[d[i]] = (((DWORD*)dst)[d[i]] & ~WriteMask) | (((DWORD*)src)[i] & WriteMask);
	}
}

void __fastcall SwizzleBlock16_c(BYTE* dst, BYTE* src, int srcpitch)
{
	const DWORD* d = &columnTable16[0][0];

	for(int j = 0; j < 8; j++, d += 16, src += srcpitch)
		for(int i = 0; i < 16; i++)
			((WORD*)dst)[d[i]] = ((WORD*)src)[i];
}

void __fastcall SwizzleBlock8_c(BYTE* dst, BYTE* src, int srcpitch)
{
	const DWORD* d = &columnTable8[0][0];

	for(int j = 0; j < 16; j++, d += 16, src += srcpitch)
		for(int i = 0; i < 16; i++)
			dst[d[i]] = src[i];
}

void __fastcall SwizzleBlock4_c(BYTE* dst, BYTE* src, int srcpitch)
{
	const DWORD* d = &columnTable4[0][0];

	for(int j = 0; j < 16; j++, d += 32, src += srcpitch)
	{
		for(int i = 0; i < 32; i++)
		{
			DWORD addr = d[i];
			BYTE c = (src[i>>1] >> ((i&1) << 2)) & 0x0f;
			DWORD shift = (addr&1) << 2;
			dst[addr >> 1] = (dst[addr >> 1] & (0xf0 >> shift)) | (c << shift);
		}
	}
}

// column swizzling (TODO: sse2)

void __fastcall SwizzleColumn32_c(int y, BYTE* dst, BYTE* src, int srcpitch, DWORD WriteMask)
{
	const DWORD* d = &columnTable32[((y/2)&3)*2][0];

	if(WriteMask == 0xffffffff)
	{
		for(int j = 0; j < 2; j++, d += 8, src += srcpitch)
			for(int i = 0; i < 8; i++)
				((DWORD*)dst)[d[i]] = ((DWORD*)src)[i];
	}
	else
	{
		for(int j = 0; j < 2; j++, d += 8, src += srcpitch)
			for(int i = 0; i < 8; i++)
				((DWORD*)dst)[d[i]] = (((DWORD*)dst)[d[i]] & ~WriteMask) | (((DWORD*)src)[i] & WriteMask);
	}
}

void __fastcall SwizzleColumn16_c(int y, BYTE* dst, BYTE* src, int srcpitch)
{
	const DWORD* d = &columnTable16[((y/2)&3)*2][0];

	for(int j = 0; j < 2; j++, d += 16, src += srcpitch)
		for(int i = 0; i < 16; i++)
			((WORD*)dst)[d[i]] = ((WORD*)src)[i];
}

void __fastcall SwizzleColumn8_c(int y, BYTE* dst, BYTE* src, int srcpitch)
{
	const DWORD* d = &columnTable8[((y/4)&3)*4][0];

	for(int j = 0; j < 4; j++, d += 16, src += srcpitch)
		for(int i = 0; i < 16; i++)
			dst[d[i]] = src[i];
}

void __fastcall SwizzleColumn4_c(int y, BYTE* dst, BYTE* src, int srcpitch)
{
	const DWORD* d = &columnTable4[y&(3<<2)][0]; // ((y/4)&3)*4

	for(int j = 0; j < 4; j++, d += 32, src += srcpitch)
	{
		for(int i = 0; i < 32; i++)
		{
			DWORD addr = d[i];
			BYTE c = (src[i>>1] >> ((i&1) << 2)) & 0x0f;
			DWORD shift = (addr&1) << 2;
			dst[addr >> 1] = (dst[addr >> 1] & (0xf0 >> shift)) | (c << shift);
		}
	}
}

//

static __m128i s_zero = _mm_setzero_si128();
static __m128i s_bgrm = _mm_set1_epi32(0x00ffffff);
static __m128i s_am = _mm_set1_epi32(0x00008000);
static __m128i s_bm = _mm_set1_epi32(0x00007c00);
static __m128i s_gm = _mm_set1_epi32(0x000003e0);
static __m128i s_rm = _mm_set1_epi32(0x0000001f);

void __fastcall ExpandBlock24_sse2(BYTE* src, int srcpitch, DWORD* dst)
{
	__m128i* d = (__m128i*)dst;

	const __m128i mask = _mm_set1_epi32(0x00ffffff);

	for(int i = 0; i < 8; i += 2, src += srcpitch * 2, d += 4)
	{
		__m128i r0 = _mm_loadu_si128((__m128i*)(src));
		__m128i r1 = _mm_or_si128(_mm_loadl_epi64((__m128i*)(src + 16)), _mm_slli_si128(_mm_loadl_epi64((__m128i*)(src + srcpitch)), 8));
		__m128i r2 = _mm_loadu_si128((__m128i*)(src + srcpitch + 8));

		__m128i r3 = _mm_unpacklo_epi32(r0, _mm_srli_si128(r0, 3));
		__m128i r4 = _mm_unpacklo_epi32(_mm_srli_si128(r0, 6), _mm_srli_si128(r0, 9));
		__m128i r5 = _mm_and_si128(_mm_unpacklo_epi64(r3, r4), mask);

		d[0] = r5;

		r0 = _mm_or_si128(_mm_srli_si128(r0, 12), _mm_slli_si128(r1, 4));

		r3 = _mm_unpacklo_epi32(r0, _mm_srli_si128(r0, 3));
		r4 = _mm_unpacklo_epi32(_mm_srli_si128(r0, 6), _mm_srli_si128(r0, 9));
		r5 = _mm_and_si128(_mm_unpacklo_epi64(r3, r4), mask);

		d[1] = r5;

		r0 = _mm_or_si128(_mm_srli_si128(r1, 8), _mm_slli_si128(r2, 8));

		r3 = _mm_unpacklo_epi32(r0, _mm_srli_si128(r0, 3));
		r4 = _mm_unpacklo_epi32(_mm_srli_si128(r0, 6), _mm_srli_si128(r0, 9));
		r5 = _mm_and_si128(_mm_unpacklo_epi64(r3, r4), mask);

		d[2] = r5;

		r0 = _mm_srli_si128(r2, 4);

		r3 = _mm_unpacklo_epi32(r0, _mm_srli_si128(r0, 3));
		r4 = _mm_unpacklo_epi32(_mm_srli_si128(r0, 6), _mm_srli_si128(r0, 9));
		r5 = _mm_and_si128(_mm_unpacklo_epi64(r3, r4), mask);

		d[3] = r5;
	}
}

void __fastcall ExpandBlock24_sse2(DWORD* src, DWORD* dst, int dstpitch, GIFRegTEXA* pTEXA)
{
	__m128i TA0 = _mm_set1_epi32((DWORD)pTEXA->TA0 << 24);

	if(!pTEXA->AEM)
	{
		for(int j = 0; j < 8; j++, src += 8, dst += dstpitch>>2)
		{
			for(int i = 0; i < 8; i += 4)
			{
				__m128i c = _mm_load_si128((__m128i*)&src[i]);
				c = _mm_and_si128(c, s_bgrm);
				c = _mm_or_si128(c, TA0);
				_mm_store_si128((__m128i*)&dst[i], c);
			}
		}
	}
	else
	{
		for(int j = 0; j < 8; j++, src += 8, dst += dstpitch>>2)
		{
			for(int i = 0; i < 8; i += 4)
			{
				__m128i c = _mm_load_si128((__m128i*)&src[i]);
				c = _mm_and_si128(c, s_bgrm);
				__m128i a = _mm_andnot_si128(_mm_cmpeq_epi32(c, s_zero), TA0);
				c = _mm_or_si128(c, a);
				_mm_store_si128((__m128i*)&dst[i], c);
			}
		}
	}
}

void __fastcall ExpandBlock16_sse2(WORD* src, DWORD* dst, int dstpitch, GIFRegTEXA* pTEXA)
{
	__m128i TA0 = _mm_set1_epi32((DWORD)pTEXA->TA0 << 24);
	__m128i TA1 = _mm_set1_epi32((DWORD)pTEXA->TA1 << 24);
	__m128i a, b, g, r;

	if(!pTEXA->AEM)
	{
		for(int j = 0; j < 8; j++, src += 16, dst += dstpitch>>2)
		{
			for(int i = 0; i < 16; i += 8)
			{
				__m128i c = _mm_load_si128((__m128i*)&src[i]);

				__m128i cl = _mm_unpacklo_epi16(c, s_zero);
				__m128i ch = _mm_unpackhi_epi16(c, s_zero);

				__m128i alm = _mm_cmplt_epi32(cl, s_am);
				__m128i ahm = _mm_cmplt_epi32(ch, s_am);

				// lo

				b = _mm_slli_epi32(_mm_and_si128(cl, s_bm), 9);
				g = _mm_slli_epi32(_mm_and_si128(cl, s_gm), 6);
				r = _mm_slli_epi32(_mm_and_si128(cl, s_rm), 3);
				a = _mm_or_si128(_mm_and_si128(alm, TA0), _mm_andnot_si128(alm, TA1));

				cl = _mm_or_si128(_mm_or_si128(a, b), _mm_or_si128(g, r));

				_mm_store_si128((__m128i*)&dst[i], cl);

				// hi

				b = _mm_slli_epi32(_mm_and_si128(ch, s_bm), 9);
				g = _mm_slli_epi32(_mm_and_si128(ch, s_gm), 6);
				r = _mm_slli_epi32(_mm_and_si128(ch, s_rm), 3);
				a = _mm_or_si128(_mm_and_si128(ahm, TA0), _mm_andnot_si128(ahm, TA1));

				ch = _mm_or_si128(_mm_or_si128(a, b), _mm_or_si128(g, r));

				_mm_store_si128((__m128i*)&dst[i+4], ch);
			}
		}
	}
	else
	{
		for(int j = 0; j < 8; j++, src += 16, dst += dstpitch>>2)
		{
			for(int i = 0; i < 16; i += 8)
			{
				__m128i c = _mm_load_si128((__m128i*)&src[i]);

				__m128i cl = _mm_unpacklo_epi16(c, s_zero);
				__m128i ch = _mm_unpackhi_epi16(c, s_zero);

				__m128i alm = _mm_cmplt_epi32(cl, s_am);
				__m128i ahm = _mm_cmplt_epi32(ch, s_am);

				__m128i trm = _mm_cmpeq_epi16(c, s_zero);
				__m128i trlm = _mm_unpacklo_epi16(trm, trm);
				__m128i trhm = _mm_unpackhi_epi16(trm, trm);

				// lo

				b = _mm_slli_epi32(_mm_and_si128(cl, s_bm), 9);
				g = _mm_slli_epi32(_mm_and_si128(cl, s_gm), 6);
				r = _mm_slli_epi32(_mm_and_si128(cl, s_rm), 3);
				a = _mm_or_si128(_mm_and_si128(alm, TA0), _mm_andnot_si128(alm, TA1));
				a = _mm_andnot_si128(trlm, a);

				cl = _mm_or_si128(_mm_or_si128(a, b), _mm_or_si128(g, r));

				_mm_store_si128((__m128i*)&dst[i], cl);

				// hi

				b = _mm_slli_epi32(_mm_and_si128(ch, s_bm), 9);
				g = _mm_slli_epi32(_mm_and_si128(ch, s_gm), 6);
				r = _mm_slli_epi32(_mm_and_si128(ch, s_rm), 3);
				a = _mm_or_si128(_mm_and_si128(ahm, TA0), _mm_andnot_si128(ahm, TA1));
				a = _mm_andnot_si128(trhm, a);

				ch = _mm_or_si128(_mm_or_si128(a, b), _mm_or_si128(g, r));

				_mm_store_si128((__m128i*)&dst[i+4], ch);
			}
		}
	}
}

void __fastcall Expand16_sse2(WORD* src, DWORD* dst, int w, GIFRegTEXA* pTEXA)
{
	ASSERT(!(w&7));

	__m128i TA0 = _mm_set1_epi32((DWORD)pTEXA->TA0 << 24);
	__m128i TA1 = _mm_set1_epi32((DWORD)pTEXA->TA1 << 24);
	__m128i a, b, g, r;

	if(!pTEXA->AEM)
	{
		for(int i = 0; i < w; i += 8)
		{
			__m128i c = _mm_load_si128((__m128i*)&src[i]);

			__m128i cl = _mm_unpacklo_epi16(c, s_zero);
			__m128i ch = _mm_unpackhi_epi16(c, s_zero);

			__m128i alm = _mm_cmplt_epi32(cl, s_am);
			__m128i ahm = _mm_cmplt_epi32(ch, s_am);

			// lo

			b = _mm_slli_epi32(_mm_and_si128(cl, s_bm), 9);
			g = _mm_slli_epi32(_mm_and_si128(cl, s_gm), 6);
			r = _mm_slli_epi32(_mm_and_si128(cl, s_rm), 3);
			a = _mm_or_si128(_mm_and_si128(alm, TA0), _mm_andnot_si128(alm, TA1));

			cl = _mm_or_si128(_mm_or_si128(a, b), _mm_or_si128(g, r));

			_mm_store_si128((__m128i*)&dst[i], cl);

			// hi

			b = _mm_slli_epi32(_mm_and_si128(ch, s_bm), 9);
			g = _mm_slli_epi32(_mm_and_si128(ch, s_gm), 6);
			r = _mm_slli_epi32(_mm_and_si128(ch, s_rm), 3);
			a = _mm_or_si128(_mm_and_si128(ahm, TA0), _mm_andnot_si128(ahm, TA1));

			ch = _mm_or_si128(_mm_or_si128(a, b), _mm_or_si128(g, r));

			_mm_store_si128((__m128i*)&dst[i+4], ch);
		}
	}
	else
	{
		for(int i = 0; i < w; i += 8)
		{
			__m128i c = _mm_load_si128((__m128i*)&src[i]);

			__m128i cl = _mm_unpacklo_epi16(c, s_zero);
			__m128i ch = _mm_unpackhi_epi16(c, s_zero);

			__m128i alm = _mm_cmplt_epi32(cl, s_am);
			__m128i ahm = _mm_cmplt_epi32(ch, s_am);

			__m128i trm = _mm_cmpeq_epi16(c, s_zero);
			__m128i trlm = _mm_unpacklo_epi16(trm, trm);
			__m128i trhm = _mm_unpackhi_epi16(trm, trm);

			// lo

			b = _mm_slli_epi32(_mm_and_si128(cl, s_bm), 9);
			g = _mm_slli_epi32(_mm_and_si128(cl, s_gm), 6);
			r = _mm_slli_epi32(_mm_and_si128(cl, s_rm), 3);
			a = _mm_or_si128(_mm_and_si128(alm, TA0), _mm_andnot_si128(alm, TA1));
			a = _mm_andnot_si128(trlm, a);

			cl = _mm_or_si128(_mm_or_si128(a, b), _mm_or_si128(g, r));

			_mm_store_si128((__m128i*)&dst[i], cl);

			// hi

			b = _mm_slli_epi32(_mm_and_si128(ch, s_bm), 9);
			g = _mm_slli_epi32(_mm_and_si128(ch, s_gm), 6);
			r = _mm_slli_epi32(_mm_and_si128(ch, s_rm), 3);
			a = _mm_or_si128(_mm_and_si128(ahm, TA0), _mm_andnot_si128(ahm, TA1));
			a = _mm_andnot_si128(trhm, a);

			ch = _mm_or_si128(_mm_or_si128(a, b), _mm_or_si128(g, r));

			_mm_store_si128((__m128i*)&dst[i+4], ch);
		}
	}
}

void __fastcall ExpandBlock24_c(BYTE* src, int srcpitch, DWORD* dst)
{
	for(int j = 0, diff = srcpitch - 8*3; j < 8; j++, src += diff, dst += 8)
		for(int i = 0; i < 8; i++, src += 3)
			dst[i] = (src[2] << 16) | (src[1] << 8) | src[0];
}

void __fastcall ExpandBlock24_c(DWORD* src, DWORD* dst, int dstpitch, GIFRegTEXA* pTEXA)
{
	DWORD TA0 = (DWORD)pTEXA->TA0 << 24;

	if(!pTEXA->AEM)
	{
		for(int j = 0; j < 8; j++, src += 8, dst += dstpitch>>2)
			for(int i = 0; i < 8; i++)
				dst[i] = TA0 | (src[i]&0xffffff);
	}
	else
	{
		for(int j = 0; j < 8; j++, src += 8, dst += dstpitch>>2)
			for(int i = 0; i < 8; i++)
				dst[i] = ((src[i]&0xffffff) ? TA0 : 0) | (src[i]&0xffffff);
	}
}

void __fastcall ExpandBlock16_c(WORD* src, DWORD* dst, int dstpitch, GIFRegTEXA* pTEXA)
{
	DWORD TA0 = (DWORD)pTEXA->TA0 << 24;
	DWORD TA1 = (DWORD)pTEXA->TA1 << 24;

	if(!pTEXA->AEM)
	{
		for(int j = 0; j < 8; j++, src += 16, dst += dstpitch>>2)
			for(int i = 0; i < 16; i++)
				dst[i] = ((src[i]&0x8000) ? TA1 : TA0)
					| ((src[i]&0x7c00) << 9) | ((src[i]&0x03e0) << 6) | ((src[i]&0x001f) << 3);
	}
	else
	{
		for(int j = 0; j < 8; j++, src += 16, dst += dstpitch>>2)
			for(int i = 0; i < 16; i++)
				dst[i] = ((src[i]&0x8000) ? TA1 : src[i] ? TA0 : 0)
					| ((src[i]&0x7c00) << 9) | ((src[i]&0x03e0) << 6) | ((src[i]&0x001f) << 3);
	}
}

void __fastcall Expand16_c(WORD* src, DWORD* dst, int w, GIFRegTEXA* pTEXA)
{
	DWORD TA0 = (DWORD)pTEXA->TA0 << 24;
	DWORD TA1 = (DWORD)pTEXA->TA1 << 24;

	if(!pTEXA->AEM)
	{
		for(int i = 0; i < w; i++)
			dst[i] = ((src[i]&0x8000) ? TA1 : TA0)
				| ((src[i]&0x7c00) << 9) | ((src[i]&0x03e0) << 6) | ((src[i]&0x001f) << 3);
	}
	else
	{
		for(int i = 0; i < w; i++)
			dst[i] = ((src[i]&0x8000) ? TA1 : src[i] ? TA0 : 0)
				| ((src[i]&0x7c00) << 9) | ((src[i]&0x03e0) << 6) | ((src[i]&0x001f) << 3);
	}
}

//

static __m128i s_clut[64];

void __fastcall WriteCLUT_T32_I8_CSM1_sse2(DWORD* vm, WORD* clut)
{
	__m128i* src = (__m128i*)vm;
	__m128i* dst = s_clut;

	for(int j = 0; j < 64; j += 32, src += 32, dst += 32)
	{
		for(int i = 0; i < 16; i += 4)
		{
			__m128i r0 = _mm_load_si128(&src[i+0]);
			__m128i r1 = _mm_load_si128(&src[i+1]);
			__m128i r2 = _mm_load_si128(&src[i+2]);
			__m128i r3 = _mm_load_si128(&src[i+3]);

			_mm_store_si128(&dst[i*2+0], _mm_unpacklo_epi64(r0, r1));
			_mm_store_si128(&dst[i*2+1], _mm_unpacklo_epi64(r2, r3));
			_mm_store_si128(&dst[i*2+2], _mm_unpackhi_epi64(r0, r1));
			_mm_store_si128(&dst[i*2+3], _mm_unpackhi_epi64(r2, r3));

			__m128i r4 = _mm_load_si128(&src[i+0+16]);
			__m128i r5 = _mm_load_si128(&src[i+1+16]);
			__m128i r6 = _mm_load_si128(&src[i+2+16]);
			__m128i r7 = _mm_load_si128(&src[i+3+16]);

			_mm_store_si128(&dst[i*2+4], _mm_unpacklo_epi64(r4, r5));
			_mm_store_si128(&dst[i*2+5], _mm_unpacklo_epi64(r6, r7));
			_mm_store_si128(&dst[i*2+6], _mm_unpackhi_epi64(r4, r5));
			_mm_store_si128(&dst[i*2+7], _mm_unpackhi_epi64(r6, r7));
		}
	}

	for(int i = 0; i < 32; i++)
	{
		__m128i r0 = s_clut[i*2];
		__m128i r1 = s_clut[i*2+1];
		__m128i r2 = _mm_unpacklo_epi16(r0, r1);
		__m128i r3 = _mm_unpackhi_epi16(r0, r1);
		r0 = _mm_unpacklo_epi16(r2, r3);
		r1 = _mm_unpackhi_epi16(r2, r3);
		r2 = _mm_unpacklo_epi16(r0, r1);
		r3 = _mm_unpackhi_epi16(r0, r1);
		_mm_store_si128(&((__m128i*)clut)[i], r2);
		_mm_store_si128(&((__m128i*)clut)[i+32], r3);
	}
}

void __fastcall WriteCLUT_T32_I4_CSM1_sse2(DWORD* vm, WORD* clut)
{
	__m128i* src = (__m128i*)vm;
	__m128i* dst = s_clut;

	__m128i r0 = _mm_load_si128(&src[0]);
	__m128i r1 = _mm_load_si128(&src[1]);
	__m128i r2 = _mm_load_si128(&src[2]);
	__m128i r3 = _mm_load_si128(&src[3]);

	_mm_store_si128(&dst[0], _mm_unpacklo_epi64(r0, r1));
	_mm_store_si128(&dst[1], _mm_unpacklo_epi64(r2, r3));
	_mm_store_si128(&dst[2], _mm_unpackhi_epi64(r0, r1));
	_mm_store_si128(&dst[3], _mm_unpackhi_epi64(r2, r3));

	for(int i = 0; i < 2; i++)
	{
		__m128i r0 = s_clut[i*2];
		__m128i r1 = s_clut[i*2+1];
		__m128i r2 = _mm_unpacklo_epi16(r0, r1);
		__m128i r3 = _mm_unpackhi_epi16(r0, r1);
		r0 = _mm_unpacklo_epi16(r2, r3);
		r1 = _mm_unpackhi_epi16(r2, r3);
		r2 = _mm_unpacklo_epi16(r0, r1);
		r3 = _mm_unpackhi_epi16(r0, r1);
		_mm_store_si128(&((__m128i*)clut)[i], r2);
		_mm_store_si128(&((__m128i*)clut)[i+32], r3);
	}
}

void __fastcall WriteCLUT_T16_I8_CSM1_sse2(WORD* vm, WORD* clut)
{
	__m128i* src = (__m128i*)vm;
	__m128i* dst = (__m128i*)clut;

	for(int i = 0; i < 32; i += 4)
	{
		__m128i r0 = _mm_load_si128(&src[i+0]);
		__m128i r1 = _mm_load_si128(&src[i+1]);
		__m128i r2 = _mm_load_si128(&src[i+2]);
		__m128i r3 = _mm_load_si128(&src[i+3]);

		__m128i r4 = _mm_unpacklo_epi16(r0, r1);
		__m128i r5 = _mm_unpackhi_epi16(r0, r1);
		__m128i r6 = _mm_unpacklo_epi16(r2, r3);
		__m128i r7 = _mm_unpackhi_epi16(r2, r3);

		r0 = _mm_unpacklo_epi32(r4, r6);
		r1 = _mm_unpackhi_epi32(r4, r6);
		r2 = _mm_unpacklo_epi32(r5, r7);
		r3 = _mm_unpackhi_epi32(r5, r7);

		r4 = _mm_unpacklo_epi16(r0, r1);
		r5 = _mm_unpackhi_epi16(r0, r1);
		r6 = _mm_unpacklo_epi16(r2, r3);
		r7 = _mm_unpackhi_epi16(r2, r3);

		_mm_store_si128(&dst[i+0], r4);
		_mm_store_si128(&dst[i+1], r6);
		_mm_store_si128(&dst[i+2], r5);
		_mm_store_si128(&dst[i+3], r7);
	}
}

void __fastcall WriteCLUT_T16_I4_CSM1_sse2(WORD* vm, WORD* clut)
{
	// TODO (probably not worth, _c is going to be just as fast)
	WriteCLUT_T16_I4_CSM1_c(vm, clut);
}

void __fastcall WriteCLUT_T32_I8_CSM1_c(DWORD* vm, WORD* clut)
{
	for(int j = 0; j < 2; j++, vm += 128, clut += 128)
	{
		for(int i = 0; i < 128; i++) 
		{
			DWORD dw = vm[clutTableT32I8[i]];
			clut[i] = (WORD)(dw & 0xffff);
			clut[i + 256] = (WORD)(dw >> 16);
		}
	}
}

void __fastcall WriteCLUT_T32_I4_CSM1_c(DWORD* vm, WORD* clut)
{
	for(int i = 0; i < 16; i++) 
	{
		DWORD dw = vm[clutTableT32I4[i]];
		clut[i] = (WORD)(dw & 0xffff);
		clut[i + 256] = (WORD)(dw >> 16);
	}
}

void __fastcall WriteCLUT_T16_I8_CSM1_c(WORD* vm, WORD* clut)
{
	for(int j = 0; j < 8; j++, vm += 32, clut += 32) 
	{
		for(int i = 0; i < 32; i++)
		{
			clut[i] = vm[clutTableT16I8[i]];
		}
	}
}

void __fastcall WriteCLUT_T16_I4_CSM1_c(WORD* vm, WORD* clut)
{
	for(int i = 0; i < 16; i++) 
	{
		clut[i] = vm[clutTableT16I4[i]];
	}
}

//

void __fastcall ReadCLUT32_T32_I8_sse2(WORD* src, DWORD* dst)
{
	for(int i = 0; i < 256; i += 16)
	{
		ReadCLUT32_T32_I4_sse2(&src[i], &dst[i]); // going to be inlined nicely
	}
}

void __fastcall ReadCLUT32_T32_I4_sse2(WORD* src, DWORD* dst)
{
	__m128i r0 = ((__m128i*)src)[0];
	__m128i r1 = ((__m128i*)src)[1];
	__m128i r2 = ((__m128i*)src)[0+32];
	__m128i r3 = ((__m128i*)src)[1+32];
	_mm_store_si128(&((__m128i*)dst)[0], _mm_unpacklo_epi16(r0, r2));
	_mm_store_si128(&((__m128i*)dst)[1], _mm_unpackhi_epi16(r0, r2));
	_mm_store_si128(&((__m128i*)dst)[2], _mm_unpacklo_epi16(r1, r3));
	_mm_store_si128(&((__m128i*)dst)[3], _mm_unpackhi_epi16(r1, r3));
}

void __fastcall ReadCLUT32_T16_I8_sse2(WORD* src, DWORD* dst)
{
	for(int i = 0; i < 256; i += 16)
	{
		ReadCLUT32_T16_I4_sse2(&src[i], &dst[i]);
	}
}

void __fastcall ReadCLUT32_T16_I4_sse2(WORD* src, DWORD* dst)
{
	__m128i r0 = ((__m128i*)src)[0];
	__m128i r1 = ((__m128i*)src)[1];
	_mm_store_si128(&((__m128i*)dst)[0], _mm_unpacklo_epi16(r0, s_zero));
	_mm_store_si128(&((__m128i*)dst)[1], _mm_unpackhi_epi16(r0, s_zero));
	_mm_store_si128(&((__m128i*)dst)[2], _mm_unpacklo_epi16(r1, s_zero));
	_mm_store_si128(&((__m128i*)dst)[3], _mm_unpackhi_epi16(r1, s_zero));
}

void __fastcall ReadCLUT32_T32_I8_c(WORD* src, DWORD* dst)
{
	for(int i = 0; i < 256; i++)
	{
		dst[i] = ((DWORD)src[i+256] << 16) | src[i];
	}
}

void __fastcall ReadCLUT32_T32_I4_c(WORD* src, DWORD* dst)
{
	for(int i = 0; i < 16; i++)
	{
		dst[i] = ((DWORD)src[i+256] << 16) | src[i];
	}
}

void __fastcall ReadCLUT32_T16_I8_c(WORD* src, DWORD* dst)
{
	for(int i = 0; i < 256; i++)
	{
		dst[i] = (DWORD)src[i];
	}
}

void __fastcall ReadCLUT32_T16_I4_c(WORD* src, DWORD* dst)
{
	for(int i = 0; i < 16; i++)
	{
		dst[i] = (DWORD)src[i];
	}
}

//