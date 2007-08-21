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
#include <VideoSupport/VideoSurfaceConverter.h>
#include <VideoSupport/VideoFormat.h>
#include <algorithm>

#ifdef _MSC_VER
# define FORCEINLINE __forceinline
#else
# define FORCEINLINE inline
#endif


class VideoSurfaceConverterMixIn
{
public:

	VideoSurfaceConverterMixIn():
		sourceSurface_(videoSurfaceUnknown),
		targetSurface_(videoSurfaceUnknown),
		width_(0),
		height_(0),
		sourceFrameByteSize_(0),
		targetFrameByteSize_(0)
	{
	}

	bool Initialize(size_t width, size_t height, VideoSurface sourceSurface, VideoSurface targetSurface);


	FORCEINLINE size_t GetWidth() const throw() {return width_;}
	FORCEINLINE size_t GetHeight() const throw() {return height_;}
	FORCEINLINE size_t GetSourceFrameByteSize() const throw() {return sourceFrameByteSize_;}
	FORCEINLINE size_t GetTargetFrameByteSize() const throw() {return targetFrameByteSize_;}
	FORCEINLINE VideoSurface GetSourceSurface() const throw() {return sourceSurface_;}
	FORCEINLINE VideoSurface GetTargetSurface() const throw() {return targetSurface_;}

private:
	VideoSurface sourceSurface_;
	VideoSurface targetSurface_;
	size_t width_;
	size_t height_;
	size_t sourceFrameByteSize_;
	size_t targetFrameByteSize_;
};

class TandemVideoSurfaceConverter: public VideoSurfaceConverter, private VideoSurfaceConverterMixIn
{
public:

	explicit TandemVideoSurfaceConverter(VideoSurface intermediateSurface);

	~TandemVideoSurfaceConverter();
	
	bool Initialize(size_t width, size_t height, VideoSurface sourceSurface, VideoSurface targetSurface);

	bool Convert(const void* sourceFrameBytes, size_t sourceFrameByteSize, void* targetFrameBytes, size_t targetFrameByteSize) throw();

	static VideoSurfaceConverter* Create(VideoSurface intermediateSurface);

	template<VideoSurface intermediate>
	static VideoSurfaceConverter* CreateTandem() {return Create(intermediate);}


private:
	TandemVideoSurfaceConverter(const TandemVideoSurfaceConverter&);
	TandemVideoSurfaceConverter& operator = (const TandemVideoSurfaceConverter&);

	VideoSurface intermediateSurface_;
	size_t intermediateFrameByteSize_;
	void* intermediateBuffer_;

	std::auto_ptr<VideoSurfaceConverter> sourceConverter_;
	std::auto_ptr<VideoSurfaceConverter> targetConverter_;
};

template<VideoSurface source, VideoSurface target>
class BasicVideoSurfaceConverter: public VideoSurfaceConverter, protected VideoSurfaceConverterMixIn
{
protected:

	virtual bool ConvertImpl(const void* sourceFrameBytes, size_t sourceFrameByteSize, void* targetFrameBytes, size_t targetFrameByteSize) throw() = 0;

public:

	bool Initialize(size_t width, size_t height, VideoSurface sourceSurface, VideoSurface targetSurface)
	{
		if (source != sourceSurface) 
			return false;

		if (target != targetSurface)
			return false;

		return VideoSurfaceConverterMixIn::Initialize(width, height, sourceSurface, targetSurface);
	}

	bool Convert(const void* sourceFrameBytes, size_t sourceFrameByteSize, void* targetFrameBytes, size_t targetFrameByteSize) throw()
	{
		assert(NULL != sourceFrameBytes);
		if (NULL == sourceFrameBytes)
			return false;

		if (GetSourceFrameByteSize() != sourceFrameByteSize)
			return false;

		assert(NULL != targetFrameBytes);
		if (NULL == targetFrameBytes)
			return false;

		if (GetTargetFrameByteSize() != targetFrameByteSize)
			return false;

		return ConvertImpl(sourceFrameBytes, sourceFrameByteSize, targetFrameBytes, targetFrameByteSize);
	}
};

FORCEINLINE BYTE ByteClip(int value) throw()
{
	return (value > 255 ? 255 : (value < 0 ? 0 : BYTE(value)));
}

FORCEINLINE void RgbToYuv_Int(BYTE R, BYTE G, BYTE B, BYTE& Y, BYTE& U, BYTE& V) throw()
{
	int iR = int(R), iG = int(G), iB = int(B);
	Y = ByteClip(( (  66 * iR + 129 * iG +  25 * iB + 128) >> 8) +  16);
	U = ByteClip(( ( -38 * iR -  74 * iG + 112 * iB + 128) >> 8) + 128);
	V = ByteClip(( ( 112 * iR -  94 * iG -  18 * iB + 128) >> 8) + 128);
}

FORCEINLINE void YuvToRgb_Int(BYTE Y, BYTE U, BYTE V, BYTE& R, BYTE& G, BYTE& B) throw()
{
	int C = Y - 16;
	int D = U - 128;
	int E = V - 128;
	R = ByteClip(( 298 * C           + 409 * E + 128) >> 8);
	G = ByteClip(( 298 * C - 100 * D - 208 * E + 128) >> 8);
	B = ByteClip(( 298 * C + 516 * D           + 128) >> 8);
}

// when performing upsampling by 2, from single byte B
// we get 2 values B0 and B1. With Catmull-Rom interpolation
// B0 is equal to B, and B1 is obtained from this function.
// Edge conditions:
//  - when calculating pair for 0th byte, use A1 = CatmullRom(A, A, B, C)
//  - when calculating pair for (N - 2)th byte R, use R1 = CatmullRom(Q, R, S, S)
//  - when calculating pair for (N - 1)th byte S, use S1 = CatmullRom(R, S, S, S)
FORCEINLINE BYTE CatmullRom(BYTE A, BYTE B, BYTE C, BYTE D) throw()
{
	return ByteClip(((9 * (int(B) + int(C))) - (int(A) + int(D)) + 8) >> 4);
}

template<class Num> 
FORCEINLINE bool IsEven(Num num) throw()
{
	return 0 == (num % 2);
}


