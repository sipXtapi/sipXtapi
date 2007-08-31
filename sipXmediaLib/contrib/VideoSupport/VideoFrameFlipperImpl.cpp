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
#include "VideoFrameFlipperImpl.h"
#include <VideoSupport/VideoFormat.h>
#include <VideoSupport/VideoProcessorFactory.h>

typedef unsigned char BYTE;

namespace 
{
	static inline void SwapLines(BYTE* top, BYTE* bot, const size_t stride)
	{
		for (size_t i = 0; i < stride; ++i)
			std::swap(top[i], bot[i]);
	}

}

GenericVerticalFlipper::GenericVerticalFlipper():
frameByteSize_(0),
stride_(0),
height_(0)
{
}

bool GenericVerticalFlipper::Initialize(VideoSurface surface, size_t width, size_t height)
{
	frameByteSize_ = GetVideoFrameByteSize(surface, width, height);
	if (0 == frameByteSize_)
		return false;

	const size_t pixelCount = width * height;
	if (0 != (frameByteSize_ % pixelCount))
		return false;

	const size_t bpp = frameByteSize_ / pixelCount;
	stride_ = width * bpp;
	height_ = height;
	return true;
}

bool GenericVerticalFlipper::Process(void* frameBytes, const size_t frameByteSize)
{
	if (frameByteSize != frameByteSize_)
		return false;

	BYTE* data = (BYTE*)frameBytes;
	const size_t h2 = height_ / 2;
	for (size_t i = 0; i < h2; ++i)
	{
		BYTE* top = data + i * stride_;
		BYTE* bot = data + (height_ - i - 1) * stride_;
		SwapLines(top, bot, stride_);
	}

	return true;
}

VideoFrameProcessor* GenericVerticalFlipper::Create()
{
	return new (std::nothrow) GenericVerticalFlipper();
}

#define VERTICAL_FLIPPER_REGISTER(code) \
static bool verticalFlipper_ ## code = VideoProcessorFactory::RegisterConstructor(videoVerticalFlipper, videoSurface ## code, GenericVerticalFlipper::Create)

VERTICAL_FLIPPER_REGISTER(RGB24);
VERTICAL_FLIPPER_REGISTER(ARGB32);

void VideoFrameFlipperImplEnsureReg()
{
}