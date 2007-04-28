//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: 

#ifdef __linux__

// SYSTEM INCLUDES
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <sched.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/soundcard.h>
#include <sys/mman.h>

// APPLICATION INCLUDES
#include "mp/MpidOSS.h"
#include "mp/MpodOSS.h"
//#include "mp/MpInputDeviceManager.h"
//#include "mp/MpOutputDeviceManager.h"
#include "os/OsTask.h"

#ifdef RTL_ENABLED
#include "rtl_macro.h"
#else
#define RTL_BLOCK(x)
#define RTL_EVENT(x,y)
#endif


#define timediff(early, late) ((late.tv_sec-early.tv_sec)*1000000+(late.tv_usec-early.tv_usec))

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
#ifndef OSS_SINGLE_DEVICE
extern MpOSSDeviceWrapperContainer mOSSContainer;
#else
MpOSSDeviceWrapper ossSingleDriver;
#endif

// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */
/* ============================ CREATORS ================================== */
// Default constructor
MpOSSDeviceWrapper::MpOSSDeviceWrapper() :
                             mfdDevice(-1),
                             mUsedSamplesPerSec(0),
                             pReader(NULL),
                             pWriter(NULL),
                             mDeviceCap(0),
                             pReaderEnabled(FALSE),
                             pWriterEnabled(FALSE),
                             mThreadExit(FALSE),
                             mbFisrtWriteCycle(FALSE),
                             musecFrameTime(0),
                             mFramesRead(0),
                             mFramesDropRead(0),
                             mFramesWritten(0),
                             mFramesWrUnderruns(0),
                             mReClocks(0),
                             mCTimeUp(0),
                             mCTimeDown(0)
{
    int res;
    
    pthread_mutex_init(&mWrMutex, NULL);
    pthread_mutex_init(&mWrMutexBuff, NULL);
    pthread_cond_init(&mNewTickCondition, NULL);
    pthread_cond_init(&mNull, NULL);
    pthread_cond_init(&mNewDataArrived, NULL);
    pthread_cond_init(&mNewQueueFrame, NULL);

    res = sem_init(&notifierBlock, 0, 0);    
    assert(res != -1);
                
    res = sem_init(&semIOBlock, 0, 0);    
    assert(res != -1);
    
    res = sem_init(&semExit, 0, 0);  
    assert(res != -1);
    /*
    struct sched_param shp;
    pthread_attr_t attr;
    res = pthread_attr_init(&attr);
    assert(res == 0);
        
    res = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    assert(res == 0);
    
    res = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    assert(res == 0);
    
    res = pthread_attr_getschedparam(&attr, &shp);
    assert(res == 0);
    
    printf("Priority: %d; MAX=%d", shp.sched_priority, sched_get_priority_max(SCHED_FIFO));
    shp.sched_priority = 50;

    res = pthread_attr_setschedparam(&attr, &shp);
    assert(res == 0);
    
    res = sched_setscheduler(0, SCHED_FIFO, &shp);
    printf("Rs=%d", res);
    */
    res = pthread_create(&iothread, NULL, soundCardIOWrapper, this);
    assert(res == 0);

    sem_wait(&semExit);
    assert(res == 0);
    
    /*
    int policy;
    res = pthread_getschedparam(iothread, &policy, &shp);
    assert(res == 0);
    
    printf("Priority was set: %d\n", shp.sched_priority);
    */
        
     
}

MpOSSDeviceWrapper::~MpOSSDeviceWrapper() 
{
    //printf("Called MpOSSDeviceWrapper destructor!\n"); fflush(stdout);
    
    assert((pReader == NULL) || (pWriter == NULL));
    int res;
    
    //Stopping thread
    mThreadExit = TRUE;
    res = sem_post(&semIOBlock);
    assert(res != -1);
    
    //Waiting while thread terminating    
    pthread_cond_signal(&mNewDataArrived);
    
    void *dummy;
    res = pthread_join(iothread, &dummy);
    assert(res == 0);
            
    pthread_mutex_destroy(&mWrMutex);
    pthread_cond_destroy(&mNewTickCondition);
    pthread_cond_destroy(&mNull);
    pthread_cond_destroy(&mNewDataArrived);       
    pthread_cond_destroy(&mNewQueueFrame);

    sem_destroy(&semExit);
    sem_destroy(&semIOBlock);
    
    //printf("end of MpOSSDeviceWrapper destructor!\n"); fflush(stdout);
}
                             
/* ============================ MANIPULATORS ============================== */
OsStatus MpOSSDeviceWrapper::setInputDevice(MpidOSS* pIDD)
{
    OsStatus ret = OS_FAILED;
    if (pReader) 
    {
        //Input device has been already set
        return ret;
    }
       
    pReader = pIDD;
        
    if (pWriter)
    {
        //Check than one device is selected when pWriter has been set.    
        //int res = name.compareTo(*pIDD);
        int res = pIDD->compareTo(*pWriter);
        assert (res == 0);
        ret = OS_SUCCESS;
    } else {
        //Device didn't open
        ret = initDevice(*pIDD);
    }
    if (ret != OS_SUCCESS)
    {
        pReader = NULL;   
    }    
    return ret;
}

OsStatus MpOSSDeviceWrapper::setOutputDevice(MpodOSS* pODD)
{
    OsStatus ret = OS_FAILED;
    if (pWriter) 
    {
        //Output device has been already set
        return ret;
    }
    
    pWriter = pODD;
        
    if (pReader)
    {
        //Check than one device is selected when pReader has been set.    
        int res = pODD->compareTo(*pReader);
        assert (res == 0);
        ret = OS_SUCCESS;
    } else {
        //Device didn't open
        ret = initDevice(*pODD);
    }
    if (ret != OS_SUCCESS)
    {
        pWriter = NULL;
    }                  
    return ret;
}
    
OsStatus MpOSSDeviceWrapper::freeInputDevice()
{
    OsStatus ret = OS_SUCCESS;
    if (pReaderEnabled)
    {
        printf("pReader ENABLED \n");
        //It's very bad freeing device when it is enabled
        ret = detachReader();
    }
    
    if (pReader != NULL) 
        pReader = NULL;      
    else
        ret = OS_FAILED; 
        
    if (isNotUsed())
    {
        noMoreNeeded();
    }
    return ret;
}

OsStatus MpOSSDeviceWrapper::freeOutputDevice()
{
    OsStatus ret = OS_SUCCESS;
    if (pWriterEnabled) 
    {
        printf("pWriter ENABLED \n");
        //It's very bad freeing device when it is enabled
        ret = detachWriter();
    }
        
    if (pWriter != NULL) 
        pWriter = NULL;        
    else
        ret = OS_FAILED; 

    if (isNotUsed())
    {
        noMoreNeeded();
    }                  
    return ret;
}

void MpOSSDeviceWrapper::noMoreNeeded()
{
    //printf("freeing OSS \n"); fflush(stdout);
    //OsSysLog::add(FAC_MP, PRI_DEBUG, "Freeing OSS device...\n");

    //Free OSS deive If it no longer using
    freeDevice();

#ifndef OSS_SINGLE_DEVICE     
    //Remove this wrapper from container
    mOSSContainer.excludeFromContainer(this);
#endif
}
    
OsStatus MpOSSDeviceWrapper::attachReader()
{
    //printf("OSS: AttachReader\n");
    OsStatus ret = OS_FAILED;
    if ((pReader == NULL) || (pReaderEnabled == TRUE))
        return ret;
    
    ret = setSampleRate(pReader->mSamplesPerSec); 
    if (ret == OS_SUCCESS) {
        pReaderEnabled = TRUE;
        emitDriverStatusChanged(true); 
        
        //printf(">rok\n");  
    }   
    ossSetTrigger(pReaderEnabled, pWriterEnabled);
    return ret;
}

OsStatus MpOSSDeviceWrapper::attachWriter()
{
    //printf("OSS: AttachWriter\n");
    OsStatus ret = OS_FAILED;
    if ((pWriter == NULL) || (pWriterEnabled == TRUE))
        return ret;
            
    ret = setSampleRate(pWriter->mSamplesPerSec);
        
    if (ret == OS_SUCCESS) {
        mWrCurrentSample = new MpAudioSample[pWriter->mSamplesPerFrame];                   
        
        pWriterEnabled = TRUE;
        mbFisrtWriteCycle = TRUE;
        emitDriverStatusChanged(true);            
        //printf(">wok\n"); 
    } else {
        mWrCurrentSample = NULL;
    }
    ossSetTrigger(pReaderEnabled, pWriterEnabled);
    return ret;
}
    
OsStatus MpOSSDeviceWrapper::detachReader()
{
    //printf("OSS: DetachReader\n");
    OsStatus ret = OS_FAILED;
    if ((pReader == NULL) || (pReaderEnabled == FALSE))
        return ret;
    
    pReaderEnabled = FALSE;
    emitDriverStatusChanged(false);    
    
    ossSetTrigger(pReaderEnabled, pWriterEnabled);    
    delete[] mWrCurrentSample;
    //printf(">rok\n");  
    
    OsSysLog::add(FAC_MP, PRI_DEBUG,
                    "OSS: Reader Statistic: Captured %d, Dropped %d", mFramesRead, mFramesDropRead);
    return OS_SUCCESS;
}

OsStatus MpOSSDeviceWrapper::detachWriter()
{
    //printf("OSS: DetachWriter\n");    
    OsStatus ret = OS_FAILED;
    if ((pWriter == NULL) || (pWriterEnabled == FALSE))
        return ret;
    
    pWriterEnabled = FALSE;
    emitDriverStatusChanged(false);  
    
    //printf(">wok\n");     
    ossSetTrigger(pReaderEnabled, pWriterEnabled);
    
    OsSysLog::add(FAC_MP, PRI_DEBUG,
                    "OSS: Writer Statistic: Played %d; Underruns %d; Reclocks %d; "
                    "TUps %d; TDowns %d",
                    mFramesWritten, mFramesWrUnderruns, mReClocks, mCTimeUp, mCTimeDown);
                        
    return OS_SUCCESS;
}

 UtlBoolean MpOSSDeviceWrapper::ossSetTrigger(bool read, bool write)
 {
    unsigned int val = ((read) ? PCM_ENABLE_INPUT : 0) |
                      ((write) ? PCM_ENABLE_OUTPUT : 0);
    UtlBoolean res = (ioctl(mfdDevice, SNDCTL_DSP_SETTRIGGER, &val) != -1);
    if (!res) 
    {
          OsSysLog::add(FAC_MP, PRI_EMERG,
                    "OSS: could not set OSS trigger\n");  
    }    
    return res;
 }
 
 
UtlBoolean MpOSSDeviceWrapper::ossGetBlockSize(unsigned& block)
{
    return (ioctl(mfdDevice, SNDCTL_DSP_GETBLKSIZE, &block) == -1);
}

UtlBoolean MpOSSDeviceWrapper::ossGetODelay(unsigned& delay)
{
    return (ioctl(mfdDevice, SNDCTL_DSP_GETODELAY, &delay) == -1);
}
 
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
OsStatus MpOSSDeviceWrapper::initDevice(const char* devname)
{
    int res;
    int stereo = OSS_SOUND_STEREO;
    int samplesize = 8 * sizeof(MpAudioSample);
    
    OsStatus ret = OS_FAILED;
    
    mfdDevice = open(devname, O_RDWR);
    if (mfdDevice == -1)
    {
        OsSysLog::add(FAC_MP, PRI_EMERG,
                    "OSS: could not open %s; *** NO SOUND! ***", devname);
        ret = OS_INVALID_ARGUMENT;
        return ret;
    }    
   
    res = ioctl(mfdDevice, SNDCTL_DSP_SETDUPLEX, 0);
    if (res == -1)
    {
        OsSysLog::add(FAC_MP, PRI_EMERG,
                    "OSS: could not set full duplex; *** NO SOUND! ***");
        freeDevice();
        return ret;
    }    
    
    // magic number, reduces latency (0x0004 dma buffers of 2 ^ 0x0008 = 256 bytes each) 
    //int fragment = 0x00040008; 
    
    //int fragment = 0x00040007; 
    int fragment = 0x00080007; 
    res = ioctl(mfdDevice, SNDCTL_DSP_SETFRAGMENT, &fragment);
    if(res == -1)
    {
        OsSysLog::add(FAC_MP, PRI_EMERG,
                    "OSS: could not set fragment size; *** NO SOUND! ***\n");
        freeDevice();
        return ret;
    }
    
    
    res = ioctl(mfdDevice, SNDCTL_DSP_STEREO, &stereo);
    if (res == -1)
    {
        OsSysLog::add(FAC_MP, PRI_EMERG,
                    "OSS: could not set single channel audio; *** NO SOUND! ***");
        freeDevice();
        return ret;
    }    
    
    res = ioctl(mfdDevice, SNDCTL_DSP_SAMPLESIZE, &samplesize);
    if ((res == -1) || (samplesize != (8 * sizeof(MpAudioSample))))
    {
        OsSysLog::add(FAC_MP, PRI_EMERG,
                    "OSS: could not set sample size; *** NO SOUND! ***");
        freeDevice();
        return ret;
    }
    
    // Make sure the sound card has the capabilities we need
    // FIXME: Correct this code for samplesize != 16 bits
    res = AFMT_QUERY;
    ioctl(mfdDevice ,SNDCTL_DSP_SETFMT, &res);
    if (res != AFMT_S16_LE)
    {
        OsSysLog::add(FAC_MP, PRI_EMERG,
                    "OSS: could not set sample format; *** NO SOUND! ***");
        freeDevice();
        return ret;
    }
    
    res = ioctl(mfdDevice, SNDCTL_DSP_GETCAPS, &mDeviceCap);
    if (res == -1)
    {
        OsSysLog::add(FAC_MP, PRI_EMERG,
                    "OSS: could not get capabilities; *** NO SOUND! ***");
        freeDevice();
        return ret;
    }
    
    if (!isDevCapDuplex())
    {
        OsSysLog::add(FAC_MP, PRI_EMERG,
                    "OSS: this device dosen't support "
                    "duplex operations; *** NO SOUND! ***");
        freeDevice();
        return ret;    
    }
    
    if (isDevCapBatch())
    {
        OsSysLog::add(FAC_MP, PRI_WARNING,
                    "OSS: Soundcard has internal buffers (increases latency)");    
    }
    
    if (!getOSpace(fullOSpace)) 
    {
        OsSysLog::add(FAC_MP, PRI_EMERG,
                    "OSS: this device dosen't support OSPACE ioctl! *** NO SOUND! ***");    
    }
    
    ret = OS_SUCCESS;       
    return ret;
}

OsStatus MpOSSDeviceWrapper::freeDevice()
{
    OsStatus ret = OS_SUCCESS;
    //Closing devive
    if (mfdDevice != -1) 
    {
       int res = close(mfdDevice);
       if (res != 0) 
           ret = OS_FAILED;
       
       mfdDevice = -1;
    }
    return ret;
}

OsStatus MpOSSDeviceWrapper::setSampleRate(unsigned samplesPerSec)
{    
    // FIXME: OSS device support only one samplerate for duplex operation
    // Cheking requested rate with whehter configured
    if ( (((pReaderEnabled && !pWriterEnabled) ||
           (!pReaderEnabled && pWriterEnabled)) && 
            (mUsedSamplesPerSec > 0) &&
            (mUsedSamplesPerSec != samplesPerSec)) ||
         (pReaderEnabled && pWriterEnabled))
    {    
        OsSysLog::add(FAC_MP, PRI_EMERG,
                    "OSS: could not set different sample speed for "
                    "input and output; *** NO SOUND! ***");             
        return OS_INVALID_ARGUMENT;
    }
    
    int speed = samplesPerSec;
    int res = ioctl(mfdDevice, SNDCTL_DSP_SPEED, &speed);
    // allow .5% variance
    if ((res == -1) || (abs(speed - samplesPerSec) > samplesPerSec / 200))
    {
       OsSysLog::add(FAC_MP, PRI_EMERG,
                    "OSS: could not set sample speed; *** NO SOUND! ***");     
       return OS_FAILED;
    }    
    
    // Hmm.. If it's used sample rate may be set value returned by IOCTL..
    mUsedSamplesPerSec = samplesPerSec;
    return OS_SUCCESS;
}

OsStatus MpOSSDeviceWrapper::doInput(char* buffer, int size)
{
    //Perform reading
    int bytesToRead = size;
    int bytesReadSoFar = 0;

    while (bytesReadSoFar < bytesToRead)
    {
        int bytesJustRead = read(mfdDevice, &buffer[bytesReadSoFar],
                           bytesToRead - bytesReadSoFar);
        
        if (bytesJustRead == -1) {
            return OS_FAILED;
        }
        bytesReadSoFar += bytesJustRead;
    }
    
    return OS_SUCCESS;
}

OsStatus MpOSSDeviceWrapper::doOutput(char* buffer, int size)
{
    //Perform writting
    int bytesToWrite = size;
    int bytesWritedSoFar = 0;

    while (bytesWritedSoFar < bytesToWrite)
    {
        int bytesJustWrite = write(mfdDevice, &buffer[bytesWritedSoFar],
                           bytesToWrite - bytesWritedSoFar);
        
        if (bytesJustWrite == -1) {
            return OS_FAILED;
        }
        bytesWritedSoFar += bytesJustWrite;
    }
    
    return OS_SUCCESS;
}


UtlBoolean MpOSSDeviceWrapper::getISpace(unsigned& ispace)
{
    audio_buf_info bi;
    int res = ioctl(mfdDevice, SNDCTL_DSP_GETISPACE, &bi);
    
    if (res != -1) {
        ispace = bi.bytes;
        return TRUE;
    } 
    return FALSE;
}

UtlBoolean MpOSSDeviceWrapper::getOSpace(unsigned& ospace)
{
    audio_buf_info bi;
    int res = ioctl(mfdDevice, SNDCTL_DSP_GETOSPACE, &bi);
    
    if (res != -1) {
        ospace = bi.bytes;
        return TRUE;
    } 
    return FALSE;
}

OsStatus MpOSSDeviceWrapper::emitDriverStatusChanged(bool bAdded)
{
    OsStatus ret = OS_SUCCESS;
    int res;

    // Neither reading nor writing ops
    if (!pWriterEnabled && !pReaderEnabled)
    {   
        //Blocking IO thread because no IO operation can occurs
        res = sem_wait(&semIOBlock);
        assert(res != -1);

        //printf("Blocking Thread\n"); fflush(stdout);     
        //Now can set various sample rate
        mUsedSamplesPerSec = 0;           
    } 
    // Started reading or writing ops
    else if (((pWriterEnabled && !pReaderEnabled) ||
             (!pWriterEnabled && pReaderEnabled)) &&
             bAdded)
    {    
        //Unblocking IO thread
        res = sem_post(&semIOBlock);
        assert(res != -1);
        
        pthread_cond_signal(&mNewDataArrived);        
        //printf("Unblocking Thread\n"); fflush(stdout);
    }

    return ret;
} 

//#define DEBUG_OSS_TIMERS

#ifdef DEBUG_OSS_TIMERS
#define dossprintf(x)  printf(x)
#else
#define dossprintf(x)
#endif 


void MpOSSDeviceWrapper::performOnlyRead()
{
    OsStatus status;
    MpAudioSample* frm;
    frm = pReader->getBuffer();
    //Do reader staff
    status = doInput((char*)frm, sizeof(MpAudioSample) * pReader->mSamplesPerFrame);
    assert (status == OS_SUCCESS);
    
    pReader->pushFrame(frm);
    
    mFramesRead++;
}

#define USEC_CORRECT_STEP   5

void MpOSSDeviceWrapper::performReaderNoDelay()
{
    if (!pReaderEnabled)
        return;
            
    unsigned inSizeReady;
    getISpace(inSizeReady);
    
    if (inSizeReady >= sizeof(MpAudioSample) * pReader->mSamplesPerFrame)
    {
        MpAudioSample* frm;     
        frm = pReader->getBuffer();
        //Do reader staff
        OsStatus status = doInput((char*)frm, sizeof(MpAudioSample) * pReader->mSamplesPerFrame);
        //assert (status == OS_SUCCESS);
        
        pReader->pushFrame(frm);
        
        mFramesRead++;
    }
}

void MpOSSDeviceWrapper::performWithWrite()
{
    int res;
    int bTimedOut = 0;   
    struct timeval tm;
    
    RTL_BLOCK("MpOSSDeviceWrapper::pww");
    
    if (mbFisrtWriteCycle) 
    {
        musecFrameTime = (1000000 * pWriter->mSamplesPerFrame) / pWriter->mSamplesPerSec;
        mbFisrtWriteCycle = FALSE;   
                
        unsigned silenceSize;// = 128;
        unsigned ospace;
        res = getOSpace(ospace);
        silenceSize = ospace;
        
        assert(res != 0);
        
        if (pWriter->mSamplesPerFrame * sizeof(MpAudioSample)  < silenceSize)
        {
            silenceSize = pWriter->mSamplesPerFrame * sizeof(MpAudioSample) ;
        }
        
        {
            struct timeval before;
            struct timeval after;
            char silenceData[silenceSize];
            unsigned ospace2;
            
            gettimeofday(&before, NULL);
            memset(silenceData, 0, silenceSize);
            doOutput(silenceData, silenceSize);
            gettimeofday(&after, NULL);
            
            int delta = timediff(before, after);
            res = getOSpace(ospace2);

            OsSysLog::add(FAC_MP, PRI_DEBUG,
                        "Timediff: %d usec; Size=%d; OSpace=%d (%d)", delta, silenceSize, ospace, ospace2);
        }   
        
        musecJitterCorrect = 0;//-USEC_CORRECT_STEP; //Ensure buffer would not emptying
        musecTimeCorrect = 0;
        
        res = gettimeofday(&tm, NULL);
        assert(res == 0);
        
        mWrTimeStarted.tv_sec = tm.tv_sec;
        mWrTimeStarted.tv_nsec = tm.tv_usec * 1000;              
    }
    else
    {
        res = gettimeofday(&tm, NULL);
        assert(res == 0);
        int usec_delta = ((tm.tv_sec - mWrTimeStarted.tv_sec) * 1000000 + (tm.tv_usec - mWrTimeStarted.tv_nsec / 1000));
            
        if (abs(usec_delta) > ((int)musecFrameTime) / 2) {
            mWrTimeStarted.tv_sec = tm.tv_sec;
            mWrTimeStarted.tv_nsec = tm.tv_usec * 1000;
            mReClocks++;    
            
            musecJitterCorrect = 0;//-USEC_CORRECT_STEP;    //Ensure buffer would not emptying
        }
        else
        {
            /*
            if (usec_delta > 0)
            {
                musecTimeCorrect ++;
            }
            else if (usec_delta < 0)
            {
                musecTimeCorrect--;
            }
            */          
            
            //musecTimeCorrect -= usec_delta / 10;
            //printf("%d\n", usec_delta);
    RTL_EVENT("MpOSSDeviceWrapper::pww", 2);        
            unsigned int currecntOSpace;
            unsigned int correctOSpace = 0;
            getOSpace(currecntOSpace);
            if (pReaderEnabled)
            {
                getISpace(correctOSpace);
                int pSize = pReader->mSamplesPerFrame *  sizeof(MpAudioSample);
                
                if (correctOSpace > 3*pSize)
                {
                    //Drop packet 
                    char silenceData[pSize];                         
                    doInput(silenceData, pSize);      
                    getISpace(correctOSpace);         
                    
                    mFramesDropRead++;                         
                }
            }
        
            if (((currecntOSpace - correctOSpace) / sizeof(MpAudioSample)) < pWriter->mSamplesPerFrame )
            {
                //Buffer is fulling
                musecJitterCorrect += USEC_CORRECT_STEP;
                mCTimeUp++;
            }
            else if (((fullOSpace - correctOSpace - currecntOSpace) / sizeof(MpAudioSample)) < 2*pWriter->mSamplesPerFrame)
            {
                //Buffer is emptying
                musecJitterCorrect -= USEC_CORRECT_STEP;
                mCTimeDown++;                   
            }
        }        
    }
       
    int nsDeviceDelay = 1000 * 1000; //Assuming 1000us is a maximum device delay
    if (pReaderEnabled)
    {
        nsDeviceDelay = 2000 * 1000;
    }
    RTL_EVENT("MpOSSDeviceWrapper::pww", 3);            
    if (pWriter->isNotificationNeeded())
    {        
        mWrTimeStarted.tv_nsec += musecFrameTime * 1000 - nsDeviceDelay + musecJitterCorrect * 1000;
        while (mWrTimeStarted.tv_nsec > 1000000000) 
        {
            mWrTimeStarted.tv_nsec -= 1000000000;
            mWrTimeStarted.tv_sec++;
        }
     
        performReaderNoDelay();   
                
        pWriter->signalForNextFrameWithCond();            
        res = pthread_cond_timedwait(&mNewTickCondition, &mWrMutex, &mWrTimeStarted);
        if (res == ETIMEDOUT)
        {
            bTimedOut = 1;
        }
        else assert(res == 0);
        
        RTL_EVENT("MpOSSDeviceWrapper::pww", 4);
        if (!bTimedOut)
        {
            doOutput((char*)mWrCurrentSample, pWriter->mSamplesPerFrame * sizeof(MpAudioSample));        
            mFramesWritten++;
        }
        else
        {
	    RTL_EVENT("MpOSSDeviceWrapper::pww", 5);
            //unsigned int currecntOSpace;
            //getOSpace(currecntOSpace);
            //if ((fullOSpace - currecntOSpace / sizeof(MpAudioSample) > 2*pWriter->mSamplesPerFrame))
            //{
                //So we have several bytes in buffer, and can use them
                //Just simply wait for next frame
            //}
            //else
            //{                                
                int silenceSize = pWriter->mSamplesPerFrame * sizeof(MpAudioSample);
                {
                    char silenceData[silenceSize];
                    memset(silenceData, 0, silenceSize);       
                    doOutput(silenceData, silenceSize);
                }     
                mFramesWrUnderruns++;      
            //}                  
        }
        //performReaderNoDelay();
        
        mWrTimeStarted.tv_nsec += nsDeviceDelay;
        while (mWrTimeStarted.tv_nsec > 1000000000) 
        {
            mWrTimeStarted.tv_nsec -= 1000000000;
            mWrTimeStarted.tv_sec++;
        } 
	
	RTL_EVENT("MpOSSDeviceWrapper::pww", 6);
	{
	RTL_BLOCK("MpOSSDeviceWrapper::pww time correction");
	int delta_delay;
	struct timeval tmv;
	res = gettimeofday(&tmv, NULL);
	delta_delay = (mWrTimeStarted.tv_sec - tmv.tv_sec) * 1000000000 + (mWrTimeStarted.tv_nsec - 1000 * tmv.tv_usec);
	if (delta_delay) {
	    struct timespec gg;
	    struct timespec df;
	    df.tv_sec = 0;
	    df.tv_nsec = delta_delay;
	    //nanosleep(&df, &gg);	    
	}
	

        //res = pthread_cond_timedwait(&mNull, &mWrMutex, &mWrTimeStarted);
	//assert (res == ETIMEDOUT);
	}
	RTL_EVENT("MpOSSDeviceWrapper::pww", 7);
    }
    else //No notification is need
    {
        mWrTimeStarted.tv_nsec += musecFrameTime * 1000 - nsDeviceDelay + musecJitterCorrect * 1000;
        while (mWrTimeStarted.tv_nsec > 1000000000) 
        {
            mWrTimeStarted.tv_nsec -= 1000000000;
            mWrTimeStarted.tv_sec++;
        }       
                        
        unsigned bnFrameSize;
        MpAudioSample* samples = pWriter->popFrame(bnFrameSize);       
        if (samples)
        {
            doOutput((char*)samples, bnFrameSize);
            mFramesWritten++;
        }
        else 
        {
            res = pthread_cond_timedwait(&mNewQueueFrame, &mWrMutex, &mWrTimeStarted);    
            if (res == ETIMEDOUT)
            {
                int silenceSize = pWriter->mSamplesPerFrame * sizeof(MpAudioSample);
                {
                    char silenceData[silenceSize];
                    memset(silenceData, 0, silenceSize);       
                    doOutput(silenceData, silenceSize);
                }     
                mFramesWrUnderruns++;      
            }
            else
            {
                assert(res == 0);
                samples = pWriter->popFrame(bnFrameSize);    
                doOutput((char*)samples, bnFrameSize);
                mFramesWritten++;
            }      
        }
        
        performReaderNoDelay();
        
        mWrTimeStarted.tv_nsec += nsDeviceDelay;
        while (mWrTimeStarted.tv_nsec > 1000000000) 
        {
            mWrTimeStarted.tv_nsec -= 1000000000;
            mWrTimeStarted.tv_sec++;
        }
	
	
	
	int delta_delay;
	struct timeval tmv;
	/*
	res = gettimeofday(&tmv, NULL);
	
	delta_delay = (mWrTimeStarted.tv_sec - tmv.tv_sec) * 1000000000 + (mWrTimeStarted.tv_nsec - 1000 * tmv.tv_usec);
	if (delta_delay) {
	    struct timespec gg;
	    struct timespec df;
	    df.tv_sec = 0;
	    df.tv_nsec = delta_delay;
	    nanosleep(&df, &gg);
	    
	}*/
        
        //mWrTimeStarted.tv_nsec += musecJitterCorrect * 1000 * 3 / 4;
	/*
	mWrTimeStarted.tv_nsec += musecJitterCorrect * 1000 * 3 / 4;
        while (mWrTimeStarted.tv_nsec > 1000000000) 
        {
            mWrTimeStarted.tv_nsec -= 1000000000;
            mWrTimeStarted.tv_sec++;
        }   */
//        res = pthread_cond_timedwait(&mNull, &mWrMutex, &mWrTimeStarted);           
//        assert (res == ETIMEDOUT);
    }   
    
       
    
    /*res = gettimeofday(&tm, NULL);
    mWrTimeStarted.tv_sec = tm.tv_sec;
    mWrTimeStarted.tv_nsec = tm.tv_usec * 1000;*/
}
/*
void MpOSSDeviceWrapper::performDuplexRW()
{

}
*/
void MpOSSDeviceWrapper::soundIOThread()
{
    int res;
    int bLock = 0;
    
    for (;;)
    {        
        //Begin IO
        res = sem_wait(&semIOBlock);
        assert(res != -1);
        
        if (mThreadExit) {
            //dossprintf("Exit flag recived\n"); fflush(stdout);
            //sem_post(&pOSSDW->semExit);
            return;
        }
                
        if (pReaderEnabled && !pWriterEnabled)
        {
            performOnlyRead();
        } 
        else if (pWriterEnabled)
        {
            performWithWrite();
        }
         /*   
        else if (!pReaderEnabled && pWriterEnabled)
        {
            performOnlyWrite();
        }
        else if (pReaderEnabled && pWriterEnabled)
        {
            //performDuplexRW();
            performOnlyWrite();
        }*/
        else //if (!pReaderEnabled && !pWriterEnabled)
        {
            bLock = TRUE;
        }
        
        res = sem_post(&semIOBlock);
        assert(res != -1);

        if (bLock)
        {
            //res = sem_post(&semIOBlock);
            //assert(res != -1);        
            
            //Wait an event to wakeup    
            res = pthread_cond_wait(&mNewDataArrived, &mWrMutex);
            assert(res == 0);
            
            bLock = FALSE;
        }    
    }
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */
void* MpOSSDeviceWrapper::soundCardIOWrapper(void* arg)
{
    MpOSSDeviceWrapper* pOSSDW = (MpOSSDeviceWrapper*)arg;
    // Setup thread priority
    pthread_mutex_lock(&pOSSDW->mWrMutex);
#if defined(_REALTIME_LINUX_AUDIO_THREADS) && defined(__linux__) /* [ */
    {
        struct sched_param realtime;
        int res;
        
        if(geteuid() != 0)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                        "_REALTIME_LINUX_AUDIO_THREADS was defined"
                        " but application does not have ROOT priv.");
        }
        else
        {
            // Set the priority to the maximum allowed for the scheduling polity.
            realtime.sched_priority = sched_get_priority_max(SCHED_FIFO);
            res = sched_setscheduler(0, SCHED_FIFO, &realtime);
            assert(res == 0);
        
            // keep all memory locked into physical mem, to guarantee realtime-behaviour
            res = mlockall(MCL_CURRENT|MCL_FUTURE);
            assert(res == 0);
        }
    }
#endif /* _REALTIME_LINUX_AUDIO_THREADS ] */            
    sem_post(&pOSSDW->semExit);
    
    pOSSDW->soundIOThread();
    
    //sem_post(&pOSSDW->semExit);    
    // Some unititialization here
    pthread_mutex_unlock(&pOSSDW->mWrMutex);
    return NULL;
}
    
#if 0
/* //////////////////////////// PRIVATE /////////////////////////////////// */
void* MpOSSDeviceWrapper::soundCardIOWrapper(void* arg)
{
    MpOSSDeviceWrapper* pOSSDW = (MpOSSDeviceWrapper*)arg;
    int res;
    //sem_wait(&pOSSDW->semExit);
//This realtime code was copied from Joe's code 
#if 0
#if defined(_REALTIME_LINUX_AUDIO_THREADS) && defined(__linux__) /* [ */
    {
        struct sched_param realtime;
        int res;
        
        if(geteuid() != 0)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                        "_REALTIME_LINUX_AUDIO_THREADS was defined"
                        " but application does not have ROOT priv.");
        }
        else
        {
            // Set the priority to the maximum allowed for the scheduling polity.
            realtime.sched_priority = sched_get_priority_max(SCHED_FIFO);
            res = sched_setscheduler(0, SCHED_FIFO, &realtime);
            assert(res == 0);
        
            // keep all memory locked into physical mem, to guarantee realtime-behaviour
            res = mlockall(MCL_CURRENT|MCL_FUTURE);
            assert(res == 0);
        }
    }
#endif /* _REALTIME_LINUX_AUDIO_THREADS ] */
#endif
/*
            struct sched_param realtime;

            // Set the priority to the maximum allowed for the scheduling polity.
            realtime.sched_priority = sched_get_priority_max(SCHED_FIFO);
            res = sched_setscheduler(0, SCHED_FIFO, &realtime);
            assert(res == 0);
            */

    OsStatus status;
    MpAudioSample* frm;
    UtlBoolean bSwitch = FALSE;
    
    MpAudioSample* pendingReaded = NULL;
    unsigned part_readed = 0;
    
    for (;;)
    {
        bool doSomeDelay = false;
        
        //Begin IO
        res = sem_wait(&pOSSDW->semIOBlock);
        assert(res != -1);
                
        // Check if we need to exit immediatly (MpOSSDeviceWrapper is destroing)
        if (pOSSDW->mThreadExit) {
            dossprintf("Exit flag recived\n"); fflush(stdout);
            //sem_post(&pOSSDW->semExit);
            return NULL;
        }
        
        if ((pOSSDW->pReaderEnabled) && (!pOSSDW->pWriterEnabled))  {
//#ifdef BLOCKED_READ        
            frm = pOSSDW->pReader->getBuffer();
            //Do reader staff
            status = pOSSDW->doInput((char*)frm, sizeof(MpAudioSample) * pOSSDW->pReader->mSamplesPerFrame);
            if (status == OS_SUCCESS)
            {
                dossprintf("r");
                pOSSDW->pReader->pushFrame(frm);
            } 
            else 
            {
                dossprintf("FAILED TO GET FRAME\n");
            }
        }
        else if (pOSSDW->pReaderEnabled)
        {    
#if 0        
            if (pOSSDW->pWriterEnabled) 
                pOSSDW->pWriter->signalForNextFrame();
                
            frm = pOSSDW->pReader->getBuffer();
            //Do reader staff
            status = pOSSDW->doInput((char*)frm, sizeof(MpAudioSample) * pOSSDW->pReader->mSamplesPerFrame);
            if (status == OS_SUCCESS)
            {
                dossprintf("r");
                pOSSDW->pReader->pushFrame(frm);
            } 
            else 
            {
                dossprintf("FAILED TO GET FRAME\n");
            }        
//#else
#endif  
#if 0           
            unsigned size = sizeof(MpAudioSample) * pOSSDW->pReader->mSamplesPerFrame;
            unsigned safe_size;
            if  (!((pOSSDW->getISpace(safe_size) && (safe_size < size))))
            {
                frm = pOSSDW->pReader->getBuffer();
                status = pOSSDW->doInput((char*)frm, sizeof(MpAudioSample) * pOSSDW->pReader->mSamplesPerFrame);
                if (status == OS_SUCCESS)
                {
                    dossprintf("r");
                    pOSSDW->pReader->pushFrame(frm);
                }
            }
            else
            {
                dossprintf("|");
                //OsTask::delay(1);
                doSomeDelay = true;
            }
#else            
            unsigned int safe_size;
            if  (!((pOSSDW->getISpace(safe_size) && (safe_size < sizeof(MpAudioSample)  * pOSSDW->pReader->mSamplesPerFrame / 2 ))))
            {
                unsigned int samplesize = sizeof(MpAudioSample) * pOSSDW->pReader->mSamplesPerFrame;
                unsigned int size_to_read = samplesize - part_readed;
                if (size_to_read > safe_size) {
                    size_to_read = safe_size;
                }
                    
                if (part_readed == 0) 
                {
                    pendingReaded = pOSSDW->pReader->getBuffer();                            
                }
                status = pOSSDW->doInput(((char*)pendingReaded) + part_readed, size_to_read);                                    
                if (status == OS_SUCCESS)
                {
                    dossprintf("r");
                    part_readed += size_to_read;
                    
                    if (part_readed == samplesize)
                    {
                        dossprintf("-");    
                        pOSSDW->pReader->pushFrame(pendingReaded);
                        part_readed = 0;
                    }
                }     
            }
            else
            {
                dossprintf("|");
                doSomeDelay = true;
            }
#endif               
//#else                        
//#endif            
        } 
        if (pOSSDW->pWriterEnabled) {
            //Do writer staff
            //pOSSDW->doOutput();
            unsigned size = sizeof(MpAudioSample) * pOSSDW->pWriter->mSamplesPerFrame;
            unsigned safe_size;
            
#if 1
            unsigned bnFrameSize;
            
            if (pOSSDW->getOSpace(safe_size) && (safe_size >= size)) 
            {
                pOSSDW->pWriter->signalForNextFrameAndDirectWrite();
            }
            else
            {
            pOSSDW->pWriter->signalForNextFrame();
            pthread_yield();
            MpAudioSample* samples = pOSSDW->pWriter->popFrame(bnFrameSize);
            if (samples)
            {
                dossprintf("o");
                pOSSDW->doOutput((char*)samples, bnFrameSize);
                //We already done IO in this circle, no delay
                doSomeDelay = false;
            } else if (pOSSDW->getOSpace(safe_size) && (safe_size > size)) {
                //Underruns
                char mem[size];
                memset(mem, 0, size);
                
                pOSSDW->doOutput((char*)mem, size);
                dossprintf("U");
            }
            }
            
            //doSomeDelay = false;          

        }
#else            
            if  (!((pOSSDW->getOSpace(safe_size) && (safe_size < size))))
            {
                unsigned bnFrameSize;
                pOSSDW->pWriter->signalForNextFrame();
                MpAudioSample* samples = pOSSDW->pWriter->popFrame(bnFrameSize);
                if (samples == NULL) 
                {
                    //printf("MpOSSDeviceWrapper: BAAD Out Sampple!\n");                
                    dossprintf("N");
                    if (!pOSSDW->pReaderEnabled)
                    {
                        //OsTask::delay(1);
                        doSomeDelay = true;
                    }
                } 
                else
                {
                    dossprintf("o");
                    pOSSDW->doOutput((char*)samples, bnFrameSize);
                    
                    //We already done IO in this circle, no delay
                    doSomeDelay = false;
                }                
            } else if (!pOSSDW->pReaderEnabled) {
                dossprintf("F");
                //Wait while output buffer empting
                //OsTask::delay(1);
                doSomeDelay = true;
            }            
        }
#endif
       
        if (!pOSSDW->pWriterEnabled && !pOSSDW->pReaderEnabled)
        {
            //It's rarely can occur this
            bSwitch = TRUE;
        }
                 
        res = sem_post(&pOSSDW->semIOBlock);
        assert(res != -1);
        //End of IO
        
        if (doSomeDelay)
        {
            dossprintf("D");
            pthread_yield();
            //OsTask::delay(1);        
        }
        
        if (bSwitch)
        {
            //Let to disable this thread by another thread
            pthread_yield();            
            bSwitch = FALSE;
            
            // FIXME: FOR TEST PORPOSE ONLY. REMOVE THIS IN RELEASE
            dossprintf("MpOSSDeviceWrapper: Input and Ouput are both disabled!\n");
        }    
        
    }
}

#endif //SOME_UNDIFENED_IDENTIFICATOR


#endif // __linux__
