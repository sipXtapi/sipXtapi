//  
// Copyright (C) 2007-2010 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

// SYSTEM INCLUDES
#include <mp/MpTypes.h>
#include <assert.h>
#include <math.h>

// APPLICATION INCLUDES
#include <mp/MpInputDeviceManager.h>
#include <mp/MpSineWaveGeneratorDeviceDriver.h>
#include <os/OsServerTask.h>
#include <os/OsTimer.h>
#include <os/OsEventMsg.h>
#ifdef RTL_ENABLED
#   include <rtl_macro.h>
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#ifndef M_PI
#   define M_PI 3.14159265358979323846
#endif

// 0 is the Highest priority
#define SINE_WAVE_DEVICE_PRIORITY 0

// STATIC VARIABLE INITIALIZATIONS
// PRIVATE CLASSES
class MpSineWaveGeneratorServer : public OsServerTask
{
public:
    MpSineWaveGeneratorServer(unsigned int startFrameTime,
                              unsigned int samplesPerFrame,
                              unsigned int samplesPerSecond,
                              unsigned int magnitude,
                              unsigned int periodMicroseconds,
                              int relativeSpeed,
                              MpInputDeviceHandle deviceId,
                              MpInputDeviceManager& inputDeviceManager)
    : OsServerTask("MpSineWaveGeneratorServer-%d", NULL, DEF_MAX_MSGS, SINE_WAVE_DEVICE_PRIORITY),
    mNextFrameTime(startFrameTime),
    mSamplesPerFrame(samplesPerFrame),
    mSamplesPerSecond(samplesPerSecond),
    mMagnitude(magnitude),
    mSinePeriodMicroseconds(periodMicroseconds),
    mRelativeSpeed(relativeSpeed),
    mDeviceId(deviceId),
    mpInputDeviceManager(&inputDeviceManager),
    mpFrameData(NULL),
    mTimer(getMessageQueue(), 0)
    {
        mpFrameData = new MpAudioSample[mSamplesPerFrame];
    };

    virtual ~MpSineWaveGeneratorServer()
    {
        OsSysLog::add(FAC_MP, PRI_DEBUG,"~MpSineWaveGeneratorServer start");

        // Do not continue until the task is safely shutdown
        waitUntilShutDown();
        assert(isShutDown());

        if(mpFrameData)
        {
            delete[] mpFrameData;
            mpFrameData = NULL;
        }
        OsSysLog::add(FAC_MP, PRI_DEBUG,"~MpSineWaveGeneratorServer end");
    };

    virtual UtlBoolean start(void)
    {
        OsSysLog::add(FAC_MP, PRI_DEBUG,"MpSineWaveGeneratorServer::start start");
        // start the task
        UtlBoolean result = OsServerTask::start();

        OsTime noDelay(0, 0);
        int microSecondsPerFrame =
            (mSamplesPerFrame * 1000000 / mSamplesPerSecond) -
            mRelativeSpeed;
        assert(microSecondsPerFrame > 0);
        OsTime framePeriod(0, microSecondsPerFrame);
        // Start re-occurring timer which causes handleMessage to be called
        // periodically
        mTimer.periodicEvery(noDelay, framePeriod);
        OsSysLog::add(FAC_MP, PRI_DEBUG,"MpSineWaveGeneratorServer::start end");
        return(result);
    };

    virtual void requestShutdown(void)
    {
        OsSysLog::add(FAC_MP, PRI_DEBUG,"MpSineWaveGeneratorServer::requestShutdown start");
        // Stop the timer first so it stops queuing messages
        mTimer.stop();

        // Then stop the server task
        OsServerTask::requestShutdown();
        OsSysLog::add(FAC_MP, PRI_DEBUG,"MpSineWaveGeneratorServer::requestShutdown end");
    };

    UtlBoolean handleMessage(OsMsg& rMsg)
    {
#ifdef RTL_ENABLED
        RTL_BLOCK("MpSineWaveGeneratorServer.handleMessage");
#endif
        // Build a frame of signal and push it to the device manager
        assert(mpFrameData);

        // Check that we've got expected message type.
        assert(rMsg.getMsgType() == OsMsg::OS_EVENT);
        assert(rMsg.getMsgSubType() == OsEventMsg::NOTIFY);

        for(unsigned int frameIndex = 0; frameIndex < mSamplesPerFrame; frameIndex++)
        {
            mpFrameData[frameIndex] =
                MpSineWaveGeneratorDeviceDriver::calculateSample(mNextFrameTime, 
                                                                 mMagnitude,
                                                                 mSinePeriodMicroseconds,
                                                                 frameIndex,
                                                                 mSamplesPerFrame,
                                                                 mSamplesPerSecond);
        }

        mpInputDeviceManager->pushFrame(mDeviceId,
                                        mSamplesPerFrame,
                                        mpFrameData,
                                        mNextFrameTime);
        mNextFrameTime += mSamplesPerFrame * 1000 / mSamplesPerSecond;

        return(TRUE);
    }

    void setTone(unsigned int periodInMicroseconds)
    {
       // Since this only does an atomic store, no lock is needed.
       mSinePeriodMicroseconds = periodInMicroseconds;
    }

    void setMagnitude(unsigned int magnitude)
    {
       // Since this only does an atomic store, no lock is needed.
       mMagnitude = magnitude;
    }

    void setRelativeSpeed(int relativeSpeed)
    {
       // Since this only does an atomic store, no lock is needed.
       mRelativeSpeed = relativeSpeed;
    }

private:
    MpFrameTime mNextFrameTime;
    unsigned int mSamplesPerFrame;
    unsigned int mSamplesPerSecond;
    unsigned int mMagnitude;
    unsigned int mSinePeriodMicroseconds;
    int mRelativeSpeed;
    MpInputDeviceHandle mDeviceId;
    MpInputDeviceManager* mpInputDeviceManager;
    MpAudioSample* mpFrameData;
    OsTimer mTimer;
};


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpSineWaveGeneratorDeviceDriver::MpSineWaveGeneratorDeviceDriver(const UtlString& name,
                                                                 MpInputDeviceManager& deviceManager,
                                                                 unsigned int magnitude,
                                                                 unsigned int periodInMicroseconds,
                                                                 int relativeSpeed)
: MpInputDeviceDriver(name, deviceManager),
mMagnitude(magnitude),
mPeriodInMicroseconds(periodInMicroseconds),
mRelativeSpeed(relativeSpeed),
mpReaderTask(NULL)
{
}

// Destructor
MpSineWaveGeneratorDeviceDriver::~MpSineWaveGeneratorDeviceDriver()
{
    OsSysLog::add(FAC_MP, PRI_DEBUG,"~MpSineWaveGeneratorDeviceDriver start");
    if(mpReaderTask)
    {
        OsStatus stat = disableDevice();
        assert(stat == OS_SUCCESS);
    }
    OsSysLog::add(FAC_MP, PRI_DEBUG,"~MpSineWaveGeneratorDeviceDriver end");
}

/* ============================ MANIPULATORS ============================== */

OsStatus MpSineWaveGeneratorDeviceDriver::enableDevice(unsigned samplesPerFrame, 
                                                      unsigned samplesPerSec,
                                                      MpFrameTime currentFrameTime)
{
    OsSysLog::add(FAC_MP, PRI_DEBUG,"MpSineWaveGeneratorDeviceDriver::enableDevice start");
    OsStatus result = OS_INVALID;
    assert(mpReaderTask == NULL);

    mSamplesPerFrame = samplesPerFrame;
    mSamplesPerSec = samplesPerSec;

    if(mpReaderTask == NULL)
    {
        mpReaderTask = 
            new MpSineWaveGeneratorServer(currentFrameTime,
                                          mSamplesPerFrame,
                                          mSamplesPerSec,
                                          mMagnitude,
                                          mPeriodInMicroseconds,
                                          mRelativeSpeed,
                                          getDeviceId(),
                                          *mpInputDeviceManager);

        if(mpReaderTask->start())
        {
            result = OS_SUCCESS;
            mIsEnabled = TRUE;

        }
    }
    OsSysLog::add(FAC_MP, PRI_DEBUG,"MpSineWaveGeneratorDeviceDriver::enableDevice end");
    return(result);
}

OsStatus MpSineWaveGeneratorDeviceDriver::disableDevice()
{
    OsSysLog::add(FAC_MP, PRI_DEBUG,"MpSineWaveGeneratorDeviceDriver::disableDevice start");
    OsStatus result = OS_TASK_NOT_STARTED;
    //assert(mpReaderTask);

    if(mpReaderTask)
    {
        mpReaderTask->requestShutdown();
        delete mpReaderTask;
        mpReaderTask = NULL;
        result = OS_SUCCESS;
        mIsEnabled = FALSE;
    }
    OsSysLog::add(FAC_MP, PRI_DEBUG,"MpSineWaveGeneratorDeviceDriver::disableDevice end");
    return(result);
}

OsStatus MpSineWaveGeneratorDeviceDriver::setTone(unsigned int periodInMicroseconds)
{
   // Set the new tone value.
   mPeriodInMicroseconds = periodInMicroseconds;
   ((MpSineWaveGeneratorServer*)mpReaderTask)->setTone(mPeriodInMicroseconds);
   return OS_SUCCESS;
}

OsStatus MpSineWaveGeneratorDeviceDriver::setMagnitude(unsigned int magnitude)
{
   // Set the new magnitude value.
   mMagnitude = magnitude;
   ((MpSineWaveGeneratorServer*)mpReaderTask)->setMagnitude(mMagnitude);
   return OS_SUCCESS;
}

/* ============================ ACCESSORS ================================= */
MpAudioSample 
MpSineWaveGeneratorDeviceDriver::calculateSample(MpFrameTime frameStartTime,
                                                 unsigned int magnitude,
                                                 unsigned int periodInMicroseconds,
                                                 unsigned int frameSampleIndex,
                                                 unsigned int samplesPerFrame, 
                                                 unsigned int samplesPerSecond)
{
    double time = ((frameStartTime + frameSampleIndex * 1000.0 / (double) samplesPerSecond)
        / ((double) periodInMicroseconds) * 1000.0 ) * 2.0 * M_PI;
    MpAudioSample sample = (MpAudioSample)(cos(time) * (double)magnitude);

    return(sample);
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

