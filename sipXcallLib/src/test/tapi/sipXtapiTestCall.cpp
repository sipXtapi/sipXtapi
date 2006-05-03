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
#include "os/OsFS.h"


extern SIPX_INST g_hInst;
extern SIPX_INST g_hInst2;
extern SIPX_INST g_hInst3;
extern SIPX_INST g_hInst5;

extern bool g_bCallbackCalled;

extern SIPX_INST g_hInstInfo;
extern SIPX_CALL ghCallHangup;

void sipXtapiTestSuite::testCallMakeAPI()
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestCallMakeAPI (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        SIPX_LINE hLine = SIPX_LINE_NULL;
        SIPX_CALL hCall = SIPX_CALL_NULL ;        
        SIPX_RESULT rc ; 
        
        rc = sipxLineAdd(g_hInst, "sip:bandreasen@pingtel.com", &hLine) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(hLine) ;

        rc = sipxCallCreate(g_hInst, hLine, &hCall) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(hCall != SIPX_CALL_NULL) ;

        rc = sipxCallDestroy(hCall) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(hCall == SIPX_CALL_NULL) ;

        rc = sipxLineRemove(hLine) ; 
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        OsTask::delay(TEST_DELAY) ;
    }

    OsTask::delay(TEST_DELAY) ;
    checkForLeaks();
}


void sipXtapiTestSuite::testCallGetID() 
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        SIPX_LINE hLine = SIPX_LINE_NULL ;
        SIPX_CALL hCall = SIPX_CALL_NULL ;

        printf("\ntestCallGetID (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        createCall(&hLine, &hCall) ;

        char cBuf[128] ;

        memset(cBuf, 0, sizeof(cBuf)) ;
        CPPUNIT_ASSERT_EQUAL(sipxCallGetID(hCall, cBuf, sizeof(cBuf)), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(strlen(cBuf) > 0) ;

        memset(cBuf, ' ', sizeof(cBuf)) ;
        CPPUNIT_ASSERT_EQUAL(sipxCallGetID(hCall, cBuf, 4), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(strlen(cBuf) == 3) ;
        CPPUNIT_ASSERT(cBuf[5] == ' ') ;

        destroyCall(hLine, hCall) ;
    }
    OsTask::delay(TEST_DELAY) ;    

    checkForLeaks();
}

void sipXtapiTestSuite::testCallRapidCallAndHangup()
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        SIPX_LINE hLine = SIPX_LINE_NULL ;
        SIPX_CALL hCall = SIPX_CALL_NULL ;

        printf("\ntestCallRapidCallAndHangup (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);


        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, "sip:bandreasen@pingtel.com", &hLine), SIPX_RESULT_SUCCESS) ;
        
         CPPUNIT_ASSERT_EQUAL(sipxCallCreate(g_hInst, hLine, &hCall), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxCallConnect(hCall, "sip:mike2@cheetah.pingtel.com"), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxCallDestroy(hCall), SIPX_RESULT_SUCCESS);
        
         CPPUNIT_ASSERT_EQUAL(sipxCallCreate(g_hInst, hLine, &hCall), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxCallConnect(hCall, "sip:mike2@cheetah.pingtel.com"), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxCallDestroy(hCall), SIPX_RESULT_SUCCESS);
        
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS) ;
        OsTask::delay(CALL_DELAY*2);
    }
    OsTask::delay(TEST_DELAY) ;    
}


void sipXtapiTestSuite::testCallGetRemoteID()
{
    bool bRC ;
    EventValidator validatorCalling("testCallGetRemoteID.calling") ;
    EventValidator validatorCalled("testCallGetRemoteID.called") ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestCallGetRemoteID (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);
        SIPX_CALL hCall ;
        SIPX_LINE hLine ;
        SIPX_LINE hReceivingLine;     

        validatorCalling.reset() ;
        validatorCalled.reset() ;

        // Setup Auto-answer call back
        resetAutoAnswerCallback() ;
        sipxEventListenerAdd(g_hInst2, AutoAnswerCallback, NULL) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled) ;        
        sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalling) ;

        sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine, CONTACT_LOCAL);
        bRC = validatorCalled.waitForLineEvent(hReceivingLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        createCall(&hLine, &hCall) ;
        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxCallConnect(hCall, "sip:foo@127.0.0.1:9100") ;
        

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DIALTONE, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // Test sipxCallGetRemoteID
        char cBuf[128] ;
        memset(cBuf, 0, sizeof(cBuf));
        CPPUNIT_ASSERT_EQUAL( sipxCallGetRemoteID(hCall, cBuf, sizeof(cBuf)), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(strlen(cBuf) > 0) ;
        
        memset(cBuf, ' ', sizeof(cBuf)) ;
        CPPUNIT_ASSERT_EQUAL(sipxCallGetRemoteID(hCall, cBuf, 4), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(strlen(cBuf) == 3) ;
        CPPUNIT_ASSERT(cBuf[5] == ' ') ;                        

        SIPX_CALL hDestroyedCall = hCall ;
        destroyCall(hCall) ;

        // Validate Calling Side
        
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_BRIDGED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true) ;
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoAnswerCallback, NULL), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled), SIPX_RESULT_SUCCESS) ;
    
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine), SIPX_RESULT_SUCCESS);
    }

    OsTask::delay(TEST_DELAY) ;

    checkForLeaks();
}


void sipXtapiTestSuite::testCallGetLocalID()
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        SIPX_LINE hLine = SIPX_LINE_NULL ;
        SIPX_CALL hCall = SIPX_CALL_NULL ;

        printf("\ntestCallGetLocalID (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        createCall(&hLine, &hCall) ;

        char cBuf[128] ;

        memset(cBuf, 0, sizeof(cBuf)) ;
        CPPUNIT_ASSERT_EQUAL(sipxCallGetLocalID(hCall, cBuf, sizeof(cBuf)), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(strlen(cBuf) > 0) ;

        memset(cBuf, ' ', sizeof(cBuf)) ;
        CPPUNIT_ASSERT_EQUAL(sipxCallGetLocalID(hCall, cBuf, 4), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(strlen(cBuf) == 3) ;
        CPPUNIT_ASSERT(cBuf[5] == ' ') ;

        destroyCall(hLine, hCall) ;
    }
    OsTask::delay(TEST_DELAY) ;

    checkForLeaks();
}


void sipXtapiTestSuite::createCall(SIPX_LINE* phLine, SIPX_CALL* phCall)
{
    CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, "sip:bandreasen@pingtel.com", phLine), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT(*phLine) ;

    createCall(*phLine, phCall) ;
}

void sipXtapiTestSuite::createCall(SIPX_LINE hLine, SIPX_CALL* phCall) 
{
    CPPUNIT_ASSERT_EQUAL(sipxCallCreate(g_hInst, hLine, phCall), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT(*phCall) ;
}


void sipXtapiTestSuite::destroyCall(SIPX_LINE& hLine, SIPX_CALL& hCall) 
{
    destroyCall(hCall) ;
    OsTask::delay(250) ;
    CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS) ;
}

void sipXtapiTestSuite::destroyCall(SIPX_CALL& hCall) 
{
    CPPUNIT_ASSERT_EQUAL(sipxCallDestroy(hCall), SIPX_RESULT_SUCCESS) ;    
    CPPUNIT_ASSERT(hCall == SIPX_CALL_NULL) ;
}

void sipXtapiTestSuite::doCallBasic(SIPX_INST              hCallingInst,
                                    const char*            szCallingParty,
                                    const char*            szCallingLine,
                                    SIPX_INST              hCalledInst,
                                    const char*            szCalledParty,
                                    const char*            szCalledLine,
                                    ADDITIONALCALLTESTPROC pAdditionalProc)
{
    bool bRC ;
    EventValidator validatorCalling("doCallBasic.calling") ;
    EventValidator validatorCalled("doCallBasic.called") ;

    SIPX_RESULT rc ;
    SIPX_CALL hCall ;
    SIPX_LINE hCallingLine ;
    SIPX_LINE hCalledLine ;

    validatorCalling.reset() ;
    validatorCalled.reset() ;

    /*
     * Setup Auto-answer call back
     */    
    rc = sipxEventListenerAdd(hCallingInst, UniversalEventValidatorCallback, &validatorCalling) ;
    CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;

    resetAutoAnswerCallback() ;
    rc = sipxEventListenerAdd(hCalledInst, AutoAnswerCallback, NULL) ;
    assert(rc == SIPX_RESULT_SUCCESS) ;
    rc = sipxEventListenerAdd(hCalledInst, UniversalEventValidatorCallback, &validatorCalled) ;        
    CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;

    /*
     * Setup Lines
     */
    rc = sipxLineAdd(hCallingInst, szCallingLine, &hCallingLine, CONTACT_LOCAL) ;
    CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;
    bRC = validatorCalling.waitForLineEvent(hCallingLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;

    rc = sipxLineAdd(hCalledInst, szCalledLine, &hCalledLine, CONTACT_LOCAL);
    CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;
    bRC = validatorCalled.waitForLineEvent(hCalledLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;

    /*
     * Create Call
     */
    rc = sipxCallCreate(hCallingInst, hCallingLine, &hCall) ;
    CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;
    bRC = validatorCalling.waitForCallEvent(hCallingLine, hCall, CALLSTATE_DIALTONE, CALLSTATE_CAUSE_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;

    /*
     * Initiate Call
     */ 
    rc = sipxCallConnect(hCall, szCalledParty) ;
    CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;

    /*
     * Validate events (listener auto-answers)
     */
    // Calling Side
    bRC = validatorCalling.waitForCallEvent(hCallingLine, hCall, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalling.waitForCallEvent(hCallingLine, hCall, CALLSTATE_REMOTE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalling.waitForCallEvent(hCallingLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
    CPPUNIT_ASSERT(bRC) ;
    // Called Side
    bRC = validatorCalled.waitForCallEvent(hCalledLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalled.waitForCallEvent(hCalledLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalled.waitForCallEvent(hCalledLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalled.waitForCallEvent(hCalledLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
    CPPUNIT_ASSERT(bRC) ;
    /*
     * Some other random sanity tests
     */
    int connectionId = -1;
        
    rc = sipxCallGetConnectionId(hCall, connectionId) ;
    CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT(connectionId != -1) ;


    // User supplied santity checks
    if (pAdditionalProc)
    {
        (*pAdditionalProc)(hCall, hCallingLine, &validatorCalling, 
                g_hAutoAnswerCallbackCall, hCalledLine, &validatorCalled) ;
    }

    /*
     * Drop Call (listener side will auto-drop)
     */ 
    SIPX_CALL hDestroyedCall = hCall ;
    rc = sipxCallDestroy(hCall) ;
    CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;

    /*
     * Validate post-drop calls
     */                

    // Validate Calling Side
    bRC = validatorCalling.waitForCallEvent(hCallingLine, hDestroyedCall, CALLSTATE_BRIDGED, CALLSTATE_CAUSE_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalling.waitForCallEvent(hCallingLine, hDestroyedCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalling.waitForCallEvent(hCallingLine, hDestroyedCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;

    // Validate Called Side
    bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalled.waitForCallEvent(hCalledLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalled.waitForCallEvent(hCalledLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;

    /*
     * Cleanup
     */        

    rc = sipxLineRemove(hCalledLine) ;
    CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;
    rc = sipxLineRemove(hCallingLine) ;
    CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;

    rc = sipxEventListenerRemove(hCallingInst, UniversalEventValidatorCallback, &validatorCalling) ;
    CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;
    rc = sipxEventListenerRemove(hCalledInst, AutoAnswerCallback, NULL) ;
    CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;
    rc = sipxEventListenerRemove(hCalledInst, UniversalEventValidatorCallback, &validatorCalled) ;
    CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;   
}


void sipXtapiTestSuite::doCallBasicSetup(SIPX_INST   hCallingInst,
                                         const char* szCallingParty,
                                         const char* szCallingLine,
                                         SIPX_INST   hCalledInst,
                                         const char* szCalledParty,
                                         const char* szCalledLine,
                                         SIPX_CALL&  hCallingCall,
                                         SIPX_LINE&  hCallingLine,
                                         SIPX_CALL&  hCalledCall,
                                         SIPX_LINE&  hCalledLine) 
{
    bool bRC ;
    EventValidator validatorCalling("doCallBasic.calling") ;
    EventValidator validatorCalled("doCallBasic.called") ;

    SIPX_RESULT rc ;

    validatorCalling.reset() ;
    validatorCalled.reset() ;

    /*
     * Setup Auto-answer call back
     */    
    rc = sipxEventListenerAdd(hCallingInst, UniversalEventValidatorCallback, &validatorCalling) ;
    CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;

    resetAutoAnswerCallback() ;
    rc = sipxEventListenerAdd(hCalledInst, AutoAnswerCallback, NULL) ;
    assert(rc == SIPX_RESULT_SUCCESS) ;
    rc = sipxEventListenerAdd(hCalledInst, UniversalEventValidatorCallback, &validatorCalled) ;
    CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;

    /*
     * Setup Lines
     */
    rc = sipxLineAdd(hCallingInst, szCallingLine, &hCallingLine, CONTACT_LOCAL) ;
    CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;
    bRC = validatorCalling.waitForLineEvent(hCallingLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;

    rc = sipxLineAdd(hCalledInst, szCalledLine, &hCalledLine, CONTACT_LOCAL);
    CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;
    bRC = validatorCalled.waitForLineEvent(hCalledLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;

    /*
     * Create Call
     */
    rc = sipxCallCreate(hCallingInst, hCallingLine, &hCallingCall) ;
    CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;
    bRC = validatorCalling.waitForCallEvent(hCallingLine, hCallingCall, CALLSTATE_DIALTONE, CALLSTATE_CAUSE_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;

    /*
     * Initiate Call
     */ 
    rc = sipxCallConnect(hCallingCall, szCalledParty) ;
    CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;

    /*
     * Validate events (listener auto-answers)
     */

    // Calling Side
    bRC = validatorCalling.waitForCallEvent(hCallingLine, hCallingCall, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalling.waitForCallEvent(hCallingLine, hCallingCall, CALLSTATE_REMOTE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalling.waitForCallEvent(hCallingLine, hCallingCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
    CPPUNIT_ASSERT(bRC) ;

    // Called Side
    bRC = validatorCalled.waitForCallEvent(hCalledLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalled.waitForCallEvent(hCalledLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalled.waitForCallEvent(hCalledLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalled.waitForCallEvent(hCalledLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
    CPPUNIT_ASSERT(bRC) ;

    hCalledCall = g_hAutoAnswerCallbackCall ;
    

    /*
     * Cleanup
     */        

    rc = sipxEventListenerRemove(hCallingInst, UniversalEventValidatorCallback, &validatorCalling) ;
    CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;
    rc = sipxEventListenerRemove(hCalledInst, AutoAnswerCallback, NULL) ;
    CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;
    rc = sipxEventListenerRemove(hCalledInst, UniversalEventValidatorCallback, &validatorCalled) ;
    CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;   
}


// A calls B, B answers, A hangs up
void sipXtapiTestSuite::testCallBasic() 
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestCallBasic (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        doCallBasic(g_hInst, "sip:calling@127.0.0.1:8000", "sip:calling@127.0.0.1:8000",
                    g_hInst2, "sip:called@127.0.0.1:9100", "sip:called@127.0.0.1:9100", 
                    NULL) ;
    }

    OsTask::delay(TEST_DELAY) ;
    checkForLeaks();
}


// A calls B, B answers, A hangs up
void sipXtapiTestSuite::testCallBasicTCP() 
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestCallBasicTCP (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        doCallBasic(g_hInst, "sip:calling@127.0.0.1:8000", "sip:calling@127.0.0.1:8000",
                    g_hInst2, "<sip:called@127.0.0.1:9100;transport=tcp>", "sip:called@127.0.0.1:9100", 
                    NULL) ;
    }

    OsTask::delay(TEST_DELAY) ;
    checkForLeaks();
}

// A calls B, B answers, A hangs up
void sipXtapiTestSuite::testCallDestroyRinging() 
{
    bool bRC ;
    EventValidator validatorCalling("testCallDestroyRinging.calling") ;
    EventValidator validatorCalled("testCallDestroyRinging.called") ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestCallDestroyRinging (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);
        SIPX_CALL hCall ;
        SIPX_LINE hLine ;
        SIPX_LINE hReceivingLine;     

        validatorCalling.reset() ;
        validatorCalled.reset() ;

        // Setup Auto-answer call back
        resetAutoAnswerCallback() ;
        sipxEventListenerAdd(g_hInst2, AutoAnswerHangupRingingCallback, NULL) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled) ;        
        sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalling) ;

        sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine, CONTACT_AUTO);
        bRC = validatorCalled.waitForLineEvent(hReceivingLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        createCall(&hLine, &hCall) ;
        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxCallConnect(hCall, "sip:foo@127.0.0.1:9100") ;
        

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DIALTONE, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        int connectionId = -1;
        
	    CPPUNIT_ASSERT_EQUAL(sipxCallGetConnectionId(hCall, connectionId), SIPX_RESULT_SUCCESS);
	    CPPUNIT_ASSERT(connectionId != -1) ;
	            
        SIPX_CALL hDestroyedCall = hCall ;
        destroyCall(hCall);

        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_UNKNOWN, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoAnswerHangupRingingCallback, NULL), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled), SIPX_RESULT_SUCCESS) ;
    
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine), SIPX_RESULT_SUCCESS);
    }

    OsTask::delay(TEST_DELAY) ;

    checkForLeaks();
}

// A calls B, B answers, A mutes and unmutes repeatedly, then hangs up
void sipXtapiTestSuite::testCallMute() 
{
    bool bRC ;
    EventValidator validatorCalling("testCallMute.calling") ;
    EventValidator validatorCalled("testCallMute.called") ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestCallMute (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);
        SIPX_CALL hCall ;
        SIPX_LINE hLine ;
        SIPX_LINE hReceivingLine;     

        validatorCalling.reset() ;
        validatorCalled.reset() ;

        // Setup Auto-answer call back
        resetAutoAnswerCallback() ;
        sipxEventListenerAdd(g_hInst2, AutoAnswerCallback, NULL) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled) ;        
        sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalling) ;

        sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine, CONTACT_LOCAL);
        bRC = validatorCalled.waitForLineEvent(hReceivingLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        createCall(&hLine, &hCall) ;
        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxCallConnect(hCall, "sip:foo@127.0.0.1:9100") ;
        

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DIALTONE, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
                
        int connectionId = -1;
        
        CPPUNIT_ASSERT_EQUAL(sipxCallGetConnectionId(hCall, connectionId), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT(connectionId != -1) ;
        
        // loop 10 times, muting and unmuting
        for (int i = 0; i < 50; i++)
        {
            sipxAudioMute(g_hInst, true);
            sipxAudioMute(g_hInst, false);
        }
                
        SIPX_CALL hDestroyedCall = hCall ;
        destroyCall(hCall) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_BRIDGED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoAnswerCallback, NULL), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled), SIPX_RESULT_SUCCESS) ;
    
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine), SIPX_RESULT_SUCCESS);
    }

    OsTask::delay(TEST_DELAY) ;

    checkForLeaks();
}


// A calls B, B answers, B hangs up
void sipXtapiTestSuite::testCallBasic2() 
{
    bool bRC ;
    EventValidator validatorCalling("testCallBasic2.calling") ;
    EventValidator validatorCalled("testCallBasic2.called") ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestCallBasic2 (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);
        SIPX_CALL hCall ;
        SIPX_LINE hLine ;
        SIPX_LINE hReceivingLine;

        validatorCalling.reset() ;
        validatorCalled.reset() ;

        // Setup Auto-answer call back
        sipxEventListenerAdd(g_hInst2, AutoAnswerHangupCallback, NULL) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled) ;        
        sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalling) ;

        sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine, CONTACT_LOCAL);        

        bRC = validatorCalled.waitForLineEvent(hReceivingLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        createCall(&hLine, &hCall) ;

        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxCallConnect(hCall, "sip:foo@127.0.0.1:9100") ;
        
        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DIALTONE, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerHangupCallbackLine, g_hAutoAnswerHangupCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerHangupCallbackLine, g_hAutoAnswerHangupCallbackCall, CALLSTATE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerHangupCallbackLine, g_hAutoAnswerHangupCallbackCall, CALLSTATE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerHangupCallbackLine, g_hAutoAnswerHangupCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // Called side with automatically hang up
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerHangupCallbackLine, g_hAutoAnswerHangupCallbackCall, CALLSTATE_BRIDGED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerHangupCallbackLine, g_hAutoAnswerHangupCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerHangupCallbackLine, g_hAutoAnswerHangupCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Calling side should disconnect
        bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

                                
        SIPX_CALL hDestroyedCall = hCall ;
        destroyCall(hCall) ;


        // Finally, calling side is cleaned up
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;



        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoAnswerHangupCallback, NULL), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled), SIPX_RESULT_SUCCESS) ;
    
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine), SIPX_RESULT_SUCCESS);
    }

    OsTask::delay(TEST_DELAY) ;

    checkForLeaks() ;
}


// A calls B, B is busy
void sipXtapiTestSuite::testCallBusy() 
{
    bool bRC ;
    EventValidator validatorCalling("testCallBusy.calling") ;
    EventValidator validatorCalled("testCallBusy.calling") ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestCallBusy (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);
       
        SIPX_CALL hCall ;
        SIPX_LINE hLine ;
        SIPX_LINE hReceivingLine;

        validatorCalling.reset() ;
        validatorCalled.reset() ;

        // Setup Auto-answer call back
        sipxEventListenerAdd(g_hInst2, AutoRejectCallback, NULL) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled) ;        
        sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalling) ;

        sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine, CONTACT_LOCAL);        

        bRC = validatorCalled.waitForLineEvent(hReceivingLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        createCall(&hLine, &hCall) ;

        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxCallConnect(hCall, "sip:foo@127.0.0.1:9100") ;
        
        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DIALTONE, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_BUSY, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side (auto rejects)
        bRC = validatorCalled.waitForCallEvent(g_hAutoRejectCallbackLine, g_hAutoRejectCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoRejectCallbackLine, g_hAutoRejectCallbackCall, CALLSTATE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoRejectCallbackLine, g_hAutoRejectCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoRejectCallbackLine, g_hAutoRejectCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Calling side should disconnect                                
        SIPX_CALL hDestroyedCall = hCall ;
        destroyCall(hCall) ;

        // Finally, calling side is cleaned up
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoRejectCallback, NULL), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled), SIPX_RESULT_SUCCESS) ;
    
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine), SIPX_RESULT_SUCCESS);
    }

    OsTask::delay(TEST_DELAY) ;

    checkForLeaks() ;
}


// A calls B, B redirects to C
void sipXtapiTestSuite::testCallRedirect()
{
    bool bRC ;
    EventValidator validatorCalling("testCallRedirect.calling") ;
    EventValidator validatorRedirector("testCallRedirect.redirector") ;
    EventValidator validatorCalled("testCallRedirected.called") ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestCallRedirect (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        SIPX_CALL hCall ;
        SIPX_LINE hLine ;
        SIPX_LINE hRedirectLine ;
        SIPX_LINE hCalledLine ;

        validatorCalling.reset() ;
        validatorRedirector.reset() ;
        validatorCalled.reset() ;

        // Setup callbacks
        resetAutoAnswerCallback() ;
        sipxEventListenerAdd(g_hInst,  UniversalEventValidatorCallback, &validatorCalling) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorRedirector) ;
        sipxEventListenerAdd(g_hInst2, AutoRedirectCallback, NULL) ;
        sipxEventListenerAdd(g_hInst3, UniversalEventValidatorCallback, &validatorCalled) ;
        sipxEventListenerAdd(g_hInst3, AutoAnswerCallback, NULL) ;

        
        sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hRedirectLine, CONTACT_LOCAL);
        sipxLineAdd(g_hInst3, "sip:foo@127.0.0.1:10000", &hCalledLine, CONTACT_LOCAL);

        bRC = validatorRedirector.waitForLineEvent(hRedirectLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForLineEvent(hCalledLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        createCall(&hLine, &hCall) ;
        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;


        sipxCallConnect(hCall, "sip:foo@127.0.0.1:9100") ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DIALTONE, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        SIPX_CALL hDestroyCall = hCall ;
        destroyCall(hDestroyCall) ;
       
        // Finally, calling side is cleaned up
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_BRIDGED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Redirected UA
        bRC = validatorRedirector.waitForCallEvent(g_hAutoRedirectCallbackLine, g_hAutoRedirectCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorRedirector.waitForCallEvent(g_hAutoRedirectCallbackLine, g_hAutoRedirectCallbackCall, CALLSTATE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorRedirector.waitForCallEvent(g_hAutoRedirectCallbackLine, g_hAutoRedirectCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_REDIRECTED, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorRedirector.waitForCallEvent(g_hAutoRedirectCallbackLine, g_hAutoRedirectCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called UA
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;


        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst,  UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorRedirector), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoRedirectCallback, NULL), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst3, UniversalEventValidatorCallback, &validatorCalled), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst3, AutoAnswerCallback, NULL), SIPX_RESULT_SUCCESS);
        
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hRedirectLine), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hCalledLine), SIPX_RESULT_SUCCESS) ;
    }
    OsTask::delay(TEST_DELAY) ;

    checkForLeaks();
}


void sipXtapiTestSuite::testCallShutdown() 
{
    bool bRC ;
    EventValidator validatorCalling("testCallBasic.calling") ;
    EventValidator validatorCalled("testCallBasic.called") ;
   
    printf("\ntestCallShutdown (%2d of %2d)", 1, 1);
    SIPX_CALL hCall ;
    SIPX_LINE hLine ;
    SIPX_LINE hReceivingLine;     

    validatorCalling.reset() ;
    validatorCalled.reset() ;

    // Setup Auto-answer call back
    resetAutoAnswerCallback() ;
    sipxEventListenerAdd(g_hInst2, AutoAnswerCallback, NULL) ;
    sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled) ;        
    sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalling) ;

    sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine, CONTACT_LOCAL);
    bRC = validatorCalled.waitForLineEvent(hReceivingLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;

    createCall(&hLine, &hCall) ;
    bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;

    sipxCallConnect(hCall, "sip:foo@127.0.0.1:9100") ;
    

    // Validate Calling Side
    bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DIALTONE, CALLSTATE_CAUSE_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
    CPPUNIT_ASSERT(bRC) ;

    // Validate Called Side
    bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
    CPPUNIT_ASSERT(bRC) ;
            
    int connectionId = -1;
    
    CPPUNIT_ASSERT_EQUAL(sipxCallGetConnectionId(hCall, connectionId), SIPX_RESULT_SUCCESS);
    CPPUNIT_ASSERT(connectionId != -1) ;

    CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoAnswerCallback, NULL), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled), SIPX_RESULT_SUCCESS) ;
            
    destroyCall(hCall) ;
    destroyCall(g_hAutoAnswerCallbackCall) ;    

    CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);
    CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine), SIPX_RESULT_SUCCESS);
}

void sipXtapiTestSuite::testCallCancel() 
{
    bool bRC ;
    EventValidator validatorCalling("testCallCancel.calling") ;
    EventValidator validatorCalled("testCallCancel.called") ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR*4; iStressFactor++)
    {
        printf("\ntestCallCancel (%2d of %2d)", iStressFactor+1, STRESS_FACTOR*4);
        SIPX_CALL hCall ;
        SIPX_LINE hLine ;
        SIPX_LINE hReceivingLine ;

        validatorCalling.reset() ;
        validatorCalling.setMaxLookhead(20) ;
        validatorCalled.reset() ;
        validatorCalled.setMaxLookhead(20) ;
        
        // Setup Auto-answer call back
        resetAutoAnswerCallback() ;
        sipxEventListenerAdd(g_hInst2, AutoAnswerCallback, NULL) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled) ;        
        sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalling) ;

        sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine, CONTACT_LOCAL);
        bRC = validatorCalled.waitForLineEvent(hReceivingLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        createCall(&hLine, &hCall) ;
        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxCallConnect(hCall, "sip:foo@127.0.0.1:9100") ;
        int delay = rand() % 40 ;
        OsTask::delay(delay) ;

        SIPX_CALL hDestroyedCall = hCall ;
        sipxCallDestroy(hCall) ;
        
        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoAnswerCallback, NULL), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled), SIPX_RESULT_SUCCESS) ;
    
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine), SIPX_RESULT_SUCCESS);
    }

    OsTask::delay(TEST_DELAY) ;

    checkForLeaks();
}


void sipXtapiTestSuite::testCallHoldX(bool bTcp)
{
    bool bRC ;
    EventValidator validatorCalling("testCallHold.calling") ;
    EventValidator validatorCalled("testCallHold.called") ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        if (bTcp)
        {
            printf("\ntestCallHoldTCP (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);
        }
        else
        {
            printf("\ntestCallHold (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);
        }
        SIPX_CALL hCall ;
        SIPX_LINE hLine ;
        SIPX_LINE hCalledLine;

        validatorCalling.reset() ;
        validatorCalled.reset() ;

        // Setup Auto-answer call back
        resetAutoAnswerCallback() ;
        sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalling) ;        
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled) ;        
        sipxEventListenerAdd(g_hInst2, AutoAnswerCallback, NULL) ;
        
        // Add Line
        sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hCalledLine, CONTACT_LOCAL);        
        bRC = validatorCalled.waitForLineEvent(hCalledLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Create Call/Line
        createCall(&hLine, &hCall) ;
        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Place Call
        sipxCallConnect(hCall, "<sip:foo@127.0.0.1:9100;transport=tcp>") ;
        
        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DIALTONE, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // hold / unhold twice
        for (int j = 0; j < 2; j++)
        {
            // Hold
            CPPUNIT_ASSERT(sipxCallHold(hCall) == SIPX_RESULT_SUCCESS) ;
            
            bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_HELD, CALLSTATE_CAUSE_NORMAL, false) ;
            CPPUNIT_ASSERT(bRC) ;
            bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, true);
            CPPUNIT_ASSERT(bRC) ;
            bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, true);
            CPPUNIT_ASSERT(bRC) ;
            bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_REMOTE_HELD, CALLSTATE_CAUSE_NORMAL, false) ;
            CPPUNIT_ASSERT(bRC) ;
            bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, true);
            CPPUNIT_ASSERT(bRC) ;
            bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, true);
            CPPUNIT_ASSERT(bRC) ;

            // Unhold
            CPPUNIT_ASSERT(sipxCallUnhold(hCall) == SIPX_RESULT_SUCCESS) ;

            bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
            CPPUNIT_ASSERT(bRC) ;

            // KNOWN BUG - should be MEDIA_CAUSE_UNHOLD, rather than NORMAL
            bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
            CPPUNIT_ASSERT(bRC) ;
    //        bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_UNHOLD, MEDIA_TYPE_AUDIO, true);
    //        CPPUNIT_ASSERT(bRC) ;

            // KNOWN BUG - should be MEDIA_CAUSE_UNHOLD, rather than NORMAL
            bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
            CPPUNIT_ASSERT(bRC) ;
    //        bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_UNHOLD, MEDIA_TYPE_AUDIO, true);
    //        CPPUNIT_ASSERT(bRC) ;


            bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
            CPPUNIT_ASSERT(bRC) ;
            bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_UNHOLD, MEDIA_TYPE_AUDIO, true);
            CPPUNIT_ASSERT(bRC) ;
            bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_UNHOLD, MEDIA_TYPE_AUDIO, true);
            CPPUNIT_ASSERT(bRC) ;
        }              


        // Drop               
        SIPX_CALL hDestroyedCall = hCall ;
        destroyCall(hCall) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_BRIDGED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;        
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoAnswerCallback, NULL), SIPX_RESULT_SUCCESS) ;
    
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hCalledLine), SIPX_RESULT_SUCCESS);
    }

    OsTask::delay(TEST_DELAY) ;

    checkForLeaks();
}
// A calls B, B answers, A places call on hold, A takes call off hold, A hangs up
void sipXtapiTestSuite::testCallHold() 
{
    testCallHoldX(false);
}


// A calls B, B answers, A places call on hold, A takes call off hold, A hangs up
void sipXtapiTestSuite::testCallHoldTCP() 
{
    testCallHoldX(true);
}

// A calls B, B answers, A places call on hold for longer than the idle timeout, A takes call off hold, A hangs up
void sipXtapiTestSuite::testCallHoldExceedingIdleTimeout() 
{
    bool bRC ;
    EventValidator validatorCalling("testCallHoldExceedingIdleTimeout.calling") ;
    EventValidator validatorCalled("testCallHoldExceedingIdleTimeout.called") ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestCallHolddExceedingIdleTimeout (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);
        SIPX_CALL hCall ;
        SIPX_LINE hLine ;
        SIPX_LINE hCalledLine;

        validatorCalling.reset() ;
        validatorCalled.reset() ;

        // Setup Auto-answer call back
        resetAutoAnswerCallback() ;
        sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalling) ;        
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled) ;        
        sipxEventListenerAdd(g_hInst2, AutoAnswerCallback, NULL) ;

        // Set idle timeout
        CPPUNIT_ASSERT(sipxConfigSetConnectionIdleTimeout(g_hInst, 5) == SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT(sipxConfigSetConnectionIdleTimeout(g_hInst2, 5) == SIPX_RESULT_SUCCESS);
        
        // Add Line
        sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hCalledLine, CONTACT_LOCAL);        
        bRC = validatorCalled.waitForLineEvent(hCalledLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Create Call/Line
        createCall(&hLine, &hCall) ;
        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Place Call
        sipxCallConnect(hCall, "sip:foo@127.0.0.1:9100") ;
        
        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DIALTONE, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // Hold
        CPPUNIT_ASSERT(sipxCallHold(hCall) == SIPX_RESULT_SUCCESS) ;

        printf("\nWaiting for 10 seconds! (making sure no idle timeout event gets fired)");
        OsTask::delay(10000);
        
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_HELD, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_REMOTE_HELD, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // Unhold
        CPPUNIT_ASSERT(sipxCallUnhold(hCall) == SIPX_RESULT_SUCCESS) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_UNHOLD, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_UNHOLD, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
              
        // Drop               
        SIPX_CALL hDestroyedCall = hCall ;
        destroyCall(hCall) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_BRIDGED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;        
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoAnswerCallback, NULL), SIPX_RESULT_SUCCESS) ;
    
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hCalledLine), SIPX_RESULT_SUCCESS);
    }

    OsTask::delay(TEST_DELAY) ;

    checkForLeaks();
}

void sipXtapiTestSuite::testSendInfo()
{
    bool bRC ;
    EventValidator validatorCalling("testSendInfo.calling") ;
    EventValidator validatorCalled("testSendInfo.called") ;
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestSendInfo (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        SIPX_CALL hCall ;
        SIPX_LINE hLine ;
        SIPX_LINE hReceivingLine ;
        SIPX_INFO hInfo;

        validatorCalling.reset() ;
        validatorCalled.reset() ;

        // Setup Auto-answer call back
        resetAutoAnswerCallback() ;
        sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalling) ;
        sipxEventListenerAdd(g_hInst2, AutoAnswerCallback, NULL) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled) ;

        CPPUNIT_ASSERT_EQUAL(sipxConfigSetUserAgentName(g_hInst, "sipXtapiTest", false), SIPX_RESULT_SUCCESS) ;

        sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine, CONTACT_LOCAL);
        bRC = validatorCalled.waitForLineEvent(hReceivingLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        
        createCall(&hLine, &hCall) ;
        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxCallConnect(hCall, "sip:foo@127.0.0.1:9100") ;

       // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DIALTONE, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // Send Info
        UtlString payload("abc") ;
        sipxCallSendInfo(&hInfo, hCall, "text/plain", payload, payload.length());
        bRC = validatorCalling.waitForInfoStatusEvent(hInfo, 0, 200, "OK") ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForInfoEvent(g_hAutoAnswerCallbackCall, g_hAutoAnswerCallbackLine, "sip:foo@127.0.0.1:9100", "sipXtapiTest", "text/plain", payload, payload.length()) ;
        CPPUNIT_ASSERT(bRC) ;

        // Send Another Info
        payload = "soylent green is people!" ;
        sipxCallSendInfo(&hInfo, hCall, "text/plain", payload, payload.length());
        bRC = validatorCalling.waitForInfoStatusEvent(hInfo, 0, 200, "OK") ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForInfoEvent(g_hAutoAnswerCallbackCall, g_hAutoAnswerCallbackLine, "sip:foo@127.0.0.1:9100", "sipXtapiTest", "text/plain", payload, payload.length()) ;
        CPPUNIT_ASSERT(bRC) ;

        SIPX_CALL hDestroyedCall = hCall ;
        destroyCall(hDestroyedCall) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_BRIDGED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoAnswerCallback, NULL), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled), SIPX_RESULT_SUCCESS) ;
    
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine), SIPX_RESULT_SUCCESS);
    }
    OsTask::delay(TEST_DELAY) ;

    checkForLeaks();
}

void sipXtapiTestSuite::testSendInfoFailure()
{
    // TEST HACK: Disable INFO on the receiving end
    sipxConfigAllowMethod(g_hInst2, "INFO", false);

    bool bRC ;
    EventValidator validatorCalling("testSendInfoFailure.calling") ;
    EventValidator validatorCalled("testSendInfoFailure.called") ;
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestSendInfoFailure (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        SIPX_CALL hCall ;
        SIPX_LINE hLine ;
        SIPX_LINE hReceivingLine ;
        SIPX_INFO hInfo;

        validatorCalling.reset() ;
        validatorCalled.reset() ;

        // Setup Auto-answer call back
        resetAutoAnswerCallback() ;
        sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalling) ;
        sipxEventListenerAdd(g_hInst2, AutoAnswerCallback, NULL) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled) ;

        CPPUNIT_ASSERT_EQUAL(sipxConfigSetUserAgentName(g_hInst, "sipXtapiTest", false), SIPX_RESULT_SUCCESS) ;

        sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine, CONTACT_LOCAL);
        bRC = validatorCalled.waitForLineEvent(hReceivingLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        
        createCall(&hLine, &hCall) ;
        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxCallConnect(hCall, "sip:foo@127.0.0.1:9100") ;

       // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DIALTONE, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // Send Info
        UtlString payload("the answer is 42") ;
        sipxCallSendInfo(&hInfo, hCall, "text/plain", payload, payload.length());
        bRC = validatorCalling.waitForInfoStatusEvent(hInfo, 2, 501, "Not Implemented") ;
        CPPUNIT_ASSERT(bRC) ;

        SIPX_CALL hDestroyedCall = hCall ;
        destroyCall(hDestroyedCall) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_BRIDGED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoAnswerCallback, NULL), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled), SIPX_RESULT_SUCCESS) ;
    
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine), SIPX_RESULT_SUCCESS);
    }
    OsTask::delay(TEST_DELAY) ;

    // TEST HACK: Enable INFO on the receiving end
    sipxConfigAllowMethod(g_hInst2, "INFO", true);

    checkForLeaks();
}

void sipXtapiTestSuite::testSendInfoTimeout()
{
    // TEST HACK: now set the response code for testing
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) g_hInst2;    
    pInst->pMessageObserver->setTestResponseCode(408);

    EventValidator validatorCalling("testSendInfoTimeout.calling") ;
    EventValidator validatorCalled("testSendInfoTimeout.called") ;
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestSendInfoTimeout (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        SIPX_CALL hCall ;
        SIPX_LINE hLine ;
        SIPX_LINE hReceivingLine ;
        SIPX_INFO hInfo;
        bool bRC ;

        validatorCalling.reset() ;
        validatorCalled.reset() ;

        // Setup Auto-answer call back
        resetAutoAnswerCallback() ;
        sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalling) ;
        sipxEventListenerAdd(g_hInst2, AutoAnswerCallback, NULL) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled) ;

        CPPUNIT_ASSERT_EQUAL(sipxConfigSetUserAgentName(g_hInst, "sipXtapiTest", false), SIPX_RESULT_SUCCESS) ;

        sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine, CONTACT_LOCAL);
        bRC = validatorCalled.waitForLineEvent(hReceivingLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        
        createCall(&hLine, &hCall) ;
        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxCallConnect(hCall, "sip:foo@127.0.0.1:9100") ;

       // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DIALTONE, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // Send Info
        UtlString payload("the answer is 42") ;
        sipxCallSendInfo(&hInfo, hCall, "text/plain", payload, payload.length());
        bRC = validatorCalling.waitForInfoStatusEvent(hInfo, 1, 408, "timed out") ;
        CPPUNIT_ASSERT(bRC) ;

        SIPX_CALL hDestroyedCall = hCall ;
        destroyCall(hDestroyedCall) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_BRIDGED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoAnswerCallback, NULL), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled), SIPX_RESULT_SUCCESS) ;
    
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine), SIPX_RESULT_SUCCESS);
    }
    OsTask::delay(TEST_DELAY) ;  

    // TEST HACK: Reset changes
    pInst->pMessageObserver->setTestResponseCode(0);

    checkForLeaks();
}


// A calls B, B answers, B hangs up
void sipXtapiTestSuite::testCallGetRemoteUserAgent() 
{
    bool bRC ;
    EventValidator validatorCalling("testCallGetRemoteUserAgent.calling") ;
    EventValidator validatorCalled("testCallGetRemoteUserAgent.called") ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestCallGetRemoteUserAgent (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);
        SIPX_CALL hCall ;
        SIPX_LINE hLine ;
        SIPX_LINE hReceivingLine;

        validatorCalling.reset() ;
        validatorCalled.reset() ;

        sipxConfigSetUserAgentName(g_hInst2, "TestCalled", false);

        // Setup Auto-answer call back
        sipxEventListenerAdd(g_hInst2, AutoAnswerHangupCallback, NULL) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled) ;        
        sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalling) ;

        sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine, CONTACT_LOCAL);        

        bRC = validatorCalled.waitForLineEvent(hReceivingLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        createCall(&hLine, &hCall) ;

        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxCallConnect(hCall, "sip:foo@127.0.0.1:9100") ;
        
        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DIALTONE, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerHangupCallbackLine, g_hAutoAnswerHangupCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerHangupCallbackLine, g_hAutoAnswerHangupCallbackCall, CALLSTATE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerHangupCallbackLine, g_hAutoAnswerHangupCallbackCall, CALLSTATE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerHangupCallbackLine, g_hAutoAnswerHangupCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        char agent[64];
        sipxCallGetRemoteUserAgent(hCall, agent, 64);
        CPPUNIT_ASSERT_EQUAL(0, strcmp(agent, "TestCalled"));

        // Called side with automatically hang up
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerHangupCallbackLine, g_hAutoAnswerHangupCallbackCall, CALLSTATE_BRIDGED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerHangupCallbackLine, g_hAutoAnswerHangupCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerHangupCallbackLine, g_hAutoAnswerHangupCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Calling side should disconnect
        bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

                                
        SIPX_CALL hDestroyedCall = hCall ;
        destroyCall(hCall) ;

        // Finally, calling side is cleaned up
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;



        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoAnswerHangupCallback, NULL), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled), SIPX_RESULT_SUCCESS) ;
    
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine), SIPX_RESULT_SUCCESS);
    }

    OsTask::delay(TEST_DELAY) ;

    checkForLeaks() ;
}


void sipXtapiTestSuite::testManualCallDialtone()
{
    SIPX_CALL hCall ;
    SIPX_LINE hLine ;
    
    // Setup
    sipxLineAdd(g_hInst, "sip:foo@10.1.1.120", &hLine) ;
    sipxCallCreate(g_hInst, hLine, &hCall) ;

    sipxCallAudioPlayFileStart(hCall, "c:\\na_dialtone.wav", true, true, false) ;
    OsTask::delay(1000) ;
    sipxCallStartTone(hCall, ID_DTMF_0, true, false) ;
    OsTask::delay(1000) ;
    sipxCallStartTone(hCall, ID_DTMF_1, true, false) ;
    OsTask::delay(1000) ;
    sipxCallStartTone(hCall, ID_DTMF_2, true, false) ;
    OsTask::delay(1000) ;
    sipxCallAudioPlayFileStop(hCall) ;
    OsTask::delay(5000) ;
    sipxCallAudioPlayFileStart(hCall, "c:\\na_dialtone.wav", true, true, false) ;
    OsTask::delay(10000) ;
    sipxCallAudioPlayFileStop(hCall) ;

    // Destory
    sipxCallDestroy(hCall) ;
    sipxLineRemove(hLine) ;
}

/*
void sipXtapiTestSuite::testCallBasicSecure()
{
    bool bRC ;
    EventValidator validatorCalling("testCallBasic.calling") ;
    EventValidator validatorCalled("testCallBasic.called") ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestCallBasicSecure (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);
        SIPX_CALL hCall ;
        SIPX_LINE hLine ;
        SIPX_LINE hReceivingLine;     

        validatorCalling.reset() ;
        validatorCalled.reset() ;

        // Setup Auto-answer call back
        resetAutoAnswerCallback() ;

        sipxConfigSetSecurityParameters(g_hInst, ".", "mcohen@pingtel.com", "password");
        sipxConfigSetSecurityParameters(g_hInst2, ".", "mcohen@pingtel.com", "password");

        SIPX_SECURITY_ATTRIBUTES security;
        SIPX_SECURITY_ATTRIBUTES calleeSecurity;

        char szPublicKey[4096];
        unsigned long actualRead = 0;
    
        OsFile publicKeyFile("user_cert_mcohen@pingtel.com.der");

        publicKeyFile.open();
        publicKeyFile.read((void*)szPublicKey, 4096, actualRead);
        publicKeyFile.close();
    
        UtlString der(szPublicKey, actualRead);

        security.setSmimeKey(szPublicKey, actualRead);
        security.setSrtpKey("012345678901234567890123456789", 30);
        security.setSecurityLevel(SRTP_LEVEL_ENCRYPTION_AND_AUTHENTICATION);

        calleeSecurity = security; // for this test both the callee and caller
                                   // can use the same private key, so they can
                                   // also encrypt with the same public key.

        calleeSecurity.setSrtpKey("", 30); // null out the callee srtp key, it should
                                           // be negotiated

        setAutoAnswerSecurity(&calleeSecurity);

        sipxEventListenerAdd(g_hInst2, AutoAnswerCallback_Secure, NULL) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled) ;        
        sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalling) ;

        sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine, CONTACT_LOCAL);
        bRC = validatorCalled.waitForLineEvent(hReceivingLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        createCall(&hLine, &hCall) ;
        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

    	sipxCallConnect(hCall, "sip:foo@127.0.0.1:9100", CONTACT_AUTO, NULL, &security) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DIALTONE, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalling.waitForSecurityEvent(SECURITY_ENCRYPT, SECURITY_CAUSE_ENCRYPT_SUCCESS, true);
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalling.waitForSecurityEvent(SECURITY_DECRYPT, SECURITY_CAUSE_DECRYPT_SUCCESS, true);
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalled.waitForSecurityEvent(SECURITY_DECRYPT, SECURITY_CAUSE_DECRYPT_SUCCESS, true);
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalled.waitForSecurityEvent(SECURITY_ENCRYPT, SECURITY_CAUSE_ENCRYPT_SUCCESS, true);
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
                
        int connectionId = -1;
        
        CPPUNIT_ASSERT_EQUAL(sipxCallGetConnectionId(hCall, connectionId), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT(connectionId != -1) ;
                
        SIPX_CALL hDestroyedCall = hCall ;
        destroyCall(hCall) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_BRIDGED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoAnswerCallback_Secure, NULL), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled), SIPX_RESULT_SUCCESS) ;
    
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine), SIPX_RESULT_SUCCESS);
    }

    OsTask::delay(TEST_DELAY) ;

    checkForLeaks();
}

void sipXtapiTestSuite::testCallHoldSecure()
{
    bool bRC ;
    EventValidator validatorCalling("testCallBasic.calling") ;
    EventValidator validatorCalled("testCallBasic.called") ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestCallHoldSecure (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);
        SIPX_CALL hCall ;
        SIPX_LINE hLine ;
        SIPX_LINE hReceivingLine;     

        validatorCalling.reset() ;
        validatorCalled.reset() ;

        // Setup Auto-answer call back
        resetAutoAnswerCallback() ;

        sipxConfigSetSecurityParameters(g_hInst, ".", "mcohen@pingtel.com", "password");
        sipxConfigSetSecurityParameters(g_hInst2, ".", "mcohen@pingtel.com", "password");

        SIPX_SECURITY_ATTRIBUTES security;
        SIPX_SECURITY_ATTRIBUTES calleeSecurity;

        char szPublicKey[4096];
        unsigned long actualRead = 0;
    
        OsFile publicKeyFile("user_cert_mcohen@pingtel.com.der");

        publicKeyFile.open();
        publicKeyFile.read((void*)szPublicKey, 4096, actualRead);
        publicKeyFile.close();
    
        UtlString der(szPublicKey, actualRead);

        security.setSmimeKey(szPublicKey, actualRead);
        security.setSrtpKey("012345678901234567890123456789", 30);
        security.setSecurityLevel(SRTP_LEVEL_ENCRYPTION_AND_AUTHENTICATION);

        calleeSecurity = security; // for this test both the callee and caller
                                   // can use the same private key, so they can
                                   // also encrypt with the same public key.

        calleeSecurity.setSrtpKey("", 30); // null out the callee srtp key, it should
                                           // be negotiated

        setAutoAnswerSecurity(&calleeSecurity);

        sipxEventListenerAdd(g_hInst2, AutoAnswerCallback_Secure, NULL) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled) ;        
        sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalling) ;

        sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine, CONTACT_LOCAL);
        bRC = validatorCalled.waitForLineEvent(hReceivingLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        createCall(&hLine, &hCall) ;
        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

    	sipxCallConnect(hCall, "sip:foo@127.0.0.1:9100", CONTACT_AUTO, NULL, &security) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DIALTONE, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalling.waitForSecurityEvent(SECURITY_ENCRYPT, SECURITY_CAUSE_ENCRYPT_SUCCESS, true);
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalling.waitForSecurityEvent(SECURITY_DECRYPT, SECURITY_CAUSE_DECRYPT_SUCCESS, true);
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalled.waitForSecurityEvent(SECURITY_DECRYPT, SECURITY_CAUSE_DECRYPT_SUCCESS, true);
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalled.waitForSecurityEvent(SECURITY_ENCRYPT, SECURITY_CAUSE_ENCRYPT_SUCCESS, true);
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // HOLD
        SIPX_RESULT result;

        // the caller puts the callee on hold
        result = sipxCallHold(hCall);
        CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == result);

        // the caller should receive a BRIDGED event
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_BRIDGED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // the caller (in this case, the hold-er) should get an encrypt success event
        bRC = validatorCalling.waitForSecurityEvent(SECURITY_ENCRYPT, SECURITY_CAUSE_ENCRYPT_SUCCESS, true);
        CPPUNIT_ASSERT(bRC) ;

        // the caller should receive a HELD event
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_HELD, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // caller should receive a AUDIO STOP event
        bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // callee should receive a DECRYPT SUCCESS
        bRC = validatorCalled.waitForSecurityEvent(SECURITY_DECRYPT, SECURITY_CAUSE_DECRYPT_SUCCESS, true);
        CPPUNIT_ASSERT(bRC) ;

        // callee should receive and AUDIO STOP
        bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // callee should receive a REMOTE_HELD event
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_REMOTE_HELD, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // the caller takes the callee off hold
        // UNHOLD
        result = sipxCallUnhold(hCall);
        CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == result);

        // caller gets an DECRYPT event, then an ENCRYPT
        bRC = validatorCalling.waitForSecurityEvent(SECURITY_DECRYPT, SECURITY_CAUSE_DECRYPT_SUCCESS, true);
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalling.waitForSecurityEvent(SECURITY_ENCRYPT, SECURITY_CAUSE_ENCRYPT_SUCCESS, true);
        CPPUNIT_ASSERT(bRC) ;

        // caller gets a BRIDGED::CALL_NORMAL event 
        // the caller should receive a BRIDGED event
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_BRIDGED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // the caller should receive a CONNECTED NORMAL event
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // caller gets an DECRYPT event
        bRC = validatorCalling.waitForSecurityEvent(SECURITY_DECRYPT, SECURITY_CAUSE_DECRYPT_SUCCESS, true);
        CPPUNIT_ASSERT(bRC) ;

        // Caller gets an AUDIO START
        bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_UNHOLD, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_UNHOLD, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // callee generates an ENCRYPT
        bRC = validatorCalled.waitForSecurityEvent(SECURITY_ENCRYPT, SECURITY_CAUSE_ENCRYPT_SUCCESS, true);
        CPPUNIT_ASSERT(bRC) ;

        // callee generates a DECRYPT
        bRC = validatorCalled.waitForSecurityEvent(SECURITY_DECRYPT, SECURITY_CAUSE_DECRYPT_SUCCESS, true);
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_UNHOLD, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_UNHOLD, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // callee generates an ENCRYPT
        bRC = validatorCalled.waitForSecurityEvent(SECURITY_ENCRYPT, SECURITY_CAUSE_ENCRYPT_SUCCESS, true);
        CPPUNIT_ASSERT(bRC) ;

        SIPX_CALL hDestroyedCall = hCall ;
        destroyCall(hCall) ;

        // caller gets a BRIDGED::CALL_NORMAL event 
        // the caller should receive a BRIDGED event
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_BRIDGED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoAnswerCallback_Secure, NULL), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled), SIPX_RESULT_SUCCESS) ;
    
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine), SIPX_RESULT_SUCCESS);
    }

    OsTask::delay(TEST_DELAY) ;

    checkForLeaks();
}

void sipXtapiTestSuite::testCallSecurityCalleeUnsupported()
{
    bool bRC ;
    EventValidator validatorCalling("testCallSecurityCalleeUnsupported.calling") ;
    EventValidator validatorCalled("testCallSecurityCalleeUnsupported.called") ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestCallSecurityCalleeUnsupported (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);
        SIPX_CALL hCall ;
        SIPX_LINE hLine ;
        SIPX_LINE hReceivingLine;     

        validatorCalling.reset() ;
        validatorCalled.reset() ;

        // Setup Auto-answer call back
        resetAutoAnswerCallback() ;

        SIPX_SECURITY_ATTRIBUTES security;
        SIPX_SECURITY_ATTRIBUTES callerSecurity;

        char szPublicKey[4096];
        unsigned long actualRead = 0;
    
        OsFile publicKeyFile("user_cert_mcohen@pingtel.com.der");

        publicKeyFile.open();
        publicKeyFile.read((void*)szPublicKey, 4096, actualRead);
        publicKeyFile.close();
    
        UtlString der(szPublicKey, actualRead);

        security.setSmimeKey(szPublicKey, actualRead);
        security.setSrtpKey("012345678901234567890123456789", 30);
        security.setSecurityLevel(SRTP_LEVEL_ENCRYPTION_AND_AUTHENTICATION);

        // Don't set calle's security parameters. This will pass in a NULL to accept
        setAutoAnswerSecurity(NULL);

        sipxEventListenerAdd(g_hInst2, AutoAnswerCallback_Secure, NULL) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled) ;        
        sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalling) ;

        sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine, CONTACT_LOCAL);
        bRC = validatorCalled.waitForLineEvent(hReceivingLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        createCall(&hLine, &hCall) ;
        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

    	sipxCallConnect(hCall, "sip:foo@127.0.0.1:9100", CONTACT_AUTO, NULL, &security) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DIALTONE, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForSecurityEvent(SECURITY_ENCRYPT, SECURITY_CAUSE_ENCRYPT_SUCCESS, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_REMOTE_SMIME_UNSUPPORTED, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_SMIME_FAILURE, true) ;
        CPPUNIT_ASSERT(bRC) ;

        int connectionId = -1;
        
        CPPUNIT_ASSERT_EQUAL(sipxCallGetConnectionId(hCall, connectionId), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT(connectionId != -1) ;
                
        SIPX_CALL hDestroyedCall = hCall ;
        destroyCall(hCall) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoAnswerCallback_Secure, NULL), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled), SIPX_RESULT_SUCCESS) ;
    
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine), SIPX_RESULT_SUCCESS);
    }

    OsTask::delay(TEST_DELAY) ;

    checkForLeaks();
}

void sipXtapiTestSuite::testCallSecurityCallerUnsupported()
{
    bool bRC ;
    EventValidator validatorCalling("testCallSecurityCallerUnsupported.calling") ;
    EventValidator validatorCalled("testCallSecurityCallerUnsupported.called") ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestCallSecurityCallerUnsupported (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);
        SIPX_CALL hCall ;
        SIPX_LINE hLine ;
        SIPX_LINE hReceivingLine;     

        validatorCalling.reset() ;
        validatorCalled.reset() ;

        // Setup Auto-answer call back
        resetAutoAnswerCallback() ;

        SIPX_SECURITY_ATTRIBUTES security;
        SIPX_SECURITY_ATTRIBUTES calleeSecurity;

        char szPublicKey[4096];
        unsigned long actualRead = 0;
    
        OsFile publicKeyFile("user_cert_mcohen@pingtel.com.der");

        publicKeyFile.open();
        publicKeyFile.read((void*)szPublicKey, 4096, actualRead);
        publicKeyFile.close();
    
        UtlString der(szPublicKey, actualRead);

        security.setSmimeKey(szPublicKey, actualRead);
        security.setSrtpKey("012345678901234567890123456789", 30);
        security.setSecurityLevel(SRTP_LEVEL_ENCRYPTION_AND_AUTHENTICATION);

        calleeSecurity = security; // for this test both the callee and caller
                                   // can use the same private key, so they can
                                   // also encrypt with the same public key.

        calleeSecurity.setSrtpKey("", 30); // null out the callee srtp key, it should
                                           // be negotiated

        setAutoAnswerSecurity(&calleeSecurity);

        sipxEventListenerAdd(g_hInst2, AutoAnswerCallback_Secure, NULL) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled) ;        
        sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalling) ;

        sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine, CONTACT_LOCAL);
        bRC = validatorCalled.waitForLineEvent(hReceivingLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        createCall(&hLine, &hCall) ;
        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

    	sipxCallConnect(hCall, "sip:foo@127.0.0.1:9100", CONTACT_AUTO, NULL, NULL) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DIALTONE, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_SMIME_FAILURE, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_REMOTE_SMIME_UNSUPPORTED, true) ;
        CPPUNIT_ASSERT(bRC) ;

        int connectionId = -1;
        
        CPPUNIT_ASSERT_EQUAL(sipxCallGetConnectionId(hCall, connectionId), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT(connectionId != -1) ;
                
        SIPX_CALL hDestroyedCall = hCall ;
        destroyCall(hCall) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoAnswerCallback_Secure, NULL), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled), SIPX_RESULT_SUCCESS) ;
    
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine), SIPX_RESULT_SUCCESS);
    }

    OsTask::delay(TEST_DELAY) ;

    checkForLeaks();
}



void sipXtapiTestSuite::testCallSecurityCalleeStepUp()
{
}

void sipXtapiTestSuite::testCallSecurityBadParams()
{
}

*/


// A holds B, B holds A, A unholds B, B unholds A
void sipXtapiTestSuite::callMultipleProc1(
                                  SIPX_CALL hCallingParty, 
                                  SIPX_LINE hCallingPartyLine,
                                  EventValidator* pCallingPartyValidator,
                                  SIPX_CALL hCalledParty,
                                  SIPX_LINE hCalledPartyLine,
                                  EventValidator* pCalledPartyValidator)
{
    bool bRC ;
    SIPX_RESULT sipxRC ;

    pCallingPartyValidator->reset() ;
    pCalledPartyValidator->reset() ;

    /*
     * A Holds B
     */
    pCallingPartyValidator->addMarker("A Holds B") ;
    pCalledPartyValidator->addMarker("A Holds B") ;
    sipxRC = sipxCallHold(hCallingParty) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
    
    bRC = pCallingPartyValidator->waitForCallEvent(hCallingPartyLine, hCallingParty, CALLSTATE_HELD, CALLSTATE_CAUSE_NORMAL) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCallingPartyValidator->waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO);
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCallingPartyValidator->waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO);
    CPPUNIT_ASSERT(bRC) ;

    bRC = pCalledPartyValidator->waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO);
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCalledPartyValidator->waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO);
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCalledPartyValidator->waitForCallEvent(hCalledPartyLine, hCalledParty, CALLSTATE_REMOTE_HELD, CALLSTATE_CAUSE_NORMAL) ;
    CPPUNIT_ASSERT(bRC) ;

    bRC = !pCallingPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = !pCalledPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;

    /*
     * B Holds A
     */
    pCallingPartyValidator->addMarker("B Holds A") ;
    pCalledPartyValidator->addMarker("B Holds A") ;

    sipxRC = sipxCallHold(hCalledParty) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

    bRC = pCalledPartyValidator->waitForCallEvent(hCalledPartyLine, hCalledParty, CALLSTATE_HELD, CALLSTATE_CAUSE_NORMAL) ;
    CPPUNIT_ASSERT(bRC) ;

    bRC = !pCallingPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = !pCalledPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;


    /*
     * A unholds B
     */
    pCallingPartyValidator->addMarker("A Unholds B") ;
    pCalledPartyValidator->addMarker("A Unholds B") ;

    sipxRC = sipxCallUnhold(hCallingParty) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

    bRC = pCallingPartyValidator->waitForCallEvent(hCallingPartyLine, hCallingParty, CALLSTATE_REMOTE_HELD, CALLSTATE_CAUSE_NORMAL) ;

    bRC = !pCallingPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = !pCalledPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;


    /*
     * B unholds A
     */
    pCallingPartyValidator->addMarker("B Unholds A") ;
    pCalledPartyValidator->addMarker("B Unholds A") ;

    sipxRC = sipxCallUnhold(hCalledParty) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

    bRC = pCalledPartyValidator->waitForCallEvent(hCalledPartyLine, hCalledParty, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCalledPartyValidator->waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO);
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCalledPartyValidator->waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO);
    CPPUNIT_ASSERT(bRC) ;

    bRC = pCallingPartyValidator->waitForCallEvent(hCallingPartyLine, hCallingParty, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCallingPartyValidator->waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_UNHOLD, MEDIA_TYPE_AUDIO, true);
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCallingPartyValidator->waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_UNHOLD, MEDIA_TYPE_AUDIO, true);
    CPPUNIT_ASSERT(bRC) ;

    bRC = !pCallingPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = !pCalledPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;
}

// A calls B, A holds B, B holds A, A unholds B, B unholds A
void sipXtapiTestSuite::testCallHoldMultiple1() 
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestCallHoldMultiple1 (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        doCallBasic(g_hInst, "sip:calling@127.0.0.1:8000", "sip:calling@127.0.0.1:8000",
                    g_hInst2, "sip:called@127.0.0.1:9100", "sip:called@127.0.0.1:9100", 
                    sipXtapiTestSuite::callMultipleProc1) ;
    }

    OsTask::delay(TEST_DELAY) ;
    checkForLeaks();
}

// B holds A, A holds B, A unholds B, B unholds A
void sipXtapiTestSuite::callMultipleProc2(
                                  SIPX_CALL hCallingParty, 
                                  SIPX_LINE hCallingPartyLine,
                                  EventValidator* pCallingPartyValidator,
                                  SIPX_CALL hCalledParty,
                                  SIPX_LINE hCalledPartyLine,
                                  EventValidator* pCalledPartyValidator)
{
    bool bRC ;
    SIPX_RESULT sipxRC ;

    pCallingPartyValidator->reset() ;
    pCalledPartyValidator->reset() ;

    /*
     * B Holds A
     */
    pCallingPartyValidator->addMarker("B Holds A") ;
    pCalledPartyValidator->addMarker("B Holds A") ;
    sipxRC = sipxCallHold(hCalledParty) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
    
    bRC = pCalledPartyValidator->waitForCallEvent(hCalledPartyLine, hCalledParty, CALLSTATE_HELD, CALLSTATE_CAUSE_NORMAL) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCalledPartyValidator->waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO);
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCalledPartyValidator->waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO);
    CPPUNIT_ASSERT(bRC) ;

    bRC = pCallingPartyValidator->waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO);
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCallingPartyValidator->waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO);
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCallingPartyValidator->waitForCallEvent(hCallingPartyLine, hCallingParty, CALLSTATE_REMOTE_HELD, CALLSTATE_CAUSE_NORMAL) ;
    CPPUNIT_ASSERT(bRC) ;

    bRC = !pCallingPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = !pCalledPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;

    /*
     * A Holds B
     */
    pCallingPartyValidator->addMarker("A Holds B") ;
    pCalledPartyValidator->addMarker("A Holds B") ;

    sipxRC = sipxCallHold(hCallingParty) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

    bRC = pCallingPartyValidator->waitForCallEvent(hCallingPartyLine, hCallingParty, CALLSTATE_HELD, CALLSTATE_CAUSE_NORMAL) ;
    CPPUNIT_ASSERT(bRC) ;

    bRC = !pCallingPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = !pCalledPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;


    /*
     * A unholds B
     */
    pCallingPartyValidator->addMarker("A Unholds B") ;
    pCalledPartyValidator->addMarker("A Unholds B") ;

    sipxRC = sipxCallUnhold(hCallingParty) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

    bRC = pCallingPartyValidator->waitForCallEvent(hCallingPartyLine, hCallingParty, CALLSTATE_REMOTE_HELD, CALLSTATE_CAUSE_NORMAL) ;

    bRC = !pCallingPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = !pCalledPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;


    /*
     * B unholds A
     */
    pCallingPartyValidator->addMarker("B Unholds A") ;
    pCalledPartyValidator->addMarker("B Unholds A") ;

    sipxRC = sipxCallUnhold(hCalledParty) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

    bRC = pCalledPartyValidator->waitForCallEvent(hCalledPartyLine, hCalledParty, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCalledPartyValidator->waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO);
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCalledPartyValidator->waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO);
    CPPUNIT_ASSERT(bRC) ;

    bRC = pCallingPartyValidator->waitForCallEvent(hCallingPartyLine, hCallingParty, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCallingPartyValidator->waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_UNHOLD, MEDIA_TYPE_AUDIO, true);
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCallingPartyValidator->waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_UNHOLD, MEDIA_TYPE_AUDIO, true);
    CPPUNIT_ASSERT(bRC) ;

    bRC = !pCallingPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = !pCalledPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;
}

// A calls B, B holds A, A holds B, A unholds B, B unholds A
void sipXtapiTestSuite::testCallHoldMultiple2() 
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestCallHoldMultiple2 (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        doCallBasic(g_hInst, "sip:calling@127.0.0.1:8000", "sip:calling@127.0.0.1:8000",
                    g_hInst2, "sip:called@127.0.0.1:9100", "sip:called@127.0.0.1:9100", 
                    sipXtapiTestSuite::callMultipleProc2) ;
    }

    OsTask::delay(TEST_DELAY) ;
    checkForLeaks();
}



// B local holds A, A holds B, A unholds B, B unlocal holds A
void sipXtapiTestSuite::callMultipleProc3(
                                  SIPX_CALL hCallingParty, 
                                  SIPX_LINE hCallingPartyLine,
                                  EventValidator* pCallingPartyValidator,
                                  SIPX_CALL hCalledParty,
                                  SIPX_LINE hCalledPartyLine,
                                  EventValidator* pCalledPartyValidator)
{
    bool bRC ;
    SIPX_RESULT sipxRC ;

    pCallingPartyValidator->reset() ;
    pCalledPartyValidator->reset() ;

    /*
     * B local Holds A
     */
    pCallingPartyValidator->addMarker("B local Holds A") ;
    pCalledPartyValidator->addMarker("B local Holds A") ;
    sipxRC = sipxCallHold(hCalledParty, false) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
    
    bRC = pCalledPartyValidator->waitForCallEvent(hCalledPartyLine, hCalledParty, CALLSTATE_BRIDGED, CALLSTATE_CAUSE_NORMAL) ;
    CPPUNIT_ASSERT(bRC) ;

    bRC = !pCallingPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = !pCalledPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;

    /*
     * A Holds B
     */
    pCallingPartyValidator->addMarker("A Holds B") ;
    pCalledPartyValidator->addMarker("A Holds B") ;

    sipxRC = sipxCallHold(hCallingParty) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

    bRC = pCallingPartyValidator->waitForCallEvent(hCallingPartyLine, hCallingParty, CALLSTATE_HELD, CALLSTATE_CAUSE_NORMAL) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCallingPartyValidator->waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO);
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCallingPartyValidator->waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO);
    CPPUNIT_ASSERT(bRC) ;

    bRC = pCalledPartyValidator->waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO);
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCalledPartyValidator->waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO);
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCalledPartyValidator->waitForCallEvent(hCalledPartyLine, hCalledParty, CALLSTATE_HELD, CALLSTATE_CAUSE_NORMAL) ;
    CPPUNIT_ASSERT(bRC) ;


    bRC = !pCallingPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = !pCalledPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;


    /*
     * A unholds B
     */
    pCallingPartyValidator->addMarker("A Unholds B") ;
    pCalledPartyValidator->addMarker("A Unholds B") ;

    sipxRC = sipxCallUnhold(hCallingParty) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

    bRC = pCallingPartyValidator->waitForCallEvent(hCallingPartyLine, hCallingParty, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCallingPartyValidator->waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCallingPartyValidator->waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
    CPPUNIT_ASSERT(bRC) ;

    bRC = pCalledPartyValidator->waitForCallEvent(hCalledPartyLine, hCalledParty, CALLSTATE_BRIDGED, CALLSTATE_CAUSE_NORMAL) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCalledPartyValidator->waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_UNHOLD, MEDIA_TYPE_AUDIO);
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCalledPartyValidator->waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_UNHOLD, MEDIA_TYPE_AUDIO);
    CPPUNIT_ASSERT(bRC) ;


    bRC = !pCallingPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = !pCalledPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;


    /*
     * B unholds A
     */
    pCallingPartyValidator->addMarker("B Unholds A") ;
    pCalledPartyValidator->addMarker("B Unholds A") ;

    sipxRC = sipxCallUnhold(hCalledParty) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

    bRC = pCalledPartyValidator->waitForCallEvent(hCalledPartyLine, hCalledParty, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL) ;
    CPPUNIT_ASSERT(bRC) ;

    bRC = !pCallingPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = !pCalledPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;
}

// A calls B, B local holds A, A holds B, A unholds B, B unlocal holds A
void sipXtapiTestSuite::testCallHoldMultiple3() 
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestCallHoldMultiple3 (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        doCallBasic(g_hInst, "sip:calling@127.0.0.1:8000", "sip:calling@127.0.0.1:8000",
                    g_hInst2, "sip:called@127.0.0.1:9100", "sip:called@127.0.0.1:9100", 
                    sipXtapiTestSuite::callMultipleProc3) ;
    }

    OsTask::delay(TEST_DELAY) ;
    checkForLeaks();
}


// B holds A, A local holds B, A unlocal holds B, B unholds A
void sipXtapiTestSuite::callMultipleProc4(
                                  SIPX_CALL hCallingParty, 
                                  SIPX_LINE hCallingPartyLine,
                                  EventValidator* pCallingPartyValidator,
                                  SIPX_CALL hCalledParty,
                                  SIPX_LINE hCalledPartyLine,
                                  EventValidator* pCalledPartyValidator)
{
    bool bRC ;
    SIPX_RESULT sipxRC ;

    pCallingPartyValidator->reset() ;
    pCalledPartyValidator->reset() ;

    /*
     * B holds A
     */
    pCallingPartyValidator->addMarker("B Holds A") ;
    pCalledPartyValidator->addMarker("B Holds A") ;
    sipxRC = sipxCallHold(hCalledParty) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
    
    bRC = pCalledPartyValidator->waitForCallEvent(hCalledPartyLine, hCalledParty, CALLSTATE_HELD, CALLSTATE_CAUSE_NORMAL) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCalledPartyValidator->waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO);
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCalledPartyValidator->waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO);
    CPPUNIT_ASSERT(bRC) ;

    bRC = pCallingPartyValidator->waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO);
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCallingPartyValidator->waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO);
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCallingPartyValidator->waitForCallEvent(hCallingPartyLine, hCallingParty, CALLSTATE_REMOTE_HELD, CALLSTATE_CAUSE_NORMAL) ;
    CPPUNIT_ASSERT(bRC) ;

    bRC = !pCallingPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = !pCalledPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;

    /*
     * A local holds B
     */
    pCallingPartyValidator->addMarker("A local holds B") ;
    pCalledPartyValidator->addMarker("A local holds B") ;

    sipxRC = sipxCallHold(hCallingParty, false) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

    bRC = pCallingPartyValidator->waitForCallEvent(hCallingPartyLine, hCallingParty, CALLSTATE_HELD, CALLSTATE_CAUSE_NORMAL) ;
    CPPUNIT_ASSERT(bRC) ;

    bRC = !pCallingPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = !pCalledPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;


    /*
     * A unholds B
     */
    pCallingPartyValidator->addMarker("A Unholds B") ;
    pCalledPartyValidator->addMarker("A Unholds B") ;

    sipxRC = sipxCallUnhold(hCallingParty) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

    bRC = pCallingPartyValidator->waitForCallEvent(hCallingPartyLine, hCallingParty, CALLSTATE_REMOTE_HELD, CALLSTATE_CAUSE_NORMAL) ;
    CPPUNIT_ASSERT(bRC) ;

    bRC = !pCallingPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = !pCalledPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;


    /*
     * B unholds A
     */
    pCallingPartyValidator->addMarker("B Unholds A") ;
    pCalledPartyValidator->addMarker("B Unholds A") ;

    sipxRC = sipxCallUnhold(hCalledParty) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

    bRC = pCallingPartyValidator->waitForCallEvent(hCallingPartyLine, hCallingParty, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCallingPartyValidator->waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_UNHOLD, MEDIA_TYPE_AUDIO);
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCallingPartyValidator->waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_UNHOLD, MEDIA_TYPE_AUDIO);
    CPPUNIT_ASSERT(bRC) ;

    bRC = pCalledPartyValidator->waitForCallEvent(hCalledPartyLine, hCalledParty, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCalledPartyValidator->waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_UNHOLD, MEDIA_TYPE_AUDIO);
    CPPUNIT_ASSERT(bRC) ;
    bRC = pCalledPartyValidator->waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_UNHOLD, MEDIA_TYPE_AUDIO);
    CPPUNIT_ASSERT(bRC) ;

    bRC = !pCallingPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = !pCalledPartyValidator->validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;
}

// A calls B, B holds A, A local holds B, A unlocal holds B, B unholds A
void sipXtapiTestSuite::testCallHoldMultiple4() 
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestCallHoldMultiple4 (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        doCallBasic(g_hInst, "sip:calling@127.0.0.1:8000", "sip:calling@127.0.0.1:8000",
                    g_hInst2, "sip:called@127.0.0.1:9100", "sip:called@127.0.0.1:9100", 
                    sipXtapiTestSuite::callMultipleProc3) ;
    }

    OsTask::delay(TEST_DELAY) ;
    checkForLeaks();
}


