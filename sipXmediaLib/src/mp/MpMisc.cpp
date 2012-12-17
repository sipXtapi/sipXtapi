//
// Copyright (C) 2005-2012 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES

#include <mp/MpTypes.h>
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

#include "os/OsDefs.h" // for min macro
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
#include "mp/MprDejitter.h"
#include "mp/MpMediaTask.h"
#include "mp/MpCodecFactory.h"
#include "mp/MpStaticCodecInit.h"
#include "os/OsDateTime.h"

//#define RTL_ENABLED

#ifdef RTL_ENABLED
#  include <rtl_macro.h>
#  ifdef INIT_EXTERNAL_RTL_COLLECTOR
      extern "C" void setExternalRtlCollector(SeScopeDataCollector *rtlCollector);
#  else
#     define setExternalRtlCollector(x)
#  endif
#else
#  define RTL_START(x)
#  define RTL_STOP
#  define RTL_WRITE(x)
#  define setExternalRtlCollector(x)
#endif


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

//#define RTP_BUFS (MprDejitter::MAX_RTP_PACKETS + 20)
#define RTCP_BUFS 16
#define UDP_BUFS 10

// STATIC VARIABLE INITIALIZATIONS

struct __MpGlobals MpMisc;


MpBufPool** sMpBufPools[] =
{
    &MpMisc.RawAudioPool,
    &MpMisc.AudioHeadersPool,
    &MpMisc.RtpPool,
    &MpMisc.RtcpPool,
    &MpMisc.RtpHeadersPool,
#ifdef REAL_RTCP
    &MpMisc.RtcpHeadersPool,
#endif
    &MpMisc.UdpPool,
    &MpMisc.UdpHeadersPool
};

int sMpNumBufPools = sizeof(sMpBufPools) / sizeof(MpBufPool**);

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
        l = sipx_min(ABSOLUTE_MAX_LOG_MSG_LEN, strlen(buf));
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
                n = sipx_min(l, MpMisc.logMsgSize);
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
extern intptr_t showMpMisc(int justAddress);
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

intptr_t showMpMisc(int justAddress)
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
   return (intptr_t) &MpMisc;
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

#ifndef CODEC_PLUGIN_PATH
// Windows compilers do not allow definition of preprocessor macros with 
// quotes in them within project files, so we need to stringify it here.
// All platforms do this now.
// Two levels of indirection are necessary to properly stringify a value in a macro.
#  define STRINGIFY(s) #s
#  define SSTRINGIFY(s) STRINGIFY(s)
#  ifndef DEFAULT_CODECS_PATH
#     define CODEC_PLUGIN_PATH  "."
#  else
#     define CODEC_PLUGIN_PATH  SSTRINGIFY(DEFAULT_CODECS_PATH)
#  endif
#endif


OsStatus mpStartUp(int sampleRate, int samplesPerFrame,
                   int numAudioBuffers, OsConfigDb* pConfigDb,
                   const size_t numCodecPaths, const UtlString codecPaths[])
{
    // TODO:
    // This should be broken down into separate arguments for:
    // Max number of flowgraphs
    // Max number of calls
    // Max calls/streams per flowgraph
    // Buffers used/stream or used/flowgraph
    // Approximate based upon current media interface initialization
    int maxCalls = numAudioBuffers / 16;

#ifdef _VXWORKS
        int defSilenceSuppressLevel = 10000;
#else
        int defSilenceSuppressLevel = 0;
#endif
        OsStatus  resCode;
        UtlBoolean silenceSuppressFlag;
        UtlString  silenceSuppressEnable;
        int       silenceSuppressLevel;

        assert(samplesPerFrame >= 8);

        showMpMisc(TRUE);

        RTL_START(100000000);
        setExternalRtlCollector(sRealtimeLogCollector);

        // First initialize static codecs
        mpStaticCodecInitializer();

        MpCodecFactory* pcf = MpCodecFactory::getMpCodecFactory();

        if(numCodecPaths != 0)
        {
           size_t i;
           for(i = 0; i < numCodecPaths; i++)
           {
              pcf->loadAllDynCodecs(codecPaths[i].data(), CODEC_PLUGINS_FILTER);
           }
        }
        else
        {
           pcf->loadAllDynCodecs(CODEC_PLUGIN_PATH, CODEC_PLUGINS_FILTER);
        }

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
                                        + MpArrayBuf::getHeaderSize(),
                                      numAudioBuffers,
                                      "RawAudioPool");
        Nprintf( "mpStartUp: MpMisc.RawAudioPool = 0x%X\n"
               , (int) MpMisc.RawAudioPool, 0,0,0,0,0);
        if (NULL == MpMisc.RawAudioPool) {
            return OS_NO_MEMORY;
        }

        // Create buffer for audio headers
        int audioBuffers  = MpMisc.RawAudioPool->getNumBlocks();
        MpMisc.AudioHeadersPool = new MpBufPool(sizeof(MpAudioBuf), audioBuffers, "AudioHeadersPool");
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
            sb->setSpeechType(MP_SPEECH_SILENT);
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
            cnb->setSpeechType(MP_SPEECH_COMFORT_NOISE);
            MpMisc.comfortNoise = cnb;
            Zprintf("mpStartUp: MpMisc.comfortNoise = 0x%X\n",
                    (int) MpMisc.comfortNoise, 0,0,0,0,0);
        }

        // Create buffer for RTP packets
        // The 50 bufs/call number comes from observation of cases when there is a burst of many
        // call setups at once.  We see 30-50 RTP buffers getting queued up in the begining of the
        // call.  There is some other problem there causing this backup.  For now we hide it with
        // sufficient buffers to backup and then catch up.
        MpMisc.RtpPool = new MpBufPool( RTP_MTU+MpArrayBuf::getHeaderSize(),
                                      50 * maxCalls + 70,
                                      "RtpPool");
        Nprintf("mpStartUp: MpMisc.RtpPool = 0x%X\n",
                (int) MpMisc.RtpPool, 0,0,0,0,0);
        if (NULL == MpMisc.RtpPool) {
            delete MpMisc.RawAudioPool;
            MpMisc.RawAudioPool = NULL;
            return OS_NO_MEMORY;
        }

        // Create buffer for RTCP packets
        MpMisc.RtcpPool = new MpBufPool( RTCP_MTU+MpArrayBuf::getHeaderSize(),
                                       maxCalls * RTCP_BUFS,
                                       "RtcpPool");
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
        MpMisc.RtpHeadersPool = new MpBufPool( sizeof(MpRtpBuf),
                                             MpMisc.RtpPool->getNumBlocks()
                                               + MpMisc.RtcpPool->getNumBlocks(),
                                               "RtpHeadersPool");
        Nprintf( "mpStartUp: MpMisc.RtpHeadersPool = 0x%X\n"
               , (int) MpMisc.RtpHeadersPool, 0,0,0,0,0);
        if (NULL == MpMisc.RtpHeadersPool) {
            // TODO:: Think about proper resource deallocation on fail in mpStartUp()
            return OS_NO_MEMORY;
        }
        MpRtpBuf::smpDefaultPool = MpMisc.RtpHeadersPool;

        // Create buffer for UDP packets
        MpMisc.UdpPool = new MpBufPool( UDP_MTU+MpArrayBuf::getHeaderSize(),
                                      UDP_BUFS,
                                      "UdpPool");
        Nprintf("mpStartUp: MpMisc.UdpPool = 0x%X\n",
                (int) MpMisc.UdpPool, 0,0,0,0,0);
        if (NULL == MpMisc.UdpPool) {
            // TODO:: Think about proper resource deallocation on fail in mpStartUp()
            return OS_NO_MEMORY;
        }

        // Create buffer for UDP packet headers
        MpMisc.UdpHeadersPool = new MpBufPool( sizeof(MpUdpBuf),
                                              MpMisc.UdpPool->getNumBlocks(),
                                              "UdpHeadersPool");
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
        if (NULL != MpMediaTask::getMediaTask()) {
           // This will MpMediaTask::spInstance to NULL
           delete MpMediaTask::getMediaTask();
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

        MpCodecFactory::freeSingletonHandle();

        mpStaticCodecUninitializer();

        OsDateTime curTime;
        char rtl_filename[1024];
        OsDateTime::getCurTime(curTime);
        sprintf(rtl_filename, "%04u%02u%02u_%02u-%02u-%02u.rtl",
                curTime.getYear(), 1 + curTime.getMonth(), curTime.getDay() - 1,
                curTime.getHour(), curTime.getMinute(), curTime.getSecond());
        setExternalRtlCollector(NULL);
        RTL_WRITE(rtl_filename)
        RTL_STOP

        return OS_SUCCESS;
}

OsStatus mpStartTasks(void)
{
        if (OS_SUCCESS != dmaStartup(MpMisc.frameSamples)) {
            return OS_TASK_NOT_STARTED;
        }

        NetInTask *pTask = NetInTask::getNetInTask();
        if (NULL == pTask) {
            dmaShutdown();
            return OS_TASK_NOT_STARTED;
        }

        return OS_SUCCESS;
}

OsStatus mpStopTasks(void)
{
    NetInTask *pTask = NetInTask::getNetInTask();
    pTask->destroy();

    dmaShutdown();
   
    return OS_SUCCESS;
}

void mpLogBufferStats(const char* label)
{
    UtlString bufferStatString;
    for(int poolIndex = 0; poolIndex < sMpNumBufPools; poolIndex++)
    {
        MpBufPool** bufferPool = sMpBufPools[poolIndex];
        bufferStatString.appendFormat("\t%s %d/%d buffers free\n",
                bufferPool[0]->getName().data(), bufferPool[0]->getFreeBufferCount(), bufferPool[0]->getNumBlocks());
    }
    OsSysLog::add(FAC_MP, PRI_DEBUG, "%s:]n%s", label, bufferStatString.data());
} 

