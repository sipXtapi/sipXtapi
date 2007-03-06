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
#include <os/OsReadLock.h>
#include <os/OsDateTime.h>
#include <os/OsSysLog.h>
#include <mp/MpInputDeviceManager.h>
#include <mp/MpInputDeviceDriver.h>
#include <mp/MpBuf.h>
#include <mp/MpAudioBuf.h>
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
                          unsigned int frameBufferLength,
                          unsigned int samplesPerFrame,
                          unsigned int samplesPerSecond,
                          MpBufPool& bufferPool)
   : UtlInt(deviceId)
   , mLastPushedFrame(frameBufferLength - 1)
   , mFrameBufferLength(frameBufferLength)
   , mppFrameBufferArray(NULL)
   , mpBufferPool(&bufferPool)
   , mpInputDeviceDriver(&deviceDriver)
   , mSamplesPerFrame(samplesPerFrame)
   , mSamplesPerSecond(samplesPerSecond)
   {
       assert(mFrameBufferLength > 0);
       assert(mSamplesPerFrame > 0);
       assert(mSamplesPerSecond > 0);
               
       mppFrameBufferArray = new MpInputDeviceFrameData[mFrameBufferLength];
   };


     /// Destructor
   virtual
   ~MpAudioInputConnection()
   {
       OsSysLog::add(FAC_MP, PRI_ERR,"~MpInputDeviceManager start dev: %p id: %d\n",
                     mpInputDeviceDriver, getValue());

       if (mppFrameBufferArray)
       {
           delete[] mppFrameBufferArray;
           mppFrameBufferArray = NULL;
       }
       OsSysLog::add(FAC_MP, PRI_ERR,"~MpInputDeviceManager end\n");
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
        int thisFrameIndex = (++mLastPushedFrame) % mFrameBufferLength;
        MpInputDeviceFrameData* thisFrameData = &mppFrameBufferArray[thisFrameIndex];

        // Current time to review device driver jitter
        OsDateTime::getCurTimeSinceBoot(thisFrameData->mFrameReceivedTime);

        // Frame time slot the driver says this is targeted for
        thisFrameData->mFrameTime = frameTime;

        // Make sure we have someplace we can stuff the data
        if (!thisFrameData->mFrameBuffer.isValid())
        {
            thisFrameData->mFrameBuffer = 
                mpBufferPool->getBuffer();
        }

        assert(thisFrameData->mFrameBuffer->getSamplesNumber() >= numSamples);

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

        return result;
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

        unsigned int lastFrame = mLastPushedFrame % mFrameBufferLength;
        numFramesBefore = 0;
        numFramesAfter = 0;
        int framePeriod = 1000 * mSamplesPerFrame / mSamplesPerSecond;

        // When requesting a frame we provide the frame that overlaps the
        // given frame time.  The frame time is for the beginning of a frame.
        // So we provide the frame that begins at or less than the requested
        // time, but not more than one frame period older.
        for (unsigned int frameIndex = 0; frameIndex < mFrameBufferLength; frameIndex++)
        {
            MpInputDeviceFrameData* frameData = 
                &mppFrameBufferArray[(lastFrame + frameIndex) % mFrameBufferLength];

            if (frameData->mFrameTime <= frameTime &&
                frameData->mFrameTime + framePeriod > frameTime)
            {
                // We have a frame of media for the requested time
                numFramesBefore = frameIndex;
                numFramesAfter = mFrameBufferLength - 1 - frameIndex;

                // We always make a copy of the frame as we are typically
                // crossing task boundaries here.
                buffer = frameData->mFrameBuffer.clone();
                result = OS_SUCCESS;
                break;
            }
        }

        return result;
    }; 

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

    MpInputDeviceDriver* getDeviceDriver() const{return(mpInputDeviceDriver);};


    unsigned getTimeDerivatives(unsigned nDerivatives, 
                                int*& derivativeBuf)
    {
        unsigned nActualDerivs = 0;
        
        int referenceFramePeriod = 1000 * mSamplesPerFrame / mSamplesPerSecond;
        unsigned int lastFrame = mLastPushedFrame % mFrameBufferLength;

        unsigned int t2FrameIdx;
        for(t2FrameIdx = 0; 
            (t2FrameIdx < mFrameBufferLength) && (nActualDerivs < nDerivatives); 
            t2FrameIdx++)
        {
            // in indexes here, higher is older, since we're subtracting before
            // modding to get the actual buffer array index.
            unsigned int t1FrameIdx = t2FrameIdx+1;

            MpInputDeviceFrameData* t2FrameData = 
                &mppFrameBufferArray[(mFrameBufferLength + lastFrame - t2FrameIdx) 
                                     % mFrameBufferLength];
            MpInputDeviceFrameData* t1FrameData = 
                &mppFrameBufferArray[(mFrameBufferLength + lastFrame - t1FrameIdx) 
                                     % mFrameBufferLength];

            // The first time we find an invalid buffer, break out of the loop.
            // This takes care of the case when only a small amount of data has
            // been pushed to the buffer.
            if(!t2FrameData->mFrameBuffer.isValid() ||
                !t1FrameData->mFrameBuffer.isValid())
            {
                break;
            }

            long t2OsMsecs = t2FrameData->mFrameReceivedTime.cvtToMsecs();
            long t1OsMsecs = t1FrameData->mFrameReceivedTime.cvtToMsecs();
            int curDeriv = (t2OsMsecs - t1OsMsecs) / referenceFramePeriod;
            derivativeBuf[t2FrameIdx] = curDeriv;
            nActualDerivs++;
        }

        return nActualDerivs;
    }

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    unsigned int mLastPushedFrame;      ///< Index of last pushed frame in mppFrameBufferArray.
    unsigned int mFrameBufferLength;    ///< Length of mppFrameBufferArray.
    MpInputDeviceFrameData* mppFrameBufferArray;
    MpInputDeviceDriver* mpInputDeviceDriver;
    unsigned int mSamplesPerFrame;      ///< Number of audio samples in one frame.
    unsigned int mSamplesPerSecond;     ///< Number of audio samples in one second.
    MpBufPool* mpBufferPool;

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
                                           unsigned defaultNumBufferedFrames,
                                           MpBufPool& bufferPool)
: mRwMutex(OsRWMutex::Q_PRIORITY)
, mLastDeviceId(0)
, mDefaultSamplesPerFrame(defaultSamplesPerFrame)
, mDefaultSamplesPerSecond(defaultSamplesPerSec)
, mDefaultNumBufferedFrames(defaultNumBufferedFrames)
, mpBufferPool(&bufferPool)
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
    // Tell the device what id to use when pushing frames to this

    newDevice.setDeviceId(newDeviceId);

    // Create a connection to contain the device and its buffered frames
    MpAudioInputConnection* connection = 
        new MpAudioInputConnection(newDeviceId,
                                   newDevice, 
                                   mDefaultNumBufferedFrames,
                                   mDefaultSamplesPerFrame,
                                   mDefaultSamplesPerSecond,
                                   *mpBufferPool);

    // Map by device name string
    UtlInt* idValue = new UtlInt(newDeviceId);
    OsSysLog::add(FAC_MP, PRI_ERR,"MpInputDeviceManager::addDevice dev: %p value: %p id: %d\n", newDevice, idValue, newDeviceId);
    mConnectionsByDeviceName.insertKeyAndValue(&newDevice, idValue);

    // Map by device ID
    mConnectionsByDeviceId.insert(connection);

    return(newDeviceId);
}


MpInputDeviceDriver* MpInputDeviceManager::removeDevice(MpInputDeviceHandle deviceId)
{
    OsWriteLock lock(mRwMutex);

    MpAudioInputConnection* connectionFound = NULL;
    UtlInt deviceKey(deviceId);
    connectionFound =
        (MpAudioInputConnection*) mConnectionsByDeviceId.find(&deviceKey);
    MpInputDeviceDriver* deviceDriver = NULL;

    if (connectionFound)
    {
        // Remove from the id indexed container
        mConnectionsByDeviceId.remove(connectionFound);

        deviceDriver = connectionFound->getDeviceDriver();
        assert(deviceDriver);

        // Get the int value mapped in the hash so we can clean up
        UtlInt* deviceIdInt =
            (UtlInt*) mConnectionsByDeviceName.findValue(deviceDriver);

        // Remove from the name indexed hash
        mConnectionsByDeviceName.remove(deviceDriver);
        if (deviceIdInt)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,"MpInputDeviceManager::addDevice dev: %p int: %p id: %d\n", deviceDriver, deviceIdInt, deviceIdInt->getValue());
            delete deviceIdInt;
            deviceIdInt = NULL;
        }

        delete connectionFound;
        connectionFound = NULL;
    }

    return(deviceDriver);
}


OsStatus MpInputDeviceManager::enableDevice(MpInputDeviceHandle deviceId)
{
    OsStatus status = OS_NOT_FOUND;
    OsWriteLock lock(mRwMutex);

    MpAudioInputConnection* connectionFound = NULL;
    UtlInt deviceKey(deviceId);
    connectionFound =
        (MpAudioInputConnection*) mConnectionsByDeviceId.find(&deviceKey);
    MpInputDeviceDriver* deviceDriver = NULL;

    if (connectionFound)
    {
        deviceDriver = connectionFound->getDeviceDriver();
        assert(deviceDriver);
        if (deviceDriver)
        {
            status = 
                deviceDriver->enableDevice(mDefaultSamplesPerFrame, 
                                           mDefaultSamplesPerSecond,
                                           getCurrentFrameTime());
        }
    }
    return(status);
}


OsStatus MpInputDeviceManager::disableDevice(MpInputDeviceHandle deviceId)
{
    OsStatus status = OS_NOT_FOUND;
    OsWriteLock lock(mRwMutex);

    MpAudioInputConnection* connectionFound = NULL;
    UtlInt deviceKey(deviceId);
    connectionFound =
        (MpAudioInputConnection*) mConnectionsByDeviceId.find(&deviceKey);
    MpInputDeviceDriver* deviceDriver = NULL;

    if (connectionFound)
    {
        deviceDriver = connectionFound->getDeviceDriver();
        assert(deviceDriver);
        if (deviceDriver)
        {
            status = 
                deviceDriver->disableDevice();
        }
    }
    return(status);
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

    if (connectionFound)
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

    if (connectionFound)
    {
        status = 
            connectionFound->getFrame(frameTime,
            buffer,
            numFramesBefore,
            numFramesAfter);
    }

    return(status);
}

/* ============================ ACCESSORS ================================= */

OsStatus MpInputDeviceManager::getDeviceName(MpInputDeviceHandle deviceId,
                                             UtlString& deviceName) const
{
    OsStatus status = OS_NOT_FOUND;

    OsReadLock lock((OsRWMutex&)mRwMutex);

    MpAudioInputConnection* connectionFound = NULL;
    UtlInt deviceKey(deviceId);
    connectionFound =
        (MpAudioInputConnection*) mConnectionsByDeviceId.find(&deviceKey);
    MpInputDeviceDriver* deviceDriver = NULL;

    if (connectionFound)
    {
        deviceDriver = connectionFound->getDeviceDriver();
        assert(deviceDriver);
        if (deviceDriver)
        {
            status = OS_SUCCESS;
            deviceName = *deviceDriver;
        }
    }
    return(status);
}


MpInputDeviceHandle MpInputDeviceManager::getDeviceId(const char* deviceName) const
{
    OsStatus status = OS_NOT_FOUND;


    OsWriteLock lock((OsRWMutex&)mRwMutex);

    UtlString deviceString(deviceName);
    UtlInt* deviceId
        = (UtlInt*) mConnectionsByDeviceName.find(&deviceString);

    return(deviceId ? deviceId->getValue() : -1);
}


MpFrameTime MpInputDeviceManager::getCurrentFrameTime() const
{
    OsTime now;
    OsDateTime::getCurTimeSinceBoot(now);

    now -= mTimeZero;


    return(now.seconds() * 1000 + now.usecs() / 1000);
}

OsStatus MpInputDeviceManager::getTimeDerivatives(MpInputDeviceHandle deviceId,
                                                  unsigned& nDerivatives,
                                                  int*& derivativeBuf) const
{
    OsStatus stat = OS_INVALID_ARGUMENT;
    unsigned nActualDerivs = 0;
    OsReadLock lock((OsRWMutex&)mRwMutex);

    MpAudioInputConnection* connectionFound = NULL;
    UtlInt deviceKey(deviceId);
    connectionFound =
        (MpAudioInputConnection*) mConnectionsByDeviceId.find(&deviceKey);

    if (connectionFound)
    {
        stat = OS_SUCCESS;
        nActualDerivs = connectionFound->getTimeDerivatives(nDerivatives, 
                                                            derivativeBuf);
    }
    nDerivatives = nActualDerivs;
    return(stat);
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

