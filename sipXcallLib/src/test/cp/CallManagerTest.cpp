//
//
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include <cp/CallManager.h>
#include <ps/PsMsg.h>
#include <ps/PsHookswTask.h>
#include <net/SipUserAgent.h>
#include <cp/CpTestSupport.h>

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
};

CPPUNIT_TEST_SUITE_REGISTRATION(CallManangerTest);
