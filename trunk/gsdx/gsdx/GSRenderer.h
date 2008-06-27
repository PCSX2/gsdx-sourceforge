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
#include "GSSettingsDlg.h"
#include "GSCapture.h"

struct GSRendererSettings
{
	int m_interlace;
	int m_aspectratio;
	int m_filter;
	bool m_vsync;
	bool m_nativeres;
};

class GSRendererBase :  public GSWnd, public GSState, protected GSRendererSettings
{
protected:
	bool m_osd;
	int m_field;

	void ProcessWindowMessages()
	{
		MSG msg;

		memset(&msg, 0, sizeof(msg));

		while(msg.message != WM_QUIT && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_KEYDOWN)
			{
				int step = (::GetAsyncKeyState(VK_SHIFT) & 0x8000) ? -1 : 1;

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

public:
	GSRendererBase(BYTE* base, bool mt, void (*irq)(), int nloophack, const GSRendererSettings& rs)
		: GSState(base, mt, irq, nloophack)
		, m_osd(true)
		, m_field(0)
	{
		m_interlace = rs.m_interlace;
		m_aspectratio = rs.m_aspectratio;
		m_filter = rs.m_filter;
		m_vsync = rs.m_vsync;
		m_nativeres = rs.m_nativeres;
	};

	virtual void VSync(int field) = 0;
	virtual bool MakeSnapshot(LPCTSTR path) = 0;
};

template<class Device> class GSRenderer : public GSRendererBase
{
protected:
	typedef typename Device::Texture Texture;

	virtual void ResetDevice() {}
	virtual bool GetOutput(int i, Texture& t) = 0;

	bool Merge()
	{
		int baseline = INT_MAX;

		for(int i = 0; i < 2; i++)
		{
			if(IsEnabled(i))
			{
				baseline = min(GetDisplayPos(i).y, baseline);
			}
		}

		CSize fs(0, 0);
		CSize ds(0, 0);

		Texture st[2];
		GSVector4 sr[2];
		GSVector4 dr[2];

		for(int i = 0; i < 2; i++)
		{
			if(IsEnabled(i) && GetOutput(i, st[i]))
			{
				CRect r = GetFrameRect(i);

				// overscan hack

				if(GetDisplaySize(i).cy > 512) // hmm
				{
					int y = GetDeviceSize(i).cy;
					if(SMODE2->INT && SMODE2->FFMD) y /= 2;
					r.bottom = r.top + y;
				}

				//

				sr[i].x = st[i].m_scale.x * r.left / st[i].GetWidth();
				sr[i].y = st[i].m_scale.y * r.top / st[i].GetHeight();
				sr[i].z = st[i].m_scale.x * r.right / st[i].GetWidth();
				sr[i].w = st[i].m_scale.y * r.bottom / st[i].GetHeight();

				GSVector2 o;

				o.x = 0;
				o.y = 0;
				
				CPoint p = GetDisplayPos(i);

				if(p.y - baseline >= 4) // 2?
				{
					o.y = st[i].m_scale.y * (p.y - baseline);
				}

				if(SMODE2->INT && SMODE2->FFMD) o.y /= 2;

				dr[i].x = o.x;
				dr[i].y = o.y;
				dr[i].z = o.x + st[i].m_scale.x * r.Width();
				dr[i].w = o.y + st[i].m_scale.y * r.Height();

#ifdef _M_AMD64
// schrödinger's bug, fs will be trashed unless we access these values
CString str;
str.Format(_T("%d %f %f %f %f "), i, o.x, o.y, dr[i].z, dr[i].w);
//::MessageBox(NULL, str, _T(""), MB_OK);
#endif
				fs.cx = max(fs.cx, (int)(dr[i].z + 0.5f));
				fs.cy = max(fs.cy, (int)(dr[i].w + 0.5f));
			}
		}

		ds.cx = fs.cx;
		ds.cy = fs.cy;

		if(SMODE2->INT && SMODE2->FFMD) ds.cy *= 2;

		bool slbg = PMODE->SLBG;
		bool mmod = PMODE->MMOD;

		if(st[0] || st[1])
		{
			GSVector4 c;

			c.r = (float)BGCOLOR->R / 255;
			c.g = (float)BGCOLOR->G / 255;
			c.b = (float)BGCOLOR->B / 255;
			c.a = (float)PMODE->ALP / 255;

			m_dev.Merge(st, sr, dr, fs, slbg, mmod, c);

			if(SMODE2->INT && m_interlace > 0)
			{
				int field = 1 - ((m_interlace - 1) & 1);
				int mode = (m_interlace - 1) >> 1;

				if(!m_dev.Interlace(ds, m_field ^ field, mode, st[1].m_scale.y)) // st[1].m_scale.y
				{
					return false;
				}
			}
		}

		return true;
	}

	void DoCapture()
	{
		if(!m_capture.IsCapturing())
		{
			return;
		}

		CSize size = m_capture.GetSize();

		Texture current;

		m_dev.GetCurrent(current);

		Texture offscreen;

		if(m_dev.CopyOffscreen(current, GSVector4(0, 0, 1, 1), offscreen, size.cx, size.cy))
		{
			BYTE* bits = NULL;
			int pitch = 0;

			if(offscreen.Map(&bits, pitch))
			{
				m_capture.DeliverFrame(bits, pitch);
			}

			m_dev.Recycle(offscreen);
		}
	}


public:
	Device m_dev;
	bool m_psrr;

	int s_n;
	bool s_dump;
	bool s_save;
	bool s_savez;

	GSCapture m_capture;

public:
	GSRenderer(BYTE* base, bool mt, void (*irq)(), int nloophack, const GSRendererSettings& rs, bool psrr)
		: GSRendererBase(base, mt, irq, nloophack, rs)
		, m_psrr(psrr)
	{
		s_n = 0;
		s_dump = false;
		s_save = false;
		s_savez = false;
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

		Flush();

		m_perfmon.Put(GSPerfMon::Frame);

		ProcessWindowMessages();

		Dump();

		if(!Merge()) return;

		// osd 

		static UINT64 s_frame = 0;
		static CString s_stats;

		if(m_perfmon.GetFrame() - s_frame >= 30)
		{
			m_perfmon.Update();

			s_frame = m_perfmon.GetFrame();

			double fps = 1000.0f / m_perfmon.Get(GSPerfMon::Frame);
			
			s_stats.Format(
				_T("%I64d | %d x %d | %.2f fps (%d%%) | %s - %s | %s | %d/%d | %d%% CPU | %.2f | %.2f"), 
				m_perfmon.GetFrame(), GetDisplaySize().cx, GetDisplaySize().cy, fps, (int)(100.0 * fps / GetFPS()),
				SMODE2->INT ? (CString(_T("Interlaced ")) + (SMODE2->FFMD ? _T("(frame)") : _T("(field)"))) : _T("Progressive"),
				g_interlace[m_interlace].name,
				g_aspectratio[m_aspectratio].name,
				(int)m_perfmon.Get(GSPerfMon::Prim),
				(int)m_perfmon.Get(GSPerfMon::Draw),
				m_perfmon.CPU(),
				m_perfmon.Get(GSPerfMon::Swizzle) / 1024,
				m_perfmon.Get(GSPerfMon::Unswizzle) / 1024
			);

			double fillrate = m_perfmon.Get(GSPerfMon::Fillrate);

			if(fillrate > 0)
			{
				s_stats.Format(_T("%s | %.2f mpps"), CString(s_stats), fps * fillrate / (1024 * 1024));
			}

			if(m_capture.IsCapturing())
			{
				s_stats += _T(" | Recording...");
			}

			if(m_perfmon.Get(GSPerfMon::COLCLAMP)) _tprintf(_T("*** NOT SUPPORTED: color wrap ***\n"));
			if(m_perfmon.Get(GSPerfMon::PABE)) _tprintf(_T("*** NOT SUPPORTED: per pixel alpha blend ***\n"));
			if(m_perfmon.Get(GSPerfMon::DATE)) _tprintf(_T("*** PERFORMANCE WARNING: destination alpha test used ***\n"));
			if(m_perfmon.Get(GSPerfMon::ABE)) _tprintf(_T("*** NOT SUPPORTED: alpha blending mode ***\n"));
			if(m_perfmon.Get(GSPerfMon::DepthTexture)) _tprintf(_T("*** NOT SUPPORTED: depth texture ***\n"));		

			SetWindowText(s_stats);
		}

		if(m_osd)
		{
			m_dev.Draw(s_stats + _T("\n\nF5: interlace mode\nF6: aspect ratio\nF7: OSD"));
		}

		if(m_frameskip)
		{
			return;
		}

		//

		if(m_dev.IsLost())
		{
			ResetDevice();
		}

		//

		static const int ar[][2] = {{0, 0}, {4, 3}, {16, 9}};

		int arx = ar[m_aspectratio][0];
		int ary = ar[m_aspectratio][1];

		CRect cr;
		
		GetClientRect(&cr);

		CRect r = cr;

		if(arx > 0 && ary > 0)
		{
			if(r.Width() * ary > r.Height() * arx)
			{
				int w = r.Height() * arx / ary;
				r.left = r.CenterPoint().x - w / 2;
				if(r.left & 1) r.left++;
				r.right = r.left + w;
			}
			else
			{
				int h = r.Width() * ary / arx;
				r.top = r.CenterPoint().y - h / 2;
				if(r.top & 1) r.top++;
				r.bottom = r.top + h;
			}
		}

		r &= cr;

		m_dev.Present(r);

		DoCapture();
	}

	void Dump()
	{
		if(m_dumpfp == NULL)
		{
			if(m_field == 0 && !m_dumpfn.IsEmpty())
			{
				m_dumpfp = _tfopen(m_dumpfn, _T("wb"));

				freezeData fd;
				fd.size = 0;
				fd.data = NULL;
				Freeze(&fd, true);
				fd.data = new BYTE[fd.size];
				Freeze(&fd, false);

				fwrite(&m_crc, 4, 1, m_dumpfp);
				fwrite(&fd.size, 4, 1, m_dumpfp);
				fwrite(fd.data, fd.size, 1, m_dumpfp);
				fwrite(PMODE, 0x2000, 1, m_dumpfp);

				delete [] fd.data;
			}
		}
		else
		{
			fputc(3, m_dumpfp);
			fwrite(PMODE, 0x2000, 1, m_dumpfp);

			fputc(1, m_dumpfp);
			fputc(m_field, m_dumpfp);

			if(m_field == 0 && !(::GetAsyncKeyState(VK_CONTROL) & 0x8000))
			{
				fclose(m_dumpfp);
				m_dumpfp = NULL;
				m_dumpfn.Empty();
			}
		}
	}

	bool MakeSnapshot(LPCTSTR path)
	{
		if((::GetAsyncKeyState(VK_CONTROL) & 0x8000))
		{
			if(m_capture.IsCapturing())
			{
				m_capture.EndCapture();
			}
			else
			{
				m_capture.BeginCapture(GetFPS());
			}

			return true;
		}

		CString fn;

		fn.Format(_T("%s_%s"), path, CTime::GetCurrentTime().Format(_T("%Y%m%d%H%M%S")));

		if((::GetAsyncKeyState(VK_SHIFT) & 0x8000) && m_dumpfn.IsEmpty())
		{
			m_dumpfn = fn + _T(".gs");
		}

		return m_dev.SaveCurrent(fn + _T(".bmp"));
	}

	virtual void MinMaxUV(int w, int h, CRect& r) {r = CRect(0, 0, w, h);}
	virtual bool CanUpscale() {return !m_nativeres;}
};

template<class Device, class Vertex> class GSRendererT : public GSRenderer<Device>
{
protected:
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
		while(m_vl.GetCount() >= m_vprim)
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

			Vertex* v = &m_vertices[m_count];

			int count = 0;

			switch(PRIM->PRIM)
			{
			case GS_POINTLIST:
				m_vl.RemoveAt(0, v[0]);
				count = 1;
				break;
			case GS_LINELIST:
				m_vl.RemoveAt(0, v[0]);
				m_vl.RemoveAt(0, v[1]);
				count = 2;
				break;
			case GS_LINESTRIP:
				m_vl.RemoveAt(0, v[0]);
				m_vl.GetAt(0, v[1]);
				count = 2;
				break;
			case GS_TRIANGLELIST:
				m_vl.RemoveAt(0, v[0]);
				m_vl.RemoveAt(0, v[1]);
				m_vl.RemoveAt(0, v[2]);
				count = 3;
				break;
			case GS_TRIANGLESTRIP:
				m_vl.RemoveAt(0, v[0]);
				m_vl.GetAt(0, v[1]);
				m_vl.GetAt(1, v[2]);
				count = 3;
				break;
			case GS_TRIANGLEFAN:
				m_vl.GetAt(0, v[0]);
				m_vl.RemoveAt(1, v[1]);
				m_vl.GetAt(1, v[2]);
				count = 3;
				break;
			case GS_SPRITE:
				m_vl.RemoveAt(0, v[0]);
				m_vl.RemoveAt(0, v[1]);
				count = 2;
				break;
			case GS_INVALID:
				ASSERT(0);
				m_vl.RemoveAll();
				return;
			default:
				__assume(0);
			}

			if(!skip)
			{
				(this->*m_fpDrawingKickHandlers[PRIM->PRIM])(v, count);

				m_count += count;
			}
		}
	}

	typedef void (GSRendererT<Device, Vertex>::*DrawingKickHandler)(Vertex* v, int& count);

	DrawingKickHandler m_fpDrawingKickHandlers[8];

	void DrawingKickNull(Vertex* v, int& count)
	{
		ASSERT(0);
	}

	virtual void Draw() = 0;

	void ResetPrim()
	{
		m_vl.RemoveAll();
	}

	void FlushPrim() 
	{
		if(m_count > 0)
		{
			/*
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
			*/

			Draw();

			m_count = 0;
		}
	}

public:
	GSRendererT(BYTE* base, bool mt, void (*irq)(), int nloophack, const GSRendererSettings& rs, bool psrr = true)
		: GSRenderer<Device>(base, mt, irq, nloophack, rs, psrr)
		, m_count(0)
		, m_maxcount(0)
		, m_vertices(NULL)
	{
		for(int i = 0; i < countof(m_fpDrawingKickHandlers); i++)
		{
			m_fpDrawingKickHandlers[i] = &GSRendererT<Device, Vertex>::DrawingKickNull;
		}
	}
};
