//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <cp/CallManager.h>
#include <ps/PsMsg.h>
#include <ps/PsHookswTask.h>
#include <net/SipUserAgent.h>
#include <cp/CpTestSupport.h>
#include <net/SipMessage.h>
#include <net/SipLineMgr.h>
#include <net/SipRefreshMgr.h>
#include <mi/CpMediaInterfaceFactoryFactory.h>

#ifdef _WIN32
  #define _CRTDBG_MAP_ALLOC
  #include <crtdbg.h>

_CrtMemState MemStateBegin;
_CrtMemState MemStateEnd;
_CrtMemState MemStateDiff;
#endif 

#define BROKEN_INITTEST

#define NUM_OF_RUNS 10
/**
 * Unittest for CallManager
 */
class CallManangerTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(CallManangerTest);

#ifdef BROKEN_UNITTEST
    CPPUNIT_TEST(testOnOffHook);
    CPPUNIT_TEST(testPickupCall);
#endif
    CPPUNIT_TEST(testSimpleTeardown);
  //CPPUNIT_TEST(testUATeardown);
  //CPPUNIT_TEST(testLineMgrUATeardown);
  //CPPUNIT_TEST(testRefreshMgrUATeardown);
    CPPUNIT_TEST(testGetNewCallId);
    CPPUNIT_TEST_SUITE_END();

public:

    void testOnOffHook()
    {
        PsMsg *keyMsg;
        SipUserAgent *ua = CpTestSupport::newSipUserAgent();
        ua->start();
        CallManager *callmgr = CpTestSupport::newCallManager(ua);
        callmgr->start();

                keyMsg = new PsMsg(PsMsg::HOOKSW_STATE, NULL, PsHookswTask::OFF_HOOK, 0);
                callmgr->postMessage(*keyMsg);
                delete keyMsg;

                keyMsg = new PsMsg(PsMsg::BUTTON_DOWN, NULL, 0, '1');
                callmgr->postMessage(*keyMsg);
        delete keyMsg;

                keyMsg = new PsMsg(PsMsg::BUTTON_UP, NULL, 0, '1');
                callmgr->postMessage(*keyMsg);
        delete keyMsg;

                keyMsg = new PsMsg(PsMsg::BUTTON_DOWN, NULL, 0, '0');
                callmgr->postMessage(*keyMsg);
                delete keyMsg;

                keyMsg = new PsMsg(PsMsg::BUTTON_UP, NULL, 0, '0');
                callmgr->postMessage(*keyMsg);
                delete keyMsg;

                keyMsg = new PsMsg(PsMsg::BUTTON_DOWN, NULL, 0, '0');
                callmgr->postMessage(*keyMsg);
                delete keyMsg;

                keyMsg = new PsMsg(PsMsg::BUTTON_UP, NULL, 0, '0');
                callmgr->postMessage(*keyMsg);
                delete keyMsg;

                keyMsg = new PsMsg(PsMsg::BUTTON_DOWN, NULL, 0, '4');
                callmgr->postMessage(*keyMsg);
                delete keyMsg;

                keyMsg = new PsMsg(PsMsg::BUTTON_UP, NULL, 0, '4');
                callmgr->postMessage(*keyMsg);
                delete keyMsg;

        delete callmgr;
        delete ua;
    }


    void testPickupCall()
    {
        PsMsg *keyMsg;
        SipUserAgent *ua = CpTestSupport::newSipUserAgent();
        ua->start();
        CallManager *callmgr = CpTestSupport::newCallManager(ua);
        callmgr->start();

                // Wait a little and pick up the hook assuming it is ringing
                OsTask::delay(30000);
                printf("Picking up ringing phone\n");

                keyMsg = new PsMsg(PsMsg::HOOKSW_STATE, NULL, PsHookswTask::OFF_HOOK, 0);
                callmgr->postMessage(*keyMsg);
                delete keyMsg;

        delete callmgr;
        delete ua;
    }

    void testSimpleTeardown()
    {
#ifdef _WIN32
        _CrtMemCheckpoint(&MemStateBegin);
#endif
        for (int i=0; i<NUM_OF_RUNS; ++i)
        {
            CallManager *pCallManager =
               new CallManager(FALSE,
                               NULL, //LineMgr
                               TRUE, // early media in 180 ringing
                               NULL, // CodecFactory
                               9000, // rtp start
                               9002, // rtp end
                               "sip:153@pingtel.com",
                               "sip:153@pingtel.com",
                               NULL, //SipUserAgent
                               0, // sipSessionReinviteTimer
                               NULL, // mgcpStackTask
                               NULL, // defaultCallExtension
                               Connection::RING, // availableBehavior
                               NULL, // unconditionalForwardUrl
                               -1, // forwardOnNoAnswerSeconds
                               NULL, // forwardOnNoAnswerUrl
                               Connection::BUSY, // busyBehavior
                               NULL, // sipForwardOnBusyUrl
                               NULL, // speedNums
                               CallManager::SIP_CALL, // phonesetOutgoingCallProtocol
                               4, // numDialPlanDigits
                               CallManager::NEAR_END_HOLD, // holdType
                               5000, // offeringDelay
                               "", // pLocal
                               CP_MAXIMUM_RINGING_EXPIRE_SECONDS, //inviteExpireSeconds
                               QOS_LAYER3_LOW_DELAY_IP_TOS, // expeditedIpTos
                               10, //maxCalls
                               sipXmediaFactoryFactory(NULL)); //pMediaFactory
#if 0
            printf("Starting CallManager\n");
#endif
            pCallManager->start();
            
            pCallManager->requestShutdown();

#if 0
            printf("Deleting CallManager\n");
#endif
            delete pCallManager;
        }
        
        for (int i=0; i<NUM_OF_RUNS; ++i)
        {
            sipxDestroyMediaFactoryFactory() ;
        }
            
#ifdef _WIN32
        _CrtMemCheckpoint(&MemStateEnd);
        if (_CrtMemDifference(&MemStateDiff, &MemStateBegin, &MemStateEnd))
        {
            _CrtMemDumpStatistics(&MemStateDiff);
        }
#endif
    }

    void testUATeardown()
    {
        for (int i=0; i<NUM_OF_RUNS; ++i)
        {
            SipUserAgent* sipUA = new SipUserAgent( 5090
                                                    ,5090
                                                    ,5091
                                                    ,NULL     // default publicAddress
                                                    ,NULL     // default defaultUser
                                                    ,"127.0.0.1" // default defaultSipAddress
                                                    ,NULL     // default sipProxyServers
                                                    ,NULL     // default sipDirectoryServers
                                                    ,NULL     // default sipRegistryServers
                                                    ,NULL     // default authenticationScheme
                                                    ,NULL     // default authenicateRealm
                                                    ,NULL     // default authenticateDb
                                                    ,NULL     // default authorizeUserIds
                                                    ,NULL     // default authorizePasswords
                                                    ,NULL     // default natPingUrl
                                                    ,0        // default natPingFrequency
                                                    ,"PING"   // natPingMethod
                                                    ,NULL //lineMgr
                                                   );

            sipUA->start();

            CallManager *pCallManager =
               new CallManager(FALSE,
                               NULL, //LineMgr
                               TRUE, // early media in 180 ringing
                               NULL, // CodecFactory
                               9000, // rtp start
                               9002, // rtp end
                               "sip:153@pingtel.com",
                               "sip:153@pingtel.com",
                               sipUA, //SipUserAgent
                               0, // sipSessionReinviteTimer
                               NULL, // mgcpStackTask
                               NULL, // defaultCallExtension
                               Connection::RING, // availableBehavior
                               NULL, // unconditionalForwardUrl
                               -1, // forwardOnNoAnswerSeconds
                               NULL, // forwardOnNoAnswerUrl
                               Connection::BUSY, // busyBehavior
                               NULL, // sipForwardOnBusyUrl
                               NULL, // speedNums
                               CallManager::SIP_CALL, // phonesetOutgoingCallProtocol
                               4, // numDialPlanDigits
                               CallManager::NEAR_END_HOLD, // holdType
                               5000, // offeringDelay
                               "", // pLocal
                               CP_MAXIMUM_RINGING_EXPIRE_SECONDS, //inviteExpireSeconds
                               QOS_LAYER3_LOW_DELAY_IP_TOS, // expeditedIpTos
                               10, //maxCalls
                               sipXmediaFactoryFactory(NULL)); //pMediaFactory
#if 0
            printf("Starting CallManager\n");
#endif
            pCallManager->start();

            sipUA->shutdown(TRUE);
            pCallManager->requestShutdown();

#if 0
            printf("Deleting CallManager\n");
#endif
            delete pCallManager;

        }
        
        for (int i=0; i<NUM_OF_RUNS; ++i)
        {
            sipxDestroyMediaFactoryFactory() ;
        }
    }

    void testLineMgrUATeardown()
    {
        for (int i=0; i<NUM_OF_RUNS; ++i)
        {
            SipLineMgr*    lineMgr = new SipLineMgr();
            lineMgr->StartLineMgr();
            SipUserAgent* sipUA = new SipUserAgent( 5090
                                                    ,5090
                                                    ,5091
                                                    ,NULL     // default publicAddress
                                                    ,NULL     // default defaultUser
                                                    ,"127.0.0.1" // default defaultSipAddress
                                                    ,NULL     // default sipProxyServers
                                                    ,NULL     // default sipDirectoryServers
                                                    ,NULL     // default sipRegistryServers
                                                    ,NULL     // default authenticationScheme
                                                    ,NULL     // default authenicateRealm
                                                    ,NULL     // default authenticateDb
                                                    ,NULL     // default authorizeUserIds
                                                    ,NULL     // default authorizePasswords
                                                    ,NULL     // default natPingUrl
                                                    ,0        // default natPingFrequency
                                                    ,"PING"   // natPingMethod
                                                    ,lineMgr
                                                   );

            sipUA->start();
            CallManager *pCallManager =
               new CallManager(FALSE,
                               NULL, //LineMgr
                               TRUE, // early media in 180 ringing
                               NULL, // CodecFactory
                               9000, // rtp start
                               9002, // rtp end
                               "sip:153@pingtel.com",
                               "sip:153@pingtel.com",
                               sipUA, //SipUserAgent
                               0, // sipSessionReinviteTimer
                               NULL, // mgcpStackTask
                               NULL, // defaultCallExtension
                               Connection::RING, // availableBehavior
                               NULL, // unconditionalForwardUrl
                               -1, // forwardOnNoAnswerSeconds
                               NULL, // forwardOnNoAnswerUrl
                               Connection::BUSY, // busyBehavior
                               NULL, // sipForwardOnBusyUrl
                               NULL, // speedNums
                               CallManager::SIP_CALL, // phonesetOutgoingCallProtocol
                               4, // numDialPlanDigits
                               CallManager::NEAR_END_HOLD, // holdType
                               5000, // offeringDelay
                               "", // pLocal
                               CP_MAXIMUM_RINGING_EXPIRE_SECONDS, //inviteExpireSeconds
                               QOS_LAYER3_LOW_DELAY_IP_TOS, // expeditedIpTos
                               10, //maxCalls
                               sipXmediaFactoryFactory(NULL)); //pMediaFactory
#if 0
            printf("Starting CallManager\n");
#endif
            pCallManager->start();

            lineMgr->requestShutdown();
            sipUA->shutdown(TRUE);
            pCallManager->requestShutdown();

#if 0
            printf("Deleting CallManager\n");
#endif

            // Delete lineMgr *after* CallManager - this seems to fix the problem
            // that SipClient->run() encounters a NULL socket. 
            delete pCallManager;
            delete lineMgr;
        }
        
        for (int i=0; i<NUM_OF_RUNS; ++i)
        {
            sipxDestroyMediaFactoryFactory() ;
        }
    }

    void testRefreshMgrUATeardown()
    {
        for (int i=0; i<NUM_OF_RUNS; ++i)
        {
            SipLineMgr*    lineMgr = new SipLineMgr();
            SipRefreshMgr* refreshMgr = new SipRefreshMgr();

            lineMgr->StartLineMgr();
            lineMgr->initializeRefreshMgr( refreshMgr );

            SipUserAgent* sipUA = new SipUserAgent( 5090
                                                    ,5090
                                                    ,5091
                                                    ,NULL     // default publicAddress
                                                    ,NULL     // default defaultUser
                                                    ,"127.0.0.1" // default defaultSipAddress
                                                    ,NULL     // default sipProxyServers
                                                    ,NULL     // default sipDirectoryServers
                                                    ,NULL     // default sipRegistryServers
                                                    ,NULL     // default authenticationScheme
                                                    ,NULL     // default authenicateRealm
                                                    ,NULL     // default authenticateDb
                                                    ,NULL     // default authorizeUserIds
                                                    ,NULL     // default authorizePasswords
                                                    ,NULL     // default natPingUrl
                                                    ,0        // default natPingFrequency
                                                    ,"PING"   // natPingMethod
                                                    ,lineMgr
                                                   );

            sipUA->start();
            refreshMgr->init(sipUA);


            CallManager *pCallManager =
               new CallManager(FALSE,
                               NULL, //LineMgr
                               TRUE, // early media in 180 ringing
                               NULL, // CodecFactory
                               9000, // rtp start
                               9002, // rtp end
                               "sip:153@pingtel.com",
                               "sip:153@pingtel.com",
                               sipUA, //SipUserAgent
                               0, // sipSessionReinviteTimer
                               NULL, // mgcpStackTask
                               NULL, // defaultCallExtension
                               Connection::RING, // availableBehavior
                               NULL, // unconditionalForwardUrl
                               -1, // forwardOnNoAnswerSeconds
                               NULL, // forwardOnNoAnswerUrl
                               Connection::BUSY, // busyBehavior
                               NULL, // sipForwardOnBusyUrl
                               NULL, // speedNums
                               CallManager::SIP_CALL, // phonesetOutgoingCallProtocol
                               4, // numDialPlanDigits
                               CallManager::NEAR_END_HOLD, // holdType
                               5000, // offeringDelay
                               "", // pLocal
                               CP_MAXIMUM_RINGING_EXPIRE_SECONDS, //inviteExpireSeconds
                               QOS_LAYER3_LOW_DELAY_IP_TOS, // expeditedIpTos
                               10, //maxCalls
                               sipXmediaFactoryFactory(NULL)); //pMediaFactory
#if 0
            printf("Starting CallManager\n");
#endif
            pCallManager->start();

            lineMgr->requestShutdown();
            refreshMgr->requestShutdown();
            sipUA->shutdown(TRUE);
            pCallManager->requestShutdown();

#if 0
            printf("Deleting CallManager\n");
#endif

            delete pCallManager;
            delete refreshMgr;
            delete lineMgr;
        }
        
        for (int i=0; i<NUM_OF_RUNS; ++i)
        {
            sipxDestroyMediaFactoryFactory() ;
        }
    }

    /* Support routine for testGetNewCallId to parse and validate a call ID.
     * The first argument is the call ID.
     * The second argument is the expected prefix.
     * The third argument receives the counter part of the call ID.
     * The fourth argument, if null, receives the suffix part of the call ID;
     *     if not null, is the expected suffix part of the call ID.
     */
    void testGetNewCallId_validate(UtlString &callId,
                                   const char* expected_prefix,
                                   UtlString* counter,
                                   UtlString* suffix)
      {
         char actual_prefix[100], actual_counter[100], actual_suffix[100];
         char msg[1000];

         // The character that separates fields in call IDs.
         // This is a #define so it is easy to change.
         // Fields in generated call IDs must never contain this character.
         #define FIELD_SEPARATOR_CHAR "_"

         int chars_consumed = -1;
         sscanf(callId.data(),
                "%[^" FIELD_SEPARATOR_CHAR "]" FIELD_SEPARATOR_CHAR
                "%[^" FIELD_SEPARATOR_CHAR "]" FIELD_SEPARATOR_CHAR
                "%s%n",
                actual_prefix, actual_counter, actual_suffix,
                &chars_consumed);
         sprintf(msg, "Cannot parse call ID '%s'", callId.data());
         CPPUNIT_ASSERT_MESSAGE(msg,
                                chars_consumed == callId.length());
         sprintf(msg, "Actual prefix '%s' does not match expected prefix '%s' in call ID '%s'",
                 actual_prefix, expected_prefix, callId.data());
         CPPUNIT_ASSERT_MESSAGE(msg,
                                strcmp(actual_prefix, expected_prefix) == 0);
         *counter = actual_counter;
         if (suffix->length() == 0)
         {
            // Set the suffix argument.
            *suffix = actual_suffix;
         }
         else
         {
            // Validate the suffix field.
            sprintf(msg, "Actual suffix '%s' does not match expected suffix '%s' in call ID '%s'",
                    actual_suffix, suffix->data(), callId.data());
            CPPUNIT_ASSERT_MESSAGE(msg,
                                   strcmp(actual_suffix, suffix->data()) == 0);
         }
      }

    /* Support routine for testGetNewCallId to write over the stack to ensure
     * that a valid pointer does not appear in getNewCallId's stack by accident.
     */
     void testGetNewCallId_hose_stack()
      {
         int buffer[1024];
         int i;
         // Access buffer through p, to confuse simple optimizers.
         int *p = &buffer[0];

         for (i = 0; i < sizeof (buffer) / sizeof (int); i++)
         {
            p[i] = i;
         }
      }

    /* Some basic tests on the CpCallManager::getNewCallId methods. */
    void testGetNewCallId()
    {
       // To hold the returned call IDs.
       UtlString callId1, callId2, callId3, callId4;
       // To hold the discovered suffix.
       UtlString suffix("");
       // To hold counter fields.
       UtlString counter;

       testGetNewCallId_hose_stack();
       CpCallManager::getNewCallId("prefix1", &callId1);
       testGetNewCallId_validate(callId1, "prefix1", &counter, &suffix);
       char msg[1000];
       sprintf(msg,
               "Actual suffix '%s' is not 16 hex characters in call ID '%s'",
               suffix.data(), callId1.data());
       CPPUNIT_ASSERT_MESSAGE(msg,
                              suffix.length() == 16 &&
                              strspn(suffix.data(), "0123456789abcdef") == 16);

       testGetNewCallId_hose_stack();
       CpCallManager::getNewCallId("prefix2", &callId2);
       testGetNewCallId_validate(callId2, "prefix2", &counter, &suffix);

       testGetNewCallId_hose_stack();
       CpCallManager::getNewCallId("prefix3", &callId3);
       testGetNewCallId_validate(callId3, "prefix3", &counter, &suffix);
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CallManangerTest);
