//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Keith Kyzivat <kkyzivat AT SIPez DOT com>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <mp/MpBufPool.h>
#include <mp/MpArrayBuf.h>
#include <mp/MpAudioBuf.h>
#include <mp/MpInputDeviceManager.h>
#include <mp/MpInputDeviceDriverWnt.h>
#include <os/OsTask.h>
#include <utl/UtlString.h>

#define MIDDT_SAMPLES_PER_FRAME 80
#define MIDDT_NBUFS 20

class MpInputDeviceDriverTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(MpInputDeviceDriverTest);
    CPPUNIT_TEST(testSetup);
    CPPUNIT_TEST_SUITE_END();

private:
    MpBufPool* mpBufPool;
    MpBufPool* mpHeadersPool;

    int mNumBufferedFrames;
    unsigned int mSamplesPerSecond;
    unsigned int mFramePeriodMSecs;

public:
    void setUp()
    {
        mpBufPool = 
            new MpBufPool(MIDDT_SAMPLES_PER_FRAME * sizeof(MpAudioSample)
                          + MpArrayBuf::getHeaderSize(), 
                          MIDDT_NBUFS);
        CPPUNIT_ASSERT(mpBufPool != NULL);

        // Create pool for buffer headers
        mpHeadersPool = new MpBufPool(sizeof(MpAudioBuf), MIDDT_NBUFS);
        CPPUNIT_ASSERT(mpHeadersPool != NULL);

        // Set mpHeadersPool as default pool for audio and data pools.
        MpAudioBuf::smpDefaultPool = mpHeadersPool;
        MpDataBuf::smpDefaultPool = mpHeadersPool;

        mNumBufferedFrames = 5;
        mSamplesPerSecond = 8000;
        mFramePeriodMSecs = MIDDT_SAMPLES_PER_FRAME * 1000 / mSamplesPerSecond;
    }

    UtlString getDefaultWaveInDevice()
    {
        UtlString devName("");
#ifdef WIN32
        UINT nInputDevs = waveInGetNumDevs();
        CPPUNIT_ASSERT(nInputDevs > 0);
        WAVEINCAPS devCaps;
        MMRESULT res = waveInGetDevCaps(0, &devCaps, sizeof(devCaps));
        CPPUNIT_ASSERT(res == MMSYSERR_NOERROR);
        if (res == MMSYSERR_NOERROR)
        {
            devName = UtlString(devCaps.szPname, MAXPNAMELEN);
        }
#endif
        return devName;
    }




    void testSetup()
    {
        MpInputDeviceManager inDevMgr(MIDDT_SAMPLES_PER_FRAME, 
                                      mSamplesPerSecond,
                                      mNumBufferedFrames, 
                                      *mpBufPool);

        MpInputDeviceDriver* pInDevDriver = NULL;
#ifdef WIN32
        MpInputDeviceDriverWnt iDevDriverWnt(getDefaultWaveInDevice(), inDevMgr);
        pInDevDriver = &iDevDriverWnt;
#endif
        if (pInDevDriver != NULL)
        {
#ifdef WIN32
            // Verify that we are pointing at an actual windows device.
            CPPUNIT_ASSERT(iDevDriverWnt.isDeviceValid());
#endif

            // Since we've only just created this device, it shouldn't be enabled.
            CPPUNIT_ASSERT(!pInDevDriver->isEnabled());
            // And shouldn't have a valid device handle/ID.
            CPPUNIT_ASSERT(pInDevDriver->getDeviceId() < 0);

            // Try to enable the device when it isn't added to a manager..
            // SHOULDN'T DO THIS - Only the manager should be able to do this..
            // perhaps enabling should be protected, and manager be friended?
            //CPPUNIT_ASSERT(iDrv->enableDevice(10,10,10) != OS_SUCCESS);

            // Add the device to an input manager.
            MpInputDeviceHandle iDrvHnd = inDevMgr.addDevice(*pInDevDriver);

            // Verify it has a valid handle/ID.
            CPPUNIT_ASSERT(iDrvHnd > 0);

            // Try to disable it -- this should fail, since it isn't enabled yet.
            // Also note that one should be disabling/enabling via the manager..
            // I'm just verifying that disabling the device itself when it isn't
            // set up doesn't kill things.
            CPPUNIT_ASSERT(pInDevDriver->disableDevice() != OS_SUCCESS);

            // Now enable it via the manager -- this should succeed.
            CPPUNIT_ASSERT(inDevMgr.enableDevice(iDrvHnd) == OS_SUCCESS);

            int* derivs = new int[mNumBufferedFrames-1];
            unsigned nMSecsToRecord = 200;
            int nMSPerBuffer = mNumBufferedFrames * mFramePeriodMSecs;
            unsigned i;
            for(i = 0; 
                i < nMSecsToRecord; 
                i = i+nMSPerBuffer)
            {
                // Sleep till when the input buffer should be full
                OsTask::delay(nMSPerBuffer);

                // Grab time derivative statistics..
                unsigned nDerivs = mNumBufferedFrames;
                CPPUNIT_ASSERT_EQUAL(OS_SUCCESS,
                                     inDevMgr.getTimeDerivatives(iDrvHnd, 
                                                                 nDerivs, 
                                                                 derivs));

                // Prepare the derivative line to print.
#               define NUMSTRSZ 32
                char startTStr[NUMSTRSZ];
                snprintf(startTStr, NUMSTRSZ, "%d", i);
                char endTStr[NUMSTRSZ];
                snprintf(endTStr, NUMSTRSZ, "%d", i+nMSPerBuffer);
                char tmpBuf[NUMSTRSZ];
                UtlString derivStr = 
                    UtlString("derivs(") + startTStr + ", " + endTStr + "): ";

                unsigned j;
                for(j = 0; j < nDerivs; j++)
                {
                    snprintf(tmpBuf, NUMSTRSZ, "%d", derivs[j]);
                    derivStr += tmpBuf;
                    if(j < nDerivs-1) // While there's still one more, put a comma
                        derivStr += ", ";
                }
                printf("%s\n", derivStr.data());
            }

            // Ok, now disable it via the manager -- this time it should succeed.
            CPPUNIT_ASSERT(inDevMgr.disableDevice(iDrvHnd) == OS_SUCCESS);
        }  // if pInDevDriver != NULL
    }

    void tearDown()
    {
        if (mpBufPool != NULL)
        {
            delete mpBufPool;
        }
        if (mpHeadersPool != NULL)
        {
            delete mpHeadersPool;
        }
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MpInputDeviceDriverTest);

