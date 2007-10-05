//  
// Copyright (C) 2006-2007 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifdef SIPX_VIDEO

// SYSTEM INCLUDES
#include <windows.h>
#include <VideoSupport/VideoCapture.h>
#include <VideoSupport/VideoFormat.h>
#include <VideoSupport/DepsAutoLink.h>
#include <VideoSupport/VideoCaptureSink.h>

// APPLICATION INCLUDES
#include "mp/video/MpDShowCaptureDevice.h"
#include "os/OsMsgQ.h"
#include "mp/MpMisc.h"
#include "mp/MpVideoBuf.h"
#include "mp/MpBufferMsg.h"
#include "os/OsMsgPool.h"
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

struct MpDShowCaptureDevice::FrameProxy: public VideoCaptureSink
{
   MpDShowCaptureDevice& mDevice;

   FrameProxy(MpDShowCaptureDevice& device): 
      mDevice(device)
   {
   }

   ~FrameProxy()
   {
   }

   bool RenderBuffer(MpVideoBufPtr& buf, const void* frameData, size_t frameSize)
   {
      void* dst = buf->getWriteDataPtr();
      if (NULL == dst)
          return false;

      buf->setFrameWidth(mDevice.mpOutputFormat->GetWidth());
      buf->setFrameHeight(mDevice.mpOutputFormat->GetHeight());
      buf->setColorspace(MpVideoBuf::ColorSpace(mDevice.mColorSpace));

      memcpy(dst, frameData, frameSize);
      // TODO: what to do with timecode? is it used anywhere? what is its format?
      return true;
   }

   void PostBufferMessage(MpVideoBufPtr& ptr)
   {
      MpBufferMsg* msg = (MpBufferMsg*)mDevice.mpMsgPool->findFreeMsg();
      if (NULL == msg)
         msg = new MpBufferMsg(MpBufferMsg::VIDEO_FRAME);
      else
         msg->setMsgSubType(MpBufferMsg::VIDEO_FRAME);

      msg->ownBuffer(ptr);

      if (mDevice.mpFrameQueue->numMsgs() >= mDevice.mpFrameQueue->maxMsgs())
      {
         OsMsg* flushMsg = NULL;
         OsStatus res = mDevice.mpFrameQueue->receive(flushMsg, OsTime::NO_WAIT_TIME);
         if (OS_SUCCESS == res) 
            flushMsg->releaseMsg();
      }
      mDevice.mpFrameQueue->send(*msg, OsTime::NO_WAIT_TIME);
      if (!msg->isMsgReusable())
          delete msg;
   }

   void OfferFrame(const void* frameData, size_t frameSize)
   {
      try
      {
         assert(NULL != MpMisc.VideoFramesPool);
         MpVideoBufPtr buf = MpMisc.VideoFramesPool->getBuffer();
         if (!buf.isValid())
            return;

         if(!RenderBuffer(buf, frameData, frameSize))
            return;

         PostBufferMessage(buf);
      }
      catch (std::bad_alloc&)
      {
      }
   }

};

MpDShowCaptureDevice::MpDShowCaptureDevice(const UtlString& deviceName):
   mDeviceName(deviceName),
   mpCapture(new VideoCapture()),
   mpFrameQueue(new OsMsgQ),
   mpFrameProxy(NULL),
   mpOutputFormat(new VideoFormat),
   mColorSpace(0),
   mPixelFormat(-1),
   mpMsgPool(NULL)
{
   mpOutputFormat->SetFrameRate(10.f);
   mpOutputFormat->SetSize(VideoFormat::size_CIF);
   mpOutputFormat->SetSurface(videoSurfaceI420);

   setOutputColorSpace(MpVideoBuf::MP_COLORSPACE_YUV420p);

   MpBufferMsg msg(MpBufferMsg::VIDEO_FRAME);
   mpMsgPool = new OsMsgPool("MpDShowCaptureDevice", msg, 40, 60, 100, 5, OsMsgPool::SINGLE_CLIENT);

   // create mpFrameProxy using mpFrameQueue as its sink. FrameProxy will 
   // forward frames received from VideoCapture into OsMsgQ
   mpFrameProxy = new FrameProxy(*this);
   mpCapture->SetSink(mpFrameProxy);
}

MpDShowCaptureDevice::~MpDShowCaptureDevice()
{
   delete mpCapture;
   mpCapture = NULL;

   delete mpFrameProxy;
   mpFrameProxy = NULL;

   delete mpFrameQueue;
   mpFrameQueue = NULL;

   delete mpOutputFormat;
   mpOutputFormat = NULL;

   delete mpMsgPool;
   mpMsgPool = NULL;
}

OsStatus MpDShowCaptureDevice::initialize()
{
   assert(NULL != mpCapture);
   assert(NULL != mpFrameProxy);
   assert(NULL != mpFrameQueue);
   assert(NULL != mpOutputFormat);
   assert(NULL != mpMsgPool);

   try
   {
      assert(-1 != mPixelFormat);
      VideoSurface videoSurface = PixelFormatToVideoSurface(PixelFormat(mPixelFormat));
      if (!IsVideoSurfaceValid(videoSurface))
         return OS_FAILED;

      std::string name = mDeviceName.data();
      if (!mpCapture->Initialize(name, NULL, mpCapture->flagPreviewDisabled))
         return OS_FAILED;

      mpOutputFormat->SetSurface(videoSurface);
      if (!mpCapture->SetOutputFormat(mpOutputFormat, true))
      {
         mpCapture->Close();
         return OS_FAILED;
      }

      mpCapture->GetOutputFormat(*mpOutputFormat);
      assert(mpOutputFormat->GetSurface() == videoSurface);
   }
   catch (std::bad_alloc&)
   {
      return OS_NO_MEMORY;
   }

   return OS_SUCCESS;
}

OsStatus MpDShowCaptureDevice::unInitialize()
{
   mpCapture->Close();
   return OS_SUCCESS;
}

OsStatus MpDShowCaptureDevice::startCapture()
{
   if (!mpCapture->Run())
      return OS_FAILED;

   return OS_SUCCESS;
}

OsStatus MpDShowCaptureDevice::stopCapture()
{
   mpCapture->Stop();
   return OS_SUCCESS;
}

OsStatus MpDShowCaptureDevice::setFPS(float fps)
{
   mpOutputFormat->SetFrameRate(fps);
   return applyFormat();
}

OsStatus MpDShowCaptureDevice::setFrameSize(int width, int height)
{
   mpOutputFormat->SetSize(width, height);
   return applyFormat();
}

OsStatus MpDShowCaptureDevice::applyFormat()
{
   if (!mpCapture->IsInitialized())
      return OS_INVALID_STATE;

   if (!mpCapture->SetOutputFormat(mpOutputFormat, true))
      return OS_FAILED;

   mpCapture->GetOutputFormat(*mpOutputFormat);
   return OS_SUCCESS;
}

void MpDShowCaptureDevice::getFrameSize(int& width, int& height) const
{
   width = mpOutputFormat->GetWidth();
   height = mpOutputFormat->GetHeight();
}

float MpDShowCaptureDevice::getFPS() const
{
   return mpOutputFormat->GetFrameRate();
}

OsStatus MpDShowCaptureDevice::setOutputColorSpace(int colorSpace)
{
   int pf = MpVideoBuf::getFFMpegColorspace(MpVideoBuf::ColorSpace(colorSpace));
   if (-1 == pf)
      return OS_FAILED;

   VideoSurface vs = PixelFormatToVideoSurface(PixelFormat(pf));
   if (!IsVideoSurfaceValid(vs))
      return OS_FAILED;

   mColorSpace = colorSpace;
   mPixelFormat = pf;
   mpOutputFormat->SetSurface(vs);

   return applyFormat();
}

#endif // SIPX_VIDEO
