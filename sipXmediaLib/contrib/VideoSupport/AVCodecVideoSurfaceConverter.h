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
//! @file AVCodecVideoSurfaceConverter.h
//! VideoSurfaceConverter using libavcodec routines.
//! @author Andrzej Ciarkowski <mailto:andrzejc@wp-sa.pl>
#pragma once

#include "VideoSurfaceConverterImpl.h"
#include "utils.h"

#ifndef VIDEO_SUPPORT_DISABLE_AVCODEC

struct AVPicture;

class AVCodecVideoSurfaceConverter: public VideoSurfaceConverter, private VideoSurfaceConverterMixIn
{
public:

	AVCodecVideoSurfaceConverter();
	~AVCodecVideoSurfaceConverter();

	bool Initialize(size_t width, size_t height, VideoSurface sourceSurface, VideoSurface targetSurface);

	bool Convert(const void* sourceFrameBytes, size_t sourceFrameByteSize, void* targetFrameBytes, size_t targetFrameByteSize) throw();

private:
	AVCodecVideoSurfaceConverter(const AVCodecVideoSurfaceConverter&);
	AVCodecVideoSurfaceConverter& operator = (const AVCodecVideoSurfaceConverter&);

	PixelFormat sourcePixelFormat_;
	PixelFormat targetPixelFormat_;
	std::auto_ptr<AVPicture> sourcePicture_;
	std::auto_ptr<AVPicture> targetPicture_;
};

#endif // VIDEO_SUPPORT_DISABLE_AVCODEC
