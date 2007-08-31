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

	VideoFrameProcessorAutoPtr CreateProcessor(VideoProcessorCategory category, VideoSurface surface, size_t width, size_t height);

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

VideoFrameProcessorAutoPtr VideoProcessorFactory::CreateProcessor(VideoProcessorCategory category, VideoSurface surface, size_t width, size_t height)
{
	return impl_->CreateProcessor(category, surface, width, height);
}

VideoFrameProcessorAutoPtr VideoProcessorFactory::Impl::CreateProcessor(VideoProcessorCategory category, VideoSurface surface, size_t width, size_t height)
{
	VideoFrameProcessorAutoPtr res;
	if (!IsVideoSurfaceValid(surface))
		return res;

	ConstructorMap::iterator it = constructors_.find(std::make_pair(category, surface));
	if (constructors_.end() != it)
	{
		VideoFrameProcessorConstructor construct = it->second;
		res.reset((*construct)());
	}

	if (NULL == res.get())
		return res;

	if (!res->Initialize(surface, width, height))
		res.reset();

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







