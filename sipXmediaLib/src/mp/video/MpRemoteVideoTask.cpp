//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifdef SIPX_VIDEO // [

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/video/MpRemoteVideoTask.h"
#include "mp/MprDejitter.h"
#include "mp/video/MpdH264.h"
#include "mp/MpMisc.h"
#include "mp/video/MpvoGdi.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern void *ghVideo;
// CONSTANTS
#define CODEC_TYPE_H264 125
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpRemoteVideoTask::MpRemoteVideoTask(MprDejitter* pDejitter, void *hwnd)
: mpDejitter(pDejitter)
, mpDecoder(new MpdH264(CODEC_TYPE_H264, MpMisc.VideoFramesPool))
, mpVideoOut(NULL)
{
   if (mpDecoder != NULL)
   {
      mpDecoder->initDecode();
   }

   mpVideoOut = new MpvoGdi(NULL);
   assert(mpVideoOut != NULL);
}

MpRemoteVideoTask::~MpRemoteVideoTask()
{
   if (mpDecoder != NULL)
   {
      delete mpDecoder;
      mpDecoder = NULL;
   }

   if (mpVideoOut != NULL)
   {
      delete mpVideoOut;
      mpVideoOut = NULL;
   }
}

/* ============================ MANIPULATORS ============================== */

void MpRemoteVideoTask::setRemoteVideoWindow(const void *hwnd)
{
   mpVideoOut->setWindow((HWND)hwnd);
}

void MpRemoteVideoTask::step()
{
   if (mpDejitter != NULL)
   {
      MpRtpBufPtr pRtpPacket = mpDejitter->pullPacket(CODEC_TYPE_H264);

      if (  pRtpPacket.isValid()
         && (mpDecoder != NULL) )
      {
         // TODO:: We need loop here!!!!!

         MpVideoBufPtr pFrame = mpDecoder->decode(pRtpPacket);

         if ( (pFrame.isValid()) && (mpVideoOut != NULL) )
         {
            mpVideoOut->render(pFrame);
         }
      }
   }
}

OsStatus MpRemoteVideoTask::stop()
{
   requestShutdown();
   if (waitUntilShutDown())
      return OS_SUCCESS;
   else
      return OS_FAILED;
}

/* ============================ ACCESSORS ================================= */

void *MpRemoteVideoTask::getRemoteVideoWindow() const
{
   return mpVideoOut->getWindow();
}

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

int MpRemoteVideoTask::run(void* pArg)
{
   while (!isShuttingDown())
   {
      step();
#ifdef _WIN32
       Sleep(10);  // TODO:: Need better synchronization
#endif
   }

   return 0;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */


#endif // SIPX_VIDEO ]
