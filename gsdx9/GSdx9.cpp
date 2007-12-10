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
 *	Special Notes: 
 *
 */

#include "stdafx.h"
#include "GSdx9.h"
#include "GSRendererHW.h"
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

BEGIN_MESSAGE_MAP(GSdx9App, CWinApp)
END_MESSAGE_MAP()

GSdx9App::GSdx9App()
{
}

GSdx9App theApp;

BOOL GSdx9App::InitInstance()
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
	CString str = _T("GSdx9");

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

#if _M_IX86_FP >= 2
	sl.AddTail(_T("SSE2"));
#elif _M_IX86_FP >= 1
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
	const UINT32 build = 11;
	const UINT32 minor = 2;

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
static GSRenderer* s_gs;
static void (*s_irq)() = NULL;

BYTE* s_basemem = NULL;

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

EXPORT_C_(INT32) GSopen(void* dsp, char* title, int mt)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	GSclose();

	bool nloophack = AfxGetApp()->GetProfileInt(_T("Settings"), _T("nloophack"), 2) == 1;

	switch(AfxGetApp()->GetProfileInt(_T("Settings"), _T("Renderer"), 0))
	{
	case 0: s_gs = new GSRendererHW(s_basemem, !!mt, s_irq, nloophack); break;
	case 1: s_gs = new GSRendererSWFP(s_basemem, !!mt, s_irq, nloophack); break;
	case 3: s_gs = new GSRendererNull(s_basemem, !!mt, s_irq, nloophack); break;
	default: return -1;
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
	return s_gs->MakeSnapshot(path);
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

	if(IDOK == GSSettingsDlg().DoModal())
	{
		GSshutdown();
		GSinit();
	}
}

EXPORT_C_(INT32) GStest()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	int ret = 0;

	D3DCAPS9 caps;
	memset(&caps, 0, sizeof(caps));
	caps.PixelShaderVersion = D3DPS_VERSION(0, 0);

	if(CComPtr<IDirect3D9> pD3D = Direct3DCreate9(D3D_SDK_VERSION))
	{
		pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);

		LPCTSTR yep = _T("^_^"), nope = _T(":'(");

		CString str, tmp;

		if(caps.VertexShaderVersion < D3DVS_VERSION(2, 0))
			ret = -1;

		tmp.Format(_T("%s Vertex Shader version %d.%d\n"), 
			caps.VertexShaderVersion >= D3DVS_VERSION(2, 0) ? yep : nope,
			D3DSHADER_VERSION_MAJOR(caps.VertexShaderVersion),
			D3DSHADER_VERSION_MINOR(caps.VertexShaderVersion));
		str += tmp;

		if(caps.PixelShaderVersion < D3DPS_VERSION(2, 0))
			ret = -1;

		tmp.Format(_T("%s Pixel Shader version %d.%d\n"), 
			caps.PixelShaderVersion >= D3DPS_VERSION(2, 0) ? yep : nope,
			D3DSHADER_VERSION_MAJOR(caps.PixelShaderVersion),
			D3DSHADER_VERSION_MINOR(caps.PixelShaderVersion));
		str += tmp;

		if(!(caps.PrimitiveMiscCaps & D3DPMISCCAPS_SEPARATEALPHABLEND))
			ret = -1;

		tmp.Format(_T("%s Separate Alpha Blend\n"), 
			!!(caps.PrimitiveMiscCaps & D3DPMISCCAPS_SEPARATEALPHABLEND) ? yep : nope);
		str += tmp;

		if(!(caps.SrcBlendCaps & D3DPBLENDCAPS_BLENDFACTOR)
		|| !(caps.DestBlendCaps & D3DPBLENDCAPS_BLENDFACTOR))
			ret = -1;

		tmp.Format(_T("%s Source Blend Factor\n"), 
			!!(caps.SrcBlendCaps & D3DPBLENDCAPS_BLENDFACTOR) ? yep : nope);
		str += tmp;

		tmp.Format(_T("%s Destination Blend Factor\n"), 
			!!(caps.DestBlendCaps & D3DPBLENDCAPS_BLENDFACTOR) ? yep : nope);
		str += tmp;

		AfxMessageBox(str);
	}
	else
	{
		ret = -1;
	}

	return ret;
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
