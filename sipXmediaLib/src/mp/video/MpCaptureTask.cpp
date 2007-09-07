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
#include "mp/video/MpCaptureTask.h"
#include "os/OsMsgQ.h"
#include "mp/MpBufferMsg.h"
#include "mp/MpVideoBuf.h"
#include "mp/video/MpvoGdi.h"
#include "mp/video/MpeH264.h"
#include "mp/video/MpVideoCallFlowGraph.h"  ///< For MpVideoCallFlowGraph::smpPreviewWindow

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define CODEC_TYPE_H264 114

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpCaptureTask::MpCaptureTask(OsMsgQ *pMsgQ, MprToNet *pRtpWriter, MpRemoteVideoTask *pTimer, const MpVideoStreamParams* pCaptureParams)
: OsTask("MpCaptureTask", NULL)
, mpMsgQueue(pMsgQ)
, mpRtpWriter(pRtpWriter)
, mpEncoder(new MpeH264(CODEC_TYPE_H264, pRtpWriter))
, mpTimer(pTimer)
{
   assert(mpMsgQueue != NULL);
   if (mpEncoder->initEncode(pCaptureParams) != OS_SUCCESS)
   {
      syslog(FAC_MP, PRI_ERR, "Could not initialize video codec");
      delete mpEncoder;
      mpEncoder = NULL;
   }
}

MpCaptureTask::~MpCaptureTask()
{
   // Try correctly exit thread, if still running.
   stop();

   if (mpEncoder != NULL)
   {
      delete mpEncoder;
      mpEncoder = NULL;
   }
}

/* ============================ MANIPULATORS ============================== */

OsStatus MpCaptureTask::stop()
{
   requestShutdown();
   if (waitUntilShutDown())
      return OS_SUCCESS;
   else
      return OS_FAILED;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

int MpCaptureTask::run(void*)
{
   MpvoGdi videoOutput;

   while (!isShuttingDown())
   {
      MpBufferMsg*    pMsg;

      // Wait 500ms for message, then continue loop.
      if (mpMsgQueue->receive((OsMsg*&)pMsg, OsTime(500)) == OS_SUCCESS)
      {
         MpVideoBufPtr   pFrame;
         pFrame = pMsg->getBuffer();
         pMsg->releaseMsg();

         assert(pFrame.isValid());
         if (!pFrame.isValid())
            continue;

         if (mpEncoder != NULL)
         {
            mpEncoder->encode(pFrame);
         }

         videoOutput.setWindow((HWND)MpVideoCallFlowGraph::getVideoPreviewWindow());
         videoOutput.render(pFrame);
      }
   }

   ackShutdown();

   return 0;
}
/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

#endif // SIPX_VIDEO ]
