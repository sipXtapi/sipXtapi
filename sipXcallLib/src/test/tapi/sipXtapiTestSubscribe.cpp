//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include "sipXtapiTest.h"
#include "EventValidator.h"
#include "callbacks.h"
#include "TestExternalTransport.h"

extern SIPX_INST g_hInst;
extern SIPX_INST g_hInst2;
extern SIPX_INST g_hInst3;
extern SIPX_TRANSPORT ghTransport1;
extern SIPX_TRANSPORT ghTransport2;

void sipXtapiTestSuite::testPublishAndSubscribeCallCustom() 
{
    return testPublishAndSubscribe(true, true, "testPublishAndSubscribeCallCustom");
}
void sipXtapiTestSuite::testPublishAndSubscribeConfigCustom() 
{
    return testPublishAndSubscribe(false, true, "testPublishAndSubscribeConfigCustom");
}

void sipXtapiTestSuite::testPublishAndSubscribeCall() 
{
    return testPublishAndSubscribe(true, false, "testPublishAndSubscribeCall");
}
void sipXtapiTestSuite::testPublishAndSubscribeConfig() 
{
    return testPublishAndSubscribe(false, false, "testPublishAndSubscribeConfig");
}

void sipXtapiTestSuite::testPublishAndSubscribe(bool bCallContext,
                                                bool bCustomTransport,
                                                const char* szTestName) 
{
    
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\n%s (%2d of %2d)", szTestName, iStressFactor+1, STRESS_FACTOR);

        EventValidator validatorPublish("testPublishAndSubscribe.publish") ;
        EventValidator validatorSubscribe1("testPublishAndSubscribe.subscribe1") ;
        EventValidator validatorSubscribe2("testPublishAndSubscribe.subscribe2") ;
        SIPX_LINE hLine = 0;
        SIPX_LINE hLine2 = 0;
        SIPX_LINE hLine3 = 0;
        SIPX_RESULT rc;
        SIPX_PUB hPub_coffee  = 0;
        SIPX_PUB hPub_lunch   = 0;
        SIPX_SUB hSub1_coffee = 0;
        SIPX_SUB hSub1_lunch  = 0;
        SIPX_SUB hSub2_coffee = 0;
        SIPX_SUB hSub2_lunch  = 0;
        TransportTask* pTask1 = NULL;
        TransportTask* pTask2 = NULL;
        int j;
        size_t nDummy;
        SIPX_CONTACT_ADDRESS aAddress[32];
        SIPX_CONTACT_ID contactId = CONTACT_LOCAL;
        memset(aAddress, 0, 32);

        if (bCustomTransport)
        {
            pTask1 = new TransportTask(1, "externalTransport1");
            pTask2 = new TransportTask(2, "externalTransport2");
            pTask1->start();
            pTask2->start();
            
            sipxConfigExternalTransportAdd(g_hInst, ghTransport1, true, "sub-tunnel", "127.0.0.1", -1, transportProc1, "sub-token1", "inst1");
            sipxConfigExternalTransportAdd(g_hInst2, ghTransport2, true, "sub-tunnel", "127.0.0.1", -1, transportProc2, "sub-token2", "inst2");
        }        


        sipxConfigGetLocalContacts(g_hInst, aAddress, 32, nDummy);
        for (j = 0; j < nDummy; j++)
        {
            if (strcmp(aAddress[j].cCustomTransportName, "sub-tunnel") == 0)
            {
                contactId = aAddress[j].id;
                break;
            }
        }
        
        validatorPublish.reset();
        validatorSubscribe1.reset();
        validatorSubscribe2.reset();
            
        sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorPublish) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorSubscribe1) ;
        sipxEventListenerAdd(g_hInst3, UniversalEventValidatorCallback, &validatorSubscribe2) ;

        rc = sipxLineAdd(g_hInst, "sip:foo@127.0.0.1:8000", &hLine) ;
        CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);

        rc = sipxLineAdd(g_hInst2, "sip:bar@127.0.0.1:9100", &hLine2) ;
        CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);

        rc = sipxLineAdd(g_hInst3, "sip:bar@127.0.0.1:10000", &hLine3) ;
        CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);

        validatorPublish.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL) ;        
        validatorSubscribe1.waitForLineEvent(hLine2, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL) ;        
        validatorSubscribe2.waitForLineEvent(hLine3, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL) ;        
        
        UtlString publisherUrl1("foo@127.0.0.1:8000");
        if (bCustomTransport)
        {
            //publisherUrl1.append(";transport=sub-tunnel");
        }        

        rc = sipxPublisherCreate(g_hInst, &hPub_coffee, publisherUrl1.data(), "coffee", "application/coffeeStuff", "java ready", 10);
        CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);

        rc = sipxPublisherCreate(g_hInst, &hPub_lunch, publisherUrl1.data(), "lunch", "application/lunchStuff", "order up", 8);
        CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);

        if (bCallContext)
        {
            SIPX_CALL hCall1 ;
            SIPX_LINE hCall2;
            SIPX_CALL hTemp;

            /*
            * Setup Auto-answer call back
            */    

            resetAutoAnswerCallback() ;
            rc = sipxEventListenerAdd(g_hInst, AutoAnswerCallback, NULL) ;
            assert(rc == SIPX_RESULT_SUCCESS) ;


            // setup and verify call 1

            /*
            * Create Call
            */
            rc = sipxCallCreate(g_hInst2, hLine2, &hCall1) ;
            CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;
            bool bRC = validatorSubscribe1.waitForCallEvent(hLine2, hCall1, CALLSTATE_DIALTONE, CALLSTATE_CAUSE_NORMAL, true) ;
            CPPUNIT_ASSERT(bRC) ;

            /*
            * Initiate Call
            */ 
            rc = sipxCallConnect(hCall1, publisherUrl1.data(), contactId, NULL, NULL, false) ;
            CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;

            /*
            * Validate events (listener auto-answers)
            */
            // Calling Side
            bRC = validatorSubscribe1.waitForCallEvent(hLine2, hCall1, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
            CPPUNIT_ASSERT(bRC) ;
            bRC = validatorSubscribe1.waitForCallEvent(hLine2, hCall1, CALLSTATE_REMOTE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
            CPPUNIT_ASSERT(bRC) ;
            bRC = validatorSubscribe1.waitForCallEvent(hLine2, hCall1, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
            CPPUNIT_ASSERT(bRC) ;
            bRC = validatorSubscribe1.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
            CPPUNIT_ASSERT(bRC) ;
            bRC = validatorSubscribe1.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
            CPPUNIT_ASSERT(bRC) ;

            sipxCallHold(g_hAutoAnswerCallbackCall, true);

            bRC = validatorSubscribe1.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, false);
            CPPUNIT_ASSERT(bRC) ;
            bRC = validatorSubscribe1.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, false);
            CPPUNIT_ASSERT(bRC) ;
            bRC = validatorSubscribe1.waitForCallEvent(hLine2, hCall1, CALLSTATE_REMOTE_HELD, CALLSTATE_CAUSE_NORMAL, false) ;
            CPPUNIT_ASSERT(bRC) ;

            bRC = !validatorSubscribe1.validateNoWaitingEvent() ;
            CPPUNIT_ASSERT(bRC) ;

           
            // setup and veryify Call 2
            /*
            * Create Call
            */
            OsTask::delay(2000);  // wait for autoanswercallback events to fire
            
            if (!bCustomTransport)
            {
                resetAutoAnswerCallback() ;

                rc = sipxCallCreate(g_hInst3, hLine3, &hCall2) ;
                CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;
                bRC = validatorSubscribe2.waitForCallEvent(hLine3, hCall2, CALLSTATE_DIALTONE, CALLSTATE_CAUSE_NORMAL, false) ;
                CPPUNIT_ASSERT(bRC) ;

                /*
                * Initiate Call
                */ 
                rc = sipxCallConnect(hCall2, publisherUrl1.data(), contactId, NULL, NULL, false);
                CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;

                /*
                * Validate events (listener auto-answers)
                */
                // Calling Side
                bRC = validatorSubscribe2.waitForCallEvent(hLine3, hCall2, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL, false) ;
                CPPUNIT_ASSERT(bRC) ;
                bRC = validatorSubscribe2.waitForCallEvent(hLine3, hCall2, CALLSTATE_REMOTE_ALERTING, CALLSTATE_CAUSE_NORMAL, false) ;
                CPPUNIT_ASSERT(bRC) ;
                bRC = validatorSubscribe2.waitForCallEvent(hLine3, hCall2, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, false) ;
                CPPUNIT_ASSERT(bRC) ;
                bRC = validatorSubscribe2.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, false);
                CPPUNIT_ASSERT(bRC) ;
                bRC = validatorSubscribe2.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, false);
                CPPUNIT_ASSERT(bRC) ;

                sipxCallHold(g_hAutoAnswerCallbackCall, true);
                validatorSubscribe2.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, false);
                validatorSubscribe2.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, false);
            }
            
            // ok, now we have a publisher set up, and two calls hCall1, hCall2, have called into
            // the publisher
            validatorSubscribe1.reset();
            validatorSubscribe2.reset();

            // hCall1 subscribes to the coffee publisher
            rc = sipxCallSubscribe(hCall1, "coffee", "application/coffeeStuff", &hSub1_coffee, false);
            CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
            validatorSubscribe1.waitForSubStatusEvent(SIPX_SUBSCRIPTION_ACTIVE, SUBSCRIPTION_CAUSE_NORMAL, true);

            // hCall1 receives the initial "java ready" content
            SIPX_NOTIFY_INFO matchingNotify;
            matchingNotify.hSub = hSub1_coffee;
            matchingNotify.nContentLength = 10;
            matchingNotify.szContentType = "application/coffeeStuff";
            matchingNotify.pContent = "java ready";
            validatorSubscribe1.waitForNotifyEvent(&matchingNotify, true);

            if (!bCustomTransport)
            {
                // hCall2 subscribes to the coffee publisher
                rc = sipxCallSubscribe(hCall2, "coffee", "application/coffeeStuff", &hSub2_coffee, false);
                CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
                validatorSubscribe2.waitForSubStatusEvent(SIPX_SUBSCRIPTION_ACTIVE, SUBSCRIPTION_CAUSE_NORMAL, true);

                // hCall2 receives the initial "java ready" content
                matchingNotify.hSub = hSub2_coffee;
                validatorSubscribe2.waitForNotifyEvent(&matchingNotify, true);
            }

            // hCall1 subscribes to the lunch publisher
            rc = sipxCallSubscribe(hCall1, "lunch", "application/lunchStuff", &hSub1_lunch, false);
            CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
            validatorSubscribe1.waitForSubStatusEvent(SIPX_SUBSCRIPTION_ACTIVE, SUBSCRIPTION_CAUSE_NORMAL, true);

            if (!bCustomTransport)
            {
                // hCall2 subscribes to the lunch publisher
                rc = sipxCallSubscribe(hCall2, "lunch", "application/lunchStuff", &hSub2_lunch, false);
                CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
                validatorSubscribe2.waitForSubStatusEvent(SIPX_SUBSCRIPTION_ACTIVE, SUBSCRIPTION_CAUSE_NORMAL, true);
            }
            
            // hCall1 receives the initial "order up" content
            matchingNotify.hSub = hSub1_lunch;
            matchingNotify.nContentLength = 8;
            matchingNotify.szContentType = "application/lunchStuff";
            matchingNotify.pContent = "order up";
            validatorSubscribe1.waitForNotifyEvent(&matchingNotify, true);

            if (!bCustomTransport)
            {
                // hCall2 receives the initial "order up content
                matchingNotify.hSub = hSub2_lunch;
                validatorSubscribe2.waitForNotifyEvent(&matchingNotify, true);
            }
            
            // start percolating
            char szPercolatingContent[256];
            for (int i = 0; i < 20; i++)
            {
                sprintf(szPercolatingContent, "Percolating %2.2d", i);
                sipxPublisherUpdate(hPub_coffee, "application/coffeeStuff", szPercolatingContent, 14);

                // hCall1 receives percolation content from the coffee publisher
                matchingNotify.hSub = hSub1_coffee;
                matchingNotify.nContentLength = 14;
                matchingNotify.szContentType = "application/coffeeStuff";
                matchingNotify.pContent = szPercolatingContent;
                validatorSubscribe1.waitForNotifyEvent(&matchingNotify, true);

                if (!bCustomTransport)
                {
                    // hCall2 receives percolation content from the coffee publisher
                    matchingNotify.hSub = hSub2_coffee;
                    validatorSubscribe2.waitForNotifyEvent(&matchingNotify, true);
                }
            }

            // Destroy the coffee Publisher, with "out of order" content
            rc = sipxPublisherDestroy(hPub_coffee, "application/coffeeStuff", "out of order", 12);
            CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);

            // hCall1 receives the final "out of order" content from the coffee publisher
            matchingNotify.hSub = hSub1_coffee;
            matchingNotify.nContentLength = 12;
            matchingNotify.szContentType = "application/coffeeStuff";
            matchingNotify.pContent = "out of order";
            validatorSubscribe1.waitForNotifyEvent(&matchingNotify, true);

            // hCall1 Unsubscribes from the coffee publisher
            rc = sipxCallUnsubscribe(hSub1_coffee);
            CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
            validatorSubscribe1.waitForSubStatusEvent(SIPX_SUBSCRIPTION_EXPIRED, SUBSCRIPTION_CAUSE_NORMAL, true);

            // hCall2 receives the final "out of order" content from the coffee publisher
            if (!bCustomTransport)
            {
                matchingNotify.hSub = hSub2_coffee;
                validatorSubscribe2.waitForNotifyEvent(&matchingNotify, true);
                // hCall2 Unsubscribes from the coffee publisher
                rc = sipxCallUnsubscribe(hSub2_coffee);
                CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
                validatorSubscribe2.waitForSubStatusEvent(SIPX_SUBSCRIPTION_EXPIRED, SUBSCRIPTION_CAUSE_NORMAL, true);
            }
            
            // Destroy the lunch Publisher, with "check please" content
            rc = sipxPublisherDestroy(hPub_lunch, "application/lunchStuff", "check please", 12);
            CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);

            // hCall1 receives the final "check please" content from the lunch publisher
            matchingNotify.hSub = hSub1_lunch;
            matchingNotify.nContentLength = 12;
            matchingNotify.szContentType = "application/lunchStuff";
            matchingNotify.pContent = "check please";
            validatorSubscribe1.waitForNotifyEvent(&matchingNotify, true);


            // hCall1 unsubscribes from the lunch publisher
            rc = sipxConfigUnsubscribe(hSub1_lunch);
            CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
            validatorSubscribe1.waitForSubStatusEvent(SIPX_SUBSCRIPTION_EXPIRED, SUBSCRIPTION_CAUSE_NORMAL, true);

            if (!bCustomTransport)
            {
                // hCall2 receives the final "check please" content from the lunch publisher
                matchingNotify.hSub = hSub2_lunch;
                validatorSubscribe2.waitForNotifyEvent(&matchingNotify, true);

                // hCall2 unsubscribes from the lunch publisher
                rc = sipxConfigUnsubscribe(hSub2_lunch);
                CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
                validatorSubscribe2.waitForSubStatusEvent(SIPX_SUBSCRIPTION_EXPIRED, SUBSCRIPTION_CAUSE_NORMAL, true);

                hTemp = hCall2;
                sipxCallDestroy(hCall2);
                validatorSubscribe2.waitForCallEvent(hLine3, hTemp, CALLSTATE_HELD, CALLSTATE_CAUSE_NORMAL, false);
                validatorSubscribe2.waitForCallEvent(hLine3, hTemp, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, false);
                validatorSubscribe2.waitForCallEvent(hLine3, hTemp, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, false);
            }
            
            hTemp = hCall1;
            sipxCallDestroy(hCall1);
            validatorSubscribe1.waitForCallEvent(hLine2, hTemp, CALLSTATE_HELD, CALLSTATE_CAUSE_NORMAL, false);
            validatorSubscribe1.waitForCallEvent(hLine2, hTemp, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, false);
            validatorSubscribe1.waitForCallEvent(hLine2, hTemp, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, false);

            OsTask::delay(2000);  // wait for any pending events

            rc = sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorSubscribe1) ;
            CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;
            rc = sipxEventListenerRemove(g_hInst3, UniversalEventValidatorCallback, &validatorSubscribe2) ;
            CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;
            rc = sipxEventListenerRemove(g_hInst, AutoAnswerCallback, NULL) ;
            CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;
        }
        else
        {
            // Line2 subscribes to the coffee publisher
            rc = sipxConfigSubscribe(g_hInst2, hLine2, publisherUrl1.data(), "coffee", "application/coffeeStuff", contactId, &hSub1_coffee);
            CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
            validatorSubscribe1.waitForSubStatusEvent(SIPX_SUBSCRIPTION_ACTIVE, SUBSCRIPTION_CAUSE_NORMAL, true);

            // Line2 receives the initial "java ready" content
            SIPX_NOTIFY_INFO matchingNotify;
            matchingNotify.hSub = hSub1_coffee;
            matchingNotify.nContentLength = 10;
            matchingNotify.szContentType = "application/coffeeStuff";
            matchingNotify.pContent = "java ready";
            validatorSubscribe1.waitForNotifyEvent(&matchingNotify, true);

            if (!bCustomTransport)
            {
                // Line3 subscribes to the coffee publisher
                rc = sipxConfigSubscribe(g_hInst3, hLine3, publisherUrl1.data(), "coffee", "application/coffeeStuff", contactId, &hSub2_coffee);
                CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
                validatorSubscribe2.waitForSubStatusEvent(SIPX_SUBSCRIPTION_ACTIVE, SUBSCRIPTION_CAUSE_NORMAL, true);

                // Line3 receives the initial "java ready" content
                matchingNotify.hSub = hSub2_coffee;
                validatorSubscribe2.waitForNotifyEvent(&matchingNotify, true);
            }
            
            // Line2 subscribes to the lunch publisher
            rc = sipxConfigSubscribe(g_hInst2, hLine2, publisherUrl1.data(), "lunch", "application/lunchStuff", contactId, &hSub1_lunch);
            CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
            validatorSubscribe1.waitForSubStatusEvent(SIPX_SUBSCRIPTION_ACTIVE, SUBSCRIPTION_CAUSE_NORMAL, true);

            if (!bCustomTransport)
            {
                // Line3 subscribes to the lunch publisher
                rc = sipxConfigSubscribe(g_hInst3, hLine3, publisherUrl1.data(), "lunch", "application/lunchStuff", contactId, &hSub2_lunch);
                CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
                validatorSubscribe2.waitForSubStatusEvent(SIPX_SUBSCRIPTION_ACTIVE, SUBSCRIPTION_CAUSE_NORMAL, true);
            }
            
            // Line2 receives the initial "order up" content
            matchingNotify.hSub = hSub1_lunch;
            matchingNotify.nContentLength = 8;
            matchingNotify.szContentType = "application/lunchStuff";
            matchingNotify.pContent = "order up";
            validatorSubscribe1.waitForNotifyEvent(&matchingNotify, true);

            if (!bCustomTransport)
            {
                // Line3 receives the initial "order up content
                matchingNotify.hSub = hSub2_lunch;
                validatorSubscribe2.waitForNotifyEvent(&matchingNotify, true);
            }            

            // start percolating
            char szPercolatingContent[256];
            for (int i = 0; i < 20; i++)
            {
                sprintf(szPercolatingContent, "Percolating %2.2d", i);
                sipxPublisherUpdate(hPub_coffee, "application/coffeeStuff", szPercolatingContent, 14);

                // hCall1 receives percolation content from the coffee publisher
                matchingNotify.hSub = hSub1_coffee;
                matchingNotify.nContentLength = 14;
                matchingNotify.szContentType = "application/coffeeStuff";
                matchingNotify.pContent = szPercolatingContent;
                validatorSubscribe1.waitForNotifyEvent(&matchingNotify, true);

                if (!bCustomTransport)
                {
                    // hCall2 receives percolation content from the coffee publisher
                    matchingNotify.hSub = hSub2_coffee;
                    validatorSubscribe2.waitForNotifyEvent(&matchingNotify, true);
                }
            }

            // Destroy the coffee Publisher, with "out of order" content
            rc = sipxPublisherDestroy(hPub_coffee, "application/coffeeStuff", "out of order", 12);
            CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);

            // Line2 receives the final "out of order" content from the coffee publisher
            matchingNotify.hSub = hSub1_coffee;
            matchingNotify.nContentLength = 12;
            matchingNotify.szContentType = "application/coffeeStuff";
            matchingNotify.pContent = "out of order";
            validatorSubscribe1.waitForNotifyEvent(&matchingNotify, true);

            // Line2 Unsubscribes from the coffee publisher
            rc = sipxConfigUnsubscribe(hSub1_coffee);
            CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
            validatorSubscribe1.waitForSubStatusEvent(SIPX_SUBSCRIPTION_EXPIRED, SUBSCRIPTION_CAUSE_NORMAL, true);

            if (!bCustomTransport)
            {
                // Line3 receives the final "out of order" content from the coffee publisher
                matchingNotify.hSub = hSub2_coffee;
                validatorSubscribe2.waitForNotifyEvent(&matchingNotify, true);
                // Line3 Unsubscribes from the coffee publisher
                rc = sipxConfigUnsubscribe(hSub2_coffee);
                CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
                validatorSubscribe2.waitForSubStatusEvent(SIPX_SUBSCRIPTION_EXPIRED, SUBSCRIPTION_CAUSE_NORMAL, true);
            }
            
            // Destroy the lunch Publisher, with "check please" content
            rc = sipxPublisherDestroy(hPub_lunch, "application/lunchStuff", "check please", 12);
            CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);


            // Line2 receives the final "check please" content from the lunch publisher
            matchingNotify.hSub = hSub1_lunch;
            matchingNotify.nContentLength = 12;
            matchingNotify.szContentType = "application/lunchStuff";
            matchingNotify.pContent = "check please";
            validatorSubscribe1.waitForNotifyEvent(&matchingNotify, true);


            // Line2 unsubscribes from the lunch publisher
            rc = sipxConfigUnsubscribe(hSub1_lunch);
            CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
            validatorSubscribe1.waitForSubStatusEvent(SIPX_SUBSCRIPTION_EXPIRED, SUBSCRIPTION_CAUSE_NORMAL, true);

            if (!bCustomTransport)
            {
                // LiNE3 receives the final "check please" content from the lunch publisher
                matchingNotify.hSub = hSub2_lunch;
                validatorSubscribe2.waitForNotifyEvent(&matchingNotify, true);

                // Line3 unsubscribes from the lunch publisher
                rc = sipxConfigUnsubscribe(hSub2_lunch);
                CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
                validatorSubscribe2.waitForSubStatusEvent(SIPX_SUBSCRIPTION_EXPIRED, SUBSCRIPTION_CAUSE_NORMAL, true);
            }
        }
        rc = sipxLineRemove(hLine);
        CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
        
        rc = sipxLineRemove(hLine2);
        CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);

        rc = sipxLineRemove(hLine3);
        CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
        
        if (bCustomTransport)
        {
            sipxConfigExternalTransportRemove(ghTransport1);
            sipxConfigExternalTransportRemove(ghTransport2);
            delete pTask1;
            delete pTask2;
        }

        sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorPublish) ;
        sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorSubscribe1) ;
        sipxEventListenerRemove(g_hInst3, UniversalEventValidatorCallback, &validatorSubscribe2) ;

    }

    OsTask::delay(TEST_DELAY) ;    
    checkForLeaks() ;
}

