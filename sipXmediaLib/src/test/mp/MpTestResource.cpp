//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
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
#include "mp/MpBuf.h"
#include "mp/MpMisc.h"
#include "mp/MpTestResource.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
static const int RESOURCE_MSG_TYPE = MpFlowGraphMsg::RESOURCE_SPECIFIC_START;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

// Constructor
MpTestResource::MpTestResource(const UtlString& rName
                              , int minInputs, int maxInputs
                              , int minOutputs, int maxOutputs
                              , int samplesPerFrame, int samplesPerSec
                              )
:  MpAudioResource(rName, minInputs, maxInputs, minOutputs, maxOutputs,
                   samplesPerFrame, samplesPerSec),
   mGenOutBufMask(0),
   mProcessInBufMask(0),
   mProcessedCnt(0),
   mMsgCnt(0),
   mLastMsg(0)
{
   mLastDoProcessArgs.inBufs = new MpBufPtr[mMaxInputs];
   mLastDoProcessArgs.outBufs = new MpBufPtr[mMaxOutputs];
}

// Destructor
MpTestResource::~MpTestResource()
{
   if (mLastDoProcessArgs.inBufs != NULL)
      delete[] mLastDoProcessArgs.inBufs;

   if (mLastDoProcessArgs.outBufs != NULL)
      delete[] mLastDoProcessArgs.outBufs;
}

/* ============================ MANIPULATORS ============================== */

// Sends a test message to this resource.
void MpTestResource::sendTestMessage(void* ptr1, void* ptr2,
                                     int int3, int int4)
{
   MpFlowGraphMsg msg(RESOURCE_MSG_TYPE, this, ptr1, ptr2, int3, int4);
   OsStatus       res;

   res = postMessage(msg);
}

// Specify the genOutBufMask.
// For each bit in the genOutBufMask that is set, if there is a
// resource connected to the corresponding output port, doProcessFrame()
// will create an output buffer on that output port.
void MpTestResource::setGenOutBufMask(int mask)
{
   mGenOutBufMask = mask;
}

// Specify the processInBufMask.
// For each bit in the processInBufMask that is set, doProcessFrame()
// will pass the input buffer from the corresponding input port,
// straight through to the corresponding output port.  If nothing is
// connected on the corresponding output port, the input buffer will
// be deleted.
void MpTestResource::setProcessInBufMask(int mask)
{
   mProcessInBufMask = mask;
}

/* ============================ ACCESSORS ================================= */

// Returns the count of the number of frames processed by this resource.
int MpTestResource::numFramesProcessed(void)
{
   return mProcessedCnt;
}

// Returns the count of the number of messages successfully processed by 
// this resource.
int MpTestResource::numMsgsProcessed(void)
{
   return mMsgCnt;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Processes the next frame interval's worth of media.
UtlBoolean MpTestResource::doProcessFrame(MpBufPtr inBufs[],
                                          MpBufPtr outBufs[],
                                          int inBufsSize,
                                          int outBufsSize,
                                          UtlBoolean isEnabled,
                                          int samplesPerFrame,
                                          int samplesPerSecond)
{
   // keep a copy of the input buffers
   for (int i=0; i<mMaxInputs; i++) {
      mLastDoProcessArgs.inBufs[i]     = inBufs[i];
   }
   // keep a copy of the arguments passed to this method
   mLastDoProcessArgs.inBufsSize       = inBufsSize;
   mLastDoProcessArgs.outBufsSize      = outBufsSize;
   mLastDoProcessArgs.isEnabled        = isEnabled;
   mLastDoProcessArgs.samplesPerFrame  = samplesPerFrame;
   mLastDoProcessArgs.samplesPerSecond = samplesPerSecond;

   for (int i=0; i < outBufsSize; i++)
   {
      outBufs[i].release();
      if (isOutputConnected(i))
      {
         if ((mProcessInBufMask & (1 << i)) &&
             (inBufsSize > i))
         {
            // if the corresponding bit in the mProcessInBufMask is set for
            // the input port then pass the input buffer straight thru
            outBufs[i] = inBufs[i];
            inBufs[i].release();
         }

         if ( isEnabled &&
             (mGenOutBufMask & (1 << i)) &&
             (!outBufs[i].isValid()))
         {
            // if the output buffer is presently NULL and the corresponding
            // bit in the mGenOutBufMask is set then allocate a new buffer
            // for the output port
            assert(MpMisc.RawAudioPool != NULL);
            MpAudioBufPtr pBuf = MpMisc.RawAudioPool->getBuffer();
            memset(pBuf->getSamples(), 0,
                   pBuf->getSamplesNumber()*sizeof(MpAudioSample));
            outBufs[i] = pBuf;
         }
      }
   }

   for (int i=0; i < inBufsSize; i++)
   {
      // if the corresponding bit in the mProcessInBufMask is set and we
      // haven't processed the input buffer then free it.
      if ((mProcessInBufMask & (1 << i)) &&
          (inBufs[i].isValid()))
      {
         inBufs[i].release();
      }
   }

   mProcessedCnt++;

   // keep a copy of the generated buffers
   for (int i=0; i<mMaxOutputs; i++) {
      mLastDoProcessArgs.outBufs[i]    = outBufs[i];
   }

   return TRUE;
}

// Handles messages for this resource.
UtlBoolean MpTestResource::handleMessage(MpFlowGraphMsg& rMsg)
{
   mLastMsg = rMsg;
   mMsgCnt++;

   if (rMsg.getMsg() == RESOURCE_MSG_TYPE)
      return TRUE;
   else
      return MpAudioResource::handleMessage(rMsg);
}

/* ============================ FUNCTIONS ================================= */

