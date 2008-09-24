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


// SYSTEM INCLUDES
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#ifndef WINCE
#   include <process.h>
#endif
#include <mmsystem.h>
#include <assert.h>

// APPLICATION INCLUDES
#include "mp/dmaTask.h"
#include "mp/MpBufferMsg.h"
#include "mp/MpBuf.h"
#include "mp/MprToSpkr.h"
#include "mp/MpMediaTask.h"
#include "os/OsDefs.h"
#include "os/OsIntPtrMsg.h"

#ifdef RTL_ENABLED
#   include <rtl_macro.h>
#endif

// DEFINES
#undef OHISTORY
#define OHISTORY (2 * (N_OUT_BUFFERS))
#undef OHISTORY

#define WM_ALT_HEARTBEAT    WM_USER+1

// EXTERNAL FUNCTIONS
extern void showWaveError(char *syscall, int e, int N, int line) ;  // dmaTaskWnt.cpp

// EXTERNAL VARIABLES
extern HANDLE hMicThread;       // dmaTaskWnt.cpp
extern HANDLE hSpkrThread;      // dmaTaskWnt.cpp
extern int smSpkrQPreload ;     // dmaTaskWnt.cpp
extern int frameCount ;         // dmaTaskWnt.cpp

// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// GLOBAL VARIABLE INITIALIZATIONS
static int gRingDeviceId;
static int gCallDeviceId;
static HGLOBAL  hOutHdr[N_OUT_BUFFERS];
static WAVEHDR* pOutHdr[N_OUT_BUFFERS];
static HGLOBAL  hOutBuf[N_OUT_BUFFERS];
OsMsgPool* gSpeakerStatusPool = NULL;
OsMsgQ* gSpeakerStatusQueue = NULL;

#ifdef OHISTORY /* [ */
static int histOut[OHISTORY];
static int lastOut;
#endif /* OHISTORY ] */
HWAVEOUT audioOutH;         // Referenced in MpCodec (vol)
HWAVEOUT audioOutCallH;     // Referenced in MpCodec (vol)

/* ============================ FUNCTIONS ================================= */

// Determine which speaker handle should be used
static HWAVEOUT selectSpeakerDevice()
{
    HWAVEOUT hOut = NULL ;

    if (DmaTask::isRingerEnabled() && audioOutH)
    {
        hOut = audioOutH;     
    }
    // else if we have a "in call" device, then use that
    else if (audioOutCallH)
    {
        hOut = audioOutCallH;
    }
    // finally, just use the default device
    else 
    {            
        hOut = audioOutH;
    }

    return hOut ;
}

static void CALLBACK TimerCallbackProc(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2) 
{
   OsIntPtrMsg *pMsg = (OsIntPtrMsg*)gSpeakerStatusPool->findFreeMsg();

   if (pMsg)
   {
      // message was taken from pool
      pMsg->setData1(WM_ALT_HEARTBEAT);
      if (gSpeakerStatusQueue->sendFromISR(*pMsg) != OS_SUCCESS)
      {
         osPrintf("Problem with sending message in TimerCallbackProc\n");
      }
   }
   else
   {
      osPrintf("Could not create message in TimerCallbackProc\n");
   }      
}

// Call back for speaker audio
void CALLBACK speakerCallbackProc(HANDLE h, UINT wMsg, DWORD dwInstance, DWORD dwParam, DWORD unused)
{
#ifdef OHISTORY /* [ */
   if (WOM_DONE == wMsg) {
      histOut[lastOut] = (((WAVEHDR*)dwParam)->dwUser) & USER_BUFFER_MASK;
      lastOut = (lastOut + 1) % OHISTORY;
   }
#endif /* OHISTORY ] */

   OsIntPtrMsg *pMsg = (OsIntPtrMsg*)gSpeakerStatusPool->findFreeMsg();

   if (pMsg)
   {
      // message was taken from pool
      pMsg->setData1(wMsg);
      pMsg->setData2(dwParam);
      if (gSpeakerStatusQueue->sendFromISR(*pMsg) != OS_SUCCESS)
      {
         osPrintf("Problem with sending message in speakerCallbackProc\n");
      }
   }
   else
   {
      osPrintf("Could not create message in speakerCallbackProc\n");
   }
}

// This function waits for given message on speaker device
static void waitForSpeakerStatusMessage(unsigned int message)
{
   UtlBoolean bSuccess = FALSE;
   unsigned int speakerStatus = 0;
   OsMsg *pMsg = NULL;
   do 
   {
      bSuccess = (gSpeakerStatusQueue->receive(pMsg) == OS_SUCCESS);
      if (bSuccess && pMsg)
      {
         OsIntPtrMsg *pIntMsg = (OsIntPtrMsg*)pMsg;
         speakerStatus = (unsigned int)pIntMsg->getData1();
         pIntMsg->releaseMsg();
         pMsg = NULL;
      }
   } while (bSuccess && (speakerStatus != message));
}

// This function will attempt to open a user specified audio device.
// If it fails, we will try to open any audio device that meets our requested format
// If we still fail, we will return 0, and display a message.
// TODO: Seems we have a problem when no audio device can be found. No calls can be made.
//       I recommend we bubble this up to java and display a message.
static int openAudioOut(int desiredDeviceId, HWAVEOUT *pAudioOutH,int nChannels, int nSamplesPerSec, int nBitsPerSample)
{
   WAVEFORMATEX fmt;
   MMRESULT     res;

   *pAudioOutH = NULL;

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
      (DWORD) speakerCallbackProc,// callback entry
      GetCurrentThreadId(),   // instance data
      CALLBACK_FUNCTION);     // callback function specified

   //if we fail to open the audio device above, then we 
   //will try to open any device that will hande the requested format
   if (res != MMSYSERR_NOERROR)
      res = waveOutOpen(
         pAudioOutH,             // handle (will be filled in)
         WAVE_MAPPER,            // select any device able to handle this format
         &fmt,                   // format
         (DWORD) speakerCallbackProc,// callback entry
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

static WAVEHDR* outPrePrep(int n, DWORD bufLen)
{
#ifdef RTL_ENABLED
   RTL_EVENT("SpeakerThreadWnt.outPrePrep", 1);
#endif

   WAVEHDR* pWH;
   int doAlloc = (hOutHdr[n] == NULL);
   MpBufferMsg* msg;
   MpBufferMsg* pFlush;
   MpAudioBufPtr ob;

   static int oPP = 0;

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
   while (MpMisc.pSpkQ && MpMisc.pSpkQ->numMsgs() > MprToSpkr::MAX_SPKR_BUFFERS) {
      OsStatus  res;
      flushes++;
      res = MpMisc.pSpkQ->receive((OsMsg*&) pFlush, OsTime::NO_WAIT_TIME);
      if (OS_SUCCESS == res) {
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

   if (MpMisc.pSpkQ) {
      if (  (skip == 0)
         && (MpMisc.pSpkQ->numMsgs() < MprToSpkr::MIN_SPKR_BUFFERS))
      {
         skip = MprToSpkr::SKIP_SPKR_BUFFERS;
         assert(MprToSpkr::MAX_SPKR_BUFFERS >= skip);
#ifdef DEBUG_WINDOZE /* [ */
         osPrintf("Skip(%d,%d)\n", skip, oPP);
#endif /* DEBUG_WINDOZE ] */
      }

      if (MpMisc.pSpkQ->numMsgs() >= skip)
      {
         skip = 0;
         if (MpMisc.pSpkQ->receive((OsMsg*&)msg, OsTime::NO_WAIT_TIME) == OS_SUCCESS)
         {
            ob = (MpAudioBufPtr)(msg->getBuffer());
            msg->releaseMsg();
         }
//         osPrintf("pSpkQ message received\n");
      } else {
//         osPrintf("pSpkQ message skipped\n");
      }
   }

   if (!ob.isValid()) {
      ob = MpMisc.mpFgSilence;
   }

   if (doAlloc) {
      hOutHdr[n] = GlobalAlloc(GPTR, sizeof(WAVEHDR));
      assert(NULL != hOutHdr[n]);
      hOutBuf[n] = GlobalAlloc(GPTR, bufLen);
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
   memcpy(pWH->lpData, ob->getSamplesPtr(), bufLen);

#ifdef RTL_ENABLED
   RTL_EVENT("SpeakerThreadWnt.outPrePrep", 0);
#endif
   return pWH;
}


static void outPostUnprep(int n, bool bFree)
{
   assert((n > -1) && (n < N_OUT_BUFFERS));

   if (bFree)
   {
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
}

static int openSpeakerDevices(WAVEHDR*& pWH, HWAVEOUT& hOut)
{
    DWORD    bufLen = ((N_SAMPLES * BITS_PER_SAMPLE) / 8);
    int i ;
    MMRESULT ret;

    // set the different device ids
    gRingDeviceId = WAVE_MAPPER;
    gCallDeviceId = WAVE_MAPPER;


    // If either the ringer or call device is set to NONE, don't engage any audio devices
    if ((strcasecmp(DmaTask::getRingDevice(), "NONE") == 0) ||
        (strcasecmp(DmaTask::getCallDevice(), "NONE") == 0))
    {
        ResumeThread(hMicThread);
        return 1;
    }

    /*
     * Select in-call / ringer devices
     */
    int ii;
    WAVEOUTCAPS devcaps;
    int numberOfDevicesOnSystem = waveOutGetNumDevs();
    for(ii=0; ii<numberOfDevicesOnSystem; ii++)
    {
        waveOutGetDevCaps(ii,&devcaps,sizeof(WAVEOUTCAPS));
        if (strcmp(devcaps.szPname, DmaTask::getRingDevice())==0) 
        {
            gRingDeviceId = ii;
            osPrintf("SpkrThread: Selected ring device: %s\n",devcaps.szPname);
        }

        if (strcmp(devcaps.szPname, DmaTask::getCallDevice())==0) 
        {
            gCallDeviceId = ii;
            osPrintf("SpkrThread: Selected call device: %s\n",devcaps.szPname);
        }
    }

    /*
     * Open ringer device
     */ 
    if (!openAudioOut(gRingDeviceId, &audioOutH, 1, SAMPLES_PER_SEC, BITS_PER_SAMPLE))
    {
        osPrintf("SpkrThread: Failed to open ring audio output channel\n\n");
        ResumeThread(hMicThread);
        return 1;
    }

    waitForSpeakerStatusMessage(WOM_OPEN);

    /*
     * Open in-call device
     */
    if (!openAudioOut(gCallDeviceId,&audioOutCallH, 1, SAMPLES_PER_SEC, BITS_PER_SAMPLE))
    {
        osPrintf("SpkrThread: Failed to open call audio output channel\n\n");
        ResumeThread(hMicThread);
        return 1;
    }

    waitForSpeakerStatusMessage(WOM_OPEN);

    // Pre load some data    
    for (i=0; i<smSpkrQPreload; i++)
    {
        pWH = outPrePrep(i, bufLen);

        hOut = selectSpeakerDevice() ;
        if (hOut)
        {
            ret = waveOutPrepareHeader(hOut, pWH, sizeof(WAVEHDR));
            if (ret != MMSYSERR_NOERROR)
            {
                showWaveError("waveOutPrepareHeader", ret, i, __LINE__);
            }
            ret = waveOutWrite(hOut, pWH, sizeof(WAVEHDR));
            if (ret != MMSYSERR_NOERROR)
            {
                   showWaveError("waveOutWrite", ret, i, __LINE__);
            }
        }      
    }

    return 0 ;
}

// wait until all the buffers have been reset. previously the code 
// waited exactly 100ms, however this wasn't always enough. now
// it will wait until there are no buffers in the queue.
// in order to avoid an infinite loop (although this should only happen
// if waveOutReset was not called) we give up after 100 iterations
// (equals 1 second). if this happens then we could be left with some 
// active data in the buffers.
static void waitForDeviceResetCompletion()
{
    int i;
    bool bStillResetting;
    int iterations = 0;

    do
    {
        bStillResetting = false;

        for (i = 0; i < N_OUT_BUFFERS; i++) 
        {
            if (hOutHdr[i] && (pOutHdr[i]->dwFlags & WHDR_INQUEUE))
            {
                bStillResetting = true;
            }
        }

        if (bStillResetting)
        {
            Sleep(10);
        }
    }
    while (bStillResetting && ++iterations < 100);
}

void closeSpeakerDevices()
{
    MMRESULT ret;
    int i ;

    // Clean up ringer audio
    if (audioOutH)
    {
        ret = waveOutReset(audioOutH);
        if (ret != MMSYSERR_NOERROR)
        {
            showWaveError("waveOutReset", ret, -1, __LINE__);
        }
        else
        {
            waitForDeviceResetCompletion();
        }

        for (i=0; i<N_OUT_BUFFERS; i++) 
        {
            if (NULL != hOutHdr[i]) 
            {
                ret = waveOutUnprepareHeader(audioOutH, pOutHdr[i], sizeof(WAVEHDR));
                if (ret != MMSYSERR_NOERROR)
                {
                    showWaveError("waveOutUnprepareHeader", ret, i, __LINE__);
                }
                outPostUnprep(i, TRUE);
            }
        }
        
        ret = waveOutClose(audioOutH);
        if (ret != MMSYSERR_NOERROR)
        {
            showWaveError("waveOutClose", ret, -1, __LINE__);
        }
        audioOutH = NULL;

        waitForSpeakerStatusMessage(WOM_CLOSE);
    }

    // Clean up call audio
    if (audioOutCallH)
    {
        ret = waveOutReset(audioOutCallH);
        if (ret != MMSYSERR_NOERROR)
        {
            showWaveError("waveOutReset", ret, -1, __LINE__);
        }
        else
        {
            waitForDeviceResetCompletion();
        }

        for (i=0; i<N_OUT_BUFFERS; i++) 
        {
            if (NULL != hOutHdr[i]) 
            {
                ret = waveOutUnprepareHeader(audioOutCallH, pOutHdr[i], sizeof(WAVEHDR));
                if (ret != MMSYSERR_NOERROR)
                {
                    showWaveError("waveOutUnprepareHeader", ret, i, __LINE__);
                }
                outPostUnprep(i, true);
            }
        }

        ret = waveOutClose(audioOutCallH);
        if (ret != MMSYSERR_NOERROR)
        {
             showWaveError("waveOutClose", ret, -1, __LINE__);
        }
        audioOutCallH = NULL;

        waitForSpeakerStatusMessage(WOM_CLOSE);
    }
}

unsigned int __stdcall SpkrThread(LPVOID Unused)
{
    int      i;
    DWORD    bufLen = ((N_SAMPLES * BITS_PER_SAMPLE) / 8);
    WAVEHDR* pWH;
    MMRESULT ret;
    int      played;
    OsMsg *pMsg = NULL;
    OsIntPtrMsg *pSpeakerMsg = NULL;
    BOOL     bGotMsg ;
    int      n;
    bool     bDone ;
    static bool sLastRingerEnabled = false;
    OsStatus res;
    static bool bRunning = false ;
    HWAVEOUT hOut = NULL;
    UINT    timerId=0;

    // Verify that only 1 instance of the MicThread is running
    if (bRunning) 
    {
       ResumeThread(hMicThread);
       return 1 ;
    }
    else
    {
       bRunning = true ;
    }
    ResumeThread(hMicThread);


#ifdef OHISTORY /* [ */
    WAVEHDR* lastWH[OHISTORY];
    int      lastInd[OHISTORY];
    int      last = 0;

    for (i=0;i<OHISTORY;i++) 
    {
        lastWH[i] = 0;
        lastInd[i] = 0;
    }

    memset(histOut, 0xff, sizeof(histOut));
    lastOut = 0;
#endif /* OHISTORY ] */
    
    // Initialize headers
    for (i=0; i<N_OUT_BUFFERS; i++) 
    {
        hOutHdr[i] = hOutBuf[i] = NULL;
        pOutHdr[i] = NULL;
    }
    
    if (openSpeakerDevices(pWH, hOut))
    {
        // NOT using a sound card
        // Set up a 10ms timer to call back to this routine
        timerId = timeSetEvent(10, 0, TimerCallbackProc, GetCurrentThreadId(), TIME_PERIODIC);
    }

    played = 0;   
    bDone = false ;
    while (!bDone)
    {
       bGotMsg = (gSpeakerStatusQueue->receive(pMsg) == OS_SUCCESS);
              
        // when switching devices, ringer to in-call we need to make 
        // sure any outstanding buffers are flushed
        if (sLastRingerEnabled != DmaTask::isRingerEnabled())
        {
            if (audioOutH)
            {
                ret = waveOutReset(audioOutH);
            }
            if (audioOutCallH)
            {
                ret = waveOutReset(audioOutCallH);
            }

            if (ret != MMSYSERR_NOERROR)
            {
                showWaveError("waveOutReset", ret, -1, __LINE__);
            }
            else
            {
                waitForDeviceResetCompletion();
            }
        }

        if (bGotMsg && pMsg) 
        {
           pSpeakerMsg = (OsIntPtrMsg*)pMsg;
           intptr_t msgType = pSpeakerMsg->getData1();
           intptr_t data2 = pSpeakerMsg->getData2();
           pSpeakerMsg->releaseMsg();
           pSpeakerMsg = NULL;
           pMsg = NULL;

            switch (msgType) 
            {
            case WM_ALT_HEARTBEAT:
                res = MpMediaTask::signalFrameStart();
                switch (res) 
                {
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
                // Check for a changed speaker device
                if (DmaTask::isOutputDeviceChanged())
                {                    
                    DmaTask::clearOutputDeviceChanged() ;
                    closeSpeakerDevices() ;
                    if (audioOutH)
                    {
                        ret = waveOutReset(audioOutH);
                    }
                    if (audioOutCallH)
                    {
                        ret = waveOutReset(audioOutCallH);
                    }

                    if (ret != MMSYSERR_NOERROR)
                    {
                        showWaveError("waveOutReset", ret, -1, __LINE__);
                    }
                    else
                    {
                        waitForDeviceResetCompletion();
                    }

                    // Kill the hearbeat timer if it exists
                    if (timerId>0)
                        timeKillEvent(timerId);
                    if (openSpeakerDevices(pWH, hOut))
                    {
                        // Open failed - so start the heartbeat timer
                        timerId = timeSetEvent(10, 0, TimerCallbackProc, GetCurrentThreadId(), TIME_PERIODIC);                    
                    }
                    continue ;                    
                }
                break ;
            case WOM_DONE:
                pWH = (WAVEHDR *) data2;
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
                
                if (DmaTask::isOutputDeviceChanged())
                {                    
                    DmaTask::clearOutputDeviceChanged() ;
                    closeSpeakerDevices() ;
                    if (openSpeakerDevices(pWH, hOut))
                    {
                        if (timerId>0)
                            timeKillEvent(timerId);
                        timerId = timeSetEvent(10, 0, TimerCallbackProc, GetCurrentThreadId(), TIME_PERIODIC);                    
                    }
                    continue ;                    
                }

                hOut = selectSpeakerDevice() ;
                if (hOut)
                {
                    ret = waveOutUnprepareHeader(hOut, pWH, sizeof(WAVEHDR));
                    if (ret != MMSYSERR_NOERROR)
                    {
                           showWaveError("waveOutUnprepareHeader", ret, played, __LINE__);
                    }
                    outPostUnprep(n, false);

                    pWH = outPrePrep(n, bufLen);

                    ret = waveOutPrepareHeader(hOut, pWH, sizeof(WAVEHDR));
                    if (ret != MMSYSERR_NOERROR)
                    {
                           showWaveError("waveOutPrepareHeader", ret, played, __LINE__);
                    }
                    ret = waveOutWrite(hOut, pWH, sizeof(WAVEHDR));
                    if (ret != MMSYSERR_NOERROR)
                    {
                           showWaveError("waveOutWrite", ret, played, __LINE__);
                    }
                    played++;
                }

                res = MpMediaTask::signalFrameStart();

                switch (res) 
                {
#ifdef DEBUG_WINDOZE /* [ */
                case OS_SUCCESS:
                    frameCount++;
                    osPrintf(" Frame %d: OS_SUCCESSFUL\n", frameCount);
                    break;
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
                case OS_SUCCESS:
                    frameCount++;
                    break;
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
                // Audio device was closed on us (doesn't happen as far as I
                // know)
                bDone = true ;
                break;
            default:                
                break;
            }
        } 
        else 
        {
            // Sky is falling, kick out so that we don't spin a high priority
            // thread.
            bDone = true ;
        }
      
        // record our last ringer state
        sLastRingerEnabled = DmaTask::isRingerEnabled();
    }

    // Stop heartbeat timer if it exist
    if (timerId>0)
        timeKillEvent(timerId);

    closeSpeakerDevices() ;

    bRunning = false ;

    return 0;
}
