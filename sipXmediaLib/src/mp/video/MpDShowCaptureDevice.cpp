//  
// Copyright (C) 2006-2007 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// SYSTEM INCLUDES
#include <windows.h>
#include <VideoSupport/VideoCapture.h>
#include <VideoSupport/VideoFormat.h>
#include <VideoSupport/DepsAutoLink.h>
#include <VideoSupport/VideoCaptureSink.h>

// APPLICATION INCLUDES
#include "mp/video/MpDShowCaptureDevice.h"
#include "os/OsMsgQ.h"
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

struct MpDShowCaptureDevice::FrameProxy: public VideoCaptureSink
{
   OsMsgQ& mFrameSink;

   FrameProxy(OsMsgQ& frameSink): 
   mFrameSink(frameSink)
   {
   }

   ~FrameProxy()
   {
   }

   void OfferFrame(const void* frameData, size_t frameSize)
   {
      // TODO: render frameData into MpVideoBuf and post it down the mFrameSink
   }
};

MpDShowCaptureDevice::MpDShowCaptureDevice(const UtlString& deviceName):
   mDeviceName(deviceName),
   mpCapture(new VideoCapture()),
   mpFrameQueue(new OsMsgQ),
   mpFrameProxy(NULL),
   mFrameWidth(0),
   mFrameHeight(0),
   mFPS(0.f)
{
   // create mpFrameProxy using mpFrameQueue as its sink. FrameProxy will 
   // forward frames received from VideoCapture into OsMsgQ
   mpFrameProxy = new FrameProxy(*mpFrameQueue);
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
}

OsStatus MpDShowCaptureDevice::initialize()
{
   try
   {
      std::string name = mDeviceName.data();
      if (!mpCapture->Initialize(name, NULL))
         return OS_FAILED;

      VideoFormat format;
      if (!mpCapture->GetCaptureFormat(format))
      {
         mpCapture->Close();
         return OS_FAILED;
      }

      bool applyFormat = false;
      if (0 != mFrameWidth)
      {
         format.SetWidth(mFrameWidth);
         applyFormat = true;
      }
      else
      {
         mFrameWidth = format.width;
      }

      if (0 != mFrameHeight)
      {
         format.SetHeight(mFrameHeight);
         applyFormat = true;
      }
      else
      {
         mFrameHeight = format.height;
      }

      if (0.f != mFPS)
      {
         format.SetFrameRate(mFPS);
         applyFormat = true;
      }
      else
      {
         mFPS = format.fps;
      }

      if (applyFormat && !mpCapture->SetCaptureFormat(format))
      {
         mpCapture->Close();
         return OS_FAILED;
      }

      format.surface = videoSurfaceYV12;
      if (!mpCapture->SetCaptureFormat(format) &&
          !mpCapture->SetOutputVideoSurface(format.GetSurface()))
      {
         mpCapture->Close();
         return OS_FAILED;
      }
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
   mFPS = fps;
   return applyFormat();
}

OsStatus MpDShowCaptureDevice::setFrameSize(int width, int height)
{
   mFrameWidth = width;
   mFrameHeight = height;
   return applyFormat();
}

OsStatus MpDShowCaptureDevice::applyFormat()
{
   if (!mpCapture->IsInitialized())
      return OS_FAILED;

   VideoFormat format;
   if (!mpCapture->GetCaptureFormat(format))
      return OS_FAILED;

   format.SetSize(mFrameWidth, mFrameHeight);
   format.SetFrameRate(mFPS);
   if (!mpCapture->SetCaptureFormat(format))
      return OS_FAILED;

   return OS_SUCCESS;
}

void MpDShowCaptureDevice::getFrameSize(int& width, int& height) const
{
   VideoFormat format;
   if (mpCapture->GetCaptureFormat(format))
   {
      mFrameWidth = format.width;
      mFrameHeight = format.height;
   }

   width = mFrameWidth;
   height = mFrameHeight;
}

float MpDShowCaptureDevice::getFPS() const
{
   VideoFormat format;
   if (mpCapture->GetCaptureFormat(format))
   {
      mFPS = format.fps;
   }

   return mFPS;
}

