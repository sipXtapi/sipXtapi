//  
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "mp/MpMediaTask.h"
#include "mp/MpRtpOutputAudioConnection.h"
#include "mp/MpFlowGraphBase.h"
#include "mp/MprEncode.h"
#include "mp/MprToNet.h"
#include "sdp/SdpCodec.h"
#include "os/OsLock.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpRtpOutputAudioConnection::MpRtpOutputAudioConnection(UtlString& resourceName,
                                                       MpConnectionID myID, 
                                                       MpFlowGraphBase* pParent,
                                                       int samplesPerFrame, 
                                                       int samplesPerSec)
: MpRtpOutputConnection(resourceName,
                        myID, 
#ifdef INCLUDE_RTCP // [
                        NULL // TODO: pParent->getRTCPSessionPtr()
#else // INCLUDE_RTCP ][
                        NULL
#endif // INCLUDE_RTCP ]
                       )
, mpFlowGraph(pParent)
, mpEncode(NULL)
{
   OsStatus     res;
   char         name[50];

   sprintf(name, "Encode-%d", myID);
   mpEncode    = new MprEncode(name, samplesPerFrame, samplesPerSec);

   // Add synchronized resources to flowgraph.
   res = pParent->addResource(*mpEncode);      assert(res == OS_SUCCESS);

   //////////////////////////////////////////////////////////////////////////
   // connect Encode -> ToNet (Non synchronous resources)
   mpEncode->setMyToNet(mpToNet);

   pParent->synchronize("new Connection, before enable(), %dx%X\n");
   enable();
   pParent->synchronize("new Connection, after enable(), %dx%X\n");
}

// Destructor
MpRtpOutputAudioConnection::~MpRtpOutputAudioConnection()
{
   if (mpEncode != NULL)
      delete mpEncode;
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean MpRtpOutputAudioConnection::processFrame(void)
{
    UtlBoolean result;

#ifdef RTL_ENABLED
    RTL_BLOCK(mName);
#endif

    assert(mpEncode);
    if(mpEncode)
    {
        // call doProcessFrame to do any "real" work
        result = doProcessFrame(mpInBufs, 
                                mpOutBufs,
                                mMaxInputs, 
                                mMaxOutputs, 
                                mIsEnabled,
                                mpEncode->getSamplesPerFrame(), 
                                mpEncode->getSamplesPerSec());
    }

   // release the input buffer
   mpInBufs[0].release();

   assert(mMaxOutputs == 0);

   return(result);
}

UtlBoolean MpRtpOutputAudioConnection::doProcessFrame(MpBufPtr inBufs[],
                                                     MpBufPtr outBufs[],
                                                     int inBufsSize,
                                                     int outBufsSize,
                                                     UtlBoolean isEnabled,
                                                     int samplesPerFrame,
                                                     int samplesPerSecond)
{
    UtlBoolean result = FALSE;
    assert(mpEncode);
    if(mpEncode)
    {
        result = mpEncode->doProcessFrame(inBufs,
                                          outBufs,
                                          inBufsSize,
                                          outBufsSize,
                                          isEnabled,
                                          samplesPerFrame,
                                          samplesPerSecond);
    }

    return(result);
}

// Disables the output path of the connection.
// Resources on the path(s) will also be disabled by these calls.
// If the flow graph is not "started", this call takes effect
// immediately.  Otherwise, the call takes effect at the start of the
// next frame processing interval.

UtlBoolean MpRtpOutputAudioConnection::handleDisable()
{
   mpEncode->disable();
   return MpResource::handleDisable();
}

// Enables the output path of the connection.
// Resources on the path(s) will also be enabled by these calls.
// Resources may allocate needed data (e.g. output path reframe buffer)
//  during this operation.
// If the flow graph is not "started", this call takes effect
// immediately.  Otherwise, the call takes effect at the start of the
// next frame processing interval.

UtlBoolean MpRtpOutputAudioConnection::handleEnable()
{
   mpEncode->enable();
   return(MpResource::handleEnable());
}

// Start sending RTP and RTCP packets.

void MpRtpOutputAudioConnection::startSendRtp(OsSocket& rRtpSocket,
                                     OsSocket& rRtcpSocket,
                                     SdpCodec* pPrimaryCodec,
                                     SdpCodec* pDtmfCodec)
{
   prepareStartSendRtp(rRtpSocket, rRtcpSocket);

   // This should be ok to set directly as long as we do not switch mid stream
   // Eventually this needs to be a message
#if 0
   osPrintf("MpConnection::startSendRtp setting send codecs:\n");

   if (NULL != pPrimaryCodec) {
      osPrintf("  Primary audio: codec=%d, payload type=%d\n",
          pPrimaryCodec->getCodecType(),
          pPrimaryCodec->getCodecPayloadFormat());
   } else {
      osPrintf("  Primary audio: NONE\n");
   }
   if (NULL != pDtmfCodec) {
      osPrintf("  DTMF Tones: codec=%d, payload type=%d\n",
          pDtmfCodec->getCodecType(), pDtmfCodec->getCodecPayloadFormat());
   } else {
      osPrintf("  DTMF Tones: NONE\n");
   }
   if (NULL != pSecondaryCodec) {
      osPrintf("  Secondary audio: codec=%d, payload type=%d\n",
          pSecondaryCodec->getCodecType(),
          pSecondaryCodec->getCodecPayloadFormat());
   } else {
      osPrintf("  Secondary audio: NONE\n");
   }
#endif

   mpEncode->selectCodecs(pPrimaryCodec, pDtmfCodec);
   mpFlowGraph->synchronize();
   mpEncode->enable();
}

// Stop sending RTP and RTCP packets.
void MpRtpOutputAudioConnection::stopSendRtp()
{
   prepareStopSendRtp();

//   osPrintf("MpRtpOutputAudioConnection::stopSendRtp resetting send codec\n");
   mpEncode->deselectCodecs();
   mpFlowGraph->synchronize();
   mpEncode->disable();
}

void MpRtpOutputAudioConnection::startTone(int toneId)
{
   mpEncode->startTone(toneId);
}

void MpRtpOutputAudioConnection::stopTone(void)
{
   mpEncode->stopTone();
}

/* ============================ ACCESSORS ================================= */

//Returns the resource to link to upstream resource's outPort.
MpResource* MpRtpOutputAudioConnection::getSinkResource() {
   return mpEncode;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
