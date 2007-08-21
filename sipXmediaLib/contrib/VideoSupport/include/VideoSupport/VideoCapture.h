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
//! @file VideoCapture.h
//! Definition of @c VideoCapture class.
//! @author Andrzej Ciarkowski <mailto:andrzejc@wp-sa.pl>
#pragma once
#include <VideoSupport/Types.h>
#include <string>
#include <vector>

typedef struct HWND__* HWND;

class VideoCapture
{
public:

	//! Constructor.
	//! Object is unusable until @c Initialize() succeeds.
	//! @post state is @c stateUninitialized.
	VideoCapture();

	~VideoCapture();

	typedef std::vector<std::string> DeviceNames;
	//! Enumerate names of capture devices present in system.
	//! @param names [out] Container filled with capture device names on return.
	static void EnumDevices(DeviceNames& names);

	//! Initialize video capture using device of specified name. 
	//! @param deviceName [in] Name of capture device, as returned by @c EnumDevices().
	//! @param previewWindoe [in] Optional window handle of the control the preview 
	//! window will be linked with. If not set, preview window will be hidden.
	bool Initialize(const std::string& deviceName, HWND previewWindow = NULL);

	//! Object state constants.
	enum State
	{
		//! Video capture is not initialized (before @c Initialize() or after 
		//! @c Close()).
		stateUninitialized,
		//! Video capture is initialized and ready to work.
		stateInitialized,
		//! Video capture is running.
		stateRunning,
	};

	//! Query object state.
	//! @return One of @c State constants idicating object state.
	State GetState() const;

	//! Test if object is initialized successfully.
	bool IsInitialized() const {return stateUninitialized != GetState();}

	//! Stop running video capture.
	//! State transition from @c stateRunning to @c stateInitialized.
	void Stop();

	//! Start video capture.
	//! State transition from @c stateInitialized to @c stateRunning.
	bool Run();

	//! Stop video capture and release associated resources.
	//! State transition to @c stateUninitialized.
	void Close();

	static void StaticDispose();

	bool GetCaptureFormat(VideoFormat& format) const;

	bool SetCaptureFormat(const VideoFormat& format);

	bool IsCaptureFormatSupported(const VideoFormat& format) const;

	bool GetOutputFormat(VideoFormat& format) const;

	bool SetOutputVideoSurface(VideoSurface surface);

	//! Must be synchronized externally w/ regard to Run() and Stop().
	//! Will fail if capture is running.
	bool SetSink(VideoCaptureSink* sink);

private:
	VideoCapture(const VideoCapture&);
	VideoCapture& operator=(const VideoCapture&);

	struct Impl;
	Impl* impl_;
};