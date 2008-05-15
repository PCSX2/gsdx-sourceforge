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
#include "GSVector.h"
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

static const GSVector4i s_rgbm(0x00ffffff);
static const GSVector4i s_am(0x00008000);
static const GSVector4i s_bm(0x00007c00);
static const GSVector4i s_gm(0x000003e0);
static const GSVector4i s_rm(0x0000001f);

// TODO: ssse3 version with pshufb and palignr

void __fastcall ExpandBlock24_sse2(BYTE* src, int srcpitch, DWORD* dst)
{
	const GSVector4i rgbm = s_rgbm;

	GSVector4i* d = (GSVector4i*)dst;

	for(int i = 0; i < 8; i += 2, src += srcpitch * 2, d += 4)
	{
		GSVector4i r0 = GSVector4i::loadu(src);
		GSVector4i r1 = GSVector4i::loadu(src + 16, src + srcpitch);
		GSVector4i r2 = GSVector4i::loadu(src + srcpitch + 8);

		d[0] = r0.upl32(r0.srl<3>()).upl64(r0.srl<6>().upl32(r0.srl<9>())) & rgbm;

		r0 = r0.srl<12>() | r1.sll<4>();

		d[1] = r0.upl32(r0.srl<3>()).upl64(r0.srl<6>().upl32(r0.srl<9>())) & rgbm;

		r0 = r1.srl<8>() | r2.sll<8>();

		d[2] = r0.upl32(r0.srl<3>()).upl64(r0.srl<6>().upl32(r0.srl<9>())) & rgbm;

		r0 = r2.srl<4>();

		d[3] = r0.upl32(r0.srl<3>()).upl64(r0.srl<6>().upl32(r0.srl<9>())) & rgbm;
	}
}

void __fastcall ExpandBlock24_sse2(DWORD* src, DWORD* dst, int dstpitch, GIFRegTEXA* pTEXA)
{
	const GSVector4i rgbm = s_rgbm;

	GSVector4i TA0(pTEXA->TA0 << 24);

	GSVector4i c;

	if(!pTEXA->AEM)
	{
		for(int j = 0; j < 8; j++, src += 8, dst += dstpitch >> 2)
		{
			GSVector4i* s = (GSVector4i*)src;
			GSVector4i* d = (GSVector4i*)dst;

			d[0] = (s[0] & rgbm) | TA0;
			d[1] = (s[1] & rgbm) | TA0;
		}
	}
	else
	{
		for(int j = 0; j < 8; j++, src += 8, dst += dstpitch >> 2)
		{
			GSVector4i* s = (GSVector4i*)src;
			GSVector4i* d = (GSVector4i*)dst;

			c = s[0] & rgbm;
			d[0] = c | TA0.andnot(c == GSVector4i::zero()); // TA0 & (c != GSVector4i::zero())

			c = s[1] & rgbm;
			d[1] = c | TA0.andnot(c == GSVector4i::zero()); // TA0 & (c != GSVector4i::zero())
		}
	}
}

void __fastcall ExpandBlock16_sse2(WORD* src, DWORD* dst, int dstpitch, GIFRegTEXA* pTEXA)
{
	const GSVector4i rm = s_rm;
	const GSVector4i gm = s_gm;
	const GSVector4i bm = s_bm;
	const GSVector4i am = s_am;

	GSVector4i TA0(pTEXA->TA0 << 24);
	GSVector4i TA1(pTEXA->TA1 << 24);

	GSVector4i c, cl, ch;

	if(!pTEXA->AEM)
	{
		for(int j = 0; j < 8; j++, src += 16, dst += dstpitch >> 2)
		{
			GSVector4i* s = (GSVector4i*)src;
			GSVector4i* d = (GSVector4i*)dst;

			c = s[0];
			cl = c.upl16();
			ch = c.uph16();
			d[0] = ((cl & rm) << 3) | ((cl & gm) << 6) | ((cl & bm) << 9) | TA1.blend(TA0, cl < am);
			d[1] = ((ch & rm) << 3) | ((ch & gm) << 6) | ((ch & bm) << 9) | TA1.blend(TA0, ch < am);

			c = s[1];
			cl = c.upl16();
			ch = c.uph16();
			d[2] = ((cl & rm) << 3) | ((cl & gm) << 6) | ((cl & bm) << 9) | TA1.blend(TA0, cl < am);
			d[3] = ((ch & rm) << 3) | ((ch & gm) << 6) | ((ch & bm) << 9) | TA1.blend(TA0, ch < am);
		}
	}
	else
	{
		for(int j = 0; j < 8; j++, src += 16, dst += dstpitch >> 2)
		{
			GSVector4i* s = (GSVector4i*)src;
			GSVector4i* d = (GSVector4i*)dst;

			c = s[0];
			cl = c.upl16();
			ch = c.uph16();
			d[0] = ((cl & rm) << 3) | ((cl & gm) << 6) | ((cl & bm) << 9) | TA1.blend(TA0, cl < am).andnot(cl == GSVector4i::zero());
			d[1] = ((ch & rm) << 3) | ((ch & gm) << 6) | ((ch & bm) << 9) | TA1.blend(TA0, ch < am).andnot(ch == GSVector4i::zero());

			c = s[1];
			cl = c.upl16();
			ch = c.uph16();
			d[2] = ((cl & rm) << 3) | ((cl & gm) << 6) | ((cl & bm) << 9) | TA1.blend(TA0, cl < am).andnot(cl == GSVector4i::zero());
			d[3] = ((ch & rm) << 3) | ((ch & gm) << 6) | ((ch & bm) << 9) | TA1.blend(TA0, ch < am).andnot(ch == GSVector4i::zero());
		}
	}
}

void __fastcall Expand16_sse2(WORD* src, DWORD* dst, int w, GIFRegTEXA* pTEXA)
{
	ASSERT(!(w&7));

	const GSVector4i rm = s_rm;
	const GSVector4i gm = s_gm;
	const GSVector4i bm = s_bm;
	const GSVector4i am = s_am;

	GSVector4i TA0(pTEXA->TA0 << 24);
	GSVector4i TA1(pTEXA->TA1 << 24);

	GSVector4i c, cl, ch;

	GSVector4i* s = (GSVector4i*)src;
	GSVector4i* d = (GSVector4i*)dst;

	if(!pTEXA->AEM)
	{
		for(int i = 0, j = w >> 3; i < j; i++)
		{
			c = s[i];
			cl = c.upl16();
			ch = c.uph16();
			d[i*2+0] = ((cl & rm) << 3) | ((cl & gm) << 6) | ((cl & bm) << 9) | TA1.blend(TA0, cl < am);
			d[i*2+1] = ((ch & rm) << 3) | ((ch & gm) << 6) | ((ch & bm) << 9) | TA1.blend(TA0, ch < am);
		}
	}
	else
	{
		for(int i = 0, j = w >> 3; i < j; i++)
		{
			c = s[i];
			cl = c.upl16();
			ch = c.uph16();
			d[i*2+0] = ((cl & rm) << 3) | ((cl & gm) << 6) | ((cl & bm) << 9) | TA1.blend(TA0, cl < am).andnot(cl == GSVector4i::zero());
			d[i*2+1] = ((ch & rm) << 3) | ((ch & gm) << 6) | ((ch & bm) << 9) | TA1.blend(TA0, ch < am).andnot(ch == GSVector4i::zero());
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

void __fastcall WriteCLUT_T32_I8_CSM1_sse2(DWORD* vm, WORD* clut)
{
	GSVector4i tmp[64];

	GSVector4i* s = (GSVector4i*)vm;
	GSVector4i* d = tmp;

	for(int j = 0; j < 2; j++, s += 32, d += 32)
	{
		for(int i = 0; i < 16; i += 4)
		{
			d[i*2+0] = s[i+0].upl64(s[i+1]);
			d[i*2+1] = s[i+2].upl64(s[i+3]);
			d[i*2+2] = s[i+0].uph64(s[i+1]);
			d[i*2+3] = s[i+2].uph64(s[i+3]);
			d[i*2+4] = s[i+0+16].upl64(s[i+1+16]);
			d[i*2+5] = s[i+2+16].upl64(s[i+3+16]);
			d[i*2+6] = s[i+0+16].uph64(s[i+1+16]);
			d[i*2+7] = s[i+2+16].uph64(s[i+3+16]);
		}
	}

	s = tmp;
	d = (GSVector4i*)clut;

	for(int i = 0; i < 32; i++)
	{
		GSVector4i r0, r1, r2, r3;

		r0 = s[i*2+0];
		r1 = s[i*2+1];
		r2 = r0.upl16(r1);
		r3 = r0.uph16(r1);
		r0 = r2.upl16(r3);
		r1 = r2.uph16(r3);
		d[i+0] = r0.upl16(r1);
		d[i+32] = r0.uph16(r1);
	}
}

void __fastcall WriteCLUT_T32_I4_CSM1_sse2(DWORD* vm, WORD* clut)
{
	GSVector4i* s = (GSVector4i*)vm;
	GSVector4i* d = (GSVector4i*)clut;

	GSVector4i r0, r1, r2, r3;
	GSVector4i r4, r5, r6, r7;

	r0 = s[0].upl64(s[1]);
	r1 = s[2].upl64(s[3]);
	r4 = s[0].uph64(s[1]);
	r5 = s[2].uph64(s[3]);
	r2 = r0.upl16(r1);
	r3 = r0.uph16(r1);
	r6 = r4.upl16(r5);
	r7 = r4.uph16(r5);
	r0 = r2.upl16(r3);
	r1 = r2.uph16(r3);
	r4 = r6.upl16(r7);
	r5 = r6.uph16(r7);
	d[0] = r0.upl16(r1);
	d[1] = r4.upl16(r5);
	d[32] = r0.uph16(r1);
	d[33] = r4.uph16(r5);
}

void __fastcall WriteCLUT_T16_I8_CSM1_sse2(WORD* vm, WORD* clut)
{
	GSVector4i* s = (GSVector4i*)vm;
	GSVector4i* d = (GSVector4i*)clut;

	for(int i = 0; i < 32; i += 4)
	{
		GSVector4i r0, r1, r2, r3;
		GSVector4i r4, r5, r6, r7;

		r0 = s[i+0].upl16(s[i+1]);
		r1 = s[i+2].upl16(s[i+3]);
		r2 = s[i+0].uph16(s[i+1]);
		r3 = s[i+2].uph16(s[i+3]);

		r4 = r0.upl32(r1);
		r5 = r0.uph32(r1);
		r6 = r2.upl32(r3);
		r7 = r2.uph32(r3);

		d[i+0] = r4.upl16(r5);
		d[i+1] = r6.upl16(r7);
		d[i+2] = r4.uph16(r5);
		d[i+3] = r6.uph16(r7);
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
	GSVector4i* s = (GSVector4i*)src;
	GSVector4i* d = (GSVector4i*)dst;

	GSVector4i r0 = s[0];
	GSVector4i r1 = s[1];
	GSVector4i r2 = s[32];
	GSVector4i r3 = s[33];

	d[0] = r0.upl16(r2);
	d[1] = r0.uph16(r2);
	d[2] = r1.upl16(r3);
	d[3] = r1.uph16(r3);
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
	GSVector4i* s = (GSVector4i*)src;
	GSVector4i* d = (GSVector4i*)dst;

	GSVector4i r0 = s[0];
	GSVector4i r1 = s[1];

	d[0] = r0.upl16();
	d[1] = r0.uph16();
	d[2] = r1.upl16();
	d[3] = r1.uph16();
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