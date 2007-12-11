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
#include <mp/MpMediaTask.h>
#include <mp/MpRtpInputAudioConnection.h>
#include <mp/MpFlowGraphBase.h>
#include <mp/MprFromNet.h>
#include <mp/MprDejitter.h>
#include <mp/MpJitterBuffer.h>
#include <mp/MprDecode.h>
#include <mp/MpResourceMsg.h>
#include <mp/MprRtpStartReceiveMsg.h>
#include <sdp/SdpCodec.h>
#include <os/OsLock.h>
#ifdef RTL_ENABLED
#  include <rtl_macro.h>
#  ifdef RTL_AUDIO_ENABLED
#     include <SeScopeAudioBuffer.h>
#  endif
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpRtpInputAudioConnection::MpRtpInputAudioConnection(const UtlString& resourceName,
                                                     MpConnectionID myID, 
                                                     int samplesPerFrame, 
                                                     int samplesPerSec)
: MpRtpInputConnection(resourceName,
                       myID, 
#ifdef INCLUDE_RTCP // [
                       NULL // TODO: pParent->getRTCPSessionPtr()
#else // INCLUDE_RTCP ][
                       NULL
#endif // INCLUDE_RTCP ]
                       )
, mpDecode(NULL)
{
   char         name[50];
   int          i;

   sprintf(name, "Decode-%d", myID);
   mpDecode    = new MprDecode(name, this, samplesPerFrame, samplesPerSec);

 //memset((char*)mpPayloadMap, 0, (NUM_PAYLOAD_TYPES*sizeof(MpDecoderBase*)));
   for (i=0; i<NUM_PAYLOAD_TYPES; i++) {
      mpPayloadMap[i] = NULL;
   }

   // decoder does not get added to the flowgraph, this connection
   // gets added to do the decoding frameprocessing.

   //////////////////////////////////////////////////////////////////////////
   // connect Dejitter -> Decode (Non synchronous resources)
   mpDecode->setMyDejitter(mpDejitter);

   //  This got moved to the call flowgraph when the connection is
   // added to the flowgraph.  Not sure it is still needed there either
   //pParent->synchronize("new Connection, before enable(), %dx%X\n");
   //enable();
   //pParent->synchronize("new Connection, after enable(), %dx%X\n");
}

// Destructor
MpRtpInputAudioConnection::~MpRtpInputAudioConnection()
{
   delete mpDecode;
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean MpRtpInputAudioConnection::processFrame(void)
{
    UtlBoolean result;

#ifdef RTL_ENABLED
    UtlString str_fg(getFlowGraph()->getFlowgraphName());
    str_fg.append("_");
    str_fg.append(*this);
    RTL_BLOCK(str_fg);						
#endif

    assert(mpDecode);
    if(mpDecode)
    {
        // call doProcessFrame to do any "real" work
        result = mpDecode->doProcessFrame(mpInBufs, 
                                          mpOutBufs,
                                          mMaxInputs, 
                                          mMaxOutputs, 
                                          mpDecode->mIsEnabled,
                                          mpDecode->getSamplesPerFrame(), 
                                          mpDecode->getSamplesPerSec());
    }



    // No input buffers to release
   assert(mMaxInputs == 0);

#ifdef RTL_AUDIO_ENABLED
   int frameIndex = mpFlowGraph ? mpFlowGraph->numFramesProcessed() : 0;
   // If there is a consumer of the output
   if(mpOutConns[0].pResource)
   {
      UtlString outputLabel(*this);
      outputLabel.append("_output_0_");
      outputLabel.append(*mpOutConns[0].pResource);
      RTL_AUDIO_BUFFER(outputLabel, 
                       8000, 
                       ((MpAudioBufPtr) mpOutBufs[0]), 
                       frameIndex);
   }
#endif

   // Push the output buffer to the next resource
   assert(mMaxOutputs == 1);
   pushBufferDownsream(0, mpOutBufs[0]);
   // release the output buffer
   mpOutBufs[0].release();


   return(result);
}

// Start receiving RTP and RTCP packets.
OsStatus MpRtpInputAudioConnection::startReceiveRtp(OsMsgQ& messageQueue,
                                                    const UtlString& resourceName,
                                                    SdpCodec* codecArray[], 
                                                    int numCodecs,
                                                    OsSocket& rRtpSocket,
                                                    OsSocket& rRtcpSocket)
{
    OsStatus result = OS_INVALID_ARGUMENT;
    if(numCodecs > 0 && codecArray)
    {
        // Create a message to contain the startRecieveRtp data
        MprRtpStartReceiveMsg msg(resourceName,
                                  codecArray,
                                  numCodecs,
                                  rRtpSocket,
                                  rRtcpSocket);

        // Send the message in the queue.
        result = messageQueue.send(msg);
    }
    return(result);
}

OsStatus MpRtpInputAudioConnection::stopReceiveRtp(OsMsgQ& messageQueue,
                                                   const UtlString& resourceName)
{
    MpResourceMsg stopReceiveMsg(MpResourceMsg::MPRM_STOP_RECEIVE_RTP, 
                                 resourceName);

    // Send the message in the queue.
    OsStatus result = messageQueue.send(stopReceiveMsg);
    return(result);
}

void MpRtpInputAudioConnection::addPayloadType(int payloadType, MpDecoderBase* decoder)
{
   OsLock lock(mLock);

   // Check that payloadType is valid.
   if ((payloadType < 0) || (payloadType >= NUM_PAYLOAD_TYPES))
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
                    "MpRtpInputAudioConnection::addPayloadType Attempting to add an invalid payload type %d", payloadType);
   }
   // Check to see if we already have a decoder for this payload type.
   else if (!(NULL == mpPayloadMap[payloadType]))
   {
      // This condition probably indicates that the sender of SDP specified
      // two decoders for the same payload type number.
      OsSysLog::add(FAC_MP, PRI_ERR,
                    "MpRtpInputAudioConnection::addPayloadType Attempting to add a second decoder for payload type %d",
                    payloadType);
   }
   else
   {
      mpPayloadMap[payloadType] = decoder;
   }
}

void MpRtpInputAudioConnection::deletePayloadType(int payloadType)
{
   OsLock lock(mLock);

   // Check that payloadType is valid.
   if ((payloadType < 0) || (payloadType >= NUM_PAYLOAD_TYPES))
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
                    "MpRtpInputAudioConnection::deletePayloadType Attempting to delete an invalid payload type %d", payloadType);
   }
   // Check to see if this entry has already been deleted.
   else if (NULL == mpPayloadMap[payloadType])
   {
      // Either this payload type was doubly-added (and reported by
      // addPayloadType) or we've hit the race condtion in XMR-29.
      OsSysLog::add(FAC_MP, PRI_ERR,
                    "MpRtpInputAudioConnection::deletePayloadType Attempting to delete again payload type %d",
                    payloadType);
      OsSysLog::add(FAC_MP, PRI_ERR,
                    "MpRtpInputAudioConnection::deletePayloadType If there is no message from MpRtpInputAudioConnection::addPayloadType above, see XMR-29");
   }
   else
   {
      mpPayloadMap[payloadType] = NULL;
   }
}

UtlBoolean MpRtpInputAudioConnection::handleSetDtmfNotify(OsNotification* pNotify)
{
   return mpDecode->handleSetDtmfNotify(pNotify);
}

/* ============================ ACCESSORS ================================= */

MpDecoderBase* MpRtpInputAudioConnection::mapPayloadType(int payloadType)
{
   OsLock lock(mLock);

   if ((payloadType < 0) || (payloadType >= NUM_PAYLOAD_TYPES))
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
                    "MpRtpInputAudioConnection::mapPayloadType Attempting to map an invalid payload type %d", payloadType);
      return NULL;
   }
   else
   {
      return mpPayloadMap[payloadType];
   }
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

UtlBoolean MpRtpInputAudioConnection::handleMessage(MpResourceMsg& rMsg)
{
   UtlBoolean result = FALSE;
   unsigned char messageSubtype = rMsg.getMsgSubType();
   switch(messageSubtype)
   {
   case MpResourceMsg::MPRM_START_RECEIVE_RTP:
      {
         MprRtpStartReceiveMsg* startMessage = (MprRtpStartReceiveMsg*) &rMsg;
         SdpCodec** codecArray = NULL;
         int numCodecs;
         startMessage->getCodecArray(numCodecs, codecArray);
         OsSocket* rtpSocket = startMessage->getRtpSocket();
         OsSocket* rtcpSocket = startMessage->getRtcpSocket();

         handleStartReceiveRtp(codecArray, numCodecs, *rtpSocket, *rtcpSocket);
         result = TRUE;
      }
      break;

   case MpResourceMsg::MPRM_STOP_RECEIVE_RTP:
      handleStopReceiveRtp();
      result = TRUE;
      break;

   default:
      result = MpResource::handleMessage(rMsg);
      break;
   }
   return(result);
}

// Enables the input path of the connection.
UtlBoolean MpRtpInputAudioConnection::handleEnable()
{
   mpDecode->enable();
   return(MpResource::handleEnable());
}

// Disables the input path of the connection.
UtlBoolean MpRtpInputAudioConnection::handleDisable()
{
   mpDecode->disable();
   return(MpResource::handleDisable());
}

void MpRtpInputAudioConnection::handleStartReceiveRtp(SdpCodec* pCodecs[], 
                                                      int numCodecs,
                                                      OsSocket& rRtpSocket,
                                                      OsSocket& rRtcpSocket)
{
   if (numCodecs)
   {
      mpDecode->selectCodecs(pCodecs, numCodecs);       
   }
   // No need to synchronize as the decoder is not part of the
   // flowgraph.  It is part of this connection/resource
   //mpFlowGraph->synchronize();
   prepareStartReceiveRtp(rRtpSocket, rRtcpSocket);
   // No need to synchronize as the decoder is not part of the
   // flowgraph.  It is part of this connection/resource
   //mpFlowGraph->synchronize();
   if (numCodecs)
   {
      mpDecode->enable();
   }
}

// Stop receiving RTP and RTCP packets.
void MpRtpInputAudioConnection::handleStopReceiveRtp()
{
   prepareStopReceiveRtp();

   // No need to synchronize as the decoder is not part of the
   // flowgraph.  It is part of this connection/resource
   //mpFlowGraph->synchronize();


   mpDecode->deselectCodec();
   // No need to synchronize as the decoder is not part of the
   // flowgraph.  It is part of this connection/resource
   //mpFlowGraph->synchronize();

   mpDecode->disable();
}

OsStatus MpRtpInputAudioConnection::setFlowGraph(MpFlowGraphBase* pFlowGraph)
{
   OsStatus stat = MpResource::setFlowGraph(pFlowGraph);

   // If the parent's call was successful, then call
   // setFlowGraph on any child resources we have.
   if(stat == OS_SUCCESS)
   {
      stat = mpDecode->setFlowGraph(pFlowGraph);
   }
   return stat;
}

OsStatus MpRtpInputAudioConnection::setNotificationsEnabled(UtlBoolean enable)
{
   OsStatus stat = 
      MpResource::setNotificationsEnabled(enable);

   // If the parent's call was successful, then call
   // setAllNotificationsEnabled on any child resources we have.
   if(stat == OS_SUCCESS)
   {
      stat = mpDecode->setNotificationsEnabled(enable);
   }
   return stat;
}
/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
