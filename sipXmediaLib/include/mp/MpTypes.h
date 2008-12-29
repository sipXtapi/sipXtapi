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


#ifndef _INCLUDED_MPTYPES_H /* [ */
#define _INCLUDED_MPTYPES_H

#include <utl/UtlDefs.h>
#include <os/OsIntTypes.h>

/**************************************************************************/
/* One of these should be defined, and the other should undefined!!!!!    */
/* These are used to determine whether 16 bit samples need to be byte     */
/* swapped.  All hosts (SA1100 and WinTel) are currently little endian    */
#ifdef _VXWORKS /* [ */
#undef  _BYTE_ORDER_IS_LITTLE_ENDIAN
#undef  _BYTE_ORDER_IS_BIG_ENDIAN
#define _BYTE_ORDER_IS_LITTLE_ENDIAN
#else /* _VXWORKS ] [ */
#undef  _BYTE_ORDER_IS_LITTLE_ENDIAN
#undef  _BYTE_ORDER_IS_BIG_ENDIAN
#define _BYTE_ORDER_IS_LITTLE_ENDIAN
#endif /* _VXWORKS ] */
/*/////////////////////////////////////////////////////////////////////// */

#ifdef _VXWORKS /* [ */
#include "vxWorks.h"
#else /* _VXWORKS ] [ */
typedef int STATUS;
typedef void * MSG_Q_ID;
typedef void * SEM_ID;
#endif /* _VXWORKS ] */

typedef int16_t MpAudioSample;    ///< Audio sample (16 bit, signed, little-endian)
/// Size of audio sample in bits.
#define MP_AUDIO_SAMPLE_SIZE 16

typedef int MpInputDeviceHandle;  ///< Logical device ID identifying device
                                  ///< driver inside device manager.
typedef int MpOutputDeviceHandle; ///< Logical device ID identifying device
                                  ///< driver inside device manager.
#define MP_INVALID_OUTPUT_DEVICE_HANDLE -1
typedef uint32_t MpFrameTime;    ///< Time of frame begin relative to device
                                 ///< manager startup (in milliseconds).

typedef int MpConnectionID;      ///< Flowgraph connection ID.
#define MP_INVALID_CONNECTION_ID  -1

typedef uint16_t RtpSeq;        ///< RTP sequence number
typedef uint32_t RtpTimestamp;  ///< RTP timestamp
typedef uint32_t RtpSRC;        ///< RTP SSRC or CSRC identifier


/// Type of audio data.
typedef enum {
   MP_SPEECH_UNKNOWN,        ///< is yet undetermined
   MP_SPEECH_SILENT,         ///< found to contain no speech
   MP_SPEECH_COMFORT_NOISE,  ///< to be replaced by comfort noise
   MP_SPEECH_ACTIVE,         ///< found to contain speech
   MP_SPEECH_MUTED,          ///< may contain speech, but must be muted
   MP_SPEECH_TONE            ///< filled with active (not silent) tone data
} MpSpeechType;

struct MpSpeechParams
{
#ifdef __cplusplus
   typedef enum {
      MAX_AMPLITUDE = INT16_MAX ///< Maximum possible amplitude.
   } MpSpeechParamsConsts;

     /// Constructor
   MpSpeechParams()
   : mSpeechType(MP_SPEECH_UNKNOWN)
   , mAmplitude(MAX_AMPLITUDE)
   , mIsClipped(FALSE)
   , mFrameEnergy(-1)
   , mSpeakerRank(0)
   {
   };
#else
#  define MAX_AMPLITUDE INT16_MAX ///< Maximum possible amplitude.
#endif

   MpSpeechType mSpeechType;  ///< Whether buffer contains speech.
   MpAudioSample mAmplitude;  ///< Amplitude of the audio data in this buffer.
   UtlBoolean mIsClipped;     ///< Is data in this buffer clipped or not?
   int mFrameEnergy;          ///< Current frame energy (normalized to frame
                              ///< length in ms).
   unsigned mSpeakerRank;     ///< Speaker selection rank, 0 is the highest
                              ///< priority, UINT_MAX is for silent participants.
};

/* ============================== FUNCTIONS ============================== */

/// Does this speech type contain active audio or silence.
static inline
UtlBoolean isActiveAudio(MpSpeechType speechType);

/// Determine type of speech after mixing two frames of given type.
MpSpeechType mixSpeechTypes(MpSpeechType src1, MpSpeechType src2);

/* =========================== INLINE FUNCTIONS =========================== */

UtlBoolean isActiveAudio(MpSpeechType speechType)
{
   switch (speechType) {
   case MP_SPEECH_SILENT:
   case MP_SPEECH_COMFORT_NOISE:
   case MP_SPEECH_MUTED:
      return FALSE;
   case MP_SPEECH_UNKNOWN:
   case MP_SPEECH_ACTIVE:
   case MP_SPEECH_TONE:
   default:
      return TRUE;
   };
}

#endif /* _INCLUDED_MPTYPES_H ] */
