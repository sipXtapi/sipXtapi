//  
// Copyright (C) 2008-2015 SIPez LLC.  All rights reserved.
//
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Daniel Petrie <dpetrie AT SIPez DOT com>
#include <os/OsIntTypes.h>
#include <sipxunittests.h>
#include <sipxunit/TestUtilities.h>

#include <os/OsFileSystem.h>
#include <os/OsFileInfoBase.h>
#include <os/OsFileBase.h>
#include <mp/MprToneDetect.h>
#include <mp/MprnIntMsg.h>
#include <mp/MpGenericResourceTest.h>

class MprToneDetectTest : public MpGenericResourceTest
{
    CPPUNIT_TEST_SUITE(MprToneDetectTest);
    CPPUNIT_TEST(testToneDetectFile);
    CPPUNIT_TEST_SUITE_END();

    void testToneDetectFile()
    {
        int framesPerSecond = 100; // 10 mSec frames


        unsigned int rateIndex;
        // TODO: test sample rates other than 8000
        for(rateIndex = 0; rateIndex < 1 /*sNumRates*/; rateIndex++)
        {
            printf("Test MprToneDetect media task rate: %d samples/second\n",
                   sSampleRates[rateIndex]);

            UtlString sourceFilename;
            sourceFilename.appendFormat("mp/testTone_2175Hz_%d.raw",
                                        sSampleRates[rateIndex]);

            UtlString tempBuffer;
            long bufferBytes = OsFile::openAndRead(sourceFilename, tempBuffer);
            UtlString message;
            message.appendFormat("File: %s not found or empty", sourceFilename.data());
            CPPUNIT_ASSERT_MESSAGE(message.data(), bufferBytes > 0);

            // Incase prior test left junk around
            tearDown();

            // Set media sample rate
            setSamplesPerSec(sSampleRates[rateIndex]);
            setSamplesPerFrame(sSampleRates[rateIndex]/framesPerSecond);
            setUp();

            int framesToProcess = bufferBytes / sizeof(MpAudioSample) / (sSampleRates[rateIndex]/framesPerSecond);
            UtlString recorderResourceName = "MprToneDetect";
            MprToneDetect* detector = new MprToneDetect(recorderResourceName);
            CPPUNIT_ASSERT(detector);

            // Build flowgraph with source, MprToneDetect and sink resources
            setupFramework(detector);

            // Add the notifier so that we get resource events
            OsMsgQ resourceEventQueue;
            OsMsgDispatcher messageDispatcher(&resourceEventQueue);
            mpFlowGraph->setNotificationDispatcher(&messageDispatcher);

            // Set the source resource buffer (truncate trailing partial frame)
            mpSourceResource->setBuffer((const MpAudioSample*)tempBuffer.data(), 
                framesToProcess * sSampleRates[rateIndex] / framesPerSecond);
            mpSourceResource->setOutSignalType(MpTestResource::MP_BUFFER);

            // Enable the source resource and the detector
            CPPUNIT_ASSERT(mpSourceResource->enable());
            CPPUNIT_ASSERT(detector->enable());

            // Process the frames
            for(int frameIndex = 0; frameIndex < framesToProcess; frameIndex++)
            {
                OsStatus frameStatus = mpFlowGraph->processNextFrame();
                CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, frameStatus);

                // Expect ON at frame 67, OFF at frame 910, but allow a few frames of
                // tolerance
                switch(messageDispatcher.numMsgs())
                {
                    case 0:
                        CPPUNIT_ASSERT_MESSAGE("Should be no tone", frameIndex < 69);
                        break;

                    case 1:
                        CPPUNIT_ASSERT_MESSAGE("Should be tone present", frameIndex > 65 && frameIndex < 912);
                        break;

                    case 2:
                        CPPUNIT_ASSERT_MESSAGE("Should be no tone", frameIndex > 908);
                        break;

                    default:
                        CPPUNIT_ASSERT_MESSAGE("Only expecting 2 notifications", 0);
                        break;
                }

#ifdef TEST_PRINT
                printf("frame: %d notifications: %d\n",
                       frameIndex,
                       messageDispatcher.numMsgs());
#endif
            }

            // Get start notification
            OsTime notificationWait(0, 1000);
            OsMsg* messagePtr = NULL;
            messageDispatcher.receive(messagePtr, notificationWait);
            CPPUNIT_ASSERT(messagePtr);
            if(messagePtr)
            {
                CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgType(), OsMsg::MP_RES_NOTF_MSG);
                CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgSubType(), MpResNotificationMsg::MPRNM_TONE_DETECT_ON);
            }

            messagePtr = NULL;
            messageDispatcher.receive(messagePtr, notificationWait);
            CPPUNIT_ASSERT(messagePtr);
            if(messagePtr)
            {
                CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgType(), OsMsg::MP_RES_NOTF_MSG);
                CPPUNIT_ASSERT_EQUAL(messagePtr->getMsgSubType(), MpResNotificationMsg::MPRNM_TONE_DETECT_OFF);
            }
            // Stop flowgraph
            haltFramework();

        } // end for iteration over sample rates

    } // end testToneDetectFile method


}; // end MprToneDetectTest class
           

CPPUNIT_TEST_SUITE_REGISTRATION(MprToneDetectTest);




