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

#include "GS.h"

#include "x86_sse2.h"
#include "x86_sse3.h"
#include "x64_sse2.h"

extern void __fastcall unSwizzleBlock32_c(BYTE* src, BYTE* dst, int dstpitch);
extern void __fastcall unSwizzleBlock16_c(BYTE* src, BYTE* dst, int dstpitch);
extern void __fastcall unSwizzleBlock8_c(BYTE* src, BYTE* dst, int dstpitch);
extern void __fastcall unSwizzleBlock4_c(BYTE* src, BYTE* dst, int dstpitch);
extern void __fastcall unSwizzleBlock8HP_c(BYTE* src, BYTE* dst, int dstpitch);
extern void __fastcall unSwizzleBlock4HLP_c(BYTE* src, BYTE* dst, int dstpitch);
extern void __fastcall unSwizzleBlock4HHP_c(BYTE* src, BYTE* dst, int dstpitch);
extern void __fastcall unSwizzleBlock4P_c(BYTE* src, BYTE* dst, int dstpitch);
extern void __fastcall SwizzleBlock32_c(BYTE* dst, BYTE* src, int srcpitch, DWORD WriteMask = 0xffffffff);
extern void __fastcall SwizzleBlock16_c(BYTE* dst, BYTE* src, int srcpitch);
extern void __fastcall SwizzleBlock8_c(BYTE* dst, BYTE* src, int srcpitch);
extern void __fastcall SwizzleBlock4_c(BYTE* dst, BYTE* src, int srcpitch);

extern void __fastcall SwizzleColumn32_c(int y, BYTE* dst, BYTE* src, int srcpitch, DWORD WriteMask = 0xffffffff);
extern void __fastcall SwizzleColumn16_c(int y, BYTE* dst, BYTE* src, int srcpitch);
extern void __fastcall SwizzleColumn8_c(int y, BYTE* dst, BYTE* src, int srcpitch);
extern void __fastcall SwizzleColumn4_c(int y, BYTE* dst, BYTE* src, int srcpitch);

extern void __fastcall ExpandBlock24_sse2(DWORD* src, DWORD* dst, int dstpitch, GIFRegTEXA* pTEXA);
extern void __fastcall ExpandBlock16_sse2(WORD* src, DWORD* dst, int dstpitch, GIFRegTEXA* pTEXA);
extern void __fastcall Expand16_sse2(WORD* src, DWORD* dst, int w, GIFRegTEXA* pTEXA);
extern void __fastcall ExpandBlock24_c(DWORD* src, DWORD* dst, int dstpitch, GIFRegTEXA* pTEXA);
extern void __fastcall ExpandBlock16_c(WORD* src, DWORD* dst, int dstpitch, GIFRegTEXA* pTEXA);
extern void __fastcall Expand16_c(WORD* src, DWORD* dst, int w, GIFRegTEXA* pTEXA);

extern "C" void __fastcall WriteCLUT_T16_I8_CSM1_sse2(WORD* vm, WORD* clut);
extern "C" void __fastcall WriteCLUT_T32_I8_CSM1_sse2(DWORD* vm, WORD* clut);
extern "C" void __fastcall WriteCLUT_T16_I4_CSM1_sse2(WORD* vm, WORD* clut);
extern "C" void __fastcall WriteCLUT_T32_I4_CSM1_sse2(DWORD* vm, WORD* clut);
extern void __fastcall WriteCLUT_T16_I8_CSM1_c(WORD* vm, WORD* clut);
extern void __fastcall WriteCLUT_T32_I8_CSM1_c(DWORD* vm, WORD* clut);
extern void __fastcall WriteCLUT_T16_I4_CSM1_c(WORD* vm, WORD* clut);
extern void __fastcall WriteCLUT_T32_I4_CSM1_c(DWORD* vm, WORD* clut);

extern "C" void __fastcall ReadCLUT32_T32_I8_sse2(WORD* src, DWORD* dst);
extern "C" void __fastcall ReadCLUT32_T32_I4_sse2(WORD* src, DWORD* dst);
extern "C" void __fastcall ReadCLUT32_T16_I8_sse2(WORD* src, DWORD* dst);
extern "C" void __fastcall ReadCLUT32_T16_I4_sse2(WORD* src, DWORD* dst);
extern void __fastcall ReadCLUT32_T32_I8_c(WORD* src, DWORD* dst);
extern void __fastcall ReadCLUT32_T32_I4_c(WORD* src, DWORD* dst);
extern void __fastcall ReadCLUT32_T16_I8_c(WORD* src, DWORD* dst);
extern void __fastcall ReadCLUT32_T16_I4_c(WORD* src, DWORD* dst);

#ifdef _M_AMD64

#define unSwizzleBlock32 unSwizzleBlock32_x64_sse2
#define unSwizzleBlock16 unSwizzleBlock16_x64_sse2
#define unSwizzleBlock8 unSwizzleBlock8_x64_sse2
#define unSwizzleBlock4 unSwizzleBlock4_x64_sse2
#define unSwizzleBlock8HP unSwizzleBlock8HP_x64_sse2
#define unSwizzleBlock4HLP unSwizzleBlock4HLP_x64_sse2
#define unSwizzleBlock4HHP unSwizzleBlock4HHP_x64_sse2
#define unSwizzleBlock4P unSwizzleBlock4P_x64_sse2
#define SwizzleBlock32 SwizzleBlock32_x64_sse2
#define SwizzleBlock16 SwizzleBlock16_x64_sse2
#define SwizzleBlock8 SwizzleBlock8_x64_sse2
#define SwizzleBlock4 SwizzleBlock4_x64_sse2
#define SwizzleBlock32u SwizzleBlock32u_x64_sse2
#define SwizzleBlock16u SwizzleBlock16u_x64_sse2
#define SwizzleBlock8u SwizzleBlock8u_x64_sse2
#define SwizzleBlock4u SwizzleBlock4u_x64_sse2

#define SwizzleColumn32 SwizzleColumn32_c
#define SwizzleColumn16 SwizzleColumn16_c
#define SwizzleColumn8 SwizzleColumn8_c
#define SwizzleColumn4 SwizzleColumn4_c

#define ExpandBlock24 ExpandBlock24_sse2
#define ExpandBlock16 ExpandBlock16_sse2
#define Expand16 Expand16_sse2

#define WriteCLUT_T16_I8_CSM1 WriteCLUT_T16_I8_CSM1_sse2
#define WriteCLUT_T32_I8_CSM1 WriteCLUT_T32_I8_CSM1_sse2
#define WriteCLUT_T16_I4_CSM1 WriteCLUT_T16_I4_CSM1_sse2
#define WriteCLUT_T32_I4_CSM1 WriteCLUT_T32_I4_CSM1_sse2

#define ReadCLUT32_T32_I8 ReadCLUT32_T32_I8_sse2
#define ReadCLUT32_T32_I4 ReadCLUT32_T32_I4_sse2
#define ReadCLUT32_T16_I8 ReadCLUT32_T16_I8_sse2
#define ReadCLUT32_T16_I4 ReadCLUT32_T16_I4_sse2

#elif _M_SSE >= 3

#define unSwizzleBlock32 unSwizzleBlock32_x86_sse2
#define unSwizzleBlock16 unSwizzleBlock16_x86_sse3
#define unSwizzleBlock8 unSwizzleBlock8_x86_sse3
#define unSwizzleBlock4 unSwizzleBlock4_x86_sse2
#define unSwizzleBlock8HP unSwizzleBlock8HP_x86_sse2
#define unSwizzleBlock4HLP unSwizzleBlock4HLP_x86_sse2
#define unSwizzleBlock4HHP unSwizzleBlock4HHP_x86_sse2
#define unSwizzleBlock4P unSwizzleBlock4P_x86_sse2
#define SwizzleBlock32 SwizzleBlock32_x86_sse2
#define SwizzleBlock16 SwizzleBlock16_x86_sse2
#define SwizzleBlock8 SwizzleBlock8_x86_sse3
#define SwizzleBlock4 SwizzleBlock4_x86_sse2
#define SwizzleBlock32u SwizzleBlock32u_x86_sse2
#define SwizzleBlock16u SwizzleBlock16u_x86_sse2
#define SwizzleBlock8u SwizzleBlock8u_x86_sse3
#define SwizzleBlock4u SwizzleBlock4u_x86_sse2

#define SwizzleColumn32 SwizzleColumn32_c
#define SwizzleColumn16 SwizzleColumn16_c
#define SwizzleColumn8 SwizzleColumn8_c
#define SwizzleColumn4 SwizzleColumn4_c
#define SwizzleColumn4h SwizzleColumn4h_c

#define ExpandBlock24 ExpandBlock24_sse2
#define ExpandBlock16 ExpandBlock16_sse2
#define Expand16 Expand16_sse2

#define WriteCLUT_T16_I8_CSM1 WriteCLUT_T16_I8_CSM1_sse2
#define WriteCLUT_T32_I8_CSM1 WriteCLUT_T32_I8_CSM1_sse2
#define WriteCLUT_T16_I4_CSM1 WriteCLUT_T16_I4_CSM1_sse2
#define WriteCLUT_T32_I4_CSM1 WriteCLUT_T32_I4_CSM1_sse2

#define ReadCLUT32_T32_I8 ReadCLUT32_T32_I8_sse2
#define ReadCLUT32_T32_I4 ReadCLUT32_T32_I4_sse2
#define ReadCLUT32_T16_I8 ReadCLUT32_T16_I8_sse2
#define ReadCLUT32_T16_I4 ReadCLUT32_T16_I4_sse2

#elif _M_SSE >= 2

#define unSwizzleBlock32 unSwizzleBlock32_x86_sse2
#define unSwizzleBlock16 unSwizzleBlock16_x86_sse2
#define unSwizzleBlock8 unSwizzleBlock8_x86_sse2
#define unSwizzleBlock4 unSwizzleBlock4_x86_sse2
#define unSwizzleBlock8HP unSwizzleBlock8HP_x86_sse2
#define unSwizzleBlock4HLP unSwizzleBlock4HLP_x86_sse2
#define unSwizzleBlock4HHP unSwizzleBlock4HHP_x86_sse2
#define unSwizzleBlock4P unSwizzleBlock4P_x86_sse2
#define SwizzleBlock32 SwizzleBlock32_x86_sse2
#define SwizzleBlock16 SwizzleBlock16_x86_sse2
#define SwizzleBlock8 SwizzleBlock8_x86_sse2
#define SwizzleBlock4 SwizzleBlock4_x86_sse2
#define SwizzleBlock32u SwizzleBlock32u_x86_sse2
#define SwizzleBlock16u SwizzleBlock16u_x86_sse2
#define SwizzleBlock8u SwizzleBlock8u_x86_sse2
#define SwizzleBlock4u SwizzleBlock4u_x86_sse2

#define SwizzleColumn32 SwizzleColumn32_c
#define SwizzleColumn16 SwizzleColumn16_c
#define SwizzleColumn8 SwizzleColumn8_c
#define SwizzleColumn4 SwizzleColumn4_c
#define SwizzleColumn4h SwizzleColumn4h_c

#define ExpandBlock24 ExpandBlock24_sse2
#define ExpandBlock16 ExpandBlock16_sse2
#define Expand16 Expand16_sse2

#define WriteCLUT_T16_I8_CSM1 WriteCLUT_T16_I8_CSM1_sse2
#define WriteCLUT_T32_I8_CSM1 WriteCLUT_T32_I8_CSM1_sse2
#define WriteCLUT_T16_I4_CSM1 WriteCLUT_T16_I4_CSM1_sse2
#define WriteCLUT_T32_I4_CSM1 WriteCLUT_T32_I4_CSM1_sse2

#define ReadCLUT32_T32_I8 ReadCLUT32_T32_I8_sse2
#define ReadCLUT32_T32_I4 ReadCLUT32_T32_I4_sse2
#define ReadCLUT32_T16_I8 ReadCLUT32_T16_I8_sse2
#define ReadCLUT32_T16_I4 ReadCLUT32_T16_I4_sse2

#else

#define unSwizzleBlock32 unSwizzleBlock32_c
#define unSwizzleBlock16 unSwizzleBlock16_c
#define unSwizzleBlock8 unSwizzleBlock8_c
#define unSwizzleBlock4 unSwizzleBlock4_c
#define unSwizzleBlock8HP unSwizzleBlock8HP_c
#define unSwizzleBlock4HLP unSwizzleBlock4HLP_c
#define unSwizzleBlock4HHP unSwizzleBlock4HHP_c
#define unSwizzleBlock4P unSwizzleBlock4P_c
#define SwizzleBlock32 SwizzleBlock32_c
#define SwizzleBlock16 SwizzleBlock16_c
#define SwizzleBlock8 SwizzleBlock8_c
#define SwizzleBlock4 SwizzleBlock4_c
#define SwizzleBlock32u SwizzleBlock32_c
#define SwizzleBlock16u SwizzleBlock16_c
#define SwizzleBlock8u SwizzleBlock8_c
#define SwizzleBlock4u SwizzleBlock4_c

#define SwizzleColumn32 SwizzleColumn32_c
#define SwizzleColumn16 SwizzleColumn16_c
#define SwizzleColumn8 SwizzleColumn8_c
#define SwizzleColumn4 SwizzleColumn4_c

#define ExpandBlock24 ExpandBlock24_c
#define ExpandBlock16 ExpandBlock16_c
#define Expand16 Expand16_c

#define WriteCLUT_T16_I8_CSM1 WriteCLUT_T16_I8_CSM1_c
#define WriteCLUT_T32_I8_CSM1 WriteCLUT_T32_I8_CSM1_c
#define WriteCLUT_T16_I4_CSM1 WriteCLUT_T16_I4_CSM1_c
#define WriteCLUT_T32_I4_CSM1 WriteCLUT_T32_I4_CSM1_c

#define ReadCLUT32_T32_I8 ReadCLUT32_T32_I8_c
#define ReadCLUT32_T32_I4 ReadCLUT32_T32_I4_c
#define ReadCLUT32_T16_I8 ReadCLUT32_T16_I8_c
#define ReadCLUT32_T16_I4 ReadCLUT32_T16_I4_c

#endif
