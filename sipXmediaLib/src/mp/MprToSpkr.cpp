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

#include <os/OsMsg.h>
#include <os/OsMsgPool.h>

#include "mp/MpBuf.h"
#include "mp/MprToSpkr.h"
#include "mp/MpBufferMsg.h"
#include "mp/dsplib.h"
#include "mp/MpMediaTask.h"
#ifdef _VXWORKS
#include "pinger/PingerInfo.h"
#endif

int iTrainingNoiseFlag = 0;
static int iComfortNoiseFlag = 1;
int comfortNoise(int Flag) {
   int save = iComfortNoiseFlag;
   iComfortNoiseFlag = Flag;
   return (save);
}

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

#ifdef _VXWORKS /* [ */
#include "mp/DspResampling.h"
#ifdef FLOWGRAPH_DOES_RESAMPLING /* [ */
   int DmaExpectsToSpkrToResample = 0;
   extern int ToSpkrResamplesForDma;
#else /* FLOWGRAPH_DOES_RESAMPLING ] [ */
   int ToSpkrExpectsDmaToResample = 0;
   extern int DmaResamplesForToSpkr;
#endif /* FLOWGRAPH_DOES_RESAMPLING ] */
#endif /* _VXWORKS ] */

// CONSTANTS
static const int NO_WAIT = 0;
#ifdef INJECT_32K_TO_SPEAKER /* [ */
static const int MAXFILESIZE = 4000000;
static const int MINFILESIZE = 8000;
static const int READ_BUFFER_SIZE = 8000;
#endif /* INJECT_32K_TO_SPEAKER ] */

// STATIC VARIABLE INITIALIZATIONS

#ifdef INJECT_32K_TO_SPEAKER /* [ */
Sample StereoMask0 = 0xffff, StereoMask1 = 0xffff;
#endif /* INJECT_32K_TO_SPEAKER ] */

//atic const int DEF_INIT_VOL = 32768 * 4 * 4; // about 12 dB higher...
//atic const int DEF_INIT_VOL =  6300 * 4 * 4; // how about 9 dB higher...
static const int DEF_INIT_VOL = 363188;
static const int DEF_VOL_STEP = 27500; // approx. 2dB

int MprToSpkr::slInitAtten = 32768;
int MprToSpkr::slInitVol   = DEF_INIT_VOL;
int MprToSpkr::slVolStep   = DEF_VOL_STEP;

#define DEFAULT_RAMP_STEPS 256
#ifndef TUNING_AUDIO_POP_DELAY /* [ */
const
#endif /* TUNING_AUDIO_POP_DELAY ] */
   int MprToSpkr::sNRampSteps = DEFAULT_RAMP_STEPS;

#ifdef TUNING_AUDIO_POP_DELAY /* [ */
extern "C" {extern int RampSteps(int num);};

int RampSteps(int num) {
   return MprToSpkr::setRampSteps(num);
}
#endif /* TUNING_AUDIO_POP_DELAY ] */

#ifdef DETECT_SPKR_OVERFLOW /* [ */
int MprToSpkr::smStatsReports = 0;
#endif /* DETECT_SPKR_OVERFLOW ] */

#ifdef REIMPLEMENT_CLARISIS_EQ /* [ */
#if defined(_WIN32) || defined(__pingtel_on_posix__) /* [ */
int MprToSpkr::smClarisisHandsetSpeakerEq[EqFilterLen_ix] =  {
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 74,
      231, -6883, 32767, -6883, 231, 74
   };
#endif /* WIN32 ] */
#endif /* REIMPLEMENT_CLARISIS_EQ ] */

#ifdef _VXWORKS /* [ */

   int MprToSpkr::smHandsetNewEq[EqFilterLen] =  {
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 74,
      231, -6883, 32767, -6883, 231, 74
   };

   int MprToSpkr::smHandsetEq[EqFilterLen] =  {
      0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,
      1607, -1285, 3214,-4500, 20892, -4500, 3214, -1285, 1607
   };


   int MprToSpkr::smHeadsetEq[EqFilterLen] = {
      0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,
      0,0,0,0,0, 0, 32768
};
   int MprToSpkr::smBaseEq[EqFilterLen] = {
      0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,
      74, 231, -6883, 32767, -6883, 231, 74
      // 0,0,0,0,0, 0, 32768
};
   extern volatile int* pOsTC; // SA1110 timer

   int beforeSpkrEq;
   int afterSpkrEq;

// MpCodecSpkrChoice MprToSpkr::smForceSpkr = CODEC_DISABLE_SPKR;
MpCodecSpkrChoice MprToSpkr::smForceSpkr = (MpCodecSpkrChoice) -1;

int MprToSpkr::sbDoEqualization = 1;


#endif /* _VXWORKS ] */

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprToSpkr::MprToSpkr(const UtlString& rName,
                           int samplesPerFrame, int samplesPerSec)
:  MpResource(rName, 1, 1, 0, 2, samplesPerFrame, samplesPerSec),
   mCurAttenDb(ATTEN_LOUDEST),
   mMaxAttenDb(ATTEN_LOUDEST),
   mulNoiseLevel(1000L),
#ifdef _VXWORKS /* [ */
   mpDspResamp(NULL),
   mpEqSave(NULL),
   mLastSpkr(0),
   mpCurEq(NULL),
#endif /* _VXWORKS ] */
#ifdef INJECT_32K_TO_SPEAKER /* [ */
   mpFileBuffer(NULL),
   mpNotify(NULL),
   mPlaying(0),
   mFileBytes(0),
   mFileBufferIndex(0),
#endif /* INJECT_32K_TO_SPEAKER ] */
#ifdef DETECT_SPKR_OVERFLOW /* [ */
   mOverflowsIn(0),
   mUnderflowsIn(0),
   mOverflowsOut(0),
   mUnderflowsOut(0),
   mTotalSamples(0),
   mMaxIn(0),
   mMinIn(0),
   mMaxOut(0),
   mMinOut(0),
#endif /* DETECT_SPKR_OVERFLOW ] */

#ifdef REIMPLEMENT_CLARISIS_EQ /* [ */
#if defined(_WIN32) || defined(__pingtel_on_posix__) /* [ */
   mpEqSave_ix(NULL),
   mpCurEq_ix(NULL),
   mLastSpkr_ix(0),
#endif /* WIN32 ] */
#endif /* REIMPLEMENT_CLARISIS_EQ ] */

   mCurRampStep(0),
   mCurVolumeFactor(0),
   mOldVolumeFactor(0),
   mTargetVolumeFactor(0),
   mLastVolume(-1)
{
   int i;

   init_CNG();

#ifdef REIMPLEMENT_CLARISIS_EQ /* [ */
#if defined(_WIN32) || defined(__pingtel_on_posix__) /* [ */
   mpCurEq_ix = &MprToSpkr::smClarisisHandsetSpeakerEq[0];
   mLastSpkr_ix = CODEC_DISABLE_SPKR;

   mpEqSave_ix = new int[samplesPerFrame + EqFilterLen_ix - 1];
   memset(mpEqSave_ix, 0, (samplesPerFrame + EqFilterLen_ix - 1) * sizeof(int));
#endif /* WIN32 ] */
#endif /* REIMPLEMENT_CLARISIS_EQ ] */

#ifdef _VXWORKS /* [ */
#ifdef FLOWGRAPH_DOES_RESAMPLING /* [ */
   mpDspResamp = new DspResampling (4, samplesPerFrame);

   for (i = 0; i < GTABLE_SIZE; i++) {
      mlpAttenTable[i] = 0;
   }

#endif /* FLOWGRAPH_DOES_RESAMPLING ] */

   mpCurEq = &MprToSpkr::smHandsetNewEq[0];
   mLastSpkr = CODEC_DISABLE_SPKR;

   mpEqSave = new int[samplesPerFrame + EqFilterLen - 1];
   memset(mpEqSave, 0, (samplesPerFrame + EqFilterLen - 1) * sizeof(int));

#endif /* _VXWORKS ] */

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

#ifdef _VXWORKS /* [ */
#ifdef FLOWGRAPH_DOES_RESAMPLING /* [ */
   ToSpkrResamplesForDma = 1;
#else /* FLOWGRAPH_DOES_RESAMPLING ] [ */
   DmaResamplesForToSpkr = 1;
#endif /* FLOWGRAPH_DOES_RESAMPLING ] */

   if (mpDspResamp) delete mpDspResamp;

   if (NULL != mpEqSave) {
      delete[] mpEqSave;
      mpEqSave = NULL;
   }



#endif /* _VXWORKS ] */

#ifdef INJECT_32K_TO_SPEAKER /* [ */
   if (NULL != mpFileBuffer) {
      free(mpFileBuffer);
      mpFileBuffer = NULL;
   }
#endif /* INJECT_32K_TO_SPEAKER ] */
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

#if defined(__pingtel_on_posix__) /* [ */
int MpCodec_getVolume() { return 0;}
int MpCodec_isBaseSpeakerOn() { return 0;}
#endif /* __pingtel_on_posix__ ] */

void MprToSpkr::setAttenuation(int finalDb, int framesPerStep)
{
   assert((finalDb <= 0) && (finalDb >= ATTEN_QUIETEST));
   mMaxAttenDb = finalDb;

}

#ifdef _VXWORKS /* [ */

int MprToSpkr::setInitAtten(int gain)
{
   int save = MprToSpkr::slInitAtten;
   MprToSpkr::slInitAtten = gain;
   return save;
}

int MprToSpkr::setInitVol(int gain, int volStep)
{
   int save = MprToSpkr::slInitVol;

   if (0 == gain) gain = slInitVol;
   if (gain < 0) gain = DEF_INIT_VOL;
   if (0 == volStep) volStep = slVolStep;
   if (volStep < 0) volStep = DEF_VOL_STEP;

   if ((gain == slInitVol) && (volStep == slVolStep)) {
      osPrintf("ToSpkr: volume seeds are {%d,%d}\n", slInitVol, slVolStep);
   } else {
      osPrintf("ToSpkr: replacing volume seeds {%d,%d} with {%d,%d}\n",
         slInitVol, slVolStep, gain, volStep);
      slInitVol = gain;
      slVolStep = volStep;
   }
   return save;
}

#ifdef TUNING_AUDIO_POP_DELAY /* [ */
int MprToSpkr::setRampSteps(int num) {
   int save = MprToSpkr::sNRampSteps;
   if (num > 0) MprToSpkr::sNRampSteps = num;
   return save;
}
#endif /* TUNING_AUDIO_POP_DELAY ] */

int MprToSpkr::enableSpkrEq(int disable)
{
   int save = MprToSpkr::sbDoEqualization;
   MprToSpkr::sbDoEqualization = (0 == disable) ? 1 : 0;
   return save;
}

int MprToSpkr::disableSpkrEq(int enable)
{
   int save = MprToSpkr::sbDoEqualization;
   MprToSpkr::sbDoEqualization = (0 == enable) ? 0 : 1;
   return save;
}

MpCodecSpkrChoice MprToSpkr::forceSpkr(MpCodecSpkrChoice what)
{
   MpCodecSpkrChoice save = MprToSpkr::smForceSpkr;
   MprToSpkr::smForceSpkr = what;
   osPrintf("Filter arrays:\n"
      "  smHandsetNewEq @ 0x%X, smHeadsetEq @ 0x%X, smBaseEq @ 0x%X\n",
      &MprToSpkr::smHandsetNewEq[0], &MprToSpkr::smHeadsetEq[0],
      &MprToSpkr::smBaseEq[0]);
// osPrintf("Filter array:  smBaseEq @ 0x%X\n", &MprToSpkr::smBaseEq[0]);
   return save;
}

void MprToSpkr::suppression(Sample* shpSamples, int iLength) // PRIVATE $$$
{
   int i;
   int iVolume = MpCodec_getVolume();      // Volume control
   Sample sSample;
   int iSample;
   int volumeFactor;
   int ramping;

  /* Calculate attenuation degree of speaker based on the volume control.
   * Then, execute the attenuation via software.
   * Originally, the attenuation was done the the audio codec. We move
   * it to here for the purpose that the acoustic
   * canceler needs to information of the attenuated signals.
   */


   int atten = (iVolume < 0) ? (VOLUME_CONTROL_TABLE_SIZE-1) :
                               ((VOLUME_CONTROL_TABLE_SIZE-1) - iVolume);

   if(atten < 0)  atten = 0;

   if (atten != mLastVolume) {
      osPrintf("Volume change from %d to %d at 0x%08X\n",
         mLastVolume, atten, *pOsTC);
   }
   if ((atten != mLastVolume) || (mCurAttenDb != mMaxAttenDb)) {
      long long int temp1, temp2, temp3;
      mCurRampStep = 0;
      mOldVolumeFactor = mCurVolumeFactor;
      temp1 = mlpVolTable[atten];
      temp2 = mlpAttenTable[-mMaxAttenDb];
      temp3 = (temp1 * temp2) >> 15;
      mTargetVolumeFactor = temp3;
      mTotalRampFactor = mTargetVolumeFactor - mOldVolumeFactor;
      mLastVolume = atten;
      mCurAttenDb = mMaxAttenDb;
   }

   ramping = (mCurVolumeFactor != mTargetVolumeFactor);

   volumeFactor = mTargetVolumeFactor;

   for (i = 0; i < iLength; i++) {
      if (ramping) {
         mCurRampStep++;
         volumeFactor = mOldVolumeFactor +
            ((mCurRampStep * mTotalRampFactor) / MprToSpkr::sNRampSteps);
         mCurVolumeFactor = volumeFactor;
         if ((volumeFactor == mTargetVolumeFactor) ||
             (mCurRampStep >= MprToSpkr::sNRampSteps)) {
            ramping = FALSE;
         }
      }
      sSample = shpSamples[i];
      iSample = sSample;
#ifdef DETECT_SPKR_OVERFLOW /* [ */
      if (iSample > mMaxIn) mMaxIn = iSample;
      if (iSample < mMinIn) mMinIn = iSample;
      if (iSample > MAX_SPKR_DtoA) {
         mOverflowsIn++;
         iSample = MAX_SPKR_DtoA;
      }
      if (iSample < MIN_SPKR_DtoA) {
         mUnderflowsIn++;
         iSample = MIN_SPKR_DtoA;
      }
#else /* DETECT_SPKR_OVERFLOW ] [ */
      if (iSample > MAX_SPKR_DtoA) {
         iSample = MAX_SPKR_DtoA;
      }
      if (iSample < MIN_SPKR_DtoA) {
         iSample = MIN_SPKR_DtoA;
      }
#endif /* DETECT_SPKR_OVERFLOW ] */

      if (volumeFactor > 32767) volumeFactor = 32767;
      iSample = (iSample * volumeFactor ) / 32768;

#ifdef DETECT_SPKR_OVERFLOW /* [ */
      if (iSample > mMaxOut) mMaxOut = iSample;
      if (iSample < mMinOut) mMinOut = iSample;
      if (iSample > MAX_SPKR_DtoA) {
         mOverflowsOut++;
         iSample = MAX_SPKR_DtoA;
      }
      if (iSample < MIN_SPKR_DtoA) {
         mUnderflowsOut++;
         iSample = MIN_SPKR_DtoA;
      }
      mTotalSamples++;
#else /* DETECT_SPKR_OVERFLOW ] [ */
      if (iSample > MAX_SPKR_DtoA) {
         iSample = MAX_SPKR_DtoA;
      }
      if (iSample < MIN_SPKR_DtoA) {
         iSample = MIN_SPKR_DtoA;
      }
#endif /* DETECT_SPKR_OVERFLOW ] */

      shpSamples[i] = iSample;

   }

}
#endif /* _VXWORKS ] */

#ifdef INJECT_32K_TO_SPEAKER /* [ */
// play file w/ file name & repeat option
OsStatus MprToSpkr::playFile(const char* audioFileName, UtlBoolean repeat,
   OsNotification* event)
{
   char* charBuffer;
   FILE* audioFilePtr = NULL;
   OsStatus res = OS_INVALID_ARGUMENT;
   unsigned long filesize;
   unsigned long trueFilesize;
   int totalRead = 0;
   int bytesRead;
   int readLen;

   if (!audioFileName)
      return res;

   osPrintf("AudioFile: raw 32K file\n");
   audioFilePtr = fopen(audioFileName, "rb");
   if (audioFilePtr)
   {

      //get file size
      fseek(audioFilePtr, 0, SEEK_END);
      filesize = trueFilesize = ftell(audioFilePtr);
      fseek(audioFilePtr, 0, SEEK_SET);

      if (trueFilesize > MAXFILESIZE) {
         osPrintf("playFile('%s') WARNING:\n"
            "    length (%d) exceeds size limit (%d)\n",
            audioFileName, trueFilesize, MAXFILESIZE);
         filesize = MAXFILESIZE;
      }

      if (trueFilesize < MINFILESIZE) {
         osPrintf("playFile('%s') WARNING:\n"
            "    length (%d) is suspiciously short!\n",
            audioFileName, trueFilesize);
      }

      charBuffer = (char*) malloc(filesize);
      if (NULL == charBuffer) {
         fclose(audioFilePtr);
         return OS_NO_MEMORY;
      }

      while (NULL != audioFilePtr) {
         readLen = (filesize - totalRead);
         if (readLen > READ_BUFFER_SIZE) readLen = READ_BUFFER_SIZE;
         bytesRead = fread(charBuffer+totalRead, 1, readLen, audioFilePtr);
         if (bytesRead == readLen) {
             totalRead += bytesRead;
         } else {
             filesize = totalRead;  // read error, quit now
         }
         if (totalRead == filesize) {
             fclose(audioFilePtr);
             audioFilePtr = NULL;
         }
      }
   } else {
      return OS_INVALID_ARGUMENT;
   }

   if (0 < totalRead) {
      MpFlowGraphMsg msg(PLAY_FILE, this, event, charBuffer,
            repeat, totalRead);
      res = postMessage(msg);
      if (OS_SUCCESS != res) free(charBuffer);
   }

   return res;
}

void MprToSpkr::startPlay(void)
{
   mPlaying = 1;
}

#endif /* INJECT_32K_TO_SPEAKER ] */


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
   osPrintf(" ranges: input(%d .. %d), output(%d .. %d)\n",
      mMinIn, mMaxIn, mMinOut, mMaxOut);

   mMinIn = mMaxIn = mMinOut = mMaxOut = mOverflowsOut =
   mUnderflowsOut = mOverflowsIn = mUnderflowsIn = mTotalSamples = 0;
}
#endif /* DETECT_SPKR_OVERFLOW ] */

#ifdef _VXWORKS /* [ */
#ifdef FLOWGRAPH_DOES_RESAMPLING /* [ */

   // 26029 = 2.00 dB in Q15
   // 27571 = 1.50 dB in Q15
   // 28376 = 1.25 dB in Q15
   // 29205 = 1.00 dB in Q15

void MprToSpkr::initAttenTable()
{

   long long int tempGain;

   int i;

   mlpAttenTable[0] = tempGain = MprToSpkr::slInitAtten;
   for( i = 1; i < GTABLE_SIZE; i++) {
      mlpAttenTable[i] = tempGain = (tempGain * 29205)>>15;
   }

   osPrintf("MprToSpkr::atten:");
   for( i = 0; i < GTABLE_SIZE; i++) {
      osPrintf(" %d", mlpAttenTable[i]);
   }
   osPrintf("\n");

}
#endif /* FLOWGRAPH_DOES_RESAMPLING ] */
#endif /* _VXWORKS ] */

void MprToSpkr::initVolTable()
{

#ifdef _VXWORKS
   long long int tempGain;
#else
   __int64 tempGain;
#endif  /* _VXWORKS */

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

#ifdef _VXWORKS /* [ */
int* MprToSpkr::chooseEqFilter()
{
   MpCodecSpkrChoice curSpkr = MpCodec_getSpeakerMode();

   if (-1 != MprToSpkr::smForceSpkr) {
      curSpkr = MprToSpkr::smForceSpkr;
   }
   int NewHandset = 1;
   if(PingerInfo::getHandsetEQSetting() == PingerInfo::HANDSET_EQ_REV_C)
   {
      NewHandset = 1; // do EQ only for Rev C handset
      //osPrintf ("MprToSpkr EQ = Rev C Handset \n");
   }
   if(PingerInfo::getHandsetEQSetting() == PingerInfo::HANDSET_EQ_REV_B)
   {
      NewHandset = 0;
      //osPrintf ("MprToSpkr EQ = Rev B Handset \n");
   }

   if (mLastSpkr != curSpkr) {
      mLastSpkr = curSpkr;
      if (CODEC_DISABLE_SPKR == curSpkr) {
         mpCurEq = NULL;
      } else if ((CODEC_ENABLE_SPKR1 & curSpkr) && NewHandset) {
         mpCurEq = &MprToSpkr::smHandsetNewEq[0]; // Rev C handset
      } else if ((CODEC_ENABLE_SPKR1 & curSpkr) && !NewHandset) {
         mpCurEq = &MprToSpkr::smHandsetEq[0]; // Rev B handset
      } else if (CODEC_ENABLE_SPKR2 & curSpkr) {
         mpCurEq = &MprToSpkr::smBaseEq[0]; // Speakerphone
      } else if (CODEC_ENABLE_SPKR3 & curSpkr) {
         mpCurEq = &MprToSpkr::smHeadsetEq[0];
      } else if (CODEC_ENABLE_SPKR4 & curSpkr) {
         mpCurEq = &MprToSpkr::smHeadsetEq[0];
      } else if (CODEC_ENABLE_SPKR5 & curSpkr) {
         mpCurEq = &MprToSpkr::smHeadsetEq[0];
      } else if (CODEC_ENABLE_SPKR6 & curSpkr) {
         mpCurEq = &MprToSpkr::smBaseEq[0]; //Ringer
      }
   }

   return mpCurEq;
}

void MprToSpkr::equalization(Sample* shpSamples, int iLength)
{

   int ShiftGain;
   int i;
   int c;
   Sample sSample;
   int iSample;
   int* ip = mpEqSave;
   const int* pCoeffSet = chooseEqFilter();
   const int* pCurCoeff;
   Sample* sp = shpSamples;

   if (PingerInfo::getHookSwitchControl() == PingerInfo::HOOKSWITCH_CONTROL_NORMAL)
   {
      ShiftGain = 14; // normal handset operation
   }
   else // if (PingerInfo::getHookSwitchControl() == HOOKSWITCH_CONTROL_HEADSET)
   {
      ShiftGain = 13; // Headset button acts as a hookswitch, headset
                  // is connected to Handset port, +6dB gain
   }


   if (NULL == pCoeffSet) return;
   assert(NULL != shpSamples);

   for (c = 1; c < EqFilterLen; c++) {  // copy last few down to beginning
      *ip = ip[iLength];
      ip++;
   }

   for (i = 0; i < iLength; i++) { // convert new set to 32 bits
      *ip++ = *sp++;
   }

   for (i = 0; i < iLength; i++) {
      iSample = 0;
      ip = &mpEqSave[i];
      pCurCoeff = pCoeffSet;
      for (c = 0; c < EqFilterLen; c++) {
         iSample += *pCurCoeff++ * *ip++;
      }

      // rescale, then down 3 dB for Handset, +3dB for Headset
      iSample = ((iSample >> 15) * 13300) >> ShiftGain;
      if (iSample < MIN_SPKR_DtoA) iSample = MIN_SPKR_DtoA;
      if (iSample > MAX_SPKR_DtoA) iSample = MAX_SPKR_DtoA;
      shpSamples[i] = iSample;
   }

#if 0 /* [ */
   {
      static int loopCounter = 0;  // used for display purposes

      loopCounter++;
      if (loopCounter > 199)
      {
         osPrintf (" shift gain = %d \n", ShiftGain);
         loopCounter = 0;
      }
   }
#endif /* ] */


} //end equalization for VxWorks


#endif /* _VXWORKS ] */

#ifdef REIMPLEMENT_CLARISIS_EQ /* [ */
#if defined(_WIN32) || defined(__pingtel_on_posix__) /* [ */
void MprToSpkr::SpeakerEqualization_ix(Sample* shpSamples, int iLength)
{

   int         ShiftGain_ix = 13;
   int         i;
   int         c;
   int         iSample;
   int*        ip = mpEqSave_ix;
   const int*  pCurCoeff;
   Sample*     sp = shpSamples;

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

#if 0 /* PRINT [ */
   {
      static int loopCounter = 0;  // used for display purposes

      loopCounter++;
      if (loopCounter > 199)
      {
         osPrintf (" shift gain = %d \n", ShiftGain_ix);
         loopCounter = 0;
      }
   }
#endif /* PRINT ] */
}
#endif /* WIN32 ] */
#endif /* REIMPLEMENT_CLARISIS_EQ ] */

UtlBoolean MprToSpkr::doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame,
                                    int samplesPerSecond)
{
   MpBufferMsg* pMsg;
   MpBufferMsg* pFlush;
   OsMsgPool*   spkrPool;
   MpBufPtr     ob;
   MpBufPtr     out;
   Sample*      shpSamples;
   int          iLength;


   if (0 == inBufsSize) {
      osPrintf("ToSpkr: returning FALSE, inBufsSize=%d\n", inBufsSize);
      return FALSE;
   }

#ifdef _VXWORKS /* [ */
#ifdef FLOWGRAPH_DOES_RESAMPLING /* [ */
   if (mlpAttenTable[0] != MprToSpkr::slInitAtten) {
      initAttenTable();
   }
   if (mlpVolTable[0] != MprToSpkr::slInitVol) {
      initVolTable();
   }
#endif /* FLOWGRAPH_DOES_RESAMPLING ] */
#endif /* _VXWORKS ] */

#ifdef DETECT_SPKR_OVERFLOW /* [ */
   if (mReport < smStatsReports) {
      mReport = smStatsReports;
      stats();
   }
#endif /* DETECT_SPKR_OVERFLOW ] */



   out = *inBufs;
   if ((NULL != out) && isEnabled) {
      shpSamples = MpBuf_getSamples(out);
      iLength = MpBuf_getNumSamples(out);

      /////////////////////////////////////////////////
      // samples ready for EQ processing //////////////
      /////////////////////////////////////////////////

#ifdef REIMPLEMENT_CLARISIS_EQ /* [ */
#if defined(_WIN32) || defined(__pingtel_on_posix__) /* [ */

      SpeakerEqualization_ix(shpSamples, iLength);

#endif /* WIN32 ] */
#endif /* REIMPLEMENT_CLARISIS_EQ ] */



      if(iTrainingNoiseFlag > 0) {
        /* generate white noise to test the performance if AEC only.
         * This is for parameter tweaking only. The original speaker
         * signal will be dropped.
         */
         {
            MpBuf_delRef(out);
            out = MpBuf_getBuf(MpMisc.UcbPool, samplesPerFrame, 0, MP_FMT_T12);
            assert(NULL != out);
            shpSamples = MpBuf_getSamples(out);
         }
         white_noise_generator(shpSamples, iLength, iTrainingNoiseFlag);
      }
      else {
         if(out == MpMisc.comfortNoise) {
            MpBuf_delRef(out);
            out = MpBuf_getBuf(MpMisc.UcbPool,
                               samplesPerFrame, 0, MP_FMT_T12);
            assert(NULL != out);
            shpSamples = MpBuf_getSamples(out);
            if(iComfortNoiseFlag > 0) {
               comfort_noise_generator(shpSamples,
                              samplesPerFrame, mulNoiseLevel);
            }
            else {
                  memset((char *)shpSamples, 0 , iLength*2);
                  //osPrintf("%4ld ", mulNoiseLevel);
            }
         }
         else {
            background_noise_level_estimation(mulNoiseLevel,
                shpSamples, iLength);
         }
      }

#if !defined(_WIN32) && !defined(__pingtel_on_posix__) /* [ */
//DWW  (something is wrong with suppression...it supresses the
//     volume to nothing on WIN32)

//     Note - this is because of the MpCodec_getVolume() function defined above
//     to return 0 on WIN32 and now Linux as well. Someday this problem might
//     be fixed; it's caused by the VxWorks code that used to talk to the audio
//     hardware's volume control now implementing software volume adjustment
//     while the WIN32 code didn't change (it never talked to the volume
//     controls provided by the OS to begin with). -Mike

      out = MpBuf_allowMods(out);
      suppression(MpBuf_getSamples(out), MpBuf_getNumSamples(out));

      // EQ for Handset only
      if (MpCodec_isHandsetSpeakerOn())
      {
         equalization(MpBuf_getSamples(out), MpBuf_getNumSamples(out));
      }



#endif /* !WIN32 && !__pingtel_on_posix__ ] */

#ifdef FLOWGRAPH_DOES_RESAMPLING /* [ */
      ob = MpBuf_getBuf(MpMisc.DMAPool, 640, 0, MP_FMT_T12);
      assert(NULL != ob);
      Sample* dest = MpBuf_getSamples(ob);
#ifdef INJECT_32K_TO_SPEAKER /* [ */
      if (mPlaying && (NULL != mpFileBuffer))
      {
         int copyLen;
         int copies;

         mPlaying++;
#define FILE_IS_STEREO
#undef FILE_IS_STEREO
#ifdef FILE_IS_STEREO /* [ */
         copyLen = (mFileBytes - mFileBufferIndex);
         if (copyLen > 1280) copyLen = 1280;
         if (copyLen < 1280) memset (dest, 0, 1280);
         memcpy(dest, mpFileBuffer + mFileBufferIndex, copyLen);
#else /* FILE_IS_STEREO ] [ */
         copyLen = (mFileBytes - mFileBufferIndex);
         if (copyLen > 640) copyLen = 640;
         if (copyLen < 640) memset (dest, 0, 2*640);
         copies = copyLen / sizeof(Sample);
         {
            Sample* src = (Sample*) (mpFileBuffer + mFileBufferIndex);
            Sample* d16 = (Sample*) dest;
            while (copies > 0) {
               *d16++ = (*src) & StereoMask0;
               *d16++ = (*src++) & StereoMask1;
               copies--;
            }
         }
#endif /* FILE_IS_STEREO ] */
         mFileBufferIndex += copyLen;
         if (mFileBufferIndex >= mFileBytes) {
            osPrintf("MprToSpkr: finished playing, %d bytes, %d frames\n",
               mFileBufferIndex, mPlaying - 1);
            free(mpFileBuffer);
            mpFileBuffer = NULL;
            mPlaying = 0;
         }
      } else
#endif /* INJECT_32K_TO_SPEAKER ] */

      mpDspResamp->up(dest, shpSamples, MpCodec_isBaseSpeakerOn());

      MpBuf_delRef(out);
      if (isOutputConnected(1)) {
         MpBufPtr ob2 = MpBuf_getBuf(MpMisc.DMAPool, 640, 0, MP_FMT_T12);
         assert(NULL != ob2);
         short* dest2 = MpBuf_getSamples(ob2);
         memcpy(dest2, dest, MpBuf_getByteLen(ob));
         outBufs[1] = ob2;
      }
#else /* FLOWGRAPH_DOES_RESAMPLING ] [ */
      ob = out;
#endif /* FLOWGRAPH_DOES_RESAMPLING ] */

#ifdef _VXWORKS /* [ */
      if (MpMisc.doLoopBack) {
         MpBufPtr dup;
         MpBuf_addRef(ob);
         dup = MpBuf_allowMods(ob);
         MpBuf_setOsTC(dup, *pOsTC);
         while (MpMisc.max_mic_buffers < MpMisc.pLoopBackQ->numMsgs()) {
            OsStatus  res;
            res=MpMisc.pLoopBackQ->receive((OsMsg*&) pFlush, OsTime::NO_WAIT);
            if (OS_SUCCESS == res) {
               MpBuf_delRef(pFlush->getTag());
               MpBuf_delRef(pFlush->getTag(1));
               pFlush->releaseMsg();
            } else {
               osPrintf("MprToSpkr: queue was full, now empty (1)!"
                  " (res=%d)\n", res);
            }
         }

         MpBuf_setAtten(dup, 0);


         spkrPool = MpMediaTask::getMediaTask(0)->getBufferMsgPool();
         assert(NULL != spkrPool);
         pMsg = spkrPool ? (MpBufferMsg*) spkrPool->findFreeMsg() : NULL;
         if (NULL == pMsg) {
            pMsg = new MpBufferMsg(MpBufferMsg::AUD_PLAY, __LINE__);
         } else {
            pMsg->setTag(NULL);
            pMsg->setTag(NULL, 1);
            pMsg->setTag(NULL, 2);
            pMsg->setTag(NULL, 3);
         }

         pMsg->setMsgSubType(MpBufferMsg::AUD_PLAY);
         pMsg->setTag(dup);
         pMsg->setBuf(MpBuf_getSamples(dup));
         pMsg->setLen(MpBuf_getNumSamples(dup));

         if (OS_SUCCESS == MpMisc.pLoopBackQ->send(*pMsg, OsTime::NO_WAIT)) {
            *inBufs = NULL;
         } else {
            if (pMsg->isMsgReusable()) pMsg->releaseMsg();
            MpBuf_delRef(dup);
         }
         if (!pMsg->isMsgReusable()) delete pMsg;
      }
#endif /* _VXWORKS ] */
      while (MpMisc.max_spkr_buffers < MpMisc.pSpkQ->numMsgs()) {
         OsStatus  res;
         res = MpMisc.pSpkQ->receive((OsMsg*&) pFlush, OsTime::NO_WAIT);
         if (OS_SUCCESS == res) {
            MpBuf_delRef(pFlush->getTag());
            MpBuf_delRef(pFlush->getTag(1));
            pFlush->releaseMsg();
         } else {
            osPrintf("MprToSpkr: queue was full, now empty (2)!"
               " (res=%d)\n", res);
         }
      }

      if (isOutputConnected(0)) {
         outBufs[0] = ob;
         MpBuf_addRef(ob);
      }


      MpBuf_setAtten(ob, 0);


      spkrPool = MpMediaTask::getMediaTask(0)->getBufferMsgPool();
      assert(NULL != spkrPool);
      pMsg = spkrPool ? (MpBufferMsg*) spkrPool->findFreeMsg() : NULL;
      if (NULL == pMsg) {
         pMsg = new MpBufferMsg(MpBufferMsg::AUD_PLAY, __LINE__);
      } else {
         pMsg->setTag(NULL);
         pMsg->setTag(NULL, 1);
         pMsg->setTag(NULL, 2);
         pMsg->setTag(NULL, 3);
      }

      pMsg->setMsgSubType(MpBufferMsg::AUD_PLAY);
      pMsg->setTag(ob);
      pMsg->setBuf(MpBuf_getSamples(ob));
      pMsg->setLen(MpBuf_getNumSamples(ob));


      if (OS_SUCCESS == MpMisc.pSpkQ->send(*pMsg, OsTime::NO_WAIT)) {
         *inBufs = NULL;
      } else {
         if (pMsg->isMsgReusable()) pMsg->releaseMsg();
         MpBuf_delRef(ob);
      }
      if (!pMsg->isMsgReusable()) delete pMsg;
   } else {
      mCurAttenDb = mMaxAttenDb;
      mCurRampStep = mCurVolumeFactor = mOldVolumeFactor = 0;
      mTotalRampFactor = mTargetVolumeFactor = mLastVolume = 0;
   }

   return TRUE;
}

#ifdef INJECT_32K_TO_SPEAKER /* [ */
// Handle messages for this resource.

UtlBoolean MprToSpkr::handleSetup(MpFlowGraphMsg& rMsg)
{
   if(mpFileBuffer) {
      osPrintf("MprToSpkr: freeing buffer=0x%X, %d bytes\n",
         mpFileBuffer, mFileBytes);
      free(mpFileBuffer);
      mpFileBuffer = NULL;
   }

   mFileBytes = rMsg.getInt2();
   mpNotify = (OsNotification*) rMsg.getPtr1();
   mpFileBuffer = (char*) rMsg.getPtr2();

   if(mpFileBuffer) {
      osPrintf("MprToSpkr: starting buffer=0x%X, %d bytes\n",
         mpFileBuffer, mFileBytes);
      mFileBufferIndex = 0;
      mFileRepeat = rMsg.getInt1();
   }
   return TRUE;
}

UtlBoolean MprToSpkr::handleMessage(MpFlowGraphMsg& rMsg)
{
   switch (rMsg.getMsg()) {
   case PLAY_FILE:
      return handleSetup(rMsg);
      break;

   default:
      return MpResource::handleMessage(rMsg);
      break;
   }
   return TRUE;
}

#endif /* INJECT_32K_TO_SPEAKER ] */

/* ============================ FUNCTIONS ================================= */
