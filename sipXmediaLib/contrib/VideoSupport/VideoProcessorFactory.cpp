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
#include <VideoSupport/VideoFrameProcessor.h>
#include <VideoSupport/VideoProcessorFactory.h>
#include <VideoSupport/VideoFormat.h>
#include <map>

#include "AVCodecVideoScaler.h"

namespace 
{
	bool factoryCreated = false;
	VideoProcessorFactory* factory = NULL;
}

void VideoProcessorFactory::StaticDispose()
{
	if (!factoryCreated)
		return;

	delete factory;
	factory = NULL;
}

VideoProcessorFactory* VideoProcessorFactory::GetInstance()
{
	if (NULL == factory && !factoryCreated)
	{
		factoryCreated = true;
		factory = new VideoProcessorFactory();
	}

	return factory;
}

struct VideoProcessorFactory::Impl
{
	typedef std::pair<VideoProcessorCategory, VideoSurface> CategorySurfacePair;
	typedef std::map<CategorySurfacePair, VideoFrameProcessorConstructor> ConstructorMap;

	ConstructorMap constructors_;

	VideoFrameProcessorAutoPtr CreateProcessor(VideoProcessorCategory category, VideoSurface surface);
};

VideoProcessorFactory::VideoProcessorFactory():
	impl_(NULL)
{
	impl_ = new Impl();
}

VideoProcessorFactory::~VideoProcessorFactory()
{
	delete impl_;
	impl_ = NULL;
}

VideoFrameProcessorAutoPtr VideoProcessorFactory::CreateProcessor(VideoProcessorCategory category, VideoSurface surface)
{
	return impl_->CreateProcessor(category, surface);
}

VideoFrameProcessorAutoPtr VideoProcessorFactory::Impl::CreateProcessor(VideoProcessorCategory category, VideoSurface surface)
{
	VideoFrameProcessorAutoPtr res;
	if (!IsVideoSurfaceValid(surface))
		return res;

#ifndef VIDEO_SUPPORT_DISABLE_AVCODEC
# ifndef VIDEO_SUPPORT_USE_SWSCALER
	if (videoScaler == category)
	{
		// AVCodec img_resample() works only with planar YUV formats :/ - knowledge gained
		// the hard way.
		assert(IsVideoSurfacePlanar(surface) && IsVideoSurfaceYUV(surface));
		if (IsVideoSurfacePlanar(surface) && IsVideoSurfaceYUV(surface))
		{
			PixelFormat pf = VideoSurfaceToPixelFormat(surface);
			if (PIX_FMT_NONE != pf)
			{
				res.reset(new (std::nothrow) AVCodecVideoScaler());
				if (NULL != res.get())
					return res;
			}
		}
	}
# else // VIDEO_SUPPORT_USE_SWSCALER

# endif // VIDEO_SUPPORT_USE_SWSCALER
#endif // VIDEO_SUPPORT_DISABLE_AVCODEC

	ConstructorMap::iterator it = constructors_.find(std::make_pair(category, surface));
	if (constructors_.end() != it)
	{
		VideoFrameProcessorConstructor construct = it->second;
		res.reset((*construct)());
	}
	return res;
}

bool VideoProcessorFactory::RegisterConstructor(VideoProcessorCategory category, VideoSurface surface, VideoFrameProcessorConstructor constructor)
{
	VideoProcessorFactory* inst = GetInstance();
	if (NULL == inst)
		return false;

	assert(NULL != inst->impl_);
	inst->impl_->constructors_.insert(std::make_pair(Impl::CategorySurfacePair(category, surface), constructor));
	return true;
}







