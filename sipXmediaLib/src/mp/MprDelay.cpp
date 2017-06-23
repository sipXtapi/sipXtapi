//  
// Copyright (C) 2008-2017 SIPez LLC. All rights reserved.
//  
// Copyright (C) 2008 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

// SYSTEM INCLUDES
#include <os/OsIntTypes.h>
#include <assert.h>
#include <os/OsSysLog.h>

// APPLICATION INCLUDES
#include "mp/MprDelay.h"
#include "mp/MpResNotificationMsg.h"
#include "mp/MpFlowGraphBase.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// TYPEDEFS
// DEFINES
// MACROS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////////// PUBLIC //////////////////////////////// */

MprDelay::MprDelay(const UtlString& rName, int capacity)
: MpAudioResource(rName, 1, 1, 1, 1)
, mState(MPDS_WAITING)
, mIsQuiescenceNotified(FALSE)
, mFifo(capacity+1)
{

}

// Destructor
MprDelay::~MprDelay()
{
}

/* =============================== CREATORS =============================== */

/* ============================= MANIPULATORS ============================= */

UtlBoolean MprDelay::startPlay(const UtlString& namedResource, 
                               OsMsgQ& fgQ)
{
   MpResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_DELAY_START_PLAY, namedResource);
   return fgQ.send(msg);
}

UtlBoolean MprDelay::stopPlay(const UtlString& namedResource, 
                              OsMsgQ& fgQ)
{
   MpResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_DELAY_STOP_PLAY, namedResource);
   return fgQ.send(msg);
}


/* ============================== ACCESSORS =============================== */

/* =============================== INQUIRY ================================ */

int MprDelay::getDelayMs()
{
   if (mpFlowGraph == NULL)
      return -1;

   return mFifo.getFifoLength() * mpFlowGraph->getSamplesPerFrame() * 1000 / mpFlowGraph->getSamplesPerSec();
}

int MprDelay::getDelaySamples()
{
   if (mpFlowGraph == NULL)
      return -1;

   return mFifo.getFifoLength() * mpFlowGraph->getSamplesPerFrame();
}

int MprDelay::getDelayFrames()
{
   if (mpFlowGraph == NULL)
      return -1;

   return mFifo.getFifoLength();
}

/* ////////////////////////////// PROTECTED /////////////////////////////// */

UtlBoolean MprDelay::doDelay(MpBufPtr& iBuf,
                             MpBufPtr& oBuf,
                             int samplesPerFrame,
                             int samplesPerSecond)
{
     
   UtlBoolean res = TRUE;
   MpAudioBufPtr inBuf = iBuf;

   if (inBuf.isValid())
      assert((int)(inBuf->getSamplesNumber()) == samplesPerFrame);

   switch (mState)
   {
   case MPDS_WAITING:
      if (!inBuf.isValid() || !isActiveAudio(inBuf->getSpeechType()))
         break;
      mState = MPDS_BUFFERING;
      sendNotification(MpResNotificationMsg::MPRNM_DELAY_SPEECH_STARTED);

   case MPDS_BUFFERING:
      mFifo.pushData(inBuf);
      break;

   case MPDS_PLAYING:
      if (!mFifo.isEmpty())
      {
         MpAudioBufPtr outBuf;

         mFifo.pushData(inBuf);
         mFifo.popData(outBuf);
         oBuf = outBuf;

         // Check quiescence
         if (mFifo.isQuiescent())
         {
            if (!mIsQuiescenceNotified)
            {
               mIsQuiescenceNotified = TRUE;
               sendNotification(MpResNotificationMsg::MPRNM_DELAY_QUIESCENCE);
            }
         }
         else
         {
            if (mIsQuiescenceNotified)
            {
               mIsQuiescenceNotified = FALSE;
               sendNotification(MpResNotificationMsg::MPRNM_DELAY_SPEECH_STARTED);
            }
         }
         break;
      }
      // Else FIFO is empty and we're proceeding to MPDS_PLAYING_WITHOUT_DELAY
      mState = MPDS_PLAYING_WITHOUT_DELAY;
      sendNotification(MpResNotificationMsg::MPRNM_DELAY_NO_DELAY);

   case MPDS_PLAYING_WITHOUT_DELAY:
      oBuf = iBuf;
      break;
   }
   return res;
}

UtlBoolean MprDelay::handleMessage(MpResourceMsg& rMsg)
{
   UtlBoolean msgHandled = TRUE;
   switch (rMsg.getMsg())
   {
   case MPRM_DELAY_START_PLAY:
      mState = MPDS_PLAYING;
      break;
   case MPRM_DELAY_STOP_PLAY:
      mState = MPDS_WAITING;
      mFifo.reset();
      break;
   default:
      return MpResource::handleMessage(rMsg);
   }
   return msgHandled;
}


UtlBoolean MprDelay::doProcessFrame(MpBufPtr inBufs[],
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

   return doDelay(inBufs[0], outBufs[0], samplesPerFrame, samplesPerSecond);
}

/* /////////////////////////////// PRIVATE //////////////////////////////// */


MprDelay::MprDelayFifo::MprDelayFifo(int capacity)
: mBuffSize(capacity)
, mEndPosition(0)
, mStartPosition(0)
, mNumActiveFrames(0)
{
   mBuff = new MpAudioBufPtr[capacity];
   mBuffSize = capacity;
}

MprDelay::MprDelayFifo::~MprDelayFifo()
{
   delete[] mBuff;
}

void MprDelay::MprDelayFifo::reset()
{
   mStartPosition = 0;
   mEndPosition = 0;
   for (int i = 0; i < mBuffSize; i++)
      mBuff[i].release();
   mNumActiveFrames = 0;
}

void MprDelay::MprDelayFifo::pushData(MpAudioBufPtr& databuff)
{
   // Update number of active frames.
   if (databuff.isValid() && isActiveAudio(databuff->getSpeechType()))
   {
      mNumActiveFrames++;
   }

   // Store original start position and move it forward.
   int origStartPosition = mStartPosition;
   mStartPosition++;
   if (mStartPosition >= mBuffSize)
   {
      mStartPosition = 0;
   }

   // Store frame to the buffer.
   mBuff[origStartPosition].swap(databuff);

   // Check for overflow by checking start and end pointers equality.
   // If they're equal after we've moved mStartPosition, then queue was
   // full and have an overflow now.
   if (mEndPosition == mStartPosition)
   {
      // In case of overflow we must (1) bump mEndPosition and (2) adjust
      // mNumActiveFrames value according to the overwritten frame activity.
      MpAudioBufPtr emptyBuf;
      popData(emptyBuf);
      OsSysLog::add(FAC_AUDIO, PRI_WARNING,
                    "Delay FIFO buffer has overflowed! "
                    "Increase capacity of MprDelayFifo.");
   }
}

void MprDelay::MprDelayFifo::popData(MpAudioBufPtr& databuff)
{
   databuff.swap(mBuff[mEndPosition]);
   mEndPosition++;
   if (mEndPosition >= mBuffSize)
   {
      mEndPosition = 0;
   }

   // Update number of active frames.
   if (databuff.isValid() && isActiveAudio(databuff->getSpeechType()))
   {
      mNumActiveFrames--;
   }
}

int MprDelay::MprDelayFifo::getFifoLength() const
{
   if (mStartPosition >= mEndPosition)
      return mStartPosition - mEndPosition;
   else
      return mStartPosition + mBuffSize - mEndPosition;
}

UtlBoolean MprDelay::MprDelayFifo::isQuiescent() const
{
   return mNumActiveFrames == 0;
}

UtlBoolean MprDelay::MprDelayFifo::isEmpty() const
{
   return mStartPosition == mEndPosition;
}


/* ============================== FUNCTIONS =============================== */


