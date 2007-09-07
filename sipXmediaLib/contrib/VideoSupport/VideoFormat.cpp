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
//! @file VideoFormat.cpp
//! Implementation of video format-related utils.
//! @author Andrzej Ciarkowski <mailto:andrzejc@wp-sa.pl>
#include "stdafx.h"
#include <VideoSupport/VideoFormat.h>
#include "utils.h"

namespace 
{
	struct Size_
	{
		const char* label;
		int w;
		int h;
	};

#define DEFSIZE(code) {#code, (VideoFormat::width_ ## code), (VideoFormat::height_ ## code)}

	static const Size_ frameSizes[] =
	{
		{"", 0, 0}, // sizeOther
		DEFSIZE(SQCIF),
		DEFSIZE(QCIF),
		DEFSIZE(QVGA),
		DEFSIZE(CIF),
		DEFSIZE(VGA),
		DEFSIZE(4CIF),
		DEFSIZE(16CIF),
	};
}

void VideoFormat::SetSize(VideoFormat::SizePreset size)
{
	assert(size > sizeOther);
	assert(size <= size_16CIF);
	// no-op is better than access violation
	if (size < sizeOther || size > size_16CIF)
		return;

	SetSize(frameSizes[size].w, frameSizes[size].h);
}

VideoFormat::SizePreset VideoFormat::GetSize() const
{
	for (int i = 1; i < sizeof(frameSizes) / sizeof(*frameSizes); ++i)
	{
		if (frameSizes[i].w == width && frameSizes[i].h == height)
			return SizePreset(i);
	}

	return sizeOther;
}

VideoFormat::SizePreset VideoFormat::GetSizeByLabel(const char* label)
{
	for (int i = 1; i < sizeof(frameSizes) / sizeof(*frameSizes); ++i)
	{
		if (0 == stricmp(label, frameSizes[i].label))
			return SizePreset(i);
	}

	return sizeOther;
}

const char* VideoFormat::GetSizeLabel(SizePreset preset) 
{
	if (preset < size_SQCIF || preset > size_16CIF)
		return "";

	return frameSizes[preset].label;
}

size_t GetVideoFrameByteSize(VideoSurface surface, size_t width, size_t height)
{
	if (0 == width || 0 == height)
		return 0;

	assert(IsVideoSurfaceValid(surface));
	if (!IsVideoSurfaceValid(surface))
		return 0;

	switch (surface) {
	// 32bpp 4:4:4 formats
	case videoSurfaceAYUV: 
		return 4 * width * height;

	// 16bpp 4:2:2 formats
	case videoSurfaceUYVY:
	case videoSurfaceYUY2:
	case videoSurfaceYVYU:
		// only even frame widths are allowed due to 2:1 horizontal downsampling of chroma
		if (0 != (width % 2))
			return 0;
		else
			return 2 * width * height;

	// 16bpp 4:2:0 formats
	case videoSurfaceIMC1:
	case videoSurfaceIMC3:
		// only even frame dimensions are allowed due to 2:1 downsampling of chroma
		if (0 != (width % 2) || 0 != (height % 2))
			return 0;
		else
			// other than that this is still 16bpp packed format (some space is wasted)
			return 2 * width * height;

	// 12bpp 4:2:0 formats
	case videoSurfaceIMC2:
	case videoSurfaceIMC4:
	case videoSurfaceYV12:
	case videoSurfaceIYUV:
	case videoSurfaceNV12:
		// only even frame dimensions are allowed due to 2:1 downsampling of chroma
		if (0 != (width % 2) || 0 != (height % 2))
			return 0;
		else
			return width * height + width * height / 2;

	case videoSurfaceARGB32:
		return 4 * width * height;

	case videoSurfaceRGB24:
		return 3 * width * height;

	default:
		assert(!"GetFrameByteSize() lacks case for supported video surface enumerator!");
		return 0;
	}
}

VideoSurface PixelFormatToVideoSurface(const PixelFormat pixelFormat)
{
#ifndef VIDEO_SUPPORT_DISABLE_AVCODEC
	switch (pixelFormat) {
	// TODO: determine I420 or YV12. For now we assume I420, as this is what FFmpeg 
	// returns. However, this is suspicious - it treats YV12 identically.
	//case PIX_FMT_YUV420P: return videoSurfaceYV12;
	case PIX_FMT_YUV420P: return videoSurfaceI420;
	case PIX_FMT_YUYV422: return videoSurfaceYUY2;
	case PIX_FMT_UYVY422: return videoSurfaceUYVY;
	case PIX_FMT_RGB24: return videoSurfaceRGB24;

	case PIX_FMT_RGB32:
	case PIX_FMT_ARGB: return videoSurfaceARGB32;

	case PIX_FMT_NV12: return videoSurfaceNV12;
	}
#endif // VIDEO_SUPPORT_DISABLE_AVCODEC
	return videoSurfaceUnknown;
}

PixelFormat VideoSurfaceToPixelFormat(const VideoSurface surface)
{
#ifndef VIDEO_SUPPORT_DISABLE_AVCODEC
	switch (surface) {
	case videoSurfaceI420: return PIX_FMT_YUV420P;
	// hack: AVCodecVideoSurfaceConverter will check surface and adjust U&V plane pointers appropriately
	case videoSurfaceYV12: return PIX_FMT_YUV420P;
	case videoSurfaceYUY2: return PIX_FMT_YUYV422;
	case videoSurfaceUYVY: return PIX_FMT_UYVY422;
	case videoSurfaceRGB24: return PIX_FMT_RGB24;
	case videoSurfaceARGB32: return PIX_FMT_RGB32;
	case videoSurfaceNV12: return PIX_FMT_NV12;
	}
#endif // VIDEO_SUPPORT_DISABLE_AVCODEC
	return PixelFormat(-1);
}

static bool IsVideoSurfaceSubsampled(VideoSurface surface)
{
	assert(IsVideoSurfaceValid(surface));

	if (IsVideoSurfaceRGB(surface))
		return false;

	return (videoSurfaceAYUV != surface);
}

size_t GetVideoSurfaceDistance(VideoSurface from, VideoSurface to)
{
	if (to == from)
		return 0;

	size_t prox = 0;
	if ((IsVideoSurfaceRGB(from) && IsVideoSurfaceYUV(to)) ||
		(IsVideoSurfaceYUV(from) && IsVideoSurfaceRGB(to)))
	{
		++prox;
	}

	if (IsVideoSurfaceSubsampled(from))
		++prox;

	if (IsVideoSurfaceSubsampled(to))
		++prox;

	return prox;
}
