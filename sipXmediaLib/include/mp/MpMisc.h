//  
// Copyright (C) 2006-2012 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _INCLUDED_MPMISC_H /* [ */
#define _INCLUDED_MPMISC_H

// Forward declarations
class OsConfigDb;

#include "os/OsMsgQ.h"
#include "mp/MpTypes.h"
#include "mp/MpBufPool.h"
#include "mp/MpAudioBuf.h"

/* miscellaneous debugging support: */

#define Zprintf printf
#undef Zprintf
#define Zprintf(fmt, a, b, c, d, e, f)
#define Nprintf(fmt, a, b, c, d, e, f)
#define Lprintf(fmt, a, b, c, d, e, f)

extern MpBufPool** sMpBufPools[];

extern int sMpNumBufPools;

/* mpStartUp initializes the MpMisc struct and other MP data, for */
/*    example, the buffer pools and tables */
extern OsStatus mpStartUp(int sampleRate, int samplesPerFrame,
                          int numAudioBuffers, OsConfigDb* pConfigDb,
                          const size_t numCodecPaths, const UtlString codecPaths[]);

/* tears down whatever was created in mpStartUp */
extern OsStatus mpShutdown(void);

/* mpStartTasks spawns the low level MP tasks for DMA and network I/O */
extern OsStatus mpStartTasks(void);

/* mpStopTasks stops the low level MP tasks */
extern OsStatus mpStopTasks(void);

/* Log to syslog buffer statistics with the given log label */
void mpLogBufferStats(const char* label);

/* Get buffer pool statistics */
/* void mpGetBufferPoolStats(int numPools, UtlString poolNames[], int freeCount, int totalCount[]); */

/// This structure contain all static variables
struct __MpGlobals {
        OsMsgQ* pMicQ;          ///< Message queue for microphone data
        OsMsgQ* pSpkQ;          ///< Message queue for speaker data
        OsMsgQ* pEchoQ;         ///< Message queue for echo cancelation data
                                ///<  (it is copy of speaker data).
        int frameSamples;       ///< Number of samples in one audio frame
        int frameBytes;         ///< Size of one audio frame 
        int sampleBytes;        ///< Size of one audio sample (in bytes)
        int rtpMaxBytes;        ///< Maximum bytes in an RTP packet
        MpBufPool *RawAudioPool;     ///< Memory pool for raw audio data buffers
        MpBufPool *AudioHeadersPool; ///< Memory pool for headers of raw audio
                                     ///<  data buffers
        MpBufPool *RtpPool;          ///< Memory pool for RTP data buffers
        MpBufPool *RtcpPool;         ///< Memory pool for RTCP data buffers
        MpBufPool *RtpHeadersPool;   ///< Memory pool for headers of RTP and
                                     ///<  RTCP data buffers
#ifdef REAL_RTCP // [   This is just a reminder - we should implement MpRtcpBuf
        MpBufPool *RtcpHeadersPool;  ///< Memory pool for headers of RTCP
#endif // REAL_RTCP ]
        MpBufPool *UdpPool;          ///< Memory pool for raw UDP packets
        MpBufPool *UdpHeadersPool;   ///< Memory pool for headers of UDP packets
                                     ///<  buffers
        MpAudioBufPtr mpFgSilence;   ///< Buffer filled with silence. Used for
                                     ///<  mutting and as default output. You
                                     ///<  should not modify this buffer, cause
                                     ///<  it is used many times.
        MpAudioBufPtr comfortNoise;  ///< Buffer filled with comfort noise. You
                                     ///<  should not modify this buffer, cause
                                     ///<  it is used many times.
        int max_mic_buffers;    ///< Maximum messages in mic queue (soft limit)
        int max_spkr_buffers;   ///< Maximum messages in spkr queue (soft limit)
};

extern struct __MpGlobals MpMisc;

#endif /* _INCLUDED_MPMISC_H ] */
