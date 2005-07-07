// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
#define INSERT_SAWTOOTH
#undef INSERT_SAWTOOTH
#endif
#define  SILENCE_DETECTION

// SYSTEM INCLUDES
#include <assert.h>

#ifdef __pingtel_on_posix__
#  ifdef __linux__
#     include <stdlib.h>
      typedef __int64_t __int64;
#  elif defined(sun)
#     include <sys/int_types.h>
      typedef int64_t __int64;
#  else
#     error Unsupported POSIX OS.
#  endif
#endif

// APPLICATION INCLUDES
#include "mp/MpMisc.h"
#include "mp/MpBuf.h"
#include "mp/MprFromMic.h"
#include "mp/MpBufferMsg.h"
#include "mp/dmaTask.h"
#include "mp/DSP_type.h"
#ifdef _VXWORKS
#include "pinger/PingerInfo.h"
#endif


// function prototype
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

#ifdef _VXWORKS /* [ */
#include "mp/DspResampling.h"
#ifdef FLOWGRAPH_DOES_RESAMPLING /* [ */
   int DmaExpectsFromMicToResample = 0;
   extern int FromMicResamplesForDma;
#else /* FLOWGRAPH_DOES_RESAMPLING ] [ */
   int FromMicExpectsDmaToResample = 0;
   extern int DmaResamplesForFromMic;
#endif /* FLOWGRAPH_DOES_RESAMPLING ] */
#endif /* _VXWORKS ] */

// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS

#ifdef _VXWORKS /* [ */


   static int micEqSampleLimit = 32000;
   int MLimit (int Input)
   {
      int save = micEqSampleLimit;
      micEqSampleLimit = Input;
      return (save);
   }
   
   extern volatile int* pOsTC; // SA1110 timer 
   int beforeMicEq;
   int afterMicEq;

   int MprFromMic::smHandsetNewEq[EqFilterLen] =  {  // for the NEW handset
             -81*3,
              78*3,
            -115*3,
             163*3,
            -225*3,
             306*3,
            -414*3,
             565*3,
            -792*3,
            1181*3,
           -2053*3,
            6287*3,
            6287*3,
           -2053*3,
            1181*3,
            -792*3,
             565*3,
            -414*3,
             306*3,
            -225*3,
             163*3,
            -115*3,
              78*3,
             -81*3
      };
   
   
   
/*  this is the current handset mic EQ
*/
   int MprFromMic::smHandsetEq[EqFilterLen] =  {
      0,0,0,0,0,
      0,0,0,0,0,
      0, 275*3, -536*3, 606*3, -312*3,
      -303*3, 933*3, 7000*3, 933*3, -303*3,
      -312*3, 606*3, -536*3, 275*3
   };





/*int MprFromMic::smHandsetEq[EqFilterLen] =  {
      0,0,0,0,0,
      0,0,0,0,0,
      0, 275*2, -536*2, 606*2, -312*2,
      -303*2, 933*2, 10396*2, 933*2, -303*2,
      500*2, 606*2, 500*2, 500*2
   };
*/
   int MprFromMic::smHeadsetEq[EqFilterLen] = {
      0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
      0,0,0, 32768
   };

#if 0 /////////////////////////////////////////
   int MprFromMic::smBaseEq[EqFilterLen] = {
          0,0,0,0,0, 0,0,0,0,0, 0,
         -908,  -457,   1617,   2106,  -2297, -9527,
          32767, -9527,  -2297,   2106, 1617,  -457,  -908
   };
#endif ///////////////////////////////////////

   int MprFromMic::smBaseEq[EqFilterLen] = {
          0,0,0,0,0, 0,0,0,0,0, 0,
          0,0,   600,   0,  -600, -3000,
          19660, -3000,  -600,   0, 600,  0,  0
   };


MpCodecMicChoice MprFromMic::smForceMic = (MpCodecMicChoice) -1;
int MprFromMic::sbDoEqualization = 1;
#endif /* _VXWORKS ] */

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprFromMic::MprFromMic(const UtlString& rName,
                           int samplesPerFrame, int samplesPerSec)
#ifdef  FLOWGRAPH_DOES_RESAMPLING /* [ */
:  MpResource(rName, 0, 0, 2, 2, samplesPerFrame, samplesPerSec),
#else /* FLOWGRAPH_DOES_RESAMPLING ] [ */
:  MpResource(rName, 0, 0, 1, 2, samplesPerFrame, samplesPerSec),
#endif /* FLOWGRAPH_DOES_RESAMPLING ] */
#ifdef _VXWORKS /* [ */
   mpEqSave(NULL),
   mpCurEq(NULL),
   mLastMic(0),
   mEqFilterZeroState0(0),
   mEqFilterZeroState1(0),
   mEqFilterZeroState2(0),
   mEqFilterPoleState0(0),
   mEqFilterPoleState1(0),
   mEqFilterPoleState2(0),
#endif /* _VXWORKS ] */
   mpDspResamp(0),
   mNumEmpties(0),
   mNumFrames(0)
{
#ifdef _VXWORKS /* [ */
#ifdef FLOWGRAPH_DOES_RESAMPLING /* [ */
   mpDspResamp = new DspResampling(4, samplesPerFrame);
#endif /* FLOWGRAPH_DOES_RESAMPLING ] */

   mpCurEq = &MprFromMic::smHandsetNewEq[0];
   mLastMic = CODEC_DISABLE_MIC;

   mpEqSave = new int[samplesPerFrame + EqFilterLen - 1];
   memset(mpEqSave, 0, (samplesPerFrame + EqFilterLen - 1) * sizeof(int));

#endif /* _VXWORKS ] */

#ifdef SILENCE_DETECTION
   Init_highpass_filter800();
#endif

}

// Destructor
MprFromMic::~MprFromMic()
{
#ifdef _VXWORKS /* [ */
#ifdef FLOWGRAPH_DOES_RESAMPLING /* [ */
   FromMicResamplesForDma = 1;
#else /* FLOWGRAPH_DOES_RESAMPLING ] [ */
   DmaResamplesForFromMic = 1;
#endif /* FLOWGRAPH_DOES_RESAMPLING ] */

   if (mpDspResamp) delete mpDspResamp;

   if (NULL != mpEqSave) {
      delete[] mpEqSave;
      mpEqSave = NULL;
   }
#endif /* _VXWORKS ] */
}

/* ============================ MANIPULATORS ============================== */

#ifdef _VXWORKS /* [ */
int MprFromMic::enableMicEq(int disable)
{
   int save = MprFromMic::sbDoEqualization;
   MprFromMic::sbDoEqualization = (0 == disable) ? 1 : 0;
   return save;
}

int MprFromMic::disableMicEq(int enable)
{
   int save = MprFromMic::sbDoEqualization;
   MprFromMic::sbDoEqualization = (0 == enable) ? 0 : 1;
   return save;
}

MpCodecMicChoice MprFromMic::forceMic(MpCodecMicChoice what)
{
   MpCodecMicChoice save = MprFromMic::smForceMic;
   MprFromMic::smForceMic = what;
   osPrintf("Filter arrays:\n"
      "  smHandsetEq @ 0x%X, smHeadsetEq @ 0x%X, smBaseEq @ 0x%X\n",
      &MprFromMic::smHandsetEq[0], &MprFromMic::smHeadsetEq[0],
      &MprFromMic::smBaseEq[0]);
   return save;
}
#endif /* _VXWORKS ] */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
#ifdef TIMING_ALTERNATE_CODEC_FORMAT /* [ */

// For gathering data on the approximate software impact of using an
// alternate codec chip that requires software handling of the serial
// data stream and access via the SSP, rather than the higher level
// handling that the MCP provides for a few particular codec chips.

static int doingCodecScramble = 0;
static unsigned short codecCmd1 = 0x0123;
static unsigned short codecCmd2 = 0x3210;
static unsigned short codecStat1;
static unsigned short codecStat2;

int codecScramble(int onOff) {
   int save = doingCodecScramble;
   doingCodecScramble = onOff;
   return save;
}
#endif /* TIMING_ALTERNATE_CODEC_FORMAT ] */

#ifdef _VXWORKS /* [ */
int* MprFromMic::chooseEqFilter()
{
   MpCodecMicChoice curMic = MpCodec_getMicMode();

   if (-1 != MprFromMic::smForceMic) {
      curMic = MprFromMic::smForceMic;
   }
   
   int NewHandset = 1; //default is new Rev C handset
   if (PingerInfo::getHandsetEQSetting() == PingerInfo::HANDSET_EQ_REV_C)
   {
      NewHandset = 1; // Rev C handset
      //osPrintf ("MprFromMic EQ = Rev C Handset \n");
   }
   else if (PingerInfo::getHandsetEQSetting() == PingerInfo::HANDSET_EQ_REV_B)
   {
      NewHandset = 0; // Rev B handset
      //osPrintf ("MprFromMic EQ = Rev B Handset \n");
   }

   if (mLastMic != curMic) {
      mLastMic = curMic;
      if (CODEC_DISABLE_MIC == curMic) {
         mpCurEq = NULL;
      }
      else if ((CODEC_ENABLE_MIC1 & curMic) && NewHandset) {
         mpCurEq = &MprFromMic::smHandsetNewEq[0]; //Rev C handset
      } 
      else if ((CODEC_ENABLE_MIC1 & curMic) && !NewHandset) {
         mpCurEq = &MprFromMic::smHandsetEq[0]; // Rev B handset
      } 
      
      else if (CODEC_ENABLE_MIC2 & curMic) {
         mpCurEq = &MprFromMic::smBaseEq[0];
      } 
      else if (CODEC_ENABLE_MIC3 & curMic) {
         mpCurEq = &MprFromMic::smHeadsetEq[0];
      }
   }
   return mpCurEq;
}

void MprFromMic::equalization(Sample* shpSamples, int iLength)
{
   
   /* 1000/3.6864 = 271.26736111111111 */
   /* 1000 nanoSec/microSec */
   /* 3.6864 MHz counter */
   /* 3 place accuracy */
   // beforeMicEq = *pOsTC;


   int i;
   int c;

   Sample sSample;
   int iSample;
   int* ip = mpEqSave;
   const int* pCoeffSet = chooseEqFilter();
   const int* pCurCoeff;
   Sample* sp = shpSamples;

   if (NULL == pCoeffSet) return;
   assert(NULL != shpSamples);

   for (c = 1; c < EqFilterLen; c++) {  // copy last few down to beginning
      *ip = ip[iLength];
      ip++;
   }

   for (i = 0; i < iLength; i++) { // convert new set to 32 bits
      *ip++ = *sp++;
   }

   for (i = 0; i < iLength; i++) 
      {
         iSample = 0;
         ip = &mpEqSave[i];
         pCurCoeff = pCoeffSet;
         for (c = 0; c < EqFilterLen; c++) 
         {
            iSample += *pCurCoeff++ * *ip++;
         }

         if (MpCodec_isBaseSpeakerOn())
         {
            iSample = iSample >> 13;   // was 14, +6dB increase together with
                                       // corresponding decrease by 6dB in MpCodec.cpp
            if (iSample > micEqSampleLimit)
            {
               iSample = micEqSampleLimit;
            }
            if (iSample < -micEqSampleLimit)
            {
               iSample = -micEqSampleLimit;
            }
            shpSamples[i] = iSample;
         }
         else // handset or headset
         {
            iSample = iSample >> 14;   

            if (iSample > micEqSampleLimit)
            {
               iSample = micEqSampleLimit;
            }
            if (iSample < -micEqSampleLimit)
            {
               iSample = -micEqSampleLimit;
            }
            shpSamples[i] = iSample;
         }


      }



//////////////////////////////////////////////////////////////////////////////
// High-pass filter removes DC and low frequency noise (of mysterious origin)
// below 40 hz.
//  3 zeros on unit circle at DC and +/- 29 hz.
//  3 poles all with r=0.958 and theta = 0 and +/- 60 hz.
//  scale factor 0.9385 normalizes the spectrum to unity gain.
//////////////////////////////////////////////////////////////////////////////
// filter power spectrum in db at multiples of 10 hz:
// -99.99 -33.85 -32.72 -47.27 -23.39 -15.69 -10.95  -7.77  -5.59  -4.09
//  -3.06  -2.35  -1.84  -1.47  -1.20  -1.00  -0.84  -0.72  -0.62  -0.54
//  -0.47  -0.42  -0.38  -0.34  -0.30  -0.28  -0.25  -0.23  -0.21  -0.19
//  -0.18  -0.17  -0.16  -0.14  -0.13  -0.13  -0.12  -0.11  -0.10  -0.10
//  -0.09  -0.09  -0.08  -0.08  -0.07  -0.07  -0.07  -0.06  -0.06  -0.06
//  -0.06  -0.05  -0.05  -0.05  -0.05  -0.04  -0.04  -0.04  -0.04  -0.04 etc.
//////////////////////////////////////////////////////////////////////////////
   int b[4];                // zero coefficients Q12 scaling
   int a[3];                // pole coefficients Q12 scaling

// $$$ These filter coefficients should be assigned once in static memory.
   b[0] =   3844;
   b[1] = -11530;
   b[2] =  11530;
   b[3] =  -3844;
   a[0] =  11763;
   a[1] = -11269;
   a[2] =   3601;
   int lS;


   for (i = 0; i < iLength; i++) {
      lS = ((Word64S)mEqFilterPoleState0 * (Word64S)a[0] +
             (Word64S)mEqFilterPoleState1 * (Word64S)a[1] +
             (Word64S)mEqFilterPoleState2 * (Word64S)a[2] ) >> 12;
      lS += (Word64S)shpSamples[i] * (Word64S)b[0] +
             (Word64S)mEqFilterZeroState0 * (Word64S)b[1] +
             (Word64S)mEqFilterZeroState1 * (Word64S)b[2] +
             (Word64S)mEqFilterZeroState2 * (Word64S)b[3];
      mEqFilterZeroState2 = mEqFilterZeroState1;
      mEqFilterZeroState1 = mEqFilterZeroState0;
      mEqFilterZeroState0 = shpSamples[i];
      mEqFilterPoleState2 = mEqFilterPoleState1;
      mEqFilterPoleState1 = mEqFilterPoleState0;
      mEqFilterPoleState0 = lS;

      iSample = (lS + 2048) >> 12;

      // limiting to 32767
      if (iSample > micEqSampleLimit) 
      {
         iSample = micEqSampleLimit;
      }
      if (iSample < -micEqSampleLimit)
      {
         iSample = -micEqSampleLimit;
      }
      shpSamples[i] = iSample;
   }

   
}

#endif /* _VXWORKS ] */

UtlBoolean MprFromMic::doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame,
                                     int samplesPerSecond)
{
   MpBufPtr        out;
   MpBufferMsg*    pMsg;

   if (0 == outBufsSize) return FALSE;

   out = NULL;

   mNumFrames++;
   if (0 == (mNumFrames & 0x1ff)) {
      mNumEmpties = 0;
   }
   if (isEnabled) {
      OsMsgQ* pMicOutQ;
#ifdef _VXWORKS /* [ */
      pMicOutQ = MpMisc.doLoopBack ? MpMisc.pLoopBackQ : MpMisc.pMicQ;
#else /* _VXWORKS ] [*/
      pMicOutQ = MpMisc.pMicQ;
#endif /* _VXWORKS ] */
      while (MpMisc.max_mic_buffers < pMicOutQ->numMsgs()) {
         if (OS_SUCCESS == pMicOutQ->receive((OsMsg*&) pMsg,
                                                       OsTime::NO_WAIT)) {
            Nprintf("MprFromMic: flushing(0x%X)\n",
                                     (int) pMsg->getTag(), 0,0,0,0,0);
            MpBuf_delRef(pMsg->getTag());
            MpBuf_delRef(pMsg->getTag(1));
            pMsg->releaseMsg();
         }
      }
      if (0 < pMicOutQ->numMsgs()) {
         if (OS_SUCCESS == pMicOutQ->receive((OsMsg*&) pMsg,
                                                      OsTime::NO_WAIT)) {
            out = pMsg->getTag();
            pMsg->releaseMsg();
            Nprintf("MprFromMic: received: 0x%X\n", (int) out, 0,0,0,0,0);

#ifdef _VXWORKS /* [ */
#ifdef FLOWGRAPH_DOES_RESAMPLING /* [ */
            MpBufPtr out1;
            if (MpBuf_isActiveAudio(out)) {
               out1 =
                  MpBuf_getBuf(MpMisc.UcbPool, samplesPerFrame, 0, MP_FMT_T12);
               assert(NULL != out1);
               Sample* src = MpBuf_getSamples(out);
               Sample* dst = MpBuf_getSamples(out1);

#ifdef CPU_XSCALE /* [ */
#define UseRight 1
#else /* CPU_XSCALE ] [ */
#define UseRight (MP_MIC_SELECT_BASE != MpDma_getMicMode())
#endif /* CPU_XSCALE ] [ */

               mpDspResamp->down(dst, src, UseRight);
            } else {
               out1 = MpBuf_getFgSilence();
            }
            MpBuf_setOsTC(out1, MpBuf_getOsTC(out));
            if (1 < numOutputs()) {
               outBufs[1] = out; // pass along the original, if linked
            } else {
               MpBuf_delRef(out);
            }
            out = out1;
#endif /* FLOWGRAPH_DOES_RESAMPLING ] */
#endif /* _VXWORKS ] */

#ifdef INSERT_SAWTOOTH /* [ */
            MpBuf_insertSawTooth(out);
#endif /* INSERT_SAWTOOTH ] */
#ifdef SILENCE_DETECTION /* [ */
            if(NULL != out ) {
#ifdef REAL_WAY /* [ */
               Sample* shpTmpFrame;
               MpBufPtr tpBuf;
               int n;
#endif /* REAL_WAY ] */

               switch(MpBuf_getSpeech(out)) {
               case MP_SPEECH_TONE:
                  break;

               case MP_SPEECH_MUTED:
                  MpBuf_setSpeech(out, MP_SPEECH_SILENT);
                  break;

               default:
#ifdef REAL_WAY /* [ */
                  Sample *shpSamples;
                  n = MpBuf_getNumSamples(out);
                  shpSamples = MpBuf_getSamples(out);

                  tpBuf = MpBuf_getBuf(MpMisc.UcbPool, n, 0, MP_FMT_T12);
                  assert(NULL != tpBuf);
                  shpTmpFrame = MpBuf_getSamples(tpBuf);
                  highpass_filter800(shpSamples, shpTmpFrame, n);

                  if(0 == speech_detected(shpTmpFrame,n)) {
                     MpBuf_setSpeech(out, MP_SPEECH_SILENT);
                  }
                  else {
                     MpBuf_setSpeech(out, MP_SPEECH_ACTIVE);
                  }
                  MpBuf_delRef(tpBuf);
#else /* REAL_WAY ] [ */
           // 24 April 2001 (HZM)  I am disabling this because it takes
           // too long to recognize the beginning of a talk spurt, and
           // causes the bridge mixer to drop the start of each word.
  // (hzm, 20010726, too aggressive...) MpBuf_setSpeech(out, MP_SPEECH_ACTIVE);
                  MpBuf_isActiveAudio(out);
#endif /* REAL_WAY ] */
                  break;
               }
            }
#endif /* SILENCE_DETECTION ] */
#ifdef _VXWORKS /* [ */
            if (MpBuf_isActiveAudio(out)) {
               if (MprFromMic::sbDoEqualization) {
                  out = MpBuf_allowMods(out);
                  equalization(MpBuf_getSamples(out), MpBuf_getNumSamples(out));
               }
            }
#endif /* _VXWORKS ] */

         }
      } else if (5 > mNumEmpties++) {
         osPrintf("FromMic: No incoming buffer?!?!?\n");
      }
   }

#ifdef INSERT_SAWTOOTH /* [ */
   if (NULL == out) { /* nothing in Q, or we are disabled */
      out = MpBuf_getBuf(MpMisc.UcbPool, MpMisc.frameSamples, 0, MP_FMT_T12);
      if (NULL != out) {
         int i, n;
         Sample *s;

         s = MpBuf_getSamples(out);
         n = MpBuf_getNumSamples(out);
         for (i=0; i<n; i++)
            *s++ = ((i % 16) << 12);
      }
   }
#endif /* INSERT_SAWTOOTH ] */
   if (NULL == out) { /* nothing in Q, or we are disabled */
      out = MpBuf_getFgSilence();
   }
#ifdef TIMING_ALTERNATE_CODEC_FORMAT /* [ */
   {
      int N = 0;

      assert(doingCodecScramble != 42);

      while (N++ < doingCodecScramble) {
         struct {
            unsigned short left;
            unsigned short cmd1;
            unsigned short right;
            unsigned short cmd2;
         } unpack[320];
         int i, n;
         Sample *ps;

         n = min(320, MpBuf_getNumSamples(out));
         ps = MpBuf_getSamples(out);
         for (i=0; i<n; i++) {
            unpack[i].left = *ps++;
            unpack[i].cmd1 = codecCmd1;
            unpack[i].right = 0;
            unpack[i].cmd2 = codecCmd2;
         }
         ps = MpBuf_getSamples(out);
         for (i=0; i<n; i++) {
            *ps++ = unpack[i].left;
            codecStat1 = unpack[i].cmd1;
            codecStat2 = unpack[i].cmd2;
         }
      }
   }
#endif /* TIMING_ALTERNATE_CODEC_FORMAT ] */
   *outBufs = out;

   return TRUE;
}






/* ============================ FUNCTIONS ================================= */
#ifdef SILENCE_DETECTION //[

static const short         shLambda = 32765;        // 0.9999 in Q15
static const short         shLambdaC =    3;        // 0.0001 in Q15

static const short         shLambdaSr = 30147;      // 0.92 in Q15
static const short         shLambdaCSr = 2621;      // 0.08 in Q15

static const short         shLambdaSf = 32702;      // 0.998 in Q15
static const short         shLambdaCSf =   67;      // 0.002 in Q15

int FromMicThresh = 3;
short MprFromMic::speech_detected(Sample* shpSample, int iLength)
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
      return (1);
   }
   else  {
      return (0);     // speech detected
   }
}

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
#endif  // ] SILENCE_DETECTION

#ifdef HF_ANALYSIS
   static unsigned long ulStrength = 1000L;
   static unsigned long ulStrength1 = 1000L;
   HF_HF(src, (samples<<3), &ulStrength, &ulStrength1);

   int kkk = ulStrength1/ulStrength;
#if 0
   if (ulStrength > 3000L && ulStrength < 6000L) {
      if(kkk < 16) osPrintf("NE ");
   }
#endif
   if (ulStrength >= 6000L && ulStrength < 9000L) {
      if(kkk < 12) osPrintf("NE1 ");
   }
   else if (ulStrength >= 9000L) {
      if( kkk < 9) osPrintf("NE2 ");
   }
   if(iShowHFFlag > 0) {
      osPrintf("%6d %6d %4d\n", ulStrength,ulStrength1,samples);
      iShowHFFlag--;
   }
#endif /* HF_ANALYSIS */
