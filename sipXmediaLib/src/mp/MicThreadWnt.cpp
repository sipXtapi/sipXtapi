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
#include "os/OsMsgPool.h"
#include "os/OsDefs.h"
#include "os/OsIntPtrMsg.h"

#ifdef RTL_ENABLED
#   include <rtl_macro.h>
#endif

// DEFINES
// EXTERNAL FUNCTIONS
extern void showWaveError(char *syscall, int e, int N, int line) ;  // dmaTaskWnt.cpp

// EXTERNAL VARIABLES

extern HANDLE hMicThread;       // dmaTaskWnt.cpp
extern HANDLE hSpkrThread;      // dmaTaskWnt.cpp

// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// GLOBAL VARIABLE INITIALIZATIONS
static int gMicDeviceId ;
static HGLOBAL  hInHdr[N_IN_BUFFERS];
static WAVEHDR* pInHdr[N_IN_BUFFERS];
static HGLOBAL  hInBuf[N_IN_BUFFERS];
#ifdef IHISTORY /* [ */
static int histIn[IHISTORY];
static int lastIn;
#endif /* HISTORY ] */
static HWAVEIN  audioInH;
static OsMsgPool* DmaMsgPool = NULL;
OsMsgPool* gMicStatusPool = NULL;
OsMsgQ* gMicStatusQueue = NULL;

/* ============================ FUNCTIONS ================================= */


void CALLBACK micOutCallBackProc(HANDLE h, UINT wMsg, DWORD dwInstance, DWORD dwParam, DWORD unused)
{
#ifdef IHISTORY /* [ */
   if (WIM_DATA == wMsg) {
      histIn[lastIn] = (((WAVEHDR*)dwParam)->dwUser) & USER_BUFFER_MASK;
      lastIn = (lastIn + 1) % IHISTORY;
   }
#endif /* HISTORY ] */

   OsIntPtrMsg *pMsg = (OsIntPtrMsg*)gMicStatusPool->findFreeMsg();

   if (pMsg)
   {
      // message was taken from pool
      pMsg->setData1(wMsg);
      pMsg->setData2(dwParam);
      if (gMicStatusQueue->sendFromISR(*pMsg) != OS_SUCCESS)
      {
         osPrintf("Problem with sending message in micOutCallBackProc\n");
      }
   }
   else
   {
      osPrintf("Could not create message in micOutCallBackProc\n");
   }
}

/**
*  Detects ID of a mixer that can set gain on the wave input device
*  identified by supplied handle. A sound card can have separate mixer
*  for output devices and input devices. If we have multiple sound cards
*  then we will have multiple wave input and wave output devices with several
*  mixers possibly separate input and output mixers. Thus we need to detect
*  the right mixer ID in order to set gain of the right microphone later.
*  
*  @param pAudioInH handle of the open wave input device
*  @return result of detection of mixer ID
*
*  @see detectInputMixerId
*  @see openAudioIn
*/
OsStatus detectInputMixerId(HWAVEIN *pAudioInH)
{
   MMRESULT mmresult;
   unsigned int uMxId;

   mmresult = mixerGetID((HMIXEROBJ)(*pAudioInH), &uMxId, MIXER_OBJECTF_HWAVEIN);

   if (mmresult == MMSYSERR_NOERROR)
   {
      return MpCodec_setInputMixerId(uMxId);
   }
   else return OS_UNSPECIFIED;
}

// This function waits for given status message on mic device
static void waitForMicStatusMessage(unsigned int message)
{
   UtlBoolean bSuccess = FALSE;
   unsigned int micStatus = 0;
   OsMsg *pMsg = NULL;
   do 
   {
      bSuccess = (gMicStatusQueue->receive(pMsg) == OS_SUCCESS);
      if (bSuccess && pMsg)
      {
         OsIntPtrMsg *pIntMsg = (OsIntPtrMsg*)pMsg;
         micStatus = (unsigned int)pIntMsg->getData1();
         pIntMsg->releaseMsg();
         pMsg = NULL;
      }
   } while (bSuccess && (micStatus != message));
}

int openAudioIn(HWAVEIN *pAudioInH,
                int nChannels, int nSamplesPerSec, int nBitsPerSample)
{
   WAVEFORMATEX fmt;
   MMRESULT     res;
   OsStatus osstatus = OS_UNSPECIFIED;

   *pAudioInH = NULL;
   
   fmt.wFormatTag      = WAVE_FORMAT_PCM;
   fmt.nChannels       = nChannels;
   fmt.nSamplesPerSec  = nSamplesPerSec;
   fmt.nAvgBytesPerSec = (nChannels * nSamplesPerSec * nBitsPerSample) / 8;
   fmt.nBlockAlign     = (nChannels * nBitsPerSample) / 8;
   fmt.wBitsPerSample  = nBitsPerSample;
   fmt.cbSize          = 0;

   res = waveInOpen(
      pAudioInH,              // handle (will be filled in)
      gMicDeviceId,            // attempt to open the "in call" device
      &fmt,                   // format
      (DWORD) micOutCallBackProc,// callback entry
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
      (DWORD) micOutCallBackProc,// callback entry
      GetCurrentThreadId(),   // instance data
      CALLBACK_FUNCTION);     // callback function specified

      if (res == MMSYSERR_NOERROR)
      {
         // when no error then detect input mixer id
         osstatus = detectInputMixerId(pAudioInH);
      }
   }
   else
   {
      // when no error then detect input mixer id
      osstatus = detectInputMixerId(pAudioInH);
   }

   if (osstatus != OS_SUCCESS)
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


WAVEHDR* inPrePrep(int n, DWORD bufLen)
{
   WAVEHDR* pWH;
   int doAlloc = (hInHdr[n] == NULL);

   assert((n > -1) && (n < N_IN_BUFFERS));
   if (doAlloc) {
      hInHdr[n] = GlobalAlloc(GPTR, sizeof(WAVEHDR));
      assert(NULL != hInHdr[n]);
      hInBuf[n] = GlobalAlloc(GPTR, bufLen);
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

bool inPostUnprep(int n, int discard, DWORD bufLen, bool bFree)
{
#ifdef RTL_ENABLED
   RTL_EVENT("MicThreadWnt.outPrePrep", 0);
#endif
   bool retVal = false;  //assume we didn't succeed for now

   WAVEHDR* pWH;
   MpAudioBufPtr ob;

   static int iPU = 0;
   static int flushes = 0;

   assert((n > -1) && (n < N_IN_BUFFERS));
   pWH = pInHdr[n];


   // assert(NULL!=pWH);
   
   if (pWH && bufLen == pWH->dwBytesRecorded)
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
         ob = MpMisc.RawAudioPool->getBuffer();
         if (!ob.isValid())
            return false;
         ob->setSamplesNumber(N_SAMPLES);
      }
      if (!discard) {
         MpBufferMsg* pFlush;
         MpBufferMsg* pMsg;

         if (ob.isValid())
         {
            memcpy( ob->getSamplesWritePtr()
                   , pWH->lpData
                   , min( pWH->dwBytesRecorded
                        , ob->getSamplesNumber()*sizeof(MpAudioSample)));
         }
#ifdef INSERT_SAWTOOTH /* [ */
         if (NULL == ob) { /* nothing in Q, or we are disabled */
            ob = MpMisc.RawAudioPool->getBuffer();
            if (ob.isValid()) {
                ob->setSamplesNumber(MpMisc.frameSamples);
                int i, n;
                MpAudioSample *s;

                s = ob->getSamplesPtr();
                n = ob->getSamplesNumber();
                for (i=0; i<n; i++)
                    *s++= ((i % 80) << 10);
            }
         }
#endif /* INSERT_SAWTOOTH ] */


         pMsg = (MpBufferMsg*) DmaMsgPool->findFreeMsg();
         if (NULL == pMsg)
            pMsg = new MpBufferMsg(MpBufferMsg::AUD_RECORDED);

         pMsg->setMsgSubType(MpBufferMsg::AUD_RECORDED);

         // Buffer is moved to the message. ob pointer is invalidated.
         pMsg->ownBuffer(ob);

         if (MpMisc.pMicQ)
         {
            if (MpMisc.pMicQ->numMsgs() >= MpMisc.pMicQ->maxMsgs())
            {
                // if its full, flush one and send
                OsStatus  res;
                flushes++;
                res = MpMisc.pMicQ->receive((OsMsg*&) pFlush, OsTime::NO_WAIT_TIME);
                if (OS_SUCCESS == res) {
                   pFlush->releaseMsg();
                } else {
                   osPrintf("DmaTask: queue was full, now empty (3)!"
                            " (res=%d)\n", res);
                }
            }
            MpMisc.pMicQ->send(*pMsg, OsTime::NO_WAIT_TIME);
         }
         if (!pMsg->isMsgReusable())
             delete pMsg;
      }
      return true;
   }

   if (bFree)
   {
      if (NULL != hInHdr[n]) 
      {
         GlobalUnlock(hInHdr[n]);
         pInHdr[n] = NULL;
         GlobalFree(hInHdr[n]);
         hInHdr[n] = NULL;
      }
      if (NULL != hInBuf[n]) 
      {
         GlobalUnlock(hInBuf[n]);
         GlobalFree(hInBuf[n]);
         hInBuf[n] = NULL;
      }
   }

#ifdef RTL_ENABLED
   RTL_EVENT("MicThreadWnt.outPrePrep", 1);
#endif

   return retVal;
}

int openMicDevice(bool& bRunning, WAVEHDR*& pWH)
{
    int        i, ii;
    WAVEINCAPS devcaps;
    DWORD      bufLen = ((N_SAMPLES * BITS_PER_SAMPLE) / 8);
    MMRESULT   ret;

    gMicDeviceId = WAVE_MAPPER;

    // If the mic device is set to NONE, don't engage
    if (strcasecmp(DmaTask::getMicDevice(), "NONE") == 0)
    {
        ResumeThread(hSpkrThread);
        return 1;
    }

        
    int numberOfDevicesOnSystem = waveInGetNumDevs();
    for(ii=0; ii<numberOfDevicesOnSystem; ii++)
    {
        waveInGetDevCaps(ii, &devcaps, sizeof(devcaps));
        if (strcmp(devcaps.szPname, DmaTask::getMicDevice())==0) 
        {
            gMicDeviceId = ii ;
            osPrintf("MicThread: Selected mic device: %s\n", devcaps.szPname);
        }
    }

    // Open the input wave device
    if (!openAudioIn(&audioInH, 1, SAMPLES_PER_SEC, BITS_PER_SAMPLE))
    {
        osPrintf("MicThread: Failed to open audio input channel\n\n");
        ResumeThread(hSpkrThread);
        bRunning = false ;
        return 1;
    }

    waitForMicStatusMessage(WIM_OPEN);

    ret = waveInStart(audioInH);
    if (ret != MMSYSERR_NOERROR)
    {
        showWaveError("waveInStart", ret, -1, __LINE__);
        ResumeThread(hSpkrThread);
        bRunning = false ;
        return 1;
    }

    // Preload audio data
    for (i=0; i<N_IN_PRIME; i++) 
    {
        pWH = inPrePrep(i, bufLen);

        ret = waveInPrepareHeader(audioInH, pWH, sizeof(WAVEHDR));
        if (ret != MMSYSERR_NOERROR)
        {
            showWaveError("waveInPrepareHeader", ret, i, __LINE__);
            ResumeThread(hSpkrThread);
            bRunning = false ;
            return 1;
        }
        ret = waveInAddBuffer(audioInH, pWH, sizeof(WAVEHDR));
        if (ret != MMSYSERR_NOERROR)
        {
            showWaveError("waveInAddBuffer", ret, i, __LINE__);
            ResumeThread(hSpkrThread);
            bRunning = false ;
            return 1;
        }
    }

    return 0 ;
}

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
         if (hInHdr[i] && (pInHdr[i]->dwFlags & WHDR_INQUEUE))
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

void closeMicDevice()
{
    DWORD      bufLen = ((N_SAMPLES * BITS_PER_SAMPLE) / 8);
    MMRESULT   ret;
    int        i ;

    // Cleanup
    if (!audioInH)
        return;
    ret = waveInReset(audioInH);
    if (ret != MMSYSERR_NOERROR)
    {
        showWaveError("waveInReset", ret, -1, __LINE__);
    }    
    ret = waveInStop(audioInH);
    if (ret != MMSYSERR_NOERROR)
    {
        showWaveError("waveInStop", ret, -1, __LINE__);
    }
    waitForDeviceResetCompletion();

    for (i=0; i<N_IN_BUFFERS; i++) 
    {
        if (NULL != hInHdr[i]) 
        {
            ret = waveInUnprepareHeader(audioInH, pInHdr[i], sizeof(WAVEHDR));
            if (ret != MMSYSERR_NOERROR)
            {
                showWaveError("waveInUnprepareHeader", ret, i, __LINE__);
            }
            inPostUnprep(i, TRUE, bufLen, TRUE);
        }
    }
    Sleep(50);

    ret = waveInClose(audioInH);
    if (ret != MMSYSERR_NOERROR)
    {
        showWaveError("waveInClose", ret, -1, __LINE__);
    }

    waitForMicStatusMessage(WIM_CLOSE);

    audioInH = NULL;
}


unsigned int __stdcall MicThread(LPVOID Unused)
{
    int      i;
    DWORD    bufLen = ((N_SAMPLES * BITS_PER_SAMPLE) / 8);
    WAVEHDR* pWH;
    MMRESULT ret;
    int      recorded;
    OsMsg *pMsg = NULL;
    OsIntPtrMsg *pMicMsg = NULL;
    BOOL     bGotMsg ;
    int      n;
    bool     bDone ;
    static bool bRunning = false ;

    // Verify that only 1 instance of the MicThread is running
    if (bRunning) 
    {
        ResumeThread(hSpkrThread);
        return 1 ;
    }
    else
    {
        bRunning = true ;
    }

    MpBufferMsg *pBuffMsg = new MpBufferMsg(MpBufferMsg::AUD_RECORDED);
    DmaMsgPool = new OsMsgPool("DmaTask", (*(OsMsg*)pBuffMsg),
       40, 60, 100, 5,
       OsMsgPool::SINGLE_CLIENT);
    delete pBuffMsg;

    // Initialize Buffers
    for (i=0; i<N_IN_BUFFERS; i++) 
    {
        hInHdr[i] = hInBuf[i] = NULL;
        pInHdr[i] = NULL;
    }


#ifdef IHISTORY /* [ */
    WAVEHDR* lastWH[IHISTORY];
    int      lastInd[IHISTORY];
    int      last = 0;

    for (i=0;i<IHISTORY;i++) 
    {
        lastWH[i] = 0;
        lastInd[i] = 0;
    }

    memset(histIn, 0xff, sizeof(histIn));
    lastIn = 0;

#endif /* IHISTORY ] */

    if (openMicDevice(bRunning, pWH))
    {
        return 1 ;
    }

    // Start up Speaker thread
    ResumeThread(hSpkrThread);

    recorded = 0;
    bDone = false ;
    while (!bDone) 
    {
       bGotMsg = (gMicStatusQueue->receive(pMsg) == OS_SUCCESS);

        if (bGotMsg && pMsg) 
        {
           pMicMsg = (OsIntPtrMsg*)pMsg;
           intptr_t msgType = pMicMsg->getData1();
           intptr_t data2 = pMicMsg->getData2();
           pMicMsg->releaseMsg();
           pMicMsg = NULL;
           pMsg = NULL;

            switch (msgType) 
            {
            case WIM_DATA:
                // Check if we got data - if not - then this signals a device change
                if (!data2)
                {
                    if (DmaTask::isInputDeviceChanged())
                    {
                        DmaTask::clearInputDeviceChanged();

                        closeMicDevice();
                        openMicDevice(bRunning, pWH);
                    }
                    break;
                }
                pWH = (WAVEHDR *)data2;
                n = (pWH->dwUser) & USER_BUFFER_MASK;

#ifdef IHISTORY /* [ */
                lastWH[last] = pWH;
                lastInd[last] = n;
                last = (last + 1) % IHISTORY;

                if (N_IN_BUFFERS == recorded) 
                {
                    osPrintf("after first %d buffers:", recorded + 1);
                    osPrintf("\nCall Backs:");
                    for (i=0; i<IHISTORY; i++) 
                    {
                        osPrintf("%c%3d", (0 == (i % 20)) ? '\n' : ' ', histIn[i]);
                    }
                    osPrintf("\n\nMessages:");
                    for (i=0; i<IHISTORY; i++) 
                    {
                        osPrintf("%c%3d", (0 == (i % 20)) ? '\n' : ' ', lastInd[i]);
                    }
                    osPrintf("\n");
                }
#endif /* IHISTORY ] */

                if (DmaTask::isInputDeviceChanged())
                {                    
                    DmaTask::clearInputDeviceChanged() ;

                    closeMicDevice() ;
                    openMicDevice(bRunning, pWH) ;

                    continue ;
                }

                ret = waveInUnprepareHeader(audioInH, pWH, sizeof(WAVEHDR));
                if (ret != MMSYSERR_NOERROR)
                {
                    showWaveError("waveInUnprepareHeader", ret, recorded, __LINE__);
                }

                if (DmaTask::isMuteEnabled())
                {
                    memset(pWH->lpData, 0, pWH->dwBytesRecorded); /* clear it out */
                }


                if (inPostUnprep(n, FALSE, bufLen, FALSE))
                {
                    pWH = inPrePrep(n, bufLen);
                    ret = waveInPrepareHeader(audioInH, pWH, sizeof(WAVEHDR));
                    if (ret != MMSYSERR_NOERROR)
                    {
                        showWaveError("waveInPrepareHeader", ret, recorded, __LINE__);
                    }

                    ret = waveInAddBuffer(audioInH, pWH, sizeof(WAVEHDR));
                    if (ret != MMSYSERR_NOERROR)
                    {
                        showWaveError("waveInAddBuffer", ret, recorded, __LINE__);
                        recorded++;
                    }
                }
                break;
            case WIM_CLOSE:            
                bDone = true ;
                break;
            }
        } 
        else 
        {
            // Failed to get msg; don't spin high priority thread
            bDone = true ;
        }
    }

    closeMicDevice() ;    

    bRunning = false;
    delete DmaMsgPool;
    DmaMsgPool = NULL;

    return 0;
}
