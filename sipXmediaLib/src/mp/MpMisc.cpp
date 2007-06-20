//
// Copyright (C) 2005-2006 SIPez LLC.
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


// SYSTEM INCLUDES

#include <stdio.h> 
#include <assert.h> 
#include <string.h>
#ifdef _VXWORKS /* [ */
#include <unistd.h>

#include <msgQLib.h>
#include <taskLib.h>
#include <vmLib.h>
#include <intLib.h>
#include <tickLib.h>
#endif /* _VXWORKS ] */

// APPLICATION INCLUDES
#ifdef _VXWORKS /* [ */
#if CPU == ARMSA110 || (CPU == ARMARCH4) || (CPU == STRONGARM) /* [ [ */
#include "mp/sa1100.h"
#elif (CPU == XSCALE) /* ] [ */
#include "mp/pxa255.h"
#else /* ] [ */
#error Unexpected CPU value
#endif /* ] ] */
#endif /* _VXWORKS ] */

#include "os/OsMsgQ.h"
#include "os/OsConfigDb.h"
#include "mp/MpTypes.h"
#include "mp/MpCodec.h"
#include "mp/dmaTask.h"
#include "mp/MpBuf.h"
#include "mp/MpAudioBuf.h"
#include "mp/MpRtpBuf.h"
#include "mp/MpUdpBuf.h"
#include "mp/MpBufferMsg.h"
#include "mp/MpMisc.h"
#include "mp/NetInTask.h"
#include "mp/MprFromMic.h"
#include "mp/MprToSpkr.h"
#include "mp/MpMediaTask.h"

// EXTERNAL FUNCTIONS

// EXTERNAL VARIABLES

// CONSTANTS

#define MIC_BUFFER_Q_LEN 10
#define SPK_BUFFER_Q_LEN 14
#define ECHO_BUFFER_Q_LEN MIC_BUFFER_Q_LEN+SPK_BUFFER_Q_LEN

#ifdef _VXWORKS /* [ */
#define LOG_MSGQ_MAX_MSGS 8000
#define LOG_MSGQ_ITEM_LEN 60
#define ABSOLUTE_MAX_LOG_MSG_LEN 2048
#endif /* _VXWORKS ] */

#define RTP_BUFS 250
#define RTCP_BUFS 16
#define UDP_BUFS 10

// STATIC VARIABLE INITIALIZATIONS

    struct __MpGlobals MpMisc;

#ifdef _VXWORKS /* [ */
/************************************************************************/

static int logTaskPrio = 250;
int setLogTaskPrio(int p) {
   int save = logTaskPrio;
   logTaskPrio = 0xff & p;
   return save;
}

static int logging = 1;

static int printingLog = 0;
static int numDiscarded = 0;

int drain_log(int level /* how many to leave */)
{
        char msg[1];
        int l, n;

        if (level < 0) level = 50;
        n = msgQNumMsgs(MpMisc.LogQ);
        while(level < msgQNumMsgs(MpMisc.LogQ)) {
            l = msgQReceive(MpMisc.LogQ, msg, 1, VX_NO_WAIT);
            numDiscarded++;
        }
        return n;
}

int StartLogging()
{
        int i;
        i = logging;
        logging = 1;
        return i;
}

int StopLogging(int opt)
{
        int i;

        logging = 0;
        i = msgQNumMsgs(MpMisc.LogQ);
        if (opt) drain_log(50);
        return i;
}

int Zprintf0(int force, char *buf)
{
        int l;
        int n;
        int ret;
        int msgret;
        char *str;
        char junk;

        str = buf;
        l = min(ABSOLUTE_MAX_LOG_MSG_LEN, strlen(buf));
        ret = l;
        if (0 == MpMisc.LogQ) {
            ret = force ? fwrite(str, 1, ret, stderr) : 0;
            return ret;
        }
        if (force || logging) {
            if ((ret > MpMisc.logMsgSize) && !intContext()) {
                taskLock();
            }
            while (l > 0) {
                n = min(l, MpMisc.logMsgSize);
                msgret = msgQSend(MpMisc.LogQ, buf, n,
                                       VX_NO_WAIT, MSG_PRI_NORMAL);
                if (ERROR == msgret) {
                    // int r2 =
                    msgQReceive(MpMisc.LogQ, &junk, 1, VX_NO_WAIT);
                    // osPrintf("discard message; r1=%d, r2=%d, q=0x%X: '%s'\n",
                        // msgret, r2, MpMisc.LogQ, buf);
                    numDiscarded++;
                    msgret = msgQSend(MpMisc.LogQ, buf, n,
                                       VX_NO_WAIT, MSG_PRI_NORMAL);
                }
                if ((ERROR == msgret) && force) {
                    fwrite(str, 1, ret, stderr);
                    l = 0;
                } else {
                    l -= n;
                    buf += n;
                }
            }
            if ((ret > MpMisc.logMsgSize) && !intContext()) {
                taskUnlock();
            }
        } else {
            ret = 0;
        }
        return ret;
}

int Zprintfx(int force, char *fmt, int a, int b, int c, int d, int e, int f)
{
        int l, ret;
        char buf[256];

        if (force || logging) {
            sprintf(buf, fmt, a, b, c, d, e, f);
            l = Zprintf0(force, buf);
        } else {
            l = 0;
        }
        return l;
}

int NoPrintLog()
{
    int save = printingLog;
    if (printingLog) {
        numDiscarded = 0;
    }
    osPrintf("\n");
    printingLog = 0;
    return save;
}
int PrintLog(int num)
{
    int save = printingLog;
    if (!printingLog) {
        if (num > 0) drain_log(num);
        fprintf(stderr, "%d messages discarded\n", numDiscarded);
        numDiscarded = 0;
    }
    printingLog = 1;
    return save;
}

extern "C" int NPL(){return NoPrintLog();}
extern "C" int PL(int n){return PrintLog(n);}

static int printTask(int t1, int t2, int t3, int t4, int t5,
                int t6, int t7, int t8, int t9, int t10)
{
        char msg[LOG_MSGQ_ITEM_LEN+1];
        int l;
        int lastTick;
        int thisTick;
        int Hz;
        int qHz;

        Hz = sysClkRateGet();
        qHz = Hz / 4;
#define LOG_ANNOUNCE_MINUTES 5

        lastTick = (int) tickGet() - (LOG_ANNOUNCE_MINUTES * 61 * Hz);

        while (1) {
            while (!printingLog) taskDelay(qHz);
            l = msgQReceive(MpMisc.LogQ, msg, LOG_MSGQ_ITEM_LEN, VX_WAIT_FOREVER);
            thisTick = (int) tickGet();
            if ((LOG_ANNOUNCE_MINUTES * 60 * Hz) < (thisTick - lastTick)) {
                lastTick = thisTick;
                fprintf(stderr, "MsgLogger: %d.%03d seconds since boot\n",
                    thisTick/Hz, ((thisTick % Hz) * 1000) / Hz);
            }
            if (l > 0) {
                msg[l] = 0;
                fwrite(msg, 1, l, stderr);
            } else {
                osPrintf("\n\nLogger: quitting!\n\n");
                return 0;
            }
        }
}

MSG_Q_ID startLogging(int nmsgs, int maxlen)
{
        MpMisc.LogQ = msgQCreate(nmsgs, maxlen, MSG_Q_FIFO);
        if (NULL == MpMisc.LogQ) {
                osPrintf("cannot create LogQ!  Quitting\n");
        } else {
            Zprintf("logQ is 0x%X\n", (int) MpMisc.LogQ, 0,0,0,0,0);
            taskSpawn("Logger", logTaskPrio, 0, 8192, (FUNCPTR) printTask,
                           0, 0, 0,0,0,0,0,0,0,0);
        }
        return MpMisc.LogQ;
}

#ifdef DEBUG /* [ */
int jamLog(char *fmt, int count, int a, int b, int c, int d, int e)
{
    int i;

    for (i=0; i<count; i++) {
        Zprintfx(1, fmt, i, a, b, c, d, e);
    }
    return msgQNumMsgs(MpMisc.LogQ);
}
#endif /* DEBUG ] */

OsStatus startLogTask()
{
#ifdef _VXWORKS /* [ */
        MpMisc.logMsgLimit = LOG_MSGQ_MAX_MSGS;
        MpMisc.logMsgSize = LOG_MSGQ_ITEM_LEN;
        if (NULL == startLogging(MpMisc.logMsgLimit, MpMisc.logMsgSize)) {
            return OS_UNSPECIFIED;
        }
#endif /* _VXWORKS ] */
        return OS_SUCCESS;
}

/************************************************************************/

/* Work around for GDB crash... */
static int dontStepInThis()
{
        int ps;

        ps = vmPageSizeGet();
        return ps;
}

static int goGetThePageSize()
{
        return dontStepInThis(); /*!!!*/
}
#endif /* _VXWORKS ] */

/************************************************************************/
extern "C" {
extern int showMpMisc(int justAddress);
extern int setMaxMic(int v);
extern int setMaxSpkr(int v);
extern int mpSetLatency(int maxMic, int maxSpkr, int minRtp);
#ifdef _VXWORKS /* [ */
extern int LoopBack(int on);
extern int LBon();
extern int LBoff();
extern int Spkr1On();
extern int Spkr1Off();
extern int Spkr2On();
extern int Spkr2Off();
#endif /* _VXWORKS ] */
}
/************************************************************************/

#ifdef _VXWORKS /* [ */
#if CPU == ARMSA110 || (CPU == ARMARCH4) || (CPU == STRONGARM) /* [ */

int LoopBack(int on) {
   MpBufferMsg*    pMsg;
   int save = MpMisc.doLoopBack;

   MpMisc.doLoopBack = on;
   while (0 < MpMisc.pLoopBackQ->numMsgs()) {
      if (OS_SUCCESS == MpMisc.pLoopBackQ->receive((OsMsg*&) pMsg,
                                                    OsTime::NO_WAIT_TIME)) {
         pMsg->releaseMsg();
      }
   }
   return save;
}
int LBon()  {return LoopBack(1);}
int LBoff() {return LoopBack(0);}

static int* gpsr = (int*) SA1100_GPSR;
static int* gpcr = (int*) SA1100_GPCR;
int Spkr1On()  {*gpcr = (1<<25); return 0;}
int Spkr1Off() {*gpsr = (1<<25); return 0;}
int Spkr2On()  {*gpcr = (1<<26); return 0;}
int Spkr2Off() {*gpsr = (1<<26); return 0;}

#endif /* StrongARM ] */
#endif /* _VXWORKS ] */

int showMpMisc(int justAddress)
{
   Zprintf("&MpMisc = 0x%X\n", (int) &MpMisc, 0,0,0,0,0);
   if (!justAddress) {
      Zprintf(" MicQ=0x%X, SpkQ=0x%X, EchoQ=0x%X, silence=0x%X\n"
         (int) MpMisc.pMicQ, (int) MpMisc.pSpkQ, (int) MpMisc.pEchoQ,
         (int) MpMisc.mpFgSilence, 0, 0, 0);
      Zprintf(" \n frameSamples=%d, frameBytes=%d, sampleBytes=%d,",
         MpMisc.frameSamples, MpMisc.frameBytes, MpMisc.sampleBytes, 0,0,0);
      Zprintf(" rtpMaxBytes=%d\n RawAudioPool=0x%X, RtpPool=0x%X, RtcpPool=0x%X\n",
         MpMisc.rtpMaxBytes, (int) MpMisc.RawAudioPool, (int) MpMisc.RtpPool,
         (int) MpMisc.RtcpPool, 0,0);
#ifdef _VXWORKS /* [ */
      Zprintf(" mem_page_size=%d, mem_page_mask=0x%08X\n"
         "LogQ=0x%X, logMsgLimit=%d, logMsgSize=%d\n",
         MpMisc.mem_page_size, MpMisc.mem_page_mask,
         (int) MpMisc.LogQ, MpMisc.logMsgLimit, MpMisc.logMsgSize, 0);
#endif /* _VXWORKS ] */
      Zprintf(" Latency: maxMic=%d, maxSpkr=%d"
         MpMisc.max_mic_buffers, MpMisc.max_spkr_buffers,
         0,0,0,0,0);
   }
   return (int) &MpMisc;
}

int setMaxMic(int v)
{
    int save = MpMisc.max_mic_buffers;

    if (v >= MIC_BUFFER_Q_LEN) {
        int vWas = v;
        v = MIC_BUFFER_Q_LEN - 1;
        osPrintf("\nmax_mic_buffers MUST BE less than %d... setting to"
            " %d instead of %d\n",
            MIC_BUFFER_Q_LEN, v, vWas);
    }
    if (v > 0) MpMisc.max_mic_buffers = v;
    return save;
}

int setMaxSpkr(int v)
{
    int save = MpMisc.max_spkr_buffers;

    if (v >= SPK_BUFFER_Q_LEN) {
        int vWas = v;
        v = SPK_BUFFER_Q_LEN - 1;
        osPrintf("\nmax_spkr_buffers MUST BE less than %d... setting to"
            " %d instead of %d\n",
            SPK_BUFFER_Q_LEN, v, vWas);
    }
    if (v > 0) MpMisc.max_spkr_buffers = v;
    return save;
}


int mpSetLatency(int maxMic, int maxSpkr, int minRtp)
{
    setMaxMic(maxMic);
    setMaxSpkr(maxSpkr);
    return 0;
}

int mpSetHighLatency()
{
    return mpSetLatency(2, 2, 3);
}

int mpSetMedLatency()
{
    return mpSetLatency(1, 1, 2);
}

int mpSetLowLatency()
{
    return mpSetLatency(1, 1, 1);
}

extern void doFrameLoop(int sampleRate, int frame_samples);
extern STATUS netStartup();

OsStatus mpStartUp(int sampleRate, int samplesPerFrame,
                   int numAudioBuffers, OsConfigDb* pConfigDb)
{
#ifdef _VXWORKS
        int defSilenceSuppressLevel = 10000;
#else
        int defSilenceSuppressLevel = 0;
#endif
        OsStatus  resCode;
        UtlBoolean silenceSuppressFlag;
        UtlString  silenceSuppressEnable;
        int       silenceSuppressLevel;

        if (samplesPerFrame < 8) samplesPerFrame = 80;
        samplesPerFrame = min(samplesPerFrame, FRAME_SAMPS);

        showMpMisc(TRUE);

#ifdef _VXWORKS /* [ */
        /* Rashly assumes page size is a power of two */
        MpMisc.mem_page_size = goGetThePageSize();
        MpMisc.mem_page_mask = MpMisc.mem_page_size - 1;
#endif /* _VXWORKS ] */

        MpMisc.sampleBytes = sizeof(MpAudioSample);
        MpMisc.frameSamples = samplesPerFrame;
        MpMisc.frameBytes = MpMisc.sampleBytes * MpMisc.frameSamples;
        MpMisc.rtpMaxBytes = /* sizeof(struct RtpHeader) */ 12 + 
            (((sampleRate + 24) / 25) * MpMisc.sampleBytes);

        // Create buffer for audio data in mediagraph
        MpMisc.RawAudioPool = new MpBufPool( samplesPerFrame*sizeof(MpAudioSample)
                                        +MpArrayBuf::getHeaderSize()
                                      , numAudioBuffers);
        Nprintf( "mpStartUp: MpMisc.RawAudioPool = 0x%X\n"
               , (int) MpMisc.RawAudioPool, 0,0,0,0,0);
        if (NULL == MpMisc.RawAudioPool) {
            return OS_NO_MEMORY;
        }

        // Create buffer for audio headers
        int audioBuffers  = MpMisc.RawAudioPool->getNumBlocks();
        MpMisc.AudioHeadersPool = new MpBufPool(sizeof(MpAudioBuf), audioBuffers);
        Nprintf( "mpStartUp: MpMisc.AudioHeadersPool = 0x%X\n"
               , (int) MpMisc.AudioHeadersPool, 0,0,0,0,0);
        if (NULL == MpMisc.AudioHeadersPool) {
            // TODO:: Think about proper resource deallocation on fail in mpStartUp()
            return OS_NO_MEMORY;
        }
        MpAudioBuf::smpDefaultPool = MpMisc.AudioHeadersPool;

        /*
        * Go get a buffer and fill with silence.  We will use this for muting
        * either or both of input and output, and whenever we are starved for
        * audio data.
        */
        {
            MpAudioBufPtr sb = MpMisc.RawAudioPool->getBuffer();
            if (!sb.isValid()) {
                Zprintf("\n\mpStartUp:"
                        " MpBufPool::getBuffer() failed, quitting!\n\n\n",
                        0,0,0,0,0,0);
                delete MpMisc.RawAudioPool;
                MpMisc.RawAudioPool = NULL;
                return OS_LIMIT_REACHED;
            }

            sb->setSamplesNumber(samplesPerFrame);
            memset(sb->getSamplesWritePtr(), 0, sb->getSamplesNumber()*sizeof(MpAudioSample));
            sb->setSpeechType(MpAudioBuf::MP_SPEECH_SILENT);
            MpMisc.mpFgSilence = sb;
            Zprintf("mpStartUp: MpMisc.silence = 0x%X\n",
                (int) MpMisc.mpFgSilence, 0,0,0,0,0);
        }

        /*
        * generate a buffer called comfort noise buffer. Even though the zero
        * initiation is not necessary, we do it as the silence buffer for safety.
        */
        {
            MpAudioBufPtr cnb = MpMisc.RawAudioPool->getBuffer();
            if (!cnb.isValid()) {
                Zprintf("\n\mpStartUp:"
                        " MpBufPool::getBuffer() failed, quitting!\n\n\n",
                        0,0,0,0,0,0);
                delete MpMisc.RawAudioPool;
                MpMisc.RawAudioPool = NULL;
                return OS_LIMIT_REACHED;
            }

            cnb->setSamplesNumber(samplesPerFrame);
            memset(cnb->getSamplesWritePtr(), 0, cnb->getSamplesNumber()*sizeof(MpAudioSample));
            cnb->setSpeechType(MpAudioBuf::MP_SPEECH_COMFORT_NOISE);
            MpMisc.comfortNoise = cnb;
            Zprintf("mpStartUp: MpMisc.comfortNoise = 0x%X\n",
                    (int) MpMisc.comfortNoise, 0,0,0,0,0);
        }

        // Create buffer for RTP packets
        MpMisc.RtpPool = new MpBufPool( RTP_MTU+MpArrayBuf::getHeaderSize()
                                      , RTP_BUFS);
        Nprintf("mpStartUp: MpMisc.RtpPool = 0x%X\n",
                (int) MpMisc.RtpPool, 0,0,0,0,0);
        if (NULL == MpMisc.RtpPool) {
            delete MpMisc.RawAudioPool;
            MpMisc.RawAudioPool = NULL;
            return OS_NO_MEMORY;
        }

        // Create buffer for RTCP packets
        MpMisc.RtcpPool = new MpBufPool( RTCP_MTU+MpArrayBuf::getHeaderSize()
                                       , RTCP_BUFS);
        Nprintf("mpStartUp: MpMisc.RtcpPool = 0x%X\n",
                (int) MpMisc.RtcpPool, 0,0,0,0,0);
        if (NULL == MpMisc.RtcpPool) {
            delete MpMisc.RawAudioPool;
            MpMisc.RawAudioPool = NULL;
            delete MpMisc.RtpPool;
            MpMisc.RtpPool = NULL;
            return OS_NO_MEMORY;
        }

        // Create buffer for RTP and RTCP headers
        MpMisc.RtpHeadersPool = new MpBufPool( sizeof(MpRtpBuf)
                                             , MpMisc.RtpPool->getNumBlocks()
                                               + MpMisc.RtcpPool->getNumBlocks());
        Nprintf( "mpStartUp: MpMisc.RtpHeadersPool = 0x%X\n"
               , (int) MpMisc.RtpHeadersPool, 0,0,0,0,0);
        if (NULL == MpMisc.RtpHeadersPool) {
            // TODO:: Think about proper resource deallocation on fail in mpStartUp()
            return OS_NO_MEMORY;
        }
        MpRtpBuf::smpDefaultPool = MpMisc.RtpHeadersPool;

        // Create buffer for UDP packets
        MpMisc.UdpPool = new MpBufPool( UDP_MTU+MpArrayBuf::getHeaderSize()
                                      , UDP_BUFS);
        Nprintf("mpStartUp: MpMisc.UdpPool = 0x%X\n",
                (int) MpMisc.UdpPool, 0,0,0,0,0);
        if (NULL == MpMisc.UdpPool) {
            // TODO:: Think about proper resource deallocation on fail in mpStartUp()
            return OS_NO_MEMORY;
        }

        // Create buffer for UDP packet headers
        MpMisc.UdpHeadersPool = new MpBufPool( sizeof(MpUdpBuf)
                                              , MpMisc.UdpPool->getNumBlocks());
        Nprintf( "mpStartUp: MpMisc.UdpHeadersPool = 0x%X\n"
               , (int) MpMisc.UdpHeadersPool, 0,0,0,0,0);
        if (NULL == MpMisc.UdpHeadersPool) {
            // TODO:: Think about proper resource deallocation on fail in mpStartUp()
            return OS_NO_MEMORY;
        }
        MpUdpBuf::smpDefaultPool = MpMisc.UdpHeadersPool;

        // Use the config database to determine the silence suppression level
        silenceSuppressFlag  = FALSE;
        silenceSuppressLevel = defSilenceSuppressLevel;
        if (pConfigDb)
        {
           resCode = pConfigDb->get("PHONESET_SILENCE_SUPPRESSION",
                                    silenceSuppressEnable);
           if (resCode == OS_SUCCESS)
           {
              silenceSuppressFlag =
                (silenceSuppressEnable.compareTo("enable",
                                                 UtlString::ignoreCase) == 0);
           }

           resCode = pConfigDb->get("PHONESET_SILENCE_SUPPRESSION_LEVEL",
                                    silenceSuppressLevel);
           if ((resCode != OS_SUCCESS) ||
               (silenceSuppressLevel < 0) ||
               (silenceSuppressLevel > 20000))
           {
              silenceSuppressLevel = defSilenceSuppressLevel;
           }
        }

#ifdef WIN32 /* [ */
        // Adjust initial audio latency if specified in config files:
        if (pConfigDb)
        {
           UtlString  latency;
           static const int MS_PER_FRAME = 10;
           int frames;
           resCode = pConfigDb->get("PHONESET_INITIAL_OUTPUT_LATENCY_MS",
                                    latency);
           if (resCode == OS_SUCCESS)
           {
              frames = atoi(latency.data()) / MS_PER_FRAME;
              DmaTask_setSpkrQPreload(frames);
           }
           resCode = pConfigDb->get("PHONESET_INITIAL_INPUT_LATENCY_MS",
                                    latency);
           if (resCode == OS_SUCCESS)
           {
              frames = atoi(latency.data()) / MS_PER_FRAME;
              DmaTask_setMicQPreload(frames);
          }
        }
#endif /* WIN32 ] */

#ifdef _VXWORKS /* [ */
        if (OS_SUCCESS != MpCodecOpen(sampleRate, START_GAIN, START_VOLUME)) {
            return OS_UNSPECIFIED;
        }
#endif /* _VXWORKS ] */

        if (NULL != MpMisc.pMicQ) {
            OsMsgQ* q = MpMisc.pMicQ;
            MpMisc.pMicQ = NULL;
            delete q;
        }
        if (NULL != MpMisc.pSpkQ) {
            OsMsgQ* q = MpMisc.pSpkQ;
            MpMisc.pSpkQ = NULL;
            delete q;
        }
        if (NULL != MpMisc.pEchoQ) {
            OsMsgQ* q = MpMisc.pEchoQ;
            MpMisc.pEchoQ = NULL;
            delete q;
        }
#ifdef _VXWORKS /* [ */
        if (NULL != MpMisc.pLoopBackQ) {
            OsMsgQ* q = MpMisc.pLoopBackQ;
            MpMisc.pLoopBackQ = NULL;
            delete q;
        }
#endif /* _VXWORKS ] */
        assert( (MIC_BUFFER_Q_LEN+SPK_BUFFER_Q_LEN+MIC_BUFFER_Q_LEN)
                < (MpMisc.RawAudioPool->getNumBlocks()-3) );
        MpMisc.pMicQ = new OsMsgQ(MIC_BUFFER_Q_LEN);
        MpMisc.pSpkQ = new OsMsgQ(SPK_BUFFER_Q_LEN);
        MpMisc.pEchoQ = new OsMsgQ(ECHO_BUFFER_Q_LEN);
#ifdef _VXWORKS /* [ */
        MpMisc.doLoopBack = 0;
        MpMisc.pLoopBackQ = new OsMsgQ(MIC_BUFFER_Q_LEN);
#endif /* _VXWORKS ] */

        assert(MprFromMic::MAX_MIC_BUFFERS > 0);
        assert(MprToSpkr::MAX_SPKR_BUFFERS > 0);

        setMaxMic(MprFromMic::MAX_MIC_BUFFERS - 1);
        setMaxSpkr(MprToSpkr::MAX_SPKR_BUFFERS);

        return OS_SUCCESS;
}

OsStatus mpShutdown(void)
{
        if (NULL != MpMediaTask::getMediaTask(0)) {
           // This will MpMediaTask::spInstance to NULL
           delete MpMediaTask::getMediaTask(0);
        }

        if (NULL != MpMisc.pMicQ) {
            OsMsgQ* q = MpMisc.pMicQ;
            MpMisc.pMicQ = NULL;
            delete q;
        }
        if (NULL != MpMisc.pSpkQ) {
            OsMsgQ* q = MpMisc.pSpkQ;
            MpMisc.pSpkQ = NULL;
            delete q;
        }
        if (NULL != MpMisc.pEchoQ) {
            OsMsgQ* q = MpMisc.pEchoQ;
            MpMisc.pEchoQ = NULL;
            delete q;
        }
#ifdef _VXWORKS /* [ */
        if (NULL != MpMisc.pLoopBackQ) {
            OsMsgQ* q = MpMisc.pLoopBackQ;
            MpMisc.pLoopBackQ = NULL;
            delete q;
        }
#endif /* _VXWORKS ] */

        MpMisc.mpFgSilence.release();
        MpMisc.comfortNoise.release();

        if (NULL != MpMisc.UdpHeadersPool) {
            delete MpMisc.UdpHeadersPool;
            MpMisc.UdpHeadersPool = NULL;
        }

        if (NULL != MpMisc.UdpPool) {
            delete MpMisc.UdpPool;
            MpMisc.UdpPool = NULL;
        }

        if (NULL != MpMisc.RtpHeadersPool) {
            delete MpMisc.RtpHeadersPool;
            MpMisc.RtpHeadersPool = NULL;
        }

        if (NULL != MpMisc.RtpPool) {
            delete MpMisc.RtpPool;
            MpMisc.RtpPool = NULL;
        }

        if (NULL != MpMisc.RtcpPool) {
            delete MpMisc.RtcpPool;
            MpMisc.RtcpPool = NULL;
        }

        if (NULL != MpMisc.AudioHeadersPool) {
            delete MpMisc.AudioHeadersPool;
            MpMisc.AudioHeadersPool = NULL;
        }

        if (NULL != MpMisc.RawAudioPool) {
            delete MpMisc.RawAudioPool;
            MpMisc.RawAudioPool = NULL;
        }

        return OS_SUCCESS;
}

OsStatus mpStartTasks(void)
{
        if (OS_SUCCESS != dmaStartup(MpMisc.frameSamples)) {
            return OS_TASK_NOT_STARTED;
        }

        if (OS_SUCCESS != startNetInTask()) {
            return OS_TASK_NOT_STARTED;
        }

        return OS_SUCCESS;
}

OsStatus mpStopTasks(void)
{

    shutdownNetInTask();
    dmaShutdown();
   
    return OS_SUCCESS;
}
