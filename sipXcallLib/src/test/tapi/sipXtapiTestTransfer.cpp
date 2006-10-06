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

extern SIPX_INST g_hInst;
extern SIPX_INST g_hInst2;
extern SIPX_INST g_hInst3;
extern SIPX_INST g_hInst4;

void sipXtapiTestSuite::testBlindTransferSuccess() 
{
    EventValidator validatorTransferController("testBlindTransferSuccess.validatorTransferController") ;
    EventValidator validatorTransferee("testBlindTransferSuccess.validatorTransferee") ;
    EventValidator validatorTransferTarget("testBlindTransferSuccess.validatorTransferTarget") ;

    SIPX_CALL hTransferee ;
    SIPX_LINE hLine ;
    SIPX_LINE hReceivingLine1 ;
    SIPX_LINE hReceivingLine2 ;

    SIPX_RESULT sipxRC ;
    bool        bRC ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestBlindTransferSuccess (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        validatorTransferController.reset() ;
        validatorTransferee.reset() ;
        validatorTransferTarget.reset() ;

        resetAutoAnswerCallback() ;
        resetAutoAnswerCallback2() ;
        
        sipxRC = sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorTransferController) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorTransferee) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst2, AutoAnswerCallback, NULL) ;        
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst3, UniversalEventValidatorCallback, &validatorTransferTarget) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst3, AutoAnswerCallback2, NULL) ;       
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        // Create Lines
        sipxRC = sipxLineAdd(g_hInst, "sip:foo@127.0.0.1:8000", &hLine) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        bRC = validatorTransferController.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxRC = sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine1, CONTACT_AUTO) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        bRC = validatorTransferee.waitForLineEvent(hReceivingLine1, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxRC = sipxLineAdd(g_hInst3, "sip:foo@127.0.0.1:10000", &hReceivingLine2, CONTACT_AUTO) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        bRC = validatorTransferTarget.waitForLineEvent(hReceivingLine2, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        // Setup call to transfer source (transferee)
        sipxRC = sipxCallCreate(g_hInst, hLine, &hTransferee) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxCallConnect(hTransferee, "sip:foo@127.0.0.1:9100") ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        validatorTransferController.addMarker("Validate Calling Side: Transferee") ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_DIALTONE, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_REMOTE_ALERTING, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        validatorTransferee.addMarker("Validated Called Side: Transferee") ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorTransferController.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorTransferee.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorTransferTarget.validateNoWaitingEvent()) ;

        sipxRC = sipxCallBlindTransfer(hTransferee, "sip:foo@127.0.0.1:10000") ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side: Transfer Target
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_TRANSFER_EVENT, CALLSTATE_CAUSE_TRANSFER_INITIATED) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_BRIDGED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_HELD, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_TRANSFER_EVENT, CALLSTATE_CAUSE_TRANSFER_ACCEPTED) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_TRANSFER_EVENT, CALLSTATE_CAUSE_TRANSFER_RINGING) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_TRANSFER_EVENT, CALLSTATE_CAUSE_TRANSFER_SUCCESS) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Calling Side: Transferee
        bRC = validatorTransferee.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_REMOTE_HELD, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_HELD, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorTransferee.waitForCallEvent(hReceivingLine1, g_hAutoAnswerCallbackCallOther, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_TRANSFER) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(hReceivingLine1, g_hAutoAnswerCallbackCallOther, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(hReceivingLine1, g_hAutoAnswerCallbackCallOther, CALLSTATE_REMOTE_ALERTING, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(hReceivingLine1, g_hAutoAnswerCallbackCallOther, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;


        // Validated Called Side: Transfer Target
        bRC = validatorTransferTarget.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferTarget.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_OFFERING, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferTarget.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_ALERTING, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferTarget.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferTarget.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferTarget.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
      
        // Drop Original Call
        SIPX_CALL hDestroy = hTransferee ;
        sipxRC = sipxCallDestroy(hDestroy) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL) ;        
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL) ;        
        CPPUNIT_ASSERT(bRC) ;

        // Drop Calls
        hDestroy = g_hAutoAnswerCallbackCallOther ;
        sipxRC = sipxCallDestroy(hDestroy) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        bRC = validatorTransferee.waitForCallEvent(hReceivingLine1, g_hAutoAnswerCallbackCallOther, CALLSTATE_BRIDGED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(hReceivingLine1, g_hAutoAnswerCallbackCallOther, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(hReceivingLine1, g_hAutoAnswerCallbackCallOther, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL) ;        
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferTarget.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferTarget.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferTarget.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferTarget.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        CPPUNIT_ASSERT(!validatorTransferController.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorTransferee.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorTransferTarget.validateNoWaitingEvent()) ;

        // Remove Listeners
        sipxRC = sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorTransferController) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorTransferee) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst2, AutoAnswerCallback, NULL) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst3, UniversalEventValidatorCallback, &validatorTransferTarget) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst3, AutoAnswerCallback2, NULL) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        sipxRC = sipxLineRemove(hLine) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxLineRemove(hReceivingLine1) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxLineRemove(hReceivingLine2) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
    }

    OsTask::delay(TEST_DELAY) ;    
    checkForLeaks() ;
}

void sipXtapiTestSuite::testBlindTransferFailureBusy() 
{
    EventValidator validatorTransferController("testBlindTransferFailureBusy.validatorTransferController") ;
    EventValidator validatorTransferee("testBlindTransferFailureBusy.validatorTransferee") ;
    EventValidator validatorTransferTarget("testBlindTransferFailureBusy.validatorTransferTarget") ;

    SIPX_CALL hTransferee ;
    SIPX_LINE hLine ;
    SIPX_LINE hReceivingLine1 ;
    SIPX_LINE hReceivingLine2 ;

    SIPX_RESULT sipxRC ;
    bool        bRC ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestBlindTransferFailureBusy (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        validatorTransferController.reset() ;
        validatorTransferee.reset() ;
        validatorTransferTarget.reset() ;

        resetAutoAnswerCallback() ;
        resetAutoAnswerCallback2() ;
        sipxRC = sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorTransferController) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorTransferee) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst2, AutoAnswerCallback, NULL) ;        
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst3, UniversalEventValidatorCallback, &validatorTransferTarget) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst3, AutoRejectCallback, NULL) ;       
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        // Create Lines
        sipxRC = sipxLineAdd(g_hInst, "sip:foo@127.0.0.1:8000", &hLine) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        bRC = validatorTransferController.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxRC = sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine1, CONTACT_AUTO) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        bRC = validatorTransferee.waitForLineEvent(hReceivingLine1, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxRC = sipxLineAdd(g_hInst3, "sip:foo@127.0.0.1:10000", &hReceivingLine2, CONTACT_AUTO) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        bRC = validatorTransferTarget.waitForLineEvent(hReceivingLine2, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        // Setup call to transfer source (transferee)
        sipxRC = sipxCallCreate(g_hInst, hLine, &hTransferee) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxCallConnect(hTransferee, "blah:foo@127.0.0.1:9100", 0, NULL, false) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side: Transferee
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_DIALTONE, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_REMOTE_ALERTING, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // Validated Called Side: Transferee
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorTransferController.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorTransferee.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorTransferTarget.validateNoWaitingEvent()) ;

        sipxRC = sipxCallBlindTransfer(hTransferee, "sip:foo@127.0.0.1:10000") ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side: Transfer Target
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_TRANSFER_EVENT, CALLSTATE_CAUSE_TRANSFER_INITIATED) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_BRIDGED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_HELD, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_TRANSFER_EVENT, CALLSTATE_CAUSE_TRANSFER_ACCEPTED) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_TRANSFER_EVENT, CALLSTATE_CAUSE_TRANSFER_FAILURE) ;
        CPPUNIT_ASSERT(bRC) ;
        

        // Validate Calling Side: Transferee
        bRC = validatorTransferee.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_REMOTE_HELD, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_HELD, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCallOther, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_TRANSFER) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCallOther, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCallOther, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_BUSY) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCallOther, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validated Called Side: Transfer Target
        bRC = validatorTransferTarget.waitForCallEvent(g_hAutoRejectCallbackLine, g_hAutoRejectCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferTarget.waitForCallEvent(g_hAutoRejectCallbackLine, g_hAutoRejectCallbackCall, CALLSTATE_OFFERING, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferTarget.waitForCallEvent(g_hAutoRejectCallbackLine, g_hAutoRejectCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferTarget.waitForCallEvent(g_hAutoRejectCallbackLine, g_hAutoRejectCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        
        CPPUNIT_ASSERT(!validatorTransferController.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorTransferee.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorTransferTarget.validateNoWaitingEvent()) ;

        // Drop Original Call
        SIPX_CALL hDestroy = hTransferee ;
        sipxRC = sipxCallDestroy(hDestroy) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL) ;        
        CPPUNIT_ASSERT(bRC) ;

        CPPUNIT_ASSERT(!validatorTransferController.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorTransferee.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorTransferTarget.validateNoWaitingEvent()) ;

        // Remove Listeners
        sipxRC = sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorTransferController) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorTransferee) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst2, AutoAnswerCallback, NULL) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst3, UniversalEventValidatorCallback, &validatorTransferTarget) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst3, AutoRejectCallback, NULL) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        sipxRC = sipxLineRemove(hLine) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxLineRemove(hReceivingLine1) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxLineRemove(hReceivingLine2) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
    }

    OsTask::delay(TEST_DELAY) ;    
    checkForLeaks() ;
}



/**
 * This tests a basic transfer between two outbound calls.  The first call 
 * (hCallSource) is transferred to the second outbound call (hSourceTarget).
 *
 */
void sipXtapiTestSuite::testTransferSuccess()
{
    EventValidator validatorSource("testTransferSuccess.source") ;
    EventValidator validatorCalled1("testTransferSuccess.validatorCalled1") ;
    EventValidator validatorCalled2("testTransferSuccess.validatorCalled2") ;

    SIPX_RESULT sipxRC ;
    bool        bRC ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        SIPX_LINE hLine ;           // Local Line definition
        SIPX_CALL hCallSource ;     // Transferee
        SIPX_CALL hCallTarget ;     // Tranfer Target
        SIPX_LINE hReceivingLine1;  // Other side of hCallSource
        SIPX_LINE hReceivingLine2;  // Other side of hCallTarget

        printf("\ntestTransferSuccess (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        validatorSource.reset() ;
        validatorCalled1.reset() ;
        validatorCalled2.reset() ;

        //
        // Setup Listeners
        //
        resetAutoAnswerCallback() ;
        resetAutoAnswerCallback2() ;
        sipxRC = sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorSource) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled1) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst2, AutoAnswerCallback, NULL) ;        
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst3, UniversalEventValidatorCallback, &validatorCalled2) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst3, AutoAnswerCallback2, NULL) ;       
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        // Create Lines
        sipxRC = sipxLineAdd(g_hInst, "sip:foo@127.0.0.1:8000", &hLine) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        bRC = validatorSource.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxRC = sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine1, CONTACT_AUTO) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        bRC = validatorCalled1.waitForLineEvent(hReceivingLine1, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxRC = sipxLineAdd(g_hInst3, "sip:foo@127.0.0.1:10000", &hReceivingLine2, CONTACT_AUTO) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        bRC = validatorCalled2.waitForLineEvent(hReceivingLine2, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        // Setup call to transfer source (transferee)
        sipxRC = sipxCallCreate(g_hInst, hLine, &hCallSource) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxCallConnect(hCallSource, "blah:foo@127.0.0.1:9100") ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side: Transferee
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_DIALTONE, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_REMOTE_ALERTING, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // Validated Called Side: Transferee
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorSource.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Setup call to transfer target (Transfer Target)        
        sipxRC = sipxCallCreate(g_hInst, hLine, &hCallTarget) ;        
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxCallConnect(hCallTarget, "sip:foo@127.0.0.1:10000") ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side: Transfer Target
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_DIALTONE, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_REMOTE_ALERTING, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_BRIDGED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // Validated Called Side: Transfer Target
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_OFFERING, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_ALERTING, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorSource.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;        

        // Kick off transfer
        sipxRC = sipxCallTransfer(hCallSource, hCallTarget) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        validatorSource.setMaxLookhead(16) ;

        // Validate Calling Side
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_TRANSFER_EVENT, CALLSTATE_CAUSE_TRANSFER_INITIATED, false) ;
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_TRANSFER_EVENT, CALLSTATE_CAUSE_TRANSFER_ACCEPTED, false) ;
        CPPUNIT_ASSERT(bRC) ;
        
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_REMOTE_HELD, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_HELD, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorSource.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorSource.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_TRANSFER_EVENT, CALLSTATE_CAUSE_TRANSFER_SUCCESS, false) ;
        CPPUNIT_ASSERT(bRC) ;
        

        // Moved this up a few lines
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate remote Transferee Side (called1)
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_REMOTE_HELD, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_HELD, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCallOther, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_TRANSFER, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCallOther, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCallOther, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;


        // Validate remote Transfer Target Side (called2)
        // Adapting test, moving expected CALLSTATE_CAUSE_NORMAL_HELD event to top
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_REMOTE_HELD, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2Other, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_TRANSFERRED, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2Other, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;        

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorSource.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;        
        
        // Destroy calls.
        SIPX_CALL hDestroy = hCallSource ;
        sipxRC = sipxCallDestroy(hDestroy) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        hDestroy = hCallTarget ;
        sipxRC = sipxCallDestroy(hDestroy) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        // Validate destroy
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;

        // Drop other calls
        hDestroy = g_hAutoAnswerCallbackCallOther ;
        sipxRC = sipxCallDestroy(hDestroy) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        hDestroy = g_hAutoAnswerCallbackCall2Other ;
        sipxRC = sipxCallDestroy(hDestroy) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCallOther, CALLSTATE_BRIDGED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCallOther, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCallOther, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        // Adapting test to new expected CALLSTATE_CAUSE_NORMAL_HELD event
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2Other, CALLSTATE_BRIDGED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2Other, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2Other, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        // Remove Listeners
        sipxRC = sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorSource) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled1) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst2, AutoAnswerCallback, NULL) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst3, UniversalEventValidatorCallback, &validatorCalled2) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst3, AutoAnswerCallback2, NULL) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        sipxRC = sipxLineRemove(hLine) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxLineRemove(hReceivingLine1) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxLineRemove(hReceivingLine2) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

    }

    OsTask::delay(TEST_DELAY) ;    
    checkForLeaks() ;
}

/**
 * This test creates a conference by creating two outbound call legs.  
 * Afterwards, it removes itself from the conference by transferring
 * the first party called to the second party called.
 */
void sipXtapiTestSuite::testTransferConferenceSuccess()
{
    EventValidator validatorSource("testTransferConferenceSuccess.source") ;
    EventValidator validatorCalled1("testTransferConferenceSuccess.validatorCalled1") ;
    EventValidator validatorCalled2("testTransferConferenceSuccess.validatorCalled2") ;

    SIPX_RESULT sipxRC ;
    bool        bRC ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        SIPX_LINE hLine ;           // Local Line definition
        SIPX_CONF hConf ;           // Local Conference
        SIPX_CALL hCallSource ;     // Transferee
        SIPX_CALL hCallTarget ;     // Tranfer Target
        SIPX_LINE hReceivingLine1;  // Other side of hCallSource
        SIPX_LINE hReceivingLine2;  // Other side of hCallTarget

        printf("\ntestTransferConferenceSuccess (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        validatorSource.reset() ;
        validatorCalled1.reset() ;
        validatorCalled2.reset() ;

        //
        // Setup Listeners
        //
        resetAutoAnswerCallback() ;
        resetAutoAnswerCallback2() ;
        sipxRC = sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorSource) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled1) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst2, AutoAnswerCallback, NULL) ;        
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst3, UniversalEventValidatorCallback, &validatorCalled2) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst3, AutoAnswerCallback2, NULL) ;       
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        // Create Lines
        sipxRC = sipxLineAdd(g_hInst, "sip:foo@127.0.0.1:8000", &hLine) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        bRC = validatorSource.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxRC = sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine1, CONTACT_AUTO) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        bRC = validatorCalled1.waitForLineEvent(hReceivingLine1, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxRC = sipxLineAdd(g_hInst3, "sip:foo@127.0.0.1:10000", &hReceivingLine2, CONTACT_AUTO) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        bRC = validatorCalled2.waitForLineEvent(hReceivingLine2, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        // Create Conference
        sipxRC = sipxConferenceCreate(g_hInst, &hConf) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        // Setup call to transfer source (transferee)
        sipxRC = sipxConferenceAdd(hConf, hLine, "sip:foo@127.0.0.1:9100", &hCallSource) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;        

        // Validate Calling Side: Transferee
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_DIALTONE, CALLSTATE_CAUSE_CONFERENCE) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_REMOTE_ALERTING, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // Validated Called Side: Transferee
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorSource.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Setup call to transfer target (Transfer Target)        
        sipxRC = sipxConferenceAdd(hConf, hLine, "sip:foo@127.0.0.1:10000", &hCallTarget) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;        

        // Validate Calling Side: Transfer Target
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_DIALTONE, CALLSTATE_CAUSE_CONFERENCE) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_REMOTE_ALERTING, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // Validated Called Side: Transfer Target
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_OFFERING, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_ALERTING, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorSource.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;        

        // Kick off transfer
        sipxRC = sipxCallTransfer(hCallSource, hCallTarget) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side
        validatorSource.setMaxLookhead(12) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_TRANSFER_EVENT, CALLSTATE_CAUSE_TRANSFER_INITIATED, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_REMOTE_HELD, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_TRANSFER_EVENT, CALLSTATE_CAUSE_TRANSFER_ACCEPTED, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_REMOTE_HELD, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_TRANSFER_EVENT, CALLSTATE_CAUSE_TRANSFER_SUCCESS, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate remote Transferee Side (called1)
        bRC = validatorCalled1.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_REMOTE_HELD, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_HELD, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        // BUG: Line handle should not be 0??
        bRC = validatorCalled1.waitForCallEvent(hReceivingLine1, g_hAutoAnswerCallbackCallOther, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_TRANSFER, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(hReceivingLine1, g_hAutoAnswerCallbackCallOther, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(hReceivingLine1, g_hAutoAnswerCallbackCallOther, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;


        // Validate remote Transfer Target Side (called2)
        bRC = validatorCalled2.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_REMOTE_HELD, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2Other, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_TRANSFERRED, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2Other, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;        

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorSource.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;        
        
        // Destroy calls.
        sipxRC = sipxConferenceDestroy(hConf) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        // Drop other calls
        SIPX_CALL hDestroy = g_hAutoAnswerCallbackCallOther ;
        sipxRC = sipxCallDestroy(hDestroy) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        hDestroy = g_hAutoAnswerCallbackCall2Other ;
        sipxRC = sipxCallDestroy(hDestroy) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        bRC = validatorCalled1.waitForCallEvent(hReceivingLine1, g_hAutoAnswerCallbackCallOther, CALLSTATE_BRIDGED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(hReceivingLine1, g_hAutoAnswerCallbackCallOther, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(hReceivingLine1, g_hAutoAnswerCallbackCallOther, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        // Adapting test for new CALLSTATE_CAUSE_NORMAL_HELD event
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2Other, CALLSTATE_BRIDGED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, true);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2Other, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2Other, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        // Remove Listeners
        sipxRC = sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorSource) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled1) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst2, AutoAnswerCallback, NULL) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst3, UniversalEventValidatorCallback, &validatorCalled2) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst3, AutoAnswerCallback2, NULL) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        sipxRC = sipxLineRemove(hLine) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxLineRemove(hReceivingLine1) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxLineRemove(hReceivingLine2) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

    }

    OsTask::delay(TEST_DELAY) ;    
    checkForLeaks() ;
}

void sipXtapiTestSuite::doConfAddParty(SIPX_INST   hCallingInst,
                                       SIPX_CONF   hCallingConf,
                                       SIPX_LINE   hCallingLine,
                                       SIPX_INST   hCalledInst,
                                       const char* szCalledParty,
                                       const char* szCalledLine,
                                       SIPX_CALL&  hSourceCall,
                                       SIPX_CALL&  hCalledCall,
                                       SIPX_LINE&  hCalledLine) 
{   
    EventValidator validatorCalling("doConfAddParty.calling") ;
    EventValidator validatorCalled("doConfAddParty.called") ;
    bool bRC ;
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

    rc = sipxLineAdd(hCalledInst, szCalledLine, &hCalledLine, CONTACT_LOCAL);
    CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;
    bRC = validatorCalled.waitForLineEvent(hCalledLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;

    /*
     * Initiate Call
     */ 

    rc = sipxConferenceAdd(hCallingConf, hCallingLine, szCalledParty, &hSourceCall) ;
    CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;

    /*
     * Validate events (listener auto-answers)
     */

    // Calling Side
    bRC = validatorCalling.waitForCallEvent(hCallingLine, hSourceCall, CALLSTATE_DIALTONE, CALLSTATE_CAUSE_CONFERENCE, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalling.waitForCallEvent(hCallingLine, hSourceCall, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalling.waitForCallEvent(hCallingLine, hSourceCall, CALLSTATE_REMOTE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCalling.waitForCallEvent(hCallingLine, hSourceCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
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

void sipXtapiTestSuite::doConfTransfer(SIPX_INST hInstTC,           /**< Transfer Controller's instance */
                                       SIPX_CALL hTC,               /**< Transfer Controller */                                       
                                       SIPX_CALL hTC_TT,            /**< Transfer Controller's Target */
                                       SIPX_LINE hLineTC,           /**< Transfer Controller's Line */
                                       SIPX_INST hInstTE,           /**< Transferee's instance */
                                       SIPX_CALL hTE,               /**< Transferee */
                                       SIPX_LINE hLineTE,           /**< Transferee's Line */
                                       SIPX_INST hInstTT,           /**< Transfer Target's instance */
                                       SIPX_CALL hTT,               /**< Transfer Target */
                                       SIPX_LINE hLineTT,           /**< Target Target Line */
                                       bool bTargetOnHold,
                                       SIPX_CALL& hNewCallTT_TE,    /**< New call handle for TT's TE connection */
                                       SIPX_CALL& hNewCallTE_TT)    /**< New call handle for TE's TT connection */

{
    EventValidator validatorTC("doConfTransfer.TC") ;
    EventValidator validatorTE("doConfTransfer.TE") ;
    EventValidator validatorTT("doConfTransfer.TT") ;
    bool bRC ;
    SIPX_RESULT sipxRC ;

    validatorTC.reset() ;
    validatorTE.reset() ;
    validatorTT.reset() ;

    /**
     * The look aheads are cranked up so that we can bunch the events by call 
     * id -- this make it much easier to see/understand the events at the 
     * expensive of missing out-of-order event sequences.  In reality, since
     * multiple calls are touch on each party (TC, TE, TT), races exist in 
     * order anyways.  We could update the infrastructure to check for order
     * purely by call id...
     */
    validatorTC.setMaxLookhead(16) ;
    validatorTE.setMaxLookhead(16) ;
    validatorTT.setMaxLookhead(16) ;

    /*
     * Setup Listeners
     */    
    sipxRC = sipxEventListenerAdd(hInstTC, UniversalEventValidatorCallback, &validatorTC) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

    sipxRC = sipxEventListenerAdd(hInstTE, NewCallDetector1, NULL) ;        
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
    sipxRC = sipxEventListenerAdd(hInstTE, UniversalEventValidatorCallback, &validatorTE) ;        
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

    sipxRC = sipxEventListenerAdd(hInstTT, NewCallDetector2, NULL) ;        
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
    sipxRC = sipxEventListenerAdd(hInstTT, UniversalEventValidatorCallback, &validatorTT) ;        
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

    /**
     * Do Transfer
     */   
    sipxRC = sipxCallTransfer(hTC, hTC_TT) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;    
    
    /**
     * Validate Transfer Controller
     */ 
    // Transferee side
    bRC = validatorTC.waitForCallEvent(hLineTC, hTC, CALLSTATE_TRANSFER_EVENT, CALLSTATE_CAUSE_TRANSFER_INITIATED, false) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorTC.waitForCallEvent(hLineTC, hTC, CALLSTATE_REMOTE_HELD, CALLSTATE_CAUSE_NORMAL, false) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorTC.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, false);
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorTC.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, false);
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorTC.waitForCallEvent(hLineTC, hTC, CALLSTATE_TRANSFER_EVENT, CALLSTATE_CAUSE_TRANSFER_ACCEPTED, false) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorTC.waitForCallEvent(hLineTC, hTC, CALLSTATE_TRANSFER_EVENT, CALLSTATE_CAUSE_TRANSFER_SUCCESS, false) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorTC.waitForCallEvent(hLineTC, hTC, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, false) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorTC.waitForCallEvent(hLineTC, hTC, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, false) ;
    CPPUNIT_ASSERT(bRC) ;

    if (!bTargetOnHold)
    {
        bRC = validatorTC.waitForCallEvent(hLineTC, hTC_TT, CALLSTATE_REMOTE_HELD, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTC.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTC.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;
    }
    bRC = validatorTC.waitForCallEvent(hLineTC, hTC_TT, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, false) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorTC.waitForCallEvent(hLineTC, hTC_TT, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, false) ;
    CPPUNIT_ASSERT(bRC) ;

    /**
     * Validate Transfer Target
     *
     * NOTE: Order here matters -- We validate the original call first so the 
     * NewCallDetector has time to record the newly created call.
     */
    // Original Call
    if (!bTargetOnHold)
    {
        bRC = validatorTT.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTT.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTT.waitForCallEvent(hLineTT, hTT, CALLSTATE_REMOTE_HELD, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
    }
    bRC = validatorTT.waitForCallEvent(hLineTT, hTT, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, false) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorTT.waitForCallEvent(hLineTT, hTT, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, false) ;
    CPPUNIT_ASSERT(bRC) ;

    // New Call with Transferee
    hNewCallTT_TE = g_hNewCallDetectorCall2 ;
    CPPUNIT_ASSERT(g_hNewCallDetectorCall2 != hTT) ;
    CPPUNIT_ASSERT(g_hNewCallDetectorSourceCall2 == hTT) ;
    bRC = validatorTT.waitForCallEvent(hLineTT, hNewCallTT_TE, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_TRANSFERRED, false) ;
    CPPUNIT_ASSERT(bRC) ;
    if (!bTargetOnHold)
    {
        bRC = validatorTT.waitForCallEvent(hLineTT, hNewCallTT_TE, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTT.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTT.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;
    }
    else
    {
        bRC = validatorTT.waitForCallEvent(hLineTT, hNewCallTT_TE, CALLSTATE_HELD, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
    }


    /**
     * Validate Transferee
     *
     * NOTE: Order here matters -- We validate the original call first so the 
     * NewCallDetector has time to record the newly created call.
     */ 
    // Original Call
    bRC = validatorTE.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, false);
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorTE.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, false);
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorTE.waitForCallEvent(hLineTE, hTE, CALLSTATE_REMOTE_HELD, CALLSTATE_CAUSE_NORMAL, false) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorTE.waitForCallEvent(hLineTE, hTE, CALLSTATE_HELD, CALLSTATE_CAUSE_NORMAL, false) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorTE.waitForCallEvent(hLineTE, hTE, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, false) ;
    CPPUNIT_ASSERT(bRC) ;

    // TODO:: Should we auto drop this call in sipXtapi??
    SIPX_CALL hDrop = hTE ;
    sipxRC = sipxCallDestroy(hDrop) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;    

    bRC = validatorTE.waitForCallEvent(hLineTE, hTE, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, false) ;
    CPPUNIT_ASSERT(bRC) ;

    // New Call w/ Target
    hNewCallTE_TT = g_hNewCallDetectorCall1 ; 
    CPPUNIT_ASSERT(g_hNewCallDetectorCall1 != hTE) ;
    CPPUNIT_ASSERT(g_hNewCallDetectorSourceCall1 == hTE) ;

    bRC = validatorTE.waitForCallEvent(hLineTE, hNewCallTE_TT, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_TRANSFER, false) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorTE.waitForCallEvent(hLineTE, hNewCallTE_TT, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL, false) ;
    CPPUNIT_ASSERT(bRC) ;
    
    if (!bTargetOnHold)
    {
        bRC = validatorTE.waitForCallEvent(hLineTE, hNewCallTE_TT, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTE.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTE.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, false);
        CPPUNIT_ASSERT(bRC) ;
    }
    else
    {
        bRC = validatorTE.waitForCallEvent(hLineTE, hNewCallTE_TT, CALLSTATE_REMOTE_HELD, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
    }

    /*
     * Make sure no events are pending
     */
    bRC = !validatorTC.validateNoWaitingEvent(); 
    CPPUNIT_ASSERT(bRC) ;
    bRC = !validatorTE.validateNoWaitingEvent(); 
    CPPUNIT_ASSERT(bRC) ;
    bRC = !validatorTT.validateNoWaitingEvent(); 
    CPPUNIT_ASSERT(bRC) ;


    /*
     * Cleanup
     */        
    sipxRC = sipxEventListenerRemove(hInstTC, UniversalEventValidatorCallback, &validatorTC) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

    sipxRC = sipxEventListenerRemove(hInstTE, NewCallDetector1, NULL) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
    sipxRC = sipxEventListenerRemove(hInstTE, UniversalEventValidatorCallback, &validatorTE) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

    sipxRC = sipxEventListenerRemove(hInstTT, NewCallDetector2, NULL) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
    sipxRC = sipxEventListenerRemove(hInstTT, UniversalEventValidatorCallback, &validatorTT) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
}

void sipXtapiTestSuite::doValidateEmptyConference(SIPX_CONF hConf) 
{
    SIPX_CALL calls[16] ;
    size_t nCalls ;
    SIPX_RESULT sipxRC ;

    sipxRC = sipxConferenceGetCalls(hConf, calls, 16, nCalls) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;   
    CPPUNIT_ASSERT(nCalls == 0) ;
}


void sipXtapiTestSuite::doValidateConference(SIPX_CONF hConf,
                                             SIPX_CALL hCall) 
{
    SIPX_CALL calls[16] ;
    size_t nCalls ;
    SIPX_RESULT sipxRC ;
/*
    sipxRC = sipxConferenceGetCalls(hConf, calls, 16, nCalls) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;   

    printf("doValidateConference hConf=%d, call=%d\n", hConf, hCall) ;
    for (int i=0; i<nCalls; i++)
    {
        printf("Call %d == %d\n", i, calls[i]) ;
    }
*/

    sipxRC = sipxConferenceGetCalls(hConf, calls, 16, nCalls) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;   
    CPPUNIT_ASSERT(nCalls == 1) ;
    CPPUNIT_ASSERT(calls[0] == hCall) ;
}


void sipXtapiTestSuite::doValidateConference(SIPX_CONF hConf,
                                             SIPX_CALL hCall1,
                                             SIPX_CALL hCall2) 
{
    SIPX_CALL calls[16] ;
    size_t nCalls ;
    SIPX_RESULT sipxRC ;
/*
    sipxRC = sipxConferenceGetCalls(hConf, calls, 16, nCalls) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;   

    printf("doValidateConference hConf=%d, call1=%d, call2=%d\n", hConf, hCall1, hCall2) ;
    for (int i=0; i<nCalls; i++)
    {
        printf("Call %d == %d\n", i, calls[i]) ;
    }
*/

    sipxRC = sipxConferenceGetCalls(hConf, calls, 16, nCalls) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;   
    CPPUNIT_ASSERT(nCalls == 2) ;
    CPPUNIT_ASSERT(hCall1 != hCall2) ;
    CPPUNIT_ASSERT(calls[0] != calls[1]) ;
    CPPUNIT_ASSERT( (calls[0] == hCall1) || (calls[0] == hCall2)) ;
    CPPUNIT_ASSERT( (calls[1] == hCall1) || (calls[1] == hCall2)) ;
}


void sipXtapiTestSuite::doRemoveProvisionedLine(SIPX_INST hInst, SIPX_LINE hLine) 
{
    SIPX_RESULT sipxRC ;

    sipxRC = sipxLineRemove(hLine) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;    
}

void sipXtapiTestSuite::doDestroyActiveCalls(SIPX_INST hInst1, 
                                             SIPX_CALL hCall1,
                                             SIPX_CALL hLine1,
                                             bool      bExpectBridgedEvent,
                                             SIPX_INST hInst2,
                                             SIPX_CALL hCall2,
                                             SIPX_LINE hLine2) 
{
    EventValidator validatorCall1("doDestroyActiveCalls.Call1") ;
    EventValidator validatorCall2("doDestroyActiveCalls.Call2") ;

    bool bRC ;
    SIPX_RESULT sipxRC ;

    validatorCall1.reset() ;
    validatorCall2.reset() ;


    /*
     * Setup Listeners
     */    
    sipxRC = sipxEventListenerAdd(hInst1, UniversalEventValidatorCallback, &validatorCall1) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
    sipxRC = sipxEventListenerAdd(hInst2, UniversalEventValidatorCallback, &validatorCall2) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;


    /*
     * Do Hangup on Call 1
     */   
    SIPX_CALL hDestroy1 = hCall1 ;
    sipxRC = sipxCallDestroy(hDestroy1) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;    
    
    /*
     * Validate Call1
     */ 
    if (bExpectBridgedEvent)
    {
        bRC = validatorCall1.waitForCallEvent(hLine1, hCall1, CALLSTATE_BRIDGED, CALLSTATE_CAUSE_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
    }
    bRC = validatorCall1.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, false);
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCall1.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, false);
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCall1.waitForCallEvent(hLine1, hCall1, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, false) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCall1.waitForCallEvent(hLine1, hCall1, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, false) ;
    CPPUNIT_ASSERT(bRC) ;

    /*
     * Validate Call2
     */
    bRC = validatorCall2.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, false);
    CPPUNIT_ASSERT(bRC) ;

    // *** Does not get sent::
    bRC = validatorCall2.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, false);
    CPPUNIT_ASSERT(bRC) ;


    bRC = validatorCall2.waitForCallEvent(hLine2, hCall2, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, false) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = !validatorCall2.validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;
    SIPX_CALL hDestroy2 = hCall2 ;
    sipxRC = sipxCallDestroy(hDestroy2) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
    bRC = validatorCall2.waitForCallEvent(hLine2, hCall2, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, false) ;
    CPPUNIT_ASSERT(bRC) ;

    /*
     * Cleanup
     */
    bRC = !validatorCall1.validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = !validatorCall2.validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;

    sipxRC = sipxEventListenerRemove(hInst1, UniversalEventValidatorCallback, &validatorCall1) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
    sipxRC = sipxEventListenerRemove(hInst2, UniversalEventValidatorCallback, &validatorCall2) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
}


// 1. Party A calls Party B, Party B Confs Party C, Party C Confs Party D (A -> B -> C -> D)
// 2. Party B transfers Party A to Party C (A -> C -> D)
// 3. Party C transfers Party A to Party D (A -> D)
void sipXtapiTestSuite::testChainedConfTransfer1() 
{
    SIPX_RESULT sipxRC ;
    SIPX_CALL   hCallPartyA, hCallPartyB, hCallPartyC, hCallPartyD, hCallPartyB_C, hCallPartyC_D ;
    SIPX_LINE   hLinePartyA, hLinePartyB, hLinePartyC, hLinePartyD ;
    SIPX_CONF   hConfPartyB, hConfPartyC ;   

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestChainedConfTransfer1 (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        /*
         * Setup call from Party A to Party B
         */
        doCallBasicSetup(g_hInst, HINST_ADDRESS, HINST_ADDRESS,
                        g_hInst2, HINST2_ADDRESS, HINST2_ADDRESS, 
                        hCallPartyA, hLinePartyA,
                        hCallPartyB, hLinePartyB) ;

        /*
         * Extend Party B's conf to include Party C
         */ 
        sipxRC = sipxConferenceCreate(g_hInst2, &hConfPartyB) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxConferenceJoin(hConfPartyB, hCallPartyB) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        doConfAddParty(g_hInst2, hConfPartyB, hLinePartyB,
                g_hInst3, HINST3_ADDRESS, HINST3_ADDRESS, 
                hCallPartyB_C, hCallPartyC, hLinePartyC) ;


        /*
         * Extend Party C's conf to include Party D
         */ 
        sipxRC = sipxConferenceCreate(g_hInst3, &hConfPartyC) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxConferenceJoin(hConfPartyC, hCallPartyC) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        doConfAddParty(g_hInst3, hConfPartyC, hLinePartyC,
                g_hInst4, HINST4_ADDRESS, HINST4_ADDRESS, 
                hCallPartyC_D, hCallPartyD, hLinePartyD) ;

        /**
         * Party B transfers Party A to Party C
         */
        SIPX_CALL hNewPartyA_PartyC ;   // Transferee (TE)
        SIPX_CALL hNewPartyC_PartyA ;   // Transfer Target (TT)
        doConfTransfer(g_hInst2, hCallPartyB, hCallPartyB_C, hLinePartyB,
                       g_hInst, hCallPartyA, hLinePartyA, 
                       g_hInst3, hCallPartyC, hLinePartyC, false,
                       hNewPartyC_PartyA, hNewPartyA_PartyC) ;

        doValidateConference(hConfPartyC, hNewPartyC_PartyA, hCallPartyC_D) ;
        doValidateEmptyConference(hConfPartyB) ;
        sipxRC = sipxConferenceDestroy(hConfPartyB) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        /**
         * Party C transfers A to D
         */
        SIPX_CALL hNewPartyA_PartyD ;   // Transferee (TE)
        SIPX_CALL hNewPartyD_PartyA ;   // Transfer Target (TT)
        doConfTransfer(g_hInst3, hNewPartyC_PartyA, hCallPartyC_D, hLinePartyC,
                       g_hInst, hNewPartyA_PartyC, hLinePartyA, 
                       g_hInst4, hCallPartyD, hLinePartyD, false,
                       hNewPartyD_PartyA, hNewPartyA_PartyD) ;

        doValidateEmptyConference(hConfPartyC) ;
        sipxRC = sipxConferenceDestroy(hConfPartyC) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;


        /**
         * Hangup A -> D
         */
        doDestroyActiveCalls(g_hInst, hNewPartyA_PartyD, hLinePartyA, true, g_hInst4, hNewPartyD_PartyA, hLinePartyD) ;
        
        doRemoveProvisionedLine(g_hInst, hLinePartyA) ;
        doRemoveProvisionedLine(g_hInst2, hLinePartyB) ;
        doRemoveProvisionedLine(g_hInst3, hLinePartyC) ;
        doRemoveProvisionedLine(g_hInst4, hLinePartyD) ;
    }

    OsTask::delay(TEST_DELAY) ;    
    checkForLeaks() ;
}


void sipXtapiTestSuite::doDestroyOneCallInConf(SIPX_INST hInstCall,
                                               SIPX_CALL hCallCall,
                                               SIPX_LINE hLineCall,                                               
                                               SIPX_INST hInstConf,                                              
                                               SIPX_CALL hCallConf,
                                               SIPX_LINE hLineConf)
{
    EventValidator validatorCall("doDestroyOneCallInConf.call") ;
    EventValidator validatorConf("doDestroyOneCallInConf.conf") ;

    bool bRC ;
    SIPX_RESULT sipxRC ;

    validatorCall.reset() ;
    validatorConf.reset() ;


    /*
     * Setup Listeners
     */    
    sipxRC = sipxEventListenerAdd(hInstCall, UniversalEventValidatorCallback, &validatorCall) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
    sipxRC = sipxEventListenerAdd(hInstConf, UniversalEventValidatorCallback, &validatorConf) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;


    /*
     * Do Hangup on Call
     */   
    SIPX_CALL hDestroy = hCallCall ;
    sipxRC = sipxCallDestroy(hDestroy) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;    
    
    /*
     * Validate Call
     */ 
    bRC = validatorCall.waitForCallEvent(hLineCall, hCallCall, CALLSTATE_BRIDGED, CALLSTATE_CAUSE_NORMAL, false) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCall.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, false);
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCall.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, false);
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCall.waitForCallEvent(hLineCall, hCallCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, false) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorCall.waitForCallEvent(hLineCall, hCallCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, false) ;
    CPPUNIT_ASSERT(bRC) ;

    /*
     * Validate Conf
     */
    bRC = validatorConf.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, false);
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorConf.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO, false);
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorConf.waitForCallEvent(hLineConf, hCallConf, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, false) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorConf.waitForCallEvent(hLineConf, hCallConf, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, false) ;
    CPPUNIT_ASSERT(bRC) ;

    /*
     * Cleanup
     */
    bRC = !validatorCall.validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = !validatorConf.validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;

    sipxRC = sipxEventListenerRemove(hInstCall, UniversalEventValidatorCallback, &validatorCall) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
    sipxRC = sipxEventListenerRemove(hInstConf, UniversalEventValidatorCallback, &validatorConf) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
}


void sipXtapiTestSuite::doRemoteHoldInConf(SIPX_INST hInstRemote,
                                           SIPX_CALL hCallRemote,
                                           SIPX_LINE hLineRemote,
                                           SIPX_INST hInstConf,
                                           SIPX_CALL hCallConf,
                                           SIPX_LINE hLineConf) 
{
    EventValidator validatorRemote("doRemoteHoldInConf.remote") ;
    EventValidator validatorConf("doRemoteHoldInConf.conf") ;

    bool bRC ;
    SIPX_RESULT sipxRC ;

    validatorRemote.reset() ;
    validatorConf.reset() ;

    /*
     * Setup Listeners
     */    
    sipxRC = sipxEventListenerAdd(hInstRemote, UniversalEventValidatorCallback, &validatorRemote) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
    sipxRC = sipxEventListenerAdd(hInstConf, UniversalEventValidatorCallback, &validatorConf) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;


    /*
     * Do Hold
     */   
    sipxRC = sipxCallHold(hCallRemote, true) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;    
    
    /*
     * Validate Call
     */ 
    bRC = validatorRemote.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, false);
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorRemote.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, false);
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorRemote.waitForCallEvent(hLineRemote, hCallRemote, CALLSTATE_HELD, CALLSTATE_CAUSE_NORMAL, false) ;
    CPPUNIT_ASSERT(bRC) ;

    /*
     * Validate Conf
     */
    bRC = validatorConf.waitForMediaEvent(MEDIA_LOCAL_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, false);
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorConf.waitForMediaEvent(MEDIA_REMOTE_STOP, MEDIA_CAUSE_HOLD, MEDIA_TYPE_AUDIO, false);
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorConf.waitForCallEvent(hLineConf, hCallConf, CALLSTATE_REMOTE_HELD, CALLSTATE_CAUSE_NORMAL, false) ;
    CPPUNIT_ASSERT(bRC) ;
    
    /*
     * Cleanup
     */
    bRC = !validatorRemote.validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = !validatorConf.validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;

    sipxRC = sipxEventListenerRemove(hInstRemote, UniversalEventValidatorCallback, &validatorRemote) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
    sipxRC = sipxEventListenerRemove(hInstConf, UniversalEventValidatorCallback, &validatorConf) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
}


// 1. Party A calls Party B, Party B Confs Party C, Party C Confs Party D (A -> B -> C -> D)
// 2. Party B transfres Party A to Party C (A -> C -> D)
// 3. Party A hangs up (C -> D)
void sipXtapiTestSuite::testChainedConfTransfer2()
{
    SIPX_RESULT sipxRC ;
    SIPX_CALL   hCallPartyA, hCallPartyB, hCallPartyC, hCallPartyD, hCallPartyB_C, hCallPartyC_D ;
    SIPX_LINE   hLinePartyA, hLinePartyB, hLinePartyC, hLinePartyD ;
    SIPX_CONF   hConfPartyB, hConfPartyC ;   

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestChainedConfTransfer2 (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        /*
         * Setup call from Party A to Party B
         */
        doCallBasicSetup(g_hInst, HINST_ADDRESS, HINST_ADDRESS,
                        g_hInst2, HINST2_ADDRESS, HINST2_ADDRESS, 
                        hCallPartyA, hLinePartyA,
                        hCallPartyB, hLinePartyB) ;

        /*
         * Extend Party B's conf to include Party C
         */ 
        sipxRC = sipxConferenceCreate(g_hInst2, &hConfPartyB) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxConferenceJoin(hConfPartyB, hCallPartyB) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        doConfAddParty(g_hInst2, hConfPartyB, hLinePartyB,
                g_hInst3, HINST3_ADDRESS, HINST3_ADDRESS, 
                hCallPartyB_C, hCallPartyC, hLinePartyC) ;

        /*
         * Extend Party C's conf to include Party D
         */ 
        sipxRC = sipxConferenceCreate(g_hInst3, &hConfPartyC) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxConferenceJoin(hConfPartyC, hCallPartyC) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        doConfAddParty(g_hInst3, hConfPartyC, hLinePartyC,
                g_hInst4, HINST4_ADDRESS, HINST4_ADDRESS, 
                hCallPartyC_D, hCallPartyD, hLinePartyD) ;

        /**
         * Party B transfers Party A to Party C
         */
        SIPX_CALL hNewPartyA_PartyC ;   // Transferee (TE)
        SIPX_CALL hNewPartyC_PartyA ;   // Transfer Target (TT)
        doConfTransfer(g_hInst2, hCallPartyB, hCallPartyB_C, hLinePartyB,
                       g_hInst, hCallPartyA, hLinePartyA, 
                       g_hInst3, hCallPartyC, hLinePartyC, false,
                       hNewPartyC_PartyA, hNewPartyA_PartyC) ;

        doValidateConference(hConfPartyC, hNewPartyC_PartyA, hCallPartyC_D) ;
        doValidateEmptyConference(hConfPartyB) ;
        sipxRC = sipxConferenceDestroy(hConfPartyB) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;


        /**
         * Hang up Party A 
         */
        doDestroyOneCallInConf(g_hInst, hNewPartyA_PartyC, hLinePartyA,
                               g_hInst3, hNewPartyC_PartyA, hLinePartyC) ;
        doValidateConference(hConfPartyC, hCallPartyC_D) ;
                                                                            

        /**
         * Hangup C -> D
         */
        doDestroyActiveCalls(g_hInst3, hCallPartyC_D, hLinePartyC, false, g_hInst4, hCallPartyD, hLinePartyD) ;
        doValidateEmptyConference(hConfPartyC) ;
        sipxRC = sipxConferenceDestroy(hConfPartyC) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        
        doRemoveProvisionedLine(g_hInst, hLinePartyA) ;
        doRemoveProvisionedLine(g_hInst2, hLinePartyB) ;
        doRemoveProvisionedLine(g_hInst3, hLinePartyC) ;
        doRemoveProvisionedLine(g_hInst4, hLinePartyD) ;
    }

    OsTask::delay(TEST_DELAY) ;    
    checkForLeaks() ;
}

// Take remote entity off hold -- assumes local party is on REMOTE_HOLD
void sipXtapiTestSuite::doRemoteOffHold(SIPX_INST hInstRemote,
                                        SIPX_CALL hCallRemote,
                                        SIPX_LINE hLineRemote,
                                        SIPX_INST hInstLocal,
                                        SIPX_CALL hCallLocal,
                                        SIPX_LINE hLineLocal)
{
    EventValidator validatorRemote("doRemoteOffHold.remote") ;
    EventValidator validatorLocal("doRemoteOffHold.local") ;

    bool bRC ;
    SIPX_RESULT sipxRC ;

    validatorRemote.reset() ;
    validatorLocal.reset() ;

    /*
     * Setup Listeners
     */    
    sipxRC = sipxEventListenerAdd(hInstRemote, UniversalEventValidatorCallback, &validatorRemote) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
    sipxRC = sipxEventListenerAdd(hInstLocal, UniversalEventValidatorCallback, &validatorLocal) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

    /*
     * Unhold
     */
    sipxRC = sipxCallUnhold(hCallRemote) ;
    assert(sipxRC == SIPX_RESULT_SUCCESS) ;

    /*
     * Validate Events
     */
    bRC = validatorRemote.waitForCallEvent(hLineRemote, hCallRemote, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorRemote.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO);
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorRemote.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_NORMAL, MEDIA_TYPE_AUDIO);
    CPPUNIT_ASSERT(bRC) ;

    bRC = validatorLocal.waitForCallEvent(hLineLocal, hCallLocal, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL) ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorLocal.waitForMediaEvent(MEDIA_LOCAL_START, MEDIA_CAUSE_UNHOLD, MEDIA_TYPE_AUDIO);
    CPPUNIT_ASSERT(bRC) ;
    bRC = validatorLocal.waitForMediaEvent(MEDIA_REMOTE_START, MEDIA_CAUSE_UNHOLD, MEDIA_TYPE_AUDIO);
    CPPUNIT_ASSERT(bRC) ;

    /*
     * Cleanup
     */
    bRC = !validatorRemote.validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;
    bRC = !validatorLocal.validateNoWaitingEvent() ;
    CPPUNIT_ASSERT(bRC) ;

    sipxRC = sipxEventListenerRemove(hInstRemote, UniversalEventValidatorCallback, &validatorRemote) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
    sipxRC = sipxEventListenerRemove(hInstLocal, UniversalEventValidatorCallback, &validatorLocal) ;
    CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
}


// Party A calls Party B, Party B conferences Party C (A -> B -> C)
// Party C placed call on full hold (A -> B -> C/h)
// Party B tranfers Party A to Party C (A -> C/h)
// NOTE: B should send a 200 OK response to A with 0.0.0.0 in the SDP
void sipXtapiTestSuite::testChainedConfTransfer3()
{
    SIPX_RESULT sipxRC ;
    SIPX_CALL   hCallPartyA, hCallPartyB, hCallPartyC, hCallPartyB_C ;
    SIPX_LINE   hLinePartyA, hLinePartyB, hLinePartyC ;
    SIPX_CONF   hConfPartyB ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestChainedConfTransfer3 (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        /*
         * Setup call from Party A to Party B
         */
        doCallBasicSetup(g_hInst, HINST_ADDRESS, HINST_ADDRESS,
                g_hInst2, HINST2_ADDRESS, HINST2_ADDRESS, 
                hCallPartyA, hLinePartyA,
                hCallPartyB, hLinePartyB) ;

        /*
         * Extend Party B's conf to include Party C
         */ 
        sipxRC = sipxConferenceCreate(g_hInst2, &hConfPartyB) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxConferenceJoin(hConfPartyB, hCallPartyB) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        doConfAddParty(g_hInst2, hConfPartyB, hLinePartyB,
                g_hInst3, HINST3_ADDRESS, HINST3_ADDRESS, 
                hCallPartyB_C, hCallPartyC, hLinePartyC) ;

        /*
         * Place C on Hold
         */
        doRemoteHoldInConf(g_hInst3, hCallPartyC, hLinePartyC,
                           g_hInst2, hCallPartyB_C, hLinePartyB) ;

       
        /**
         * Party B transfers Party A to Party C
         */
        SIPX_CALL hNewPartyA_PartyC ;   // Transferee (TE)
        SIPX_CALL hNewPartyC_PartyA ;   // Transfer Target (TT)
        doConfTransfer(g_hInst2, hCallPartyB, hCallPartyB_C, hLinePartyB,
                       g_hInst, hCallPartyA, hLinePartyA, 
                       g_hInst3, hCallPartyC, hLinePartyC, true,
                       hNewPartyC_PartyA, hNewPartyA_PartyC) ;

        doValidateEmptyConference(hConfPartyB) ;
        sipxRC = sipxConferenceDestroy(hConfPartyB) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        /**
         * Take C off hold
         */
        doRemoteOffHold(g_hInst3, hNewPartyC_PartyA, hLinePartyC,
                        g_hInst, hNewPartyA_PartyC, hLinePartyA) ;
                        
                                                                            
        /**
         * Hangup C -> D
         */
        doDestroyActiveCalls(g_hInst3, hNewPartyC_PartyA, hLinePartyC, true, g_hInst, hNewPartyA_PartyC, hLinePartyA) ;
        
        doRemoveProvisionedLine(g_hInst, hLinePartyA) ;
        doRemoveProvisionedLine(g_hInst2, hLinePartyB) ;
        doRemoveProvisionedLine(g_hInst3, hLinePartyC) ;
    }

    OsTask::delay(TEST_DELAY) ;
    checkForLeaks() ;
}


// 1. Party A calls Party B, Party B Confs Party C, Party C Confs Party D (A -> B -> C -> D)
// 2. Party C transfers Party B to Party D (A -> B -> D)
// 3. Party D hangs up (A -> B)

// 1. Party A calls Party B, Party B Confs Party C, Party C Confs Party D (A -> B -> C -> D)
// 2. Party C transfers Party B to Party D (A -> B -> D)
// 3. Party B transfers Party A to Party D (A -> D)

