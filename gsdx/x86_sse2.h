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

extern "C" void __fastcall memsetd(void* dst, unsigned int c, size_t len);

extern "C" void __fastcall unSwizzleBlock32_x86_sse2(BYTE* src, BYTE* dst, int dstpitch);
extern "C" void __fastcall unSwizzleBlock16_x86_sse2(BYTE* src, BYTE* dst, int dstpitch);
extern "C" void __fastcall unSwizzleBlock8_x86_sse2(BYTE* src, BYTE* dst, int dstpitch);
extern "C" void __fastcall unSwizzleBlock4_x86_sse2(BYTE* src, BYTE* dst, int dstpitch);
extern "C" void __fastcall unSwizzleBlock8HP_x86_sse2(BYTE* src, BYTE* dst, int dstpitch);
extern "C" void __fastcall unSwizzleBlock4HLP_x86_sse2(BYTE* src, BYTE* dst, int dstpitch);
extern "C" void __fastcall unSwizzleBlock4HHP_x86_sse2(BYTE* src, BYTE* dst, int dstpitch);
extern "C" void __fastcall unSwizzleBlock4P_x86_sse2(BYTE* src, BYTE* dst, int dstpitch);

extern "C" void __fastcall SwizzleBlock32_x86_sse2(BYTE* dst, BYTE* src, int srcpitch, DWORD WriteMask = 0xffffffff);
extern "C" void __fastcall SwizzleBlock16_x86_sse2(BYTE* dst, BYTE* src, int srcpitch);
extern "C" void __fastcall SwizzleBlock8_x86_sse2(BYTE* dst, BYTE* src, int srcpitch);
extern "C" void __fastcall SwizzleBlock4_x86_sse2(BYTE* dst, BYTE* src, int srcpitch);
extern "C" void __fastcall SwizzleBlock32u_x86_sse2(BYTE* dst, BYTE* src, int srcpitch, DWORD WriteMask = 0xffffffff);
extern "C" void __fastcall SwizzleBlock16u_x86_sse2(BYTE* dst, BYTE* src, int srcpitch);
extern "C" void __fastcall SwizzleBlock8u_x86_sse2(BYTE* dst, BYTE* src, int srcpitch);
extern "C" void __fastcall SwizzleBlock4u_x86_sse2(BYTE* dst, BYTE* src, int srcpitch);
