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
#include "sdp/SdpCodec.h"
#include "mp/video/MpvoGdi.h"
#include "mp/video/MpeH264.h"
#include "mp/video/MpeH263.h"
#include "mp/video/MpeH263p.h"
#include "mp/video/MpVideoCallFlowGraph.h"  ///< For MpVideoCallFlowGraph::smpPreviewWindow

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpCaptureTask::MpCaptureTask(OsMsgQ *pMsgQ, MprToNet *pRtpWriter, MpRemoteVideoTask *pTimer, const MpVideoStreamParams* pCaptureParams)
: OsTask("MpCaptureTask", NULL)
, mpMsgQueue(pMsgQ)
, mpRtpWriter(pRtpWriter)
, mpEncoder(NULL)
, mpVideoParams(NULL)
, mpTimer(pTimer)
{
   assert(mpMsgQueue != NULL);

   if (NULL != pCaptureParams)
   {
      mpVideoParams = new MpVideoStreamParams(*pCaptureParams);
   }
}

MpCaptureTask::~MpCaptureTask()
{
   // Try correctly exit thread, if still running.
   stop();

   delete mpEncoder;
   mpEncoder = NULL;

   delete mpVideoParams;
   mpVideoParams = NULL;
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

OsStatus MpCaptureTask::applyCodec(const SdpCodec& codec)
{
   if (NULL == mpVideoParams)
      return OS_FAILED;

   if (NULL == mpRtpWriter)
      return OS_FAILED;

   UtlString mimeSubtype;
   codec.getEncodingName(mimeSubtype);

   // first: check if we have currently active codec
   if (NULL != mpEncoder)
   {
      // if so, it is currently unsupported to change the codec type on the fly.
      // such change would require us to stop the graph, reinitialize the codec
      // and restart the graph, what I believe is not really useful - I can't 
      // imagine the situation that primary codec is changed after session is 
      // established.
      // verify, that passed-in SdpCodec describes exactly the same codec as the 
      // one currently selected. if so - we're done, ohterwise - fail.

      bool valid = false;
      // TODO: using dynamic_cast to check codec validity is a hack. Need to 
      // find some better way to identify MpVideoEncoder.
      if (MIME_SUBTYPE_H263 == mimeSubtype)
         valid = (NULL != dynamic_cast<MpeH263*>(mpEncoder));
      else if (MIME_SUBTYPE_H264 == mimeSubtype)
         valid = (NULL != dynamic_cast<MpeH264*>(mpEncoder));
      else if (MIME_SUBTYPE_H263_1998 == mimeSubtype || MIME_SUBTYPE_H263_2000 == mimeSubtype)
         valid = (NULL != dynamic_cast<MpeH263p*>(mpEncoder));
      else
      {
         assert(!"Unsupported video encoder.");
         valid = false;
      }
      return (valid ? OS_SUCCESS : OS_FAILED);
   }

   // we get here only on when mpEncoder is NULL
   assert(NULL == mpEncoder);

   MpVideoEncoder* encoder = NULL;
   if (MIME_SUBTYPE_H263 == mimeSubtype)
   {
      encoder = new MpeH263(codec.getCodecPayloadFormat(), mpRtpWriter);
   }
   else if (MIME_SUBTYPE_H264 == mimeSubtype)
   {
      encoder = new MpeH264(codec.getCodecPayloadFormat(), mpRtpWriter);
   }
   else if (MIME_SUBTYPE_H263_1998 == mimeSubtype || MIME_SUBTYPE_H263_2000 == mimeSubtype)
   {
      encoder = new MpeH263p(codec.getCodecPayloadFormat(), mpRtpWriter);
   }
   else
      return OS_FAILED;

   assert(NULL != encoder);
   if (OS_SUCCESS != encoder->initEncode(mpVideoParams))
   {
      delete encoder;
      return OS_FAILED;
   }
   else
   {
      mpEncoder = encoder;
      return OS_SUCCESS;
   }
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
