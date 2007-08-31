/*
* Copyright (c) 2007, Wirtualna Polska S.A.
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/
//! @file RawVideoRenderer.cpp
//! Implementation of RawVideoRenderer & its sub-interfaces
//! @author Andrzej Ciarkowski <mailto:andrzejc@wp-sa.pl>
#include "stdafx.h"
#include "RawVideoRenderer.h"
#include "MediaSampleSink.h"

#include <initguid.h>
#include "RawVideoRendererUids.h"

class RawVideoRendererFilter : public CBaseFilter
{
public:

	// Constructor
	RawVideoRendererFilter(RawVideoRenderer& renderer, LPUNKNOWN pUnk, CCritSec& lock, HRESULT* phr);

	// Pin enumeration
	CBasePin* GetPin(int n);
	int GetPinCount();

	// Open and close the file as necessary
	STDMETHODIMP Run(REFERENCE_TIME tStart);
	STDMETHODIMP Pause();
	STDMETHODIMP Stop();

private:
	RawVideoRenderer& renderer_;
	CCritSec& lock_;
};


class RawVideoRendererInputPin : public CRenderedInputPin
{
public:

	RawVideoRendererInputPin(RawVideoRenderer& renderer, LPUNKNOWN pUnk, CBaseFilter& filter, CCritSec& lock, CCritSec& receiveLock, HRESULT* phr);

	// Do something with this media sample
	STDMETHODIMP Receive(IMediaSample *pSample);
	STDMETHODIMP EndOfStream(void);
	STDMETHODIMP ReceiveCanBlock();

	// Write detailed information about this sample to a file
	// HRESULT WriteStringInfo(IMediaSample *pSample);

	// Check if the pin can support this specific proposed type and format
	HRESULT CheckMediaType(const CMediaType* );

	// Break connection
	HRESULT BreakConnect();

	// Track NewSegment
	STDMETHODIMP NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

	void SetMediaSampleSink(MediaSamplePushSink* sink) 
	{
		sink_ = sink;
	}

private:
	RawVideoRenderer& renderer_;           // Main renderer object
	CCritSec& receiveLock_;    // Sample critical section
	REFERENCE_TIME lastSampleTime_;             // Last sample receive time
	MediaSamplePushSink* sink_;
};

// Setup data

const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
	&MEDIATYPE_NULL,            // Major type
	&MEDIASUBTYPE_NULL          // Minor type
};

const AMOVIESETUP_PIN sudPins =
{
	L"Input",                   // Pin string name
	FALSE,                      // Is it rendered
	FALSE,                      // Is it an output
	FALSE,                      // Allowed none
	FALSE,                      // Likewise many
	&CLSID_NULL,                // Connects to filter
	L"Output",                  // Connects to pin
	1,                          // Number of types
	&sudPinTypes                // Pin information
};

const AMOVIESETUP_FILTER sudDump =
{
	&CLSID_RawVideoRenderer,                // Filter CLSID
	L"Raw Video Renderer",                    // String name
	MERIT_DO_NOT_USE,           // Filter merit
	1,                          // Number pins
	&sudPins                    // Pin details
};


RawVideoRendererFilter::RawVideoRendererFilter(RawVideoRenderer& renderer, LPUNKNOWN pUnk, CCritSec& lock, HRESULT* phr):
	CBaseFilter(NAME("RawVideoRendererFilter"), pUnk, &lock, CLSID_RawVideoRenderer),
		renderer_(renderer),
		lock_(lock)
	{
	}


	CBasePin* RawVideoRendererFilter::GetPin(int n)
	{
		if (n == 0) 
		{
			return renderer_.pin_.get();
		} 
		else 
		{
			return NULL;
		}
	}


	int RawVideoRendererFilter::GetPinCount()
	{
		return 1;
	}


	STDMETHODIMP RawVideoRendererFilter::Stop()
	{
		//CAutoLock cObjectLock(&lock_);
		return CBaseFilter::Stop();
	}


	STDMETHODIMP RawVideoRendererFilter::Pause()
	{
		//CAutoLock cObjectLock(&lock_);
		return CBaseFilter::Pause();
	}


	STDMETHODIMP RawVideoRendererFilter::Run(REFERENCE_TIME tStart)
	{
		//CAutoLock cObjectLock(&lock_);
		return CBaseFilter::Run(tStart);
	}


	RawVideoRendererInputPin::RawVideoRendererInputPin(RawVideoRenderer& renderer, LPUNKNOWN pUnk, CBaseFilter& filter, CCritSec& lock, CCritSec& receiveLock, HRESULT* phr):
	CRenderedInputPin(NAME("RawVideoRendererInputPin"), &filter, &lock, phr, L"Input"),
		receiveLock_(receiveLock),
		renderer_(renderer),
		lastSampleTime_(0),
		sink_(NULL)
	{
	}

	HRESULT RawVideoRendererInputPin::CheckMediaType(const CMediaType* type)
	{
//		bool bottomUp = false;
//		if (!IsEqualGUID(type->formattype, FORMAT_VideoInfo) && !IsEqualGUID(type->formattype, FORMAT_VideoInfo2))
//			goto Finish;
//	
//		if (sizeof(VIDEOINFOHEADER) > type->FormatLength()|| NULL == type->Format())
//			goto Finish;
//	
//		VIDEOINFOHEADER* vi = (VIDEOINFOHEADER*)type->Format();
//		BITMAPINFOHEADER& bi = vi->bmiHeader;
//		if (bi.biHeight < 0)
//			bottomUp = true;
//
//Finish:
		return S_OK;
	}


	HRESULT RawVideoRendererInputPin::BreakConnect()
	{
		if (NULL != renderer_.position_.get()) 
			renderer_.position_->ForceRefresh();

		return CRenderedInputPin::BreakConnect();
	}


	STDMETHODIMP RawVideoRendererInputPin::ReceiveCanBlock()
	{
		return S_FALSE;
	}


	STDMETHODIMP RawVideoRendererInputPin::Receive(IMediaSample* pSample)
	{
		CheckPointer(pSample, E_POINTER);

		//{
		//	CAutoLock lock(&receiveLock_);
		//	PBYTE pbData;

		//	REFERENCE_TIME tStart, tStop;
		//	pSample->GetTime(&tStart, &tStop);

		//	//TCHAR buffer[128];
		//	//_stprintf(buffer, TEXT("tStart(%s), tStop(%s), Diff(%d ms), Bytes(%d)\n"), (LPCTSTR) CDisp(tStart), (LPCTSTR) CDisp(tStop), (LONG)((tStart - lastSampleTime_) / 10000), pSample->GetActualDataLength());
		//	//OutputDebugString(buffer);

		//	lastSampleTime_ = tStart;

		//	HRESULT hr = pSample->GetPointer(&pbData);
		//	if (FAILED(hr)) 
		//	{
		//		return hr;
		//	}
		//}

		if (NULL != sink_)
			sink_->PushSample(pSample);

		return S_OK; 
	}



	STDMETHODIMP RawVideoRendererInputPin::EndOfStream(void)
	{
		//CAutoLock lock(&receiveLock_);
		return CRenderedInputPin::EndOfStream();

	}


	STDMETHODIMP RawVideoRendererInputPin::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
	{
		lastSampleTime_ = 0;
		return S_OK;
	}


	RawVideoRenderer::RawVideoRenderer(LPUNKNOWN pUnk, HRESULT* phr):
	CUnknown(NAME("RawVideoRenderer"), pUnk)
	{
		ASSERT(phr);
		lock_.reset(new (std::nothrow) CCritSec());
		if (NULL == lock_.get())
		{
			if (NULL != phr)
				*phr = E_OUTOFMEMORY;

			return;
		}

		receiveLock_.reset(new (std::nothrow) CCritSec());
		if (NULL == receiveLock_.get())
		{
			if (NULL != phr)
				*phr = E_OUTOFMEMORY;

			return;
		}


		filter_.reset(new (std::nothrow) RawVideoRendererFilter(*this, GetOwner(), *lock_, phr));
		if (NULL == filter_.get()) 
		{
			if (NULL != phr)
				*phr = E_OUTOFMEMORY;

			return;
		}

		pin_.reset(new (std::nothrow) RawVideoRendererInputPin(*this, GetOwner(), *filter_, *lock_, *receiveLock_, phr));
		if (NULL == pin_.get()) 
		{
			if (NULL != phr)
				*phr = E_OUTOFMEMORY;

			return;
		}
	}

	void RawVideoRenderer::SetMediaSampleSink(MediaSamplePushSink* sink)
	{
		if (NULL != pin_.get())
			pin_->SetMediaSampleSink(sink);
	}

	RawVideoRenderer::~RawVideoRenderer()
	{
		pin_.reset();
		filter_.reset();
		position_.reset();
	}


	CUnknown * WINAPI RawVideoRenderer::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
	{
		ASSERT(phr);

		RawVideoRenderer* pNewObject = new (std::nothrow) RawVideoRenderer(punk, phr);
		if (NULL == pNewObject) 
		{
			if (NULL != phr)
				*phr = E_OUTOFMEMORY;
		}

		return pNewObject;
	}


	STDMETHODIMP RawVideoRenderer::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
	{
		CheckPointer(ppv,E_POINTER);
		CAutoLock lock(lock_.get());
		if (riid == IID_IBaseFilter || riid == IID_IMediaFilter || riid == IID_IPersist) 
		{
			return filter_->NonDelegatingQueryInterface(riid, ppv);
		} 
		else if (riid == IID_IMediaPosition || riid == IID_IMediaSeeking) 
		{
			if (NULL == position_.get()) 
			{
				HRESULT hr = S_OK;
				position_.reset(new (std::nothrow) CPosPassThru(NAME("RawVideoRenderer Pass Through"), GetOwner(), &hr, pin_.get()));
				if (NULL == position_.get()) 
					return E_OUTOFMEMORY;

				if (FAILED(hr)) 
				{
					position_.reset();
					return hr;
				}
			}

			return position_->NonDelegatingQueryInterface(riid, ppv);
		} 

		return CUnknown::NonDelegatingQueryInterface(riid, ppv);

	}
