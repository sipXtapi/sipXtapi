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

#include "mp/dmaTask.h"

// The following local static variable stores the state of the device's
// ringer.  The only way this variable can be set from outside this module is
// via the DmaTask_setRingerEnabled() function.
static bool sIsRingerEnabled = false;

// Maintain the "ringer" state for the device and return the previous setting.
bool MpDma_setRingerEnabled(bool enabled)
{
   bool prevSetting = sIsRingerEnabled;

   sIsRingerEnabled = enabled;

   return prevSetting;
}

#ifdef WIN32 /* [ */
// Test whether the device's ringer is currently enabled.
// This function is declared static because it is only used within this
// module.
static bool MpDma_isRingerEnabled(void)
{
   return sIsRingerEnabled;
}
#endif /* WIN32 ] */

// The following local static variable stores the "muted" state for the
// device.  The only way this variable can be set from outside this module is
// via the DmaTask_setMuteEnabled() function.
static bool sIsMuted = false;

// If there is a listener for changes in "mute" state, the function pointer
// for the listener is stored in the following static variable.
static MpDma_MuteListenerFuncPtr spMuteListener = NULL;

// Set a listener for "mute" state changes.  To remove the listener, set the
// listener to NULL.
void MpDma_setMuteListener(MpDma_MuteListenerFuncPtr pFunc)
{
   spMuteListener = pFunc;
}

// Set the "mute" state for the device and return the previous setting.
bool MpDma_setMuteEnabled(bool enabled)
{
   bool prevSetting = sIsMuted;

   sIsMuted = enabled;

   if (spMuteListener)
   {
      spMuteListener(enabled);
   }

   return prevSetting;
}

// Test whether the device's ringer is currently enabled.
bool MpDma_isMuteEnabled(void)
{
   return sIsMuted;
}

// TODO
// The rest of this file is really two independent platform-specific
// implementations of the dmaTask module.  These should be broken out into
// separate files.

#ifdef WIN32 /* [ */
#define DEBUG_WINDOZE
#undef DEBUG_WINDOZE

#define INSERT_SAWTOOTH
#undef INSERT_SAWTOOTH

#define INTERVAL 5
#define RESOLUTION 0

#include <windows.h>
#include <process.h>
#include <mmsystem.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

// APPLICATION INCLUDES

#include "mp/MpBufferMsg.h"
#include "mp/MpBuf.h"
#include "mp/MpMisc.h"
#include "mp/MprToSpkr.h"
#include "mp/MpMediaTask.h"
#include "mp/dmaTask.h"
#include "os/OsMsgPool.h"

OsMsgPool* DmaMsgPool = NULL;

// if N_BUFFERS is greater than 256, must change USER_BUFFER_MASK!
#define N_BUFFERS 100
#define USER_BUFFER_MASK 0xff

#define N_IN_BUFFERS N_BUFFERS
#define N_OUT_BUFFERS N_BUFFERS

// must not be more than N_IN_BUFFERS:
#define N_IN_PRIME 32 // N_IN_BUFFERS
// must not be more than N_OUT_BUFFERS:
#define N_OUT_PRIME 8 // N_OUT_BUFFERS

#define N_SAMPLES 80

#undef IHISTORY
#undef OHISTORY
#define IHISTORY (2 * (N_IN_BUFFERS))
#define OHISTORY (2 * (N_OUT_BUFFERS))
#undef IHISTORY
#undef OHISTORY

#define SAMPLES_PER_SEC 8000
#define MIN_SAMPLE_RATE 100
#define BITS_PER_SAMPLE 16

static int smSpkrQPreload = N_OUT_PRIME;
static int smMicQPreload = N_IN_PRIME;

int DmaTask_setSpkrQPreload(int qlen)
{
   int save = smSpkrQPreload;
   if (qlen < 1) qlen = 1;
   if (qlen > N_OUT_BUFFERS) qlen = N_OUT_BUFFERS;
   smSpkrQPreload = qlen;
   osPrintf(" Changing spkr preload from %d to %d frames\n", save, qlen);
   return save;
}

int DmaTask_setMicQPreload(int qlen)
{
   int save = smMicQPreload;
   if (qlen < 1) qlen = 1;
   if (qlen > N_IN_BUFFERS) qlen = N_IN_BUFFERS;
   smMicQPreload = qlen;
   osPrintf(" Changing mic preload from %d to %d frames\n", save, qlen);
   return save;
}

static HGLOBAL  hInHdr[N_IN_BUFFERS];
static WAVEHDR* pInHdr[N_IN_BUFFERS];
static HGLOBAL  hInBuf[N_IN_BUFFERS];

static HGLOBAL  hOutHdr[N_OUT_BUFFERS];
static WAVEHDR* pOutHdr[N_OUT_BUFFERS];
static HGLOBAL  hOutBuf[N_OUT_BUFFERS];

static int      samplesPerSecond = SAMPLES_PER_SEC;
static int      bitsPerSample = BITS_PER_SAMPLE;
static int      samplesPerFrame = N_SAMPLES;

#ifdef IHISTORY /* [ */
int histIn[IHISTORY];
int lastIn;
#endif /* HISTORY ] */

#ifdef OHISTORY /* [ */
int histOut[OHISTORY];
int lastOut;
#endif /* OHISTORY ] */

static void CALLBACK myInOutCallBack(HANDLE h, UINT wMsg,
                        DWORD dwInstance, DWORD dwParam, DWORD unused)
{
#ifdef IHISTORY /* [ */
   if (WIM_DATA == wMsg) {
      histIn[lastIn] = (((WAVEHDR*)dwParam)->dwUser) & USER_BUFFER_MASK;
      lastIn = (lastIn + 1) % IHISTORY;
   }
#endif /* HISTORY ] */

#ifdef OHISTORY /* [ */
   if (WOM_DONE == wMsg) {
      histOut[lastOut] = (((WAVEHDR*)dwParam)->dwUser) & USER_BUFFER_MASK;
      lastOut = (lastOut + 1) % OHISTORY;
   }
#endif /* OHISTORY ] */

   int retval = PostThreadMessage(dwInstance, wMsg, dwParam, GetTickCount());

   if (retval == 0)
   {
      Sleep(500);
      retval = PostThreadMessage(dwInstance, wMsg, dwParam, GetTickCount());
      if (retval == 0)
         osPrintf("Could not PostTheadMessage after two tries.\n");
   }
}

void showWaveError(char *syscall, int e, int N, int line)
{
   char *str = "(Unknown return code)";
   char *name = "(unknown)";

   if (MMSYSERR_NOERROR == e) return;
   switch (e) {
   case MMSYSERR_ALLOCATED:
      name = "MMSYSERR_ALLOCATED";
      str = "Specified resource is already allocated. ";
      break;
   case MMSYSERR_BADDEVICEID:
      name = "MMSYSERR_BADDEVICEID";
      str = "Specified device identifier is out of range. ";
      break;
   case MMSYSERR_NODRIVER:
      name = "MMSYSERR_NODRIVER";
      str = "No device driver is present. ";
      break;
   case MMSYSERR_NOMEM:
      name = "MMSYSERR_NOMEM";
      str = "Unable to allocate or lock memory. ";
      break;
   case WAVERR_BADFORMAT:
      name = "WAVERR_BADFORMAT";
      str = "Attempted to open with an unsupported waveform-audio format. ";
      break;
   case WAVERR_SYNC:
      name = "WAVERR_SYNC";
      str = "The device is synchronous but waveOutOpen was called without"
                  " using the WAVE_ALLOWSYNC flag. ";
      break;
   case MMSYSERR_INVALHANDLE:
      name = "MMSYSERR_INVALHANDLE";
      str = "Specified device handle is invalid.";
      break;
   case WAVERR_STILLPLAYING:
      name = "WAVERR_STILLPLAYING";
      str = "There are still buffers in the queue.";
      break;
   case WAVERR_UNPREPARED:
      name = "WAVERR_UNPREPARED";
      str = "The buffer pointed to by the pwh parameter has not been prepared.";
      break;
   case MMSYSERR_NOTSUPPORTED:
      name = "MMSYSERR_NOTSUPPORTED";
      str = "Specified device is synchronous and does not support pausing.";
      break;
   case MMSYSERR_INVALPARAM:
      name = "MMSYSERR_INVALPARAM";
      str = "Invalid parameter passed.";
      break;
   }
   if (-1 == N) {
      osPrintf("%s failed (line %d): res = %s (%d):\n   %s\n\n",
         syscall, line, name, e, str);
   } else {
      osPrintf("%s failed (line %d): res = %s (%d, %d):\n   %s\n\n",
        syscall, line, name, e, N, str);
   }
}

int isFormatSupported(int nChannels, int nSamplesPerSec, int nBitsPerSample)
{
   WAVEFORMATEX fmt;
   MMRESULT     res1, res2;

   fmt.wFormatTag      = WAVE_FORMAT_PCM;
   fmt.nChannels       = nChannels;
   fmt.nSamplesPerSec  = nSamplesPerSec;
   fmt.nAvgBytesPerSec = (nChannels * nSamplesPerSec * nBitsPerSample) / 8;
   fmt.nBlockAlign     = (nChannels * nBitsPerSample) / 8;
   fmt.wBitsPerSample  = nBitsPerSample;
   fmt.cbSize          = 0;

   res1 = waveOutOpen(
      NULL,                   // handle can be NULL for query
      WAVE_MAPPER,            // select a device able to handle the
                              //  requested format
      &fmt,                   // format
      (DWORD) NULL,           // no callback
      (DWORD) NULL,           // no instance data
      WAVE_FORMAT_QUERY);     // query only, do not open device

   res2 = waveInOpen(
      NULL,                   // handle can be NULL for query
      WAVE_MAPPER,            // select a device able to handle the
                              //  requested format
      &fmt,                   // format
      (DWORD) NULL,           // no callback
      (DWORD) NULL,           // no instance data
      WAVE_FORMAT_QUERY);     // query only, do not open device

   if (res1 == MMSYSERR_NOERROR && res2 == MMSYSERR_NOERROR)
      return 1;
   else
      return 0;
}

int checkFormat(int nChannels, int nSamplesPerSec, int nBitsPerSample)
{
   int good;
   good = isFormatSupported(nChannels, nSamplesPerSec, nBitsPerSample);
   printf(" %3d channels, %5d samples/sec, %2d bits/sample: is%s supported\n",
      nChannels, nSamplesPerSec, nBitsPerSample, good ? "" : " NOT");
   return good;
}

WAVEHDR* inPrePrep(int n, DWORD bufLen)
{
   WAVEHDR* pWH;
   int doAlloc = (hInHdr[n] == NULL);

   assert((n > -1) && (n < N_IN_BUFFERS));
   if (doAlloc) {
      hInHdr[n] = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, sizeof(WAVEHDR));
      assert(NULL != hInHdr[n]);
      hInBuf[n] = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, bufLen);
      assert(NULL != hInBuf[n]);
   }
   pInHdr[n] = pWH = (WAVEHDR*) GlobalLock(hInHdr[n]);
   assert(NULL != pInHdr[n]);
   pWH->lpData = (char*) GlobalLock(hInBuf[n]);
   pWH->dwBufferLength = bufLen;
   pWH->dwUser = n;
   pWH->dwBytesRecorded = 0;
   pWH->dwFlags = 0;
   pWH->dwLoops = 0;
   pWH->lpNext = 0;
   pWH->reserved = 0;
   if (doAlloc) {
         memset(pWH->lpData, 0, pWH->dwBufferLength);
   }
   return pWH;
}

static int frameCount = 0;

bool inPostUnprep(int n, int discard, DWORD bufLen)
{
   bool retVal = false;  //assume we didn't succeed for now

   WAVEHDR* pWH;
   MpBufPtr ob = NULL;

   static int iPU = 0;
   static int flushes = 0;

   assert((n > -1) && (n < N_IN_BUFFERS));
   pWH = pInHdr[n];
   assert(NULL!=pWH);
   
   if (bufLen == pWH->dwBytesRecorded)
   {

#ifdef DEBUG_WINDOZE /* [ */
      if (1) {
         static int micQLen[1024];
         int in = iPU % 1024;
         int i, j;

         micQLen[in] = MpMisc.pMicQ->numMsgs();
         if (in == 1023) {
            osPrintf("\n\n Microphone Queue lengths [%d,%d]:\n", iPU, frameCount);
            for (i=0; i<1024; i+=32) {
               for (j=i; j<(i+32); j++) {
                  osPrintf("%3d", micQLen[j]);
               }
               osPrintf("\n");
            }
            osPrintf("\n\n");
         }
      }
#endif /* DEBUG_WINDOZE ] */

      iPU++;

#ifdef DEBUG_WINDOZE /* [ */
      if (0 && (0 == (iPU % 1000))) {
         osPrintf("inPostUnprep(): %d records, %d flushes\n", iPU, flushes);
      }
#endif /* DEBUG_WINDOZE ] */

      if (!discard) {
         ob = MpBuf_getBuf(MpMisc.UcbPool, samplesPerFrame, 0, MP_FMT_T12);
      }
      if (!discard) {
         MpBufferMsg* pFlush;
         MpBufferMsg* pMsg;

   //    DWW took this assert out, because on windows, when you pull the usb dev out
   //    you can receive 0 bytes back
   //    assert(bufLen == pWH->dwBytesRecorded);
         memcpy(MpBuf_getSamples(ob), pWH->lpData, pWH->dwBytesRecorded);
#ifdef INSERT_SAWTOOTH /* [ */
         if (NULL == ob) { /* nothing in Q, or we are disabled */
            ob = MpBuf_getBuf(MpMisc.UcbPool, MpMisc.frameSamples, 0, MP_FMT_T12);
            if (NULL != ob) {
               int i, n;
               Sample *s;

               s = MpBuf_getSamples(ob);
               n = MpBuf_getNumSamples(ob);
               for (i=0; i<n; i++)
                   *s++= ((i % 80) << 10);
            }
         }
#endif /* INSERT_SAWTOOTH ] */


         pMsg = (MpBufferMsg*) DmaMsgPool->findFreeMsg();
         if (NULL == pMsg)
            pMsg = new MpBufferMsg(MpBufferMsg::AUD_RECORDED);

         pMsg->setMsgSubType(MpBufferMsg::AUD_RECORDED);
         pMsg->setTag(ob);
         pMsg->setBuf(MpBuf_getSamples(ob));
         pMsg->setLen(MpBuf_getNumSamples(ob));

         if (OS_SUCCESS != MpMisc.pMicQ->send(*pMsg, OsTime::NO_WAIT)) {
            OsStatus  res;
            flushes++;
            res = MpMisc.pMicQ->receive((OsMsg*&) pFlush, OsTime::NO_WAIT);
            if (OS_SUCCESS == res) {
               MpBuf_delRef(pFlush->getTag());
               pFlush->releaseMsg();
            } else {
               osPrintf("DmaTask: queue was full, now empty (3)!"
                  " (res=%d)\n", res);
            }
            if (OS_SUCCESS != MpMisc.pMicQ->send(*pMsg, OsTime::NO_WAIT)) {
               MpBuf_delRef(ob);
            }
         }
         if (!pMsg->isMsgReusable()) delete pMsg;
      }
      return true;
   }

   if (NULL != hInHdr[n]) {
      GlobalUnlock(hInHdr[n]);
      pInHdr[n] = NULL;
      GlobalFree(hInHdr[n]);
      hInHdr[n] = NULL;
   }
   if (NULL != hInBuf[n]) {
      GlobalUnlock(hInBuf[n]);
      GlobalFree(hInBuf[n]);
      hInBuf[n] = NULL;
   }

   return retVal;
}

static MpBufPtr conceal(MpBufPtr prev, int concealed)
{
#ifdef XXX_DEBUG_WINDOZE /* [ */
   MpBufPtr ret;
   Sample* src;
   Sample* dst;
   int len;
   int halfLen;
   int i;

   if (NULL == prev) {
      ret = MpBuf_getFgSilence();
      return ret;
   }
   len = MpBuf_getNumSamples(prev);
   ret = MpBuf_getBuf(MpBuf_getPool(prev), len, 0, MpBuf_getFormat(prev));
   src = MpBuf_getSamples(prev);
   dst = MpBuf_getSamples(ret);
   halfLen = (len + 1) >> 1;
   for (i=0; i<halfLen; i++) {
      dst[i] = src[len - i];
   }
   for (i=halfLen; i<len; i++) {
      dst[i] = src[i];
   }
   if (concealed > 2) {
      for (i=0; i<len; i++) {
         dst[i] = dst[i] >> 1; // attenuate
      }
   }
   return ret;
#else /* DEBUG_WINDOZE ] [ */
   return MpBuf_getFgSilence();
#endif /* DEBUG_WINDOZE ] */
}

WAVEHDR* outPrePrep(int n, DWORD bufLen)
{
   WAVEHDR* pWH;
   int doAlloc = (hOutHdr[n] == NULL);
   MpBufferMsg* msg;
   MpBufferMsg* pFlush;
   MpBufPtr     ob;

   static int oPP = 0;
   static MpBufPtr prev = NULL; // prev is for future concealment use
   static int concealed = 0; 

   static int flushes = 0;
   static int skip = 0;

   assert((n > -1) && (n < N_OUT_BUFFERS));

#ifdef DEBUG_WINDOZE /* [ */
   if (1) {
      static int spkQLen[1024];
      int in = oPP % 1024;
      int i, j;

      spkQLen[in] = MpMisc.pSpkQ->numMsgs();
      if (in == 1023) {
         osPrintf("\n\n Speaker Queue lengths [%d,%d]:\n  ", oPP, frameCount);
         for (i=0; i<1024; i+=32) {
            for (j=i; j<(i+32); j++) {
               osPrintf("%3d", spkQLen[j]);
            }
            osPrintf("\n  ");
         }
         osPrintf("\n\n");
      }
   }
#endif /* DEBUG_WINDOZE ] */

   oPP++;

#ifdef DEBUG_WINDOZE /* [ */
   if (0 && (0 == (oPP % 1000))) {
      osPrintf("outPrePrep(): %d playbacks, %d flushes\n", oPP, flushes);
   }
#endif /* DEBUG_WINDOZE ] */
   while (MprToSpkr::MAX_SPKR_BUFFERS < MpMisc.pSpkQ->numMsgs()) {
      OsStatus  res;
      flushes++;
      res = MpMisc.pSpkQ->receive((OsMsg*&) pFlush, OsTime::NO_WAIT);
      if (OS_SUCCESS == res) {
         MpBuf_delRef(pFlush->getTag());
         pFlush->releaseMsg();
      } else {
         osPrintf("DmaTask: queue was full, now empty (4)!"
            " (res=%d)\n", res);
      }
      if (flushes > 100) {
         osPrintf("outPrePrep(): %d playbacks, %d flushes\n", oPP, flushes);
         flushes = 0;
      }
   }

   if ((skip == 0) && (MprToSpkr::MIN_SPKR_BUFFERS > MpMisc.pSpkQ->numMsgs())) {
      skip = MprToSpkr::SKIP_SPKR_BUFFERS;
      assert(MprToSpkr::MAX_SPKR_BUFFERS >= skip);
#ifdef DEBUG_WINDOZE /* [ */
      osPrintf("Skip(%d,%d)\n", skip, oPP);
#endif /* DEBUG_WINDOZE ] */
   }

   ob = NULL;
   if (0 == skip) {
      if (OS_SUCCESS == MpMisc.pSpkQ->receive((OsMsg*&)msg, OsTime::NO_WAIT)) {
         ob = msg->getTag();
         msg->releaseMsg();
      }
   } else {
      if (MpMisc.pSpkQ->numMsgs() >= skip) skip = 0;
   }

   if (NULL == ob) {
      ob = conceal(prev, concealed);
      concealed++;
   } else {
      concealed = 0;
   }

   if (doAlloc) {
      hOutHdr[n] = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, sizeof(WAVEHDR));
      assert(NULL != hOutHdr[n]);
      hOutBuf[n] = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, bufLen);
      assert(NULL != hOutBuf[n]);
   }
   pOutHdr[n] = pWH = (WAVEHDR*) GlobalLock(hOutHdr[n]);
   assert(NULL != pOutHdr[n]);
   pWH->lpData = (char*) GlobalLock(hOutBuf[n]);
   pWH->dwBufferLength = bufLen;
   pWH->dwUser = n;
   pWH->dwBytesRecorded = 0;
   pWH->dwFlags = 0;
   pWH->dwLoops = 0;
   pWH->lpNext = 0;
   pWH->reserved = 0;
   memcpy(pWH->lpData, MpBuf_getSamples(ob), bufLen);
   MpBuf_delRef(prev);
   prev = ob;
   return pWH;
}

void outPostUnprep(int n)
{
   assert((n > -1) && (n < N_OUT_BUFFERS));
   assert(NULL != hOutHdr[n]);
   if (NULL != hOutHdr[n]) {
      GlobalUnlock(hOutHdr[n]);
      pOutHdr[n] = NULL;
      GlobalFree(hOutHdr[n]);
      hOutHdr[n] = NULL;
   }
   if (NULL != hOutBuf[n]) {
      GlobalUnlock(hOutBuf[n]);
      GlobalFree(hOutBuf[n]);
      hOutBuf[n] = NULL;
   }
}

/************************************************************************/
/*
 * dmaStartup -- Start the threads that generate the 10ms interval signal,
 *      and that handle the audio input and output.
 */

HANDLE hMicThread;
HANDLE hSpkrThread;

int showFrameCount(int silent)
{
    if (!silent) osPrintf("%d DMA Frames\n", frameCount, 0,0,0,0,0);
    return frameCount;
}

#if 0 /* [ */
// Start 10 msec interval generation

HANDLE hTimerThread;
HANDLE hTimerEvent;

void CALLBACK TimeProc(UINT uID, UINT uMsg, DWORD dwUser,
   DWORD dw1, DWORD dw2 )
{
   SetEvent(hTimerEvent);
}

int checkTimeCaps(UINT interval)
{
   TIMECAPS caps;
   MMRESULT res;

   res = timeGetDevCaps(&caps, sizeof(caps));
   printf("res = %d, PeriodMin = %d, PeriodMax = %d\n", res,
      caps.wPeriodMin, caps.wPeriodMax);
   return ((interval < caps.wPeriodMin) || (interval > caps.wPeriodMax));
}

unsigned int __stdcall TimerThread(LPVOID unused)
{
   MMRESULT timerID;
   OsStatus res;
   int  toggle;

   static int delayMS = 0; // 4000;

   if (checkTimeCaps(INTERVAL)) {
      printf("Timer does not support desired interval (%d)\n", INTERVAL);
      return 1;
   }

   if (delayMS) {
      osPrintf("TimerThread: About to delay %d milliseconds\n", delayMS);
      Sleep(delayMS);
      osPrintf("TimerThread: Proceeding...\n");
   }

   timeBeginPeriod(INTERVAL);

   hTimerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

   timerID = timeSetEvent(INTERVAL, RESOLUTION, TimeProc, 0, TIME_PERIODIC);

   if (0 != timerID) {
      printf("Timer started (ID=%d)...\n", timerID);
   } else {
      printf("timeSetEvent failed: %d\n", GetLastError());
      return 2;
   }

   frameCount = 0;
   toggle = 0;
   while (1) {
      WaitForSingleObject(hTimerEvent, 1000);
      if (1 == toggle) {
         res = MpMediaTask::signalFrameStart();
         switch (res) {
         case OS_ALREADY_SIGNALED:
            // Should bump missed frame statistic
            break;
         case OS_SUCCESS:
            frameCount++;
            toggle = 0;
            break;
         default:
            osPrintf("Frame %d, signalFrameStart() returned %d\n",
               frameCount, res);
            break;
         }
      } else {
         toggle = 1;
      }
   }

   if (0 != timerID) {
      timeKillEvent(timerID);
   }

   timeEndPeriod(INTERVAL);

   printf("Getting ready to exit ...\n");
   return 0;
}
// End 10 msec interval generation
#endif /* 0 ] */

HWAVEIN  audioInH;
HWAVEOUT audioOutH;

HWAVEOUT audioOutCallH;

UtlString gRingDevice;
UtlString gCallDevice;

int gRingDeviceId;
int gCallDeviceId;



int openAudioIn(HWAVEIN *pAudioInH,
                int nChannels, int nSamplesPerSec, int nBitsPerSample)
{
   WAVEFORMATEX fmt;
   MMRESULT     res;

   *pAudioInH = NULL;

   osPrintf("open audio in",1);
   fmt.wFormatTag      = WAVE_FORMAT_PCM;
   fmt.nChannels       = nChannels;
   fmt.nSamplesPerSec  = nSamplesPerSec;
   fmt.nAvgBytesPerSec = (nChannels * nSamplesPerSec * nBitsPerSample) / 8;
   fmt.nBlockAlign     = (nChannels * nBitsPerSample) / 8;
   fmt.wBitsPerSample  = nBitsPerSample;
   fmt.cbSize          = 0;

   res = waveInOpen(
      pAudioInH,              // handle (will be filled in)
       gCallDeviceId,         // attempt to open the "in call" device
      &fmt,                   // format
      (DWORD) myInOutCallBack,// callback entry
      GetCurrentThreadId(),   // instance data
      CALLBACK_FUNCTION);     // callback function specified
   
   //if we fail to open the audio device above, then we 
   //will try to open any device that will hande the requested format
   if (res != MMSYSERR_NOERROR)
   {
      res = waveInOpen(
      pAudioInH,              // handle (will be filled in)
      WAVE_MAPPER,            // select any device able to handle this format
      &fmt,                   // format
      (DWORD) myInOutCallBack,// callback entry
      GetCurrentThreadId(),   // instance data
      CALLBACK_FUNCTION);     // callback function specified
   }

   if (res != MMSYSERR_NOERROR)
   {
      showWaveError("waveInOpen", res, -1, __LINE__);
      waveInClose(*pAudioInH);
      *pAudioInH = NULL;
      return 0;
   }
   else
   {
      return 1;
   }
}

// This function will attempt to open a user specified audio device.
// If it fails, we will try to open any audio device that meets our requested format
// If we still fail, we will return 0, and display a message.
// TODO: Seems we have a problem when no audio device can be found. No calls can be made.
//       I recommend we bubble this up to java and display a message.
int openAudioOut(int desiredDeviceId, HWAVEOUT *pAudioOutH,int nChannels, int nSamplesPerSec, int nBitsPerSample)
{
   WAVEFORMATEX fmt;
   MMRESULT     res;



   *pAudioOutH = NULL;

   osPrintf("open audio out\n");

   fmt.wFormatTag      = WAVE_FORMAT_PCM;
   fmt.nChannels       = nChannels;
   fmt.nSamplesPerSec  = nSamplesPerSec;
   fmt.nAvgBytesPerSec = (nChannels * nSamplesPerSec * nBitsPerSample) / 8;
   fmt.nBlockAlign     = (nChannels * nBitsPerSample) / 8;
   fmt.wBitsPerSample  = nBitsPerSample;
   fmt.cbSize          = 0;

   res = waveOutOpen(
      pAudioOutH,             // handle (will be filled in)
      desiredDeviceId,        // select the device specified by user
      &fmt,                   // format
      (DWORD) myInOutCallBack,// callback entry
      GetCurrentThreadId(),   // instance data
      CALLBACK_FUNCTION);     // callback function specified

   //if we fail to open the audio device above, then we 
   //will try to open any device that will hande the requested format
   if (res != MMSYSERR_NOERROR)
	   res = waveOutOpen(
		  pAudioOutH,             // handle (will be filled in)
		  WAVE_MAPPER,            // select any device able to handle this format
		  &fmt,                   // format
		  (DWORD) myInOutCallBack,// callback entry
		  GetCurrentThreadId(),   // instance data
		  CALLBACK_FUNCTION);     // callback function specified
   
   if (res != MMSYSERR_NOERROR)
   {
      //hmm, couldn't open any audio device... things don't look good at this point
      showWaveError("waveOutOpen", res, -1, __LINE__);
      waveOutClose(*pAudioOutH);
      *pAudioOutH = NULL;
      return 0;
   }
   else
   {
      return 1;
   }
}



unsigned int __stdcall MicThread(LPVOID Unused)
{
   int      i;
   DWORD    bufLen = ((samplesPerFrame * bitsPerSample) / 8);
   WAVEHDR* pWH;
   MMRESULT ret;
   int      recorded;
   MSG      tMsg;
   int      get;
   int      n;
   int      looping;

#ifdef IHISTORY /* [ */
   WAVEHDR* lastWH[IHISTORY];
   int      lastInd[IHISTORY];
   int      last = 0;

   for (i=0;i<IHISTORY;i++) {
      lastWH[i] = 0;
      lastInd[i] = 0;
   }
#endif /* IHISTORY ] */

   osPrintf(" **********START MIC!**********\n");

   // open the input wave device

   if (openAudioIn(&audioInH, 1, samplesPerSecond, bitsPerSample))
   {
      osPrintf("Opened audio input channel\n\n");
   } else {
      osPrintf("Open of audio input channel failed\n\n");
      ResumeThread(hSpkrThread);
      return 1;
   }

   looping = 1;

   ret = waveInStart(audioInH);
   if (ret != MMSYSERR_NOERROR)
      showWaveError("waveInStart", ret, -1, __LINE__);

   osPrintf(" MicThread: preloading input device with %d frames\n", 
      smMicQPreload);

   for (i=0; i<smMicQPreload; i++) {
      pWH = inPrePrep(i, bufLen);
      ret = waveInPrepareHeader(audioInH, pWH, sizeof(WAVEHDR));
      if (ret != MMSYSERR_NOERROR)
         showWaveError("waveInPrepareHeader", ret, i, __LINE__);
      ret = waveInAddBuffer(audioInH, pWH, sizeof(WAVEHDR));
      if (ret != MMSYSERR_NOERROR)
         showWaveError("waveInAddBuffer", ret, i, __LINE__);
   }

   ResumeThread(hSpkrThread);

   frameCount = 0;
   recorded = 0;

   while (looping) {
      get = GetMessage(&tMsg, NULL, 0, 0);
      if (1 == get) {
         switch (tMsg.message) {
         case WIM_OPEN:
            break;
         case WIM_DATA:
            pWH = (WAVEHDR *) tMsg.wParam;
            n = (pWH->dwUser) & USER_BUFFER_MASK;

#ifdef IHISTORY /* [ */
            lastWH[last] = pWH;
            lastInd[last] = n;
            last = (last + 1) % IHISTORY;

            if (N_IN_BUFFERS == recorded) {
               osPrintf("after first %d buffers:", recorded + 1);
               osPrintf("\nCall Backs:");
               for (i=0; i<IHISTORY; i++) {
                  osPrintf("%c%3d", (0 == (i % 20)) ? '\n' : ' ',
                     histIn[i]);
               }
               osPrintf("\n\nMessages:");
               for (i=0; i<IHISTORY; i++) {
                  osPrintf("%c%3d", (0 == (i % 20)) ? '\n' : ' ',
                     lastInd[i]);
               }
               osPrintf("\n");
            }
#endif /* IHISTORY ] */

            ret = waveInUnprepareHeader(audioInH, pWH, sizeof(WAVEHDR));
            if (ret != MMSYSERR_NOERROR)
               showWaveError("waveInUnprepareHeader", ret, recorded, __LINE__);

            if (MpDma_isMuteEnabled())
               memset(pWH->lpData,0,pWH->dwBytesRecorded); /* clear it out */


            if (inPostUnprep(n, FALSE, bufLen))
            {
               pWH = inPrePrep(n, bufLen);
               ret = waveInPrepareHeader(audioInH, pWH, sizeof(WAVEHDR));
               if (ret != MMSYSERR_NOERROR)
                  showWaveError("waveInPrepareHeader", ret, recorded, __LINE__);

               ret = waveInAddBuffer(audioInH, pWH, sizeof(WAVEHDR));

               if (ret != MMSYSERR_NOERROR)
                  showWaveError("waveInAddBuffer", ret, recorded, __LINE__);
               recorded++;
            }

            break;
         case WIM_CLOSE:
            osPrintf("Input channel closed...\n");
            looping = 0;
            break;
         }
      } else {
         if (get < 0) {
            osPrintf("\nGetMessage (Mic) returned -1!\n");
         }
         looping = 0;
      }
   }

   osPrintf(" ***********STOP!**********\n");

   ret = waveInReset(audioInH);
   if (ret != MMSYSERR_NOERROR)
      showWaveError("waveInReset", ret, -1, __LINE__);
   ret = waveInStop(audioInH);
   if (ret != MMSYSERR_NOERROR)
      showWaveError("waveInStop", ret, -1, __LINE__);

   for (i=0; i<N_IN_BUFFERS; i++) {
      if (NULL != hInHdr[i]) {
         ret = waveInUnprepareHeader(audioInH, pInHdr[i], sizeof(WAVEHDR));
         if (ret != MMSYSERR_NOERROR)
            showWaveError("waveInUnprepareHeader", ret, i, __LINE__);
         inPostUnprep(i, TRUE, bufLen);
      }
   }

   osPrintf("closing input channel\n");
   ret = waveInClose(audioInH);
   if (ret != MMSYSERR_NOERROR)
      showWaveError("waveInClose", ret, -1, __LINE__);
   audioInH = NULL;

   return 0;
}


unsigned int __stdcall SpkrThread(LPVOID Unused)
{
   int      i;
   DWORD    bufLen = ((samplesPerFrame * bitsPerSample) / 8);
   WAVEHDR* pWH;
   MMRESULT ret;
   int      played;
   MSG      tMsg;
   int      get;
   int      n;
   int      looping;
   static bool sLastRingerEnabled = false;
   OsStatus res;

#ifdef OHISTORY /* [ */
   WAVEHDR* lastWH[OHISTORY];
   int      lastInd[OHISTORY];
   int      last = 0;

   for (i=0;i<OHISTORY;i++) {
      lastWH[i] = 0;
      lastInd[i] = 0;
   }
#endif /* OHISTORY ] */

   osPrintf(" **********START SPKR!**********\n");


	// set the different device ids
	gRingDeviceId = WAVE_MAPPER;
	gCallDeviceId = WAVE_MAPPER;

	int ii;
	WAVEOUTCAPS devcaps;

	int numberOfDevicesOnSystem = waveOutGetNumDevs();

	for(ii=0; ii<numberOfDevicesOnSystem; ii++)
	{
		waveOutGetDevCaps(ii,&devcaps,sizeof(WAVEOUTCAPS));
		if (strcmp(devcaps.szPname,gRingDevice)==0) 
      {
         gRingDeviceId = ii;
         osPrintf("Selected ring device: %s\n",devcaps.szPname);
      }
		if (strcmp(devcaps.szPname,gCallDevice)==0) 
      {
         gCallDeviceId = ii;
         osPrintf("Selected call device: %s\n",devcaps.szPname);
      }
	}


   //This function tries to open the specified device using an enumerated id.
   //If it fails, internally it will switch to the system default device
   //It returns a handle to the device
   if (openAudioOut(gRingDeviceId,&audioOutH, 1, samplesPerSecond, bitsPerSample))
   {
      osPrintf("Opened Ring audio output channel\n\n");
   } else {
      osPrintf("Open of Ring audio output channel failed\n\n");
      ResumeThread(hMicThread);
      return 1;
   }

   //Tthis function tries to open the specified device using an enumerated id.
   //If it fails, internally it will switch to the system default device
   //It returns a handle to the device
   if (openAudioOut(gCallDeviceId,&audioOutCallH, 1, samplesPerSecond, bitsPerSample))
   {
      osPrintf("Opened Call audio output channel\n\n");
   } else {
      osPrintf("Open of Call audio output channel failed\n\n");
      ResumeThread(hMicThread);
      return 1;
   }


   looping = 1;

   osPrintf(" SpkrThread: preloading output device with %d frames\n", 
      smSpkrQPreload);

   
   HWAVEOUT hOut = NULL;
   for (i=0; i<smSpkrQPreload; i++)
   {
      pWH = outPrePrep(i, bufLen);

      // if the ringer is enabled and we have a valid audio out device
      // then use that device
      if (MpDma_isRingerEnabled() && audioOutH)
        hOut = audioOutH;     
	   else
      // else if we have a "in call" device, then use that
      if (audioOutCallH)
         hOut = audioOutCallH;
      else 
      // finally, just use the default device
        hOut = audioOutH;
      
      if (hOut)
      {
	      ret = waveOutPrepareHeader(hOut, pWH, sizeof(WAVEHDR));
         if (ret != MMSYSERR_NOERROR)
            showWaveError("waveOutPrepareHeader", ret, i, __LINE__);
	      ret = waveOutWrite(hOut, pWH, sizeof(WAVEHDR));
         if (ret != MMSYSERR_NOERROR)
   	      showWaveError("waveOutWrite", ret, i, __LINE__);
      }
      
   }





   osPrintf("\n\n*** SpkrThread: WOM_OPEN=%d, WOM_DONE=%d, WOM_CLOSE=%d\n\n",
      WOM_OPEN, WOM_DONE, WOM_CLOSE);

   ResumeThread(hMicThread);

   played = 0;
   

   while (looping) {
      get = GetMessage(&tMsg, NULL, 0, 0);
      
      //when switching devices, ringer to in-call we need to make 
      //sure any outstanding buffers are flushed
      if (sLastRingerEnabled != MpDma_isRingerEnabled())
      {
         if (audioOutH)
            waveOutReset(audioOutH);
         if (audioOutCallH)
            waveOutReset(audioOutCallH);
      }

      if (1 == get) {
         switch (tMsg.message) {
         case WOM_OPEN:
            osPrintf("WOM_OPEN\n");
            break;
         case WOM_DONE:
            pWH = (WAVEHDR *) tMsg.wParam;
            n = (pWH->dwUser) & USER_BUFFER_MASK;

#ifdef OHISTORY /* [ */
            lastWH[last] = pWH;
            lastInd[last] = n;
            last = (last + 1) % OHISTORY;

            if (N_OUT_BUFFERS == played) {
               osPrintf("after first %d buffers:", played + 1);
               osPrintf("\nCall Backs:");
               for (i=0; i<OHISTORY; i++) {
                  osPrintf("%c%3d", (0 == (i % 20)) ? '\n' : ' ',
                     histOut[i]);
               }
               osPrintf("\n\nMessages:");
               for (i=0; i<OHISTORY; i++) {
                  osPrintf("%c%3d", (0 == (i % 20)) ? '\n' : ' ',
                     lastInd[i]);
               }
               osPrintf("\n");
            }
#endif /* OHISTORY ] */

         // if the ringer is enabled and we have a valid audio out device
         // then use that device
			if (MpDma_isRingerEnabled() && audioOutH)
            hOut = audioOutH;
			else
         //else use the "in call" device
         if (audioOutCallH)
            hOut = audioOutCallH;
         else
            hOut = audioOutH;

         if (hOut)
         {
			   ret = waveOutUnprepareHeader(hOut, pWH, sizeof(WAVEHDR));
            if (ret != MMSYSERR_NOERROR)
   				showWaveError("waveOutUnprepareHeader", ret, played, __LINE__);
				outPostUnprep(n);
				pWH = outPrePrep(n, bufLen);

				ret = waveOutPrepareHeader(hOut, pWH, sizeof(WAVEHDR));
            if (ret != MMSYSERR_NOERROR)
   				showWaveError("waveOutPrepareHeader", ret, played, __LINE__);
				ret = waveOutWrite(hOut, pWH, sizeof(WAVEHDR));
            if (ret != MMSYSERR_NOERROR)
   				showWaveError("waveOutWrite", ret, played, __LINE__);
				played++;
			}


         res = MpMediaTask::signalFrameStart();
         switch (res) {
            case OS_SUCCESS:
            frameCount++;
            break;
#ifdef DEBUG_WINDOZE /* [ */
            case OS_LIMIT_REACHED:
               // Should bump missed frame statistic
               osPrintf(" Frame %d: OS_LIMIT_REACHED\n", frameCount);
               break;
            case OS_WAIT_TIMEOUT:
               // Should bump missed frame statistic
               osPrintf(" Frame %d: OS_WAIT_TIMEOUT\n", frameCount);
               break;
            case OS_ALREADY_SIGNALED:
               // Should bump missed frame statistic
               osPrintf(" Frame %d: OS_ALREADY_SIGNALED\n", frameCount);
               break;
            default:
               osPrintf("Frame %d, signalFrameStart() returned %d\n",
                  frameCount, res);
               break;
#else /* DEBUG_WINDOZE ] [ */
            case OS_LIMIT_REACHED:
            case OS_WAIT_TIMEOUT:
            case OS_ALREADY_SIGNALED:
            default:
               // Should bump missed frame statistic
               break;
#endif /* DEBUG_WINDOZE ] */
            }

            break;
         case WOM_CLOSE:
            osPrintf("WOM_CLOSE\n");
            osPrintf("Output channel closed...\n");
            looping = 0;
            break;
         default:
            osPrintf("SpkrThread: Msg=%d\n", tMsg.message);
            break;
         }
      } else {
         if (get < 0) {
            osPrintf("\nGetMessage returned %d!\n", get);
         }
         looping = 0;
      }
      
      // record our last ringer state
      sLastRingerEnabled = MpDma_isRingerEnabled();

   }

   osPrintf(" ***********STOP!**********\n");


   if (audioOutH)
   {
      ret = waveOutReset(audioOutH);
      if (ret != MMSYSERR_NOERROR)
         showWaveError("waveOutReset", ret, -1, __LINE__);

      for (i=0; i<N_OUT_BUFFERS; i++) {
         if (NULL != hOutHdr[i]) {
            ret = waveOutUnprepareHeader(audioOutH, pOutHdr[i], sizeof(WAVEHDR));
            if (ret != MMSYSERR_NOERROR)
               showWaveError("waveOutUnprepareHeader", ret, i, __LINE__);
            outPostUnprep(i);
         }
      }

      osPrintf("closing Ring output channel\n");
      ret = waveOutClose(audioOutH);
      if (ret != MMSYSERR_NOERROR)
         showWaveError("waveOutClose", ret, -1, __LINE__);
      audioOutH = NULL;
   }


   if (audioOutCallH)
   {
      ret = waveOutReset(audioOutCallH);
      if (ret != MMSYSERR_NOERROR)
         showWaveError("waveOutReset", ret, -1, __LINE__);

      for (i=0; i<N_OUT_BUFFERS; i++) {
         if (NULL != hOutHdr[i]) {
            ret = waveOutUnprepareHeader(audioOutCallH, pOutHdr[i], sizeof(WAVEHDR));
            if (ret != MMSYSERR_NOERROR)
               showWaveError("waveOutUnprepareHeader", ret, i, __LINE__);
            outPostUnprep(i);
         }
      }

      osPrintf("closing Call output channel\n");
      ret = waveOutClose(audioOutCallH);
      if (ret != MMSYSERR_NOERROR)
         showWaveError("waveOutClose", ret, -1, __LINE__);
      audioOutH = NULL;
   }



   return 0;
}

OsStatus dmaStartup(int samplesPerFrame)
{
   UINT dummyThreadID;
   int   i;


#define TRYING_PERFORMANCE_COUNTERS
#undef TRYING_PERFORMANCE_COUNTERS
#ifdef TRYING_PERFORMANCE_COUNTERS /* [ */
   {
      const int NUMQUERIES = 64;
      DWORD le;
      // LARGE_INTEGER curCount; // = 0xfedcba9876543210;
      LARGE_INTEGER curCounts[NUMQUERIES];
      int i;
      LARGE_INTEGER curFreq; //  = 0x0123456789abcdef;
      // int initCount[2] = {0x76543210, 0xfedcba98};
      // int initFreq[2] = {0x89abcdef, 0x01234567};

      // memcpy(&curCount, initCount, 8);
      // memcpy(&curFreq, initFreq, 8);
      // osPrintf(" Query Cal: 0x%08x%08x, 0x%08x%08x\n", curCount.HighPart,
         // curCount.LowPart, curFreq.HighPart, curFreq.LowPart);
      if (QueryPerformanceFrequency(&curFreq)) {
         osPrintf(" QueryPerformanceFrequency: 0x%08x%08x = %d\n",
         curFreq.HighPart, curFreq.LowPart, curFreq.LowPart);
      } else {
         le = GetLastError();
         osPrintf(" QueryPerformanceFrequency returned error = %d\n", le);
      }
      for (i=0; i<NUMQUERIES; i++) {
         if (!QueryPerformanceCounter(&(curCounts[i]))) {
            le = GetLastError();
            osPrintf(" QueryPerformanceCounter returned error = %d\n", le);
            curCounts[i].HighPart = curCounts[i].LowPart = 0;
         }
      }
      le = curCounts[0].LowPart;
      osPrintf(" QueryPerformanceCounter[0]:  0x%08x%08x\n",
         curCounts[0].HighPart, curCounts[0].LowPart);
      for (i=1; i<NUMQUERIES; i++) {
         osPrintf(" QueryPerformanceCounter[%d]:%s 0x%08x%08x (%d)\n",
            i, ((i<10) ? " " : ""),
            curCounts[i].HighPart, curCounts[i].LowPart,
            curCounts[i].LowPart - le);
         le = curCounts[i].LowPart;
      }
   }
#endif /* TRYING_PERFORMANCE_COUNTERS ] */

    
/* commenting this out. DWW.  Although this doesn make the audio better, 
   John M. confirmed that closing the jre window at this priority causes
   his machine to reboot.  We can't have that, now can we!
   We may want to revisit this code and find out why it made his system crash, because 
  the audio is pretty nice
*/
/*
   if (!SetPriorityClass(GetCurrentProcess(),REALTIME_PRIORITY_CLASS))
       osPrintf("!!!!!!! Could not set process priority to HIGH !!!!!!!\n");
   else
       osPrintf("Successfully change priority to high.\n");
*/

        // DmaTask* pDma;

        // pDma = DmaTask::getDmaTask(samplesPerFrame);
        // return (NULL != pDma) ? OS_SUCCESS : OS_UNSPECIFIED;

   if (!checkFormat(1, samplesPerSecond, bitsPerSample)) {
      printf("Requested format is not supported!  Quitting.\n");
      return OS_INVALID_ARGUMENT;
   }

   {
      // UtlString* DmaName = new UtlString("DmaPool");
      MpBufferMsg* pMsg = new MpBufferMsg(MpBufferMsg::AUD_RECORDED);
      DmaMsgPool = new OsMsgPool("DmaTask", (*(OsMsg*)pMsg),
                          40, 60, 100, 5,
                          OsMsgPool::SINGLE_CLIENT);
   }

#if 0 /* [ */
   // start a thread and a timer callback to signal the 10 msec intervals

   hTimerThread = (void *)_beginthreadex(
      NULL,             // pointer to thread security attributes
      16000,            // initial thread stack size, in bytes
      TimerThread,      // pointer to thread function
      (LPVOID) 0,       // argument for new thread
      CREATE_SUSPENDED, // creation flags
      &dummyThreadID    // pointer to returned thread identifier
   );

   assert(NULL != hTimerThread);
#endif /* 0 ] */

   for (i=0; i<N_IN_BUFFERS; i++) {
      hInHdr[i] = hInBuf[i] = NULL;
      pInHdr[i] = NULL;
   }

   for (i=0; i<N_OUT_BUFFERS; i++) {
      hOutHdr[i] = hOutBuf[i] = NULL;
      pOutHdr[i] = NULL;
   }

#ifdef IHISTORY /* [ */
   memset(histIn, 0xff, sizeof(histIn));
   lastIn = 0;
#endif /* IHISTORY ] */

#ifdef OHISTORY /* [ */
   memset(histOut, 0xff, sizeof(histOut));
   lastOut = 0;
#endif /* OHISTORY ] */

   // start a thread to receive microphone input
   // mic thread will prime the device input queue

   hMicThread = (void *)_beginthreadex(
      NULL,             // pointer to thread security attributes
      16000,            // initial thread stack size, in bytes
      MicThread,        // pointer to thread function
      (LPVOID) 0,       // argument for new thread
      CREATE_SUSPENDED, // creation flags
      &dummyThreadID    // pointer to returned thread identifier
   );

   assert(NULL != hMicThread);

   // start a thread to send audio out to the speaker
   // speaker thread will prime the device output queue

   hSpkrThread = (void *)_beginthreadex(
      NULL,             // pointer to thread security attributes
      16000,            // initial thread stack size, in bytes
      SpkrThread,       // pointer to thread function
      (LPVOID) 0,       // argument for new thread
      CREATE_SUSPENDED, // creation flags
      &dummyThreadID    // pointer to returned thread identifier
   );

   assert(NULL != hSpkrThread);

   // All these threads were started with the SUSPENDED option so that
   // the following thread priority manipulations can happen without
   // yielding the CPU.  They will be resumed soon, but see the comment
   // next below...

   SetThreadPriority(hSpkrThread, THREAD_PRIORITY_TIME_CRITICAL);
   SetThreadPriority(hMicThread, THREAD_PRIORITY_TIME_CRITICAL);

#if 0 /* [ */
   SetThreadPriority(hTimerThread, THREAD_PRIORITY_TIME_CRITICAL);
   ResumeThread(hTimerThread);
#endif /* 0 ] */

   // Both the Microphone thread and the Speaker thread issue resume
   // commands for the other thread (Mic resumes Spkr, Spkr resumes Mic).
   // (Resuming a running thread is harmless).
   //
   // Exactly one of the two threads should be resumed here, and that one
   // will get the first opportunity to open its side of the audio device.
   // Once it has done so, it will issue the resume command for the other
   // thread, so that on systems with half-duplex audio devices we will be
   // consistent about which device will be opened and which will fail.
   //
   // Currently, we start the Speaker thread first, so that we will be
   // sure to open the output device on a half-duplex system.  If it is
   // decided that we want to open only the input device in such situations,
   // REPLACE the next statement with "ResumeThread(hMicThread);"

   ResumeThread(hSpkrThread);

   return OS_SUCCESS;
}

#endif /* WIN32 ] */

#ifdef __pingtel_on_posix__ /* [ */
/* OK, so here's how this file is supposed to work as I understand it - To
 * start up audio hardware services, the outside world will call dmaStartup()
 * which is supposed to initialize the audio hardware, starting threads if
 * necessary (the thread that calls dmaStartup() needs it to return quickly,
 * and not be interrupted in the future by any timers dmaStartup() might set
 * up), and begin calling MpMediaTask::signalFrameStart() every 10 ms. Now all
 * we have to do is read from the audio device and start sending MpBufferMsg
 * messages into MpMisc.pMicQ (an OsMsgQ), and read messages from MpMisc.pSpkQ
 * and write to the audio device. Nice references for this queue interaction
 * are the outPrePrep() and inPostUnprep() functions in the WIN32 code - they
 * demonstrate the OsMsgPool implementation to avoid repeated malloc() and
 * free() calls that not only slow the system down (about 600 per second
 * without the OsMsgPool) but also fragment system memory. The pool is
 * initialized in dmaStartup(). */

/*
--- Sample pMicQ send clipped from WIN32 code ---

      pMsg = (MpBufferMsg*) DmaMsgPool->findFreeMsg();
      if (NULL == pMsg)
         pMsg = new MpBufferMsg(MpBufferMsg::AUD_RECORDED);

      pMsg->setMsgSubType(MpBufferMsg::AUD_RECORDED);
      pMsg->setTag(ob);
      pMsg->setBuf(MpBuf_getSamples(ob));
      pMsg->setLen(MpBuf_getNumSamples(ob));

      if (OS_SUCCESS != MpMisc.pMicQ->send(*pMsg, OsTime::NO_WAIT)) {
         OsStatus  res;
         flushes++;
         res = MpMisc.pMicQ->receive((OsMsg*&) pFlush, OsTime::NO_WAIT);
         if (OS_SUCCESS == res) {
            MpBuf_delRef(pFlush->getTag());
            pFlush->releaseMsg();
         } else {
            osPrintf("DmaTask: queue was full, now empty (4)!"
               " (res=%d)\n", res);
         }
         if (OS_SUCCESS != MpMisc.pMicQ->send(*pMsg, OsTime::NO_WAIT)) {
            MpBuf_delRef(ob);
         }
      }
      if (!pMsg->isMsgReusable()) delete pMsg;

--- End Sample ---
*/

/* What follows is the generic POSIX sound driver interface. OS-specific
 * details are enclosed in #ifdef blocks further below. */

#include "os/OsMsgPool.h"
#include "mp/MpMediaTask.h"
#include "mp/dmaTask.h"
#include "os/OsTime.h"
#include "os/OsTimer.h"
#include "os/OsCallback.h"

#define SAMPLES_PER_SEC 8000
#define MIN_SAMPLE_RATE 100
#define BITS_PER_SAMPLE 16
#define N_SAMPLES 80

#define BUFLEN (N_SAMPLES*(BITS_PER_SAMPLE>>3))

/* If sound driver startup fails, we can revert to the dummy output driver. */
/* Also in the future maybe a flag that can be set at runtime will enable this. */
OsStatus dummyDriver_dmaStartup(int samplesPerFrame);
void dummyDriver_dmaShutdown(void);

/* Local variables */

static OsMsgPool* DmaMsgPool = NULL;

/* We want to keep these around to be able to shutdown... */
/* in case we should ever decide to do that. */
static OsCallback * pNotifier;
static OsTimer * pTimer;

static bool dmaOnline = 0;
static int frameCount = 0;

int showFrameCount(int silent)
{
   if (!silent)
      osPrintf("%d DMA Frames\n", frameCount);
   return frameCount;
}

static void mediaSignaller(const int userData, const int eventData)
{
   /* here's our 10ms call */
   frameCount++;
   MpMediaTask::signalFrameStart();
}

/* These systems have real sound drivers which are very similar. The common
 * functionality is included in this block. */
#if defined(__linux__) || defined(sun) /* [ */

// SYSTEM INCLUDES
#include <pthread.h> /* we're already fairly os-specific, just use pthreads */
#include <time.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <semaphore.h>

// APPLICATION INCLUDES

#include "mp/MpBufferMsg.h"
#include "mp/MpBuf.h"
#include "mp/MpMisc.h"
#include "mp/MprToSpkr.h"

/* Force the threads to alternate read/write */
static sem_t read_sem, write_sem;

static int soundCard = -1;

/* Only for Linux. Probably not needed. Needs root priveleges anyway. */
//#define REALTIME_LINUX_AUDIO_THREADS

#ifdef REALTIME_LINUX_AUDIO_THREADS
static void getRealtimePriority(void)
{
   struct sched_param realtime;
   int res;
   realtime.sched_priority=1;
   res = sched_setscheduler(0, SCHED_RR, &realtime);
   assert(res == 0);
}
#endif /* REALTIME_LINUX_AUDIO_THREADS */

static void * soundCardReader(void * arg)
{
   MpBufferMsg* pMsg;
   MpBufferMsg* pFlush;
   MpBufPtr ob;
   Sample* buffer;
   int recorded;
   int justRead;

#ifdef REALTIME_LINUX_AUDIO_THREADS
   getRealtimePriority();
#endif /* REALTIME_LINUX_AUDIO_THREADS */
   osPrintf(" **********START MIC!**********\n");

   while(dmaOnline)
   {
      ob = MpBuf_getBuf(MpMisc.UcbPool, N_SAMPLES, 0, MP_FMT_T12);
      assert(ob != NULL);
      buffer = MpBuf_getSamples(ob);
      recorded = 0;
      sem_wait(&read_sem);
      while(recorded < N_SAMPLES)
      {
         justRead = read(soundCard, &buffer[recorded], BUFLEN - (recorded * sizeof(Sample)));

         assert(justRead > 0);
         recorded += justRead/sizeof(Sample);
      }
      sem_post(&write_sem);

      if (MpDma_isMuteEnabled())
         memset(buffer, 0, sizeof(Sample) * N_SAMPLES); /* clear it out */

      assert(recorded == N_SAMPLES);

      pMsg = (MpBufferMsg*) DmaMsgPool->findFreeMsg();
      if(pMsg == NULL)
         pMsg = new MpBufferMsg(MpBufferMsg::AUD_RECORDED);

      pMsg->setMsgSubType(MpBufferMsg::AUD_RECORDED);
      pMsg->setTag(ob);
      pMsg->setBuf(MpBuf_getSamples(ob));
      pMsg->setLen(MpBuf_getNumSamples(ob));

      if(MpMisc.pMicQ->send(*pMsg, OsTime::NO_WAIT) != OS_SUCCESS)
      {
         OsStatus  res;
         res = MpMisc.pMicQ->receive((OsMsg*&) pFlush, OsTime::NO_WAIT);
         if (OS_SUCCESS == res) {
            MpBuf_delRef(pFlush->getTag());
            pFlush->releaseMsg();
         } else {
            osPrintf("DmaTask: queue was full, now empty (5)!"
               " (res=%d)\n", res);
         }
         if(MpMisc.pMicQ->send(*pMsg, OsTime::NO_WAIT) != OS_SUCCESS)
            MpBuf_delRef(ob);
      }
      if(!pMsg->isMsgReusable())
         delete pMsg;
   }

   osPrintf(" ***********STOP!**********\n");
   return NULL;
}

#define timediff(early, late) ((late.tv_sec-early.tv_sec)*1000000+(late.tv_usec-early.tv_usec))

static void * soundCardWriter(void * arg)
{
   struct timeval start;
   int framesPlayed;
   
#ifdef REALTIME_LINUX_AUDIO_THREADS
   getRealtimePriority();
#endif /* REALTIME_LINUX_AUDIO_THREADS */
   osPrintf(" **********START SPKR!**********\n");
   
   gettimeofday(&start, NULL);
   framesPlayed = 0;
   
   while(dmaOnline)
   {
      MpBufferMsg* pMsg;
      MpBufPtr ob;
      Sample last_buffer[N_SAMPLES] = {0};

      /* write to the card */

      struct timeval now;
      bool playFrame;

      playFrame = 0;
      gettimeofday(&now, NULL);
      if(now.tv_sec > start.tv_sec + 5)
      {
         /* we're really behind (more than 5 seconds) */
         playFrame = 1;
         start = now;
         framesPlayed = 0;
         osPrintf("soundCardWriter resetting output synchronization\n");
      }
      else
      {
         int delta = timediff(start, now);
         /* avoid overflowing 31 bits by doing a divide by 100 first,
            then dividing by only 10000 at the end */
         int targetFrames = (delta / 100) * SAMPLES_PER_SEC / N_SAMPLES / 10000;
         if(framesPlayed <= targetFrames)
            playFrame = 1;
         if(delta > 1000000)
         {
            start.tv_sec++;
            /* SAMPLES_PER_SEC should be an integer multiple of N_SAMPLES... */
            framesPlayed -= SAMPLES_PER_SEC / N_SAMPLES;
         }
      }

      if(MpMisc.pSpkQ->receive((OsMsg*&) pMsg, OsTime::NO_WAIT) == OS_SUCCESS)
      {
         ob = (MpBufPtr) pMsg->getTag();
         assert(ob != NULL);
         if(playFrame)
         {
            int played = 0;
            Sample* buffer = MpBuf_getSamples(ob);
            
            /* copy the buffer for skip protection */
            memcpy(&last_buffer[N_SAMPLES / 2], &buffer[N_SAMPLES / 2], BUFLEN / 2);
            
            sem_wait(&write_sem);
            while(played < N_SAMPLES)
            {
               int justWritten;
               justWritten = write(soundCard, &buffer[played], BUFLEN - (played * sizeof(Sample)));
               assert(justWritten > 0);
               played += justWritten/sizeof(Sample);
            }
            sem_post(&read_sem);
            assert(played == N_SAMPLES);
            framesPlayed++;
         }
         else
            osPrintf("soundCardWriter dropping sound packet\n");

         MpBuf_delRef(ob);
         pMsg->releaseMsg();
      }
      else if(playFrame)
      {
         int played = 0;
         //osPrintf("soundCardWriter smoothing over audio delay\n");
         
         /* play half of the last sample backwards, then forward again */
         for(int i = 0; i != N_SAMPLES / 2; i++)
            last_buffer[i] = last_buffer[N_SAMPLES - i - 1];
         
         sem_wait(&write_sem);
         while(played < N_SAMPLES)
         {
            int justWritten;
            justWritten = write(soundCard, &last_buffer[played], BUFLEN - (played * sizeof(Sample)));
            assert(justWritten > 0);
            played += justWritten/sizeof(Sample);
         }
         sem_post(&read_sem);
         assert(played == N_SAMPLES);
      }
   }
   
   osPrintf(" ***********STOP!**********\n");
   return NULL;
}

/* This will be defined by the OS-specific section below. */
static int setupSoundCard(void);

/* Because we don't have the kind of notification provided by directly talking
* to the DMA to know when sound has played back and when it's ready to be read,
* it's easiest to start up two threads and have one handle each side of the
* hardware. This also simplifies the complicated timing code necessary to make
* sure that when we play back audio, it plays at the right time - since Linux
* and Solaris have built in sound buffers, if we send it audio too fast then
* the audio playing out the sound card will be delayed outside of our code by
* the readback buffer in the sound driver. */

OsStatus dmaStartup(int samplesPerFrame)
{
   int res;
   pthread_t thread;
   OsTime tenMilliseconds(0, 10000); /* 10 ms = 10000 us */
   
   dmaOnline = 1;
   soundCard = setupSoundCard();
   /* Revert to the dummy driver as we could not open the sound card. */
   if(soundCard == -1)
      return dummyDriver_dmaStartup(samplesPerFrame);

   pNotifier = new OsCallback(0, mediaSignaller);
   pTimer = new OsTimer(*pNotifier);

   pTimer->periodicEvery(OsTime::NO_WAIT, tenMilliseconds);

   /* OsMsgPool setup */
   MpBufferMsg* pMsg = new MpBufferMsg(MpBufferMsg::AUD_RECORDED);
   DmaMsgPool = new OsMsgPool("DmaTask", *(OsMsg*)pMsg,
         40, 60, 100, 5,
         OsMsgPool::SINGLE_CLIENT);

   /* let the read thread go first */
   sem_init(&write_sem, 0, 0);
   sem_init(&read_sem, 0, 1);
   
   /* Start the reader and writer threads */
   res = pthread_create(&thread, NULL, soundCardReader, NULL);
   assert(res == 0);
   pthread_detach(thread);
   res = pthread_create(&thread, NULL, soundCardWriter, NULL);
   assert(res == 0);
   pthread_detach(thread);
   
   return OS_SUCCESS;
}

void dmaShutdown(void)
{
   if(dmaOnline)
   {
      pTimer->stop();
      delete pTimer;
      delete pNotifier;
      dmaOnline = 0;
      /* make sure the threads aren't wedged */
      sem_post(&read_sem);
      sem_post(&write_sem);
      close(soundCard);
      soundCard = -1;
      sem_destroy(&read_sem);
      sem_destroy(&write_sem);
   }
}

#endif /* __linux__ || sun ] */

/* Now the OS-specific sections... */
#ifdef __linux__ /* [ */

#include <sys/types.h>
#include <sys/soundcard.h>

static int setupSoundCard(void)
{
   int res, fd;
   int fragment = 0x00040008; /* magic number, reduces latency (0x0004 dma buffers of 2 ^ 0x0008 = 256 bytes each) */
   int samplesize = BITS_PER_SAMPLE;
   int stereo = 0; /* mono */
   int speed = SAMPLES_PER_SEC;
   
   fd = open("/dev/dsp", O_RDWR);
   if(fd == -1)
   {
      osPrintf("OSS: could not open /dev/dsp; *** NO SOUND! ***\n");
      return -1;
   }

   res = ioctl(fd, SNDCTL_DSP_SETDUPLEX, 0);
   if(res == -1)
   {
      osPrintf("OSS: could not set full duplex; *** NO SOUND! ***\n");
      close(fd);
      return -1;
   }

   res = ioctl(fd, SNDCTL_DSP_SETFRAGMENT, &fragment);
   if(res == -1)
   {
      osPrintf("OSS: could not set fragment size; *** NO SOUND! ***\n");
      close(fd);
      return -1;
   }
   
   res = ioctl(fd, SNDCTL_DSP_SAMPLESIZE, &samplesize);
   if(res == -1 || samplesize != BITS_PER_SAMPLE)
   {
      osPrintf("OSS: could not set sample size; *** NO SOUND! ***\n");
      close(fd);
      return -1;
   }
   res = ioctl(fd, SNDCTL_DSP_STEREO, &stereo);
   if(res == -1)
   {
      osPrintf("OSS: could not set single channel audio; *** NO SOUND! ***\n");
      close(fd);
      return -1;
   }
   res = ioctl(fd, SNDCTL_DSP_SPEED, &speed);
   if(res == -1 || abs(speed - SAMPLES_PER_SEC) > SAMPLES_PER_SEC / 200) /* allow .5% variance */
   {
      osPrintf("OSS: could not set sample speed; *** NO SOUND! ***\n");
      close(fd);
      return -1;
   }

   osPrintf("OSS: /dev/dsp opened OK, %dHz %d-bit %s\n", speed, samplesize, (stereo==1) ? "stereo" : "mono");

   /* Make sure the sound card has the capabilities we need */
   res = AFMT_QUERY;
   ioctl(fd ,SNDCTL_DSP_SETFMT, &res);
   if(res != AFMT_S16_LE)
   {
      osPrintf("OSS: could not set sample format; *** NO SOUND! ***\n");
      close(fd);
      return -1;
   }
   ioctl(fd, SNDCTL_DSP_GETCAPS, &res);
   if(!(res & DSP_CAP_DUPLEX))
   {
      osPrintf("OSS: could not set full duplex; *** NO SOUND! ***\n");
      close(fd);
      return -1;
   }
   
   return fd;
}

#elif defined(sun) /* ] [ */

#include <sys/audio.h>

int setupSoundCard(void)
{
   int res, fd;
   audio_info_t info;
   
   fd = open("/dev/audio", O_RDWR);
   if(fd == -1)
   {
      osPrintf("SUN: could not open /dev/audio; *** NO SOUND! ***\n");
      return -1;
   }

   res = fcntl(fd, F_SETFL, fcntl(soundCard, F_GETFL) & ~O_NONBLOCK);
   if(res == -1)
   {
      osPrintf("SUN: could not set blocking I/O; *** NO SOUND! ***\n");
      close(fd);
      return -1;
   }

   AUDIO_INITINFO(&info);
   info.play.sample_rate = SAMPLES_PER_SEC;
   info.play.channels = 1; /* mono */
   info.play.precision = BITS_PER_SAMPLE;
   info.play.encoding = AUDIO_ENCODING_LINEAR;
   info.record.sample_rate = SAMPLES_PER_SEC;
   info.record.channels = 1; /* mono */
   info.record.precision = BITS_PER_SAMPLE;
   info.record.encoding = AUDIO_ENCODING_LINEAR;
   
   res = ioctl(soundCard, AUDIO_SETINFO, &info);
   if(res == -1)
   {
      osPrintf("SUN: could not set audio parameters; *** NO SOUND! ***\n");
      close(fd);
      return -1;
   }
   
   return fd;
}

#else /* ] [ */

#warning No POSIX sound driver available; building only the dummy sound driver.

OsStatus dmaStartup(int samplesPerFrame)
{
   dmaOnline = 1;
   return dummyDriver_dmaStartup(samplesPerFrame);
}

void dmaShutdown(void)
{
   dummyDriver_dmaShutdown();
}

#endif /* __linux__, sun ] */

OsStatus dummyDriver_dmaStartup(int samplesPerFrame)
{
   OsTime tenMilliseconds(0, 10000); /* 10 ms = 10000 us */
   
   osPrintf("DSD: Using dummy sound driver.\n");
   
   pNotifier = new OsCallback(0, mediaSignaller);
   pTimer = new OsTimer(*pNotifier);
   pTimer->periodicEvery(OsTime::NO_WAIT, tenMilliseconds);
   
   return OS_SUCCESS;

}

void dummyDriver_dmaShutdown(void)
{
   if(dmaOnline)
   {
      pTimer->stop();
      delete pTimer;
      delete pNotifier;
      dmaOnline = 0;
   }
}

#endif /* __pingtel_on_posix__ ] */


