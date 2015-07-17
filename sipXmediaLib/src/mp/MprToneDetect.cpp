//  
// Copyright (C) 2006-2015 SIPez LLC. All rights reserved.
//
// $$
// Simple implementation of a Tone Detector using the Goertzel
// algorithm with a window.  Target is 2175 Hz, block size (N) is 
// 160 (2 buffers worth), debouncing is done by computing average of last 3 
// and requring this to be above the threshold to send notification.  If 
// below the threshold and we've sent an "on", send and off.
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include <assert.h>
#include <cmath>
#include <limits>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsConfigDb.h"
#include "os/OsUtil.h"
#include "os/OsSysLog.h"
#include "mp/MpMisc.h"
#include "mp/MpBuf.h"
#include "mp/MpResourceMsg.h"
#include <mp/MpIntResourceMsg.h>
#include "mp/MprToneDetect.h"



// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS
const double MprToneDetect::DEFAULT_TARGET_FREQ=2175.0;
const double MprToneDetect::DEFAULT_SAMPLING_RATE=8000.0;
const double MprToneDetect::DEFAULT_THRESHOLD=10000.0;
const unsigned MprToneDetect::DEFAULT_BLOCK_SIZE=92; // other values to try 205, 160
const int MprToneDetect::DEFAULT_WINDOW_SIZE=3;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprToneDetect::MprToneDetect(const UtlString& rName)
: MpAudioResource(rName, 0, 1, 1, 1)
, mCoeff(0.0)
, mQ1(0.0)
, mQ2(0.0)
, mThreshold(DEFAULT_THRESHOLD)
, mTargetFreq(DEFAULT_TARGET_FREQ)
, mBlockCnt(0)
, mCurTd(false)
, mToneSignaled(false)
, mAvg3Mag(0.0)
, mSampleRate(DEFAULT_SAMPLING_RATE)
, mBlockSize(DEFAULT_BLOCK_SIZE)
, mWindowSize(DEFAULT_WINDOW_SIZE)
, mpFlowGraph(NULL)
{
    mFilterData = new double[mBlockSize];
    initFilter();
    initGoertzel();
}

// Destructor
MprToneDetect::~MprToneDetect()
{
    delete [] mFilterData;
}

/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean MprToneDetect::doProcessFrame(MpBufPtr inBufs[],
                                         MpBufPtr outBufs[],
                                         int inBufsSize,
                                         int outBufsSize,
                                         UtlBoolean isEnabled,
                                         int samplesPerFrame,
                                         int samplesPerSecond)
{
   MpAudioBufPtr out;
   const MpAudioSample* pBuf;

   // We're disabled or have nothing to process.
   if ( outBufsSize == 0 || inBufsSize == 0 )
   {
      return TRUE;
   }
   out = inBufs[0];
   if (!isEnabled)
   {
      outBufs[0] = out;
      return TRUE;
   }

   // We're not modifying the buffers, simply running them through the algorithm
   pBuf = out->getSamplesPtr();

   for (unsigned i = 0; i < out->getSamplesNumber(); i++)
   {
       processSample((MpAudioSample)filterSample(pBuf[i], mBlockCnt));
       mBlockCnt++; // this is "n" in the block
       if (mBlockCnt == mBlockSize)
       {
           double tMag = 0;
           // ok, we've computed a full block, start
           // seeing if we've detected tone
           tMag = sqrt(getMagnitudeSquared());
           mAvg3Mag += (tMag - mAvg3Mag) / mWindowSize;
           mCurTd = (mAvg3Mag > mThreshold) ? true : false;
           mBlockCnt = 0;
           resetGoertzel();
           OsSysLog::add(FAC_MP, PRI_DEBUG,"MprToneDetect cur %d avg3 %d", (int)tMag, (int)mAvg3Mag);
           if (mCurTd && !mToneSignaled)
           {
               // Send found notification
               OsSysLog::add(FAC_MP, PRI_DEBUG, "MprToneDetect sending MPRNM_TONE_DETECT_ON");
               sendNotification(MpResNotificationMsg::MPRNM_TONE_DETECT_ON);
               mToneSignaled = true;
           }
           else if (!mCurTd && mToneSignaled)
           {
               // send !found notification
               sendNotification(MpResNotificationMsg::MPRNM_TONE_DETECT_OFF);
               OsSysLog::add(FAC_MP, PRI_DEBUG, "MprToneDetect sending MPRNM_TONE_DETECT_OFF");
               mToneSignaled = false;
           }
       }
   }
   outBufs[0] = out;

   return TRUE;
}

// Handle messages for this resource.
UtlBoolean MprToneDetect::handleMessage(MpFlowGraphMsg& fgMsg)
{
   int msgType;

   msgType = fgMsg.getMsg();
   switch (msgType)
   {
   case SET_TARGET_FREQ:
       mTargetFreq = double(fgMsg.getInt1());
       initGoertzel();
       break;
   case SET_THRESHOLD:
       mThreshold = double(fgMsg.getInt1());
       initGoertzel();
       break;
   default:
      return MpAudioResource::handleMessage(fgMsg);
      break;
   }
   return TRUE;
}

// Handle messages for this resource.
UtlBoolean MprToneDetect::handleMessage(MpResourceMsg& rMsg)
{
   int msgType = rMsg.getMsg();
   switch (msgType)
   {
   case MPRM_CHANGE_TARGET_FREQ:
   {
       MpIntResourceMsg *pMsg = (MpIntResourceMsg*)&rMsg;
       mTargetFreq = double(pMsg->getData());
       initGoertzel();
   }
       break;
   case MPRM_CHANGE_THRESHOLD:
   {
       MpIntResourceMsg *pMsg = (MpIntResourceMsg*)&rMsg;
       mThreshold = double(pMsg->getData());
       initGoertzel();
   }
       break;
   default:
      // If we don't handle the message here, let our parent try.
      return MpResource::handleMessage(rMsg); 
      break;
   }
   return TRUE;
}

OsStatus MprToneDetect::setFlowGraph(MpFlowGraphBase* pFlowGraph)
{
   OsStatus res =  MpAudioResource::setFlowGraph(pFlowGraph);

   if (res == OS_SUCCESS)
   {
       mpFlowGraph = pFlowGraph;
   }
   return res;
}

// reset the algorithm for the next block
void MprToneDetect::resetGoertzel(void)
{
    mQ2 = 0;
    mQ1 = 0;
}

// this initializes the constants based on the target frequency
// resets counters, history and min/max
// call on ctor() or change in frequency/threshold
void MprToneDetect::initGoertzel(void)
{
#if 1
    int        k;
    double omega;

    k = (int) (0.5 + ((mBlockSize * mTargetFreq) / mSampleRate));
    omega = (2.0 * M_PI * k) / mBlockSize;
    mCoeff = 2.0 * cos(omega);
#else
    // the above simplifies to this, but the truncate to integer of k
    // gives better results than leaving it all floats
    mCoeff = 2.0 * cos((2.0 * M_PI * mTargetFreq / mSampleRate));
#endif
    mBlockCnt = 0;
    mCurTd = false;
    mAvg3Mag = 0.0;

    resetGoertzel();
}

// calculate the values for this sample
void MprToneDetect::processSample(MpAudioSample sample)
{
    double Q0;
    Q0 = mCoeff * mQ1 - mQ2 + (double) sample;
    mQ2 = mQ1;
    mQ1 = Q0;
}

// Magnitude of the block
double MprToneDetect::getMagnitudeSquared(void)
{
    double result;

    result = mQ1 * mQ1 + mQ2 * mQ2 - mQ1 * mQ2 * mCoeff;
    return result;
} 

#define HAMM
// initialize data for the filter
// done on ctor()
void MprToneDetect::initFilter()
{
    unsigned n;
    for (n = 0; n < mBlockSize; n++) 
    {
#ifdef HAMM
        mFilterData[n] = 0.54 - 0.46*cos(2.0*M_PI*n/mBlockSize);
#else
        mFilterData[n] = 0.426591 - .496561*cos(2.0*M_PI*n/mBlockSize) +
                                                .076848*cos(4.0*M_PI*n/mBlockSize);
#endif
    }
}

// apply the filter to the sample before calling processSample()
double MprToneDetect::filterSample(MpAudioSample sample, uint32_t n)
{
    return mFilterData[n]*double(sample);
}

OsStatus MprToneDetect::changeTargetFrequency(const UtlString& namedResource, 
                                              OsMsgQ& fgQ,
                                              int targetFreq)
{
   MpIntResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_CHANGE_TARGET_FREQ,
                        namedResource,
                        targetFreq);
   return fgQ.send(msg);
}


OsStatus MprToneDetect::changeDetectionThreshold(const UtlString& namedResource, 
                                                 OsMsgQ& fgQ,
                                                 int threshold)
{
   MpIntResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_CHANGE_THRESHOLD,
                        namedResource,
                        threshold);
   return fgQ.send(msg);
}

double MprToneDetect::getAvgMag() const
{
    return mAvg3Mag;
}

double MprToneDetect::getAvgMagnitude(const UtlString& refName)
{
    return getAvgMag();
}

   
/* ============================ FUNCTIONS ================================= */


