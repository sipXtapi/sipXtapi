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
        int numBufferedFrames = 20;
        unsigned int samplesPerFrame = 80;
        unsigned int samplesPerSecond = 8000;
        unsigned int framePeriodMilliseconds = samplesPerFrame * 1000 / samplesPerSecond;

        // Create a input device manager
        MpInputDeviceManager inputDevicemanager(samplesPerFrame, 
                                                samplesPerSecond,
                                                numBufferedFrames,
                                                *mpPool);

        // Create a source input device (sine wave generator)
        // This driver should be running with no clock skew.
        unsigned int sineMagnatude = 32000;
        unsigned int sinePeriod = 4 * framePeriodMilliseconds; // 4 frames
        MpSineWaveGeneratorDeviceDriver* sineWaveDevice = 
            new MpSineWaveGeneratorDeviceDriver("sineWave",
                                                inputDevicemanager,
                                                sineMagnatude,
                                                sinePeriod,
                                                0); // no clock squew

        // Add the device
        int sineWaveDeviceId = 
            inputDevicemanager.addDevice(*sineWaveDevice);
        CPPUNIT_ASSERT(sineWaveDeviceId > 0);

        // Should fail as it is not enabled yet
        CPPUNIT_ASSERT(inputDevicemanager.disableDevice(sineWaveDeviceId) !=
                       OS_SUCCESS);

        // Enable the device
        CPPUNIT_ASSERT_EQUAL(inputDevicemanager.enableDevice(sineWaveDeviceId),
                             OS_SUCCESS);

        // Read a bunch of frames
        MpAudioBufPtr*  storedSignal = new MpAudioBufPtr[numBufferedFrames];
        OsTime start;
        OsTime end;
        OsDateTime::getCurTimeSinceBoot(start);
        int frameIndex;
        for(frameIndex = 0; frameIndex < numBufferedFrames; frameIndex++)
        {
            printf("delay: %d\n", framePeriodMilliseconds);

            OsTask::delay(framePeriodMilliseconds);
        }
        OsDateTime::getCurTimeSinceBoot(end);

        // Stop generating the sine wave ASAP
        CPPUNIT_ASSERT(inputDevicemanager.disableDevice(sineWaveDeviceId) !=
                       OS_SUCCESS);

        int lapseTime = framePeriodMilliseconds * 1000 * numBufferedFrames;
        OsTime delta = end - start;
        int actualLapseTime = delta.seconds() * 1000000 + delta.usecs();
        printf("actual time: %d scheduled: %d (milli seconds) abs: %d\n",
            actualLapseTime, lapseTime, abs(actualLapseTime - lapseTime));
        CPPUNIT_ASSERT(abs(actualLapseTime - lapseTime) < 2000 * numBufferedFrames);

        printf("finished good clock\n");
        CPPUNIT_ASSERT_EQUAL(inputDevicemanager.removeDevice(sineWaveDeviceId), 
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
