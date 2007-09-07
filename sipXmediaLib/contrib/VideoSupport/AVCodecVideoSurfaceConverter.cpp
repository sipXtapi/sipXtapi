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
#include "AVCodecVideoSurfaceConverter.h"

#ifndef VIDEO_SUPPORT_DISABLE_AVCODEC

AVCodecVideoSurfaceConverter::AVCodecVideoSurfaceConverter():
	sourcePixelFormat_(PIX_FMT_NONE),
	targetPixelFormat_(PIX_FMT_NONE)
{
}

AVCodecVideoSurfaceConverter::~AVCodecVideoSurfaceConverter()
{
	// reset pointer explicitly to ease debugging
	sourcePicture_.reset();
	targetPicture_.reset();
}

bool AVCodecVideoSurfaceConverter::Initialize(const size_t width, const size_t height, const VideoSurface sourceSurface, const VideoSurface targetSurface)
{
	const int w = int(width);
	const int h = int(height);

	const PixelFormat sourceFormat = VideoSurfaceToPixelFormat(sourceSurface);
	assert(PIX_FMT_NONE != sourceFormat);
	if (PIX_FMT_NONE == sourceFormat)
		return false;

	const PixelFormat targetFormat = VideoSurfaceToPixelFormat(targetSurface);
	assert(PIX_FMT_NONE != targetFormat);
	if (PIX_FMT_NONE == targetFormat)
		return false;

	if (sourceFormat == targetFormat)
		return false;

	const int srcSize = avpicture_get_size(sourceFormat, w, h);
	assert(srcSize == GetVideoFrameByteSize(sourceSurface, width, height));
	if (srcSize != GetVideoFrameByteSize(sourceSurface, width, height))
		return false;

	const int trgSize = avpicture_get_size(targetFormat, w, h);
	assert(trgSize == GetVideoFrameByteSize(targetSurface, width, height));
	if (trgSize != GetVideoFrameByteSize(targetSurface, width, height))
		return false;

	if (!VideoSurfaceConverterMixIn::Initialize(width, height, sourceSurface, targetSurface))
		return false;

	std::auto_ptr<AVPicture> srcPic(new (std::nothrow) AVPicture);
	if (NULL == srcPic.get())
		return false;

	std::auto_ptr<AVPicture> trgPic(new (std::nothrow) AVPicture);
	if (NULL == trgPic.get())
		return false;

	void* srcFrame = malloc(srcSize);
	void* trgFrame = malloc(trgSize);
	if (NULL == srcFrame || NULL == trgFrame)
	{
		free(srcFrame);
		free(trgFrame);
		return false;
	}

	memset(srcFrame, 128, srcSize);
	memset(trgFrame, 0, trgSize);

	avpicture_fill(srcPic.get(), (uint8_t*)srcFrame, sourceFormat, w, h);
	avpicture_fill(trgPic.get(), (uint8_t*)trgFrame, targetFormat, w, h);
	int res = img_convert(trgPic.get(), targetFormat, srcPic.get(), sourceFormat, w, h);

	free(srcFrame);
	free(trgFrame);

	if (res >= 0)
	{
		sourcePicture_ = srcPic;
		targetPicture_ = trgPic;
		sourcePixelFormat_ = sourceFormat;
		targetPixelFormat_ = targetFormat;
		return true;
	}
	else
		return false;
}

bool AVCodecVideoSurfaceConverter::Convert(const void* sourceFrameBytes, const size_t sourceFrameByteSize, void* targetFrameBytes, const size_t targetFrameByteSize)
{
	if (GetSourceFrameByteSize() != sourceFrameByteSize)
		return false;
	if (GetTargetFrameByteSize() != targetFrameByteSize)
		return false;

	assert(NULL != sourcePicture_.get());
	assert(NULL != targetPicture_.get());

	const int w = int(GetWidth());
	const int h = int(GetHeight());

	avpicture_fill(sourcePicture_.get(), (uint8_t*)sourceFrameBytes, sourcePixelFormat_, w, h);
	// hack: PIX_FMT_YUV420P is used both for YV12 and I420, we adjust plane pointers as these formats differ by U & V plane positions
	if (videoSurfaceYV12 == GetSourceSurface())
		std::swap(sourcePicture_->data[1], sourcePicture_->data[2]);

	avpicture_fill(targetPicture_.get(), (uint8_t*)targetFrameBytes, targetPixelFormat_, w, h);
	if (videoSurfaceYV12 == GetTargetSurface())
		std::swap(targetPicture_->data[1], targetPicture_->data[2]);

	int res = img_convert(targetPicture_.get(), targetPixelFormat_, sourcePicture_.get(), sourcePixelFormat_, w, h);
	return res >= 0;
}

#endif // VIDEO_SUPPORT_DISABLE_AVCODEC
