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
//! @file RawVideoRenderer.h
//! Definition of RawVideoRenderer class.
//! @author Andrzej Ciarkowski <mailto:andrzejc@wp-sa.pl>
#pragma once
#include <memory>

#include <wxdebug.h> // required by combase.h
#include <combase.h> // from Platform SDK DirectShow Samples, defines CUnknown

class RawVideoRendererInputPin;
class RawVideoRenderer;
class RawVideoRendererFilter;

class CPosPassThru;
class CCritSec;

class MediaSamplePushSink;

//! DirectShow Video Renderer which sinks raw video frames into associated 
//! @c MediaSampleSink. Loosly based on Platform SDK Dump example.
class RawVideoRenderer : public CUnknown
{
public:

    DECLARE_IUNKNOWN

    RawVideoRenderer(LPUNKNOWN pUnk, HRESULT* phr);
    ~RawVideoRenderer();

    static CUnknown* WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

	//! This function is safe to call only when filter graph is not running.
	void SetMediaSampleSink(MediaSamplePushSink* sink);

private:
    // Overriden to say what interfaces we support where
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    friend class RawVideoRendererFilter;
    friend class RawVideoRendererInputPin;

	std::auto_ptr<RawVideoRendererFilter> filter_;       // Methods for filter interfaces
	std::auto_ptr<RawVideoRendererInputPin> pin_;          // A simple rendered input pin

	std::auto_ptr<CCritSec> lock_;                // Main renderer critical section
    std::auto_ptr<CCritSec> receiveLock_;         // Sublock for received samples
	std::auto_ptr<CPosPassThru> position_;      // Renderer position controls
};

