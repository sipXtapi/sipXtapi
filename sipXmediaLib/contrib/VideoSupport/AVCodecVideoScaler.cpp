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
//! @file AVCodecVideoSurfaceConverter.cpp
//! VideoSurfaceConverter using libavcodec routines.
//! @author Andrzej Ciarkowski <mailto:andrzejc@wp-sa.pl>
#include "stdafx.h"
#include "AVCodecVideoScaler.h"
#include <VideoSupport/VideoFormat.h>

#ifndef VIDEO_SUPPORT_DISABLE_AVCODEC

AVCodecVideoScaler::AVCodecVideoScaler():
	pixelFormat_(PIX_FMT_NONE),
	resampleContext_(NULL),
	sourceWidth_(0),
	sourceHeight_(0),
	targetWidth_(0),
	targetHeight_(0)
{
}

AVCodecVideoScaler::~AVCodecVideoScaler()
{
	// reset pointer explicitly to ease debugging
	sourcePicture_.reset();
	targetPicture_.reset();

	if (NULL != resampleContext_)
	{
		img_resample_close(resampleContext_);
		resampleContext_ = NULL;
	}
}

bool AVCodecVideoScaler::Initialize(const VideoSurface surface, const size_t sourceWidth, const size_t sourceHeight, const size_t targetWidth, const size_t targetHeight)
{
	const int sw = int(sourceWidth);
	const int sh = int(sourceHeight);
	const int tw = int(targetWidth);
	const int th = int(targetHeight);

	const PixelFormat pixelFormat = VideoSurfaceToPixelFormat(surface);
	assert(PIX_FMT_NONE != pixelFormat);
	if (PIX_FMT_NONE == pixelFormat)
		return false;

	const int srcSize = avpicture_get_size(pixelFormat, sw, sh);
	assert(srcSize == GetVideoFrameByteSize(surface, sourceWidth, sourceHeight));
	if (srcSize != GetVideoFrameByteSize(surface, sourceWidth, sourceHeight))
		return false;

	const int trgSize = avpicture_get_size(pixelFormat, tw, th);
	assert(trgSize == GetVideoFrameByteSize(surface, targetWidth, targetHeight));
	if (trgSize != GetVideoFrameByteSize(surface, targetWidth, targetHeight))
		return false;

	std::auto_ptr<AVPicture> srcPic(new (std::nothrow) AVPicture);
	if (NULL == srcPic.get())
		return false;

	std::auto_ptr<AVPicture> trgPic(new (std::nothrow) AVPicture);
	if (NULL == trgPic.get())
		return false;

	if (NULL != resampleContext_)
	{
		img_resample_close(resampleContext_);
		resampleContext_ = NULL;
	}

	resampleContext_ = img_resample_init(tw, th, sw, sh);
	if (NULL == resampleContext_)
		return false;
	
	sourceWidth_ = sw;
	sourceHeight_ = sh;
	targetWidth_ = tw;
	targetHeight_ = th;
	sourcePicture_ = srcPic;
	targetPicture_ = trgPic;
	pixelFormat_ = pixelFormat;
	return true;
}

bool AVCodecVideoScaler::Process(const void* sourceFrameBytes, const size_t sourceFrameByteSize, void* targetFrameBytes, const size_t targetFrameByteSize)
{
	if (NULL == resampleContext_)
		return false;

	if (avpicture_get_size(pixelFormat_, sourceWidth_, sourceHeight_) != sourceFrameByteSize)
		return false;

	if (avpicture_get_size(pixelFormat_, targetWidth_, targetHeight_) != targetFrameByteSize)
		return false;

	assert(NULL != sourcePicture_.get());
	assert(NULL != targetPicture_.get());


	avpicture_fill(sourcePicture_.get(), (uint8_t*)sourceFrameBytes, pixelFormat_, sourceWidth_, sourceHeight_);
	avpicture_fill(targetPicture_.get(), (uint8_t*)targetFrameBytes, pixelFormat_, targetWidth_, targetHeight_);

	img_resample(resampleContext_, targetPicture_.get(), sourcePicture_.get());
	return true;
}

#endif // VIDEO_SUPPORT_DISABLE_AVCODEC
