//  
// Copyright (C) 2005-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _INCLUDED_MPCODEC_H /* [ */
#define _INCLUDED_MPCODEC_H

#include <os/OsStatus.h>
#include <utl/UtlBool.h>
/*************************************************************************/

/* MCP/Codec interface: */

typedef enum {
    CODEC_DISABLE_SPKR=0,
    CODEC_ENABLE_SPKR1=(1<<0),
    CODEC_ENABLE_SPKR2=(1<<1),
    CODEC_ENABLE_SPKR3=(1<<2),
    CODEC_ENABLE_SPKR4=(1<<3),
    CODEC_ENABLE_SPKR5=(1<<4),
    CODEC_ENABLE_SPKR6=(1<<5)
} MpCodecSpkrChoice;

#define CODEC_ENABLE_HANDSET_SPKR         CODEC_ENABLE_SPKR1
#define CODEC_ENABLE_BASE_SPKR            CODEC_ENABLE_SPKR2
#define CODEC_ENABLE_HEADSET_SPKR         CODEC_ENABLE_SPKR3
#define CODEC_ENABLE_EXTERNAL_SPKR_MONO   CODEC_ENABLE_SPKR4
#define CODEC_ENABLE_EXTERNAL_SPKR_STEREO CODEC_ENABLE_SPKR5
#define CODEC_ENABLE_EXTERNAL_SPKR    (CODEC_ENABLE_SPKR4 | CODEC_ENABLE_SPKR5)
#define CODEC_ENABLE_RINGER_SPKR          CODEC_ENABLE_SPKR6

#define START_GAIN 25
#define START_VOLUME 60
#define MIC_GAIN_MIN 0    ///< Minimal microphone gain. This gain will mute mic.
#define MIC_GAIN_MAX 100  ///< Maximal microphone gain.

extern OsStatus MpCodecOpen(int sampleRate, int gain, int volume);

extern UtlBoolean MpCodec_isBaseSpeakerOn(void); ///< the speakerphone speaker
extern UtlBoolean MpCodec_isHeadsetSpeakerOn(void); ///< the headset speaker
extern UtlBoolean MpCodec_isHandsetSpeakerOn(void); ///< the handset speaker

extern OsStatus MpCodec_getVolumeRange(
                      int& low,         ///< lowest value
                      int& high,        ///< highest value
                      int& nominal,     ///< low <= nominal <= high
                      int& stepsize,    ///< in .1 dB
                      int& mute,        ///< input value to mute
                      int& splash,      ///< value to use during startup
                      MpCodecSpkrChoice Choice);

extern int MpCodec_getInputMixerId(void);
extern int MpCodec_getGain(void);
extern int MpCodec_getVolume(void);
extern OsStatus MpCodec_setInputMixerId(unsigned int newmixerid);
extern OsStatus MpCodec_setGain(int newgain);
extern OsStatus MpCodec_setVolume(int newvolume);

#endif /* _INCLUDED_MPCODEC_H ] */
