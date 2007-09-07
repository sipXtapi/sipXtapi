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
//! @file VideoCapture.cpp
//! Implementation of @c VideoCapture class.
//! @author Andrzej Ciarkowski <mailto:andrzejc@wp-sa.pl>
#include "stdafx.h"
#include <set>

// #define VIDEO_CAPTURE_NOTIFY_WINDOW
#define VIDEO_CAPTURE_NOTIFY_THREAD

#if defined(VIDEO_CAPTURE_NOTIFY_WINDOW) && defined(VIDEO_CAPTURE_NOTIFY_THREAD)
# error "Only one notification mechanism may be selected"
#endif 

#include <VideoSupport/VideoFormat.h>
#include <VideoSupport/VideoCapture.h>
#include <VideoSupport/DepsAutoLink.h>
#include <VideoSupport/VideoSurfaceConverter.h>
#include <VideoSupport/VideoSurfaceConverterFactory.h>
#include <VideoSupport/VideoCaptureSink.h>
#include <VideoSupport/VideoFrameProcessor.h>
#include <VideoSupport/VideoProcessorFactory.h>
#include <VideoSupport/VideoScaler.h>
#include "VideoFrameFlipperImpl.h"

#include <map>

#include "RawVideoRenderer.h"
#include "MediaSampleSink.h"
#include "utils.h"

_COM_SMARTPTR_TYPEDEF(ICaptureGraphBuilder2, __uuidof(ICaptureGraphBuilder2));
_COM_SMARTPTR_TYPEDEF(IGraphBuilder, __uuidof(IGraphBuilder));
_COM_SMARTPTR_TYPEDEF(ICreateDevEnum, __uuidof(ICreateDevEnum));
_COM_SMARTPTR_TYPEDEF(IEnumMoniker, __uuidof(IEnumMoniker));
_COM_SMARTPTR_TYPEDEF(IMoniker, __uuidof(IMoniker));
_COM_SMARTPTR_TYPEDEF(IPropertyBag, __uuidof(IPropertyBag));
_COM_SMARTPTR_TYPEDEF(IBaseFilter, __uuidof(IBaseFilter));
_COM_SMARTPTR_TYPEDEF(IAMStreamConfig, __uuidof(IAMStreamConfig));
_COM_SMARTPTR_TYPEDEF(IAMStreamControl, __uuidof(IAMStreamControl));
_COM_SMARTPTR_TYPEDEF(IMediaControl, __uuidof(IMediaControl));
_COM_SMARTPTR_TYPEDEF(IMediaEvent, __uuidof(IMediaEvent));
_COM_SMARTPTR_TYPEDEF(IMediaEventEx, __uuidof(IMediaEventEx));
_COM_SMARTPTR_TYPEDEF(IVideoWindow, __uuidof(IVideoWindow));

namespace 
{

	static HRESULT CreateGraphBuilder(IGraphBuilderPtr& graph, ICaptureGraphBuilder2Ptr& build)
	{
		HRESULT res;
		if (FAILED(res = build.CreateInstance(CLSID_CaptureGraphBuilder2)))
			return res;

		assert(NULL != build.GetInterfacePtr());
		if (FAILED(res = graph.CreateInstance(CLSID_FilterGraph)))
		{
			build.Release();
			return res;
		}

		assert(NULL != graph.GetInterfacePtr());
		build->SetFiltergraph(graph);
		return S_OK;
	}

	static HRESULT CreateDeviceEnum(IEnumMonikerPtr& devEnum)
	{
		HRESULT res;
		ICreateDevEnumPtr dev;
		if (FAILED(res = dev.CreateInstance(CLSID_SystemDeviceEnum)))
			return res;

		res = dev->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &devEnum, 0);
		return res;
	}

	static HRESULT GetFilterName(IPropertyBagPtr& props, std::string& name)
	{
		HRESULT hr;

		_variant_t var;
		if (FAILED(hr = props->Read(L"Description", &var, NULL)) && FAILED(hr = props->Read(L"FriendlyName", &var, NULL)))
			return hr;

		_bstr_t str = var;
		const char* n = str;
		if (NULL == n)
			return E_FAIL;

		name = n;
		return S_OK;
	}

	static HRESULT FindEnumCaptureDevices(VideoCapture::DeviceNames* names, const std::string* findName, IBaseFilterPtr& filter)
	{
		HRESULT hr;
		
		IEnumMonikerPtr devEnum;
		if (FAILED(hr = CreateDeviceEnum(devEnum)))
			return hr;

		if (S_FALSE == hr)
			return E_FAIL;

		assert(NULL != devEnum.GetInterfacePtr());
		IMonikerPtr moniker;
		while (S_OK == devEnum->Next(1, &moniker, NULL))
		{
			assert(NULL != moniker.GetInterfacePtr());

			IPropertyBagPtr props;
			if (FAILED(hr = moniker->BindToStorage(NULL, NULL, props.GetIID(), (void**)&props)))
				continue;

			assert(NULL != props.GetInterfacePtr());
			std::string name;

			if (FAILED(hr = GetFilterName(props, name)))
				continue;

			if (NULL != names)
				names->push_back(name);
			else if (NULL != findName)
			{
				if (*findName != name)
					continue;

				if (FAILED(hr = moniker->BindToObject(NULL, NULL, filter.GetIID(), (void**)&filter)))
					return hr;

				assert(NULL != filter.GetInterfacePtr());
				return S_OK;
			}
		}

		if (NULL != findName)
			return E_FAIL;

		return S_OK;
	}

	static HRESULT CreateRawRenderer(IBaseFilterPtr& renderer, MediaSamplePushSink& sink)
	{
		HRESULT hr = S_OK;
		CUnknown* pUnk = RawVideoRenderer::CreateInstance(NULL, &hr);
		if (FAILED(hr))
			return hr;

		assert(NULL != pUnk);
		
		RawVideoRenderer* r = dynamic_cast<RawVideoRenderer*>(pUnk);
		assert(NULL != r);
		if (NULL != r)
			r->SetMediaSampleSink(&sink);

		INonDelegatingUnknown* ndUnk = pUnk;
		IUnknownPtr unknown(reinterpret_cast<IUnknown*>(ndUnk));
		if (FAILED(hr = unknown.QueryInterface(IID_IBaseFilter, &renderer)))
			return hr;

		assert(NULL != renderer.GetInterfacePtr());
		return S_OK;
	}

	static UINT GetVideoCaptureMessage()
	{
		static UINT msg = ::RegisterWindowMessageA("VideoCapture Notify Message {B1E97A14-8F5C-4749-9AA8-D432E000E90D}");
		return msg;
	}

}

void VideoCapture::EnumDevices(VideoCapture::DeviceNames &names)
{
	CoInitializer coInit;

	IBaseFilterPtr dummy;
	FindEnumCaptureDevices(&names, NULL, dummy);
}

struct VideoCapture::Impl: public MediaSamplePushSink
{
	CoInitializer coInit_;
	VideoCapture& owner_;
	IBaseFilterPtr capture_;
	IGraphBuilderPtr graph_;
	ICaptureGraphBuilder2Ptr build_;
	IAMStreamConfigPtr streamConfig_;
	IMediaControlPtr mediaControl_;
	IMediaEventExPtr mediaEvent_;
	IBaseFilterPtr render_;
	IVideoWindowPtr videoWindow_;
	HWND previewWindow_;
	mutable std::auto_ptr<VideoFormat> captureFormatCache_;
	std::auto_ptr<VideoFormat> outputFormat_;

	VideoSurfaceConverterAutoPtr converter_;
	size_t converterBufferSize_;
	void* converterBuffer_;
	std::auto_ptr<VideoScaler> scaler_;
	void* scalerBuffer_;
	size_t scalerBufferSize_;

#ifdef VIDEO_CAPTURE_NOTIFY_THREAD
	HANDLE eventThread_;
	HANDLE mediaEventHandle_;
	std::auto_ptr<CAMEvent> threadStopEvent_;
#elif defined(VIDEO_CAPTURE_NOTIFY_WINDOW)
	WNDPROC previewWindowProc_;
#endif // VIDEO_CAPTURE_NOTIFY_THREAD

	std::auto_ptr<CCritSec> lock_;
	State state_;
	volatile bool running_;
	bool bottomUp_;
	bool preview_;
	VideoCaptureSink* sink_;

	std::auto_ptr<VideoFrameInPlaceProcessor> flipper_;

	explicit Impl(VideoCapture& owner): 
	owner_(owner),
	previewWindow_(NULL),

#ifdef VIDEO_CAPTURE_NOTIFY_THREAD
	eventThread_(NULL),
	mediaEventHandle_(NULL),
#elif defined(VIDEO_CAPTURE_NOTIFY_WINDOW)
	previewWindowProc_(NULL),
#endif // VIDEO_CAPTURE_NOTIFY_THREAD

	state_(stateUninitialized),
	running_(false),
	bottomUp_(false),
	preview_(false),
	converterBufferSize_(0),
	converterBuffer_(NULL),
	scalerBufferSize_(0),
	scalerBuffer_(NULL),
	sink_(NULL)
	{
		lock_.reset(new CCritSec());
	}

	~Impl()
	{
	}

	bool Initialize(const std::string& deviceName, HWND previewWindow, long flags);

	State GetState() const
	{
		CAutoLock lock(lock_.get());
		return state_;
	}

	bool SetPreviewWindow(HWND previewWindow);

	void Stop();

	bool StopNoLock();

	void Close();

	bool Run();

	bool GetCaptureFormat(VideoFormat& format) const;
	bool GetOutputFormat(VideoFormat& format) const;

	bool CommitOutputFormatNoLock();
	bool GetBestCaptureFormat(const VideoFormat& outputFormat, VideoFormat& captureFormat) const;
	bool SetOutputFormat(const VideoFormat* format, bool adjustCaptureFormat);

	bool MatchFormat(const VideoFormat& format, bool set);
	bool MatchFormat(const VideoFormat& format, AM_MEDIA_TYPE& mediaType, VIDEO_STREAM_CONFIG_CAPS& caps, bool set);

	void PushSample(IMediaSample* sample);

	void ProcessMediaEvent(long event, LONG_PTR param1, LONG_PTR param2);

	bool SetSink(VideoCaptureSink* sink);

	void AdjustVideoWindowSize();

	bool EnumCaptureSurfaces(VideoSurfaces& surfaces) const;

#ifdef VIDEO_CAPTURE_NOTIFY_THREAD

	static unsigned _stdcall ThreadWorker(void* self);
	void Worker();

#elif defined(VIDEO_CAPTURE_NOTIFY_WINDOW)

	typedef std::map<HWND, VideoCapture::Impl*> WindowHandleMap;
	static WindowHandleMap* windowHandles_;
	static CCritSec* windowHandlesLock_;

	static Impl* FindByWindow(HWND window);
	static void AssociateWindow(HWND window, Impl* impl);
	static void DeassociateWindow(HWND window);

	static LRESULT CALLBACK SubclassPreviewProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	bool PreFilterMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& res);
	void PostFilterMessage(UINT msg, WPARAM wParam, LPARAM lParam);

#endif // VIDEO_CAPTURE_NOTIFY_WINDOW

	void ConverterDispose();
	void ScalerDispose();

	void BestFormatMatch(const AM_MEDIA_TYPE& type, const VIDEO_STREAM_CONFIG_CAPS& caps, const VideoFormat& format, size_t& prevW, size_t& prevH, size_t& nextW, size_t& nextH, VideoSurface& prevSurface, VideoSurface& nextSurface) const;
};


#ifdef VIDEO_CAPTURE_NOTIFY_WINDOW

VideoCapture::Impl::WindowHandleMap* VideoCapture::Impl::windowHandles_ = NULL;
CCritSec* VideoCapture::Impl::windowHandlesLock_ = NULL;

#endif // VIDEO_CAPTURE_NOTIFY_WINDOW

bool VideoCapture::Impl::Initialize(const std::string& deviceName, HWND previewWindow, long flags)
{
	CAutoLock lock(lock_.get());
	if (stateUninitialized != state_)
		return false;

	HRESULT hr;
	// create both graph and its helper builder
	if (FAILED(hr = CreateGraphBuilder(graph_, build_)))
		goto Fail;

	assert(NULL != graph_.GetInterfacePtr());
	assert(NULL != build_.GetInterfacePtr());
	// create video device of specified name
	if (FAILED(hr = FindEnumCaptureDevices(NULL, &deviceName, capture_)))
		goto Fail;

	assert(NULL != capture_.GetInterfacePtr());
	// create our special raw renderer and set this as its sample sink
	if (FAILED(hr = CreateRawRenderer(render_, *this)))
		goto Fail;

	assert(NULL != render_.GetInterfacePtr());
	// start creating graph by adding source capture filter
	if (FAILED(hr = graph_->AddFilter(capture_, L"Source")))
		goto Fail;

	// add raw sample renderer to graph
	if (FAILED(hr = graph_->AddFilter(render_, L"Sink")))
		goto Fail;

	// connect our magic raw sample renderer to capture pin
	if (FAILED(hr = build_->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, capture_, NULL, render_)))
		goto Fail;

	bool preview = (0 == (flags & flagPreviewDisabled));
	if (preview)
	{
		// create preview renderer & connect it to preview pin
		if (FAILED(hr = build_->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, capture_, NULL, NULL)))
			goto Fail;

		// obtain IVideoWindow interface for previewing the video
		if (FAILED(hr = graph_.QueryInterface(videoWindow_.GetIID(), &videoWindow_)))
			goto Fail;

		assert(NULL != videoWindow_.GetInterfacePtr());
	}

	// obtain IMediaControl interface for controlling the graph
	if (FAILED(hr = graph_.QueryInterface(mediaControl_.GetIID(), &mediaControl_)))
		goto Fail;

	assert(NULL != mediaControl_.GetInterfacePtr());

	// and IMediaEvent for getting event notifications
	if (FAILED(hr = graph_.QueryInterface(mediaEvent_.GetIID(), &mediaEvent_)))
		goto Fail;

	assert(NULL != mediaEvent_.GetInterfacePtr());

	// find IAMStreamConfig for configuring *capture* stream
	if (FAILED(hr = build_->FindInterface(&PIN_CATEGORY_CAPTURE, NULL, capture_, streamConfig_.GetIID(), (void**)&streamConfig_)))
		goto Fail;

	assert(NULL != streamConfig_.GetInterfacePtr());

#ifdef VIDEO_CAPTURE_NOTIFY_THREAD

	assert(NULL == mediaEventHandle_);
	// obtain event handle from IMediaEvent, for use by event thread
	if (FAILED(hr = mediaEvent_->GetEventHandle((OAEVENT*)&mediaEventHandle_)))
		goto Fail;

	// create event which will be used to stop the event processing thread
	threadStopEvent_.reset(new (std::nothrow) CAMEvent());
	if (NULL == threadStopEvent_.get())
		goto Fail;

	// create the event thread, initially suspended
	assert(NULL == eventThread_);
	unsigned threadId = 0;
	eventThread_ = (HANDLE)_beginthreadex(NULL, 0, ThreadWorker, this, CREATE_SUSPENDED, &threadId);
	if (NULL == eventThread_)
		goto Fail;

	// event thread will sleep most of the time, only waking to handle the events. it should be safe
	// to set thread priority to realtime, so that events would be handled responsively
	::SetThreadPriority(eventThread_, THREAD_PRIORITY_TIME_CRITICAL);

	// resume the thread
	if (DWORD(-1) == ::ResumeThread(eventThread_))
	{
		// if some shit happens we have to kill the thread as it's unusable
		::TerminateThread(eventThread_, DWORD(-1));
		::CloseHandle(eventThread_);
		eventThread_ = NULL;
		goto Fail;
	}

#endif // VIDEO_CAPTURE_NOTIFY_THREAD

	state_ = stateInitialized;
	preview_ = preview;

	// once we are initialized, setup preview window
	if (!SetPreviewWindow(previewWindow))
		goto Fail;

	// this is rather unnecessary, but it doesn't hurt
	if (!CommitOutputFormatNoLock())
		goto Fail;

	return true;
Fail:
	Close();
	return false;
}

bool VideoCapture::Impl::StopNoLock()
{
	if (!running_)
		return false;

	assert(stateRunning == state_);

	// it shouldn't be possible to get running capture without IMediaControl interface
	assert(NULL != mediaControl_.GetInterfacePtr());
	HRESULT hr = mediaControl_->Stop();

	running_ = false;
	state_ = stateInitialized;
	return true;
}

void VideoCapture::Impl::Stop()
{
	CAutoLock lock(lock_.get());
	StopNoLock();
}

void VideoCapture::Impl::Close()
{
	// stop the stream (if running)
	Stop();

	// since stream is stopped, holding lock should be no longer necessary, but it won't hurt too
	CAutoLock lock(lock_.get());

#ifdef VIDEO_CAPTURE_NOTIFY_THREAD

	// set the thread stop event just in case the thread is running
	if (NULL != threadStopEvent_.get())
		threadStopEvent_->Set();

	if (NULL != eventThread_)
	{
		lock.Unlock();
		// join the thread 
		DWORD res = ::WaitForSingleObject(eventThread_, INFINITE);
		assert(WAIT_OBJECT_0 == res);

		lock.Lock();
		::CloseHandle(eventThread_);
		eventThread_ = NULL;
	}

	// now it's safe to get rid of thread stop event
	threadStopEvent_.reset();

#endif  // VIDEO_CAPTURE_NOTIFY_THREAD

	// unsubclass window, deassociate it
	SetPreviewWindow(NULL);

	// release the COM objects
	if (NULL != mediaControl_.GetInterfacePtr())
		mediaControl_.Release();

#ifdef VIDEO_CAPTURE_NOTIFY_THREAD
	mediaEventHandle_ = NULL;
#endif  // VIDEO_CAPTURE_NOTIFY_THREAD

	if (NULL != mediaEvent_.GetInterfacePtr())
		mediaEvent_.Release();

	if (NULL != videoWindow_.GetInterfacePtr())
		videoWindow_.Release();

	if (NULL != streamConfig_.GetInterfacePtr())
		streamConfig_.Release();

	if (NULL != build_.GetInterfacePtr())
		build_.Release();

	if (NULL != graph_.GetInterfacePtr())
		graph_.Release();

	if (NULL != capture_.GetInterfacePtr())
		capture_.Release();

	if (NULL != render_.GetInterfacePtr())
		render_.Release();

	outputFormat_.reset();
	CommitOutputFormatNoLock();

	state_ = stateUninitialized;
}


VideoCapture::VideoCapture():
impl_(NULL)
{
	VideoFrameFlipperImplEnsureReg();
	impl_ = new Impl(*this);
}

VideoCapture::~VideoCapture()
{
	impl_->Close();

	delete impl_;
	impl_ = NULL;
}

void VideoCapture::Stop()
{
	impl_->Stop();
}

void VideoCapture::Close()
{
	impl_->Close();
}

bool VideoCapture::Initialize(const std::string& deviceName, HWND previewWindow, long flags)
{
	return impl_->Initialize(deviceName, previewWindow, flags);
}

void VideoCapture::StaticDispose()
{

#ifdef VIDEO_CAPTURE_NOTIFY_WINDOW

	if (NULL == Impl::windowHandlesLock_)
		return;

	{
		CAutoLock lock(Impl::windowHandlesLock_);
		assert(NULL != Impl::windowHandles_);
		// there is a bug in deinitialization code if StaticDispose() is executed with some windows still associated
		assert(Impl::windowHandles_->empty());

		delete Impl::windowHandles_;
		Impl::windowHandles_ = NULL;
	}

	delete Impl::windowHandlesLock_;
	Impl::windowHandlesLock_ = NULL;

#endif // VIDEO_CAPTURE_NOTIFY_WINDOW

}

#ifdef VIDEO_CAPTURE_NOTIFY_THREAD

unsigned VideoCapture::Impl::ThreadWorker(void* self)
{
	Impl* me = static_cast<Impl*>(self);
	assert(NULL != me);

	{
		CoInitializer coInit;
		me->Worker();
	}

	_endthreadex(0);
	// never reached
	return 0;
}

void VideoCapture::Impl::Worker()
{
	// thread should never be started if we don't have IMediaEvent interface
	assert(NULL != mediaEvent_.GetInterfacePtr());
	// stop event must be present unconditionally
	assert(NULL != threadStopEvent_.get());
	// media event too
	assert(NULL != mediaEventHandle_);

	HANDLE events[] = {*threadStopEvent_, mediaEventHandle_};
	while (true)
	{
		DWORD res = ::WaitForMultipleObjects(2, events, FALSE, INFINITE);
		if (WAIT_OBJECT_0 + 1  != res)
			// stop event signalled or wait failed (serious shit)
			break;

		while (true)
		{
			// media event was signalled - IMediaEvent::GetEvent() should succeed
			HRESULT hr;
			long event;
			LONG_PTR param1, param2;
			// using 0 timeout - don't wait
			if (FAILED(hr = mediaEvent_->GetEvent(&event, &param1, &param2, 0)))
				// proceed to waiting for notifications
				break;

			ProcessMediaEvent(event, param1, param2);
			// free event params after processing the event
			hr = mediaEvent_->FreeEventParams(event, param1, param2);
		}
	}
}

#endif // VIDEO_CAPTURE_NOTIFY_THREAD

bool VideoCapture::Impl::Run()
{
	CAutoLock lock(lock_.get());
	if (stateInitialized != state_)
		return false;

	assert(!running_);
	assert(NULL != build_.GetInterfacePtr());
	assert(NULL != capture_.GetInterfacePtr());

	REFERENCE_TIME start = 0, stop = MAXLONGLONG;

	HRESULT hr;
	if (FAILED(hr = build_->ControlStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, capture_, &start, &stop, 1, 2)))
		return false;

	if (preview_ && FAILED(hr = build_->ControlStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, capture_, &start, &stop, 3, 4)))
		return false;

	// won't be initialized without IMediaControl
	assert(NULL != mediaControl_.GetInterfacePtr());
	
	if (FAILED(hr = mediaControl_->Run()))
		return false;

	running_ = true;
	state_ = stateRunning;
	return true;
}

bool VideoCapture::Run()
{
	return impl_->Run();
}

void VideoCapture::Impl::AdjustVideoWindowSize()
{
	if (!preview_)
		return;

	if (stateUninitialized == state_)
		return;

	if (NULL == previewWindow_)
		return;

	assert(NULL != videoWindow_.GetInterfacePtr());

	RECT rect;
	if (!::GetClientRect(previewWindow_, &rect))
		return;

	assert(0 == rect.left);
	assert(0 == rect.top);

	HRESULT hr = videoWindow_->SetWindowPosition(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
	assert(SUCCEEDED(hr));
}


bool VideoCapture::Impl::SetPreviewWindow(HWND previewWindow)
{
	CAutoLock lock(lock_.get());
	// ignore requests on uninitialized object
	if (stateUninitialized == state_)
		return false;

	// changing window handle to itself is a no-op, unless we change NULL to NULL 
	// (this is allowed for the purpose of proper initialization of IVideoWindow)
	if (previewWindow_ == previewWindow && NULL != previewWindow)
		return true;

	HRESULT hr;
	assert(NULL != mediaEvent_.GetInterfacePtr());

#ifdef VIDEO_CAPTURE_NOTIFY_WINDOW

	UINT msgId = GetVideoCaptureMessage();
	if (NULL != previewWindow_)
	{
		// deassociate window handle from IMediaEventEx
		hr = mediaEvent_->SetNotifyWindow(NULL, msgId, NULL);
		// unsubclass window handle
		::SetWindowLongPtr(previewWindow_, GWL_WNDPROC, reinterpret_cast<LONG_PTR>(previewWindowProc_));
		// reset stored window proc to mark it as unsubclassed
		previewWindowProc_ = NULL;
		// remove window->this association
		DeassociateWindow(previewWindow_);
	}

#endif // VIDEO_CAPTURE_NOTIFY_WINDOW

	// pretty straightforward ;)
	previewWindow_ = previewWindow;
	// DirectShow uses typedef OAHWND instead of HWND
	OAHWND oaWnd = reinterpret_cast<OAHWND>(previewWindow_);

#ifdef VIDEO_CAPTURE_NOTIFY_WINDOW

	if (NULL != previewWindow_)
	{
		// create window->this association for use by subclassed window proc
		AssociateWindow(previewWindow_, this);
		// subclass the preview window with our smart handler
		previewWindowProc_ = reinterpret_cast<WNDPROC>(::SetWindowLongPtr(previewWindow_, GWL_WNDPROC, reinterpret_cast<LONG_PTR>(&SubclassPreviewProc)));
		// associate window handle within IMediaEventEx
		hr = mediaEvent_->SetNotifyWindow(oaWnd, msgId, reinterpret_cast<LONG_PTR>(this));
		// add WS_CLIPCHILDREN to previewWindow_
		LONG ws = ::GetWindowLong(previewWindow_, GWL_STYLE);
		ws |= WS_CLIPCHILDREN;
		::SetWindowLong(previewWindow_, GWL_STYLE, ws);
	}

#endif // VIDEO_CAPTURE_NOTIFY_WINDOW

	if (!preview_)
		return true;

	assert(NULL != videoWindow_.GetInterfacePtr());

	// hide video window without parent, to prevent "mysterious window effect"
	if (NULL == previewWindow_)
	{
		hr = videoWindow_->put_Visible(OAFALSE);
		assert(SUCCEEDED(hr));
	}

	hr = videoWindow_->put_Owner(oaWnd);
	assert(SUCCEEDED(hr));
	hr = videoWindow_->put_MessageDrain(oaWnd);
	assert(SUCCEEDED(hr));

	// depending on previewWindow_, make IVideoWindow a child of it or stand-alone window
	long style = 0;
	hr = videoWindow_->get_WindowStyle(&style);
	assert(SUCCEEDED(hr));

	if (NULL != previewWindow_)
	{
		style &= ~(WS_OVERLAPPED | WS_POPUP | WS_BORDER | WS_CAPTION | WS_DLGFRAME | WS_THICKFRAME);
		style |= WS_CHILD; 
	}
	else
	{
		style &= ~(WS_CHILD);
		style |= WS_OVERLAPPEDWINDOW;
	}
	// apply window style
	hr = videoWindow_->put_WindowStyle(style);
	assert(SUCCEEDED(hr));

	// adjust size of video window to match parent's client area
	AdjustVideoWindowSize();

	// disable auto-show
	hr = videoWindow_->put_AutoShow(OAFALSE);
	assert(SUCCEEDED(hr));

	// show video window after setting the parent
	if (NULL != previewWindow_)
	{
		hr = videoWindow_->put_Visible(OATRUE);
		assert(SUCCEEDED(hr));
	}
	return true;
}

#ifdef VIDEO_CAPTURE_NOTIFY_WINDOW

VideoCapture::Impl* VideoCapture::Impl::FindByWindow(HWND window)
{
	if (NULL == windowHandlesLock_)
		return NULL;

	CAutoLock lock(windowHandlesLock_);
	assert(NULL != windowHandles_);
	if (NULL == windowHandles_)
		// some serious shit
		return NULL;

	WindowHandleMap::iterator it = windowHandles_->find(window);
	if (windowHandles_->end() == it)
		return NULL;
	else
		return it->second;
}

void VideoCapture::Impl::AssociateWindow(HWND window, Impl* impl)
{
	bool fresh = false;
	if (NULL == windowHandlesLock_)
	{
		windowHandlesLock_ = new CCritSec();
		fresh = true;
	}

	CAutoLock lock(windowHandlesLock_);
	if (fresh)
	{
		assert(NULL == windowHandles_);
		windowHandles_ = new WindowHandleMap;
	}

	assert(NULL != windowHandles_);
	assert(windowHandles_->end() == windowHandles_->find(window));
	windowHandles_->insert(std::make_pair(window, impl));
}

void VideoCapture::Impl::DeassociateWindow(HWND window)
{
	assert(NULL != windowHandlesLock_);
	// unless we have a bug in deinitialization sequence
	if (NULL == windowHandlesLock_)
		return;

	CAutoLock lock(windowHandlesLock_);
	assert(NULL != windowHandles_);
	if (NULL == windowHandles_)
		// some serious shit
		return;

	assert(windowHandles_->end() != windowHandles_->find(window));
	windowHandles_->erase(window);
}

LRESULT VideoCapture::Impl::SubclassPreviewProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Impl* self = FindByWindow(hwnd);
	assert(NULL != self);
	if (NULL == self)
		return ::DefWindowProc(hwnd, msg, wParam, lParam);

	assert(hwnd == self->previewWindow_);
	// TODO: what will happen if SetPreviewWindow() is called from any handler (PreFilter, WndProc, PostFilter)?
	LRESULT res = 0;
	if (self->PreFilterMessage(msg, wParam, lParam, res))
		return res;

	if (NULL == self->previewWindowProc_)
		res = ::DefWindowProc(hwnd, msg, wParam, lParam);
	else
		res = ::CallWindowProc(self->previewWindowProc_, hwnd, msg, wParam, lParam);

	self->PostFilterMessage(msg, wParam, lParam);
	return res;
}

bool VideoCapture::Impl::PreFilterMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& res)
{
	if (GetVideoCaptureMessage() == msg)
	{
		while (true)
		{
			HRESULT hr;
			long event;
			LONG_PTR param1, param2;
			assert(NULL != mediaEvent_.GetInterfacePtr());
			// using 0 timeout - don't wait
			if (FAILED(hr = mediaEvent_->GetEvent(&event, &param1, &param2, 0)))
				break;

			ProcessMediaEvent(event, param1, param2);
			// free event params after processing the event
			hr = mediaEvent_->FreeEventParams(event, param1, param2);
		}
		return true;
	}
	else
	{
		switch (msg) {
		case WM_ERASEBKGND:
			res = TRUE;
			return true;
		//case WM_PAINT:
		//	if (running_)
		//		::ValidateRect(previewWindow_, NULL);
		//	else
		//	{
		//		RECT rect;
		//		if (0 != ::GetUpdateRect(previewWindow_, &rect, FALSE))
		//		{
		//			PAINTSTRUCT ps;
		//			if (HDC dc = ::BeginPaint(previewWindow_, &ps))
		//			{
		//				HBRUSH brush = ::CreateSolidBrush(RGB(0, 0, 0));
		//				if (NULL != brush)
		//				{
		//					::FillRect(dc, &rect, brush);
		//					::DeleteObject(brush);
		//				}
		//				::EndPaint(previewWindow_, &ps);
		//			}
		//		}
		//	}
		//	return true;
		}
	}
	return false;
}

void VideoCapture::Impl::PostFilterMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_SIZE: 
		AdjustVideoWindowSize();
		::InvalidateRect(previewWindow_, NULL, FALSE);
		return;
	}
}

#endif // VIDEO_CAPTURE_NOTIFY_WINDOW

void VideoCapture::Impl::ProcessMediaEvent(long event, LONG_PTR param1, LONG_PTR param2)
{
	// do some shit
	switch (event) {
	case EC_COMPLETE:  // Fall through.
	case EC_USERABORT: // Fall through.
	case EC_ERRORABORT: 
	{
		CAutoLock lock(lock_.get());
		running_ = false;
		state_ = stateInitialized;
		return;
	}
	
	case EC_REPAINT:
	{
		OutputDebugStringA("Got EC_REPAINT!\n");
 		return;
	}

	}
}

bool VideoCapture::Impl::GetCaptureFormat(VideoFormat& format) const
{
	CAutoLock lock(lock_.get());
	if (stateInitialized != state_)
		return false;

	if (NULL != captureFormatCache_.get())
	{
		format = *captureFormatCache_;
		return true;
	}

	HRESULT hr;
	assert(NULL != streamConfig_.GetInterfacePtr());
	AM_MEDIA_TYPE* type = NULL;
	if (FAILED(hr = streamConfig_->GetFormat(&type)))
		return false;

	assert(NULL != type);
	VideoSurface surface = MediaSubtypeToVideoSurface(type->subtype);
	if (!IsVideoSurfaceValid(surface))
		goto Fail;

	if (!IsEqualGUID(type->formattype, FORMAT_VideoInfo) && !IsEqualGUID(type->formattype, FORMAT_VideoInfo2))
		goto Fail;

	if (sizeof(VIDEOINFOHEADER) > type->cbFormat || NULL == type->pbFormat)
		goto Fail;

	VIDEOINFOHEADER* vi = (VIDEOINFOHEADER*)type->pbFormat;

	format.surface = surface;
	format.width = size_t(vi->bmiHeader.biWidth);
	format.height = size_t(abs(vi->bmiHeader.biHeight));
	format.fps = float(10000000. / double(vi->AvgTimePerFrame));
	DeleteMediaType(type);

	if (NULL == captureFormatCache_.get())
		captureFormatCache_.reset(new (std::nothrow) VideoFormat(format));

	return true;

Fail:
	DeleteMediaType(type);
	return false;
}

bool VideoCapture::GetCaptureFormat(VideoFormat &format) const
{
	return impl_->GetCaptureFormat(format);
}

bool VideoCapture::Impl::MatchFormat(const VideoFormat& format, bool set)
{
	if (!IsVideoSurfaceValid(format.surface))
		return false;

	const GUID& subtype = VideoSurfaceToMediaSubtype(format.surface);
	if (IsEqualGUID(subtype, GUID_NULL))
		return false;

	CAutoLock lock(lock_.get());
	if (stateInitialized != state_)
		return false;

	HRESULT hr;
	assert(NULL != streamConfig_.GetInterfacePtr());
	int count = 0, size = 0;
	if (FAILED(hr = streamConfig_->GetNumberOfCapabilities(&count, &size)))
		return false;

	if (sizeof(VIDEO_STREAM_CONFIG_CAPS) != size)
		return false;

	for (int i = 0; i < count; ++i)
	{
		VIDEO_STREAM_CONFIG_CAPS caps = {0};
		AM_MEDIA_TYPE* type = NULL;
		if (FAILED(hr = streamConfig_->GetStreamCaps(i, &type, (BYTE*)&caps)))
			continue;

		assert(NULL != type);
		bool done = MatchFormat(format, *type, caps, set);
		DeleteMediaType(type);

		if (done)
			return true;
	}
	return false;
}

bool VideoCapture::Impl::MatchFormat(const VideoFormat& format, AM_MEDIA_TYPE& type, VIDEO_STREAM_CONFIG_CAPS& caps, bool set)
{
	VideoSurface surface = MediaSubtypeToVideoSurface(type.subtype);
	if (format.GetSurface() != surface)
		return false;

	if (!IsEqualGUID(type.formattype, FORMAT_VideoInfo) && !IsEqualGUID(type.formattype, FORMAT_VideoInfo2))
		return false;

	if (sizeof(VIDEOINFOHEADER) > type.cbFormat || NULL == type.pbFormat)
		return false;

	VIDEOINFOHEADER* vi = (VIDEOINFOHEADER*)type.pbFormat;

	// check if requested fps is in allowed range
	float maxFps = float(10000000. / double(caps.MinFrameInterval));
	float minFps = 1.f; 
	if (MAXLONGLONG != caps.MaxFrameInterval)
		minFps = float(10000000. / double(caps.MaxFrameInterval));

	assert(minFps <= maxFps);
	if (format.fps < minFps || format.fps > maxFps)
		return false;

	REFERENCE_TIME frameDuration = REFERENCE_TIME(10000000. / double(format.fps) + .5);
	
	// check frame size constraints & granularity
	if (LONG(format.width) < caps.MinOutputSize.cx || LONG(format.width) > caps.MaxOutputSize.cx)
		return false;
	if (LONG(format.height) < caps.MinOutputSize.cy || LONG(format.height) > caps.MaxOutputSize.cy)
		return false;

	// check frame size granularity
	if (0 != ((LONG(format.width) - caps.MinOutputSize.cx) % caps.OutputGranularityX))
		return false;
	if (0 != ((LONG(format.height) - caps.MinOutputSize.cy) % caps.OutputGranularityY))
		return false;
	
	// if not applying the settings, we're finished; the format seems to be supported
	if (!set)
		return true;

	// apply the settings
	vi->AvgTimePerFrame = frameDuration;
	vi->bmiHeader.biWidth = LONG(format.width);
	
	bool bottomUp = false;
	if (BI_RGB == vi->bmiHeader.biCompression || BI_BITFIELDS == vi->bmiHeader.biCompression)
	{
		// watch out for both bottom-up and top-down DIBs
		if (vi->bmiHeader.biHeight < 0)
		{
			bottomUp = false;
			vi->bmiHeader.biHeight = -LONG(format.height);
		}
		else
		{
			bottomUp = true;
			vi->bmiHeader.biHeight = LONG(format.height);
		}
	}
	else
	{
		bottomUp = false;
		vi->bmiHeader.biHeight = LONG(format.height);
	}

	vi->bmiHeader.biSizeImage = DIBSIZE(vi->bmiHeader);

	HRESULT hr;
	if (FAILED(hr = streamConfig_->SetFormat(&type)))
	{
		if (VFW_E_INVALIDMEDIATYPE == hr)
			assert(!"Invalid media type reported even though supported media used");

		return false;
	}

	flipper_.reset();
	bottomUp_ = bottomUp;
	if (bottomUp_)
	{
		VideoProcessorFactory* factory = VideoProcessorFactory::GetInstance();
		if (NULL != factory)
		{
			VideoFrameProcessorAutoPtr ptr = factory->CreateProcessor(videoVerticalFlipper, format.GetSurface());
			if (VideoFrameInPlaceProcessor* proc = dynamic_cast<VideoFrameInPlaceProcessor*>(ptr.get()))
			{
				flipper_.reset(proc);
				ptr.release();

				if (!flipper_->Initialize(format.GetSurface(), format.GetWidth(), format.GetHeight()))
					flipper_.reset();
			}
		}
	}

	captureFormatCache_.reset();

	// if unable to allocate proper output format converter, revert output format to capture format
	if (!CommitOutputFormatNoLock())
	{
		outputFormat_.reset();
		CommitOutputFormatNoLock();
		return false;
	}

	return true;
}

bool VideoCapture::SetCaptureFormat(const VideoFormat &format) 
{
	return impl_->MatchFormat(format, true);
}

bool VideoCapture::IsCaptureFormatSupported(const VideoFormat &format) const
{
	return impl_->MatchFormat(format, false);
}

bool VideoCapture::Impl::GetOutputFormat(VideoFormat& format) const
{
	CAutoLock lock(lock_.get());
	if (stateInitialized != state_)
		return false;

	if (NULL != outputFormat_.get())
	{
		format = *outputFormat_;
		return true;
	}

	return GetCaptureFormat(format);
}

bool VideoCapture::GetOutputFormat(VideoFormat& format) const
{
	return impl_->GetOutputFormat(format);
}

void VideoCapture::Impl::ConverterDispose()
{
	free(converterBuffer_);
	converterBuffer_ = NULL;
	converterBufferSize_ = 0;
	converter_.reset();
}

void VideoCapture::Impl::ScalerDispose()
{
	free(scalerBuffer_);
	scalerBuffer_ = NULL;
	scalerBufferSize_ = 0;
	scaler_.reset();
}

bool VideoCapture::Impl::CommitOutputFormatNoLock()
{
	if (NULL == outputFormat_.get())
	{
		ConverterDispose();
		ScalerDispose();
	}
	else
	{
		VideoFormat format;
		if (!GetCaptureFormat(format))
			return false;

		if (!format.IsSurfaceValid())
			return false;

		if (format.GetSurface() == outputFormat_->GetSurface())
			ConverterDispose();
		else
		{
			VideoSurfaceConverterFactory* factory = VideoSurfaceConverterFactory::GetInstance();
			if (NULL == factory)
				return false;

			VideoSurfaceConverterAutoPtr converter;
			try
			{
				converter = factory->CreateConverter(format.width, format.height, format.GetSurface(), outputFormat_->GetSurface());
				if (NULL == converter.get())
					return false;
			}
			catch (std::bad_alloc&)
			{
				return false;
			}

			format.surface = outputFormat_->GetSurface();
			const size_t bufferSize = format.GetFrameByteSize();
			if (0 == bufferSize)
				return false;

			converterBuffer_ = realloc(converterBuffer_, bufferSize);
			if (NULL == converterBuffer_)
				return false;

			converterBufferSize_ = bufferSize;
			converter_ = converter;
		}

		if (format.GetWidth() == outputFormat_->GetWidth() && format.GetHeight() == outputFormat_->GetHeight())
			ScalerDispose();
		else
		{
			VideoProcessorFactory* factory = VideoProcessorFactory::GetInstance();
			if (NULL == factory)
				return false;

			VideoFrameProcessorAutoPtr processor;
			try
			{
				processor = factory->CreateProcessor(videoScaler, outputFormat_->GetSurface());
				if (NULL == processor.get())
					return false;
			}
			catch (std::bad_alloc&)
			{
				return false;
			}

			if (VideoScaler* scaler = dynamic_cast<VideoScaler*>(processor.get()))
			{
				scaler_.reset(scaler);
				processor.release();
			}
			else
				return false;

			if (!scaler_->Initialize(outputFormat_->GetSurface(), format.GetWidth(), format.GetHeight(), outputFormat_->GetWidth(), outputFormat_->GetHeight()))
				return false;

			const size_t bufferSize = outputFormat_->GetFrameByteSize();
			if (0 == bufferSize)
				return false;

			scalerBuffer_ = realloc(scalerBuffer_, bufferSize);
			if (NULL == scalerBuffer_)
				return false;

			scalerBufferSize_ = bufferSize;
		}
	}
	return true;
}

bool VideoCapture::Impl::SetOutputFormat(const VideoFormat* format, bool adjustCaptureFormat)
{
	CAutoLock lock(lock_.get());
	if (stateInitialized != state_)
		return false;

	if (NULL == format)
	{
		outputFormat_.reset();
		CommitOutputFormatNoLock();
		return true;
	}

	if (NULL != outputFormat_.get())
		*outputFormat_ = *format;
	else
		outputFormat_.reset(new VideoFormat(*format));

	if (adjustCaptureFormat)
	{
		VideoFormat f;
		if (!GetBestCaptureFormat(*format, f))
			goto Fail;

		// SetCaptureFormat() will call CommitOutputFormatNoLock on success - no need to call it again
		if (!MatchFormat(f, true))
			goto Fail;

		return true;
	}
	
	if (CommitOutputFormatNoLock())
		return true;

Fail:
	outputFormat_.reset();
	CommitOutputFormatNoLock();
	return false;
}

bool VideoCapture::SetOutputFormat(const VideoFormat* format, bool adjustCaptureFormat)
{
	return impl_->SetOutputFormat(format, adjustCaptureFormat);
}

void VideoCapture::Impl::PushSample(IMediaSample* sample)
{
	if (NULL == sample)
		return;

	if (!running_)
		return;

	if (NULL == sink_)
		return;

	HRESULT hr;
	BYTE* data = NULL;
	if (FAILED(hr = sample->GetPointer(&data)))
		return;

	long length = sample->GetActualDataLength();
	if (0 == length)
		return;

	if (bottomUp_ && NULL != flipper_.get())
		flipper_->Process(data, length);

	if (NULL != converter_.get())
	{
		assert(NULL != converterBuffer_);
		//DWORD timeStart = timeGetTime();
		if (!converter_->Convert(data, length, converterBuffer_, converterBufferSize_))
			return;

		data = static_cast<BYTE*>(converterBuffer_);
		length = long(converterBufferSize_);
		//DWORD timeEnd = timeGetTime();

		//TCHAR buffer[256];
		//_stprintf(buffer, TEXT("PushSample(): processing time %d\n"), timeEnd - timeStart);
		//OutputDebugString(buffer);
	}

	if (NULL != scaler_.get())
	{
		assert(NULL != scalerBuffer_);
		if (!scaler_->Process(data, length, scalerBuffer_, scalerBufferSize_))
			return;

		data = static_cast<BYTE*>(scalerBuffer_);
		length = long(scalerBufferSize_);
	}

	sink_->OfferFrame(data, length);
}

VideoCapture::State VideoCapture::GetState() const
{
	CAutoLock lock(impl_->lock_.get());
	return impl_->state_;
}

bool VideoCapture::Impl::SetSink(VideoCaptureSink* sink)
{
	CAutoLock lock(lock_.get());
	if (running_)
		return false;

	sink_ = sink;
	return true;
}

bool VideoCapture::SetSink(VideoCaptureSink* sink)
{
	return impl_->SetSink(sink);
}

bool VideoCapture::EnumCaptureSurfaces(VideoSurfaces& surfaces) const
{
	return impl_->EnumCaptureSurfaces(surfaces);
}

bool VideoCapture::Impl::EnumCaptureSurfaces(VideoSurfaces& surfaces) const
{
	CAutoLock lock(lock_.get());
	if (stateInitialized != state_)
		return false;

	HRESULT hr;
	assert(NULL != streamConfig_.GetInterfacePtr());
	int count = 0, size = 0;
	if (FAILED(hr = streamConfig_->GetNumberOfCapabilities(&count, &size)))
		return false;

	if (sizeof(VIDEO_STREAM_CONFIG_CAPS) != size)
		return false;

	std::set<VideoSurface> s;
	for (int i = 0; i < count; ++i)
	{
		VIDEO_STREAM_CONFIG_CAPS caps = {0};
		AM_MEDIA_TYPE* type = NULL;
		if (FAILED(hr = streamConfig_->GetStreamCaps(i, &type, (BYTE*)&caps)))
			continue;

		assert(NULL != type);
		VideoSurface surface = MediaSubtypeToVideoSurface(type->subtype);
		try
		{
			if (IsVideoSurfaceValid(surface))
				s.insert(surface);

			DeleteMediaType(type);
		}
		catch (std::bad_alloc&)
		{
			DeleteMediaType(type);
			return false;
		}
	}
	surfaces.clear();
	try 
	{
		surfaces.resize(s.size());
	}
	catch (std::bad_alloc&) {return false;}

	std::copy(s.begin(), s.end(), surfaces.begin());
	return true;
}

bool VideoCapture::Impl::GetBestCaptureFormat(const VideoFormat& outputFormat, VideoFormat& captureFormat) const
{
	if (!IsVideoSurfaceValid(outputFormat.GetSurface()))
		return false;

	const GUID& subtype = VideoSurfaceToMediaSubtype(outputFormat.GetSurface());
	if (IsEqualGUID(subtype, GUID_NULL))
		return false;

	CAutoLock lock(lock_.get());
	if (stateInitialized != state_)
		return false;

	HRESULT hr;
	assert(NULL != streamConfig_.GetInterfacePtr());
	int count = 0, size = 0;
	if (FAILED(hr = streamConfig_->GetNumberOfCapabilities(&count, &size)))
		return false;

	if (sizeof(VIDEO_STREAM_CONFIG_CAPS) != size)
		return false;

	size_t nextW = UINT_MAX;
	size_t nextH = UINT_MAX;
	size_t prevW = 0;
	size_t prevH = 0;
	VideoSurface prevSurface = videoSurfaceUnknown;
	VideoSurface nextSurface = videoSurfaceUnknown;

	for (int i = 0; i < count; ++i)
	{
		VIDEO_STREAM_CONFIG_CAPS caps = {0};
		AM_MEDIA_TYPE* type = NULL;
		if (FAILED(hr = streamConfig_->GetStreamCaps(i, &type, (BYTE*)&caps)))
			continue;

		assert(NULL != type);
		BestFormatMatch(*type, caps, outputFormat, prevW, prevH, nextW, nextH, prevSurface, nextSurface);
		DeleteMediaType(type);
	}

	if (videoSurfaceUnknown != nextSurface)
	{
		captureFormat.SetFrameRate(outputFormat.GetFrameRate());
		captureFormat.SetSize(nextW, nextH);
		captureFormat.SetSurface(nextSurface);
	}
	else if (videoSurfaceUnknown != prevSurface)
	{
		captureFormat.SetFrameRate(outputFormat.GetFrameRate());
		captureFormat.SetSize(prevW, prevH);
		captureFormat.SetSurface(prevSurface);
	}
	else
		return false; 

	return true;
}

void VideoCapture::Impl::BestFormatMatch(const AM_MEDIA_TYPE& type, const VIDEO_STREAM_CONFIG_CAPS& caps, const VideoFormat& format, size_t& prevW, size_t& prevH, size_t& nextW, size_t& nextH, VideoSurface& prevSurface, VideoSurface& nextSurface) const
{
	VideoSurface surface = MediaSubtypeToVideoSurface(type.subtype);
	if (!IsVideoSurfaceValid(surface))
		return;

	if (!IsEqualGUID(type.formattype, FORMAT_VideoInfo) && !IsEqualGUID(type.formattype, FORMAT_VideoInfo2))
		return;

	if (sizeof(VIDEOINFOHEADER) > type.cbFormat || NULL == type.pbFormat)
		return;

	VIDEOINFOHEADER* vi = (VIDEOINFOHEADER*)type.pbFormat;

	// check if requested fps is in allowed range
	float maxFps = float(10000000. / double(caps.MinFrameInterval));
	float minFps = 1.f; 
	if (MAXLONGLONG != caps.MaxFrameInterval)
		minFps = float(10000000. / double(caps.MaxFrameInterval));

	assert(minFps <= maxFps);
	if (format.fps < minFps || format.fps > maxFps)
		return;



	if (format.GetWidth() <= caps.MinOutputSize.cx && 
		(caps.MinOutputSize.cx < nextW || 
			(caps.MinOutputSize.cx == nextW && 
			GetVideoSurfaceDistance(format.GetSurface(), surface) < GetVideoSurfaceDistance(format.GetSurface(), nextSurface))))
	{
		nextW = caps.MinOutputSize.cx;
		nextH = caps.MinOutputSize.cy;
		nextSurface = surface;
	}

	if (format.GetWidth() >= caps.MaxOutputSize.cx && 
		(caps.MaxOutputSize.cx > prevW || 
			(caps.MaxOutputSize.cx == prevW && 
			GetVideoSurfaceDistance(format.GetSurface(), surface) < GetVideoSurfaceDistance(format.GetSurface(), prevSurface))))
	{
		prevW = caps.MaxOutputSize.cx;
		prevH = caps.MaxOutputSize.cy;
		prevSurface = surface;
	}
}
