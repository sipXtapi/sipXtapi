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
#include <VideoSupport/VideoSurfaceConverter.h>
#include <VideoSupport/VideoSurfaceConverterFactory.h>
#include <map>
#include "VideoSurfaceConverterImpl.h"
#include "AVCodecVideoSurfaceConverter.h"

namespace 
{
	bool factoryCreated = false;
	VideoSurfaceConverterFactory* factory = NULL;
}

void VideoSurfaceConverterFactory::StaticDispose()
{
	if (!factoryCreated)
		return;

	delete factory;
	factory = NULL;
}

VideoSurfaceConverterFactory* VideoSurfaceConverterFactory::GetInstance()
{
	if (NULL == factory && !factoryCreated)
	{
		factoryCreated = true;
		factory = new VideoSurfaceConverterFactory();
	}

	return factory;
}

struct VideoSurfaceConverterFactory::Impl
{
	typedef std::pair<VideoSurface, VideoSurface> VideoSurfacePair;
	typedef std::map<VideoSurfacePair, VideoSurfaceConverterConstructor> ConstructorMap;

	ConstructorMap constructors_;

	VideoSurfaceConverterAutoPtr CreateConverter(size_t width, size_t height, VideoSurface sourceSurface, VideoSurface targetSurface);

	VideoSurfaceConverterAutoPtr CreateTandemConverter(size_t width, size_t height, VideoSurface sourceSurface, VideoSurface targetSurface);

#ifndef VIDEO_SUPPORT_DISABLE_AVCODEC
	//! Returned converter will be already initialized.
	VideoSurfaceConverterAutoPtr CreateAVCodecConverter(size_t width, size_t height, VideoSurface sourceSurface, VideoSurface targetSurface);
#endif // VIDEO_SUPPORT_DISABLE_AVCODEC

};

VideoSurfaceConverterFactory::VideoSurfaceConverterFactory():
	impl_(NULL)
{
	impl_ = new Impl();
}

VideoSurfaceConverterFactory::~VideoSurfaceConverterFactory()
{
	delete impl_;
	impl_ = NULL;
}

VideoSurfaceConverterAutoPtr VideoSurfaceConverterFactory::CreateConverter(size_t width, size_t height, VideoSurface sourceSurface, VideoSurface targetSurface)
{
	return impl_->CreateConverter(width, height, sourceSurface, targetSurface);
}

VideoSurfaceConverterAutoPtr VideoSurfaceConverterFactory::Impl::CreateConverter(size_t width, size_t height, VideoSurface sourceSurface, VideoSurface targetSurface)
{
	VideoSurfaceConverterAutoPtr res;
	if (sourceSurface == targetSurface)
		return res;

	if (!IsVideoSurfaceValid(sourceSurface) || !IsVideoSurfaceValid(targetSurface))
		return res;

#ifndef VIDEO_SUPPORT_DISABLE_AVCODEC
	res = CreateAVCodecConverter(width, height, sourceSurface, targetSurface);
	if (NULL != res.get())
		return res;
#endif // VIDEO_SUPPORT_DISABLE_AVCODEC

	ConstructorMap::iterator it = constructors_.find(std::make_pair(sourceSurface, targetSurface));
	if (constructors_.end() != it)
	{
		VideoSurfaceConverterConstructor construct = it->second;
		res.reset((*construct)());
	}
	else
		res = CreateTandemConverter(width, height, sourceSurface, targetSurface);

	if (NULL == res.get())
		return res;

	if (!res->Initialize(width, height, sourceSurface, targetSurface))
		res.reset();

	return res;
}

VideoSurfaceConverterAutoPtr VideoSurfaceConverterFactory::Impl::CreateTandemConverter(size_t width, size_t height, VideoSurface sourceSurface, VideoSurface targetSurface)
{
	VideoSurfaceConverterAutoPtr res;
	VideoSurface interm = videoSurfaceUnknown;

	// when converting RGB to RGB, use ARGB32 as intermediate format
	if (IsVideoSurfaceRGB(sourceSurface) && IsVideoSurfaceRGB(targetSurface))
		interm = videoSurfaceARGB32;
	// when converting YUV to anything, use AYUV as intermediate format
	else
		interm = videoSurfaceAYUV;

	if (sourceSurface == interm || targetSurface == interm)
		return res;

	res.reset(new TandemVideoSurfaceConverter(interm));
	return res;
}

#ifndef VIDEO_SUPPORT_DISABLE_AVCODEC

VideoSurfaceConverterAutoPtr VideoSurfaceConverterFactory::Impl::CreateAVCodecConverter(size_t width, size_t height, VideoSurface sourceSurface, VideoSurface targetSurface)
{
	VideoSurfaceConverterAutoPtr res;
	res.reset(new AVCodecVideoSurfaceConverter());
	if (!res->Initialize(width, height, sourceSurface, targetSurface))
		res.reset();

	return res;
}

#endif // VIDEO_SUPPORT_DISABLE_AVCODEC

bool VideoSurfaceConverterFactory::RegisterConstructor(VideoSurface sourceSurface, VideoSurface targetSurface, VideoSurfaceConverterConstructor constructor)
{
	VideoSurfaceConverterFactory* inst = GetInstance();
	if (NULL == inst)
		return false;

	assert(NULL != inst->impl_);
	inst->impl_->constructors_.insert(std::make_pair(Impl::VideoSurfacePair(sourceSurface, targetSurface), constructor));
	return true;
}







