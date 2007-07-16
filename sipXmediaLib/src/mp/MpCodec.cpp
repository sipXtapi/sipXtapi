// 
// 
// Copyright (C) 2005-2006 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004-2006 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#include "mp/MpCodec.h"

#define DEF_HSVOLMAX 54
#define DEF_HSVOLSTEP 2
/// default value for input mixer = -1 -> not detected yet
#define DEF_INPUTMIXER -1
static int hsVolMax = DEF_HSVOLMAX;
static int hsVolStep = DEF_HSVOLSTEP;

/**
*  ID of the mixer containing microphone line
*/
static int sInputMixerId = DEF_INPUTMIXER;

/**
*  @file MpCodec.cpp
*
*  @TODO The rest of this file is really two independent platform-specific
*  implementations of the MpCodec module.  These should be broken out into
*  separate files.
*/

#if defined(_WIN32) /* [ */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <MMSystem.h>


#include "os/OsUtil.h"
#include "mp/dmaTask.h"
#include "mp/MpCodec.h"

extern "C" void setHandsetMuteState(UtlBoolean state);
static int s_iGainLevel = 5 ;

/**
*  Gets the ID of the mixer containing microphone line.
*  
*  @return ID of mixer
*
*  @see MpCodec_setInputMixerId
*  @see MpCodec_initInputMixerId
*/
int MpCodec_getInputMixerId(void)
{
   return sInputMixerId;
}

/**
*  Sets the ID of the mixer containing microphone line. For systems
*  with multiple mixer devices or multiple sound cards we need to
*  know the ID of the right mixer device so that we set gain on
*  the right one.
*  
*  @param newMixerId ID of the mixer containing microphone line
*  @return result of setting mixer ID
*
*  @see MpCodec_initInputMixerId
*/
OsStatus MpCodec_setInputMixerId(unsigned int newMixerId)
{
   unsigned int devices = mixerGetNumDevs();

   // to prevent setting invalid mixer id
   if (newMixerId < devices)
   {
      sInputMixerId = newMixerId;
      return OS_SUCCESS;
   }
   else return OS_INVALID_ARGUMENT;
}

/**
*  Finds ID of a mixer of the first wave input device present in
*  system. A sound card can have separate mixer for output devices
*  and another one for input devices. If we have multiple sound cards
*  then we will have multiple output and input devices with several
*  mixers. This function is executed only if we haven't opened a
*  wave input device as a fail-safe. First input device in the system
*  is used to detect the mixer ID. Here we don't know yet which wave input
*  device will be used so we have to make this simplification. Once
*  wave input device is opened, we detect the mixer Id associated with it
*  elsewhere. This ID is only used for setting gain.
*  
*  @return result of detection of mixer ID
*
*  @see detectInputMixerId
*  @see openAudioIn
*/
OsStatus MpCodec_initInputMixerId(void)
{
   OsStatus ret = OS_UNSPECIFIED;
   MMRESULT mmresult;
   int uDeviceID = 0;
   unsigned int uMxId;

   // Mixer ID is also detected in MicThreadWnt.cpp, this one doesn't need
   // wave input handle, but assumes we use wave input device 0
   mmresult = mixerGetID((HMIXEROBJ)uDeviceID, &uMxId, MIXER_OBJECTF_WAVEIN);

   if (mmresult == MMSYSERR_NOERROR)
   {
      ret = MpCodec_setInputMixerId(uMxId);
   }
   
   return ret;
}

OsStatus MpCodec_setGain(int level)
{
   s_iGainLevel = level ;
   OsStatus ret = OS_UNSPECIFIED;
   MMRESULT mmresult;

   // mixer id is not yet available, try to detect it by assuming we use
   // wave input device 0
   if (sInputMixerId == -1)
   {
      if (MpCodec_initInputMixerId() != OS_SUCCESS)
      {
         return ret;
      }
   }

   // Open the mixer device
   HMIXER hmx;
   mmresult = mixerOpen(&hmx, sInputMixerId, 0, 0, MIXER_OBJECTF_MIXER );

   if (MMSYSERR_NOERROR == mmresult)
   {
       // Get the line info for the wave in destination line
       MIXERLINE mxl;
       memset(&mxl, 0, sizeof(MIXERLINE));
       mxl.cbStruct = sizeof(mxl);
       mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;
       mmresult = mixerGetLineInfo((HMIXEROBJ)hmx, &mxl, MIXER_GETLINEINFOF_COMPONENTTYPE);

       if (MMSYSERR_NOERROR == mmresult)
       {
           // Now find the microphone source line connected to this wave in
           // destination
           DWORD cConnections = mxl.cConnections;

           for(DWORD j=0; j<cConnections; j++)
           {
              mxl.dwSource = j;
              mixerGetLineInfo((HMIXEROBJ)hmx, &mxl, MIXER_GETLINEINFOF_SOURCE);
              if (MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE == mxl.dwComponentType)
                 break;
           }
           
           // Find a volume control, if any, of the microphone line
           LPMIXERCONTROL pmxctrl = (LPMIXERCONTROL)calloc(1, sizeof(MIXERCONTROL));
           MIXERLINECONTROLS mxlctrl = {sizeof(mxlctrl), mxl.dwLineID, MIXERCONTROL_CONTROLTYPE_VOLUME, 1, sizeof(MIXERCONTROL), pmxctrl};
           
           if(!mixerGetLineControls((HMIXEROBJ) hmx, &mxlctrl, MIXER_GETLINECONTROLSF_ONEBYTYPE))
           {
              // Found!
              DWORD cChannels = mxl.cChannels;
              if (MIXERCONTROL_CONTROLF_UNIFORM & pmxctrl->fdwControl)
                 cChannels = 1;

              LPMIXERCONTROLDETAILS_UNSIGNED pUnsigned =
              (LPMIXERCONTROLDETAILS_UNSIGNED)  malloc(cChannels * sizeof(MIXERCONTROLDETAILS_UNSIGNED));
              
              MIXERCONTROLDETAILS mxcd = {sizeof(mxcd), pmxctrl->dwControlID, cChannels, (HWND)0, 
                 sizeof(MIXERCONTROLDETAILS_UNSIGNED), (LPVOID) pUnsigned};
              mixerGetControlDetails((HMIXEROBJ)hmx, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE);

              // Set the volume to the the level  (for both channels as needed)
              pUnsigned[0].dwValue 
                 = pUnsigned[cChannels - 1].dwValue
                 = pmxctrl->Bounds.dwMinimum
                   + (DWORD)((double)(level - MIC_GAIN_MIN) / (MIC_GAIN_MAX - MIC_GAIN_MIN)
                     * (pmxctrl->Bounds.dwMaximum - pmxctrl->Bounds.dwMinimum));
              mixerSetControlDetails((HMIXEROBJ)hmx, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE);

              free(pmxctrl);
              free(pUnsigned);
           }
           else
           {
              free(pmxctrl);
           }

           // mute if Mic level is less then 1
           DmaTask::setMuteEnabled(s_iGainLevel <= 1);
           ret = OS_SUCCESS;
       }
       mixerClose(hmx);
   }

   return ret;
}


int MpCodec_getGain()
{      
   return s_iGainLevel;
}


extern HWAVEOUT audioOutH;
extern HWAVEOUT audioOutCallH;

//setVolume for Win32
OsStatus MpCodec_setVolume(int level)
{
   OsStatus ret = OS_INVALID;
   
   //scale to our max value
   unsigned short volSeg = 0xFFFF/100;
   unsigned short newLeftVal = level * volSeg;
   unsigned short newRightVal = newLeftVal;
   
   //now set this to the left and right speakers
   DWORD bothVolume = (newRightVal << 16) + newLeftVal ;

   HWAVEOUT hOut = NULL ;
   if (DmaTask::isRingerEnabled())
   {
      hOut = audioOutH ;
   }
   else
   {
      hOut = audioOutCallH ;
   }

   
   if (hOut != NULL)
   {
      if (waveOutSetVolume(hOut, bothVolume) == MMSYSERR_NOERROR )
        ret = OS_SUCCESS;
      else
        ret = OS_FAILED;
   }

   return ret;
}

int MpCodec_getVolume() 
{
   DWORD bothVolume;
   int volume=0;
   unsigned short volSeg = 0xFFFF/100;

   HWAVEOUT hOut = NULL ;
   if (DmaTask::isRingerEnabled())
   {
      hOut = audioOutH ;
   }
   else
   {
      hOut = audioOutCallH ;
   }

   
   if (hOut != NULL)
   {
      MMRESULT mmret;
      mmret = waveOutGetVolume(hOut, &bothVolume);
      if (mmret == MMSYSERR_NOERROR )
      {
          //mask out one
          unsigned short rightChannel = ((unsigned short) bothVolume & 0xFFFF) ;
          volume = rightChannel /volSeg;         
      } else
          volume = -1;
   }

   return volume;
}


/* dummy routine */
int MpCodec_isSpeakerOn() { return 0;}

#elif defined(__pingtel_on_posix__) /* WIN32 ] [ */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "os/OsUtil.h"
#include "mp/dmaTask.h"
#include "mp/MpCodec.h"

/* Variable to save the gain level that has been set.
 * This is only significant in that level 0 means "muted" and level > 0 means
 * "not muted", but our callers expect that MpCodec_getGain() will return the
 * value that was set with MpCodec_setGain().
 * The initial value of 1 is an arbitrary number > 0.
 */
static int s_iGainLevel = 1;

/* FUNCTION IS CALLED ONLY BY SOFTWARE PHONE */
OsStatus MpCodec_setGain(int level)
{
   OsStatus ret = OS_UNSPECIFIED;
   
   osPrintf("MpCodec_setGain (softphone version) GAIN: %d\n",level);

   DmaTask::setMuteEnabled(level == 0);
   /* Save level so it can be returned by MpCodec_getGain(). */
   s_iGainLevel = level;
   ret = OS_SUCCESS;

   return ret;
}

int MpCodec_getGain()
{      
   return s_iGainLevel;
}

static int dummy_volume = 0;

OsStatus MpCodec_setVolume(int level)
{
   OsStatus ret = OS_SUCCESS;
   dummy_volume = level;
   return ret;
}

int MpCodec_getVolume()
{
   return dummy_volume;
}

int MpCodec_isBaseSpeakerOn()
{
   return 0;
}

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

