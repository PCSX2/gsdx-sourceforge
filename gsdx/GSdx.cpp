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

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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

	return TRUE;
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

#if _M_SSE >= 0x301
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
	const UINT32 minor = 7;

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

EXPORT_C_(INT32) __GSopen(void* dsp, char* title, int mt, int renderer)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	GSclose();

	// TODO: wrap constructor params into a GSOptions class 

	int nloophack = AfxGetApp()->GetProfileInt(_T("Settings"), _T("nloophack"), 2);
	int interlace = AfxGetApp()->GetProfileInt(_T("Settings"), _T("interlace"), 0);
	int aspectratio = AfxGetApp()->GetProfileInt(_T("Settings"), _T("aspectratio"), 1);
	int filter = AfxGetApp()->GetProfileInt(_T("Settings"), _T("filter"), 1);
	bool vsync = !!AfxGetApp()->GetProfileInt(_T("Settings"), _T("vsync"), FALSE);

	switch(renderer)
	{
	default: 
	case 0: s_gs = new GSRendererHW9(s_basemem, !!mt, s_irq, nloophack, interlace, aspectratio, filter, vsync); break;
	case 1: s_gs = new GSRendererSWFP<GSDevice9>(s_basemem, !!mt, s_irq, nloophack, interlace, aspectratio, filter, vsync); break;
	case 2: s_gs = new GSRendererNull<GSDevice9>(s_basemem, !!mt, s_irq, nloophack, interlace, aspectratio, filter, vsync); break;
	case 3: s_gs = new GSRendererHW10(s_basemem, !!mt, s_irq, nloophack, interlace, aspectratio, filter, vsync); break;
	case 4: s_gs = new GSRendererSWFP<GSDevice10>(s_basemem, !!mt, s_irq, nloophack, interlace, aspectratio, filter, vsync); break;
	case 5: s_gs = new GSRendererNull<GSDevice10>(s_basemem, !!mt, s_irq, nloophack, interlace, aspectratio, filter, vsync); break;
	}

	s_hr = ::CoInitialize(0);

	if(!s_gs->Create(CString(title)))
	{
		GSclose();
		return -1;
	}

	s_gs->Show();

	*(HWND*)dsp = *s_gs;

	return 0;
}

EXPORT_C_(INT32) GSopen(void* dsp, char* title, int mt)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	int renderer = AfxGetApp()->GetProfileInt(_T("Settings"), _T("renderer"), 0);

	return __GSopen(dsp, title, mt, renderer);
}

EXPORT_C GSreset()
{
	s_gs->Reset();
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
	s_gs->Transfer(mem + addr, (0x4000 - addr) / 16, 0);
}

EXPORT_C GSgifTransfer2(BYTE* mem, UINT32 size)
{
	s_gs->Transfer(mem, size, 1);
}

EXPORT_C GSgifTransfer3(BYTE* mem, UINT32 size)
{
	s_gs->Transfer(mem, size, 2);
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

EXPORT_C GSsetGameCRC(int crc, int options)
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

	CAtlArray<BYTE> buff;

	if(FILE* fp = fopen(lpszCmdLine, "rb"))
	{
		GSinit();

		BYTE regs[0x2000];
		GSsetBaseMem(regs);

		HWND hWnd = NULL;
		__GSopen(&hWnd, _T(""), true, renderer);

		int crc;
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

		int index, size, addr;

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
			default:
				return;
			}
		}

		GSclose();

		GSshutdown();

		fclose(fp);
	}
}
