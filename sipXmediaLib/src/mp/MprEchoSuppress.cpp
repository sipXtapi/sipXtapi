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

#ifndef __pingtel_on_posix__ /* [ */
/* to use this file with __pingtel_on_posix__, must #include <stdlib.h> */

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "mp/MpBuf.h"

#ifdef _VXWORKS /* [ */
#include "mp/MpCodec.h"
#endif /* _VXWORKS ] */

#include "mp/MprEchoSuppress.h"
#include "mp/MprToSpkr.h"
#include "mp/MpMisc.h"
#include "mp/MpBufferMsg.h"
#include "mp/DspResampling.h"
#include "mp/FilterBank.h"
#include "mp/HandsetFilterBank.h"

// EXTERNAL FUNCTIONS

#define DEBUG_TC
#undef  DEBUG_TC

#ifdef DEBUG_TC /* [ */


static int showSome = 0;

extern "C" int doShowMe() {
   if (showSome > 0) {
      showSome--;
      return 1;
   }
   showSome = 0;
   return 0;
}

static int sampleLimit = 32767;
int SLimit (int Input)
   {
      int save = sampleLimit;
      sampleLimit = Input;
      return (save);
   }

extern "C" int ShowMe(int v) {
   int save = showSome;
   showSome = v;
   return save;
}

static int HandAEC = 0;
extern "C" int Hand (int Flag)
{
   int save = HandAEC;
   HandAEC = Flag;
   return save;
}


#else /* DEBUG_TC ] [ */
#define HandAEC 0
#define sampleLimit 32767
#endif /* DEBUG_TC ] */


// EXTERNAL VARIABLES
// CONSTANTS

#define GTABLE_SIZE                7  // 310 ms transit time


unsigned short  gTableSize = GTABLE_SIZE;
unsigned short  shpAttenTable[GTABLE_SIZE];

extern int iLoudspeakerFadeDB;
extern int iDoubleTalkIndicatorFlag;

int iShowAudioSync = 0;


// Console function to toggle AEC on/off
static int iSub = 0;
int bypassSub(int iFlag) {
   int save = iSub;
   iSub = iFlag;
   return save;
}

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprEchoSuppress::MprEchoSuppress(const UtlString& rName,
                           int samplesPerFrame, int samplesPerSec)
:  MpAudioResource(rName, 1, 1, 1, 2, samplesPerFrame, samplesPerSec),

   mpFilterBank(0),
   mpHandsetFilterBank(0)
{
   int  i;
   long lS;
   shpAttenTable[0] = 32768;
   for( i = 1; i < GTABLE_SIZE; i++) {
      lS = (uint32_t) shpAttenTable[i-1] * 29205;  //29205 = 1.00 db in Q15
      shpAttenTable[i] = (short) ((lS >> 15));
   }

   mpFilterBank = new FilterBank();
   mpHandsetFilterBank = new HandsetFilterBank();


   mState  = MprToSpkr::ATTEN_LOUDEST;
   // Other initialization of energy detector
   mLastSpkrAtten = 0;
   mpPrev = NULL;
   mSpeechFake = 0;
   mTicksPerFrame = (3686400 * samplesPerFrame) / samplesPerSec;

   mshDelay = 0;

#ifdef DEBUG_TC /* [ */
   ShowMe(20);  // Enable a few lines of warnings...
#endif /* DEBUG_TC ] */

}

// Destructor
MprEchoSuppress::~MprEchoSuppress()
{
    if (mpFilterBank ) {
        delete mpFilterBank;
        mpFilterBank = 0;
    }

    if (mpHandsetFilterBank){
       delete mpHandsetFilterBank;
       mpHandsetFilterBank = 0;
    }
}

/* ============================ MANIPULATORS ============================== */
void MprEchoSuppress::setSpkrPal(MprToSpkr* pal)
{
   mpSpkrPal = pal;
}

int MprEchoSuppress::startSpeech()
{
   mSpeechFake = 1;
   return 0;
}

int MprEchoSuppress::endSpeech()
{
   mSpeechFake = -1;
   return 0;
}
/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void MprEchoSuppress::frame_match(const MpAudioBufPtr &in)
{
    MpBufferMsg*    pMsg;
    int             micStartTC;
    int             micEndTC;
    int             spkEndTC;

    micEndTC = in->getTimecode();
    micStartTC = micEndTC - mTicksPerFrame;

    // long lll = micEndTC - spkEndTC;
    if (mpPrev.isValid()) {
        spkEndTC = mpPrev->getTimecode();
        if (0 > (spkEndTC - micStartTC)) {
#ifdef DEBUG_TC /* [ */
            if (doShowMe())  {
                osPrintf("Discard(1) 0x%X: Tm=%d, Ts=%d (delta=%d)\n",
                    (int) mpPrev, micEndTC, spkEndTC, (spkEndTC-micStartTC));
            }
#endif /* DEBUG_TC ] */
            mpPrev.release();
#ifdef DEBUG_TC /* [ */
        } else if (doShowMe())  {
            osPrintf("Keep       0x%X: Tm=%d, Ts=%d (delta=%d)\n",
            (int) mpPrev, micEndTC, spkEndTC, (spkEndTC-micStartTC));
#endif /* DEBUG_TC ] */
        }
    }
    while ((!mpPrev.isValid()) && (0 < MpMisc.pEchoQ->numMsgs())) {
        if (OS_SUCCESS == MpMisc.pEchoQ->receive((OsMsg*&) pMsg,
                                                          OsTime::NO_WAIT_TIME)) {
            mpPrev = pMsg->getBuffer();
            pMsg->releaseMsg();
            if (mpPrev == MpMisc.mpFgSilence) { // $$$ !
                mpPrev.release();
            }
            if (mpPrev.isValid()) {
                int t = mpPrev->getAttenDb();
                if (mLastSpkrAtten != t) {
                    /*osPrintf(
                        "Echo: speaker attenuation stepped from %d to %d\n",
                         mLastSpkrAtten, t);*/
                    mLastSpkrAtten = t;
                }
                spkEndTC = mpPrev->getTimecode();
#ifdef DEBUG_TC /* [ */
                if (doShowMe())  {
                    osPrintf("Receive 0x%X:    Ts=%d (delta=%d)\n",
                        (int) mpPrev, spkEndTC, (spkEndTC-micStartTC));
                }
#endif /* DEBUG_TC ] */
                long lll = micEndTC - spkEndTC;
                if (0 > (spkEndTC - micStartTC)) {
#ifdef DEBUG_TC /* [ */
                    if (doShowMe()) {
                        osPrintf("Discard(2) 0x%X: Tm=%d, Ts=%d (delta=%d)\n",
                            (int) mpPrev, micEndTC, spkEndTC,
                            (spkEndTC - micStartTC));
                    }
#endif /* DEBUG_TC ] */
                    mpPrev.release();
                } else {
                    if (iShowAudioSync) {
                        osPrintf("ShowAudioSync %ld\n", lll*8000/3686400);
                        iShowAudioSync = 0;
                    }
                }
#ifdef DEBUG_TC /* [ */
            } else if (doShowMe()) {
                osPrintf("Receive NULL!\n");
#endif /* DEBUG_TC ] */
            }
        }
    }
}





MpBufPtr MprEchoSuppress::LoudspeakerFade(MpBufPtr  in,
                                          short&  shSpkState,
                                          int  iLoudspeakerFadeDB)
{
    MpAudioSample      *shpMicSig = NULL;

/////////////////////////////////////////////////////////////////////

    if(iLoudspeakerFadeDB == 0) {
       shSpkState = MprToSpkr::ATTEN_LOUDEST; // currently 0
    }
    else if (iLoudspeakerFadeDB) {
       if (iLoudspeakerFadeDB < -6) iLoudspeakerFadeDB = -6;
       if (iLoudspeakerFadeDB > 0) iLoudspeakerFadeDB =0;
       shSpkState = iLoudspeakerFadeDB;//MprToSpkr::ATTEN_QUIETEST; // currently -6
    }

   //Now attenuate speaker data 
    
    if((mState != shSpkState) ) {
        mState = shSpkState;

//        mpSpkrPal->setAttenuation(mState);
    }

///////////////////////////////////////////////////////////////////////

    return in;
}



UtlBoolean MprEchoSuppress::doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame,
                                     int samplesPerSecond)

{
   MpAudioBufPtr        out;
   MpAudioBufPtr        in8;
   MpAudioBufPtr        in32;
   const MpAudioSample* shpMicSig;

   static int Frame10msCount;
   static short    shSpkState= MprToSpkr::ATTEN_LOUDEST;

   if ((1 > outBufsSize) || (3 < outBufsSize))
   {
      return FALSE;
   }

   in8.swap(inBufs[0]);

   if (1 < outBufsSize)
      outBufs[1] = NULL;
   if (2 < outBufsSize)
      outBufs[2] = NULL;

   if (in32 == NULL)
   {
      out = MpMisc.mpFgSilence;
      outBufs[0] = out;
      return TRUE;
   }


   /* If the object is not enabled, pass input to output */
   if (!isEnabled)
   {
      out = in8;
      outBufs[0] = out;
      return TRUE;
   }

   frame_match(in8);               /* If match not found, mpPrev == NULL */

   MpAudioBufPtr   out2;           /* To point to 8k speaker buffer */
   if(mpPrev == NULL) 
   {
      out2 = MpMisc.mpFgSilence;
   }
   else
   {
      out2.swap(mpPrev);
      outBufs[1] = out2;
   }

   if (!in8->isActiveAudio())
   {
      shpMicSig = in8->getSamplesPtr();
      MpAudioSample* shpSpkSig = out2->getSamplesWritePtr();

      if ( iSub == 0)
      {
         if (1) //speakers
         {
            mpFilterBank->DoFilterBank((MpAudioSample*)shpMicSig, shpSpkSig);
         }
         else // handset
         {

            if (1 /*HandAEC == 1*/)
            {
               mpHandsetFilterBank->DoHandsetFilterBank((MpAudioSample*)shpMicSig, (MpAudioSample*)shpMicSig, shpSpkSig);
            }

         }

      }
      ///////////////////////////////////////////////////////////////////////////
      //////////// Limiting signal //////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////////////////

      // This code is commented out, because MpCodec_isBaseSpeakerOn() always
      // return false now. And this code seems to operate on wrong array -
      // shpMicSig is const and is not used in further calculations, so
      // changing it is useless.
#if 0 // [
      if (MpCodec_isBaseSpeakerOn())
      {
         int i;
         int Temp;

         for ( i = 0; i < 80; i++)
         {
            Temp = shpMicSig[i] << 2;  // +12dB boost AFTER AEC

            if (Temp > sampleLimit)
            {
               Temp = sampleLimit;
            }
            else if (Temp < -sampleLimit)
            {
               Temp = -sampleLimit;
            }

            shpMicSig[i] = Temp;
         }
      }
#endif // 0 ]

      ////////////////////////////////////////////////////////////////////////////

      {
         int FadeDB = MpCodec_isBaseSpeakerOn() ?
                         iLoudspeakerFadeDB : //speakerphone 
                         0;                   //never fade earpiece
         out = LoudspeakerFade(in8, shSpkState, FadeDB);
      }

   }

   if (!out.isValid()) {
      out = MpMisc.mpFgSilence;
   }

   *outBufs = out;
   return TRUE;
}


/* ============================ FUNCTIONS ================================= */


#if 0 /* [ */
   //////////////////////////////////////////////////////////////////////
   // AGC processing block, done inline for the
   // sake of experiments now, later should be made a C++ function.
   //////////////////////////////////////////////////////////////////////

   // Variables

   static int  iTrigger = 7000;        // Max allowable sample value, if it is over, trigger AGC
   static int  iBlockSize = iLength;   // Processing block size, iLength = 80 samples
   static int  iNormGain = 3;          // Normal processing gain of AGC block, 3 is ~10dB
   static int  iLossThisFrame;         // Additional multiplier, reducing the gain to 
                                       // avoid clipping
   static int  iLossPrevFrame;         // Additional multiplier, reducing the gain to 
                                       // avoid clipping
   static int iLoss;
   static int iFrame = 0;              // Frame counter

   int      iOverloadRate;             // Number of samples in a sample block over iTrigger
   static int   iAttackNumSamples;     // Number of samples to ramp the gain to new value  
   static int   iDecayNumSamples;      // Number of samples to ramp the gain back to Nominal

   // Measure the frequency of overload condition. Look at iBlockSize samples
   // and determine how many are over iTrigger.

   iOverloadRate = 0; // no overload at the beginning of the frame

   for (i = 0; i < iLength; i++)
   {
      if (shpSamples[i] > iTrigger)
      {
         iOverloadRate++;
      }
   }

   // Set the desired amount of loss in dB based on severity of overload
   if      (iOverloadRate == 0) iLossThisFrame = 16;
   else if (iOverloadRate > 0 && iOverloadRate < 4) iLossThisFrame = 14;
   else if (iOverloadRate >= 4 && iOverloadRate < 8) iLossThisFrame = 12;
   else if (iOverloadRate >= 8 && iOverloadRate < 12) iLossThisFrame = 10;
   else if (iOverloadRate >= 12 && iOverloadRate < 16) iLossThisFrame = 8;
   else if (iOverloadRate >= 16 && iOverloadRate < 20) iLossThisFrame = 6;
   else if (iOverloadRate >= 20 && iOverloadRate < 30) iLossThisFrame = 4;
   else if (iOverloadRate >= 30 && iOverloadRate < 40) iLossThisFrame = 3;

   // Apply the loss factor to the entire frame at once
   if (iLossThisFrame < iLossPrevFrame)
   {
      iLossPrevFrame = iLossThisFrame;
      iFrame = 0;
   }

   if (iOverloadRate > 0)
   {
      
      iFrame ++;

      for (i = 0; i < iLength; i++)
      {
         //When there is no clipping the gain is 16/4 (12dB)
         shpSamples[i] = ((int32_t)shpSamples[i] * iLoss) >> 2; 
         
      }

   }

   

   // Printout 
   
#endif /* ] */

#endif /* __pingtel_on_posix ] */

//////////////////////////// End of AGC ///////////////////////////////
