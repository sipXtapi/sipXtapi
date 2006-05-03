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

#include <ptapi/PtCallEvent.h>
#include <ptapi/PtEvent.h>

/**
 * Unittest for PtCallEvent
 */
class PtCallEventTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(PtCallEventTest);
    CPPUNIT_TEST(testCreators);
    CPPUNIT_TEST(testManipulators);
    CPPUNIT_TEST_SUITE_END();


public:
    void testCreators()
    {
        PtCallEvent* pTempPtCallEvent;
        PtCallEvent* pTempPtCallEvent_1;
        PtCallEvent::PtEventId*   pTempPtEventId;

        pTempPtCallEvent = new PtCallEvent();
        pTempPtEventId = new PtEvent::PtEventId(PtEvent::PROVIDER_IN_SERVICE);
        pTempPtCallEvent->getId(*pTempPtEventId);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be invalid event", PtEvent::EVENT_INVALID, *pTempPtEventId);
        delete pTempPtEventId;
        delete pTempPtCallEvent;
                                                                                
        pTempPtCallEvent = new PtCallEvent(PtEvent::PROVIDER_IN_SERVICE);
        // mCallId is protected and no accessor method my be for better encapsulation
        // CPPUNIT_ASSERT_EQUAL_MESSAGE("callid label", 0, 
        //        strcmp(pTempPtCallEvent->mCallId, "callId"));
        delete pTempPtCallEvent;
                                                                                
        pTempPtCallEvent = new PtCallEvent(PtEvent::CALL_INVALID);
        pTempPtCallEvent_1 = new PtCallEvent(*pTempPtCallEvent);
        // mCallId is protected and no accessor method my be for better encapsulation
        // CPPUNIT_ASSERT_EQUAL_MESSAGE("callid label", 0, 
        //        strcmp(pTempPtCallEvent->mCallId, "callId"));
        delete pTempPtCallEvent;
        delete pTempPtCallEvent_1;
    }

    void testManipulators()
    {
        PtCallEvent* pTempPtCallEvent;
        PtCallEvent* pTempPtCallEvent_1;
        
        pTempPtCallEvent = new PtCallEvent(PtEvent::CALL_INVALID);
        pTempPtCallEvent_1 = new PtCallEvent(PtEvent::PROVIDER_IN_SERVICE);
        *pTempPtCallEvent = *pTempPtCallEvent_1;
        PtEvent::PtEventId check;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Getting event id", PT_SUCCESS, pTempPtCallEvent->getId(check));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Id copied", PtEvent::PROVIDER_IN_SERVICE, check);

        delete pTempPtCallEvent;
        delete pTempPtCallEvent_1;
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(PtCallEventTest);
