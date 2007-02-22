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


#ifdef WIN32
#define INSERT_SAWTOOTH
#undef INSERT_SAWTOOTH
#endif

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
#include "mp/MpBuf.h"
#include "mp/MprFromMic.h"
#include "mp/MpBufferMsg.h"
#include "mp/dmaTask.h"
#include "mp/DSP_type.h"


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

            s = out->getSamples();
            n = out->getSamplesNumber();
            
            s_fnMicDataHook(n, s) ;

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
                  shpSamples = out->getSamples();

                  tpBuf = MpMisc.RawAudioPool->getBuffer();
                  if (!out.isValid())
                     return FALSE;
                  tpBuf->setSamplesNumber(n);
                  assert(tpBuf.isValid());
                  shpTmpFrame = tpBuf->getSamples();
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
MpAudioBuf::SpeechType MprFromMic::speech_detected(MpAudioSample* shpSample, int iLength)
{
   int i;
   static Word64S  llLTPower = 8000L;
   static Word64S  llSTPower = 80000L;

   static int      iSpeechHangOver = 0;
   static short    shThreshold = 21799;     // 2.661 in Q13

   Word32  ulSample;
   Word32  ulSampleH;
   int     iSpeechCounter = 0;
   Word32  tmp32;

   for( i = 0; i < iLength; i++) {
      ulSample = (Word32) abs(*shpSample++);
      ulSampleH = ulSample << 8;

      if( ulSampleH > (Word32) llSTPower) {
         tmp32 = (Word32) shLambdaCSr * ulSample;
         llSTPower *= (Word64S) shLambdaSr;
         llSTPower += (((Word64S) tmp32) << 8);
         llSTPower >>= 15;
      }
      else {
         tmp32 = (Word32) shLambdaCSf * ulSample;
         llSTPower *= (Word64S) shLambdaSf;
         llSTPower += (((Word64S) tmp32) << 8);
         llSTPower >>= 15;
      }

      //If STPower > 2.661 LTPower, then speech activity
      //Note 2.661 == 8.5 dB

      Word64S tmp64 = llLTPower * (Word64S) shThreshold;
      tmp64 >>= 13;

      if(iSpeechHangOver > 0) FromMicThresh = 2;
      else FromMicThresh = 3;
      if (llSTPower > (llLTPower*FromMicThresh) ) {
         iSpeechCounter++;
      }
      else if((2*llSTPower) <= (llLTPower*3) ) {
         /* long term */
         tmp32 = (Word32) shLambdaC * ulSample;
         llLTPower *= (Word64S) shLambda;
         llLTPower += (((Word64S) tmp32) << 8);
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

MpAudioBuf::SpeechType MprFromMic::speech_detected( MpAudioSample* shpSample
                                                  , int iLength)
{
   int i;
   MpAudioSample prev;
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

static const short           shpB800[] = {15, 0, -123, -446, -844, 1542};
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
                short *signal,    /* input signal */
                short *pOutput,   /* output signal */
                short lg)         /* length of signal    */
{
   short   i, j;
   Word32S lS;           //32bit temp storage
   short*  shp1;
   short*  shp2;
   short*  shp0;

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
         lS += (Word32S) (*shp1++ + *shp2--) * (Word32S) shpB800[j];
      }
      pOutput[i] = (lS>>12);
   }
   return;
}
