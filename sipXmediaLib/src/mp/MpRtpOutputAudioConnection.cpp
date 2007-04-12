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
#include "mp/MpCallFlowGraph.h"
#include "mp/MprEncode.h"
#include "mp/MprToNet.h"
#include "mp/MprFromNet.h"
#include "mp/MprDejitter.h"
#include "mp/MprDecode.h"
#include "mp/JB/JB_API.h"
#include "sdp/SdpCodec.h"
#include "os/OsLock.h"
#include "os/OsTask.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpRtpOutputAudioConnection::MpRtpOutputAudioConnection(MpConnectionID myID, MpCallFlowGraph* pParent,
                                     int samplesPerFrame, int samplesPerSec)
: MpRtpOutputConnection(myID, 
#ifdef INCLUDE_RTCP // [
               pParent->getRTCPSessionPtr()
#else // INCLUDE_RTCP ][
               NULL
#endif // INCLUDE_RTCP ]
               )
, mpFlowGraph(pParent)
, mpEncode(NULL)
, mBridgePort(-1)
{
   OsStatus     res;
   char         name[50];
   int          i;

   sprintf(name, "Encode-%d", myID);
   mpEncode    = new MprEncode(name, samplesPerFrame, samplesPerSec);

 //memset((char*)mpPayloadMap, 0, (NUM_PAYLOAD_TYPES*sizeof(MpDecoderBase*)));
   for (i=0; i<NUM_PAYLOAD_TYPES; i++) {
      mpPayloadMap[i] = NULL;
   }

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

// Disables the output path of the connection.
// Resources on the path(s) will also be disabled by these calls.
// If the flow graph is not "started", this call takes effect
// immediately.  Otherwise, the call takes effect at the start of the
// next frame processing interval.
//!retcode: OS_SUCCESS - for now, these methods always return success


/*OsStatus MpRtpOutputAudioConnection::disable()
{
   mpEncode->disable();
   return MpRtpOutputConnection::disable();
}*/

// Enables the output path of the connection.
// Resources on the path(s) will also be enabled by these calls.
// Resources may allocate needed data (e.g. output path reframe buffer)
//  during this operation.
// If the flow graph is not "started", this call takes effect
// immediately.  Otherwise, the call takes effect at the start of the
// next frame processing interval.
//!retcode: OS_SUCCESS - for now, these methods always return success

OsStatus MpRtpOutputAudioConnection::enable()
{
   mpEncode->enable();
   return MpRtpOutputConnection::enable();
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

OsStatus MpRtpOutputAudioConnection::setBridgePort(int port)
{
   if (-1 != mBridgePort) return OS_BUSY;
   mBridgePort = port;
   return OS_SUCCESS;
}

void MpRtpOutputAudioConnection::startTone(int toneId)
{
   mpEncode->startTone(toneId);
}

void MpRtpOutputAudioConnection::stopTone(void)
{
   mpEncode->stopTone();
}

void MpRtpOutputAudioConnection::addPayloadType(int payloadType, MpDecoderBase* decoder)
{
   OsLock lock(mLock);

   // Check that payloadType is valid.
   if ((payloadType < 0) || (payloadType >= NUM_PAYLOAD_TYPES))
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
                    "MpRtpOutputAudioConnection::addPayloadType Attempting to add an invalid payload type %d", payloadType);
   }
   // Check to see if we already have a decoder for this payload type.
   else if (!(NULL == mpPayloadMap[payloadType]))
   {
      // This condition probably indicates that the sender of SDP specified
      // two decoders for the same payload type number.
      OsSysLog::add(FAC_MP, PRI_ERR,
                    "MpRtpOutputAudioConnection::addPayloadType Attempting to add a second decoder for payload type %d",
                    payloadType);
   }
   else
   {
      mpPayloadMap[payloadType] = decoder;
   }
}

void MpRtpOutputAudioConnection::deletePayloadType(int payloadType)
{
   OsLock lock(mLock);

   // Check that payloadType is valid.
   if ((payloadType < 0) || (payloadType >= NUM_PAYLOAD_TYPES))
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
                    "MpRtpOutputAudioConnection::deletePayloadType Attempting to delete an invalid payload type %d", payloadType);
   }
   // Check to see if this entry has already been deleted.
   else if (NULL == mpPayloadMap[payloadType])
   {
      // Either this payload type was doubly-added (and reported by
      // addPayloadType) or we've hit the race condtion in XMR-29.
      OsSysLog::add(FAC_MP, PRI_ERR,
                    "MpRtpOutputAudioConnection::deletePayloadType Attempting to delete again payload type %d",
                    payloadType);
      OsSysLog::add(FAC_MP, PRI_ERR,
                    "MpRtpOutputAudioConnection::deletePayloadType If there is no message from MpRtpOutputAudioConnection::addPayloadType above, see XMR-29");
   }
   else
   {
      mpPayloadMap[payloadType] = NULL;
   }
}

/* ============================ ACCESSORS ================================= */

//Returns the resource to link to upstream resource's outPort.
MpResource* MpRtpOutputAudioConnection::getSinkResource() {
   return mpEncode;
}

//Retrieves the port number that was assigned by the bridge.
int MpRtpOutputAudioConnection::getBridgePort() {
   return mBridgePort;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
