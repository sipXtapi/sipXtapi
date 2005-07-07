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

#ifndef _INCLUDED_MPMISC_H /* [ */
#define _INCLUDED_MPMISC_H

// Forward declarations
class OsConfigDb;

#ifdef _VXWORKS /* [ */

// The option FLOWGRAPH_DOES_RESAMPLING controls whether the conversion
//     between 8KHz and 32KHz sample rate is done in dmaTask.cpp (the old,
//     less correct way) or in the flowgraph resources (the new, more
//     proper way, using a 4KHz low pass filter).

// The option INJECT_32K_TO_SPEAKER controls the inclusion of the debugging
//     feature that allows raw 32KHz sample rate files to be substituted in
//     the MprToSpkr resource.  This requires FLOWGRAPH_DOES_RESAMPLING to
//     be turned on.

#  define FLOWGRAPH_DOES_RESAMPLING
#  undef FLOWGRAPH_DOES_RESAMPLING
#  define FLOWGRAPH_DOES_RESAMPLING
#  undef INJECT_32K_TO_SPEAKER
#  ifdef FLOWGRAPH_DOES_RESAMPLING /* [ */
#    define INJECT_32K_TO_SPEAKER
#    undef INJECT_32K_TO_SPEAKER
#  endif /* FLOWGRAPH_DOES_RESAMPLING ] */
#endif /* _VXWORKS ] */

#ifdef _VXWORKS /* [ */
#include <msgQLib.h>
#endif /* _VXWORKS ] */

#include "os/OsMsgQ.h"
#include "mp/MpTypes.h"

#define KBHIT
#undef  KBHIT

#ifndef MUTEX_OPS
#define MUTEX_OPS (SEM_Q_PRIORITY|SEM_DELETE_SAFE|SEM_INVERSION_SAFE)
#endif

#define FRAME_SAMPS  320

#ifndef max
#define max(x,y) (((x)>(y))?(x):(y))
#endif

#ifndef min
#define min(x,y) (((x)<(y))?(x):(y))
#endif

/* miscellaneous debugging support: */

#ifdef _VXWORKS /* [ */
#ifdef  KBHIT /* [ */
extern int kbhit(void);
#endif  /* KBHIT ] */

/* Nprintf() is a former Lprintf() or Zprintf(), left around in case
 * it may be useful again in some future debugging endeavor */
#define Nprintf(fmt, a, b, c, d, e, f)

/* Zprintf() is something to be printed out always */
#define Zprintf(fmt, a, b, c, d, e, f) Zprintfx(1, fmt, a, b, c, d, e, f)

/* Lprintf() is something to be printed out only during verbose logging */
#ifdef LOGEM
#define Lprintf(fmt, a, b, c, d, e, f) Zprintfx(0, fmt, a, b, c, d, e, f)
#else
#define Lprintf(fmt, a, b, c, d, e, f)
#endif

extern int NoPrintLog(void);
extern int PrintLog(int num = 0);

extern MSG_Q_ID startLogging(int nmsgs, int maxlen);
extern int Zprintfx(int z, /* 1 means always, 0 means if room in queue */
            char *fmt, /* format string */
            int a, int b, int c, /* 6 required integer arguments */
            int d, int e, int f);

extern int drain_log(int level /* how many to leave */);
extern int StartLogging();
extern int StopLogging(int opt);
#else /* _VXWORKS ] [ */
#define Zprintf printf
#undef Zprintf
#define Zprintf(fmt, a, b, c, d, e, f)
#define Nprintf(fmt, a, b, c, d, e, f)
#define Lprintf(fmt, a, b, c, d, e, f)
#endif /* _VXWORKS ] */

/* mpStartUp initializes the MpMisc struct and other MP data, for */
/*    example, the buffer pools and tables */
extern OsStatus mpStartUp(int sampleRate, int samplesPerFrame,
		  int numAudioBuffers, OsConfigDb* pConfigDb);

/* mpStartTasks spawns the low level MP tasks for DMA and network I/O */
extern OsStatus mpStartTasks(void);

struct __MpGlobals {
        OsMsgQ* pMicQ;
        OsMsgQ* pSpkQ;
        OsMsgQ* pEchoQ;
        int micMuteStatus;
        int spkrMuteStatus;
        int audio_on;
        int frameSamples;
        int frameBytes;
        int sampleBytes;
        int rtpMaxBytes;
        MpBufPoolPtr UcbPool;
        MpBufPoolPtr DMAPool;
        MpBufPoolPtr RtpPool;
        MpBufPoolPtr RtcpPool;
        MpBufPtr XXXsilence;
        MpBufPtr XXXlongSilence;
        MpBufPtr comfortNoise;
#ifdef _VXWORKS /* [ */
        int mem_page_size;
        int mem_page_mask;
        MSG_Q_ID LogQ;
        int logMsgLimit;
        int logMsgSize;
#endif /* _VXWORKS ] */
        int max_mic_buffers;
        int max_spkr_buffers;
        int min_rtp_packets;
        int micSawTooth;

#ifdef _VXWORKS /* [ */
        int doLoopBack;
        OsMsgQ* pLoopBackQ;
#endif /* _VXWORKS ] */

};

extern struct __MpGlobals MpMisc;

#endif /* _INCLUDED_MPMISC_H ] */
