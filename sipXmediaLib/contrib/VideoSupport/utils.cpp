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
//! @file utils.cpp
//! Assorted non-published utilities.
//! @author Andrzej Ciarkowski <mailto:andrzejc@wp-sa.pl>
#include "stdafx.h"
#include "utils.h"
#include <VideoSupport/VideoFormat.h>

//extern "C" {
//#include <avcodec.h>
//}

CoInitializer::CoInitializer():
init_(false)
{
	if (!FAILED(::CoInitialize(NULL)))
		init_ = true;
}

CoInitializer::~CoInitializer()
{
	if (init_)
		::CoUninitialize();
}

#ifndef INITGUID
# define INITGUID
#endif 
#include <guiddef.h>

// 3231564E-0000-0010-8000-00AA00389B71  'NV12' ==  MEDIASUBTYPE_NV12
DEFINE_GUID(MEDIASUBTYPE_NV12,
0x3231564E, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

// 31434D49-0000-0010-8000-00AA00389B71  'IMC1' ==  MEDIASUBTYPE_IMC1
DEFINE_GUID(MEDIASUBTYPE_IMC1,
0x31434D49, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

// 32434d49-0000-0010-8000-00AA00389B71  'IMC2' ==  MEDIASUBTYPE_IMC2
DEFINE_GUID(MEDIASUBTYPE_IMC2,
0x32434D49, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

// 33434d49-0000-0010-8000-00AA00389B71  'IMC3' ==  MEDIASUBTYPE_IMC3
DEFINE_GUID(MEDIASUBTYPE_IMC3,
0x33434D49, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

// 34434d49-0000-0010-8000-00AA00389B71  'IMC4' ==  MEDIASUBTYPE_IMC4
DEFINE_GUID(MEDIASUBTYPE_IMC4,
0x34434D49, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

#define MATCH4CC(code) if (IsEqualGUID(mediaSubtype, (MEDIASUBTYPE_ ## code))) return (videoSurface ## code)

VideoSurface MediaSubtypeToVideoSurface(const GUID& mediaSubtype)
{
	MATCH4CC(AYUV);
	MATCH4CC(UYVY);
	MATCH4CC(YUY2);
	MATCH4CC(YVYU);
	MATCH4CC(IMC1);
	MATCH4CC(IMC3);
	MATCH4CC(IMC2);
	MATCH4CC(IMC4);
	MATCH4CC(YV12);
	MATCH4CC(NV12);
	MATCH4CC(ARGB32);
	MATCH4CC(RGB24);

	return videoSurfaceUnknown;
}

#undef MATCH4CC
#define MATCH4CC(code) if ((videoSurface ## code) == surface) return (MEDIASUBTYPE_ ## code)

const GUID& VideoSurfaceToMediaSubtype(VideoSurface surface)
{
	MATCH4CC(AYUV);
	MATCH4CC(UYVY);
	MATCH4CC(YUY2);
	MATCH4CC(YVYU);
	MATCH4CC(IMC1);
	MATCH4CC(IMC3);
	MATCH4CC(IMC2);
	MATCH4CC(IMC4);
	MATCH4CC(YV12);
	MATCH4CC(NV12);
	MATCH4CC(ARGB32);
	MATCH4CC(RGB24);

	return GUID_NULL;
}
