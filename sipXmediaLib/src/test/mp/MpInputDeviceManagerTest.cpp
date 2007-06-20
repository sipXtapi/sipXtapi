//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <os/OsTask.h>
#include <os/OsTime.h>
#include <os/OsDateTime.h>
#include <os/OsEvent.h>
#include <mp/MpAudioBuf.h>
#include <mp/MpInputDeviceManager.h>
#include <mp/MpSineWaveGeneratorDeviceDriver.h>
#ifdef RTL_ENABLED
#   include <rtl_macro.h>
    RTL_DECLARE;
#endif

#define TEST_SAMPLES_PER_FRAME_SIZE   80
#define BUFFER_NUM    50

class MpInputDeviceManagerTestReader : public OsTask
{
public:
    MpInputDeviceManagerTestReader(int numBufferedFrames,
                                   int framePeriodMilliseconds,
                                   MpAudioBufPtr* storedSignal,
                                   int deviceId,
                                   MpInputDeviceManager& inputDeviceManager,
                                   OsEvent &finishedEvent)
      : OsTask("MpInputDeviceManagerTestReader-%d", NULL, 1 /* HIGHEST PRIORITY is 0*/),
      mRunDone(FALSE),
      mNumStarvations(0),
      mFirstFrameTime(0),
      mLapseTime(0),
      mActualLapseTime(0),
      mRetryCount(0),
      mNumBufferedFrames(numBufferedFrames),
      mFramePeriodMilliseconds(framePeriodMilliseconds),
      mpStoredSignal(storedSignal),
      mDeviceId(deviceId),
      mpInputDeviceManager(&inputDeviceManager),
      mpFinishedEvent(&finishedEvent)
    {
          CPPUNIT_ASSERT(storedSignal);
    };

    ~MpInputDeviceManagerTestReader()
    {
    }

    int run(void* pArg)
    {
        MpFrameTime frameTime = mpInputDeviceManager->getCurrentFrameTime();
        OsSysLog::add(FAC_AUDIO, PRI_DEBUG, "Start frame time: %u\n", frameTime);
        int frameIndex;
        unsigned numFramesBefore;
        unsigned numFramesAfter;
        OsTime start;
        OsTime end;
        OsDateTime::getCurTimeSinceBoot(start);
        OsStatus result;
        // Give device a chance to push first
        delay(10);
        int mNumStarvations = 0;
        mFirstFrameTime;
        UtlBoolean firstFrame = TRUE;
        for(frameIndex = 0; frameIndex < mNumBufferedFrames; frameIndex++)
        {
            //OsSysLog::add(FAC_AUDIO, PRI_DEBUG, 
            //printf("delay: %d\n", mFramePeriodMilliseconds);

            { // Block scope for instrumentation timing
#ifdef RTL_ENABLED
                RTL_BLOCK("MpInputDeviceManagerTestReader.run");
#endif

                result = 
                    mpInputDeviceManager->getFrame(mDeviceId,
                                                   frameTime,
                                                   mpStoredSignal[frameIndex],
                                                   numFramesBefore,
                                                   numFramesAfter);

                // Retry once
                if(result != OS_SUCCESS)
                {
                    mRetryCount++;
                    while (result != OS_SUCCESS && numFramesAfter == 0 && numFramesBefore > 0)
                    {
                        printf(".\n");
                        frameTime += mFramePeriodMilliseconds;
                        result = 
                              mpInputDeviceManager->getFrame(mDeviceId,
                                                             frameTime,
                                                             mpStoredSignal[frameIndex],
                                                             numFramesBefore,
                                                             numFramesAfter);
                    }
                }

            }

            /*printf("got frame #%d frameTime: %u numFramesBefore: %d numFramesAfter: %d result: %d valid frame: %s\n", 
                           frameIndex,
                           frameTime,
                           numFramesBefore, 
                           numFramesAfter,
                           result,
                           mpStoredSignal[frameIndex].isValid() ? "TRUE" : "FALSE");
            */

            if(result == OS_SUCCESS && mpStoredSignal[frameIndex].isValid())
            {
                if(firstFrame == TRUE)
                {
                    printf("First frame time: %u\n", frameTime);
                    mFirstFrameTime = frameTime;
                    firstFrame = FALSE;
                }
                frameTime += mFramePeriodMilliseconds;
            }
            else
            {
                mNumStarvations++;
                //OsSysLog::add(FAC_AUDIO, PRI_DEBUG, 
                printf("BAAAAD driver: frame #%d not available\n", mDeviceId);
            }

            delay(mFramePeriodMilliseconds);
        }
        OsDateTime::getCurTimeSinceBoot(end);
        mLapseTime = mFramePeriodMilliseconds * mNumBufferedFrames;
        OsTime delta = end - start;
        mActualLapseTime = delta.seconds() * 1000 + delta.usecs() / 1000;
        printf("actual time: %dms, scheduled: %dms, abs: %dms\n",
            mActualLapseTime, mLapseTime, abs(mActualLapseTime - mLapseTime));
        mRunDone = TRUE;
        mpFinishedEvent->signal(0);
        return 1;
    }

    UtlBoolean mRunDone;
    int mNumStarvations;
    MpFrameTime mFirstFrameTime;
    int mLapseTime;
    int mActualLapseTime;
    int mRetryCount;

private:
    int mNumBufferedFrames;
    int mFramePeriodMilliseconds;
    MpAudioBufPtr* mpStoredSignal;
    int mDeviceId;
    MpInputDeviceManager* mpInputDeviceManager;
    OsEvent *mpFinishedEvent;

};

/**
 * Unittest for MpAudioBuf
 */
class MpInputDeviceManagerTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(MpInputDeviceManagerTest);
    CPPUNIT_TEST(testSineInput);
    CPPUNIT_TEST_SUITE_END();


public:

    void setUp()
    {
        // Create pool for data buffers
        mpPool = new MpBufPool(TEST_SAMPLES_PER_FRAME_SIZE * sizeof(MpAudioSample)
                             + MpArrayBuf::getHeaderSize(), BUFFER_NUM);
        CPPUNIT_ASSERT(mpPool != NULL);

        // Create pool for buffer headers
        mpHeadersPool = new MpBufPool(sizeof(MpAudioBuf), BUFFER_NUM);
        CPPUNIT_ASSERT(mpHeadersPool != NULL);

        // Set mpHeadersPool as default pool for audio and data pools.
        MpAudioBuf::smpDefaultPool = mpHeadersPool;
        MpDataBuf::smpDefaultPool = mpHeadersPool;
    }

    void tearDown()
    {
        if (mpPool != NULL)
        {
            delete mpPool;
        }
        if (mpHeadersPool != NULL)
        {
            delete mpHeadersPool;
        }
    }

    void testSineInput()
    {
#ifdef RTL_ENABLED
        RTL_START(1000000);
#endif
        int numBufferedFrames = 20;
        unsigned int samplesPerFrame = 80;
        unsigned int samplesPerSecond = 8000;
        unsigned int framePeriodMilliseconds = samplesPerFrame * 1000 / samplesPerSecond;

        // Create a input device manager
        MpInputDeviceManager inputDeviceManager(samplesPerFrame, 
                                                samplesPerSecond,
                                                numBufferedFrames,
                                                *mpPool);

        // Create a source input device (sine wave generator)
        // This driver should be running with no clock skew.
        unsigned int sineMagnatude = 32000;
        unsigned int sinePeriod = 4 * framePeriodMilliseconds; // 4 frames
        MpSineWaveGeneratorDeviceDriver* sineWaveDevice = 
            new MpSineWaveGeneratorDeviceDriver("sineWave",
                                                inputDeviceManager,
                                                sineMagnatude,
                                                sinePeriod,
                                                0); // no clock skew
        CPPUNIT_ASSERT(!sineWaveDevice->isEnabled());

        // Add the device
        int sineWaveDeviceId = 
            inputDeviceManager.addDevice(*sineWaveDevice);
        CPPUNIT_ASSERT(sineWaveDeviceId > 0);

        // Create a task to read frames from manager
        MpAudioBufPtr*  storedSignal = new MpAudioBufPtr[numBufferedFrames];
        OsEvent readerFinished;
        MpInputDeviceManagerTestReader readerTask(numBufferedFrames,
                                                  framePeriodMilliseconds,
                                                  storedSignal,
                                                  sineWaveDeviceId,
                                                  inputDeviceManager,
                                                  readerFinished);

        // Should fail as it is not enabled yet
        CPPUNIT_ASSERT(inputDeviceManager.disableDevice(sineWaveDeviceId) !=
                       OS_SUCCESS);

        // Enable the device
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             inputDeviceManager.enableDevice(sineWaveDeviceId)
                             );
        CPPUNIT_ASSERT(inputDeviceManager.isDeviceEnabled(sineWaveDeviceId));

        OsTask::delay(1); // give the device thread a chance to get started

        // Start the task to read frames
        readerTask.start();

        // wait for frames to be read with slop time
        int readerTaskWait = (int)(framePeriodMilliseconds * 1.5 * numBufferedFrames);
        printf("waiting %dms for reader to finish\n", readerTaskWait);
        // Do not block forever, wait 1sec
        OsTime maxWaitTime(0, 1000000);
        CPPUNIT_ASSERT(readerFinished.wait(maxWaitTime) == OS_SUCCESS);
        printf("done waiting for reader\n");

        CPPUNIT_ASSERT(readerTask.mRunDone);
        CPPUNIT_ASSERT_EQUAL(0, readerTask.mNumStarvations);
        CPPUNIT_ASSERT(readerTask.isShutDown());

        CPPUNIT_ASSERT(abs(readerTask.mActualLapseTime - readerTask.mLapseTime) < 4000 * numBufferedFrames);
        CPPUNIT_ASSERT_EQUAL(0, readerTask.mRetryCount);

        // Validate the actual frame data
        int frameIndex;
        char frameMessage[100];
        int tolerance = 1;
        int sampleIndex;
        MpAudioBufPtr referenceFrame = mpPool->getBuffer();
        CPPUNIT_ASSERT(referenceFrame.isValid());
        MpAudioSample* referenceSamples = referenceFrame->getSamplesWritePtr();
        MpAudioSample* actualSamples;
        MpFrameTime frameTime = readerTask.mFirstFrameTime;
        for(frameIndex = 0; frameIndex < numBufferedFrames; frameIndex++)
        {
            sprintf(frameMessage, "Validating frame: %d", frameIndex);
            CPPUNIT_ASSERT_MESSAGE(frameMessage, storedSignal[frameIndex].isValid());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(frameMessage, 
                                         samplesPerFrame,
                                         storedSignal[frameIndex]->getSamplesNumber());

            // Calculate the reference frame data
            actualSamples = storedSignal[frameIndex]->getSamplesWritePtr();
            for(sampleIndex = 0; (unsigned)sampleIndex < samplesPerFrame; sampleIndex++)
            {
                referenceSamples[sampleIndex] =
                    MpSineWaveGeneratorDeviceDriver::calculateSample(frameTime, 
                                                                     sineMagnatude,
                                                                     sinePeriod,
                                                                     sampleIndex,
                                                                     samplesPerFrame,
                                                                     samplesPerSecond);
            }

            /*printf("frame sample[%d,%d] r=%d a=%d\n",
                   frameIndex,
                   sampleIndex,
                   referenceSamples[0],
                   actualSamples[0]);*/

            CPPUNIT_ASSERT_EQUAL_MESSAGE(frameMessage,
                                         0,
                                         storedSignal[frameIndex].compareSamples(referenceFrame, tolerance));
            frameTime += framePeriodMilliseconds;
        }


#ifdef RTL_ENABLED
        RTL_WRITE("testSineInput1.rtl");
#endif
        // Stop generating the sine wave ASAP
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             inputDeviceManager.disableDevice(sineWaveDeviceId));

#ifdef RTL_ENABLED
        RTL_WRITE("testSineInput2.rtl");
        RTL_STOP;
#endif

        printf("finished good clock\n");
        CPPUNIT_ASSERT_EQUAL(inputDeviceManager.removeDevice(sineWaveDeviceId), 
                             (MpInputDeviceDriver*)sineWaveDevice);
        delete sineWaveDevice;
        sineWaveDevice = NULL;


        // TODO: Create another driver which runs too slow






        // TODO: Create another driver which runs too fast.


        delete[] storedSignal;
        storedSignal = NULL;
    }



protected:
    MpBufPool *mpPool;         ///< Pool for data buffers
    MpBufPool *mpHeadersPool;  ///< Pool for buffers headers
};

CPPUNIT_TEST_SUITE_REGISTRATION(MpInputDeviceManagerTest);
