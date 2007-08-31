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
#include "stdafx.h"

// #define VIDEO_SURFACE_CONVERTERS_DEBUG

#ifdef VIDEO_SURFACE_CONVERTERS_DEBUG
# define VC_ASSERT(a) assert(a)
#else
# define VC_ASSERT(a) (0)
#endif 

#include "VideoSurfaceConverterImpl.h"
#include <VideoSupport/VideoSurfaceConverterFactory.h>

#pragma pack(push, 1)

struct AYUV
{
	BYTE V;
	BYTE U;
	BYTE Y;
	BYTE A;
};

struct ARGB32
{
	BYTE B;
	BYTE G;
	BYTE R;
	BYTE A;
};

struct RGB24
{
	BYTE B;
	BYTE G;
	BYTE R;
};

#pragma pack(pop)

static const BYTE alphaOpaque = 255;
static const BYTE alphaTransparent = 0;

bool VideoSurfaceConverterMixIn::Initialize(size_t width, size_t height, VideoSurface sourceSurface, VideoSurface targetSurface)
{
	assert(IsVideoSurfaceValid(sourceSurface));
	if (!IsVideoSurfaceValid(sourceSurface))
		return false;

	assert(IsVideoSurfaceValid(targetSurface));
	if (!IsVideoSurfaceValid(targetSurface))
		return false;

	size_t sourceSize = GetVideoFrameByteSize(sourceSurface, width, height);
	if (0 == sourceSize)
		return false;

	size_t targetSize = GetVideoFrameByteSize(targetSurface, width, height);
	if (0 == targetSize)
		return false;

	sourceSurface_ = sourceSurface;
	targetSurface_ = targetSurface;
	width_ = width;
	height_ = height;
	sourceFrameByteSize_ = sourceSize;
	targetFrameByteSize_ = targetSize;
	return true;
}

TandemVideoSurfaceConverter::TandemVideoSurfaceConverter(VideoSurface intermediateSurface):
intermediateSurface_(intermediateSurface),
intermediateFrameByteSize_(0),
intermediateBuffer_(NULL)
{
	assert(IsVideoSurfaceValid(intermediateSurface_));
}

TandemVideoSurfaceConverter::~TandemVideoSurfaceConverter()
{
	sourceConverter_.reset();
	targetConverter_.reset();

	free(intermediateBuffer_);
	intermediateBuffer_ = NULL;
}

bool TandemVideoSurfaceConverter::Initialize(size_t width, size_t height, VideoSurface sourceSurface, VideoSurface targetSurface)
{
	if (!IsVideoSurfaceValid(intermediateSurface_))
		return false;

	size_t intermSize = GetVideoFrameByteSize(intermediateSurface_, width, height);
	if (0 == intermSize)
		return false;


	VideoSurfaceConverterFactory* factory = VideoSurfaceConverterFactory::GetInstance();
	if (NULL == factory)
		return false;

	VideoSurfaceConverterAutoPtr source = factory->CreateConverter(width, height, sourceSurface, intermediateSurface_);
	if (NULL == source.get())
		return false;

	VideoSurfaceConverterAutoPtr target = factory->CreateConverter(width, height, intermediateSurface_, targetSurface);
	if (NULL == target.get())
		return false;

	if (!VideoSurfaceConverterMixIn::Initialize(width, height, sourceSurface, targetSurface))
		return false;

	intermediateBuffer_ = realloc(intermediateBuffer_, intermSize);
	if (NULL == intermediateBuffer_)
		return false;

	intermediateFrameByteSize_ = intermSize;
	sourceConverter_.reset(source.release());
	targetConverter_.reset(target.release());
	return true;
}

bool TandemVideoSurfaceConverter::Convert(const void *sourceFrameBytes, size_t sourceFrameByteSize, void *targetFrameBytes, size_t targetFrameByteSize)
{
	assert(NULL != sourceConverter_.get());
	if (NULL == sourceConverter_.get())
		return false;

	assert(NULL != targetConverter_.get());
	if (NULL == targetConverter_.get())
		return false;

	assert(NULL != intermediateBuffer_);
	if (NULL == intermediateBuffer_)
		return false;

	assert(0 != intermediateFrameByteSize_);
	if (0 == intermediateFrameByteSize_)
		return false;

	if (!sourceConverter_->Convert(sourceFrameBytes, sourceFrameByteSize, intermediateBuffer_, intermediateFrameByteSize_))
		return false;

	return targetConverter_->Convert(intermediateBuffer_, intermediateFrameByteSize_, targetFrameBytes, targetFrameByteSize); 
}

VideoSurfaceConverter* TandemVideoSurfaceConverter::Create(VideoSurface intermediateSurface)
{
	return new TandemVideoSurfaceConverter(intermediateSurface);
}

#define TANDEM_CONVERTER_REGISTER(codeFrom, codeInterm, codeTo) \
static bool converterRegister_ ## codeFrom ## _To_ ## codeTo = VideoSurfaceConverterFactory::RegisterConstructor(videoSurface ## codeFrom, videoSurface ## codeTo, TandemVideoSurfaceConverter::CreateTandem<videoSurface ## codeInterm>)

#define BASIC_SURFACE_CONVERTER_BEGIN(codeFrom, codeTo) \
class Converter_ ## codeFrom ## _To_ ## codeTo: public BasicVideoSurfaceConverter<videoSurface ## codeFrom, videoSurface ## codeTo> { \
public: \
	static VideoSurfaceConverter* Create() {return new Converter_ ## codeFrom ## _To_ ## codeTo ();} \
protected: \
	bool ConvertImpl(const void* sourceFrameBytes, size_t sourceFrameByteSize, void* targetFrameBytes, size_t targetFrameByteSize) throw() { 

#define BASIC_SURFACE_CONVERTER_END(codeFrom, codeTo) \
	} \
}; \
static bool converterRegister_ ## codeFrom ## _To_ ## codeTo = VideoSurfaceConverterFactory::RegisterConstructor(videoSurface ## codeFrom, videoSurface ## codeTo, Converter_ ## codeFrom ## _To_ ## codeTo::Create)


BASIC_SURFACE_CONVERTER_BEGIN(AYUV, ARGB32)
	assert(0 == (sourceFrameByteSize % 4));
	const size_t pixelCount = sourceFrameByteSize / 4;

	assert(sourceFrameByteSize == targetFrameByteSize);
	const BYTE* src = (const BYTE*)sourceFrameBytes;
	BYTE* trg = (BYTE*)targetFrameBytes;
	for (size_t i = 0; i < pixelCount; ++i)
	{
		const AYUV& ayuv = *(const AYUV*)src;
		src += 4;

		ARGB32& argb32 = *(ARGB32*)trg;
		trg += 4;

		argb32.A = ayuv.A;
		YuvToRgb_Int(ayuv.Y, ayuv.U, ayuv.V, argb32.R, argb32.G, argb32.B);

	}
	return true;
BASIC_SURFACE_CONVERTER_END(AYUV, ARGB32);

BASIC_SURFACE_CONVERTER_BEGIN(ARGB32, AYUV)
	assert(0 == (sourceFrameByteSize % 4));
	const size_t pixelCount = sourceFrameByteSize / 4;

	assert(sourceFrameByteSize == targetFrameByteSize);
	const BYTE* src = (const BYTE*)sourceFrameBytes;
	BYTE* trg = (BYTE*)targetFrameBytes;
	for (size_t i = 0; i < pixelCount; ++i)
	{
		const ARGB32& argb32 = *(const ARGB32*)src;
		src += 4;

		AYUV& ayuv = *(AYUV*)trg;
		trg += 4;

		ayuv.A = argb32.A;
		RgbToYuv_Int(argb32.R, argb32.G, argb32.B, ayuv.Y, ayuv.U, ayuv.V);

	}
	return true;
BASIC_SURFACE_CONVERTER_END(ARGB32, AYUV);

BASIC_SURFACE_CONVERTER_BEGIN(AYUV, RGB24)
	assert(0 == (sourceFrameByteSize % 4));
	const size_t pixelCount = sourceFrameByteSize / 4;

	assert(0 == (targetFrameByteSize % 3));
	assert(pixelCount == targetFrameByteSize / 3);

	const BYTE* src = (const BYTE*)sourceFrameBytes;
	BYTE* trg = (BYTE*)targetFrameBytes;
	for (size_t i = 0; i < pixelCount; ++i)
	{
		const AYUV& ayuv = *(const AYUV*)src;
		src += 4;

		RGB24& rgb24 = *(RGB24*)trg;
		trg += 3;

		YuvToRgb_Int(ayuv.Y, ayuv.U, ayuv.V, rgb24.R, rgb24.G, rgb24.B);

	}
	return true;
BASIC_SURFACE_CONVERTER_END(AYUV, RGB24);

BASIC_SURFACE_CONVERTER_BEGIN(RGB24, AYUV)
	assert(0 == (sourceFrameByteSize % 3));
	const size_t pixelCount = sourceFrameByteSize / 3;

	assert(0 == (targetFrameByteSize % 4));
	assert(pixelCount == targetFrameByteSize / 4);

	const BYTE* src = (const BYTE*)sourceFrameBytes;
	BYTE* trg = (BYTE*)targetFrameBytes;
	for (size_t i = 0; i < pixelCount; ++i)
	{
		const RGB24& rgb24 = *(const RGB24*)src;
		src += 3;

		AYUV& ayuv = *(AYUV*)trg;
		trg += 4;

		ayuv.A = alphaOpaque;
		RgbToYuv_Int(rgb24.R, rgb24.G, rgb24.B, ayuv.Y, ayuv.U, ayuv.V);
	}
	return true;
BASIC_SURFACE_CONVERTER_END(RGB24, AYUV);

BASIC_SURFACE_CONVERTER_BEGIN(ARGB32, RGB24)
	assert(0 == (sourceFrameByteSize % 4));
	const size_t pixelCount = sourceFrameByteSize / 4;

	assert(0 == (targetFrameByteSize % 3));
	assert(pixelCount == targetFrameByteSize / 3);

	const BYTE* src = (const BYTE*)sourceFrameBytes;
	BYTE* trg = (BYTE*)targetFrameBytes;
	for (size_t i = 0; i < pixelCount; ++i)
	{
		const ARGB32& argb = *(const ARGB32*)src;
		src += 4;

		RGB24& rgb24 = *(RGB24*)trg;
		trg += 3;
		
		rgb24.B = argb.B;
		rgb24.G = argb.G;
		rgb24.R = argb.R;
	}
	return true;
BASIC_SURFACE_CONVERTER_END(ARGB32, RGB24);

BASIC_SURFACE_CONVERTER_BEGIN(RGB24, ARGB32)
	assert(0 == (sourceFrameByteSize % 3));
	const size_t pixelCount = sourceFrameByteSize / 3;

	assert(0 == (targetFrameByteSize % 4));
	assert(pixelCount == targetFrameByteSize / 4);

	const BYTE* src = (const BYTE*)sourceFrameBytes;
	BYTE* trg = (BYTE*)targetFrameBytes;
	for (size_t i = 0; i < pixelCount; ++i)
	{
		const RGB24& rgb24 = *(const RGB24*)src;
		src += 3;

		ARGB32& argb = *(ARGB32*)trg;
		trg += 4;

		argb.A = alphaOpaque;
		argb.B = rgb24.B;
		argb.G = rgb24.G;
		argb.R = rgb24.R;
	}
	return true;
BASIC_SURFACE_CONVERTER_END(RGB24, ARGB32);

template<VideoSurface source, class YuvMapper>
class Yuv422UpsampleConverter: public BasicVideoSurfaceConverter<source, videoSurfaceAYUV>
{
	FORCEINLINE const BYTE& MapY(const BYTE* src, size_t i) throw()
	{
		const size_t address = mapper_.MapYAddress(i, GetWidth(), GetHeight());
		VC_ASSERT(address < GetSourceFrameByteSize());
		return src[address];
	}

	FORCEINLINE const BYTE& MapU(const BYTE* src, size_t i) throw() 
	{
		const size_t address = mapper_.MapUAddress(i, GetWidth(), GetHeight());
		VC_ASSERT(address < GetSourceFrameByteSize());
		return src[address];
	}

	FORCEINLINE const BYTE& MapV(const BYTE* src, size_t i) throw() 
	{
		const size_t address = mapper_.MapVAddress(i, GetWidth(), GetHeight());
		VC_ASSERT(address < GetSourceFrameByteSize());
		return src[address];
	}

	YuvMapper mapper_;

public:

	static VideoSurfaceConverter* Create() {return new Yuv422UpsampleConverter();}

protected:

	bool ConvertImpl(const void* sourceFrameBytes, size_t sourceFrameByteSize, void* targetFrameBytes, size_t targetFrameByteSize) throw()
	{
		assert(0 == (sourceFrameByteSize % 2));
		const size_t pixelCount = sourceFrameByteSize / 2;

		assert(0 == (targetFrameByteSize % 4));
		assert(pixelCount == targetFrameByteSize / 4);

		const BYTE* src = (const BYTE*)sourceFrameBytes;
		BYTE* trg = (BYTE*)targetFrameBytes;

		const size_t width = GetWidth();
		const size_t srcCrWidth = width / 2;

		for (size_t i = 0; i < pixelCount; ++i)
		{
			AYUV& ayuv = *(AYUV*)trg;
			trg += 4;

			// Y value is not resampled, copy it directly without interpolation
			ayuv.Y = MapY(src, i);
			ayuv.A = alphaOpaque;

			const size_t x = i % width;
			const size_t y = i / width;

			// map the coordinates - horizontal 2:1 downsampling
			const size_t xCr = x / 2;
			const size_t iCr = y * srcCrWidth + xCr;
			if (IsEven(x))
			{
				ayuv.U = MapU(src, iCr);
				ayuv.V = MapV(src, iCr);
			}
			else
			{
				BYTE ua, ub, uc, ud, va, vb, vc, vd;
				if (0 == xCr)
				{
					// a = CR(aabc)
					ua = ub = MapU(src, iCr);
					uc = MapU(src, iCr + 1);
					ud = MapU(src, iCr + 2);

					va = vb = MapV(src, iCr);
					vc = MapV(src, iCr + 1);
					vd = MapV(src, iCr + 2);
				}
				else if (srcCrWidth - 2 == xCr)
				{
					// r = CR(qrss)
					ua = MapU(src, iCr - 1);
					ub = MapU(src, iCr);
					uc = ud = MapU(src, iCr + 1);

					va = MapV(src, iCr - 1);
					vb = MapV(src, iCr);
					vc = vd = MapV(src, iCr + 1);
				}
				else if (srcCrWidth - 1 == xCr)
				{
					// s = CR(rsss)
					ua = MapU(src, iCr - 1);
					ub = uc = ud = MapU(src, iCr);

					va = MapV(src, iCr - 1);
					vb = vc = vd = MapV(src, iCr);
				}
				else
				{
					// b = CR(abcd)
					ua = MapU(src, iCr - 1);
					ub = MapU(src, iCr);
					uc = MapU(src, iCr + 1);
					ud = MapU(src, iCr + 2);

					va = MapV(src, iCr - 1);
					vb = MapV(src, iCr);
					vc = MapV(src, iCr + 1);
					vd = MapV(src, iCr + 2);
				}

				ayuv.U = CatmullRom(ua, ub, uc, ud);
				ayuv.V = CatmullRom(va, vb, vc, vd);
			}
		}
		return true;
	}
};

#define YUV422_UPSAMPLER_REGISTER(codeFrom) \
static bool converterRegister_ ## codeFrom ## _To_AYUV = VideoSurfaceConverterFactory::RegisterConstructor(videoSurface ## codeFrom, videoSurfaceAYUV, Yuv422UpsampleConverter<videoSurface ## codeFrom, codeFrom ## _Mapper>::Create)

template<VideoSurface target, class YuvMapper>
class Yuv422DownsampleConverter: public BasicVideoSurfaceConverter<videoSurfaceAYUV, target>
{
	FORCEINLINE BYTE& MapY(BYTE* trg, size_t i) throw() 
	{
		const size_t address = mapper_.MapYAddress(i, GetWidth(), GetHeight());
		VC_ASSERT(address < GetTargetFrameByteSize());
		return trg[address];
	}

	FORCEINLINE BYTE& MapU(BYTE* trg, size_t i) throw() 
	{
		const size_t address = mapper_.MapUAddress(i, GetWidth(), GetHeight());
		VC_ASSERT(address < GetTargetFrameByteSize());
		return trg[address];
	}

	FORCEINLINE BYTE& MapV(BYTE* trg, size_t i) throw() 
	{
		const size_t address = mapper_.MapVAddress(i, GetWidth(), GetHeight());
		VC_ASSERT(address < GetTargetFrameByteSize());
		return trg[mapper_.MapVAddress(i, GetWidth(), GetHeight())];
	}

	YuvMapper mapper_;
public:

	static VideoSurfaceConverter* Create() {return new Yuv422DownsampleConverter();}

protected:

	bool ConvertImpl(const void* sourceFrameBytes, size_t sourceFrameByteSize, void* targetFrameBytes, size_t targetFrameByteSize) throw()
	{
		assert(0 == (sourceFrameByteSize % 4));
		const size_t pixelCount = sourceFrameByteSize / 4;

		assert(0 == (targetFrameByteSize % 2));
		assert(pixelCount == targetFrameByteSize / 2);

		const BYTE* src = (const BYTE*)sourceFrameBytes;
		BYTE* trg = (BYTE*)targetFrameBytes;

		const size_t width = GetWidth();
		const size_t srcCrWidth = width / 2;

		for (size_t i = 0; i < pixelCount; ++i)
		{
			const AYUV& ayuv = *(const AYUV*)src;
			src += 4;

			// Y value is not resampled, copy it directly without interpolation
			MapY(trg, i) = ayuv.Y;

			const size_t x = i % width;
			const size_t y = i / width;

			// map the coordinates - horizontal 2:1 downsampling
			const size_t xCr = x / 2;
			const size_t iCr = y * srcCrWidth + xCr;

			// TODO: probably we should apply more sophisticated LPF to chroma to avoid aliasing. 
			// Until this gets really annoying we stick to this over-simplified 
			// implementation using averaging.
			if (!IsEven(x))
				continue;

			const AYUV& nextPixel = *(const AYUV*)src;
			MapU(trg, iCr) = BYTE((int(ayuv.U) + int(nextPixel.U)) / 2);
			MapV(trg, iCr) = BYTE((int(ayuv.V) + int(nextPixel.V)) / 2);
		}
		return true;
	}
};

#define YUV422_DOWNSAMPLER_REGISTER(codeTo) \
static bool converterRegister_AYUV_To_ ## codeTo = VideoSurfaceConverterFactory::RegisterConstructor(videoSurfaceAYUV, videoSurface ## codeTo, Yuv422DownsampleConverter<videoSurface ## codeTo, codeTo ## _Mapper>::Create)

struct UYVY_Mapper
{
	FORCEINLINE size_t MapYAddress(size_t n, size_t width, size_t height) throw()
	{
		VC_ASSERT(n < width * height);
		return 2 * n + 1;
	}

	FORCEINLINE size_t MapUAddress(size_t n, size_t width, size_t height) throw()
	{
		VC_ASSERT(n < width * height / 2);
		return 4 * n;
	}

	FORCEINLINE size_t MapVAddress(size_t n, size_t width, size_t height) throw()
	{
		VC_ASSERT(n < width * height / 2);
		return 4 * n + 2;
	}
};


YUV422_UPSAMPLER_REGISTER(UYVY);
YUV422_DOWNSAMPLER_REGISTER(UYVY);

struct YUY2_Mapper
{
	FORCEINLINE size_t MapYAddress(size_t n, size_t width, size_t height) throw()
	{
		VC_ASSERT(n < width * height);
		return 2 * n;
	}

	FORCEINLINE size_t MapUAddress(size_t n, size_t width, size_t height) throw()
	{
		VC_ASSERT(n < width * height / 2);
		return 4 * n + 1;
	}

	FORCEINLINE size_t MapVAddress(size_t n, size_t width, size_t height) throw()
	{
		VC_ASSERT(n < width * height / 2);
		return 4 * n + 3;
	}
};

YUV422_UPSAMPLER_REGISTER(YUY2);
YUV422_DOWNSAMPLER_REGISTER(YUY2);

struct YVYU_Mapper
{
	FORCEINLINE size_t MapYAddress(size_t n, size_t width, size_t height) throw()
	{
		VC_ASSERT(n < width * height);
		return 2 * n;
	}

	FORCEINLINE size_t MapUAddress(size_t n, size_t width, size_t height) throw()
	{
		VC_ASSERT(n < width * height / 2);
		return 4 * n + 3;
	}

	FORCEINLINE size_t MapVAddress(size_t n, size_t width, size_t height) throw()
	{
		VC_ASSERT(n < width * height / 2);
		return 4 * n + 1;
	}
};

YUV422_UPSAMPLER_REGISTER(YVYU);
YUV422_DOWNSAMPLER_REGISTER(YVYU);

template<VideoSurface target, class YuvMapper>
class Yuv420DownsampleConverter: public BasicVideoSurfaceConverter<videoSurfaceAYUV, target>
{
	FORCEINLINE BYTE& MapY(BYTE* trg, size_t i) throw() 
	{
		const size_t address = mapper_.MapYAddress(i, GetWidth(), GetHeight());
		VC_ASSERT(address < GetTargetFrameByteSize());
		return trg[address];
	}

	FORCEINLINE BYTE& MapU(BYTE* trg, size_t i) throw() 
	{
		const size_t address = mapper_.MapUAddress(i, GetWidth(), GetHeight());
		VC_ASSERT(address < GetTargetFrameByteSize());
		return trg[address];
	}
	
	FORCEINLINE BYTE& MapV(BYTE* trg, size_t i) throw() 
	{
		const size_t address = mapper_.MapVAddress(i, GetWidth(), GetHeight());
		VC_ASSERT(address < GetTargetFrameByteSize());
		return trg[address];
	}

	YuvMapper mapper_;
public:

	static VideoSurfaceConverter* Create() {return new Yuv420DownsampleConverter();}

protected:

	bool ConvertImpl(const void* sourceFrameBytes, size_t sourceFrameByteSize, void* targetFrameBytes, size_t targetFrameByteSize) throw()
	{
		assert(0 == (sourceFrameByteSize % 4));
		const size_t pixelCount = sourceFrameByteSize / 4;

		// 4:2:0 formats use both 16bpp and 12bpp, so no single way to transform targetFrameByteSize into pixelCount
		// we assume it is correct (this is actually enforced by Initialize()).

		const BYTE* src = (const BYTE*)sourceFrameBytes;
		BYTE* trg = (BYTE*)targetFrameBytes;

		const size_t width = GetWidth();
		const size_t srcCrWidth = width / 2;
		const size_t srcCrHeight = width / 2;

		for (size_t i = 0; i < pixelCount; ++i)
		{
			const AYUV& ayuv = *(const AYUV*)src;
			src += 4;

			// Y value is not resampled, copy it directly without interpolation
			MapY(trg, i) = ayuv.Y;

			const size_t x = i % width;
			const size_t y = i / width;

			// map the coordinates - horizontal and vertical 2:1 downsampling
			const size_t xCr = x / 2;
			const size_t yCr = y / 2;
			const size_t iCr = yCr * srcCrWidth + xCr;

			// TODO: probably we should apply more sophisticated LPF to chroma to avoid aliasing. 
			// Until this gets really annoying we stick to this over-simplified 
			// implementation using averaging.
			if (!IsEven(x) || !IsEven(y))
				continue;

			const AYUV& nextX = *(const AYUV*)src;
			const AYUV& nextY = *(((const AYUV*)src) + GetWidth() - 1);
			const AYUV& nextXY = *(((const AYUV*)src) + GetWidth());

			MapU(trg, iCr) = BYTE((int(ayuv.U) + int(nextX.U) + int(nextY.U) + int(nextXY.U)) / 4);
			MapV(trg, iCr) = BYTE((int(ayuv.V) + int(nextX.V) + int(nextY.V) + int(nextXY.V)) / 4);
		}
		return true;
	}
};

#define YUV420_DOWNSAMPLER_REGISTER(codeTo) \
static bool converterRegister_AYUV_To_ ## codeTo = VideoSurfaceConverterFactory::RegisterConstructor(videoSurfaceAYUV, videoSurface ## codeTo, Yuv420DownsampleConverter<videoSurface ## codeTo, codeTo ## _Mapper>::Create)

struct IMC1_Mapper
{
	FORCEINLINE size_t MapYAddress(size_t n, size_t width, size_t height) throw()
	{
		VC_ASSERT(n < width * height);
		return n;
	}

	FORCEINLINE size_t MapVAddress(size_t n, size_t width, size_t height) throw()
	{
		VC_ASSERT(n < width * height / 4);

		const size_t stride = width / 2;
		const size_t l = n / stride;
		const size_t c = n % stride;
		return width * (height + l) + c;
	}

	FORCEINLINE size_t MapUAddress(size_t n, size_t width, size_t height) throw()
	{
		VC_ASSERT(n < width * height / 4);

		const size_t stride = width / 2;
		const size_t l = n / stride;
		const size_t c = n % stride;

		return width * (height + height / 2 + l) + c;
	}
};

struct IMC3_Mapper
{
	FORCEINLINE size_t MapYAddress(size_t n, size_t width, size_t height) throw()
	{
		VC_ASSERT(n < width * height);
		return n;
	}

	FORCEINLINE size_t MapUAddress(size_t n, size_t width, size_t height) throw()
	{
		VC_ASSERT(n < width * height / 4);

		const size_t stride = width / 2;
		const size_t l = n / stride;
		const size_t c = n % stride;
		return width * (height + l) + c;
	}

	FORCEINLINE size_t MapVAddress(size_t n, size_t width, size_t height) throw()
	{
		VC_ASSERT(n < width * height / 4);

		const size_t stride = width / 2;
		const size_t l = n / stride;
		const size_t c = n % stride;

		return width * (height + height / 2 + l) + c;
	}
};

struct IMC2_Mapper
{
	FORCEINLINE size_t MapYAddress(size_t n, size_t width, size_t height) throw()
	{
		VC_ASSERT(n < width * height);
		return n;
	}

	FORCEINLINE size_t MapVAddress(size_t n, size_t width, size_t height) throw()
	{
		VC_ASSERT(n < width * height / 4);

		const size_t stride = width / 2;
		const size_t l = n / stride;
		const size_t c = n % stride;

		return width * (height + l) + c;
	}

	FORCEINLINE size_t MapUAddress(size_t n, size_t width, size_t height) throw()
	{
		VC_ASSERT(n < width * height / 4);

		const size_t stride = width / 2;
		const size_t l = n / stride;
		const size_t c = n % stride;

		return width * (height + l) + stride + c;
	}
};

struct IMC4_Mapper
{
	FORCEINLINE size_t MapYAddress(size_t n, size_t width, size_t height) throw()
	{
		VC_ASSERT(n < width * height);
		return n;
	}

	FORCEINLINE size_t MapVAddress(size_t n, size_t width, size_t height) throw()
	{
		VC_ASSERT(n < width * height / 4);

		const size_t stride = width / 2;
		const size_t l = n / stride;
		const size_t c = n % stride;

		return width * (height + l) + stride + c;
	}

	FORCEINLINE size_t MapUAddress(size_t n, size_t width, size_t height) throw()
	{
		VC_ASSERT(n < width * height / 4);

		const size_t stride = width / 2;
		const size_t l = n / stride;
		const size_t c = n % stride;

		return width * (height + l) + c;
	}
};

struct YV12_Mapper
{
	FORCEINLINE size_t MapYAddress(size_t n, size_t width, size_t height) throw()
	{
		VC_ASSERT(n < width * height);
		return n;
	}

	FORCEINLINE size_t MapVAddress(size_t n, size_t width, size_t height) throw()
	{
		VC_ASSERT(n < width * height / 4);

		return width * height + n;
	}

	FORCEINLINE size_t MapUAddress(size_t n, size_t width, size_t height) throw()
	{
		VC_ASSERT(n < width * height / 4);

		return width * (height + height / 4) + n;
	}
};

struct IYUV_Mapper
{
	FORCEINLINE size_t MapYAddress(size_t n, size_t width, size_t height) throw()
	{
		VC_ASSERT(n < width * height);
		return n;
	}

	FORCEINLINE size_t MapUAddress(size_t n, size_t width, size_t height) throw()
	{
		VC_ASSERT(n < width * height / 4);

		return width * height + n;
	}

	FORCEINLINE size_t MapVAddress(size_t n, size_t width, size_t height) throw()
	{
		VC_ASSERT(n < width * height / 4);

		return width * (height + height / 4) + n;
	}
};

struct NV12_Mapper
{
	FORCEINLINE size_t MapYAddress(size_t n, size_t width, size_t height) throw()
	{
		VC_ASSERT(n < width * height);
		return n;
	}

	FORCEINLINE size_t MapVAddress(size_t n, size_t width, size_t height) throw()
	{
		VC_ASSERT(n < width * height / 4);

		return width * height + 2 * n + 1;
	}

	FORCEINLINE size_t MapUAddress(size_t n, size_t width, size_t height) throw()
	{
		VC_ASSERT(n < width * height / 4);

		return width * height + 2 * n;
	}
};

YUV420_DOWNSAMPLER_REGISTER(IMC1);
YUV420_DOWNSAMPLER_REGISTER(IMC3);
YUV420_DOWNSAMPLER_REGISTER(IMC2);
YUV420_DOWNSAMPLER_REGISTER(IMC4);
YUV420_DOWNSAMPLER_REGISTER(YV12);
YUV420_DOWNSAMPLER_REGISTER(IYUV);
YUV420_DOWNSAMPLER_REGISTER(NV12);

// Convert 4:2:0 to 4:2:2
template<VideoSurface source, VideoSurface target, class Yuv420Mapper, class Yuv422Mapper>
class Yuv420VerticalUpsampleConverter: public BasicVideoSurfaceConverter<source, target>
{
	FORCEINLINE const BYTE& MapSrcY(const BYTE* src, size_t i) throw() 
	{
		const size_t address = srcMap_.MapYAddress(i, GetWidth(), GetHeight());
		VC_ASSERT(address < GetSourceFrameByteSize());
	
		return src[address];
	}

	FORCEINLINE const BYTE& MapSrcU(const BYTE* src, size_t i) throw() 
	{
		const size_t address = srcMap_.MapUAddress(i, GetWidth(), GetHeight());
		VC_ASSERT(address < GetSourceFrameByteSize());
		return src[address];
	}

	FORCEINLINE const BYTE& MapSrcV(const BYTE* src, size_t i) throw() 
	{
		const size_t address = srcMap_.MapVAddress(i, GetWidth(), GetHeight());
		VC_ASSERT(address < GetSourceFrameByteSize());
		return src[address];
	}

	FORCEINLINE BYTE& MapTrgY(BYTE* trg, size_t i) throw() 
	{
		const size_t address = trgMap_.MapYAddress(i, GetWidth(), GetHeight());
		VC_ASSERT(address < GetTargetFrameByteSize());
		return trg[address];
	}

	FORCEINLINE BYTE& MapTrgU(BYTE* trg, size_t i) throw() 
	{
		const size_t address = trgMap_.MapUAddress(i, GetWidth(), GetHeight());
		VC_ASSERT(address < GetTargetFrameByteSize());
		return trg[address];
	}

	FORCEINLINE BYTE& MapTrgV(BYTE* trg, size_t i) throw() 
	{
		const size_t address = trgMap_.MapVAddress(i, GetWidth(), GetHeight());
		VC_ASSERT(address < GetTargetFrameByteSize());
		return trg[address];
	}

	Yuv420Mapper srcMap_;
	Yuv422Mapper trgMap_;

public:

	static VideoSurfaceConverter* Create() {return new Yuv420VerticalUpsampleConverter();}

protected:

	bool ConvertImpl(const void* sourceFrameBytes, size_t sourceFrameByteSize, void* targetFrameBytes, size_t targetFrameByteSize) throw()
	{
		// target is 16bpp 4:2:2
		assert(0 == (targetFrameByteSize % 2));
		const size_t pixelCount = targetFrameByteSize / 2;

		const BYTE* src = (const BYTE*)sourceFrameBytes;
		BYTE* trg = (BYTE*)targetFrameBytes;

		const size_t width = GetWidth();
		const size_t srcCrWidth = width / 2;
		const size_t srcCrHeight = width / 2;

		for (size_t i = 0; i < pixelCount; ++i)
		{
			// copy luma directly
			MapTrgY(trg, i) = MapSrcY(src, i);

			const size_t x = i % width;
			const size_t y = i / width;

			// skip odd chroma columns
			if (!IsEven(x))
				continue;

			const size_t xCr = x / 2;
			const size_t ySrcCr = y / 2;
			const size_t iTrgCr = y * srcCrWidth + xCr;
			const size_t iSrcCr = ySrcCr * srcCrWidth + xCr;

			if (IsEven(y))
			{
				MapTrgU(trg, iTrgCr) = MapSrcU(src, iSrcCr);
				MapTrgV(trg, iTrgCr) = MapSrcV(src, iSrcCr);
			}
			else
			{
				BYTE ua, ub, uc, ud, va, vb, vc, vd;
				if (0 == ySrcCr)
				{
					// a = CR(aabc)
					ua = ub = MapSrcU(src, iSrcCr);
					uc = MapSrcU(src, iSrcCr + srcCrWidth);
					ud = MapSrcU(src, iSrcCr + 2 * srcCrWidth);

					va = vb = MapSrcV(src, iSrcCr);
					vc = MapSrcV(src, iSrcCr + srcCrWidth);
					vd = MapSrcV(src, iSrcCr + 2 * srcCrWidth);
				}
				else if (srcCrHeight - 2 == ySrcCr)
				{
					// r = CR(qrss)
					ua = MapSrcU(src, iSrcCr - srcCrWidth);
					ub = MapSrcU(src, iSrcCr);
					uc = ud = MapSrcU(src, iSrcCr + srcCrWidth);

					va = MapSrcV(src, iSrcCr - srcCrWidth);
					vb = MapSrcV(src, iSrcCr);
					vc = vd = MapSrcV(src, iSrcCr + srcCrWidth);
				}
				else if (srcCrHeight - 1 == ySrcCr)
				{
					// s = CR(rsss)
					ua = MapSrcU(src, iSrcCr - srcCrWidth);
					ub = uc = ud = MapSrcU(src, iSrcCr);

					va = MapSrcV(src, iSrcCr - srcCrWidth);
					vb = vc = vd = MapSrcV(src, iSrcCr);
				}
				else
				{
					// b = CR(abcd)
					ua = MapSrcU(src, iSrcCr - srcCrWidth);
					ub = MapSrcU(src, iSrcCr);
					uc = MapSrcU(src, iSrcCr + srcCrWidth);
					ud = MapSrcU(src, iSrcCr + 2 * srcCrWidth);

					va = MapSrcV(src, iSrcCr - srcCrWidth);
					vb = MapSrcV(src, iSrcCr);
					vc = MapSrcV(src, iSrcCr + srcCrWidth);
					vd = MapSrcV(src, iSrcCr + 2 * srcCrWidth);
				}

				MapTrgU(trg, iTrgCr) = CatmullRom(ua, ub, uc, ud);
				MapTrgV(trg, iTrgCr) = CatmullRom(va, vb, vc, vd);
			}
		}
		return true;
	}
};

#define YUV420_VERTICAL_UPSAMPLER_REGISTER(codeFrom, codeTo) \
static bool converterRegister_ ## codeFrom ## _To_ ## codeTo = VideoSurfaceConverterFactory::RegisterConstructor(videoSurface ## codeFrom, videoSurface ## codeTo, Yuv420VerticalUpsampleConverter<videoSurface ## codeFrom, videoSurface ## codeTo, codeFrom ## _Mapper, codeTo ## _Mapper>::Create)

YUV420_VERTICAL_UPSAMPLER_REGISTER(IMC1, UYVY);
YUV420_VERTICAL_UPSAMPLER_REGISTER(IMC3, UYVY);
YUV420_VERTICAL_UPSAMPLER_REGISTER(IMC2, UYVY);
YUV420_VERTICAL_UPSAMPLER_REGISTER(IMC4, UYVY);
YUV420_VERTICAL_UPSAMPLER_REGISTER(IYUV, UYVY);
YUV420_VERTICAL_UPSAMPLER_REGISTER(YV12, UYVY);
YUV420_VERTICAL_UPSAMPLER_REGISTER(NV12, UYVY);

TANDEM_CONVERTER_REGISTER(IMC1, UYVY, AYUV);
TANDEM_CONVERTER_REGISTER(IMC3, UYVY, AYUV);
TANDEM_CONVERTER_REGISTER(IMC2, UYVY, AYUV);
TANDEM_CONVERTER_REGISTER(IMC4, UYVY, AYUV);
TANDEM_CONVERTER_REGISTER(YV12, UYVY, AYUV);
TANDEM_CONVERTER_REGISTER(IYUV, UYVY, AYUV);
TANDEM_CONVERTER_REGISTER(NV12, UYVY, AYUV);

