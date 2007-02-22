//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <os/OsWriteLock.h>
#include <os/OsDateTime.h>
#include <mp/MpInputDeviceManager.h>
#include <mp/MpInputDeviceDriver.h>
#include <mp/MpBuf.h>
#include <mp/MpMisc.h>
#include <utl/UtlInt.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// PRIVATE CLASSES
/**
*  @brief Private class container for input device buffer and related info
*/
class MpInputDeviceFrameData
{
public:
    MpInputDeviceFrameData()
    : mFrameTime(0)
    {};

    virtual ~MpInputDeviceFrameData()
    {};

    MpAudioBufPtr mFrameBuffer;
    MpFrameTime mFrameTime;
    OsTime mFrameReceivedTime;

private:
      /// Copy constructor (not implemented for this class)
    MpInputDeviceFrameData(const MpInputDeviceFrameData& rMpInputDeviceFrameData);

      /// Assignment operator (not implemented for this class)
    MpInputDeviceFrameData& operator=(const MpInputDeviceFrameData& rhs);
};

/**
*  @brief Private class container for MpInputDeviceDriver pointer and window of buffers
*/
class MpAudioInputConnection : public UtlInt
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Default constructor
   MpAudioInputConnection(MpInputDeviceHandle deviceId,
                          MpInputDeviceDriver& deviceDriver,
                          unsigned int numFramesBuffered,
                          unsigned int samplesPerFrame,
                          unsigned int samplesPerSecond)
   : UtlInt(deviceId)
   , mLastPushedFrame(numFramesBuffered - 1)
   , mNumFramesBuffered(numFramesBuffered)
   , mppFrameBufferArray(NULL)
   , mpInputDeviceDriver(&deviceDriver)
   , mSamplesPerFrame(samplesPerFrame)
   , mSamplesPerSecond(samplesPerSecond)
   {
       assert(numFramesBuffered > 0);
       assert(samplesPerFrame > 0);
       assert(samplesPerSecond > 0);

       mppFrameBufferArray = new MpInputDeviceFrameData[mNumFramesBuffered];
   };

     /// Destructor
   virtual
   ~MpAudioInputConnection()
   {
       if (mppFrameBufferArray)
       {
           delete[] mppFrameBufferArray;
           mppFrameBufferArray = NULL;
       }
   }

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

    OsStatus pushFrame(unsigned int numSamples,
                       MpAudioSample* samples,
                       MpFrameTime frameTime)
    {
        OsStatus result = OS_FAILED;
        assert(samples);

        // TODO: could support reframing here.  For now
        // the driver must do the correct framing.
        assert(numSamples == mSamplesPerFrame);

        // Circular buffer of frames
        int thisFrameIndex = (++mLastPushedFrame) % mNumFramesBuffered;
        MpInputDeviceFrameData* thisFrameData = &mppFrameBufferArray[thisFrameIndex];

        // Current time to review device driver jitter
        OsDateTime::getCurTimeSinceBoot(thisFrameData->mFrameReceivedTime);

        // Frame time slot the driver says this is targeted for
        thisFrameData->mFrameTime = frameTime;

        // Make sure we have someplace we can stuff the data
        if (!thisFrameData->mFrameBuffer.isValid())
        {
            thisFrameData->mFrameBuffer = 
                MpMisc.RawAudioPool->getBuffer();
        }

        // Stuff the data in a buffer
        if (thisFrameData->mFrameBuffer.isValid())
        {
            memcpy(thisFrameData->mFrameBuffer->getSamples(), samples, numSamples);
            thisFrameData->mFrameBuffer->setSamplesNumber(numSamples);
            thisFrameData->mFrameBuffer->setSpeechType(MpAudioBuf::MP_SPEECH_UNKNOWN);
            result = OS_SUCCESS;
        }
        else
        {
            assert(0);
        }

        return(result);
    };

    OsStatus getFrame(MpFrameTime frameTime,
                      MpBufPtr& buffer,
                      unsigned& numFramesBefore,
                      unsigned& numFramesAfter)
    {
        OsStatus result = OS_INVALID_STATE;
        // Need to look for the frame even if the device is disabled
        // as it may already be queued up
        if (mpInputDeviceDriver && mpInputDeviceDriver->isEnabled())
        {
            result = OS_NOT_FOUND;
        }

        unsigned int lastFrame = mLastPushedFrame % mNumFramesBuffered;
        numFramesBefore = 0;
        numFramesAfter = 0;
        int framePeriod = 1000 * mSamplesPerFrame / mSamplesPerSecond;

        // When requesting a frame we provide the frame that overlaps the
        // given frame time.  The frame time is for the begining of a frame.
        // So we provide the frame that begins at or less than the requested
        // time, but not more than one frame period older.
        for (unsigned int frameIndex = 0; frameIndex < mNumFramesBuffered; frameIndex++)
        {
            MpInputDeviceFrameData* frameData = 
                &mppFrameBufferArray[(lastFrame + frameIndex) % mNumFramesBuffered];

            if (frameData->mFrameTime <= frameTime &&
                frameData->mFrameTime + framePeriod > frameTime)
            {
                // We have a frame of media for the requested time
                numFramesBefore = frameIndex;
                numFramesAfter = mNumFramesBuffered - 1 - frameIndex;

                // We always make a copy of the frame as we are typically
                // crossing task boundries here.
                buffer = frameData->mFrameBuffer.clone();
                result = OS_SUCCESS;
                break;
            }
        }
        return(result);
    }; 

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

    MpInputDeviceDriver* getDeviceDriver() const{return(mpInputDeviceDriver);};

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    unsigned int mLastPushedFrame;
    unsigned int mNumFramesBuffered;
    MpInputDeviceFrameData* mppFrameBufferArray;
    MpInputDeviceDriver* mpInputDeviceDriver;
    unsigned int mSamplesPerFrame;
    unsigned int mSamplesPerSecond;

      /// Copy constructor (not implemented for this class)
    MpAudioInputConnection(const MpAudioInputConnection& rMpAudioInputConnection);

      /// Assignment operator (not implemented for this class)
    MpAudioInputConnection& operator=(const MpAudioInputConnection& rhs);
};


//               MpInputDeviceManager implementation

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpInputDeviceManager::MpInputDeviceManager(unsigned defaultSamplesPerFrame, 
                                           unsigned defaultSamplesPerSec,
                                           unsigned defaultNumBufferedFrames)
: mRwMutex(OsRWMutex::Q_PRIORITY)
, mLastDeviceId(0)
, mDefaultSamplesPerFrame(defaultSamplesPerFrame)
, mDefaultSamplesPerSecond(defaultSamplesPerSec)
, mDefaultNumBufferedFrames(defaultNumBufferedFrames)
{
    assert(defaultSamplesPerFrame > 0);
    assert(defaultSamplesPerSec > 0);
    assert(defaultNumBufferedFrames > 0);

    OsDateTime::getCurTimeSinceBoot(mTimeZero);
}

// Destructor
MpInputDeviceManager::~MpInputDeviceManager()
{
}

/* ============================ MANIPULATORS ============================== */
int MpInputDeviceManager::addDevice(MpInputDeviceDriver& newDevice)
{
    OsWriteLock lock(mRwMutex);

    MpInputDeviceHandle newDeviceId = ++mLastDeviceId;
    // Tell the device what id to use when pusing frames to this
    newDevice.setDeviceId(newDeviceId);

    // Create a connection to contain the device and its buffered frames
    MpAudioInputConnection* connection = 
        new MpAudioInputConnection(newDeviceId,
                                   newDevice, 
                                   mDefaultNumBufferedFrames,
                                   mDefaultSamplesPerFrame,
                                   mDefaultSamplesPerSecond);

    // Map by device name string
    mConnectionsByDeviceName.insertKeyAndValue(&newDevice, new UtlInt(newDeviceId));

    // Map by device ID
    mConnectionsByDeviceId.insert(connection);

    return(newDeviceId);
}

MpInputDeviceDriver* MpInputDeviceManager::removeDevice(int deviceId)
{
    OsWriteLock lock(mRwMutex);

    MpAudioInputConnection* connectionFound = NULL;
    UtlInt deviceKey(deviceId);
    connectionFound =
        (MpAudioInputConnection*) mConnectionsByDeviceId.find(&deviceKey);
    MpInputDeviceDriver* deviceDriver = NULL;

    if(connectionFound)
    {
        // Remove from the id indexed container
        mConnectionsByDeviceId.remove(connectionFound);

        deviceDriver = connectionFound->getDeviceDriver();
        assert(deviceDriver);

        // Get the int value mapped in the hash so we can clean up
        UtlInt* deviceIdInt =
            (UtlInt*) mConnectionsByDeviceName.find(deviceDriver);

        // Remove from the name indexed hash
        mConnectionsByDeviceName.remove(deviceDriver);
        if(deviceIdInt)
        {
            delete deviceIdInt;
            deviceIdInt = NULL;
        }

        delete connectionFound;
        connectionFound = NULL;
    }

    return(deviceDriver);
}

OsStatus MpInputDeviceManager::enableDevice(int deviceId)
{
    OsStatus status = OS_NOT_FOUND;
    OsWriteLock lock(mRwMutex);

    MpAudioInputConnection* connectionFound = NULL;
    UtlInt deviceKey(deviceId);
    connectionFound =
        (MpAudioInputConnection*) mConnectionsByDeviceId.find(&deviceKey);
    MpInputDeviceDriver* deviceDriver = NULL;

    if(connectionFound)
    {
        deviceDriver = connectionFound->getDeviceDriver();
        assert(deviceDriver);
        if(deviceDriver)
        {
            status = 
                deviceDriver->enableDevice(mDefaultSamplesPerFrame, 
                                           mDefaultSamplesPerSecond,
                                           getCurrentFrameTime());
        }
    }
    return(status);
}

OsStatus MpInputDeviceManager::disableDevice(int deviceId)
{
    OsStatus status = OS_NOT_FOUND;
    OsWriteLock lock(mRwMutex);

    MpAudioInputConnection* connectionFound = NULL;
    UtlInt deviceKey(deviceId);
    connectionFound =
        (MpAudioInputConnection*) mConnectionsByDeviceId.find(&deviceKey);
    MpInputDeviceDriver* deviceDriver = NULL;

    if(connectionFound)
    {
        deviceDriver = connectionFound->getDeviceDriver();
        assert(deviceDriver);
        if(deviceDriver)
        {
            status = 
                deviceDriver->disableDevice();
        }
    }
    return(status);
}
/* ============================ ACCESSORS ================================= */

OsStatus MpInputDeviceManager::getDeviceName(int deviceId, UtlString& deviceName) const
{
    OsStatus status = OS_NOT_FOUND;

    OsWriteLock lock((OsRWMutex&)mRwMutex);

    MpAudioInputConnection* connectionFound = NULL;
    UtlInt deviceKey(deviceId);
    connectionFound =
        (MpAudioInputConnection*) mConnectionsByDeviceId.find(&deviceKey);
    MpInputDeviceDriver* deviceDriver = NULL;

    if(connectionFound)
    {
        deviceDriver = connectionFound->getDeviceDriver();
        assert(deviceDriver);
        if(deviceDriver)
        {
            status = OS_SUCCESS;
            deviceName = *deviceDriver;
        }
    }
    return(status);
}

int MpInputDeviceManager::getDeviceId(const char* deviceName) const
{
    OsStatus status = OS_NOT_FOUND;


    OsWriteLock lock((OsRWMutex&)mRwMutex);

    UtlString deviceString(deviceName);
    UtlInt* deviceId
        = (UtlInt*) mConnectionsByDeviceName.find(&deviceString);

    return(deviceId ? deviceId->getValue() : -1);
}

unsigned int MpInputDeviceManager::getCurrentFrameTime() const
{
    OsTime now;
    OsDateTime::getCurTimeSinceBoot(now);

    now -= mTimeZero;


    return(now.seconds() * 1000 + now.usecs() / 1000);
}

OsStatus MpInputDeviceManager::pushFrame(MpInputDeviceHandle deviceId,
                                         unsigned numSamples,
                                         MpAudioSample* samples,
                                         MpFrameTime frameTime)
{
    OsStatus status = OS_NOT_FOUND;

    OsWriteLock lock((OsRWMutex&)mRwMutex);

    MpAudioInputConnection* connectionFound = NULL;
    UtlInt deviceKey(deviceId);
    connectionFound =
        (MpAudioInputConnection*) mConnectionsByDeviceId.find(&deviceKey);
    MpInputDeviceDriver* deviceDriver = NULL;

    if(connectionFound)
    {
        status = 
            connectionFound->pushFrame(numSamples, samples, frameTime);
    }

    return(status);
}

OsStatus MpInputDeviceManager::getFrame(MpInputDeviceHandle deviceId,
                                        MpFrameTime frameTime,
                                        MpBufPtr& buffer,
                                        unsigned& numFramesBefore,
                                        unsigned& numFramesAfter)
{
    OsStatus status = OS_INVALID_ARGUMENT;

    OsWriteLock lock((OsRWMutex&)mRwMutex);

    MpAudioInputConnection* connectionFound = NULL;
    UtlInt deviceKey(deviceId);
    connectionFound =
        (MpAudioInputConnection*) mConnectionsByDeviceId.find(&deviceKey);
    MpInputDeviceDriver* deviceDriver = NULL;

    if(connectionFound)
    {
        status = 
            connectionFound->getFrame(frameTime,
                                      buffer,
                                      numFramesBefore,
                                      numFramesAfter);
    }

    return(status);
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

