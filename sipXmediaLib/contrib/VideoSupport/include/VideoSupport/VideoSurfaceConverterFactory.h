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
#pragma once

#include <VideoSupport/Types.h>
#include <memory>

typedef std::auto_ptr<VideoSurfaceConverter> VideoSurfaceConverterAutoPtr;
typedef VideoSurfaceConverter* (* VideoSurfaceConverterConstructor)();

class VideoSurfaceConverterFactory
{
public:

	static VideoSurfaceConverterFactory* GetInstance();

	static void StaticDispose();

	VideoSurfaceConverterAutoPtr CreateConverter(size_t width, size_t height, VideoSurface sourceSurface, VideoSurface targetSurface);

	static bool RegisterConstructor(VideoSurface sourceSurface, VideoSurface targetSurface, VideoSurfaceConverterConstructor constructor);

private:

	VideoSurfaceConverterFactory();
	~VideoSurfaceConverterFactory();
	
	VideoSurfaceConverterFactory(const VideoSurfaceConverterFactory&);
	VideoSurfaceConverterFactory& operator=(const VideoSurfaceConverterFactory&);

	struct Impl;
	Impl* impl_;
};
