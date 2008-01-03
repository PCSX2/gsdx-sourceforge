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

extern "C" void unSwizzleBlock32_x64_sse2(BYTE* src, BYTE* dst, __int64 dstpitch);
extern "C" void unSwizzleBlock16_x64_sse2(BYTE* src, BYTE* dst, __int64 dstpitch);
extern "C" void unSwizzleBlock8_x64_sse2(BYTE* src, BYTE* dst, __int64 dstpitch);
extern "C" void unSwizzleBlock4_x64_sse2(BYTE* src, BYTE* dst, __int64 dstpitch);
extern "C" void unSwizzleBlock8HP_x64_sse2(BYTE* src, BYTE* dst, __int64 dstpitch);
extern "C" void unSwizzleBlock4HLP_x64_sse2(BYTE* src, BYTE* dst, __int64 dstpitch);
extern "C" void unSwizzleBlock4HHP_x64_sse2(BYTE* src, BYTE* dst, __int64 dstpitch);
extern "C" void unSwizzleBlock4P_x64_sse2(BYTE* src, BYTE* dst, __int64 dstpitch);

extern "C" void SwizzleBlock32_x64_sse2(BYTE* dst, BYTE* src, __int64 srcpitch, DWORD WriteMask = 0xffffffff);
extern "C" void SwizzleBlock16_x64_sse2(BYTE* dst, BYTE* src, __int64 srcpitch);
extern "C" void SwizzleBlock8_x64_sse2(BYTE* dst, BYTE* src, __int64 srcpitch);
extern "C" void SwizzleBlock4_x64_sse2(BYTE* dst, BYTE* src, __int64 srcpitch);
extern "C" void SwizzleBlock32u_x64_sse2(BYTE* dst, BYTE* src, __int64 srcpitch, DWORD WriteMask = 0xffffffff);
extern "C" void SwizzleBlock16u_x64_sse2(BYTE* dst, BYTE* src, __int64 srcpitch);
extern "C" void SwizzleBlock8u_x64_sse2(BYTE* dst, BYTE* src, __int64 srcpitch);
extern "C" void SwizzleBlock4u_x64_sse2(BYTE* dst, BYTE* src, __int64 srcpitch);
