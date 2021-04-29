//  
// Copyright (C) 2008-2014 SIPfoundry Inc. All rights reserved.
//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

// SYSTEM INCLUDES
#include <os/OsIntTypes.h>
#include <assert.h>
//#include <os/OsSysLog.h>

// APPLICATION INCLUDES
#include <mp/MprVad.h>
#include <utl/UtlSerialized.h>
#include <mp/MpVadBase.h>
#include <mp/MpStringResourceMsg.h>
//#include <mp/MpResNotificationMsg.h>
#include <mp/MpFlowGraphBase.h>
#include <mp/MpPackedResourceMsg.h>


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// TYPEDEFS
// DEFINES
// MACROS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////////// PUBLIC //////////////////////////////// */

MprVad::MprVad(const UtlString& rName, const UtlString &vadAlgorithm)
: MpAudioResource(rName, 1, 1, 1, 1)
, mpVad(NULL)
{
   mpVad = MpVadBase::createVad(vadAlgorithm);
   mpVad->setName(*this);
   assert(mpVad != NULL);
}

MprVad::~MprVad()
{
   delete mpVad;
}

/* =============================== CREATORS =============================== */

/* ============================= MANIPULATORS ============================= */

OsStatus MprVad::setVadParameter(const UtlString& parameterName,
                                 int parameterValue,
                                 const UtlString& resourceName,
                                 OsMsgQ& flowgraphQueue)
{
    MpPackedResourceMsg message((MpResourceMsg::MpResourceMsgType)MPRM_SET_VAD_PARAM,
                                resourceName);

    UtlSerialized& messagePayload = message.getData();
    OsStatus status = messagePayload.serialize(parameterName);
    assert(status == OS_SUCCESS);
    status = messagePayload.serialize(parameterValue);
    assert(status == OS_SUCCESS);
    messagePayload.finishSerialize();

    status = flowgraphQueue.send(message, sOperationQueueTimeout);

    return(status);
}


OsStatus MprVad::changeVadAlgorithm(const UtlString& namedResource, 
                                    OsMsgQ& fgQ,
                                    const UtlString &vadAlgorithm)
{
   MpStringResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_CHANGE_VAD,
                           namedResource,
                           vadAlgorithm);
   return fgQ.send(msg);
}

/* ============================== ACCESSORS =============================== */

/* =============================== INQUIRY ================================ */

/* ////////////////////////////// PROTECTED /////////////////////////////// */

UtlBoolean MprVad::doProcessFrame(MpBufPtr inBufs[],
                                  MpBufPtr outBufs[],
                                  int inBufsSize,
                                  int outBufsSize,
                                  UtlBoolean isEnabled,
                                  int samplesPerFrame,
                                  int samplesPerSecond)
{
   // We're disabled or have nothing to process.
   if ( outBufsSize == 0 || inBufsSize == 0 )
   {
      return TRUE;
   }
   if (!isEnabled)
   {
      outBufs[0] = inBufs[0];
      return TRUE;
   }

   MpAudioBufPtr pBuf = inBufs[0];

   if (pBuf.isValid())
   {
      if (pBuf->getSpeechType() == MP_SPEECH_UNKNOWN)
      {
         MpSpeechType vadValue;
         vadValue = mpVad->processFrame(mpFlowGraph->numFramesProcessed(),
                                        pBuf->getSamplesPtr(),
                                        pBuf->getSamplesNumber(),
                                        pBuf->getSpeechParams());
//       printf("VAD: %d\n", vadValue);
         pBuf->setSpeechType(vadValue);
      }
      else
      {
         mpVad->processFrame(mpFlowGraph->numFramesProcessed(),
                             pBuf->getSamplesPtr(),
                             pBuf->getSamplesNumber(),
                             pBuf->getSpeechParams(),
                             TRUE);
      }
      pBuf->setEnergy(mpVad->getEnergy());
   }

   outBufs[0] = pBuf;

   return TRUE;
}

UtlBoolean MprVad::handleMessage(MpResourceMsg& rMsg)
{
   UtlBoolean messageHandled = FALSE;

   switch (rMsg.getMsg())
   {
   case MPRM_CHANGE_VAD:
      {
         MpStringResourceMsg *pMsg = (MpStringResourceMsg*)&rMsg;
         messageHandled = handleChageVadAlgorithm(pMsg->getData());
      }
      break;

   case MPRM_SET_VAD_PARAM:
      {
          MpPackedResourceMsg packedMessage = (MpPackedResourceMsg&) rMsg;
          UtlSerialized& serialData = packedMessage.getData();
          messageHandled = handleSetVadParam(serialData);
      }
      break;


   default:
      messageHandled = MpResource::handleMessage(rMsg);
      break;
   }
   return(messageHandled);
}

UtlBoolean MprVad::handleChageVadAlgorithm(const UtlString &vadAlgorithm)
{
   // Free old PLC
   delete mpVad;

   // Set PLC to a new one
   mpVad = MpVadBase::createVad(vadAlgorithm);
   mpVad->setName(*this);
   if (mpFlowGraph != NULL)
   {
      mpVad->init(mpFlowGraph->getSamplesPerSec());
   }
   return TRUE;
}

UtlBoolean MprVad::handleSetVadParam(UtlSerialized& serialData)
{
    assert(mpVad);
    if(mpVad)
    {
        UtlString name;
        int value;
        serialData.deserialize(name);
        serialData.deserialize(value);
        mpVad->setParam(name, &value);
    }
    return(TRUE);
}

OsStatus MprVad::setFlowGraph(MpFlowGraphBase* pFlowGraph)
{
   OsStatus res =  MpAudioResource::setFlowGraph(pFlowGraph);

   if (res == OS_SUCCESS)
   {
      // Check whether we've been added to flowgraph or removed.
      if (pFlowGraph != NULL)
      {
         mpVad->init(mpFlowGraph->getSamplesPerSec());
      }
      else
      {
         mpVad->reset();
      }
   }

   return res;
}

/* /////////////////////////////// PRIVATE //////////////////////////////// */

/* ============================== FUNCTIONS =============================== */


