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


#ifndef _INCLUDED_DTMFLIB_H /* [ */
#define _INCLUDED_DTMFLIB_H

#include "mp/MpTypes.h"
#include "os/OsStatus.h"

#define DTMF_TONES_BASE 512

typedef enum {
DTMF_TONE_DIALTONE   = (DTMF_TONES_BASE + 0),
DTMF_TONE_BUSY,
DTMF_TONE_RINGBACK,
DTMF_TONE_RINGTONE,
DTMF_TONE_CALLFAILED,
DTMF_TONE_SILENCE,
DTMF_TONE_BACKSPACE,
DTMF_TONE_CALLWAITING,
DTMF_TONE_CALLHELD,
DTMF_TONE_LOUD_FAST_BUSY
} toneID;

typedef struct __MpToneGen_tag *MpToneGenPtr;

extern void MpToneGen_startTone(MpToneGenPtr p, int toneID);
extern void MpToneGen_stopTone(MpToneGenPtr p);
extern OsStatus MpToneGen_getNextBuff(MpToneGenPtr thisobj, MpAudioSample *b, int N);
extern void MpToneGen_delete(MpToneGenPtr p);
extern MpToneGenPtr MpToneGen_MpToneGen(int samprate, const char* toneLocale=NULL);

#endif /* _INCLUDED_DTMFLIB_H ] */
