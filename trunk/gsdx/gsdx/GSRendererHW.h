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

#include "GSRenderer.h"
#include "GSTextureCache.h"
#include "GSCrc.h"

template<class Device, class Vertex> 
class GSRendererHW : public GSRendererT<Device, Vertex>
{
protected:
	GSTextureCache<Device>* m_tc; // derived class creates this
	int m_width;
	int m_height;
	int m_skip;
	bool m_reset;

	void Reset() 
	{
		// TODO: GSreset can come from the main thread too => crash
		// m_tc->RemoveAll();

		m_reset = true;

		__super::Reset();
	}

	void MinMaxUV(GSVector4& mm)
	{
		if(PRIM->FST)
		{
			#if _M_SSE >= 0x200

			__m128 min = _mm_set1_ps(+1e10);
			__m128 max = _mm_set1_ps(-1e10);

			int i = 0;

			for(int count = m_count - 3; i < count; i += 4)
			{
				__m128 min0 = _mm_min_ps(m_vertices[i+0].m128[0], min);
				__m128 max0 = _mm_max_ps(m_vertices[i+0].m128[0], max);
				__m128 min1 = _mm_min_ps(m_vertices[i+1].m128[0], min);
				__m128 max1 = _mm_max_ps(m_vertices[i+1].m128[0], max);
				__m128 min2 = _mm_min_ps(m_vertices[i+2].m128[0], min);
				__m128 max2 = _mm_max_ps(m_vertices[i+2].m128[0], max);
				__m128 min3 = _mm_min_ps(m_vertices[i+3].m128[0], min);
				__m128 max3 = _mm_max_ps(m_vertices[i+3].m128[0], max);

				min0 = _mm_min_ps(min0, min1);
				max0 = _mm_max_ps(max0, max1);
				min2 = _mm_min_ps(min2, min3);
				max2 = _mm_max_ps(max2, max3);

				min = _mm_min_ps(min0, min2);
				max = _mm_max_ps(max0, max2);
			}

			for(; i < m_count; i++)
			{
				min = _mm_min_ps(m_vertices[i+0].m128[0], min);
				max = _mm_max_ps(m_vertices[i+0].m128[0], max);
			}

			mm.x = ((float*)&min)[0];
			mm.y = ((float*)&min)[1];
			mm.z = ((float*)&max)[0];
			mm.w = ((float*)&max)[1];

			#else

			for(int i = 0, j = m_count; i < j; i++)
			{
				float x = m_vertices[i].t.x;

				if(x < mm.x) mm.x = x;
				if(x > mm.z) mm.z = x;
				
				float y = m_vertices[i].t.y;

				if(y < mm.y) mm.y = y;
				if(y > mm.w) mm.w = y;
			}

			#endif

			mm.x *= 1.0f / (16 << m_context->TEX0.TW);
			mm.y *= 1.0f / (16 << m_context->TEX0.TH);
			mm.z *= 1.0f / (16 << m_context->TEX0.TW);
			mm.w *= 1.0f / (16 << m_context->TEX0.TH);
		}
		else
		{
			// TODO: sse

			mm.x = mm.y = +1e10;
			mm.z = mm.w = -1e10;

			for(int i = 0, j = m_count; i < j; i++)
			{
				float w = 1.0f / m_vertices[i].GetQ();

				float x = m_vertices[i].t.x * w;

				if(x < mm.x) mm.x = x;
				if(x > mm.z) mm.z = x;
				
				float y = m_vertices[i].t.y * w;

				if(y < mm.y) mm.y = y;
				if(y > mm.w) mm.w = y;
			}
		}
	}

	void MinMaxUV(int w, int h, CRect& r)
	{
		r.SetRect(0, 0, w, h);

		GSVector4 mm(0, 0, 1, 1);

		if((m_context->CLAMP.WMS < 3 || m_context->CLAMP.WMT < 3) && m_count < 100)
		{
			MinMaxUV(mm);
		}

		CSize bs = GSLocalMemory::m_psm[m_context->TEX0.PSM].bs;
		CSize bsm(bs.cx - 1, bs.cy - 1);

		if(m_context->CLAMP.WMS != 3)
		{
			if(m_context->CLAMP.WMS == 0)
			{
				float fmin = floor(mm.x);
				float fmax = floor(mm.z);

				if(fmin != fmax) {mm.x = 0; mm.z = 1.0f;}
				else {mm.x -= fmin; mm.z -= fmax;}

				// FIXME: 
				if(mm.x == 0 && mm.z != 1.0f) mm.z = 1.0f;
			}
			else if(m_context->CLAMP.WMS == 1)
			{
				if(mm.x < 0) mm.x = 0;
				else if(mm.x > 1.0f) mm.x = 1.0f;
				if(mm.z < 0) mm.z = 0;
				else if(mm.z > 1.0f) mm.z = 1.0f;
				if(mm.x > mm.z) mm.x = mm.z;
			}
			else if(m_context->CLAMP.WMS == 2)
			{
				float minu = 1.0f * m_context->CLAMP.MINU / w;
				float maxu = 1.0f * m_context->CLAMP.MAXU / w;
				if(mm.x < minu) mm.x = minu;
				else if(mm.x > maxu) mm.x = maxu;
				if(mm.z < minu) mm.z = minu;
				else if(mm.z > maxu) mm.z = maxu;
				if(mm.x > mm.z) mm.x = mm.z;
			}

			r.left = (int)(mm.x * w);
			r.right = (int)(mm.z * w);
		}
		else
		{
			if(m_psrr)
			{
				r.left = (int)(m_context->CLAMP.MAXU);
				r.right = (int)(r.left + (m_context->CLAMP.MINU + 1));
			}
			else
			{
				r.left = 0;
				r.right = w;
			}
		}

		if(m_context->CLAMP.WMT != 3)
		{
			if(m_context->CLAMP.WMT == 0)
			{
				float fmin = floor(mm.y);
				float fmax = floor(mm.w);

				if(fmin != fmax) {mm.y = 0; mm.w = 1.0f;}
				else {mm.y -= fmin; mm.w -= fmax;}

				// FIXME: 
				if(mm.y == 0 && mm.w != 1.0f) mm.w = 1.0f;
			}
			else if(m_context->CLAMP.WMT == 1)
			{
				if(mm.y < 0) mm.y = 0;
				else if(mm.y > 1.0f) mm.y = 1.0f;
				if(mm.w < 0) mm.w = 0;
				else if(mm.w > 1.0f) mm.w = 1.0f;
				if(mm.y > mm.w) mm.y = mm.w;
			}
			else if(m_context->CLAMP.WMT == 2)
			{
				float minv = 1.0f * m_context->CLAMP.MINV / h;
				float maxv = 1.0f * m_context->CLAMP.MAXV / h;
				if(mm.y < minv) mm.y = minv;
				else if(mm.y > maxv) mm.y = maxv;
				if(mm.w < minv) mm.w = minv;
				else if(mm.w > maxv) mm.w = maxv;
				if(mm.y > mm.w) mm.y = mm.w;
			}

			r.top = (int)(mm.y * h);
			r.bottom = (int)(mm.w * h);
		}
		else
		{
			if(m_psrr)
			{
				r.top = (int)(m_context->CLAMP.MAXV);
				r.bottom = (int)(r.top + (m_context->CLAMP.MINV + 1));
			}
			else
			{
				r.top = 0;
				r.bottom = h;
			}
		}

		r.InflateRect(1, 1); // one more pixel because of bilinear filtering

		r.left = max(r.left & ~bsm.cx, 0);
		r.right = min((r.right + bsm.cx) & ~bsm.cx, w);

		r.top = max(r.top & ~bsm.cy, 0);
		r.bottom = min((r.bottom + bsm.cy) & ~bsm.cy, h);
	}

	void VSync(int field)
	{
		__super::VSync(field);

		m_tc->IncAge();

		m_skip = 0;

		if(m_reset)
		{
			m_tc->RemoveAll();

			m_reset = false;
		}
	}

	void ResetDevice() 
	{
		m_tc->RemoveAll();
	}

	bool GetOutput(int i, Texture& t)
	{
		GIFRegTEX0 TEX0;

		TEX0.TBP0 = DISPFB[i]->Block();
		TEX0.TBW = DISPFB[i]->FBW;
		TEX0.PSM = DISPFB[i]->PSM;

		TRACE(_T("[%d] GetOutput %d %05x (%d)\n"), (int)m_perfmon.GetFrame(), i, (int)TEX0.TBP0, (int)TEX0.PSM);

		if(GSTextureCache<Device>::GSRenderTarget* rt = m_tc->GetRenderTarget(TEX0, m_width, m_height, true))
		{
			t = rt->m_texture;

			if(s_dump)
			{
				CString str;
				str.Format(_T("c:\\temp2\\_%05d_f%I64d_fr%d_%05x_%d.bmp"), s_n++, m_perfmon.GetFrame(), i, (int)TEX0.TBP0, (int)TEX0.PSM);
				if(s_save) rt->m_texture.Save(str);
			}

			return true;
		}

		return false;
	}

	void InvalidateVideoMem(const GIFRegBITBLTBUF& BITBLTBUF, CRect r)
	{
		TRACE(_T("[%d] InvalidateVideoMem %d,%d - %d,%d %05x (%d)\n"), (int)m_perfmon.GetFrame(), r.left, r.top, r.right, r.bottom, (int)BITBLTBUF.DBP, (int)BITBLTBUF.DPSM);

		m_tc->InvalidateVideoMem(BITBLTBUF, r);
	}

	void InvalidateLocalMem(const GIFRegBITBLTBUF& BITBLTBUF, CRect r)
	{
		TRACE(_T("[%d] InvalidateLocalMem %d,%d - %d,%d %05x (%d)\n"), (int)m_perfmon.GetFrame(), r.left, r.top, r.right, r.bottom, (int)BITBLTBUF.SBP, (int)BITBLTBUF.SPSM);

		m_tc->InvalidateLocalMem(BITBLTBUF, r);
	}

	virtual bool OverrideInput(int& prim, Texture& rt, Texture& ds, Texture* t)
	{
		#pragma region ffxii pal video conversion

		if(m_crc == CRC::FFXII_EU1 || m_crc == CRC::FFXII_EU2 || m_crc == CRC::FFXII_EU3 || m_crc == CRC::FFXII_EU4)
		{
			static DWORD* video = NULL;
			static bool ok = false;

			if(prim == GS_POINTLIST && m_count >= 448*448 && m_count <= 448*512)
			{
				// incoming pixels are stored in columns, one column is 16x512, total res 448x512 or 448x454

				if(!video) video = new DWORD[512*512];

				for(int x = 0, i = 0, rows = m_count / 448; x < 448; x += 16)
				{
					DWORD* dst = &video[x];

					for(int y = 0; y < rows; y++, dst += 512)
					{
						for(int j = 0; j < 16; j++, i++)
						{
							dst[j] = m_vertices[i].c0;
						}
					}
				}

				ok = true;

				return false;
			}
			else if(prim == GS_LINELIST && m_count == 512*2 && ok)
			{
				// normally, this step would copy the video onto screen with 512 texture mapped horizontal lines,
				// but we use the stored video data to create a new texture, and replace the lines with two triangles

				ok = false;

				m_dev.CreateTexture(*t, 512, 512);

				t->Update(CRect(0, 0, 448, 512), video, 512*4);

				m_vertices[0] = m_vertices[0];
				m_vertices[1] = m_vertices[1];
				m_vertices[2] = m_vertices[m_count - 2];
				m_vertices[3] = m_vertices[1];
				m_vertices[4] = m_vertices[2];
				m_vertices[5] = m_vertices[m_count - 1];

				prim = GS_TRIANGLELIST;
				m_count = 6;

				return true;
			}
		}

		#pragma endregion

		#pragma region ffx random battle transition (z buffer written directly, clear it now)

		if(m_ffx)
		{
			DWORD FBP = m_context->FRAME.Block();
			DWORD ZBP = m_context->ZBUF.Block();
			DWORD TBP = m_context->TEX0.TBP0;

			if((FBP == 0x00d00 || FBP == 0x00000) && ZBP == 0x02100 && PRIM->TME && TBP == 0x01a00 && m_context->TEX0.PSM == PSM_PSMCT16S)
			{
				m_dev.ClearDepth(ds, 0);
			}

			return true;
		}

		#pragma endregion

		#pragma region metal slug missing red channel fix
		
		if(m_crc == CRC::MetalSlug6)
		{
			for(int i = 0, j = m_count; i < j; i++)
			{
				if(m_vertices[i].r == 0 && m_vertices[i].g != 0 && m_vertices[i].b != 0)
				{
					m_vertices[i].r = (m_vertices[i].g + m_vertices[i].b) / 2;
				}
			}

			return true;
		}

		#pragma endregion

		#pragma region tomoyo after, clannad (palette uploaded in a point list, pure genius...)

		if(m_crc == CRC::TomoyoAfter_JP || m_crc == CRC::Clannad_JP)
		{
			if(prim == GS_POINTLIST && !PRIM->TME)
			{
				DWORD bp = m_context->FRAME.Block();
				DWORD bw = m_context->FRAME.FBW;

				if(bp >= 0x03f40 && (bp & 0x1f) == 0)
				{
					if(m_count == 16)
					{
						for(int i = 0; i < 16; i++)
						{
							m_vertices[i].a = m_vertices[i].a >= 0x80 ? 0xff : m_vertices[i].a * 2;

							m_mem.WritePixel32(i & 7, i >> 3, m_vertices[i].c0, bp, bw);
						}

						m_mem.InvalidateCLUT();

						return false;
					}
					else if(m_count == 256)
					{
						for(int i = 0; i < 256; i++)
						{
							m_vertices[i].a = m_vertices[i].a >= 0x80 ? 0xff : m_vertices[i].a * 2;

							m_mem.WritePixel32(i & 15, i >> 4, m_vertices[i].c0, bp, bw);
						}

						m_mem.InvalidateCLUT();

						return false;
					}
					else
					{
						ASSERT(0);
					}
				}
			}

			return true;
		}

		#pragma endregion

		return true;
	}

	virtual void OverrideOutput()
	{
		#pragma region dbzbt2 palette readback (cannot detect yet, when fetching the texture later)

		if(m_crc == CRC::DBZBT2_US || m_crc == CRC::DBZBT2_EU)
		{
			DWORD FBP = m_context->FRAME.Block();
			DWORD TBP0 = m_context->TEX0.TBP0;

			if(PRIM->TME && (FBP == 0x03c00 && TBP0 == 0x03c80 || FBP == 0x03ac0 && TBP0 == 0x03b40))
			{
				GIFRegBITBLTBUF BITBLTBUF;

				BITBLTBUF.SBP = FBP;
				BITBLTBUF.SBW = 1;
				BITBLTBUF.SPSM = PSM_PSMCT32;

				InvalidateLocalMem(BITBLTBUF, CRect(0, 0, 64, 64));
			}
		}

		#pragma endregion
	}

	bool CanUpscale()
	{
		#pragma region dbzbt2 palette should stay 64 x 64

		if(m_crc == CRC::DBZBT2_US || m_crc == CRC::DBZBT2_EU)
		{
			DWORD FBP = m_context->FRAME.Block();

			if(FBP == 0x03c00 || FBP == 0x03ac0)
			{
				return false;
			}
		}

		#pragma endregion

		return __super::CanUpscale();
	}

public:
	GSRendererHW(BYTE* base, bool mt, void (*irq)(), int nloophack, const GSRendererSettings& rs, bool psrr)
		: GSRendererT<Device, Vertex>(base, mt, irq, nloophack, rs, psrr)
		, m_tc(NULL)
		, m_width(1024)
		, m_height(1024)
		, m_skip(0)
		, m_reset(false)
	{
		if(!m_nativeres)
		{
			m_width = AfxGetApp()->GetProfileInt(_T("Settings"), _T("resx"), 1024);
			m_height = AfxGetApp()->GetProfileInt(_T("Settings"), _T("resy"), 1024);
		}
	}

	virtual ~GSRendererHW()
	{
		delete m_tc;
	}

	void SetGameCRC(DWORD crc, int options)
	{
		__super::SetGameCRC(crc, options);

		if(crc == CRC::JackieChanAdv)
		{
			m_width = 1280;
		}
	}
};
