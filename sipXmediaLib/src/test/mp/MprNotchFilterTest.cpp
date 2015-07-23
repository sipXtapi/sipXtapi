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

#include <mp/MprNotchFilter.h>
#include <mp/MprnIntMsg.h>
#include <mp/MpGenericResourceTest.h>

class MprNotchFilterTest : public MpGenericResourceTest
{
    CPPUNIT_TEST_SUITE(MprNotchFilterTest);
    CPPUNIT_TEST(testToneNotchFilter);
    CPPUNIT_TEST_SUITE_END();

    void testToneNotchFilter()
    {
        int framesPerSecond = 100; // 10 mSec frames


        unsigned int rateIndex;
        for(rateIndex = 0; rateIndex < sNumRates; rateIndex++)
        {
            printf("Test MprNotchFilter media task rate: %d samples/second\n",
                   sSampleRates[rateIndex]);

            // Incase prior test left junk around
            tearDown();

            // Set media sample rate
            setSamplesPerSec(sSampleRates[rateIndex]);
            setSamplesPerFrame(sSampleRates[rateIndex]/framesPerSecond);
            setUp();

            UtlString resourceName = "MprNotchFilter";
            MprNotchFilter* filter = new MprNotchFilter(resourceName);
            CPPUNIT_ASSERT(filter);

            // Build flowgraph with source, MprNotchFilter and sink resources
            setupFramework(filter);

            // Add the notifier so that we get resource events
            OsMsgQ resourceEventQueue;
            OsMsgDispatcher messageDispatcher(&resourceEventQueue);
            mpFlowGraph->setNotificationDispatcher(&messageDispatcher);

            // Set source to generate 2175 Hz signal
            mpSourceResource->setSignalPeriod(0, ((float)sSampleRates[rateIndex] / 2175.0));
            mpSourceResource->setSignalAmplitude(0, 1000);
            mpSourceResource->setOutSignalType(MpTestResource::MP_SINE);

            // Enable the source resource and the detector
            CPPUNIT_ASSERT(mpSourceResource->enable());
            CPPUNIT_ASSERT(filter->enable());

            // Process the frames
            int framesToProcess = 500;
            for(int frameIndex = 0; frameIndex < framesToProcess; frameIndex++)
            {
                OsStatus frameStatus = mpFlowGraph->processNextFrame();
                CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, frameStatus);

                double rms = mpSinkResource->getLastInputFrameMagnitude(0);
                UtlString message;
                message.appendFormat("Expecting zero signal frame[%d]", frameIndex);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(message, (double)0.0, rms);
            }

            // Stop flowgraph
            haltFramework();

        } // end for iteration over sample rates

    } // end testToneNotchFilter method


}; // end MprNotchFilterTest class
           

CPPUNIT_TEST_SUITE_REGISTRATION(MprNotchFilterTest);




