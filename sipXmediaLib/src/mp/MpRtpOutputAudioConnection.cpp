// Copyright 2008 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA. 
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
#include <mp/MpRtpOutputAudioConnection.h>
#include <mp/MpFlowGraphBase.h>
#include <mp/MprEncode.h>
#include <mp/MprToNet.h>
#include <mp/MprRtpStartSendMsg.h>
#include <mp/MpResourceMsg.h>
#include <sdp/SdpCodec.h>
#include <os/OsLock.h>
#ifdef RTL_ENABLED
#   include <rtl_macro.h>
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpRtpOutputAudioConnection::MpRtpOutputAudioConnection(const UtlString& resourceName,
                                                       MpConnectionID myID)
: MpRtpOutputConnection(resourceName,
                        myID,
#ifdef INCLUDE_RTCP // [
                        NULL // TODO: pParent->getRTCPSessionPtr()
#else // INCLUDE_RTCP ][
                        NULL
#endif // INCLUDE_RTCP ]
                       )
, mpEncode(NULL)
{
   char         name[50];

   sprintf(name, "Encode-%d", myID);
   mpEncode    = new MprEncode(name);

   // encoder does not get added to the flowgraph, this connection
   // gets added to do the encoding frame processing.

   //////////////////////////////////////////////////////////////////////////
   // connect Encode -> ToNet (Non synchronous resources)
   mpEncode->setMyToNet(mpToNet);

   //  This got moved to the call flowgraph when the connection is
   // added to the flowgraph.  Not sure it is still needed there either
   //pParent->synchronize("new Connection, before enable(), %dx%X\n");
   //enable();
   //pParent->synchronize("new Connection, after enable(), %dx%X\n");
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
    UtlString str_fg(getFlowGraph()->getFlowgraphName());
    str_fg.append("_");
    str_fg.append(*this);
    RTL_BLOCK(str_fg);    
#endif

    assert(mpEncode);
    if(mpEncode)
    {
        // call doProcessFrame to do any "real" work
        result = mpEncode->doProcessFrame(mpInBufs,
                                          mpOutBufs,
                                          mMaxInputs,
                                          mMaxOutputs,
                                          mpEncode->mIsEnabled,
                                          mpFlowGraph->getSamplesPerFrame(),
                                          mpFlowGraph->getSamplesPerSec());
    }

   // release the input buffer, we are done with it
   mpInBufs[0].release();
   assert(mMaxInputs == 1);

   // no outputs to release
   assert(mMaxOutputs == 0);

   return(result);
}

// Queues a message to start sending RTP and RTCP packets.
OsStatus MpRtpOutputAudioConnection::startSendRtp(OsMsgQ& messageQueue,
                                                  const UtlString& resourceName,
                                                  IMediaTransportAdapter* pAdapter,
                                                  SdpCodec* audioCodec,
                                                  SdpCodec* dtmfCodec)
{
    OsStatus result = OS_INVALID_ARGUMENT;
    if(audioCodec || dtmfCodec)
    {
        // Create a message to contain the startRecieveRtp data
        MprRtpStartSendMsg msg(resourceName,
                               audioCodec,
                               dtmfCodec,
                               pAdapter);

        // Send the message in the queue.
        result = messageQueue.send(msg);
    }
    return(result);
}

// Queues a message to stop sending RTP and RTCP packets.
OsStatus MpRtpOutputAudioConnection::stopSendRtp(OsMsgQ& messageQueue,
                                                 const UtlString& resourceName)
{
    MpResourceMsg stopSendMsg(MpResourceMsg::MPRM_STOP_SEND_RTP,
                              resourceName);

    // Send the message in the queue.
    OsStatus result = messageQueue.send(stopSendMsg);
    return(result);
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

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

UtlBoolean MpRtpOutputAudioConnection::handleMessage(MpResourceMsg& rMsg)
{
   UtlBoolean result = FALSE;
   unsigned char messageSubtype = rMsg.getMsgSubType();
   switch(messageSubtype)
   {
   case MpResourceMsg::MPRM_START_SEND_RTP:
      {
         MprRtpStartSendMsg* startMessage = (MprRtpStartSendMsg*) &rMsg;
         SdpCodec* audioCodec = NULL;
         SdpCodec* dtmfCodec = NULL;
         startMessage->getCodecs(audioCodec, dtmfCodec);
         IMediaTransportAdapter* pAdapter = startMessage->getSocketAdapter();

         assert(pAdapter);
         handleStartSendRtp(pAdapter,
            audioCodec,
            dtmfCodec);
         result = TRUE;
      }
      break;

   case MpResourceMsg::MPRM_STOP_SEND_RTP:
      handleStopSendRtp();
      result = TRUE;
      break;

   default:
      result = MpResource::handleMessage(rMsg);
      break;
   }
   return(result);
}

// Enables the output path of the connection.
UtlBoolean MpRtpOutputAudioConnection::handleEnable()
{
   mpEncode->enable();
   return(MpResource::handleEnable());
}

// Disables the output path of the connection.
UtlBoolean MpRtpOutputAudioConnection::handleDisable()
{
   mpEncode->disable();
   return MpResource::handleDisable();
}

OsStatus MpRtpOutputAudioConnection::handleStartSendRtp(IMediaTransportAdapter* pAdapter,
                                                        SdpCodec* pPrimaryCodec,
                                                        SdpCodec* pDtmfCodec)
{
   prepareStartSendRtp(pAdapter);

   // This should be ok to set directly as long as we do not switch mid stream
   // Eventually this needs to be a message

   OsStatus result = OS_NOT_FOUND;
   if(mpEncode)
   {
      result = OS_SUCCESS;
      // Tell encoder which codecs to use (data codec and signaling codec).
      mpEncode->selectCodecs(pPrimaryCodec, pDtmfCodec);
      mpEncode->enable();
   }
   return(result);
}

// Stop sending RTP and RTCP packets.
OsStatus MpRtpOutputAudioConnection::handleStopSendRtp()
{
   OsStatus result = OS_NOT_FOUND;
   prepareStopSendRtp();

   if(mpEncode)
   {
      result = OS_SUCCESS;
      //   osPrintf("MpRtpOutputAudioConnection::stopSendRtp resetting send codec\n");
      mpEncode->handleDeselectCodecs();
      // No need to synchronize as the encoder is not part of the
      // flowgraph.
      //mpFlowGraph->synchronize();
      mpEncode->disable();
   }
   return(result);
}

OsStatus MpRtpOutputAudioConnection::setFlowGraph(MpFlowGraphBase* pFlowGraph)
{
   OsStatus stat = MpResource::setFlowGraph(pFlowGraph);

   // If the parent's call was successful, then call
   // setFlowGraph on any child resources we have.
   if(stat == OS_SUCCESS)
   {
      stat = mpEncode->setFlowGraph(pFlowGraph);
   }
   return stat;
}

OsStatus MpRtpOutputAudioConnection::setNotificationsEnabled(UtlBoolean enable)
{
   OsStatus stat = 
      MpResource::setNotificationsEnabled(enable);

   // If the parent's call was successful, then call
   // setAllNotificationsEnabled on any child resources we have.
   if(stat == OS_SUCCESS)
   {
      stat = mpEncode->setNotificationsEnabled(enable);
   }
   return stat;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
