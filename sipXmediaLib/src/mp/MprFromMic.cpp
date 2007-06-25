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


#ifdef WIN32
#define INSERT_SAWTOOTH
#undef INSERT_SAWTOOTH
#endif

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "mp/MpBuf.h"
#include "mp/MprFromMic.h"
#include "mp/MpBufferMsg.h"
#include "mp/dmaTask.h"

#ifdef RTL_ENABLED
#   include <rtl_macro.h>
#endif

// function prototype
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS
MICDATAHOOK MprFromMic::s_fnMicDataHook = 0 ;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprFromMic::MprFromMic(const UtlString& rName,
                       int samplesPerFrame,
                       int samplesPerSec,
                       OsMsgQ *pMicQ)
: MpAudioResource(rName, 0, 1, 1, 1, samplesPerFrame, samplesPerSec)
, mpMicQ(pMicQ)
, mNumFrames(0)
#ifndef REAL_SILENCE_DETECTION
, MinVoiceEnergy(0)
#endif
{
   Init_highpass_filter800();
}

// Destructor
MprFromMic::~MprFromMic()
{
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


UtlBoolean MprFromMic::doProcessFrame(MpBufPtr inBufs[],
                                      MpBufPtr outBufs[],
                                      int inBufsSize,
                                      int outBufsSize,
                                      UtlBoolean isEnabled,
                                      int samplesPerFrame,
                                      int samplesPerSecond)
{
   MpAudioBufPtr   out;
   MpBufferMsg*    pMsg;

   // We need one output buffer
   if (outBufsSize != 1) 
      return FALSE;

   // Don't waste the time if output is not connected
   if (!isOutputConnected(0))
       return TRUE;

   // One more frame processed
   mNumFrames++;

#ifdef RTL_ENABLED
   RTL_EVENT("FromMic queue", mpMicQ->numMsgs());
#endif

   if (isEnabled) 
   {
      // If the microphone queue (holds unprocessed mic data) has more then
      // the max_mic_buffers threshold, drain the queue until in range)
      while (mpMicQ && mpMicQ->numMsgs() > MpMisc.max_mic_buffers) 
      {
         if (mpMicQ->receive((OsMsg*&)pMsg, OsTime::NO_WAIT_TIME) == OS_SUCCESS) 
         {
            pMsg->releaseMsg();
                osPrintf( "mpMicQ drained. %d msgs in queue now\n"
                        , mpMicQ->numMsgs());
         }
      }

      if (mpMicQ && mpMicQ->numMsgs() > 0)
      {
         if (mpMicQ->receive((OsMsg*&)pMsg, OsTime::NO_WAIT_TIME) == OS_SUCCESS) 
         {
//                osPrintf( "mpMicQ->receive() succeed, %d msgs in queue\n"
//                        , mpMicQ->numMsgs());
            out = pMsg->getBuffer();
            pMsg->releaseMsg();
         }
      }
      else
      {
//         osPrintf("MprFromMic: No data available (total frames=%d)\n", 
//               mNumFrames);
      }

#ifdef INSERT_SAWTOOTH /* [ */
      if (!out.isValid())
      {
         out = MpMisc.RawAudioPool->getBuffer();
            if (!out.isValid())
               return FALSE;
         out->setSamplesNumber(MpMisc.frameSamples);
      }
      MpBuf_insertSawTooth(out);
      out->setSpeechType(MpAudioBuf::MP_SPEECH_ACTIVE);
#endif /* INSERT_SAWTOOTH ] */

      if (s_fnMicDataHook)
      {
         // 
         // Allow an external identity to source microphone data.  Ideally,
         // this should probably become a different resource, but abstracting
         // a new CallFlowGraph is a lot of work.
         //

         if (!out.isValid())
         {
            out = MpMisc.RawAudioPool->getBuffer();
            if (!out.isValid())
               return FALSE;
            out->setSamplesNumber(MpMisc.frameSamples);
         }
         
         if (out.isValid()) 
         {
            int n = 0;
            MpAudioSample* s = NULL;

            s = out->getSamplesWritePtr();
            n = out->getSamplesNumber();
            
            s_fnMicDataHook(n, (short*)s) ;

            out->setSpeechType(MpAudioBuf::MP_SPEECH_UNKNOWN);
         }
      }

      if (out.isValid())
      {
         switch(out->getSpeechType()) 
         {
            case MpAudioBuf::MP_SPEECH_TONE:
               break;
            case MpAudioBuf::MP_SPEECH_MUTED:
               out->setSpeechType(MpAudioBuf::MP_SPEECH_SILENT);
               break;
            default:
               {
                  MpAudioSample* shpTmpFrame;
                  MpAudioBufPtr tpBuf;

                  MpAudioSample *shpSamples;
                  int n = out->getSamplesNumber();
                  shpSamples = out->getSamplesWritePtr();

                  tpBuf = MpMisc.RawAudioPool->getBuffer();
                  if (!out.isValid())
                     return FALSE;
                  tpBuf->setSamplesNumber(n);
                  assert(tpBuf.isValid());
                  shpTmpFrame = tpBuf->getSamplesWritePtr();
                  highpass_filter800(shpSamples, shpTmpFrame, n);

                  out->setSpeechType(speech_detected(shpTmpFrame,n));
               }
               break;
         }
      }
    }
    else
    {
        out = inBufs[0];
    }

   outBufs[0] = out;

   return TRUE;
}

/* ============================ FUNCTIONS ================================= */

// 24 April 2001 (HZM)  I am disabling this because it takes
// too long to recognize the beginning of a talk spurt, and
// causes the bridge mixer to drop the start of each word.                                 
#ifdef REAL_SILENCE_DETECTION /* [ */

static const short         shLambda = 32765;        // 0.9999 in Q15
static const short         shLambdaC =    3;        // 0.0001 in Q15

static const short         shLambdaSr = 30147;      // 0.92 in Q15
static const short         shLambdaCSr = 2621;      // 0.08 in Q15

static const short         shLambdaSf = 32702;      // 0.998 in Q15
static const short         shLambdaCSf =   67;      // 0.002 in Q15

int FromMicThresh = 3;
MpAudioBuf::SpeechType MprFromMic::speech_detected(int16_t* shpSample, int iLength)
{
   int i;
   static int64_t  llLTPower = 8000L;
   static int64_t  llSTPower = 80000L;

   static int      iSpeechHangOver = 0;
   static int16_t  shThreshold = 21799;     // 2.661 in Q13

   uint32_t  ulSample;
   uint32_t  ulSampleH;
   int       iSpeechCounter = 0;
   uint32_t  tmp32;

   for( i = 0; i < iLength; i++) {
      ulSample = (uint32_t) abs(*shpSample++);
      ulSampleH = ulSample << 8;

      if( ulSampleH > (uint32_t) llSTPower) {
         tmp32 = (uint32_t) shLambdaCSr * ulSample;
         llSTPower *= (int64_t) shLambdaSr;
         llSTPower += (((int64_t) tmp32) << 8);
         llSTPower >>= 15;
      }
      else {
         tmp32 = (uint32_t) shLambdaCSf * ulSample;
         llSTPower *= (int64_t) shLambdaSf;
         llSTPower += (((int64_t) tmp32) << 8);
         llSTPower >>= 15;
      }

      //If STPower > 2.661 LTPower, then speech activity
      //Note 2.661 == 8.5 dB

      int64_t tmp64 = llLTPower * (int64_t) shThreshold;
      tmp64 >>= 13;

      if(iSpeechHangOver > 0) FromMicThresh = 2;
      else FromMicThresh = 3;
      if (llSTPower > (llLTPower*FromMicThresh) ) {
         iSpeechCounter++;
      }
      else if((2*llSTPower) <= (llLTPower*3) ) {
         /* long term */
         tmp32 = (uint32_t) shLambdaC * ulSample;
         llLTPower *= (int64_t) shLambda;
         llLTPower += (((int64_t) tmp32) << 8);
         llLTPower >>= 15;
      }

   }
   if(  (llSTPower>>4) > llLTPower )
      llLTPower = llSTPower >> 4;

   if(iSpeechCounter > 1)  {
      iSpeechHangOver = 30;
   }
   if(iSpeechHangOver) {
      iSpeechHangOver--;
      return MpAudioBuf::MP_SPEECH_ACTIVE;
   }
   else  {
      return MpAudioBuf::MP_SPEECH_SILENT;     // speech detected
   }
}

#else /* REAL_SILENCE_DETECTION ] [ */

MpAudioBuf::SpeechType MprFromMic::speech_detected( int16_t* shpSample
                                                  , int iLength)
{
   int i;
   int16_t prev;
   unsigned long energy = 0;
   unsigned long t;

   i = 0;
   while (i < iLength) {
      i++;
      prev = *shpSample++;
      t = (prev - *shpSample) >> 1;
      energy += t * t;
      if (energy >= MinVoiceEnergy)
          return MpAudioBuf::MP_SPEECH_ACTIVE;
   }

   return MpAudioBuf::MP_SPEECH_SILENT;
}

#endif /* REAL_SILENCE_DETECTION ] */

static const int             HP800_N = 10;
static const int             HP800_N_HALF = HP800_N/2 + 1;

static const int16_t           shpB800[] = {15, 0, -123, -446, -844, 1542};
/*
 * shpB800[0] =   15;   // 0.0036158;     in Q12
 * shpB800[1] =    0;   // 0.0;           in Q12
 * shpB800[2] = -123;   //-0.0299701;     in Q12
 * shpB800[3] = -446;   //-0.1090062;     in Q12
 * shpB800[4] = -844;   //-0.2061356;     in Q12
 * shpB800[5] = 1542;   // 0.3765164;     in Q12, original value 0.7530327. Here we
 *                      // divide it by 2 to make the following arthmetic process simpler.
 */
void MprFromMic::Init_highpass_filter800(void)
{
  int i;
  for(i = 0; i < 80+HP800_N; i++) {
      shpFilterBuf[i] = 0;
  }
}

void MprFromMic::highpass_filter800(
                int16_t *signal,    /* input signal */
                int16_t *pOutput,   /* output signal */
                short lg)         /* length of signal    */
{
   short   i, j;
   int32_t lS;           //32bit temp storage
   int16_t*  shp1;
   int16_t*  shp2;
   int16_t*  shp0;

   shp1 = shpFilterBuf;
   shp2 = shpFilterBuf + lg;
   for (i = 0; i < HP800_N; i++) {
      *shp1++ = *shp2++;
   }
   shp2 = signal;
   for (i = 0; i < lg; i++) {
      *shp1++ = *shp2++;
   }

   shp0 = shpFilterBuf;
   for(i = 0; i < lg; i++)
   {
      lS = 0L;
      shp1 = shp0++;
      shp2 = shp1 + HP800_N;
      for(j = 0; j < HP800_N_HALF; j++) {
         lS += (int32_t) (*shp1++ + *shp2--) * (int32_t) shpB800[j];
      }
      pOutput[i] = (lS>>12);
   }
   return;
}
