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
#include "GSCapture.h"
#include "GSVector.h"

//
// GSSource
//

#ifdef __INTEL_COMPILER
interface __declspec(uuid("59C193BB-C520-41F3-BC1D-E245B80A86FA")) 
#else
[uuid("59C193BB-C520-41F3-BC1D-E245B80A86FA")] interface
#endif
IGSSource : public IUnknown
{
	STDMETHOD(DeliverNewSegment)() PURE;
	STDMETHOD(DeliverFrame)(const void* bits, int pitch) PURE;
	STDMETHOD(DeliverEOS)() PURE;
};

#ifdef __INTEL_COMPILER
class __declspec(uuid("F8BB6F4F-0965-4ED4-BA74-C6A01E6E6C77"))
#else
[uuid("F8BB6F4F-0965-4ED4-BA74-C6A01E6E6C77")] class 
#endif
GSSource : public CBaseFilter, private CCritSec, public IGSSource
{
	CSize m_size;
	REFERENCE_TIME m_atpf;
	REFERENCE_TIME m_now;
	CAutoPtr<CBaseOutputPin> m_output;

	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv)
	{
		return 
			QI(IGSSource)
			__super::NonDelegatingQueryInterface(riid, ppv);
	}

	class GSSourceOutputPin : public CBaseOutputPin
	{
		CMediaType m_mt;

	public:
		GSSourceOutputPin(CMediaType& mt, CBaseFilter* pFilter, CCritSec* pLock, HRESULT& hr)
			: CBaseOutputPin("GSSourceOutputPin", pFilter, pLock, &hr, L"Output")
			, m_mt(mt)
		{
		}

		HRESULT GSSourceOutputPin::DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pProperties)
		{
			ASSERT(pAlloc && pProperties);

			HRESULT hr;

			pProperties->cBuffers = 1;
			pProperties->cbBuffer = m_mt.lSampleSize;

			ALLOCATOR_PROPERTIES Actual;
			
			if(FAILED(hr = pAlloc->SetProperties(pProperties, &Actual)))
			{
				return hr;
			}

			if(Actual.cbBuffer < pProperties->cbBuffer)
			{
				return E_FAIL;
			}

			ASSERT(Actual.cBuffers == pProperties->cBuffers);

			return S_OK;
		}

	    HRESULT CheckMediaType(const CMediaType* pmt)
		{
			return pmt->majortype == MEDIATYPE_Video && pmt->subtype == MEDIASUBTYPE_RGB32 ? S_OK : E_FAIL;
		}

	    HRESULT GetMediaType(int iPosition, CMediaType* pmt)
		{
			CheckPointer(pmt, E_POINTER);
			if(iPosition < 0) return E_INVALIDARG;
			if(iPosition > 0) return VFW_S_NO_MORE_ITEMS;
			*pmt = m_mt;
			return S_OK;
		}

		STDMETHODIMP Notify(IBaseFilter* pSender, Quality q)
		{
			return E_NOTIMPL;
		}
	};

public:

	GSSource(int w, int h, int fps, IUnknown* pUnk, HRESULT& hr)
		: CBaseFilter(NAME("GSSource"), pUnk, this, __uuidof(this), &hr)
		, m_output(NULL)
		, m_size(w, h)
		, m_atpf(10000000i64 / fps)
		, m_now(0)
	{
		int size = w * h * 4;

		CMediaType mt;

		mt.majortype = MEDIATYPE_Video;
		mt.subtype = MEDIASUBTYPE_RGB32;
		mt.formattype = FORMAT_VideoInfo;
		mt.lSampleSize = size;

		VIDEOINFOHEADER vih;
		memset(&vih, 0, sizeof(vih));
		vih.AvgTimePerFrame = m_atpf;
		vih.bmiHeader.biSize = sizeof(vih.bmiHeader);
		vih.bmiHeader.biCompression = BI_RGB;
		vih.bmiHeader.biPlanes = 1;
		vih.bmiHeader.biWidth = w;
		vih.bmiHeader.biHeight = -h;
		vih.bmiHeader.biBitCount = 32;
		vih.bmiHeader.biSizeImage = size;
		mt.SetFormat((BYTE*)&vih, sizeof(vih));

		m_output.Attach(new GSSourceOutputPin(mt, this, this, hr));
	}

	DECLARE_IUNKNOWN;

	int GetPinCount()
	{
		return 1;
	}

	CBasePin* GetPin(int n) 
	{
		return n == 0 ? m_output.m_p : NULL;
	}

	// IGSSource

	STDMETHODIMP DeliverNewSegment()
	{
		m_now = 0;

		return m_output->DeliverNewSegment(0, _I64_MAX, 1.0);
	}

	STDMETHODIMP DeliverFrame(const void* bits, int pitch)
	{
		if(!m_output || !m_output->IsConnected())
		{
			return E_UNEXPECTED;
		}

		CComPtr<IMediaSample> sample;

		if(FAILED(m_output->GetDeliveryBuffer(&sample, NULL, NULL, 0)))
		{
			return E_FAIL;
		}

		REFERENCE_TIME start = m_now;
		REFERENCE_TIME stop = m_now + m_atpf;

		sample->SetTime(&start, &stop);
		sample->SetSyncPoint(TRUE);

		CMediaType mt;
		m_output->GetMediaType(0, &mt);

		BYTE* src = (BYTE*)bits;
		int srcpitch = pitch;

		BYTE* dst = NULL;
		sample->GetPointer(&dst);
		int dstpitch = ((VIDEOINFOHEADER*)mt.Format())->bmiHeader.biWidth * 4;

		int w = m_size.cx;
		int h = m_size.cy;

		for(int j = 0; j < h; j++, dst += dstpitch, src += srcpitch)
		{
			#if _M_SSE >= 0x301

			GSVector4i* s = (GSVector4i*)src;
			GSVector4i* d = (GSVector4i*)dst;

			GSVector4i mask(2, 1, 0, 3, 6, 5, 4, 7, 10, 9, 8, 11, 14, 13, 12, 15);

			for(int i = 0, w4 = w >> 2; i < w4; i++)
			{
				d[i] = s[i].shuffle8(mask);
			}

			#elif _M_SSE >= 0x200

			GSVector4i* s = (GSVector4i*)src;
			GSVector4i* d = (GSVector4i*)dst;

			for(int i = 0, w4 = w >> 2; i < w4; i++)
			{
				d[i] = ((s[i] & 0x00ff0000) >> 16) | ((s[i] & 0x000000ff) << 16) | (s[i] & 0x0000ff00);
			}

			#else

			DWORD* s = (DWORD*)src;
			DWORD* d = (DWORD*)dst;
			
			for(int i = 0; i < w; i++)
			{
				d[i] = ((s[i] & 0x00ff0000) >> 16) | ((s[i] & 0x000000ff) << 16) | (s[i] & 0x0000ff00);
			}

			#endif
		}

		if(FAILED(m_output->Deliver(sample)))
		{
			return E_FAIL;
		}

		m_now = stop;

		return S_OK;
	}

	STDMETHODIMP DeliverEOS()
	{
		return m_output->DeliverEndOfStream();
	}
};

//
// GSCapture
//

GSCapture::GSCapture()
	: m_capturing(false)
{
}

GSCapture::~GSCapture()
{
	EndCapture();
}

bool GSCapture::BeginCapture(int fps)
{
	CAutoLock cAutoLock(this);

	ASSERT(fps != 0);

	EndCapture();

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	GSCaptureDlg dlg;

	dlg.DoModal(); // if(IDOK != dlg.DoModal()) return false;

	// TODO: get dimension through the dialog box

	m_size.cx = (dlg.m_width + 7) & ~7;
	m_size.cy = (dlg.m_height + 7) & ~7; 

	//

	CComPtr<ICaptureGraphBuilder2> cgb;
	CComPtr<IBaseFilter> mux;

	if(FAILED(m_graph.CoCreateInstance(CLSID_FilterGraph))
	|| FAILED(cgb.CoCreateInstance(CLSID_CaptureGraphBuilder2))
	|| FAILED(cgb->SetFiltergraph(m_graph))
	|| FAILED(cgb->SetOutputFileName(&MEDIASUBTYPE_Avi, CStringW(dlg.m_filename), &mux, NULL)))
	{
		return false;
	}

	HRESULT hr;

	m_src = new GSSource(m_size.cx, m_size.cy, fps, NULL, hr);

	if(FAILED(hr = m_graph->AddFilter(m_src, L"Source"))
	|| FAILED(hr = m_graph->AddFilter(dlg.m_enc, L"Encoder"))
	|| FAILED(hr = cgb->RenderStream(NULL, NULL, m_src, dlg.m_enc, mux)))
	{
		return false;
	}

	CComQIPtr<IMediaControl>(m_graph)->Run();

	CComQIPtr<IGSSource>(m_src)->DeliverNewSegment();

	m_capturing = true;

	return true;
}

bool GSCapture::DeliverFrame(const void* bits, int pitch)
{
	CAutoLock cAutoLock(this);

	if(bits == NULL || pitch == 0) 
	{
		ASSERT(0); 

		return false;
	}

	if(m_src)
	{
		CComQIPtr<IGSSource>(m_src)->DeliverFrame(bits, pitch);

		return true;
	}

	return false;
}

bool GSCapture::EndCapture()
{
	CAutoLock cAutoLock(this);

	if(m_src)
	{
		CComQIPtr<IGSSource>(m_src)->DeliverEOS();

		m_src = NULL;
	}

	if(m_graph)
	{
		CComQIPtr<IMediaControl>(m_graph)->Stop();

		m_graph = NULL;
	}

	m_capturing = false;

	return true;
}
