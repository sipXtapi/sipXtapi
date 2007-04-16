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
#include "mp/MpRtpInputAudioConnection.h"
#include "mp/MpFlowGraphBase.h"
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
MpRtpInputAudioConnection::MpRtpInputAudioConnection(UtlString& resourceName,
                                                     MpConnectionID myID, 
                                                     MpFlowGraphBase* pParent,
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
, mpFlowGraph(pParent)
, mpDecode(NULL)
, mpJB_inst(NULL)
{
   OsStatus     res;
   char         name[50];
   int          i;

   sprintf(name, "Decode-%d", myID);
   mpDecode    = new MprDecode(name, this, samplesPerFrame, samplesPerSec);

 //memset((char*)mpPayloadMap, 0, (NUM_PAYLOAD_TYPES*sizeof(MpDecoderBase*)));
   for (i=0; i<NUM_PAYLOAD_TYPES; i++) {
      mpPayloadMap[i] = NULL;
   }

   // Add synchronized resources to flowgraph.
   res = pParent->addResource(*mpDecode);      assert(res == OS_SUCCESS);

   //////////////////////////////////////////////////////////////////////////
   // connect Dejitter -> Decode (Non synchronous resources)
   mpDecode->setMyDejitter(mpDejitter);

   pParent->synchronize("new Connection, before enable(), %dx%X\n");
   enable();
   pParent->synchronize("new Connection, after enable(), %dx%X\n");
}

// Destructor
MpRtpInputAudioConnection::~MpRtpInputAudioConnection()
{
   if (mpDecode != NULL)
      delete mpDecode;

   if (NULL != mpJB_inst) {
      JB_free(mpJB_inst);
      mpJB_inst = NULL;
   }
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean MpRtpInputAudioConnection::processFrame(void)
{
    UtlBoolean result;

#ifdef RTL_ENABLED
    RTL_BLOCK(mName);
#endif

    assert(mpDecode);
    // call doProcessFrame to do any "real" work
    result = mpDecode->doProcessFrame(mpInBufs, 
                                      mpOutBufs,
                                      mMaxInputs, 
                                      mMaxOutputs, 
                                      mIsEnabled,
                                      mpDecode->getSamplesPerFrame(), 
                                      mpDecode->getSamplesPerSec());


   // release the output buffer
   mpOutBufs[0].release();

   assert(mMaxInputs == 0);

   return(result);
}

UtlBoolean MpRtpInputAudioConnection::doProcessFrame(MpBufPtr inBufs[],
                                                     MpBufPtr outBufs[],
                                                     int inBufsSize,
                                                     int outBufsSize,
                                                     UtlBoolean isEnabled,
                                                     int samplesPerFrame,
                                                     int samplesPerSecond)
{
    UtlBoolean result = FALSE;
    assert(mpDecode);
    if(mpDecode)
    {
        result = mpDecode->doProcessFrame(inBufs,
                                          outBufs,
                                          inBufsSize,
                                          outBufsSize,
                                          isEnabled,
                                          samplesPerFrame,
                                          samplesPerSecond);
    }

    return(result);
}


// Disables the input path of the connection.
// Resources on the path(s) will also be disabled by these calls.
// If the flow graph is not "started", this call takes effect
// immediately.  Otherwise, the call takes effect at the start of the
// next frame processing interval.
//!retcode: OS_SUCCESS - for now, these methods always return success

UtlBoolean MpRtpInputAudioConnection::handleDisable()
{
   mpDecode->disable();
   return(MpResource::handleDisable());
}



// Enables the input path of the connection.
// Resources on the path(s) will also be enabled by these calls.
// Resources may allocate needed data (e.g. output path reframe buffer)
//  during this operation.
// If the flow graph is not "started", this call takes effect
// immediately.  Otherwise, the call takes effect at the start of the
// next frame processing interval.
//!retcode: OS_SUCCESS - for now, these methods always return success

UtlBoolean MpRtpInputAudioConnection::handleEnable()
{
   mpDecode->enable();
   return(MpResource::handleEnable());
}

// Start receiving RTP and RTCP packets.

void MpRtpInputAudioConnection::startReceiveRtp(SdpCodec* pCodecs[], int numCodecs,
                                       OsSocket& rRtpSocket,
                                       OsSocket& rRtcpSocket)
{
   if (numCodecs)
   {
       mpDecode->selectCodecs(pCodecs, numCodecs);       
   }
   mpFlowGraph->synchronize();
   prepareStartReceiveRtp(rRtpSocket, rRtcpSocket);
   mpFlowGraph->synchronize();
   if (numCodecs)
   {
       mpDecode->enable();
   }
}

// Stop receiving RTP and RTCP packets.
void MpRtpInputAudioConnection::stopReceiveRtp()
{
   prepareStopReceiveRtp();

   JB_inst* pJB_inst; 

   mpFlowGraph->synchronize();

   mpDecode->deselectCodec();
   mpFlowGraph->synchronize();

   pJB_inst = getJBinst(TRUE);  // get NULL if not allocated
   mpJB_inst = NULL;
   mpFlowGraph->synchronize();

   if (NULL != pJB_inst) {
      JB_free(pJB_inst);
   }
   mpDecode->disable();
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

void MpRtpInputAudioConnection::setPremiumSound(PremiumSoundOptions op)
{
#ifdef HAVE_GIPS /* [ */
   int NetEqOp = NETEQ_PLAYOUT_MODE_OFF;

   // this must only be called in the context of the Media Task
   assert(OsTask::getCurrentTask() == MpMediaTask::getMediaTask(0));

   if (EnablePremiumSound == op) {
      NetEqOp = NETEQ_PLAYOUT_MODE_ON;
   }
   if (NULL != mpJB_inst) {
#ifndef __pingtel_on_posix__
      NETEQ_GIPS_10MS16B_SetPlayoutMode(mpJB_inst, NetEqOp);
#endif
/*
      osPrintf("MpRtpInputAudioConnection::setPremiumSound: %sabling Premium Sound on #%d\n",
         (EnablePremiumSound == op) ? "En" : "Dis", mMyID);
*/
   }
#endif /* HAVE_GIPS ] */
}

/* ============================ ACCESSORS ================================= */

//:Returns a pointer to the Jitter Buffer instance, creating it if necessary
// If the instance has not been created, but the argument "optional" is
// TRUE, then do not create it, just return NULL.

JB_inst* MpRtpInputAudioConnection::getJBinst(UtlBoolean optional) {

   if ((NULL == mpJB_inst) && (!optional)) {
      int res;
      res = JB_create(&mpJB_inst);
/*
      osPrintf("MpRtpInputAudioConnection::getJBinst: JB_create=>0x%X\n",
         (int) mpJB_inst);
*/

      assert(NULL != mpJB_inst);

      //Here it is hard coded to use 8000 Hz sampling frequency
      //This number is only relevant until any packet has arrived
      //When packet arrives the codec determines the output samp.freq.

      res |= JB_init(mpJB_inst, 8000);

      if (0 != res) { //just in case
         osPrintf("MpRtpInputAudioConnection::getJBinst: Jitter Buffer init failure!\n");
         if (NULL != mpJB_inst) {
            JB_free(mpJB_inst);
            mpJB_inst = NULL;
         }
      }
      if (NULL != mpJB_inst) {
/*
         UtlBoolean on = mpFlowGraph->isPremiumSoundEnabled();
         osPrintf("MpRtpInputAudioConnection::getJBinst: %sabling Premium Sound on #%d\n",
            on ? "En" : "Dis", mMyID);
         setPremiumSound(on ? EnablePremiumSound : DisablePremiumSound);
*/
      }
   }
   return(mpJB_inst);
}

//Returns the resource to link to downstream resource's inPort.
MpResource* MpRtpInputAudioConnection::getSourceResource() {
   return mpDecode;
}

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

UtlBoolean MpRtpInputAudioConnection::handleSetDtmfNotify(OsNotification* pNotify)
{
   return mpDecode->handleSetDtmfNotify(pNotify);
}

UtlBoolean MpRtpInputAudioConnection::setDtmfTerm(MprRecorder *pRecorders)
{
   return mpDecode->setDtmfTerm(pRecorders);
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
