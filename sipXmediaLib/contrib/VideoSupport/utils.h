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
//! @file utils.h
//! Assorted non-published utilities.
//! @author Andrzej Ciarkowski <mailto:andrzejc@wp-sa.pl>
#pragma once

class CoInitializer
{
public:

	CoInitializer();

	bool IsInitialized() const {return init_;}

	~CoInitializer();

private:
	CoInitializer(const CoInitializer&);
	CoInitializer& operator = (const CoInitializer&);

	bool init_;
};

enum VideoSurface;
typedef struct _GUID GUID;

VideoSurface MediaSubtypeToVideoSurface(const GUID& mediaSubtype);

const GUID& VideoSurfaceToMediaSubtype(VideoSurface surface);

//! FFmpeg pixel format
enum PixelFormat;

VideoSurface PixelFormatToVideoSurface(PixelFormat pixelFormat);

PixelFormat VideoSurfaceToPixelFormat(VideoSurface surface);
