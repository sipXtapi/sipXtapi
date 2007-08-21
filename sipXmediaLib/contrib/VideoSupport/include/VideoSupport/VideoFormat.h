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
//! @file VideoFormat.h
//! Stuff related to dealing with video formats.
//! @author Andrzej Ciarkowski <mailto:andrzejc@wp-sa.pl>
#pragma once

#include <VideoSupport/Types.h>

//! Video surface (pixel format) constants.
//! @see http://msdn2.microsoft.com/en-us/library/Aa904813(VS.80).aspx
enum VideoSurface
{
	//! Unsupported video surface.
	videoSurfaceUnknown,

	// 4:4:4, 32bits per pixel
	videoSurfaceAYUV,

	// 4:2:2, 16bpp
	// UYVY is probably the most popular of the various YUV 4:2:2 formats. It is output as the format of choice by the Radius Cinepak codec and is often the second choice of software MPEG codecs after YV12.
	videoSurfaceUYVY,
	videoSurfaceY422 = videoSurfaceUYVY,

	// 4:2:2, 16bpp
	// YUY2 is another in the family of YUV 4:2:2 formats and appears to be used by all the same codecs as UYVY.
	videoSurfaceYUY2,
	videoSurfaceV422 = videoSurfaceYUY2,

	// Despite being a simple byte ordering change from YUY2 or UYVY, YVYU seems to be seen somewhat less often than the other two formats defined above.
	videoSurfaceYVYU,

	// 4:2:0, 16bpp
	videoSurfaceIMC1,
	// 4:2:0, 16bpp (the same as IMC1 with swapped U/V planes)
	videoSurfaceIMC3,

	// 4:2:0, 12bpp
	videoSurfaceIMC2,
	// 4:2:0, 12bpp (the same as IMC2 with swapped U/V lines)
	videoSurfaceIMC4,
	// 4:2:0, 12bpp 
	videoSurfaceYV12,
	// 4:2:0, 12bpp 
	videoSurfaceNV12,

	videoSurfaceARGB32,
	videoSurfaceARGB8888 = videoSurfaceARGB32,

	videoSurfaceRGB24,
	videoSurfaceRGB888 = videoSurfaceRGB24,

	// when adding enumerators, remember to change IsVideoSurfaceValid()
};


static const VideoSurface videoSurfaceYUV_First = videoSurfaceAYUV;
static const VideoSurface videoSurfaceYUV_Last = videoSurfaceNV12;

static const VideoSurface videoSurfaceRGB_First = videoSurfaceARGB32;
static const VideoSurface videoSurfaceRGB_Last = videoSurfaceRGB24;

static const VideoSurface videoSurfaceFirst = videoSurfaceAYUV;
static const VideoSurface videoSurfaceLast = videoSurfaceRGB24;

//! Query byte size of video frame.
//! @param surface Video surface identifier.
//! @param width Video frame width.
//! @param height Video frame height.
//! @raturn Size of frame data in bytes.
size_t GetVideoFrameByteSize(VideoSurface surface, size_t width, size_t height);

inline bool IsVideoSurfaceValid(VideoSurface surface) {return surface >= videoSurfaceFirst && surface <= videoSurfaceLast;}
inline bool IsVideoSurfaceYUV(VideoSurface surface) {return surface >= videoSurfaceYUV_First && surface <= videoSurfaceYUV_Last;}
inline bool IsVideoSurfaceRGB(VideoSurface surface) {return surface >= videoSurfaceRGB_First && surface <= videoSurfaceRGB_Last;}

struct VideoFormat
{
	static const size_t width_CIF = 352;
	static const size_t width_QCIF = width_CIF / 2;
	static const size_t width_SQCIF = 128;
	static const size_t width_4CIF = width_CIF * 2;
	static const size_t width_16CIF = width_4CIF * 2;
	static const size_t width_VGA = 640;
	static const size_t width_QVGA = width_VGA / 2;

	static const size_t height_CIF = 288;
	static const size_t height_QCIF = height_CIF / 2;
	static const size_t height_SQCIF = 96;
	static const size_t height_4CIF = height_CIF * 2;
	static const size_t height_16CIF = height_4CIF * 2;
	static const size_t height_VGA = 480;
	static const size_t height_QVGA = height_VGA / 2;

	enum SizePreset
	{
		sizeOther,
		size_SQCIF,
		size_QCIF,
		size_QVGA,
		size_CIF,
		size_VGA,
		size_4CIF,
		size_16CIF,
	};

	VideoSurface surface;

	size_t width;
	size_t height;
	float fps;

	VideoFormat(): surface(videoSurfaceUnknown), width(0), height(0), fps(0.f) {}
	~VideoFormat() {}

	void SetWidth(size_t width) {this->width = width;}
	void SetHeight(size_t height) {this->height = height;}

	void SetSize(size_t width, size_t height) {SetWidth(width); SetHeight(height);}

	void SetSize(SizePreset size);

	float GetFrameRate() const {return fps;}

	void SetFrameRate(float fps) {this->fps = fps;}

	SizePreset GetSize() const;

	size_t GetFrameByteSize() const {return GetVideoFrameByteSize(surface, width, height);}

	VideoSurface GetSurface() const {return surface;}

	void SetSurface(VideoSurface surface) {this->surface = surface;}

	bool IsSurfaceValid() const {return IsVideoSurfaceValid(surface);}

};
