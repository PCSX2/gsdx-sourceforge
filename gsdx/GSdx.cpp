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
#include "GSdx.h"
#include "GSRendererHW9.h"
#include "GSRendererHW10.h"
#include "GSRendererSW.h"
#include "GSRendererNull.h"
#include "GSSettingsDlg.h"

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

BEGIN_MESSAGE_MAP(GSdxApp, CWinApp)
END_MESSAGE_MAP()

GSdxApp::GSdxApp()
{
}

GSdxApp theApp;

BOOL GSdxApp::InitInstance()
{
	__super::InitInstance();

	SetRegistryKey(_T("Gabest"));

	CString str;
	GetModuleFileName(AfxGetInstanceHandle(), str.GetBuffer(MAX_PATH), MAX_PATH);
	str.ReleaseBuffer();

	CPath path(str);
	path.RenameExtension(_T(".ini"));
	
	CPath fn = path;
	fn.StripPath();

	path.RemoveFileSpec();
	path.Append(_T("..\\inis"));
	CreateDirectory(path, NULL);
	path.Append(fn);

	if(m_pszRegistryKey)
	{
		free((void*)m_pszRegistryKey);
	}

	m_pszRegistryKey = NULL;
	
	if(m_pszProfileName)
	{
		free((void*)m_pszProfileName);
	}

	m_pszProfileName = _tcsdup((LPCTSTR)path);

	return TRUE;
}

static bool CheckSSE()
{
	__try
	{
		static __m128i m;

		#if _M_SSE >= 0x402
		m.m128i_i32[0] = _mm_popcnt_u32(1234);
		#elif _M_SSE >= 0x401
		m = _mm_packus_epi32(m, m);
		#elif _M_SSE >= 0x301
		m = _mm_alignr_epi8(m, m, 1);
		#elif _M_SSE >= 0x200
		m = _mm_packs_epi32(m, m);
		#endif
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return false;
	}

	return true;
}

//

#define PS2E_LT_GS 0x01
#define PS2E_GS_VERSION 0x0006
#define PS2E_X86 0x01   // 32 bit
#define PS2E_X86_64 0x02   // 64 bit

EXPORT_C_(UINT32) PS2EgetLibType()
{
	return PS2E_LT_GS;
}

EXPORT_C_(char*) PS2EgetLibName()
{
	CString str = _T("GSdx");

#if _M_AMD64
	str += _T(" 64-bit");
#endif

	CAtlList<CString> sl;

#ifdef __INTEL_COMPILER
	CString s;
	s.Format(_T("Intel C++ %d.%02d"), __INTEL_COMPILER/100, __INTEL_COMPILER%100);
	sl.AddTail(s);
#elif _MSC_VER
	CString s;
	s.Format(_T("MSVC %d.%02d"), _MSC_VER/100, _MSC_VER%100);
	sl.AddTail(s);
#endif

#if _M_SSE >= 0x402
	sl.AddTail(_T("SSE42"));
#elif _M_SSE >= 0x401
	sl.AddTail(_T("SSE41"));
#elif _M_SSE >= 0x301
	sl.AddTail(_T("SSSE3"));
#elif _M_SSE >= 0x200
	sl.AddTail(_T("SSE2"));
#elif _M_SSE >= 0x100
	sl.AddTail(_T("SSE"));
#endif

	POSITION pos = sl.GetHeadPosition();

	while(pos)
	{
		if(pos == sl.GetHeadPosition()) str += _T(" (");
		str += sl.GetNext(pos);
		str += pos ? _T(", ") : _T(")");
	}

	static char buff[256];
	strncpy(buff, CStringA(str), min(countof(buff)-1, str.GetLength()));
	return buff;
}

EXPORT_C_(UINT32) PS2EgetLibVersion2(UINT32 type)
{
	const UINT32 revision = 0;
	const UINT32 build = 1;
	const UINT32 minor = 9;

	return (build << 0) | (revision << 8) | (PS2E_GS_VERSION << 16) | (minor << 24);
}

EXPORT_C_(UINT32) PS2EgetCpuPlatform()
{
#if _M_AMD64
	return PS2E_X86_64;
#else
	return PS2E_X86;
#endif
}

//////////////////

static HRESULT s_hr = E_FAIL;
static GSRendererBase* s_gs;
static void (*s_irq)() = NULL;
static BYTE* s_basemem = NULL;

EXPORT_C GSsetBaseMem(BYTE* mem)
{
	s_basemem = mem - 0x12000000;
}

EXPORT_C_(INT32) GSinit()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return 0;
}

EXPORT_C GSshutdown()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
}

EXPORT_C GSclose()
{
	delete s_gs; 
	
	s_gs = NULL;

	if(SUCCEEDED(s_hr))
	{
		::CoUninitialize();

		s_hr = E_FAIL;
	}
}

static INT32 GSopen(void* dsp, char* title, int mt, int renderer)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// 

	CString str;

	str.Format(_T("d3dx9_%d.dll"), D3DX_SDK_VERSION);

	if(HINSTANCE hDll = LoadLibrary(str))
	{
		FreeLibrary(hDll);
	}
	else
	{
		int res = AfxMessageBox(_T("Please update DirectX!\n\nWould you like to open the download page in your browser?"), MB_YESNO);

		if(res == IDYES)
		{
			ShellExecute(NULL, _T("open"), _T("http://www.microsoft.com/downloads/details.aspx?FamilyId=2DA43D38-DB71-4C1B-BC6A-9B6652CD92A3"), NULL, NULL, SW_SHOWNORMAL);
		}

		return -1;
	}

	// 

	if(!CheckSSE())
	{
		CString str;
		str.Format(_T("This CPU does not support SSE %d.%02d"), _M_SSE >> 8, _M_SSE & 0xff);
		AfxMessageBox(str, MB_OK);
		return -1;
	}

	//

	GSclose();

	// TODO 

	int nloophack = AfxGetApp()->GetProfileInt(_T("Settings"), _T("nloophack"), 2);

	GSRendererSettings rs;

	rs.m_interlace = AfxGetApp()->GetProfileInt(_T("Settings"), _T("interlace"), 0);
	rs.m_aspectratio = AfxGetApp()->GetProfileInt(_T("Settings"), _T("aspectratio"), 1);
	rs.m_filter = AfxGetApp()->GetProfileInt(_T("Settings"), _T("filter"), 1);
	rs.m_vsync = !!AfxGetApp()->GetProfileInt(_T("Settings"), _T("vsync"), FALSE);
	rs.m_nativeres = !!AfxGetApp()->GetProfileInt(_T("Settings"), _T("nativeres"), FALSE);

	switch(renderer)
	{
	default: 
	case 0: s_gs = new GSRendererHW9(s_basemem, !!mt, s_irq, nloophack, rs); break;
	case 1: s_gs = new GSRendererSW<GSDevice9>(s_basemem, !!mt, s_irq, nloophack, rs); break;
	case 2: s_gs = new GSRendererNull<GSDevice9>(s_basemem, !!mt, s_irq, nloophack, rs); break;
	case 3: s_gs = new GSRendererHW10(s_basemem, !!mt, s_irq, nloophack, rs); break;
	case 4: s_gs = new GSRendererSW<GSDevice10>(s_basemem, !!mt, s_irq, nloophack, rs); break;
	case 5: s_gs = new GSRendererNull<GSDevice10>(s_basemem, !!mt, s_irq, nloophack, rs); break;
	case 6: s_gs = new GSRendererSW<GSDeviceNull>(s_basemem, !!mt, s_irq, nloophack, rs); break;
	case 7: s_gs = new GSRendererNull<GSDeviceNull>(s_basemem, !!mt, s_irq, nloophack, rs); break;
	}

	s_hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);

	if(!s_gs->Create(CString(title)))
	{
		GSclose();
		return -1;
	}

	s_gs->m_wnd.Show();

	*(HWND*)dsp = s_gs->m_wnd;

	// if(mt) _mm_setcsr(MXCSR);

	return 0;
}

EXPORT_C_(INT32) GSopen(void* dsp, char* title, int mt)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	int renderer = AfxGetApp()->GetProfileInt(_T("Settings"), _T("renderer"), 0);

	return GSopen(dsp, title, mt, renderer);
}

EXPORT_C GSreset()
{
	s_gs->Reset();
}

EXPORT_C GSgifSoftReset(int mask)
{
	s_gs->SoftReset((BYTE)mask);
}

EXPORT_C GSwriteCSR(UINT32 csr)
{
	s_gs->WriteCSR(csr);
}

EXPORT_C GSreadFIFO(BYTE* mem)
{
	s_gs->ReadFIFO(mem, 1);
}

EXPORT_C GSreadFIFO2(BYTE* mem, UINT32 size)
{
	s_gs->ReadFIFO(mem, size);
}

EXPORT_C GSgifTransfer1(BYTE* mem, UINT32 addr)
{
	s_gs->Transfer<0>(mem + addr, (0x4000 - addr) / 16);
}

EXPORT_C GSgifTransfer2(BYTE* mem, UINT32 size)
{
	s_gs->Transfer<1>(mem, size);
}

EXPORT_C GSgifTransfer3(BYTE* mem, UINT32 size)
{
	s_gs->Transfer<2>(mem, size);
}

EXPORT_C GSvsync(int field)
{
	s_gs->VSync(field);
}

EXPORT_C_(UINT32) GSmakeSnapshot(char* path)
{
	return s_gs->MakeSnapshot(CString(path) + _T("gsdx"));
}

EXPORT_C GSkeyEvent(keyEvent* ev)
{
}

EXPORT_C_(INT32) GSfreeze(int mode, freezeData* data)
{
	if(mode == FREEZE_SAVE)
	{
		return s_gs->Freeze(data, false);
	}
	else if(mode == FREEZE_SIZE)
	{
		return s_gs->Freeze(data, true);
	}
	else if(mode == FREEZE_LOAD)
	{
		return s_gs->Defrost(data);
	}

	return 0;
}

EXPORT_C GSconfigure()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	GSSettingsDlg dlg;

	if(IDOK == dlg.DoModal())
	{
		GSshutdown();
		GSinit();
	}
}

EXPORT_C_(INT32) GStest()
{
	return 0;

	// TODO

	/*
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CComPtr<ID3D10Device> dev;

	return SUCCEEDED(D3D10CreateDevice(NULL, D3D10_DRIVER_TYPE_HARDWARE, NULL, 0, D3D10_SDK_VERSION, &dev)) ? 0 : -1;
	*/
}

EXPORT_C GSabout()
{
}

EXPORT_C GSirqCallback(void (*irq)())
{
	s_irq = irq;
}

EXPORT_C GSsetGameCRC(DWORD crc, int options)
{
	s_gs->SetGameCRC(crc, options);
}

EXPORT_C GSgetLastTag(UINT32* tag) 
{
	s_gs->GetLastTag(tag);
}

EXPORT_C GSsetFrameSkip(int frameskip)
{
	s_gs->SetFrameSkip(frameskip);
}

EXPORT_C GSReplay(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)
{
	int renderer = -1;

	{
		char* start = lpszCmdLine;
		char* end = NULL;
		long n = strtol(lpszCmdLine, &end, 10);
		if(end > start) {renderer = n; lpszCmdLine = end;}
	}

	while(*lpszCmdLine == ' ') lpszCmdLine++;

	::SetPriorityClass(::GetCurrentProcess(), HIGH_PRIORITY_CLASS);

	CAtlArray<BYTE> buff;

	if(FILE* fp = fopen(lpszCmdLine, "rb"))
	{
		GSinit();

		BYTE regs[0x2000];
		GSsetBaseMem(regs);

		HWND hWnd = NULL;
		GSopen(&hWnd, _T(""), true, renderer);

		DWORD crc;
		fread(&crc, 4, 1, fp);
		GSsetGameCRC(crc, 0);

		freezeData fd;
		fread(&fd.size, 4, 1, fp);
		fd.data = new BYTE[fd.size];
		fread(fd.data, fd.size, 1, fp);
		GSfreeze(FREEZE_LOAD, &fd);
		delete [] fd.data;

		fread(regs, 0x2000, 1, fp);

		long start = ftell(fp);

		unsigned int index, size, addr;

		GSvsync(1);

		while(1)
		{
			switch(fgetc(fp))
			{
			case EOF:
				fseek(fp, start, 0);
				if(!IsWindowVisible(hWnd)) return;
				break;
			case 0:
				index = fgetc(fp);
				fread(&size, 4, 1, fp);
				switch(index)
				{
				case 0:
					if(buff.GetCount() < 0x4000) buff.SetCount(0x4000);
					addr = 0x4000 - size;
					fread(buff.GetData() + addr, size, 1, fp);
					GSgifTransfer1(buff.GetData(), addr);
					break;
				case 1:
					if(buff.GetCount() < size) buff.SetCount(size);
					fread(buff.GetData(), size, 1, fp);
					GSgifTransfer2(buff.GetData(), size / 16);
					break;
				case 2:
					if(buff.GetCount() < size) buff.SetCount(size);
					fread(buff.GetData(), size, 1, fp);
					GSgifTransfer3(buff.GetData(), size / 16);
					break;
				}
				break;
			case 1:
				GSvsync(fgetc(fp));
				if(!IsWindowVisible(hWnd)) return;
				break;
			case 2:
				fread(&size, 4, 1, fp);
				if(buff.GetCount() < size) buff.SetCount(size);
				GSreadFIFO2(buff.GetData(), size / 16);
				break;
			case 3:
				fread(regs, 0x2000, 1, fp);
				break;
			default:
				return;
			}
		}

		GSclose();

		GSshutdown();

		fclose(fp);
	}
}

EXPORT_C GSBenchmark(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)
{
	::SetPriorityClass(::GetCurrentProcess(), HIGH_PRIORITY_CLASS);

	FILE* file = _tfopen(_T("c:\\log.txt"), _T("a"));

	_ftprintf(file, _T("-------------------------\n\n"));

	if(1)
	{
		GSLocalMemory mem;

		static struct {int psm; LPCSTR name;} s_format[] = 
		{
			{PSM_PSMCT32, "32"},
			{PSM_PSMCT24, "24"},
			{PSM_PSMCT16, "16"},
			{PSM_PSMCT16S, "16S"},
			{PSM_PSMT8, "8"},
			{PSM_PSMT4, "4"},
			{PSM_PSMT8H, "8H"},
			{PSM_PSMT4HL, "4HL"},
			{PSM_PSMT4HH, "4HH"},
			{PSM_PSMZ32, "32Z"},
			{PSM_PSMZ24, "24Z"},
			{PSM_PSMZ16, "16Z"},
			{PSM_PSMZ16S, "16ZS"},
		};

		BYTE* ptr = (BYTE*)_aligned_malloc(1024 * 1024 * 4, 16);

		for(int i = 0; i < 1024 * 1024 * 4; i++) ptr[i] = (BYTE)i;

		// 

		for(int tbw = 5; tbw <= 10; tbw++)
		{
			int n = 256 << ((10 - tbw) * 2);

			int w = 1 << tbw;
			int h = 1 << tbw;

			_ftprintf(file, _T("%d x %d\n\n"), w, h);

			for(int i = 0; i < countof(s_format); i++)
			{
				const GSLocalMemory::psm_t& psm = GSLocalMemory::m_psm[s_format[i].psm];

				GSLocalMemory::writeImage wi = psm.wi;
				GSLocalMemory::readImage ri = psm.ri;
				GSLocalMemory::readTexture rtx = psm.rtx;

				GIFRegBITBLTBUF BITBLTBUF;

				BITBLTBUF.SBP = 0;
				BITBLTBUF.SBW = w / 64;
				BITBLTBUF.SPSM = s_format[i].psm;
				BITBLTBUF.DBP = 0;
				BITBLTBUF.DBW = w / 64;
				BITBLTBUF.DPSM = s_format[i].psm;
				
				GIFRegTRXPOS TRXPOS;

				TRXPOS.SSAX = 0;
				TRXPOS.SSAY = 0;
				TRXPOS.DSAX = 0;
				TRXPOS.DSAY = 0;

				GIFRegTRXREG TRXREG;

				TRXREG.RRW = w;
				TRXREG.RRH = h;

				CRect r(0, 0, w, h);

				GIFRegTEX0 TEX0;

				TEX0.TBP0 = 0;
				TEX0.TBW = w / 64;

				GIFRegTEXA TEXA;

				TEXA.TA0 = 0;
				TEXA.TA1 = 0x80;
				TEXA.AEM = 0;

				int trlen = w * h * psm.trbpp / 8;
				int len = w * h * psm.bpp / 8;

				clock_t start, end;

				_ftprintf(file, _T("[%4s] "), s_format[i].name);

				start = clock();

				for(int j = 0; j < n; j++)
				{
					int x = 0;
					int y = 0;

					(mem.*wi)(x, y, ptr, trlen, BITBLTBUF, TRXPOS, TRXREG);
				}

				end = clock();

				_ftprintf(file, _T("%6d %6d | "), (int)((float)trlen * n / (end - start) / 1000), (int)((float)(w * h) * n / (end - start) / 1000));

				start = clock();

				for(int j = 0; j < n; j++)
				{
					int x = 0;
					int y = 0;

					(mem.*ri)(x, y, ptr, trlen, BITBLTBUF, TRXPOS, TRXREG);
				}

				end = clock();

				_ftprintf(file, _T("%6d %6d | "), (int)((float)trlen * n / (end - start) / 1000), (int)((float)(w * h) * n / (end - start) / 1000));

				start = clock();

				for(int j = 0; j < n; j++)
				{
					(mem.*rtx)(r, ptr, w * 4, TEX0, TEXA);
				}

				end = clock();

				_ftprintf(file, _T("%6d %6d "), (int)((float)len * n / (end - start) / 1000), (int)((float)(w * h) * n / (end - start) / 1000));

				_ftprintf(file, _T("\n"));

				fflush(file);
			}

			_ftprintf(file, _T("\n"));
		}

		_aligned_free(ptr);
	}

	if(0)
	{
		BYTE regs[0x2000];
		GSsetBaseMem(regs);

		HWND hWnd = NULL;
		GSopen(&hWnd, _T(""), true, 6);

		s_gs->m_env.COLCLAMP.CLAMP = 1;
		s_gs->m_env.PRIM.ABE = 0;
		s_gs->m_env.PRIM.FST = 1;
		s_gs->m_env.PRIM.TME = 1;
		s_gs->m_env.PRIM.IIP = 0;
		s_gs->m_env.TEXA.TA0 = 0;
		s_gs->m_env.TEXA.TA1 = 0x80;
		s_gs->m_env.TEXA.AEM = 0;
		s_gs->m_context->ALPHA.A = 0;
		s_gs->m_context->ALPHA.B = 1;
		s_gs->m_context->ALPHA.C = 0;
		s_gs->m_context->ALPHA.D = 1;
		s_gs->m_context->CLAMP.WMS = 1;
		s_gs->m_context->CLAMP.WMT = 1;
		s_gs->m_context->CLAMP.MINU = 0;
		s_gs->m_context->CLAMP.MINV = 0;
		s_gs->m_context->CLAMP.MAXU = 511;
		s_gs->m_context->CLAMP.MAXV = 511;
		s_gs->m_context->FRAME.FBP = 0 >> 5;
		s_gs->m_context->FRAME.FBW = 8;
		s_gs->m_context->FRAME.PSM = PSM_PSMCT16S;
		s_gs->m_context->SCISSOR.SCAX0 = 0;
		s_gs->m_context->SCISSOR.SCAY0 = 0;
		s_gs->m_context->SCISSOR.SCAX1 = 511;
		s_gs->m_context->SCISSOR.SCAY1 = 511;
		s_gs->m_context->TEST.ZTE = 0;
		s_gs->m_context->TEST.ZTST = 2;
		s_gs->m_context->TEX0.TBP0 = 0x2000;
		s_gs->m_context->TEX0.TBW = 8;
		s_gs->m_context->TEX0.PSM = PSM_PSMCT32;
		s_gs->m_context->TEX0.TFX = 1;
		s_gs->m_context->TEX0.TCC = 0;
		s_gs->m_context->TEX0.TW = 9;
		s_gs->m_context->TEX0.TH = 9;
		s_gs->m_context->TEX1.MMAG = 0;
		s_gs->m_context->TEX1.MMIN = 0;
		s_gs->m_context->ZBUF.ZBP = 0x1000 >> 5;
		s_gs->m_context->ZBUF.PSM = PSM_PSMZ24;

		GSRasterizer* ras = ((GSRendererSW<GSDeviceNull>*)s_gs)->GetRasterizer();

		int count = 512 * 512;

		GSVertexSW* vertices = (GSVertexSW*)_aligned_malloc(count * sizeof(GSVertexSW), 16);
/*
		// point

		for(int j = 0; j < 512; j++)
		{
			for(int i = 0; i < 512; i++)
			{
				GSVertexSW& v = vertices[(j << 7) + i];

				v.p = GSVector4(i, j, 0, 0);
				v.t = GSVector4((float)i + 0.5, (float)j + 0.5, 1.0f, 0.0f);
				v.c = GSVector4(128.0f);
			}
		}

		s_gs->PRIM->PRIM = GS_POINTLIST;

		ras->Draw(vertices, count);

		vertices[0].p = GSVector4(0, 0, 0, 0);
		vertices[0].t = GSVector4(0.5, 0.5, 1.0f, 0.0f);
		vertices[0].c = GSVector4(128.0f);
		vertices[1].p = GSVector4(512, 512, 0, 0);
		vertices[1].t = GSVector4(512.5f, 512.5f, 1.0f, 0.0f);
		vertices[1].c = GSVector4(128.0f);

		for(int i = 2; i < 512 * 512; i += 2)
		{
			memcpy(&vertices[i], &vertices[0], sizeof(vertices[0]) * 2);
		}

		// sprite

		s_gs->PRIM->PRIM = GS_SPRITE;

		ras->Draw(vertices, count);
*/
		// triangle

		vertices[0].p = GSVector4(0, 0, 0, 0);
		vertices[0].t = GSVector4(0.5, 0.5, 1.0f, 0.0f);
		vertices[0].c = GSVector4(128.0f);
		vertices[1].p = GSVector4(512, 0, 0, 0);
		vertices[1].t = GSVector4(512.5f, 0.5f, 1.0f, 0.0f);
		vertices[1].c = GSVector4(128.0f);
		vertices[2].p = GSVector4(512, 512, 0, 0);
		vertices[2].t = GSVector4(512.5f, 512.5f, 1.0f, 0.0f);
		vertices[2].c = GSVector4(128.0f);

		for(int i = 3; i < 512 * 512 - 2; i += 3)
		{
			memcpy(&vertices[i], &vertices[0], sizeof(vertices[0]) * 3);
		}

		s_gs->PRIM->PRIM = GS_TRIANGLELIST;

		ras->Draw(vertices, 999);

		//

		_aligned_free(vertices);

		GSclose();
	}

	//

	if(0)
	{
		GSLocalMemory mem;

		BYTE* ptr = (BYTE*)_aligned_malloc(1024 * 1024 * 4, 16);

		for(int i = 0; i < 1024 * 1024 * 4; i++) ptr[i] = (BYTE)i;

		const GSLocalMemory::psm_t& psm = GSLocalMemory::m_psm[PSM_PSMCT32];

		GSLocalMemory::writeImage wi = psm.wi;

		GIFRegBITBLTBUF BITBLTBUF;

		BITBLTBUF.DBP = 0;
		BITBLTBUF.DBW = 32;
		BITBLTBUF.DPSM = PSM_PSMCT32;
			
		GIFRegTRXPOS TRXPOS;

		TRXPOS.DSAX = 0;
		TRXPOS.DSAY = 1;

		GIFRegTRXREG TRXREG;

		TRXREG.RRW = 256;
		TRXREG.RRH = 256;

		int trlen = 256 * 256 * psm.trbpp / 8;

		int x = 0;
		int y = 0;

		(mem.*wi)(x, y, ptr, trlen, BITBLTBUF, TRXPOS, TRXREG);
	}

	//

	fclose(file);
}
