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

#include "mp/MpCodec.h"

#define DEF_HSVOLMAX 54
#define DEF_HSVOLSTEP 2
static int hsVolMax = DEF_HSVOLMAX;
static int hsVolStep = DEF_HSVOLSTEP;

extern bool MpDma_setMuteEnabled(bool enabled);

// TODO
// The rest of this file is really two independent platform-specific
// implementations of the MpCodec module.  These should be broken out into
// separate files.

#if defined(_WIN32) /* [ */

#include <stdio.h>
#include <string.h>
#include <assert.h>


#include "os/OsUtil.h"
#include "mp/dmaTask.h"
#include "mp/MpCodec.h"

extern "C" void setHandsetMuteState(UtlBoolean state);

/* FUNCTION IS CALLED ONLY BY SOFTWARE PHONE */
OsStatus MpCodec_setGain(int level)
{
   OsStatus ret = OS_UNSPECIFIED;
   
   osPrintf("MpCodec_setGain (softphone version) GAIN: %d\n",level);

   MpDma_setMuteEnabled(level == 0);
   ret = OS_SUCCESS;

   return ret;
}


extern HWAVEOUT audioOutH;

//setVolume for Win32
OsStatus MpCodec_setVolume(int level)
{
   OsStatus ret = OS_INVALID;
   osPrintf("MpCodec_setVolume: %d\n",level);
   //scale to our max value
   unsigned short volSeg = 0xFFFF/100;
   unsigned short newLeftVal = level * volSeg;
   unsigned short newRightVal = newLeftVal;

   osPrintf("MpCodec_setVolume (windows channel volume): %d\n",newLeftVal);

   //now set this to the left and right speakers
   DWORD bothVolume = (newRightVal << 16) + newLeftVal ;
   
   if (audioOutH != NULL)
   {
      waveOutSetVolume(audioOutH, bothVolume) ;
      ret = OS_SUCCESS;
   }

   return ret;
}

int MpCodec_getVolume() 
{
   DWORD bothVolume;
   int volume;
   unsigned short volSeg = 0xFFFF/100;

   waveOutGetVolume(audioOutH, &bothVolume) ;
   //mask out one
   unsigned short rightChannel = (unsigned short) (bothVolume & 0xFFFF);
   volume = rightChannel /volSeg;
   
   osPrintf("MpCodec_getVolume : %d\n",volume);

   return volume;
}

/* dummy routine */
int MpCodec_isSpeakerOn() { return 0;}

/* dummy routine */
OsStatus MpCodec_doProcessFrame() { return OS_SUCCESS;}

#elif defined(__pingtel_on_posix__) /* WIN32 ] [ */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "os/OsUtil.h"
#include "mp/dmaTask.h"
#include "mp/MpCodec.h"

/* FUNCTION IS CALLED ONLY BY SOFTWARE PHONE */
OsStatus MpCodec_setGain(int level)
{
   OsStatus ret = OS_UNSPECIFIED;
   
   osPrintf("MpCodec_setGain (softphone version) GAIN: %d\n",level);

   MpDma_setMuteEnabled(level == 0);
   ret = OS_SUCCESS;

   return ret;
}



OsStatus MpCodec_setVolume(int level)
{
   OsStatus ret = OS_SUCCESS;
   return ret;
}


/* dummy routine */
OsStatus MpCodec_doProcessFrame() { return OS_SUCCESS;}


#endif /* ] */

OsStatus MpCodec_getVolumeRange( int& low, int& high, int& nominal, 
            int& stepsize, int& mute, int& splash, MpCodecSpkrChoice mask)
{
    OsStatus ret = OS_INVALID;
    int stepsPerInc = 2;
    int nomStep = 5;
    mute = -1;

    stepsize = 10;
    splash = -1;
    switch (OsUtil::getPlatformType()) {
    case OsUtil::PLATFORM_TCAS1:
    case OsUtil::PLATFORM_TCAS2:
        low = high = nominal = stepsize = mute = 0;
        ret = OS_INVALID;
        break;
    case OsUtil::PLATFORM_TCAS3:
    case OsUtil::PLATFORM_TCAS4:
        if((mask & CODEC_ENABLE_HANDSET_SPKR)||
           (mask & CODEC_ENABLE_HEADSET_SPKR)) {
            high     = 50;
            low      = 20;
            nominal  = 35;
            mute     = -1;
        }
        else if (mask&CODEC_ENABLE_BASE_SPKR) {
            high     = 55;
            low      = 35;
            nominal  = 45;
            splash   = 35;
            mute     = -1;
        }
        else if (mask&CODEC_ENABLE_RINGER_SPKR) {
            high     = 55;
            low      = 35;
            nominal  = 45;
            splash   = 35;
            mute     = 15;
        }
        else {
            high     = 50;
            low      = 20;
            nominal  = 35;
            mute     = -1;
        }
        ret = OS_SUCCESS;
        break;
    case OsUtil::PLATFORM_TCAS5:
    case OsUtil::PLATFORM_WIN32:
        if (mask & CODEC_ENABLE_HANDSET_SPKR) {
            // Revised 6Aug01 for hearing aid compatibility, per Mark G.
            high     = hsVolMax; 
            stepsPerInc = hsVolStep;
        }
        else if((mask & CODEC_ENABLE_HEADSET_SPKR)) {
            high     = 40;
        }
        else if((mask&CODEC_ENABLE_BASE_SPKR)) {
            high     = 50;
            splash   = 32;
        }
        else if((mask&CODEC_ENABLE_RINGER_SPKR)) {
            stepsPerInc = 3;
            nomStep  = 5;
            high     = 59;
            splash   = 32;
            mute     = 24;
        }
        else {
            high     = 50;
        }
        nominal  = high - ((10 - nomStep) * stepsPerInc);
        low      = high - 10 * stepsPerInc;
        ret = OS_SUCCESS;
        break;
 
    case OsUtil::PLATFORM_TCAS6:
        if (mask & CODEC_ENABLE_HANDSET_SPKR) {
            // Revised 6Aug01 for hearing aid compatibility, per Mark G.
            high     = hsVolMax - 3; // -3dB compensate for +6dB codec output boost,
                                     // which is only needed for base speaker.
            stepsPerInc = hsVolStep;
        }
        else if((mask & CODEC_ENABLE_HEADSET_SPKR)) {
            high     = 40;
        }
        else if((mask&CODEC_ENABLE_BASE_SPKR)) {
            high     = 50;
            splash   = 32;
        }
        else if((mask&CODEC_ENABLE_RINGER_SPKR)) {
            stepsPerInc = 3;
            nomStep  = 5;
            high     = 59;
            splash   = 32;
            mute     = 24;
        }
        else {
            high     = 50;
        }
        nominal  = high - ((10 - nomStep) * stepsPerInc);
        low      = high - 10 * stepsPerInc;
        ret = OS_SUCCESS;
        break;
    case OsUtil::PLATFORM_TCAS7:
    default:
        if (mask & CODEC_ENABLE_HANDSET_SPKR) {
            // Revised 6Aug01 for hearing aid compatibility, per Mark G.
            high     = hsVolMax - 3;   // -3dB compensate for +6dB codec output boost,
                                       // which is only needed for base speaker.
            stepsPerInc = hsVolStep;
        }
        else if((mask & CODEC_ENABLE_HEADSET_SPKR)) {
            high     = 40;
        }
        else if((mask&CODEC_ENABLE_BASE_SPKR)) {
            high     = 50;
            splash   = 32;
        }
        else if((mask&CODEC_ENABLE_RINGER_SPKR)) {
            stepsPerInc = 3;
            nomStep  = 5;
            high     = 59;
            splash   = 32;
            mute     = 24;
        }
        else {
            high     = 50;
        }
        nominal  = high - ((10 - nomStep) * stepsPerInc);
        low      = high - 10 * stepsPerInc;
        ret = OS_SUCCESS;
        break;
     }
    if (-1 == splash) splash = nominal;
#ifdef DEBUG_MPCODEC /* [ */
    osPrintf(
        "MpCodec_getVolumeRange for 0x%02X: %d, %d, %d, %d, %d, %d; ret %d\n",
        mask, low, high, nominal, stepsize, mute, splash);
#endif /* DEBUG_MPCODEC ] */
    return ret;
}

