//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#include "os/OsDefs.h"
#include "mp/mpau.h"
#include "mp/MpAudioFileOpen.h"
#include "mp/MpAudioWaveFileRead.h"  // Microsoft RIFF WAVE

// This function just returns a container and basic information
MpAudioAbstract *MpOpenFormat(istream &file) 
{
   MpAudioAbstract *pAudio = NULL;

   if (isWaveFile(file)) 
   {
      file.seekg(0);
      pAudio = new MpAudioWaveFileRead(file);
      
      if (pAudio)
        pAudio->setAudioFormat(AUDIO_FORMAT_WAV);
   }
   else
   if (isAuFile(file)) 
   {
      file.seekg(0);
      pAudio = new MpAuRead(file);
      if (pAudio)
        pAudio->setAudioFormat(AUDIO_FORMAT_AU);
   }

   return pAudio;
}
