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

#ifdef __pingtel_on_posix__
#  ifdef __linux__
#     include <stdlib.h>
      typedef __int64_t __int64;
#  elif defined(sun)
#     include <sys/int_types.h>
      typedef int64_t __int64;
#  elif defined(__MACH__) /* OS X */
#     include <sys/types.h>
      typedef int64_t __int64;
#  else
#     error Unsupported POSIX OS.
#  endif
#endif

// APPLICATION INCLUDES

#include <os/OsMsg.h>
#include <os/OsMsgPool.h>

#include "mp/MpBuf.h"
#include "mp/MprToSpkr.h"
#include "mp/MpBufferMsg.h"
#include "mp/dsplib.h"
#include "mp/MpMediaTask.h"

#ifdef RTL_ENABLED
#   include <rtl_macro.h>
#endif

int iTrainingNoiseFlag = 0;
static int iComfortNoiseFlag = 1;
/*
int comfortNoise(int Flag) {
   int save = iComfortNoiseFlag;
   iComfortNoiseFlag = Flag;
   return (save);
}
*/

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
TOSPEAKERHOOK MprToSpkr::s_fnToSpeakerHook = NULL ;

static const int DEF_INIT_VOL = 363188;
static const int DEF_VOL_STEP = 27500; // approx. 2dB

int MprToSpkr::slInitVol   = DEF_INIT_VOL;
int MprToSpkr::slVolStep   = DEF_VOL_STEP;

#ifdef DETECT_SPKR_OVERFLOW /* [ */
int MprToSpkr::smStatsReports = 0;
#endif /* DETECT_SPKR_OVERFLOW ] */

#ifdef REIMPLEMENT_CLARISIS_EQ /* [ */
int MprToSpkr::smClarisisHandsetSpeakerEq[EqFilterLen_ix] =  {
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 74,
      231, -6883, 32767, -6883, 231, 74
   };
#endif /* REIMPLEMENT_CLARISIS_EQ ] */


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprToSpkr::MprToSpkr(const UtlString& rName,
                     int samplesPerFrame,
                     int samplesPerSec,
                     OsMsgQ *pSpkQ,
                     OsMsgQ *pEchoQ)
:  MpAudioResource(rName, 1, 1, 0, 1, samplesPerFrame, samplesPerSec)
,  mpSpkQ(pSpkQ)
,  mpEchoQ(pEchoQ)
,  mulNoiseLevel(1000L)
#ifdef DETECT_SPKR_OVERFLOW /* [ */
,  mOverflowsIn(0)
,  mUnderflowsIn(0)
,  mOverflowsOut(0)
,  mUnderflowsOut(0)
,  mTotalSamples(0)
,  mMaxIn(0)
,  mMinIn(0)
,  mMaxOut(0)
,  mMinOut(0)
#endif /* DETECT_SPKR_OVERFLOW ] */

#ifdef REIMPLEMENT_CLARISIS_EQ /* [ */
,  mpEqSave_ix(NULL)
,  mpCurEq_ix(NULL)
,  mLastSpkr_ix(0)
#endif /* REIMPLEMENT_CLARISIS_EQ ] */
{
   int i;

   init_CNG();

#ifdef REIMPLEMENT_CLARISIS_EQ /* [ */
   mpCurEq_ix = &MprToSpkr::smClarisisHandsetSpeakerEq[0];
   mLastSpkr_ix = CODEC_DISABLE_SPKR;

   mpEqSave_ix = new int[samplesPerFrame + EqFilterLen_ix - 1];
   memset(mpEqSave_ix, 0, (samplesPerFrame + EqFilterLen_ix - 1) * sizeof(int));
#endif /* REIMPLEMENT_CLARISIS_EQ ] */

   /*** Speaker volume control ***/
   /* Originally, it is set to control by the codec. It is moved to here for
      acoustic echo cancellation purpose */

   for (i = 0; i < (VOLUME_CONTROL_TABLE_SIZE); i++) {
      mlpVolTable[i] = 0;
   }

#ifdef DETECT_SPKR_OVERFLOW /* [ */
   mReport = smStatsReports;
#endif /* DETECT_SPKR_OVERFLOW ] */

}

// Destructor
MprToSpkr::~MprToSpkr()
{
#ifdef DETECT_SPKR_OVERFLOW /* [ */
   stats();
#endif /* DETECT_SPKR_OVERFLOW ] */
}

/* ============================ MANIPULATORS ============================== */

#ifdef DETECT_SPKR_OVERFLOW /* [ */
int MprToSpkr::spkrStats()
{
   return smStatsReports++; // trigger another report and reset
}
#endif /* DETECT_SPKR_OVERFLOW ] */


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

#ifdef DETECT_SPKR_OVERFLOW /* [ */
void MprToSpkr::stats()
{
   if ((mOverflowsOut+mUnderflowsOut+mOverflowsIn+mUnderflowsIn) > 0) {
      osPrintf(
         "MprToSpkr(0x%X): volume control handled %d total samples\n"
         "  input under/overflows %d+%d, output %d+%d\n",
         (int) this, mTotalSamples,
         mUnderflowsIn, mOverflowsIn, mUnderflowsOut, mOverflowsOut);
   } else {
      osPrintf("MprToSpkr(0x%X): no under/overflows in %d samples\n",
         (int) this, mTotalSamples);
   }
/*
   osPrintf(" ranges: input(%d .. %d), output(%d .. %d)\n",
      mMinIn, mMaxIn, mMinOut, mMaxOut);
*/

   mMinIn = mMaxIn = mMinOut = mMaxOut = mOverflowsOut =
   mUnderflowsOut = mOverflowsIn = mUnderflowsIn = mTotalSamples = 0;
}
#endif /* DETECT_SPKR_OVERFLOW ] */

void MprToSpkr::initVolTable()
{

   __int64 tempGain;

   int i;

   mlpVolTable[0] = (int) (tempGain = MprToSpkr::slInitVol);
   mlpVolTable[1] = (int) tempGain;
   mlpVolTable[VOLUME_CONTROL_TABLE_SIZE-2] = 0;
   mlpVolTable[VOLUME_CONTROL_TABLE_SIZE-1] = 0;
   for (i = 2; i < (VOLUME_CONTROL_TABLE_SIZE-2); i++) {
      mlpVolTable[i] = (int) (tempGain = (tempGain * slVolStep)>>15);
   }

   osPrintf("MprToSpkr::gains:");
   for( i = 0; i < VOLUME_CONTROL_TABLE_SIZE; i++) {
      osPrintf("%c%d", ((7 == (i&7)) ? '\n' : ' '), mlpVolTable[i]);
   }
   osPrintf("\n");

}


#ifdef REIMPLEMENT_CLARISIS_EQ /* [ */
void MprToSpkr::SpeakerEqualization_ix(MpAudioSample* shpSamples, int iLength)
{

   int         ShiftGain_ix = 13;
   int         i;
   int         c;
   int         iSample;
   int*        ip = mpEqSave_ix;
   const int*  pCurCoeff;
   MpAudioSample*     sp = shpSamples;

   const int* pCoeffSet = &MprToSpkr::smClarisisHandsetSpeakerEq[0];


   for (c = 1; c < EqFilterLen_ix; c++) {  // copy last few down to beginning
      *ip = ip[iLength];
      ip++;
   }

   for (i = 0; i < iLength; i++) { // convert new set to 32 bits
      *ip++ = *sp++;
   }


   for (i = 0; i < iLength; i++) {
      iSample = 0;
      ip = &mpEqSave_ix[i];
      pCurCoeff = pCoeffSet;
      for (c = 0; c < EqFilterLen_ix; c++) {
         iSample += *pCurCoeff++ * *ip++;
      }

      // rescale, then down 3 dB for Handset, +3dB for Headset
      iSample = ((iSample >> 15) * 13300) >> ShiftGain_ix;
      if (iSample < MIN_SPKR_DtoA) iSample = MIN_SPKR_DtoA;
      if (iSample > MAX_SPKR_DtoA) iSample = MAX_SPKR_DtoA;
      shpSamples[i] = iSample;
   }
}
#endif /* REIMPLEMENT_CLARISIS_EQ ] */

UtlBoolean MprToSpkr::doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame,
                                     int samplesPerSecond)
{
    MpBufferMsg*   pMsg;
    MpBufferMsg*   pFlush;
    OsMsgPool*     spkrPool;
    MpAudioBufPtr  out;
    MpAudioSample* shpSamples;
    int            iLength;

    // We have only one input
    if (inBufsSize != 1)
    {      
        return FALSE;
    }

#ifdef DETECT_SPKR_OVERFLOW /* [ */
    if (mReport < smStatsReports) 
    {
        mReport = smStatsReports;
        stats();
    }
#endif /* DETECT_SPKR_OVERFLOW ] */

    // Do processing if enabled and if data is available
    if (isEnabled && inBufs[0].isValid()) 
    {

        // Own input buffer
        out = inBufs[0];
        inBufs[0].release();

        shpSamples = out->getSamplesWritePtr();
        iLength = out->getSamplesNumber();

        /////////////////////////////////////////////////
        // samples ready for EQ processing //////////////
        /////////////////////////////////////////////////

#ifdef REIMPLEMENT_CLARISIS_EQ /* [ */
        SpeakerEqualization_ix(shpSamples, iLength);
#endif /* REIMPLEMENT_CLARISIS_EQ ] */

        if(iTrainingNoiseFlag > 0) 
        {
            /* generate white noise to test the performance if AEC only.
             * This is for parameter tweaking only. The original speaker
             * signal will be dropped.
             */
            out->setSamplesNumber(samplesPerFrame);
            iLength = out->getSamplesNumber();
            white_noise_generator(shpSamples, iLength, iTrainingNoiseFlag);
        }
        else
        {
            if(out->getSpeechType() == MpAudioBuf::MP_SPEECH_COMFORT_NOISE) 
            {
                out->setSamplesNumber(samplesPerFrame);
                iLength = out->getSamplesNumber();
                if(iComfortNoiseFlag > 0) 
                {
                    comfort_noise_generator(shpSamples, iLength, mulNoiseLevel);
                }
                else 
                {
                    memset((char *)shpSamples, 0 , iLength*2);                     
                }
            }
            else 
            {
                background_noise_level_estimation(mulNoiseLevel, shpSamples, 
                        iLength);
            }
        }

        if (s_fnToSpeakerHook)
        {
            /* 
            * Allow an external identity to consume speaker data.  Ideally,
            * this should probably become a different resource, but 
            * abstracting a new CallFlowGraph is a lot of work.
            */

            s_fnToSpeakerHook(iLength, (short*)shpSamples) ;
        }

        // TODO:: I don't know why we set attenuation to 0 here. BTW, it used only in the MprEchoSuppress().
        out->setAttenDb(0);

        // Push data to the output, if connected.
        if (isOutputConnected(0))
        {
            outBufs[0] = out;
        }

#ifdef RTL_ENABLED
   RTL_EVENT("ToSpeaker queue", mpSpkQ->numMsgs());
#endif

        // Flush speaker queue if it is full.
        while (mpSpkQ && MAX_SPKR_BUFFERS < mpSpkQ->numMsgs()) 
        {
            if (mpSpkQ->receive((OsMsg*&) pFlush, OsTime::NO_WAIT_TIME) == OS_SUCCESS) 
            {
                pFlush->releaseMsg();
                osPrintf( "pSpkQ drained. %d msgs in queue now\n"
                        , mpSpkQ->numMsgs());
            } 
        }

        // Prepare msg to be sent to Media Task
        spkrPool = MpMediaTask::getMediaTask(0)->getBufferMsgPool();
        assert(NULL != spkrPool);
        pMsg = spkrPool ? (MpBufferMsg*) spkrPool->findFreeMsg() : NULL;
        if (NULL == pMsg) 
        {
            pMsg = new MpBufferMsg(MpBufferMsg::AUD_PLAY);
        } 

        pMsg->setMsgSubType(MpBufferMsg::AUD_PLAY);

        // Copy buffer to the message
        pMsg->setBuffer(out);

        // Send data to Media Task
        if (  mpSpkQ
           && (mpSpkQ->send(*pMsg, OsTime::NO_WAIT_TIME) == OS_SUCCESS))
        {
            // Post a copy of this message to the mpEchoQ so that it
            // can be used in AEC calculations.
            MpBufferMsg AECMsg(MpBufferMsg::ACK_EOSTREAM);

            // TODO: We should pre-allocate a bunch of messages for 
            //       this purpose (see DmaMsgPool as an example).
             
            // Buffer is moved to the message. ob pointer is invalidated.
            AECMsg.ownBuffer(out) ;         
            if (  mpEchoQ->numMsgs() >= mpEchoQ->maxMsgs()
               || mpEchoQ->send(AECMsg, OsTime::NO_WAIT_TIME) != OS_SUCCESS)
            {
//               osPrintf("pEchoQ->send() failed!\n");
            }
        } 
        else
        {
            osPrintf("pSpkQ->send() failed!\n");
            if (pMsg->isMsgReusable()) 
            {
                pMsg->releaseMsg();
            }
        }
        
        if (!pMsg->isMsgReusable()) 
        {
            delete pMsg;
        }
    }
    else 
    {
        // Push data to the output, if connected.
        if (isOutputConnected(0))
        {
            outBufs[0] = inBufs[0];
        }
    }

   return TRUE;
}

/* ============================ FUNCTIONS ================================= */
