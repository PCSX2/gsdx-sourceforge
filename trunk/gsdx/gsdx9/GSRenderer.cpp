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

#include "StdAfx.h"
#include "GSRenderer.h"
#include "GSSettingsDlg.h"

BEGIN_MESSAGE_MAP(GSRenderer, CWnd)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

GSRenderer::GSRenderer(BYTE* base, bool mt, void (*irq)(), int nloophack)
	: GSState(base, mt, irq, nloophack)
	, m_osd(true)
	, m_field(0)
{
	m_interlace = AfxGetApp()->GetProfileInt(_T("Settings"), _T("interlace"), 0);
	m_aspectratio = AfxGetApp()->GetProfileInt(_T("Settings"), _T("aspectratio"), 1);
	m_filter = AfxGetApp()->GetProfileInt(_T("Settings"), _T("filter"), 1);
	m_vsync = !!AfxGetApp()->GetProfileInt(_T("Settings"), _T("vsync"), FALSE);
}

GSRenderer::~GSRenderer()
{
	DestroyWindow();
}

bool GSRenderer::Create(LPCTSTR title)
{
	CRect r;

	GetDesktopWindow()->GetWindowRect(r);

	CSize s(r.Width() / 3, r.Width() / 4);

	r = CRect(r.CenterPoint() - CSize(s.cx / 2, s.cy / 2), s);

	LPCTSTR wc = AfxRegisterWndClass(CS_VREDRAW|CS_HREDRAW|CS_DBLCLKS, AfxGetApp()->LoadStandardCursor(IDC_ARROW), 0, 0);

	if(!CreateEx(0, wc, title, WS_OVERLAPPEDWINDOW, r, NULL, 0))
	{
		return false;
	}

	if(!m_dev.Create(m_hWnd))
	{
		return false;
	}

	// FIXME

	if(!m_dev.m_pp.Windowed)
	{
		m_osd = false;
	}

	if(!m_merge.Create(&m_dev))
	{
		return false;
	}

	Reset();

	return true;
}

void GSRenderer::Show()
{
	SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	SetForegroundWindow();
	ShowWindow(SW_SHOWNORMAL);
}

void GSRenderer::Hide()
{
	ShowWindow(SW_HIDE);
}

void GSRenderer::OnClose()
{
	Hide();

	PostMessage(WM_QUIT);
}

void GSRenderer::VSync(int field)
{
	GSPerfMonAutoTimer pmat(m_perfmon);

	m_field = !!field;

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

	Flush();

	Flip();

	m_perfmon.Put(GSPerfMon::Frame);

	// osd

	static UINT64 s_frame = 0;
	static CString s_stats;

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

		if(m_osd) // && m_d3dpp.Windowed
		{
			SetWindowText(s_stats);
		}

		if(m_perfmon.Get(GSPerfMon::COLCLAMP)) _tprintf(_T("*** NOT SUPPORTED: color wrap ***\n"));
		if(m_perfmon.Get(GSPerfMon::PABE)) _tprintf(_T("*** NOT SUPPORTED: per pixel alpha blend ***\n"));
		if(m_perfmon.Get(GSPerfMon::DATE)) _tprintf(_T("*** PERFORMANCE WARNING: destination alpha test used ***\n"));
		if(m_perfmon.Get(GSPerfMon::ABE)) _tprintf(_T("*** NOT SUPPORTED: alpha blending mode ***\n"));
		if(m_perfmon.Get(GSPerfMon::DepthTexture)) _tprintf(_T("*** NOT SUPPORTED: depth texture ***\n"));		
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

/*
	// TODO

	if(m_osd)
	{
		m_dev.Draw(s_stats + _T("\n\nF5: interlace mode\nF6: aspect ratio\nF7: OSD"));
	}
*/
}

// TODO

void GSRenderer::FinishFlip(FlipInfo src[2])
{
	CSize fs(0, 0);
	CSize ds(0, 0);

	for(int i = 0; i < 2; i++)
	{
		if(src[i].t)
		{
			CSize s = GetFrameSize(i);

			s.cx = (int)(src[i].s.x * s.cx);
			s.cy = (int)(src[i].s.y * s.cy);

			ASSERT(fs.cx == 0 || fs.cx == s.cx);
			ASSERT(fs.cy == 0 || fs.cy == s.cy || fs.cy + 1 == s.cy);

			fs.cx = s.cx;
			fs.cy = s.cy;

			if(SMODE2->INT && SMODE2->FFMD) s.cy *= 2;

			ASSERT(ds.cx == 0 || ds.cx == s.cx);
			ASSERT(ds.cy == 0 || ds.cy == s.cy || ds.cy + 1 == s.cy);

			ds.cx = s.cx;
			ds.cy = s.cy;
		}
	}

	if(fs.cx == 0 || fs.cy == 0)
	{
		return;
	}

	// merge

	if(!m_dev.m_tex_merge || m_dev.m_tex_merge.m_desc.Width != fs.cx || m_dev.m_tex_merge.m_desc.Height != fs.cy)
	{
		m_dev.CreateRenderTarget(m_dev.m_tex_merge, fs.cx, fs.cy);
	}

	Merge(src, m_dev.m_tex_merge);

	GSTextureDX9 current = m_dev.m_tex_merge;

	if(SMODE2->INT && m_interlace > 0)
	{
		int field = 1 - ((m_interlace - 1) & 1);
		int mode = (m_interlace - 1) >> 1;

		if(!m_dev.Interlace(m_dev.m_tex_merge, current, ds, m_field ^ field, mode, src[1].s.y))
		{
			return;
		}
	}

	m_dev.m_tex_current = current;
}

void GSRenderer::Merge(FlipInfo src[2], GSTextureDX9& dst)
{
	GSTextureDX9 st[2] = 
	{
		src[0].t ? src[0].t : m_dev.m_tex_1x1,
		src[1].t ? src[1].t : m_dev.m_tex_1x1,
	};

	CRect r[2];
	
	r[0] = GetFrameRect(0);
	r[1] = GetFrameRect(1);

	GSVector4 sr[2];

	sr[0].x = src[0].s.x * r[0].left / src[0].t.m_desc.Width;
	sr[1].x = src[1].s.x * r[1].left / src[1].t.m_desc.Width;
	sr[0].y = src[0].s.y * r[0].top / src[0].t.m_desc.Height;
	sr[1].y = src[1].s.y * r[1].top / src[1].t.m_desc.Height;
	sr[0].z = src[0].s.x * r[0].right / src[0].t.m_desc.Width;
	sr[1].z = src[1].s.x * r[1].right / src[1].t.m_desc.Width;
	sr[0].w = src[0].s.y * r[0].bottom / src[0].t.m_desc.Height;
	sr[1].w = src[1].s.y * r[1].bottom / src[1].t.m_desc.Height;

	GSMergeFX::PSSelector sel;

	sel.en1 = IsEnabled(0);
	sel.en2 = IsEnabled(1);
	sel.slbg = PMODE->SLBG;
	sel.mmod = PMODE->MMOD;

	GSMergeFX::PSConstantBuffer cb;

	cb.BGColor.x = (float)BGCOLOR->R / 255;
	cb.BGColor.y = (float)BGCOLOR->G / 255;
	cb.BGColor.z = (float)BGCOLOR->B / 255;
	cb.BGColor.w = (float)PMODE->ALP / 255;

	m_merge.Draw(st, sr, dst, sel, cb);
}
