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
#include "GSTables.h"
#include "GSVector.h"

class GSBlock
{
	static const GSVector4i m_r16mask;
	static const GSVector4i m_r8mask;
	static const GSVector4i m_r4mask;

	static const GSVector4i m_rgbx;
	static const GSVector4i m_xxxa;
	static const GSVector4i m_xxbx;
	static const GSVector4i m_xgxx;
	static const GSVector4i m_rxxx;

	static const GSVector4i m_uw8hmask0;
	static const GSVector4i m_uw8hmask1;
	static const GSVector4i m_uw8hmask2;
	static const GSVector4i m_uw8hmask3;

public:
	template<int i, bool aligned, DWORD mask> __forceinline static void WriteColumn32(BYTE* dst, BYTE* src, int srcpitch)
	{
		#if _M_SSE >= 0x200

		GSVector4i* s0 = (GSVector4i*)&src[srcpitch * 0];
		GSVector4i* s1 = (GSVector4i*)&src[srcpitch * 1];

		GSVector4i v0 = GSVector4i::load<aligned>(&s0[0]);
		GSVector4i v1 = GSVector4i::load<aligned>(&s0[1]);
		GSVector4i v2 = GSVector4i::load<aligned>(&s1[0]);
		GSVector4i v3 = GSVector4i::load<aligned>(&s1[1]);

		GSVector4i::sw64(v0, v2, v1, v3);

		if(mask == 0xffffffff)
		{
			((GSVector4i*)dst)[i * 4 + 0] = v0;
			((GSVector4i*)dst)[i * 4 + 1] = v1;
			((GSVector4i*)dst)[i * 4 + 2] = v2;
			((GSVector4i*)dst)[i * 4 + 3] = v3;
		}
		else
		{
			GSVector4i v4((int)mask);

			#if _M_SSE >= 0x401

			if(mask == 0xff000000 || mask == 0x00ffffff)
			{
				((GSVector4i*)dst)[i * 4 + 0] = ((GSVector4i*)dst)[i * 4 + 0].blend8(v0, v4);
				((GSVector4i*)dst)[i * 4 + 1] = ((GSVector4i*)dst)[i * 4 + 1].blend8(v1, v4);
				((GSVector4i*)dst)[i * 4 + 2] = ((GSVector4i*)dst)[i * 4 + 2].blend8(v2, v4);
				((GSVector4i*)dst)[i * 4 + 3] = ((GSVector4i*)dst)[i * 4 + 3].blend8(v3, v4);
			}
			else
			{

			#endif
			
			((GSVector4i*)dst)[i * 4 + 0] = ((GSVector4i*)dst)[i * 4 + 0].blend(v0, v4);
			((GSVector4i*)dst)[i * 4 + 1] = ((GSVector4i*)dst)[i * 4 + 1].blend(v1, v4);
			((GSVector4i*)dst)[i * 4 + 2] = ((GSVector4i*)dst)[i * 4 + 2].blend(v2, v4);
			((GSVector4i*)dst)[i * 4 + 3] = ((GSVector4i*)dst)[i * 4 + 3].blend(v3, v4);

			#if _M_SSE >= 0x401
			
			}

			#endif
		}

		#else

		const DWORD* d = &columnTable32[(i & 3) << 1][0];

		for(int j = 0; j < 2; j++, d += 8, src += srcpitch)
		{
			for(int i = 0; i < 8; i++)
			{
				if(mask == 0xffffffff)
				{
					((DWORD*)dst)[d[i]] = ((DWORD*)src)[i];
				}
				else
				{
					((DWORD*)dst)[d[i]] = (((DWORD*)dst)[d[i]] & ~mask) | (((DWORD*)src)[i] & mask);
				}
			}
		}

		#endif
	}

	template<int i, bool aligned> __forceinline static void WriteColumn16(BYTE* dst, BYTE* src, int srcpitch)
	{
		#if _M_SSE >= 0x200

		GSVector4i* s0 = (GSVector4i*)&src[srcpitch * 0];
		GSVector4i* s1 = (GSVector4i*)&src[srcpitch * 1];

		GSVector4i v0 = GSVector4i::load<aligned>(&s0[0]);
		GSVector4i v1 = GSVector4i::load<aligned>(&s0[1]);
		GSVector4i v2 = GSVector4i::load<aligned>(&s1[0]);
		GSVector4i v3 = GSVector4i::load<aligned>(&s1[1]);

		GSVector4i::sw16(v0, v1, v2, v3);
		GSVector4i::sw64(v0, v1, v2, v3);

		((GSVector4i*)dst)[i * 4 + 0] = v0;
		((GSVector4i*)dst)[i * 4 + 1] = v2;
		((GSVector4i*)dst)[i * 4 + 2] = v1;
		((GSVector4i*)dst)[i * 4 + 3] = v3;

		#else

		const DWORD* d = &columnTable16[(i & 3) << 1][0];

		for(int j = 0; j < 2; j++, d += 16, src += srcpitch)
		{
			for(int i = 0; i < 16; i++)
			{
				((WORD*)dst)[d[i]] = ((WORD*)src)[i];
			}
		}

		#endif
	}

	template<int i, bool aligned> __forceinline static void WriteColumn8(BYTE* dst, BYTE* src, int srcpitch)
	{
		#if _M_SSE >= 0x200

		GSVector4i v0 = GSVector4i::load<aligned>(&src[srcpitch * 0]);
		GSVector4i v1 = GSVector4i::load<aligned>(&src[srcpitch * 1]);
		GSVector4i v2 = GSVector4i::load<aligned>(&src[srcpitch * 2]);
		GSVector4i v3 = GSVector4i::load<aligned>(&src[srcpitch * 3]);

		if((i & 1) == 0)
		{
			v2 = v2.yxwz();
			v3 = v3.yxwz();
		}
		else
		{
			v0 = v0.yxwz();
			v1 = v1.yxwz();
		}

		GSVector4i::sw8(v0, v2, v1, v3);
		GSVector4i::sw16(v0, v1, v2, v3);
		GSVector4i::sw64(v0, v1, v2, v3);

		((GSVector4i*)dst)[i * 4 + 0] = v0;
		((GSVector4i*)dst)[i * 4 + 1] = v2;
		((GSVector4i*)dst)[i * 4 + 2] = v1;
		((GSVector4i*)dst)[i * 4 + 3] = v3;

		#else

		const DWORD* d = &columnTable8[(i & 3) << 2][0];

		for(int j = 0; j < 4; j++, d += 16, src += srcpitch)
		{
			for(int i = 0; i < 16; i++)
			{
				dst[d[i]] = src[i];
			}
		}

		#endif
	}

	template<int i, bool aligned> __forceinline static void WriteColumn4(BYTE* dst, BYTE* src, int srcpitch)
	{
		// TODO: pshufb

		#if _M_SSE >= 0x200

		GSVector4i v0 = GSVector4i::load<aligned>(&src[srcpitch * 0]);
		GSVector4i v1 = GSVector4i::load<aligned>(&src[srcpitch * 1]);
		GSVector4i v2 = GSVector4i::load<aligned>(&src[srcpitch * 2]);
		GSVector4i v3 = GSVector4i::load<aligned>(&src[srcpitch * 3]);

		if((i & 1) == 0)
		{
			v2 = v2.yxwzl().yxwzh();
			v3 = v3.yxwzl().yxwzh();
		}
		else
		{
			v0 = v0.yxwzl().yxwzh();
			v1 = v1.yxwzl().yxwzh();
		}

		GSVector4i::sw4(v0, v2, v1, v3);
		GSVector4i::sw8(v0, v1, v2, v3);
		GSVector4i::sw8(v0, v2, v1, v3);
		GSVector4i::sw64(v0, v2, v1, v3);

		((GSVector4i*)dst)[i * 4 + 0] = v0;
		((GSVector4i*)dst)[i * 4 + 1] = v1;
		((GSVector4i*)dst)[i * 4 + 2] = v2;
		((GSVector4i*)dst)[i * 4 + 3] = v3;

		#else

		const DWORD* d = &columnTable4[(i & 3) << 2][0];

		for(int j = 0; j < 4; j++, d += 32, src += srcpitch)
		{
			for(int i = 0; i < 32; i++)
			{
				DWORD addr = d[i];
				BYTE c = (src[i >> 1] >> ((i & 1) << 2)) & 0x0f;
				DWORD shift = (addr & 1) << 2;
				dst[addr >> 1] = (dst[addr >> 1] & (0xf0 >> shift)) | (c << shift);
			}
		}

		#endif
	}

	template<bool aligned, DWORD mask> static void WriteColumn32(int y, BYTE* dst, BYTE* src, int srcpitch)
	{
		switch((y >> 1) & 3)
		{
		case 0: WriteColumn32<0, aligned, mask>(dst, src, srcpitch); break;
		case 1: WriteColumn32<1, aligned, mask>(dst, src, srcpitch); break;
		case 2: WriteColumn32<2, aligned, mask>(dst, src, srcpitch); break;
		case 3: WriteColumn32<3, aligned, mask>(dst, src, srcpitch); break;
		default: __assume(0);
		}
	}

	template<bool aligned> static void WriteColumn16(int y, BYTE* dst, BYTE* src, int srcpitch)
	{
		switch((y >> 1) & 3)
		{
		case 0: WriteColumn16<0, aligned>(dst, src, srcpitch); break;
		case 1: WriteColumn16<1, aligned>(dst, src, srcpitch); break;
		case 2: WriteColumn16<2, aligned>(dst, src, srcpitch); break;
		case 3: WriteColumn16<3, aligned>(dst, src, srcpitch); break;
		default: __assume(0);
		}
	}

	template<bool aligned> static void WriteColumn8(int y, BYTE* dst, BYTE* src, int srcpitch)
	{
		switch((y >> 2) & 3)
		{
		case 0: WriteColumn8<0, aligned>(dst, src, srcpitch); break;
		case 1: WriteColumn8<1, aligned>(dst, src, srcpitch); break;
		case 2: WriteColumn8<2, aligned>(dst, src, srcpitch); break;
		case 3: WriteColumn8<3, aligned>(dst, src, srcpitch); break;
		default: __assume(0);
		}
	}

	template<bool aligned> static void WriteColumn4(int y, BYTE* dst, BYTE* src, int srcpitch)
	{
		switch((y >> 2) & 3)
		{
		case 0: WriteColumn4<0, aligned>(dst, src, srcpitch); break;
		case 1: WriteColumn4<1, aligned>(dst, src, srcpitch); break;
		case 2: WriteColumn4<2, aligned>(dst, src, srcpitch); break;
		case 3: WriteColumn4<3, aligned>(dst, src, srcpitch); break;
		default: __assume(0);
		}
	}

	template<bool aligned, DWORD mask> static void WriteBlock32(BYTE* dst, BYTE* src, int srcpitch)
	{
		#if _M_SSE >= 0x200

		WriteColumn32<0, aligned, mask>(dst, src, srcpitch);
		src += srcpitch * 2;
		WriteColumn32<1, aligned, mask>(dst, src, srcpitch);
		src += srcpitch * 2;
		WriteColumn32<2, aligned, mask>(dst, src, srcpitch);
		src += srcpitch * 2;
		WriteColumn32<3, aligned, mask>(dst, src, srcpitch);

		#else

		const DWORD* d = &columnTable32[0][0];

		for(int j = 0; j < 8; j++, d += 8, src += srcpitch)
		{
			for(int i = 0; i < 8; i++)
			{
				if(mask == 0xffffffff)
				{
					((DWORD*)dst)[d[i]] = ((DWORD*)src)[i];
				}
				else
				{
					((DWORD*)dst)[d[i]] = (((DWORD*)dst)[d[i]] & ~mask) | (((DWORD*)src)[i] & mask);
				}
			}
		}

		#endif
	}

	template<bool aligned> static void WriteBlock16(BYTE* dst, BYTE* src, int srcpitch)
	{
		#if _M_SSE >= 0x200

		WriteColumn16<0, aligned>(dst, src, srcpitch);
		src += srcpitch * 2;
		WriteColumn16<1, aligned>(dst, src, srcpitch);
		src += srcpitch * 2;
		WriteColumn16<2, aligned>(dst, src, srcpitch);
		src += srcpitch * 2;
		WriteColumn16<3, aligned>(dst, src, srcpitch);

		#else

		const DWORD* d = &columnTable16[0][0];

		for(int j = 0; j < 8; j++, d += 16, src += srcpitch)
		{
			for(int i = 0; i < 16; i++)
			{
				((WORD*)dst)[d[i]] = ((WORD*)src)[i];
			}
		}

		#endif
	}

	template<bool aligned> static void WriteBlock8(BYTE* dst, BYTE* src, int srcpitch)
	{
		#if _M_SSE >= 0x200

		WriteColumn8<0, aligned>(dst, src, srcpitch);
		src += srcpitch * 4;
		WriteColumn8<1, aligned>(dst, src, srcpitch);
		src += srcpitch * 4;
		WriteColumn8<2, aligned>(dst, src, srcpitch);
		src += srcpitch * 4;
		WriteColumn8<3, aligned>(dst, src, srcpitch);

		#else

		const DWORD* d = &columnTable8[0][0];

		for(int j = 0; j < 16; j++, d += 16, src += srcpitch)
		{
			for(int i = 0; i < 16; i++)
			{
				dst[d[i]] = src[i];
			}
		}

		#endif
	}

	template<bool aligned> static void WriteBlock4(BYTE* dst, BYTE* src, int srcpitch)
	{
		#if _M_SSE >= 0x200

		WriteColumn4<0, aligned>(dst, src, srcpitch);
		src += srcpitch * 4;
		WriteColumn4<1, aligned>(dst, src, srcpitch);
		src += srcpitch * 4;
		WriteColumn4<2, aligned>(dst, src, srcpitch);
		src += srcpitch * 4;
		WriteColumn4<3, aligned>(dst, src, srcpitch);

		#else

		const DWORD* d = &columnTable4[0][0];

		for(int j = 0; j < 16; j++, d += 32, src += srcpitch)
		{
			for(int i = 0; i < 32; i++)
			{
				DWORD addr = d[i];
				BYTE c = (src[i >> 1] >> ((i & 1) << 2)) & 0x0f;
				DWORD shift = (addr & 1) << 2;
				dst[addr >> 1] = (dst[addr >> 1] & (0xf0 >> shift)) | (c << shift);
			}
		}

		#endif
	}

	template<int i, bool aligned> __forceinline static void ReadColumn32(BYTE* src, BYTE* dst, int dstpitch)
	{
		#if _M_SSE >= 0x200

		GSVector4i v0 = ((GSVector4i*)src)[i * 4 + 0]; 
		GSVector4i v1 = ((GSVector4i*)src)[i * 4 + 1]; 
		GSVector4i v2 = ((GSVector4i*)src)[i * 4 + 2]; 
		GSVector4i v3 = ((GSVector4i*)src)[i * 4 + 3];

		GSVector4i::sw64(v0, v1, v2, v3);

		GSVector4i* d0 = (GSVector4i*)&dst[dstpitch * 0];
		GSVector4i* d1 = (GSVector4i*)&dst[dstpitch * 1];

		GSVector4i::store<aligned>(&d0[0], v0);
		GSVector4i::store<aligned>(&d0[1], v1);
		GSVector4i::store<aligned>(&d1[0], v2);
		GSVector4i::store<aligned>(&d1[1], v3);

		#else

		const DWORD* s = &columnTable32[(i & 3) << 1][0];

		for(int j = 0; j < 2; j++, s += 8, dst += dstpitch)
		{
			for(int i = 0; i < 8; i++)
			{
				((DWORD*)dst)[i] = ((DWORD*)src)[s[i]];
			}
		}

		#endif
	}

	template<int i, bool aligned> __forceinline static void ReadColumn16(BYTE* src, BYTE* dst, int dstpitch)
	{
		#if _M_SSE >= 0x301

		GSVector4i v0 = ((GSVector4i*)src)[i * 4 + 0].shuffle8(m_r16mask);
		GSVector4i v1 = ((GSVector4i*)src)[i * 4 + 1].shuffle8(m_r16mask);
		GSVector4i v2 = ((GSVector4i*)src)[i * 4 + 2].shuffle8(m_r16mask);
		GSVector4i v3 = ((GSVector4i*)src)[i * 4 + 3].shuffle8(m_r16mask);

		GSVector4i::sw32(v0, v1, v2, v3);
		GSVector4i::sw64(v0, v1, v2, v3);

		GSVector4i* d0 = (GSVector4i*)&dst[dstpitch * 0];
		GSVector4i* d1 = (GSVector4i*)&dst[dstpitch * 1];

		GSVector4i::store<aligned>(&d0[0], v0);
		GSVector4i::store<aligned>(&d0[1], v2);
		GSVector4i::store<aligned>(&d1[0], v1);
		GSVector4i::store<aligned>(&d1[1], v3);

		#elif _M_SSE >= 0x200

		GSVector4i v0 = ((GSVector4i*)src)[i * 4 + 0]; 
		GSVector4i v1 = ((GSVector4i*)src)[i * 4 + 1]; 
		GSVector4i v2 = ((GSVector4i*)src)[i * 4 + 2]; 
		GSVector4i v3 = ((GSVector4i*)src)[i * 4 + 3];

		GSVector4i::sw16(v0, v1, v2, v3);
		GSVector4i::sw32(v0, v1, v2, v3);
		GSVector4i::sw16(v0, v2, v1, v3);

		GSVector4i* d0 = (GSVector4i*)&dst[dstpitch * 0];
		GSVector4i* d1 = (GSVector4i*)&dst[dstpitch * 1];

		GSVector4i::store<aligned>(&d0[0], v0);
		GSVector4i::store<aligned>(&d0[1], v1);
		GSVector4i::store<aligned>(&d1[0], v2);
		GSVector4i::store<aligned>(&d1[1], v3);

		#else

		const DWORD* s = &columnTable16[(i & 3) << 1][0];

		for(int j = 0; j < 2; j++, s += 16, dst += dstpitch)
		{
			for(int i = 0; i < 16; i++)
			{
				((WORD*)dst)[i] = ((WORD*)src)[s[i]];
			}
		}

		#endif
	}

	template<int i, bool aligned> __forceinline static void ReadColumn8(BYTE* src, BYTE* dst, int dstpitch)
	{
		#if _M_SSE >= 0x301

		GSVector4i v0, v1, v2, v3;

		if((i & 1) == 0)
		{
			v0 = ((GSVector4i*)src)[i * 4 + 0]; 
			v1 = ((GSVector4i*)src)[i * 4 + 1]; 
			v2 = ((GSVector4i*)src)[i * 4 + 2]; 
			v3 = ((GSVector4i*)src)[i * 4 + 3];
		}
		else
		{
			v2 = ((GSVector4i*)src)[i * 4 + 0]; 
			v3 = ((GSVector4i*)src)[i * 4 + 1]; 
			v0 = ((GSVector4i*)src)[i * 4 + 2]; 
			v1 = ((GSVector4i*)src)[i * 4 + 3];
		}

		v0 = v0.shuffle8(m_r8mask);
		v1 = v1.shuffle8(m_r8mask);
		v2 = v2.shuffle8(m_r8mask);
		v3 = v3.shuffle8(m_r8mask);

		GSVector4i::sw16(v0, v1, v2, v3);
		GSVector4i::sw32(v0, v1, v3, v2);

		GSVector4i::store<aligned>(&dst[dstpitch * 0], v0);
		GSVector4i::store<aligned>(&dst[dstpitch * 1], v3);
		GSVector4i::store<aligned>(&dst[dstpitch * 2], v1);
		GSVector4i::store<aligned>(&dst[dstpitch * 3], v2);

		#elif _M_SSE >= 0x200

		GSVector4i v0 = ((GSVector4i*)src)[i * 4 + 0]; 
		GSVector4i v1 = ((GSVector4i*)src)[i * 4 + 1]; 
		GSVector4i v2 = ((GSVector4i*)src)[i * 4 + 2]; 
		GSVector4i v3 = ((GSVector4i*)src)[i * 4 + 3];

		GSVector4i::sw8(v0, v1, v2, v3);
		GSVector4i::sw16(v0, v1, v2, v3);
		GSVector4i::sw8(v0, v2, v1, v3);
		GSVector4i::sw64(v0, v1, v2, v3);

		if((i & 1) == 0)
		{
			v2 = v2.yxwz();
			v3 = v3.yxwz();
		}
		else
		{
			v0 = v0.yxwz();
			v1 = v1.yxwz();
		}		

		GSVector4i::store<aligned>(&dst[dstpitch * 0], v0);
		GSVector4i::store<aligned>(&dst[dstpitch * 1], v1);
		GSVector4i::store<aligned>(&dst[dstpitch * 2], v2);
		GSVector4i::store<aligned>(&dst[dstpitch * 3], v3);

		#else

		const DWORD* s = &columnTable8[(i & 3) << 2][0];

		for(int j = 0; j < 4; j++, s += 16, dst += dstpitch)
		{
			for(int i = 0; i < 16; i++)
			{
				dst[i] = src[s[i]];
			}
		}

		#endif
	}
	
	template<int i, bool aligned> __forceinline static void ReadColumn4(BYTE* src, BYTE* dst, int dstpitch)
	{
		#if _M_SSE >= 0x301

		GSVector4i v0 = ((GSVector4i*)src)[i * 4 + 0].xzyw(); 
		GSVector4i v1 = ((GSVector4i*)src)[i * 4 + 1].xzyw(); 
		GSVector4i v2 = ((GSVector4i*)src)[i * 4 + 2].xzyw(); 
		GSVector4i v3 = ((GSVector4i*)src)[i * 4 + 3].xzyw();

		GSVector4i::sw64(v0, v1, v2, v3);
		GSVector4i::sw4(v0, v2, v1, v3);
		GSVector4i::sw8(v0, v1, v2, v3);

		v0 = v0.shuffle8(m_r4mask);
		v1 = v1.shuffle8(m_r4mask);
		v2 = v2.shuffle8(m_r4mask);
		v3 = v3.shuffle8(m_r4mask);

		if((i & 1) == 0)
		{
			GSVector4i::sw16rh(v0, v1, v2, v3);
		}
		else
		{
			GSVector4i::sw16rl(v0, v1, v2, v3);
		}

		GSVector4i::store<aligned>(&dst[dstpitch * 0], v0);
		GSVector4i::store<aligned>(&dst[dstpitch * 1], v1);
		GSVector4i::store<aligned>(&dst[dstpitch * 2], v2);
		GSVector4i::store<aligned>(&dst[dstpitch * 3], v3);

		#elif _M_SSE >= 0x200

		GSVector4i v0 = ((GSVector4i*)src)[i * 4 + 0]; 
		GSVector4i v1 = ((GSVector4i*)src)[i * 4 + 1]; 
		GSVector4i v2 = ((GSVector4i*)src)[i * 4 + 2]; 
		GSVector4i v3 = ((GSVector4i*)src)[i * 4 + 3];

		GSVector4i::sw32(v0, v1, v2, v3);
		GSVector4i::sw32(v0, v1, v2, v3);
		GSVector4i::sw4(v0, v2, v1, v3);
		GSVector4i::sw8(v0, v1, v2, v3);
		GSVector4i::sw16(v0, v2, v1, v3);

		v0 = v0.xzyw();
		v1 = v1.xzyw();
		v2 = v2.xzyw();
		v3 = v3.xzyw();

		GSVector4i::sw64(v0, v1, v2, v3);

		if((i & 1) == 0)
		{
			v2 = v2.yxwzl().yxwzh();
			v3 = v3.yxwzl().yxwzh();
		}
		else
		{
			v0 = v0.yxwzl().yxwzh();
			v1 = v1.yxwzl().yxwzh();
		}

		GSVector4i::store<aligned>(&dst[dstpitch * 0], v0);
		GSVector4i::store<aligned>(&dst[dstpitch * 1], v1);
		GSVector4i::store<aligned>(&dst[dstpitch * 2], v2);
		GSVector4i::store<aligned>(&dst[dstpitch * 3], v3);

		#else

		const DWORD* s = &columnTable4[(i & 3) << 2][0];

		for(int j = 0; j < 4; j++, s += 32, dst += dstpitch)
		{
			for(int i = 0; i < 32; i++)
			{
				DWORD addr = s[i];
				BYTE c = (src[addr >> 1] >> ((addr & 1) << 2)) & 0x0f;
				int shift = (i & 1) << 2;
				dst[i >> 1] = (dst[i >> 1] & (0xf0 >> shift)) | (c << shift);
			}
		}

		#endif
	}

	template<bool aligned> static void ReadBlock32(BYTE* src, BYTE* dst, int dstpitch)
	{
		#if _M_SSE >= 0x200

		ReadColumn32<0, aligned>(src, dst, dstpitch);
		dst += dstpitch * 2;
		ReadColumn32<1, aligned>(src, dst, dstpitch);
		dst += dstpitch * 2;
		ReadColumn32<2, aligned>(src, dst, dstpitch);
		dst += dstpitch * 2;
		ReadColumn32<3, aligned>(src, dst, dstpitch);

		#else

		const DWORD* s = &columnTable32[0][0];

		for(int j = 0; j < 8; j++, s += 8, dst += dstpitch)
		{
			for(int i = 0; i < 8; i++)
			{
				((DWORD*)dst)[i] = ((DWORD*)src)[s[i]];
			}
		}

		#endif
	}

	template<bool aligned> static void ReadBlock16(BYTE* src, BYTE* dst, int dstpitch)
	{
		#if _M_SSE >= 0x200

		ReadColumn16<0, aligned>(src, dst, dstpitch);
		dst += dstpitch * 2;
		ReadColumn16<1, aligned>(src, dst, dstpitch);
		dst += dstpitch * 2;
		ReadColumn16<2, aligned>(src, dst, dstpitch);
		dst += dstpitch * 2;
		ReadColumn16<3, aligned>(src, dst, dstpitch);

		#else

		const DWORD* s = &columnTable16[0][0];

		for(int j = 0; j < 8; j++, s += 16, dst += dstpitch)
		{
			for(int i = 0; i < 16; i++)
			{
				((WORD*)dst)[i] = ((WORD*)src)[s[i]];
			}
		}

		#endif
	}

	template<bool aligned> static void ReadBlock8(BYTE* src, BYTE* dst, int dstpitch)
	{
		#if _M_SSE >= 0x200

		ReadColumn8<0, aligned>(src, dst, dstpitch);
		dst += dstpitch * 4;
		ReadColumn8<1, aligned>(src, dst, dstpitch);
		dst += dstpitch * 4;
		ReadColumn8<2, aligned>(src, dst, dstpitch);
		dst += dstpitch * 4;
		ReadColumn8<3, aligned>(src, dst, dstpitch);

		#else

		const DWORD* s = &columnTable8[0][0];

		for(int j = 0; j < 16; j++, s += 16, dst += dstpitch)
		{
			for(int i = 0; i < 16; i++)
			{
				dst[i] = src[s[i]];
			}
		}

		#endif
	}

	template<bool aligned> static void ReadBlock4(BYTE* src, BYTE* dst, int dstpitch)
	{
		#if _M_SSE >= 0x200

		ReadColumn4<0, aligned>(src, dst, dstpitch);
		dst += dstpitch * 4;
		ReadColumn4<1, aligned>(src, dst, dstpitch);
		dst += dstpitch * 4;
		ReadColumn4<2, aligned>(src, dst, dstpitch);
		dst += dstpitch * 4;
		ReadColumn4<3, aligned>(src, dst, dstpitch);

		#else

		const DWORD* s = &columnTable4[0][0];

		for(int j = 0; j < 16; j++, s += 32, dst += dstpitch)
		{
			for(int i = 0; i < 32; i++)
			{
				DWORD addr = s[i];
				BYTE c = (src[addr >> 1] >> ((addr & 1) << 2)) & 0x0f;
				int shift = (i & 1) << 2;
				dst[i >> 1] = (dst[i >> 1] & (0xf0 >> shift)) | (c << shift);
			}
		}

		#endif
	}

	static void UnpackBlock24(BYTE* src, int srcpitch, DWORD* dst)
	{
		#if _M_SSE >= 0x200

		GSVector4i rgbx = m_rgbx;

		for(int i = 0; i < 4; i++, src += srcpitch * 2)
		{
			GSVector4i v0 = GSVector4i::loadu(src);
			GSVector4i v1 = GSVector4i::loadu(src + 16, src + srcpitch);
			GSVector4i v2 = GSVector4i::loadu(src + srcpitch + 8);

			((GSVector4i*)dst)[i * 4 + 0] = v0.upl32(v0.srl<3>()).upl64(v0.srl<6>().upl32(v0.srl<9>())) & rgbx;

			v0 = v0.srl<12>(v1);

			((GSVector4i*)dst)[i * 4 + 1] = v0.upl32(v0.srl<3>()).upl64(v0.srl<6>().upl32(v0.srl<9>())) & rgbx;

			v0 = v1.srl<8>(v2);

			((GSVector4i*)dst)[i * 4 + 2] = v0.upl32(v0.srl<3>()).upl64(v0.srl<6>().upl32(v0.srl<9>())) & rgbx;

			v0 = v2.srl<4>();

			((GSVector4i*)dst)[i * 4 + 3] = v0.upl32(v0.srl<3>()).upl64(v0.srl<6>().upl32(v0.srl<9>())) & rgbx;
		}

		#else 

		for(int j = 0, diff = srcpitch - 8 * 3; j < 8; j++, src += diff, dst += 8)
		{
			for(int i = 0; i < 8; i++, src += 3)
			{
				dst[i] = (src[2] << 16) | (src[1] << 8) | src[0];
			}
		}

		#endif
	}

	static void UnpackBlock8H(BYTE* src, int srcpitch, DWORD* dst)
	{
		#if _M_SSE >= 0x200

		GSVector4i zero = GSVector4i::zero();

		for(int i = 0; i < 4; i++, src += srcpitch * 2)
		{
			GSVector4i v = GSVector4i::loadu(src, src + srcpitch);

			GSVector4i v0 = zero.upl8(v);
			GSVector4i v1 = zero.uph8(v);

			((GSVector4i*)dst)[i * 4 + 0] = zero.upl16(v0);
			((GSVector4i*)dst)[i * 4 + 1] = zero.uph16(v0);
			((GSVector4i*)dst)[i * 4 + 2] = zero.upl16(v1);
			((GSVector4i*)dst)[i * 4 + 3] = zero.uph16(v1);
		}

		#else

		for(int j = 0; j < 8; j++, src += srcpitch, dst += 8)
		{
			for(int i = 0; i < 8; i++)
			{
				dst[i] = src[i] << 24;
			}
		}

		#endif
	}

	static void UnpackBlock4HL(BYTE* src, int srcpitch, DWORD* dst)
	{
		#if _M_SSE >= 0x200

		GSVector4i zero = GSVector4i::zero();
		GSVector4i mask(0x0f0f0f0f);

		for(int i = 0; i < 2; i++, src += srcpitch * 4)
		{
			GSVector4i v(
				*(DWORD*)&src[srcpitch * 0], 
				*(DWORD*)&src[srcpitch * 1], 
				*(DWORD*)&src[srcpitch * 2], 
				*(DWORD*)&src[srcpitch * 3]);

			GSVector4i lo = v & mask;
			GSVector4i hi = (v >> 4) & mask;

			GSVector4i v0 = lo.upl8(hi);
			GSVector4i v1 = lo.uph8(hi);

			GSVector4i v2 = zero.upl8(v0);
			GSVector4i v3 = zero.uph8(v0);
			GSVector4i v4 = zero.upl8(v1);
			GSVector4i v5 = zero.uph8(v1);

			((GSVector4i*)dst)[i * 8 + 0] = zero.upl16(v2);
			((GSVector4i*)dst)[i * 8 + 1] = zero.uph16(v2);
			((GSVector4i*)dst)[i * 8 + 2] = zero.upl16(v3);
			((GSVector4i*)dst)[i * 8 + 3] = zero.uph16(v3);
			((GSVector4i*)dst)[i * 8 + 4] = zero.upl16(v4);
			((GSVector4i*)dst)[i * 8 + 5] = zero.uph16(v4);
			((GSVector4i*)dst)[i * 8 + 6] = zero.upl16(v5);
			((GSVector4i*)dst)[i * 8 + 7] = zero.uph16(v5);
		}

		#else

		for(int j = 0; j < 8; j++, src += srcpitch, dst += 8)
		{
			for(int i = 0; i < 4; i++)
			{
				dst[i * 2 + 0] = (src[i] & 0x0f) << 24;
				dst[i * 2 + 1] = (src[i] & 0xf0) << 20;
			}
		}

		#endif
	}

	static void UnpackBlock4HH(BYTE* src, int srcpitch, DWORD* dst)
	{
		#if _M_SSE >= 0x200

		GSVector4i zero = GSVector4i::zero();
		GSVector4i mask(0xf0f0f0f0);

		for(int i = 0; i < 2; i++, src += srcpitch * 4)
		{
			GSVector4i v(
				*(DWORD*)&src[srcpitch * 0], 
				*(DWORD*)&src[srcpitch * 1], 
				*(DWORD*)&src[srcpitch * 2], 
				*(DWORD*)&src[srcpitch * 3]);

			GSVector4i lo = (v << 4) & mask;
			GSVector4i hi = v & mask;

			GSVector4i v0 = lo.upl8(hi);
			GSVector4i v1 = lo.uph8(hi);

			GSVector4i v2 = zero.upl8(v0);
			GSVector4i v3 = zero.uph8(v0);
			GSVector4i v4 = zero.upl8(v1);
			GSVector4i v5 = zero.uph8(v1);

			((GSVector4i*)dst)[i * 8 + 0] = zero.upl16(v2);
			((GSVector4i*)dst)[i * 8 + 1] = zero.uph16(v2);
			((GSVector4i*)dst)[i * 8 + 2] = zero.upl16(v3);
			((GSVector4i*)dst)[i * 8 + 3] = zero.uph16(v3);
			((GSVector4i*)dst)[i * 8 + 4] = zero.upl16(v4);
			((GSVector4i*)dst)[i * 8 + 5] = zero.uph16(v4);
			((GSVector4i*)dst)[i * 8 + 6] = zero.upl16(v5);
			((GSVector4i*)dst)[i * 8 + 7] = zero.uph16(v5);
		}

		#else

		for(int j = 0; j < 8; j++, src += srcpitch, dst += 8)
		{
			for(int i = 0; i < 4; i++)
			{
				dst[i * 2 + 0] = (src[i] & 0x0f) << 28;
				dst[i * 2 + 1] = (src[i] & 0xf0) << 24;
			}
		}

		#endif
	}

	template<bool AEM> static void ExpandBlock24(DWORD* src, BYTE* dst, int dstpitch, const GIFRegTEXA& TEXA)
	{
		#if _M_SSE >= 0x200

		GSVector4i TA0(TEXA.TA0 << 24);
		GSVector4i mask = m_rgbx;

		for(int i = 0; i < 4; i++, dst += dstpitch * 2)
		{
			GSVector4i v0 = ((GSVector4i*)src)[i * 4 + 0] & mask;
			GSVector4i v1 = ((GSVector4i*)src)[i * 4 + 1] & mask;
			GSVector4i v2 = ((GSVector4i*)src)[i * 4 + 2] & mask;
			GSVector4i v3 = ((GSVector4i*)src)[i * 4 + 3] & mask;

			GSVector4i* d0 = (GSVector4i*)&dst[dstpitch * 0];
			GSVector4i* d1 = (GSVector4i*)&dst[dstpitch * 1];

			if(AEM)
			{
				d0[0] = v0 | TA0.andnot(v0 == GSVector4i::zero()); // TA0 & (v0 != GSVector4i::zero())
				d0[1] = v1 | TA0.andnot(v1 == GSVector4i::zero()); // TA0 & (v1 != GSVector4i::zero())
				d1[0] = v2 | TA0.andnot(v2 == GSVector4i::zero()); // TA0 & (v2 != GSVector4i::zero())
				d1[1] = v3 | TA0.andnot(v3 == GSVector4i::zero()); // TA0 & (v3 != GSVector4i::zero())
			}
			else
			{
				d0[0] = v0 | TA0;
				d0[1] = v1 | TA0;
				d1[0] = v2 | TA0;
				d1[1] = v3 | TA0;
			}
		}

		#else

		DWORD TA0 = TEXA.TA0 << 24;

		for(int j = 0; j < 8; j++, src += 8, dst += dstpitch)
		{
			for(int i = 0; i < 8; i++)
			{
				DWORD c = src[i] & 0xffffff;

				if(AEM)
				{
					((DWORD*)dst)[i] = c | (c ? TA0 : 0);
				}
				else
				{
					((DWORD*)dst)[i] = c | TA0;
				}
			}
		}

		#endif
	}

	static void ExpandBlock16(WORD* src, BYTE* dst, int dstpitch, const GIFRegTEXA& TEXA) // do not inline, uses too many xmm regs
	{
		#if _M_SSE >= 0x200

		GSVector4i TA0(TEXA.TA0 << 24);
		GSVector4i TA1(TEXA.TA1 << 24);
		GSVector4i rm = m_rxxx;
		GSVector4i gm = m_xgxx;
		GSVector4i bm = m_xxbx;
		GSVector4i am = m_xxxa;
		GSVector4i l, h;

		if(TEXA.AEM)
		{
			for(int i = 0; i < 8; i++, dst += dstpitch)
			{
				GSVector4i v0 = ((GSVector4i*)src)[i * 2 + 0];

				l = v0.upl16();
				h = v0.uph16();

				((GSVector4i*)dst)[0] = ((l & rm) << 3) | ((l & gm) << 6) | ((l & bm) << 9) | TA1.blend(TA0, l < am).andnot(l == GSVector4i::zero());
				((GSVector4i*)dst)[1] = ((h & rm) << 3) | ((h & gm) << 6) | ((h & bm) << 9) | TA1.blend(TA0, h < am).andnot(h == GSVector4i::zero());

				GSVector4i v1 = ((GSVector4i*)src)[i * 2 + 1];

				l = v1.upl16();
				h = v1.uph16();

				((GSVector4i*)dst)[2] = ((l & rm) << 3) | ((l & gm) << 6) | ((l & bm) << 9) | TA1.blend(TA0, l < am).andnot(l == GSVector4i::zero());
				((GSVector4i*)dst)[3] = ((h & rm) << 3) | ((h & gm) << 6) | ((h & bm) << 9) | TA1.blend(TA0, h < am).andnot(h == GSVector4i::zero());
			}
		}
		else
		{
			for(int i = 0; i < 8; i++, dst += dstpitch)
			{
				GSVector4i v0 = ((GSVector4i*)src)[i * 2 + 0];

				l = v0.upl16();
				h = v0.uph16();

				((GSVector4i*)dst)[0] = ((l & rm) << 3) | ((l & gm) << 6) | ((l & bm) << 9) | TA1.blend(TA0, l < am);
				((GSVector4i*)dst)[1] = ((h & rm) << 3) | ((h & gm) << 6) | ((h & bm) << 9) | TA1.blend(TA0, h < am);

				GSVector4i v1 = ((GSVector4i*)src)[i * 2 + 1];

				l = v1.upl16();
				h = v1.uph16();

				((GSVector4i*)dst)[2] = ((l & rm) << 3) | ((l & gm) << 6) | ((l & bm) << 9) | TA1.blend(TA0, l < am);
				((GSVector4i*)dst)[3] = ((h & rm) << 3) | ((h & gm) << 6) | ((h & bm) << 9) | TA1.blend(TA0, h < am);
			}
		}

		#else

		DWORD TA0 = TEXA.TA0 << 24;
		DWORD TA1 = TEXA.TA1 << 24;

		if(TEXA.AEM)
		{
			for(int j = 0; j < 8; j++, src += 16, dst += dstpitch)
			{
				for(int i = 0; i < 16; i++)
				{
					((DWORD*)dst)[i] = ((src[i] & 0x8000) ? TA1 : src[i] ? TA0 : 0) | ((src[i] & 0x7c00) << 9) | ((src[i] & 0x03e0) << 6) | ((src[i] & 0x001f) << 3);
				}
			}
		}
		else
		{
			for(int j = 0; j < 8; j++, src += 16, dst += dstpitch)
			{
				for(int i = 0; i < 16; i++)
				{
					((DWORD*)dst)[i] = ((src[i] & 0x8000) ? TA1 : TA0) | ((src[i] & 0x7c00) << 9) | ((src[i] & 0x03e0) << 6) | ((src[i] & 0x001f) << 3);
				}
			}
		}

		#endif
	}

	__forceinline static void ExpandBlock8_32(BYTE* src, BYTE* dst, int dstpitch, DWORD* pal)
	{
		for(int j = 0; j < 16; j++, dst += dstpitch)
		{
			#if _M_SSE >= 0x401

			GSVector4i* s = (GSVector4i*)src;
			GSVector4i* d = (GSVector4i*)dst;

			s[j].gather32_8(pal, d);

			#else

			for(int i = 0; i < 16; i++)
			{
				((DWORD*)dst)[i] = pal[src[j * 16 + i]];
			}

			#endif
		}
	}

	__forceinline static void ExpandBlock8_16(BYTE* src, BYTE* dst, int dstpitch, DWORD* pal)
	{
		for(int j = 0; j < 16; j++, dst += dstpitch)
		{
			#if _M_SSE >= 0x401

			GSVector4i* s = (GSVector4i*)src;
			GSVector4i* d = (GSVector4i*)dst;

			s[j].gather16_8(pal, d);

			#else

			for(int i = 0; i < 16; i++)
			{
				((WORD*)dst)[i] = (WORD)pal[src[j * 16 + i]];
			}

			#endif
		}
	}

	__forceinline static void ExpandBlock4_32(BYTE* src, BYTE* dst, int dstpitch, UINT64* pal)
	{
		for(int j = 0; j < 16; j++, dst += dstpitch)
		{
			#if _M_SSE >= 0x401

			GSVector4i* s = (GSVector4i*)src;
			GSVector4i* d = (GSVector4i*)dst;

			s[j].gather64_8(pal, d);

			#else

			for(int i = 0; i < 32 / 2; i++)
			{
				((UINT64*)dst)[i] = pal[src[j * 16 + i]];
			}

			#endif
		}
	}

	__forceinline static void ExpandBlock4_16(BYTE* src, BYTE* dst, int dstpitch, UINT64* pal)
	{
		for(int j = 0; j < 16; j++, dst += dstpitch)
		{
			#if _M_SSE >= 0x401

			GSVector4i* s = (GSVector4i*)src;
			GSVector4i* d = (GSVector4i*)dst;

			s[j].gather32_8(pal, d);

			#else

			for(int i = 0; i < 32 / 2; i++)
			{
				((DWORD*)dst)[i] = (DWORD)pal[src[j * 16 + i]];
			}

			#endif
		}
	}

	__forceinline static void ExpandBlock8H_32(DWORD* src, BYTE* dst, int dstpitch, DWORD* pal)
	{
		for(int j = 0; j < 8; j++, dst += dstpitch)
		{
			#if _M_SSE >= 0x401

			GSVector4i* s = (GSVector4i*)src;
			GSVector4i* d = (GSVector4i*)dst;

			d[0] = (s[j * 2 + 0] >> 24).gather32_32<>(pal);
			d[1] = (s[j * 2 + 1] >> 24).gather32_32<>(pal);

			#else

			for(int i = 0; i < 8; i++)
			{
				((DWORD*)dst)[i] = pal[src[j * 8 + i] >> 24];
			}

			#endif
		}
	}

	__forceinline static void ExpandBlock8H_16(DWORD* src, BYTE* dst, int dstpitch, DWORD* pal)
	{
		for(int j = 0; j < 8; j++, dst += dstpitch)
		{
			#if _M_SSE >= 0x401

			GSVector4i* s = (GSVector4i*)src;
			GSVector4i* d = (GSVector4i*)dst;

			GSVector4i v0 = (s[j * 2 + 0] >> 24).gather32_32<>(pal);
			GSVector4i v1 = (s[j * 2 + 1] >> 24).gather32_32<>(pal);

			d[0] = v0.pu32(v1);

			#else

			for(int i = 0; i < 8; i++)
			{
				((WORD*)dst)[i] = (WORD)pal[src[j * 8 + i] >> 24];
			}

			#endif
		}
	}

	__forceinline static void ExpandBlock4HL_32(DWORD* src, BYTE* dst, int dstpitch, DWORD* pal)
	{
		for(int j = 0; j < 8; j++, dst += dstpitch)
		{
			#if _M_SSE >= 0x401

			GSVector4i* s = (GSVector4i*)src;
			GSVector4i* d = (GSVector4i*)dst;

			d[0] = ((s[j * 2 + 0] >> 24) & 0xf).gather32_32<>(pal);
			d[1] = ((s[j * 2 + 1] >> 24) & 0xf).gather32_32<>(pal);

			#else

			for(int i = 0; i < 8; i++)
			{
				((DWORD*)dst)[i] = pal[(src[j * 8 + i] >> 24) & 0xf];
			}

			#endif
		}
	}

	__forceinline static void ExpandBlock4HL_16(DWORD* src, BYTE* dst, int dstpitch, DWORD* pal)
	{
		for(int j = 0; j < 8; j++, dst += dstpitch)
		{
			#if _M_SSE >= 0x401

			GSVector4i* s = (GSVector4i*)src;
			GSVector4i* d = (GSVector4i*)dst;

			GSVector4i v0 = ((s[j * 2 + 0] >> 24) & 0xf).gather32_32<>(pal);
			GSVector4i v1 = ((s[j * 2 + 1] >> 24) & 0xf).gather32_32<>(pal);

			d[0] = v0.pu32(v1);

			#else

			for(int i = 0; i < 8; i++)
			{
				((WORD*)dst)[i] = (WORD)pal[(src[j * 8 + i] >> 24) & 0xf];
			}

			#endif
		}
	}

	__forceinline static void ExpandBlock4HH_32(DWORD* src, BYTE* dst, int dstpitch, DWORD* pal)
	{
		for(int j = 0; j < 8; j++, dst += dstpitch)
		{
			#if _M_SSE >= 0x401

			GSVector4i* s = (GSVector4i*)src;
			GSVector4i* d = (GSVector4i*)dst;

			d[0] = (s[j * 2 + 0] >> 28).gather32_32<>(pal);
			d[1] = (s[j * 2 + 1] >> 28).gather32_32<>(pal);

			#else

			for(int i = 0; i < 8; i++)
			{
				((DWORD*)dst)[i] = pal[src[j * 8 + i] >> 28];
			}

			#endif
		}
		}

	__forceinline static void ExpandBlock4HH_16(DWORD* src, BYTE* dst, int dstpitch, DWORD* pal)
	{
		for(int j = 0; j < 8; j++, dst += dstpitch)
		{
			#if _M_SSE >= 0x401

			GSVector4i* s = (GSVector4i*)src;
			GSVector4i* d = (GSVector4i*)dst;

			GSVector4i v0 = (s[j * 2 + 0] >> 28).gather32_32<>(pal);
			GSVector4i v1 = (s[j * 2 + 1] >> 28).gather32_32<>(pal);

			d[0] = v0.pu32(v1);

			#else

			for(int i = 0; i < 8; i++)
			{
				((WORD*)dst)[i] = (WORD)pal[src[j * 8 + i] >> 28];
			}

			#endif
		}
	}

	__forceinline static void UnpackAndWriteBlock24(BYTE* src, int srcpitch, BYTE* dst)
	{
		#if _M_SSE >= 0x200

		GSVector4i mask(0x00ffffff);

		for(int i = 0; i < 4; i++, src += srcpitch * 2)
		{
			GSVector4i v4 = GSVector4i::loadu(src);
			GSVector4i v5 = GSVector4i::loadu(src + 16, src + srcpitch);
			GSVector4i v6 = GSVector4i::loadu(src + srcpitch + 8);

			GSVector4i v0 = v4.upl32(v4.srl<3>()).upl64(v4.srl<6>().upl32(v4.srl<9>()));

			v4 = v4.srl<12>(v5);

			GSVector4i v1 = v4.upl32(v4.srl<3>()).upl64(v4.srl<6>().upl32(v4.srl<9>()));

			v4 = v5.srl<8>(v6);

			GSVector4i v2 = v4.upl32(v4.srl<3>()).upl64(v4.srl<6>().upl32(v4.srl<9>()));

			v4 = v6.srl<4>();

			GSVector4i v3 = v4.upl32(v4.srl<3>()).upl64(v4.srl<6>().upl32(v4.srl<9>()));

			GSVector4i::sw64(v0, v2, v1, v3);

			// here blend is faster than blend8 because vc8 has a little problem optimizing register usage for pblendvb (3rd op must be xmm0)

			((GSVector4i*)dst)[i * 4 + 0] = ((GSVector4i*)dst)[i * 4 + 0].blend(v0, mask); 
			((GSVector4i*)dst)[i * 4 + 1] = ((GSVector4i*)dst)[i * 4 + 1].blend(v1, mask);
			((GSVector4i*)dst)[i * 4 + 2] = ((GSVector4i*)dst)[i * 4 + 2].blend(v2, mask);
			((GSVector4i*)dst)[i * 4 + 3] = ((GSVector4i*)dst)[i * 4 + 3].blend(v3, mask);
		}

		#else 

		const DWORD* d = &columnTable32[0][0];

		for(int j = 0, diff = srcpitch - 8 * 3; j < 8; j++, src += diff, d += 8)
		{
			for(int i = 0; i < 8; i++, src += 3)
			{
				((DWORD*)dst)[d[i]] = (((DWORD*)dst)[d[i]] & ~0x00ffffff) | (src[2] << 16) | (src[1] << 8) | src[0];
			}
		}

		#endif
	}

	__forceinline static void UnpackAndWriteBlock8H(BYTE* src, int srcpitch, BYTE* dst)
	{
		#if _M_SSE >= 0x301

		GSVector4i mask(0xff000000);

		GSVector4i mask0 = m_uw8hmask0;
		GSVector4i mask1 = m_uw8hmask1;
		GSVector4i mask2 = m_uw8hmask2;
		GSVector4i mask3 = m_uw8hmask3;

		for(int i = 0; i < 4; i++, src += srcpitch * 2)
		{
			GSVector4i v4 = GSVector4i::loadu(src, src + srcpitch);

			GSVector4i v0 = v4.shuffle8(mask0);
			GSVector4i v1 = v4.shuffle8(mask1);
			GSVector4i v2 = v4.shuffle8(mask2);
			GSVector4i v3 = v4.shuffle8(mask3);

			((GSVector4i*)dst)[i * 4 + 0] = ((GSVector4i*)dst)[i * 4 + 0].blend8(v0, mask);
			((GSVector4i*)dst)[i * 4 + 1] = ((GSVector4i*)dst)[i * 4 + 1].blend8(v1, mask);
			((GSVector4i*)dst)[i * 4 + 2] = ((GSVector4i*)dst)[i * 4 + 2].blend8(v2, mask);
			((GSVector4i*)dst)[i * 4 + 3] = ((GSVector4i*)dst)[i * 4 + 3].blend8(v3, mask);
		}

		#elif _M_SSE >= 0x200

		GSVector4i mask(0xff000000);

		for(int i = 0; i < 4; i++, src += srcpitch * 2)
		{
			GSVector4i v4 = GSVector4i::loadu(src, src + srcpitch);

			GSVector4i v5 = v4.upl8(v4);
			GSVector4i v6 = v4.uph8(v4);

			GSVector4i v0 = v5.upl16(v5);
			GSVector4i v1 = v5.uph16(v5);
			GSVector4i v2 = v6.upl16(v6);
			GSVector4i v3 = v6.uph16(v6);

			GSVector4i::sw64(v0, v2, v1, v3);

			((GSVector4i*)dst)[i * 4 + 0] = ((GSVector4i*)dst)[i * 4 + 0].blend8(v0, mask);
			((GSVector4i*)dst)[i * 4 + 1] = ((GSVector4i*)dst)[i * 4 + 1].blend8(v1, mask);
			((GSVector4i*)dst)[i * 4 + 2] = ((GSVector4i*)dst)[i * 4 + 2].blend8(v2, mask);
			((GSVector4i*)dst)[i * 4 + 3] = ((GSVector4i*)dst)[i * 4 + 3].blend8(v3, mask);
		}

		#else

		const DWORD* d = &columnTable32[0][0];

		for(int j = 0; j < 8; j++, src += srcpitch, dst += 8)
		{
			for(int i = 0; i < 8; i++)
			{
				((DWORD*)dst)[d[i]] = (((DWORD*)dst)[d[i]] & ~0xff000000) | (src[i] << 24);
			}
		}

		#endif
	}

	__forceinline static void UnpackAndWriteBlock4HL(BYTE* src, int srcpitch, BYTE* dst)
	{
		#if _M_SSE >= 0x301

		GSVector4i mask(0x0f0f0f0f);
		GSVector4i mask0 = m_uw8hmask0;
		GSVector4i mask1 = m_uw8hmask1;
		GSVector4i mask2 = m_uw8hmask2;
		GSVector4i mask3 = m_uw8hmask3;
		GSVector4i mask4(0x0f000000);

		for(int i = 0; i < 2; i++, src += srcpitch * 4)
		{
			GSVector4i v(
				*(DWORD*)&src[srcpitch * 0], 
				*(DWORD*)&src[srcpitch * 1], 
				*(DWORD*)&src[srcpitch * 2], 
				*(DWORD*)&src[srcpitch * 3]);

			GSVector4i lo = v & mask;
			GSVector4i hi = (v >> 4) & mask;

			{
				GSVector4i v4 = lo.upl8(hi);

				GSVector4i v0 = v4.shuffle8(mask0);
				GSVector4i v1 = v4.shuffle8(mask1);
				GSVector4i v2 = v4.shuffle8(mask2);
				GSVector4i v3 = v4.shuffle8(mask3);

				((GSVector4i*)dst)[i * 8 + 0] = ((GSVector4i*)dst)[i * 8 + 0].blend(v0, mask4);
				((GSVector4i*)dst)[i * 8 + 1] = ((GSVector4i*)dst)[i * 8 + 1].blend(v1, mask4);
				((GSVector4i*)dst)[i * 8 + 2] = ((GSVector4i*)dst)[i * 8 + 2].blend(v2, mask4);
				((GSVector4i*)dst)[i * 8 + 3] = ((GSVector4i*)dst)[i * 8 + 3].blend(v3, mask4);
			}

			{
				GSVector4i v4 = lo.uph8(hi);

				GSVector4i v0 = v4.shuffle8(mask0);
				GSVector4i v1 = v4.shuffle8(mask1);
				GSVector4i v2 = v4.shuffle8(mask2);
				GSVector4i v3 = v4.shuffle8(mask3);

				((GSVector4i*)dst)[i * 8 + 4] = ((GSVector4i*)dst)[i * 8 + 4].blend(v0, mask4);
				((GSVector4i*)dst)[i * 8 + 5] = ((GSVector4i*)dst)[i * 8 + 5].blend(v1, mask4);
				((GSVector4i*)dst)[i * 8 + 6] = ((GSVector4i*)dst)[i * 8 + 6].blend(v2, mask4);
				((GSVector4i*)dst)[i * 8 + 7] = ((GSVector4i*)dst)[i * 8 + 7].blend(v3, mask4);
			}
		}

		#elif _M_SSE >= 0x200
/*
		__declspec(align(16)) DWORD block[8 * 8];

		UnpackBlock4HL(src, srcpitch, block);

		WriteBlock32<true, 0x0f000000>(dst, (BYTE*)block, sizeof(block) / 8);
*/
		GSVector4i mask(0x0f0f0f0f);
		GSVector4i mask2(0x0f000000);

		for(int i = 0; i < 2; i++, src += srcpitch * 4)
		{
			GSVector4i v(
				*(DWORD*)&src[srcpitch * 0], 
				*(DWORD*)&src[srcpitch * 1], 
				*(DWORD*)&src[srcpitch * 2], 
				*(DWORD*)&src[srcpitch * 3]);

			GSVector4i lo = v & mask;
			GSVector4i hi = (v >> 4) & mask;

			{
				GSVector4i v4 = lo.upl8(hi);

				GSVector4i v5 = v4.upl8(v4);
				GSVector4i v6 = v4.uph8(v4);

				GSVector4i v0 = v5.upl16(v5);
				GSVector4i v1 = v5.uph16(v5);
				GSVector4i v2 = v6.upl16(v6);
				GSVector4i v3 = v6.uph16(v6);

				GSVector4i::sw64(v0, v2, v1, v3);

				((GSVector4i*)dst)[i * 8 + 0] = ((GSVector4i*)dst)[i * 8 + 0].blend(v0, mask2);
				((GSVector4i*)dst)[i * 8 + 1] = ((GSVector4i*)dst)[i * 8 + 1].blend(v1, mask2);
				((GSVector4i*)dst)[i * 8 + 2] = ((GSVector4i*)dst)[i * 8 + 2].blend(v2, mask2);
				((GSVector4i*)dst)[i * 8 + 3] = ((GSVector4i*)dst)[i * 8 + 3].blend(v3, mask2);
			}

			{
				GSVector4i v4 = lo.uph8(hi);

				GSVector4i v5 = v4.upl8(v4);
				GSVector4i v6 = v4.uph8(v4);

				GSVector4i v0 = v5.upl16(v5);
				GSVector4i v1 = v5.uph16(v5);
				GSVector4i v2 = v6.upl16(v6);
				GSVector4i v3 = v6.uph16(v6);

				GSVector4i::sw64(v0, v2, v1, v3);

				((GSVector4i*)dst)[i * 8 + 4] = ((GSVector4i*)dst)[i * 8 + 4].blend(v0, mask2);
				((GSVector4i*)dst)[i * 8 + 5] = ((GSVector4i*)dst)[i * 8 + 5].blend(v1, mask2);
				((GSVector4i*)dst)[i * 8 + 6] = ((GSVector4i*)dst)[i * 8 + 6].blend(v2, mask2);
				((GSVector4i*)dst)[i * 8 + 7] = ((GSVector4i*)dst)[i * 8 + 7].blend(v3, mask2);
			}
		}

		#else

		const DWORD* d = &columnTable32[0][0];

		for(int j = 0; j < 8; j++, d += 8, src += srcpitch)
		{
			for(int i = 0; i < 4; i++)
			{
				((DWORD*)dst)[d[i * 2 + 0]] = (((DWORD*)dst)[d[i * 2 + 0]] & ~0x0f000000) | ((src[i] & 0x0f) << 24);
				((DWORD*)dst)[d[i * 2 + 1]] = (((DWORD*)dst)[d[i * 2 + 1]] & ~0x0f000000) | ((src[i] & 0xf0) << 20);
			}
		}

		#endif
	}

	__forceinline static void UnpackAndWriteBlock4HH(BYTE* src, int srcpitch, BYTE* dst)
	{
		#if _M_SSE >= 0x301

		GSVector4i mask(0xf0f0f0f0);
		GSVector4i mask0 = m_uw8hmask0;
		GSVector4i mask1 = m_uw8hmask1;
		GSVector4i mask2 = m_uw8hmask2;
		GSVector4i mask3 = m_uw8hmask3;
		GSVector4i mask4(0xf0000000);

		for(int i = 0; i < 2; i++, src += srcpitch * 4)
		{
			GSVector4i v(
				*(DWORD*)&src[srcpitch * 0], 
				*(DWORD*)&src[srcpitch * 1], 
				*(DWORD*)&src[srcpitch * 2], 
				*(DWORD*)&src[srcpitch * 3]);

			GSVector4i lo = (v << 4) & mask;
			GSVector4i hi = v & mask;

			{
				GSVector4i v4 = lo.upl8(hi);

				GSVector4i v0 = v4.shuffle8(mask0);
				GSVector4i v1 = v4.shuffle8(mask1);
				GSVector4i v2 = v4.shuffle8(mask2);
				GSVector4i v3 = v4.shuffle8(mask3);

				((GSVector4i*)dst)[i * 8 + 0] = ((GSVector4i*)dst)[i * 8 + 0].blend(v0, mask4);
				((GSVector4i*)dst)[i * 8 + 1] = ((GSVector4i*)dst)[i * 8 + 1].blend(v1, mask4);
				((GSVector4i*)dst)[i * 8 + 2] = ((GSVector4i*)dst)[i * 8 + 2].blend(v2, mask4);
				((GSVector4i*)dst)[i * 8 + 3] = ((GSVector4i*)dst)[i * 8 + 3].blend(v3, mask4);
			}

			{
				GSVector4i v4 = lo.uph8(hi);

				GSVector4i v0 = v4.shuffle8(mask0);
				GSVector4i v1 = v4.shuffle8(mask1);
				GSVector4i v2 = v4.shuffle8(mask2);
				GSVector4i v3 = v4.shuffle8(mask3);

				((GSVector4i*)dst)[i * 8 + 4] = ((GSVector4i*)dst)[i * 8 + 4].blend(v0, mask4);
				((GSVector4i*)dst)[i * 8 + 5] = ((GSVector4i*)dst)[i * 8 + 5].blend(v1, mask4);
				((GSVector4i*)dst)[i * 8 + 6] = ((GSVector4i*)dst)[i * 8 + 6].blend(v2, mask4);
				((GSVector4i*)dst)[i * 8 + 7] = ((GSVector4i*)dst)[i * 8 + 7].blend(v3, mask4);
			}
		}

		#elif _M_SSE >= 0x200
/*
		__declspec(align(16)) DWORD block[8 * 8];

		UnpackBlock4HH(src, srcpitch, block);

		WriteBlock32<true, 0xf0000000>(dst, (BYTE*)block, sizeof(block) / 8);
*/
		GSVector4i mask(0xf0f0f0f0);
		GSVector4i mask2(0xf0000000);

		for(int i = 0; i < 2; i++, src += srcpitch * 4)
		{
			GSVector4i v(
				*(DWORD*)&src[srcpitch * 0], 
				*(DWORD*)&src[srcpitch * 1], 
				*(DWORD*)&src[srcpitch * 2], 
				*(DWORD*)&src[srcpitch * 3]);

			GSVector4i lo = (v << 4) & mask;
			GSVector4i hi = v & mask;

			{
				GSVector4i v4 = lo.upl8(hi);

				GSVector4i v5 = v4.upl8(v4);
				GSVector4i v6 = v4.uph8(v4);

				GSVector4i v0 = v5.upl16(v5);
				GSVector4i v1 = v5.uph16(v5);
				GSVector4i v2 = v6.upl16(v6);
				GSVector4i v3 = v6.uph16(v6);

				GSVector4i::sw64(v0, v2, v1, v3);

				((GSVector4i*)dst)[i * 8 + 0] = ((GSVector4i*)dst)[i * 8 + 0].blend(v0, mask2);
				((GSVector4i*)dst)[i * 8 + 1] = ((GSVector4i*)dst)[i * 8 + 1].blend(v1, mask2);
				((GSVector4i*)dst)[i * 8 + 2] = ((GSVector4i*)dst)[i * 8 + 2].blend(v2, mask2);
				((GSVector4i*)dst)[i * 8 + 3] = ((GSVector4i*)dst)[i * 8 + 3].blend(v3, mask2);
			}

			{
				GSVector4i v4 = lo.uph8(hi);

				GSVector4i v5 = v4.upl8(v4);
				GSVector4i v6 = v4.uph8(v4);

				GSVector4i v0 = v5.upl16(v5);
				GSVector4i v1 = v5.uph16(v5);
				GSVector4i v2 = v6.upl16(v6);
				GSVector4i v3 = v6.uph16(v6);

				GSVector4i::sw64(v0, v2, v1, v3);

				((GSVector4i*)dst)[i * 8 + 4] = ((GSVector4i*)dst)[i * 8 + 4].blend(v0, mask2);
				((GSVector4i*)dst)[i * 8 + 5] = ((GSVector4i*)dst)[i * 8 + 5].blend(v1, mask2);
				((GSVector4i*)dst)[i * 8 + 6] = ((GSVector4i*)dst)[i * 8 + 6].blend(v2, mask2);
				((GSVector4i*)dst)[i * 8 + 7] = ((GSVector4i*)dst)[i * 8 + 7].blend(v3, mask2);
			}
		}

		#else

		const DWORD* d = &columnTable32[0][0];

		for(int j = 0; j < 8; j++, d += 8, src += srcpitch)
		{
			for(int i = 0; i < 4; i++)
			{
				((DWORD*)dst)[d[i * 2 + 0]] = (((DWORD*)dst)[d[i * 2 + 0]] & ~0xf0000000) | ((src[i] & 0x0f) << 28);
				((DWORD*)dst)[d[i * 2 + 1]] = (((DWORD*)dst)[d[i * 2 + 1]] & ~0xf0000000) | ((src[i] & 0xf0) << 24);
			}
		}

		#endif
	}

	template<bool AEM> __forceinline static void ReadAndExpandBlock24(BYTE* src, BYTE* dst, int dstpitch, const GIFRegTEXA& TEXA)
	{
		#if _M_SSE >= 0x200

		GSVector4i TA0(TEXA.TA0 << 24);
		GSVector4i mask = m_rgbx;

		for(int i = 0; i < 4; i++, dst += dstpitch * 2)
		{
			GSVector4i v0 = ((GSVector4i*)src)[i * 4 + 0];
			GSVector4i v1 = ((GSVector4i*)src)[i * 4 + 1];
			GSVector4i v2 = ((GSVector4i*)src)[i * 4 + 2];
			GSVector4i v3 = ((GSVector4i*)src)[i * 4 + 3];

			GSVector4i::sw64(v0, v1, v2, v3);

			GSVector4i* d0 = (GSVector4i*)&dst[dstpitch * 0];
			GSVector4i* d1 = (GSVector4i*)&dst[dstpitch * 1];

			if(AEM)
			{
				d0[0] = (v0 & mask) | TA0.andnot(v0 == GSVector4i::zero()); // TA0 & (v0 != GSVector4i::zero())
				d0[1] = (v1 & mask) | TA0.andnot(v1 == GSVector4i::zero()); // TA0 & (v1 != GSVector4i::zero())
				d1[0] = (v2 & mask) | TA0.andnot(v2 == GSVector4i::zero()); // TA0 & (v2 != GSVector4i::zero())
				d1[1] = (v3 & mask) | TA0.andnot(v3 == GSVector4i::zero()); // TA0 & (v3 != GSVector4i::zero())
			}
			else
			{
				d0[0] = (v0 & mask) | TA0;
				d0[1] = (v1 & mask) | TA0;
				d1[0] = (v2 & mask) | TA0;
				d1[1] = (v3 & mask) | TA0;
			}
		}

		#else

		DWORD TA0 = TEXA.TA0 << 24;

		const DWORD* s = &columnTable32[0][0];

		for(int j = 0; j < 8; j++, s += 8, dst += dstpitch)
		{
			for(int i = 0; i < 8; i++)
			{
				DWORD c = ((DWORD*)src)[s[i]] & 0xffffff;

				if(AEM)
				{
					((DWORD*)dst)[i] = c | (c ? TA0 : 0);
				}
				else
				{
					((DWORD*)dst)[i] = c | TA0;
				}
			}
		}

		#endif
	}

	__forceinline static void ReadAndExpandBlock8_32(BYTE* src, BYTE* dst, int dstpitch, DWORD* pal)
	{
		#if _M_SSE >= 0x401

		GSVector4i v0, v1, v2, v3;

		GSVector4i mask = m_r8mask;

		for(int i = 0; i < 2; i++)
		{
			v0 = ((GSVector4i*)src)[i * 8 + 0].shuffle8(mask); 
			v1 = ((GSVector4i*)src)[i * 8 + 1].shuffle8(mask); 
			v2 = ((GSVector4i*)src)[i * 8 + 2].shuffle8(mask); 
			v3 = ((GSVector4i*)src)[i * 8 + 3].shuffle8(mask);

			GSVector4i::sw16(v0, v1, v2, v3);
			GSVector4i::sw32(v0, v1, v3, v2);

			v0.gather32_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;
			v3.gather32_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;
			v1.gather32_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;
			v2.gather32_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;

			v2 = ((GSVector4i*)src)[i * 8 + 4].shuffle8(mask); 
			v3 = ((GSVector4i*)src)[i * 8 + 5].shuffle8(mask); 
			v0 = ((GSVector4i*)src)[i * 8 + 6].shuffle8(mask); 
			v1 = ((GSVector4i*)src)[i * 8 + 7].shuffle8(mask);

			GSVector4i::sw16(v0, v1, v2, v3);
			GSVector4i::sw32(v0, v1, v3, v2);

			v0.gather32_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;
			v3.gather32_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;
			v1.gather32_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;
			v2.gather32_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;
		}

		#elif _M_SSE >= 0x200

		__declspec(align(16)) BYTE block[16 * 16];

		ReadBlock8<true>(src, (BYTE*)block, sizeof(block) / 16);

		ExpandBlock8_32(block, dst, dstpitch, pal);

		#else

		const DWORD* s = &columnTable8[0][0];

		for(int j = 0; j < 16; j++, s += 16, dst += dstpitch)
		{
			for(int i = 0; i < 16; i++)
			{
				((DWORD*)dst)[i] = pal[src[s[i]]];
			}
		}

		#endif
	}

	// TODO: ReadAndExpandBlock8_16

	__forceinline static void ReadAndExpandBlock4_32(BYTE* src, BYTE* dst, int dstpitch, UINT64* pal)
	{
		#if _M_SSE >= 0x401

		GSVector4i v0, v1, v2, v3;

		GSVector4i mask = m_r4mask;

		for(int i = 0; i < 2; i++)
		{
			v0 = ((GSVector4i*)src)[i * 8 + 0].xzyw(); 
			v1 = ((GSVector4i*)src)[i * 8 + 1].xzyw(); 
			v2 = ((GSVector4i*)src)[i * 8 + 2].xzyw(); 
			v3 = ((GSVector4i*)src)[i * 8 + 3].xzyw();

			GSVector4i::sw64(v0, v1, v2, v3);
			GSVector4i::sw4(v0, v2, v1, v3);
			GSVector4i::sw8(v0, v1, v2, v3);

			v0 = v0.shuffle8(mask);
			v1 = v1.shuffle8(mask);
			v2 = v2.shuffle8(mask);
			v3 = v3.shuffle8(mask);

			GSVector4i::sw16rh(v0, v1, v2, v3);

			v0.gather64_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;
			v1.gather64_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;
			v2.gather64_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;
			v3.gather64_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;

			v0 = ((GSVector4i*)src)[i * 8 + 4].xzyw(); 
			v1 = ((GSVector4i*)src)[i * 8 + 5].xzyw(); 
			v2 = ((GSVector4i*)src)[i * 8 + 6].xzyw(); 
			v3 = ((GSVector4i*)src)[i * 8 + 7].xzyw();

			GSVector4i::sw64(v0, v1, v2, v3);
			GSVector4i::sw4(v0, v2, v1, v3);
			GSVector4i::sw8(v0, v1, v2, v3);

			v0 = v0.shuffle8(mask);
			v1 = v1.shuffle8(mask);
			v2 = v2.shuffle8(mask);
			v3 = v3.shuffle8(mask);

			GSVector4i::sw16rl(v0, v1, v2, v3);

			v0.gather64_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;
			v1.gather64_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;
			v2.gather64_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;
			v3.gather64_8<>(pal, (GSVector4i*)dst);
			dst += dstpitch;
		}

		#elif _M_SSE >= 0x200

		__declspec(align(16)) BYTE block[(32 / 2) * 16];

		ReadBlock4<true>(src, (BYTE*)block, sizeof(block) / 16);

		ExpandBlock4_32(block, dst, dstpitch, pal);

		#else

		const DWORD* s = &columnTable4[0][0];

		for(int j = 0; j < 16; j++, s += 32, dst += dstpitch)
		{
			for(int i = 0; i < 16; i++)
			{
				DWORD a0 = s[i * 2 + 0];
				DWORD a1 = s[i * 2 + 1];

				BYTE c0 = (src[a0 >> 1] >> ((a0 & 1) << 2)) & 0x0f;
				BYTE c1 = (src[a1 >> 1] >> ((a1 & 1) << 2)) & 0x0f;

				((UINT64*)dst)[i] = pal[(c1 << 4) | c0];
			}
		}

		#endif
	}

	// TODO: ReadAndExpandBlock4_16

	__forceinline static void ReadAndExpandBlock8H_32(BYTE* src, BYTE* dst, int dstpitch, DWORD* pal)
	{
		#if _M_SSE >= 0x401

		GSVector4i v0, v1, v2, v3;

		for(int i = 0; i < 4; i++)
		{
			v0 = ((GSVector4i*)src)[i * 4 + 0]; 
			v1 = ((GSVector4i*)src)[i * 4 + 1]; 
			v2 = ((GSVector4i*)src)[i * 4 + 2]; 
			v3 = ((GSVector4i*)src)[i * 4 + 3];

			GSVector4i::sw64(v0, v1, v2, v3);

			(v0 >> 24).gather32_32<>(pal, (GSVector4i*)&dst[0]);
			(v1 >> 24).gather32_32<>(pal, (GSVector4i*)&dst[16]);
			
			dst += dstpitch;

			(v2 >> 24).gather32_32<>(pal, (GSVector4i*)&dst[0]);
			(v3 >> 24).gather32_32<>(pal, (GSVector4i*)&dst[16]);
			
			dst += dstpitch;
		}

		#elif _M_SSE >= 0x200

		__declspec(align(16)) DWORD block[8 * 8];

		ReadBlock32<true>(src, (BYTE*)block, sizeof(block) / 8);

		ExpandBlock8H_32(block, dst, dstpitch, pal);

		#else

		const DWORD* s = &columnTable32[0][0];

		for(int j = 0; j < 8; j++, s += 8, dst += dstpitch)
		{
			for(int i = 0; i < 8; i++)
			{
				((DWORD*)dst)[i] = pal[((DWORD*)src)[s[i]] >> 24];
			}
		}

		#endif
	}

	// TODO: ReadAndExpandBlock8H_16

	__forceinline static void ReadAndExpandBlock4HL_32(BYTE* src, BYTE* dst, int dstpitch, DWORD* pal)
	{
		#if _M_SSE >= 0x401

		GSVector4i v0, v1, v2, v3;

		for(int i = 0; i < 4; i++)
		{
			v0 = ((GSVector4i*)src)[i * 4 + 0]; 
			v1 = ((GSVector4i*)src)[i * 4 + 1]; 
			v2 = ((GSVector4i*)src)[i * 4 + 2]; 
			v3 = ((GSVector4i*)src)[i * 4 + 3];

			GSVector4i::sw64(v0, v1, v2, v3);

			((v0 >> 24) & 0xf).gather32_32<>(pal, (GSVector4i*)&dst[0]);
			((v1 >> 24) & 0xf).gather32_32<>(pal, (GSVector4i*)&dst[16]);
			
			dst += dstpitch;

			((v2 >> 24) & 0xf).gather32_32<>(pal, (GSVector4i*)&dst[0]);
			((v3 >> 24) & 0xf).gather32_32<>(pal, (GSVector4i*)&dst[16]);
			
			dst += dstpitch;
		}

		#elif _M_SSE >= 0x200

		__declspec(align(16)) DWORD block[8 * 8];

		ReadBlock32<true>(src, (BYTE*)block, sizeof(block) / 8);

		ExpandBlock4HL_32(block, dst, dstpitch, pal);

		#else

		const DWORD* s = &columnTable32[0][0];

		for(int j = 0; j < 8; j++, s += 8, dst += dstpitch)
		{
			for(int i = 0; i < 8; i++)
			{
				((DWORD*)dst)[i] = pal[(((DWORD*)src)[s[i]] >> 24) & 0xf];
			}
		}

		#endif
	}

	// TODO: ReadAndExpandBlock4HL_16
	
	__forceinline static void ReadAndExpandBlock4HH_32(BYTE* src, BYTE* dst, int dstpitch, DWORD* pal)
	{
		#if _M_SSE >= 0x401

		GSVector4i v0, v1, v2, v3;

		for(int i = 0; i < 4; i++)
		{
			v0 = ((GSVector4i*)src)[i * 4 + 0]; 
			v1 = ((GSVector4i*)src)[i * 4 + 1]; 
			v2 = ((GSVector4i*)src)[i * 4 + 2]; 
			v3 = ((GSVector4i*)src)[i * 4 + 3];

			GSVector4i::sw64(v0, v1, v2, v3);

			(v0 >> 28).gather32_32<>(pal, (GSVector4i*)&dst[0]);
			(v1 >> 28).gather32_32<>(pal, (GSVector4i*)&dst[16]);
			
			dst += dstpitch;

			(v2 >> 28).gather32_32<>(pal, (GSVector4i*)&dst[0]);
			(v3 >> 28).gather32_32<>(pal, (GSVector4i*)&dst[16]);
			
			dst += dstpitch;
		}

		#elif _M_SSE >= 0x200

		__declspec(align(16)) DWORD block[8 * 8];

		ReadBlock32<true>(src, (BYTE*)block, sizeof(block) / 8);

		ExpandBlock4HH_32(block, dst, dstpitch, pal);

		#else

		const DWORD* s = &columnTable32[0][0];

		for(int j = 0; j < 8; j++, s += 8, dst += dstpitch)
		{
			for(int i = 0; i < 8; i++)
			{
				((DWORD*)dst)[i] = pal[((DWORD*)src)[s[i]] >> 28];
			}
		}

		#endif
	}

	// TODO: ReadAndExpandBlock4HH_16

	//

	static void WriteCLUT_T32_I8_CSM1(DWORD* src, WORD* clut)
	{
		#if _M_SSE >= 0x200

		for(int i = 0; i < 64; i += 16)
		{
			WriteCLUT_T32_I4_CSM1(&src[i +   0], &clut[i * 2 +   0]);
			WriteCLUT_T32_I4_CSM1(&src[i +  64], &clut[i * 2 +  16]);
			WriteCLUT_T32_I4_CSM1(&src[i + 128], &clut[i * 2 + 128]);
			WriteCLUT_T32_I4_CSM1(&src[i + 192], &clut[i * 2 + 144]);
		}

		#else

		for(int j = 0; j < 2; j++, src += 128, clut += 128)
		{
			for(int i = 0; i < 128; i++) 
			{
				DWORD c = src[clutTableT32I8[i]];
				clut[i] = (WORD)(c & 0xffff);
				clut[i + 256] = (WORD)(c >> 16);
			}
		}

		#endif
	}

	__forceinline static void WriteCLUT_T32_I4_CSM1(DWORD* src, WORD* clut)
	{
		#if _M_SSE >= 0x200

		GSVector4i* s = (GSVector4i*)src;
		GSVector4i* d = (GSVector4i*)clut;

		GSVector4i v0 = s[0];
		GSVector4i v1 = s[1];
		GSVector4i v2 = s[2];
		GSVector4i v3 = s[3];

		GSVector4i::sw64(v0, v1, v2, v3);
		GSVector4i::sw16(v0, v1, v2, v3);
		GSVector4i::sw16(v0, v2, v1, v3);
		GSVector4i::sw16(v0, v1, v2, v3);

		d[0] = v0;
		d[1] = v1;
		d[32] = v2;
		d[33] = v3;

		#else

		for(int i = 0; i < 16; i++) 
		{
			DWORD c = src[clutTableT32I4[i]];
			clut[i] = (WORD)(c & 0xffff);
			clut[i + 256] = (WORD)(c >> 16);
		}

		#endif
	}

	static void WriteCLUT_T16_I8_CSM1(WORD* src, WORD* clut)
	{
		#if _M_SSE >= 0x200

		GSVector4i* s = (GSVector4i*)src;
		GSVector4i* d = (GSVector4i*)clut;

		for(int i = 0; i < 32; i += 4)
		{
			GSVector4i v0 = s[i + 0];
			GSVector4i v1 = s[i + 1];
			GSVector4i v2 = s[i + 2];
			GSVector4i v3 = s[i + 3];

			GSVector4i::sw16(v0, v1, v2, v3);
			GSVector4i::sw32(v0, v1, v2, v3);
			GSVector4i::sw16(v0, v2, v1, v3);

			d[i + 0] = v0;
			d[i + 1] = v2;
			d[i + 2] = v1;
			d[i + 3] = v3;
		}

		#else

		for(int j = 0; j < 8; j++, src += 32, clut += 32) 
		{
			for(int i = 0; i < 32; i++)
			{
				clut[i] = src[clutTableT16I8[i]];
			}
		}

		#endif
	}

	__forceinline static void WriteCLUT_T16_I4_CSM1(WORD* src, WORD* clut)
	{
		for(int i = 0; i < 16; i++) 
		{
			clut[i] = src[clutTableT16I4[i]];
		}
	}

	static void ReadCLUT_T32_I8(WORD* clut, DWORD* dst)
	{
		#if _M_SSE >= 0x200

		for(int i = 0; i < 256; i += 16)
		{
			ReadCLUT_T32_I4(&clut[i], &dst[i]);
		}

		#else 

		for(int i = 0; i < 256; i++)
		{
			dst[i] = ((DWORD)clut[i + 256] << 16) | clut[i];
		}

		#endif
	}

	__forceinline static void ReadCLUT_T32_I4(WORD* clut, DWORD* dst)
	{
		#if _M_SSE >= 0x200

		GSVector4i* s = (GSVector4i*)clut;
		GSVector4i* d = (GSVector4i*)dst;

		GSVector4i v0 = s[0];
		GSVector4i v1 = s[1];
		GSVector4i v2 = s[32];
		GSVector4i v3 = s[33];

		GSVector4i::sw16(v0, v2, v1, v3);

		d[0] = v0;
		d[1] = v1;
		d[2] = v2;
		d[3] = v3;

		#else 

		for(int i = 0; i < 16; i++)
		{
			dst[i] = ((DWORD)clut[i + 256] << 16) | clut[i];
		}

		#endif
	}

	static void ReadCLUT_T16_I8(WORD* clut, DWORD* dst)
	{
		#if _M_SSE >= 0x200

		for(int i = 0; i < 256; i += 16)
		{
			ReadCLUT_T16_I4(&clut[i], &dst[i]);
		}

		#else 

		for(int i = 0; i < 256; i++)
		{
			dst[i] = (DWORD)clut[i];
		}

		#endif
	}

	__forceinline static void ReadCLUT_T16_I4(WORD* clut, DWORD* dst)
	{
		#if _M_SSE >= 0x200

		GSVector4i* s = (GSVector4i*)clut;
		GSVector4i* d = (GSVector4i*)dst;

		GSVector4i r0 = s[0];
		GSVector4i r1 = s[1];

		d[0] = r0.upl16();
		d[1] = r0.uph16();
		d[2] = r1.upl16();
		d[3] = r1.uph16();

		#else 

		for(int i = 0; i < 16; i++)
		{
			dst[i] = (DWORD)clut[i];
		}

		#endif
	}
};
