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
#include <mp/MpAudioBuf.h>
#include <mp/MpInputDeviceManager.h>
#include <mp/MpSineWaveGeneratorDeviceDriver.h>

#define TEST_SAMPLES_PER_FRAME_SIZE   80
#define BUFFER_NUM    50

class MpInputDeviceManagerTestReader : public OsTask
{
public:
    MpInputDeviceManagerTestReader(int numBufferedFrames,
                                   int framePeriodMilliseconds,
                                   MpAudioBufPtr* storedSignal,
                                   int deviceId,
                                   MpInputDeviceManager& inputDeviceManager) :
      OsTask("MpInputDeviceManagerTestReader-%d", NULL, 1 /* HIGHEST PRIORITY is 0*/),
      mNumBufferedFrames(numBufferedFrames),
      mFramePeriodMilliseconds(framePeriodMilliseconds),
      mpStoredSignal(storedSignal),
      mDeviceId(deviceId),
      mpInputDeviceManager(&inputDeviceManager),
      mRunDone(FALSE)
    {
          CPPUNIT_ASSERT(storedSignal);
    };

    ~MpInputDeviceManagerTestReader()
    {
    }

    int run(void* pArg)
    {
        MpFrameTime frameTime = mpInputDeviceManager->getCurrentFrameTime();
        OsSysLog::add(FAC_AUDIO, PRI_ERR, "Start frame time: %u\n", frameTime);
        int frameIndex;
        unsigned numFramesBefore;
        unsigned numFramesAfter;
        OsTime start;
        OsTime end;
        OsDateTime::getCurTimeSinceBoot(start);
        OsStatus result;
        // Give device a chance to push first
        delay(10);
        for(frameIndex = 0; frameIndex < mNumBufferedFrames; frameIndex++)
        {
            OsSysLog::add(FAC_AUDIO, PRI_ERR, "delay: %d\n", mFramePeriodMilliseconds);


            /*CPPUNIT_ASSERT_EQUAL(*/
            result = 
                mpInputDeviceManager->getFrame(mDeviceId,
                                            frameTime,
                                            mpStoredSignal[frameIndex],
                                            numFramesBefore,
                                            numFramesAfter),
                //OS_SUCCESS);

            OsSysLog::add(FAC_AUDIO, PRI_ERR, "got frameTime: %u numFramesBefore: %d numFramesAfter: %d result: %d\n", 
                           frameTime,
                           numFramesBefore, 
                           numFramesAfter,
                           result);
            
            delay(mFramePeriodMilliseconds);

            if(result == OS_SUCCESS)
            {
                frameTime += mFramePeriodMilliseconds;
            }
            else
            {
                printf("BAAAAD driver: %d frame not available\n", mDeviceId);
            }
        }
        OsDateTime::getCurTimeSinceBoot(end);
        int lapseTime = mFramePeriodMilliseconds * 1000 * mNumBufferedFrames;
        OsTime delta = end - start;
        int actualLapseTime = delta.seconds() * 1000000 + delta.usecs();
        printf("actual time: %d scheduled: %d (milli seconds) abs: %d\n",
            actualLapseTime, lapseTime, abs(actualLapseTime - lapseTime));
        //CPPUNIT_ASSERT(abs(actualLapseTime - lapseTime) < 2000 * mNumBufferedFrames);
        mRunDone = TRUE;
        return(1);
    }

    UtlBoolean mRunDone;

private:
    int mNumBufferedFrames;
    int mFramePeriodMilliseconds;
    MpAudioBufPtr* mpStoredSignal;
    int mDeviceId;
    MpInputDeviceManager* mpInputDeviceManager;

};

/**
 * Unittest for MpAudioBuf
 */
class MpInputDeviceManagerTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(MpInputDeviceManagerTest);
    //CPPUNIT_TEST(testSineInput);
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
                                                0); // no clock squew
        CPPUNIT_ASSERT(!sineWaveDevice->isEnabled());

        // Add the device
        int sineWaveDeviceId = 
            inputDeviceManager.addDevice(*sineWaveDevice);
        CPPUNIT_ASSERT(sineWaveDeviceId > 0);

        // Create a task to read frames from manager
        MpAudioBufPtr*  storedSignal = new MpAudioBufPtr[numBufferedFrames];
        MpInputDeviceManagerTestReader readerTask(numBufferedFrames,
                                                  framePeriodMilliseconds,
                                                  storedSignal,
                                                  sineWaveDeviceId,
                                                  inputDeviceManager);

        // Should fail as it is not enabled yet
        CPPUNIT_ASSERT(inputDeviceManager.disableDevice(sineWaveDeviceId) !=
                       OS_SUCCESS);

        // Enable the device
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                             inputDeviceManager.enableDevice(sineWaveDeviceId),
                             );
        CPPUNIT_ASSERT(inputDeviceManager.isDeviceEnabled(sineWaveDeviceId));

        OsTask::delay(1); // give the device thread a chance to get started

        // Start the task to read frames
        readerTask.start();

        // wait for frames to be read with slop time
        int readerTaskWait = framePeriodMilliseconds * 1.5 * numBufferedFrames;
        printf("waiting: %d for reader to finish\n", readerTaskWait);
        OsTask::delay(readerTaskWait);
        printf("done waiting for reader\n");

        CPPUNIT_ASSERT(readerTask.mRunDone);
        CPPUNIT_ASSERT(readerTask.isShutDown());

        // Stop generating the sine wave ASAP
        CPPUNIT_ASSERT(inputDeviceManager.disableDevice(sineWaveDeviceId) !=
                       OS_SUCCESS);


        printf("finished good clock\n");
        CPPUNIT_ASSERT_EQUAL(inputDeviceManager.removeDevice(sineWaveDeviceId), 
                             (MpInputDeviceDriver*)sineWaveDevice);
        delete sineWaveDevice;
        sineWaveDevice = NULL;


        // Create another driver which runs too slow






        // Create another driver which runs too fast.


        delete[] storedSignal;
        storedSignal = NULL;
    }



protected:
    MpBufPool *mpPool;         ///< Pool for data buffers
    MpBufPool *mpHeadersPool;  ///< Pool for buffers headers
};

CPPUNIT_TEST_SUITE_REGISTRATION(MpInputDeviceManagerTest);
