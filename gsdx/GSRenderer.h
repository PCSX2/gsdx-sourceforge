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

#include "GSWnd.h"
#include "GSState.h"
#include "GSVertexList.h"
#include "GSSetting.h"

extern GSSetting g_interlace[7];
extern GSSetting g_aspectratio[3];

class GSRenderer :  public GSWnd, public GSState
{
protected:
	int m_interlace;
	int m_aspectratio;
	int m_filter;
	bool m_vsync;
	bool m_osd;
	int m_field;

public:
	GSRenderer(BYTE* base, bool mt, void (*irq)(), int nloophack, int interlace, int aspectratio, int filter, bool vsync)
		: GSState(base, mt, irq, nloophack)
		, m_interlace(interlace)
		, m_aspectratio(aspectratio)
		, m_filter(filter)
		, m_vsync(vsync)
		, m_osd(true)
		, m_field(0)
	{
	};

	virtual void VSync(int field) = 0;
	virtual bool MakeSnapshot(LPCTSTR path) = 0;
};

template<class Device, class Vertex> class GSRendererT : public GSRenderer
{
protected:
	typedef typename Device::Texture Texture;

	Vertex* m_vertices;
	int m_count;
	int m_maxcount;
	GSVertexList<Vertex> m_vl;

	void Reset()
	{
		m_count = 0;
		m_vl.RemoveAll();

		__super::Reset();
	}

	void VertexKick(bool skip)
	{
		while(m_vl.GetCount() >= primVertexCount[PRIM->PRIM])
		{
			if(m_count + 6 > m_maxcount)
			{
				m_maxcount = max(10000, m_maxcount * 3/2);

				Vertex* vertices = (Vertex*)_aligned_malloc(sizeof(Vertex) * m_maxcount, 16);

				if(m_vertices)
				{
					memcpy(vertices, m_vertices, sizeof(Vertex) * m_count);

					_aligned_free(m_vertices);
				}

				m_vertices = vertices;
			}

			DrawingKick(skip);
		}
	}

	void ResetPrim()
	{
		m_vl.RemoveAll();
	}

	void FlushPrim() 
	{
		if(m_count > 0)
		{
			TRACE(_T("[%d] Draw f %05x (%d) z %05x (%d %d %d %d) t %05x %05x (%d)\n"), 
				  (int)m_perfmon.GetFrame(), 
				  (int)m_context->FRAME.Block(), 
				  (int)m_context->FRAME.PSM, 
				  (int)m_context->ZBUF.Block(), 
				  (int)m_context->ZBUF.PSM, 
				  m_context->TEST.ZTE, 
				  m_context->TEST.ZTST, 
				  m_context->ZBUF.ZMSK, 
				  PRIM->TME ? (int)m_context->TEX0.TBP0 : 0xfffff, 
				  PRIM->TME && m_context->TEX0.PSM > PSM_PSMCT16S ? (int)m_context->TEX0.CBP : 0xfffff, 
				  PRIM->TME ? (int)m_context->TEX0.PSM : 0xff);

			Draw();

			m_count = 0;
		}
	}

	virtual void DrawingKick(bool skip) = 0;
	virtual void Draw() = 0;
	virtual void ResetDevice() {}
	virtual bool GetOutput(int i, Texture& t, GSVector2& s) = 0;

	void ProcessWindowMessages()
	{
		MSG msg;

		memset(&msg, 0, sizeof(msg));

		while(msg.message != WM_QUIT && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_KEYDOWN)
			{
				int step = (::GetAsyncKeyState(VK_SHIFT) & 0x80000000) ? -1 : 1;

				if(msg.wParam == VK_F5)
				{
					m_interlace = (m_interlace + 7 + step) % 7;
					continue;
				}

				if(msg.wParam == VK_F6)
				{
					m_aspectratio = (m_aspectratio + 3 + step) % 3;
					continue;
				}			

				if(msg.wParam == VK_F7)
				{
					SetWindowText(_T("PCSX2"));
					m_osd = !m_osd;
					continue;
				}
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	bool Merge()
	{
		CSize fs(0, 0);
		CSize ds(0, 0);

		Texture st[2];
		GSVector4 sr[2];
		GSVector2 ss[2];

		for(int i = 0; i < 2; i++)
		{
			if(IsEnabled(i) && GetOutput(i, st[i], ss[i]))
			{
				CSize s = GetFrameSize(i);

				s.cx = (int)(ss[i].x * s.cx);
				s.cy = (int)(ss[i].y * s.cy);

				ASSERT(fs.cx == 0 || fs.cx == s.cx);
				ASSERT(fs.cy == 0 || fs.cy == s.cy || fs.cy + 1 == s.cy);

				fs.cx = s.cx;
				fs.cy = s.cy;

				if(SMODE2->INT && SMODE2->FFMD) s.cy *= 2;

				ASSERT(ds.cx == 0 || ds.cx == s.cx);
				ASSERT(ds.cy == 0 || ds.cy == s.cy || ds.cy + 1 == s.cy);

				ds.cx = s.cx;
				ds.cy = s.cy;

				CRect r = GetFrameRect(i);

				sr[i].x = ss[i].x * r.left / st[i].GetWidth();
				sr[i].y = ss[i].y * r.top / st[i].GetHeight();
				sr[i].z = ss[i].x * r.right / st[i].GetWidth();
				sr[i].w = ss[i].y * r.bottom / st[i].GetHeight();
			}
		}

		bool en1 = IsEnabled(0);
		bool en2 = IsEnabled(1);
		bool slbg = PMODE->SLBG;
		bool mmod = PMODE->MMOD;

		GSVector4 c;

		c.r = (float)BGCOLOR->R / 255;
		c.g = (float)BGCOLOR->G / 255;
		c.b = (float)BGCOLOR->B / 255;
		c.a = (float)PMODE->ALP / 255;

		m_dev.Merge(st, sr, fs, en1, en2, slbg, mmod, c);

		if(SMODE2->INT && m_interlace > 0)
		{
			int field = 1 - ((m_interlace - 1) & 1);
			int mode = (m_interlace - 1) >> 1;

			if(!m_dev.Interlace(ds, m_field ^ field, mode, ss[1].y)) // ss[1].y?
			{
				return false;
			}
		}

		return true;
	}

public:
	Device m_dev;

public:
	GSRendererT(BYTE* base, bool mt, void (*irq)(), int nloophack, int interlace, int aspectratio, int filter, bool vsync)
		: GSRenderer(base, mt, irq, nloophack, interlace, aspectratio, filter, vsync)
		, m_vertices(NULL)
		, m_count(0)
		, m_maxcount(0)
	{
	}

	bool Create(LPCTSTR title)
	{
		if(!__super::Create(title))
		{
			return false;
		}

		if(!m_dev.Create(m_hWnd))
		{
			return false;
		}

		Reset();

		return true;
	}

	void VSync(int field)
	{
		GSPerfMonAutoTimer pmat(m_perfmon);

		m_field = !!field;

		ProcessWindowMessages();

		Flush();

		if(!Merge()) return;

		// osd 

		static UINT64 s_frame = 0;
		static CString s_stats;

		m_perfmon.Put(GSPerfMon::Frame);

		if(m_perfmon.GetFrame() - s_frame >= 30)
		{
			m_perfmon.Update();

			s_frame = m_perfmon.GetFrame();

			double fps = 1000.0f / m_perfmon.Get(GSPerfMon::Frame);
			
			s_stats.Format(
				_T("%I64d | %d x %d | %.2f fps (%d%%) | %s - %s | %s | %d/%d | %d%% CPU | %.2f | %.2f/%.2f | %.2f"), 
				m_perfmon.GetFrame(), GetDisplaySize().cx, GetDisplaySize().cy, fps, (int)(100.0 * fps / GetFPS()),
				SMODE2->INT ? (CString(_T("Interlaced ")) + (SMODE2->FFMD ? _T("(frame)") : _T("(field)"))) : _T("Progressive"),
				g_interlace[m_interlace].name,
				g_aspectratio[m_aspectratio].name,
				(int)m_perfmon.Get(GSPerfMon::Prim),
				(int)m_perfmon.Get(GSPerfMon::Draw),
				m_perfmon.CPU(),
				m_perfmon.Get(GSPerfMon::Swizzle) / 1024,
				m_perfmon.Get(GSPerfMon::Unswizzle) / 1024,
				m_perfmon.Get(GSPerfMon::Unswizzle2) / 1024,
				m_perfmon.Get(GSPerfMon::Texture) / 1024
				);

			if(m_perfmon.Get(GSPerfMon::COLCLAMP)) _tprintf(_T("*** NOT SUPPORTED: color wrap ***\n"));
			if(m_perfmon.Get(GSPerfMon::PABE)) _tprintf(_T("*** NOT SUPPORTED: per pixel alpha blend ***\n"));
			if(m_perfmon.Get(GSPerfMon::DATE)) _tprintf(_T("*** PERFORMANCE WARNING: destination alpha test used ***\n"));
			if(m_perfmon.Get(GSPerfMon::ABE)) _tprintf(_T("*** NOT SUPPORTED: alpha blending mode ***\n"));
			if(m_perfmon.Get(GSPerfMon::DepthTexture)) _tprintf(_T("*** NOT SUPPORTED: depth texture ***\n"));		

			SetWindowText(s_stats);
		}

		if(m_osd)
		{
			// TODO: m_dev.Draw(s_stats + _T("\n\nF5: interlace mode\nF6: aspect ratio\nF7: OSD"));
		}

		//

		if(m_dev.IsLost())
		{
			ResetDevice();
		}

		//

		static int ar[][2] = {{0, 0}, {4, 3}, {16, 9}};

		int arx = ar[m_aspectratio][0];
		int ary = ar[m_aspectratio][1];

		m_dev.Present(arx, ary);
	}

	bool MakeSnapshot(LPCTSTR path)
	{
		CString fn;
		fn.Format(_T("%s_%s.bmp"), path, CTime::GetCurrentTime().Format(_T("%Y%m%d%H%M%S")));
		return m_dev.SaveCurrent(fn);
	}
};
