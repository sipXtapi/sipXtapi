//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _SIPXTAPITEST_H
#define _SIPXTAPITEST_H

#ifdef _WIN32
// #define SIPX_TEST_FOR_MEMORY_LEAKS
#ifdef SIPX_TEST_FOR_MEMORY_LEAKS
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif
#endif

#include <cppunit/extensions/HelperMacros.h>

#include "utl/UtlSList.h"
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"
#include "tapi/sipXtapiInternal.h"
#include "TestStunServerTask.h"

// Defines
#define CALL_DELAY      500     /**< Delay where we need to wait for something in ms */
#define STRESS_FACTOR   3       /**< How many times to repeat each test case */
#define TEST_DELAY      500     /**< Time to delay between each test */

#define STUN_SERVER_PORT_1      3478
#define STUN_SERVER_PORT_2      3479
#define STUN_SERVER_PORT_3      3480
#define STUN_SERVER_PORT_4      3481

#define HINST_ADDRESS       "sip:hinst@127.0.0.1:8000" 
#define HINST2_ADDRESS      "sip:hinst2@127.0.0.1:9100" 
#define HINST3_ADDRESS      "sip:hinst3@127.0.0.1:10000" 
#define HINST4_ADDRESS      "sip:hinst4@127.0.0.1:12070" 

#ifdef _WIN32
  #define TEST_AUDIO              1
  #define TEST_LINE               1
  #define TEST_CALL               1
  #define TEST_CALL_HOLD          1
  #define TEST_CONF               1
  #define TEST_REG                1
  #define TEST_TRANSFER           1
  #define TEST_TRANSFER_ADVANCED  1
  #define TEST_CONFIG             1
  #define TEST_SUBSCRIBE          1
  #define TEST_NAT                1
  #define TEST_PROBLEMATIC_CASES  0
#else
  #define TEST_AUDIO              0
  #define TEST_LINE               0
  #define TEST_CALL               0
  #define TEST_CALL_HOLD          0
  #define TEST_CONF               0
  #define TEST_REG                0
  #define TEST_TRANSFER           0
  #define TEST_TRANSFER_ADVANCED  0
  #define TEST_CONFIG             0
  #define TEST_SUBSCRIBE          0
  #define TEST_NAT                0
  #define TEST_PROBLEMATIC_CASES  0
#endif /* _WIN32 */

typedef void (*ADDITIONALCALLTESTPROC)(SIPX_CALL hCallingParty, 
                                  SIPX_LINE hCallingPartyLine,
                                  EventValidator* pCallingPartyValidator,
                                  SIPX_CALL hCalledParty,
                                  SIPX_LINE hCalledPartyLine,
                                  EventValidator* pCalledPartyValidator) ;


bool SIPX_CALLING_CONVENTION basicCall_CallBack_Receive(SIPX_EVENT_CATEGORY category, 
                                         void* pInfo, 
                                         void* pUserData);
bool SIPX_CALLING_CONVENTION basicCall_CallBack_Receive3(SIPX_EVENT_CATEGORY category, 
                                         void* pInfo, 
                                         void* pUserData);
                                         
bool SIPX_CALLING_CONVENTION basicCall_CallBack_Place(SIPX_EVENT_CATEGORY category, 
                                         void* pInfo, 
                                         void* pUserData);                                         

bool SIPX_CALLING_CONVENTION basicCall_CallBack_Receive3_hangup(SIPX_EVENT_CATEGORY category, 
                                         void* pInfo, 
                                         void* pUserData);

bool SIPX_CALLING_CONVENTION basicCall_CallBack_Receive3_busy(SIPX_EVENT_CATEGORY category, 
                                         void* pInfo, 
                                         void* pUserData);

class sipXtapiTestSuite : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(sipXtapiTestSuite) ;

#ifdef SIPX_TEST_FOR_MEMORY_LEAKS
    CPPUNIT_TEST(testNothing) ;
#endif

#if TEST_AUDIO /* [ */     
    CPPUNIT_TEST(testGainAPI) ;
    CPPUNIT_TEST(testMuteAPI) ;
    CPPUNIT_TEST(testVolumeAPI) ;
    CPPUNIT_TEST(testAudioSettings);
#endif /* TEST_AUDIO ] */

#if TEST_LINE /* [ */
    CPPUNIT_TEST(testLineAPI_Add) ;
    CPPUNIT_TEST(testLineAPI_Remove) ;
    CPPUNIT_TEST(testLineAPI_Credential) ;
    CPPUNIT_TEST(testLineAPI_Get) ;
    CPPUNIT_TEST(testLineAPI_GetURI) ;
    CPPUNIT_TEST(testLines) ;
    CPPUNIT_TEST(testLineEvents);
    CPPUNIT_TEST(testLineAliases);
#endif /* TEST_LINE ] */

#if TEST_CALL /* [ */
    CPPUNIT_TEST(testCallMakeAPI) ;
    CPPUNIT_TEST(testCallGetID) ;
    CPPUNIT_TEST(testCallGetRemoteID) ;
    CPPUNIT_TEST(testCallGetLocalID) ;
    CPPUNIT_TEST(testCallCancel) ;
    CPPUNIT_TEST(testCallBasic) ;
    CPPUNIT_TEST(testCallBasic2) ;  
    CPPUNIT_TEST(testCallBasicTCP);
    CPPUNIT_TEST(testCallBasicNoRtcp) ;

#if 0
    // Per Mike Cohen on May/2006 -- doesn't pass because 
    // audio file isn't availabe -- remove ifdef when 
    // available
    CPPUNIT_TEST(testCallPlayAudioFile);
#endif
#ifdef _WIN32
    CPPUNIT_TEST(testCallMute);
#endif
    //CPPUNIT_TEST(testCallBusy) ;
    CPPUNIT_TEST(testCallRedirect);
    CPPUNIT_TEST(testCallShutdown) ;
    CPPUNIT_TEST(testCallShutdown) ;
    CPPUNIT_TEST(testCallShutdown) ;
    CPPUNIT_TEST(testCallShutdown) ;
    CPPUNIT_TEST(testCallShutdown) ;    
    CPPUNIT_TEST(testSendInfo);
    CPPUNIT_TEST(testSendInfoExternalTransport);
    CPPUNIT_TEST(testSendInfoTimeout);
    CPPUNIT_TEST(testSendInfoFailure);
    CPPUNIT_TEST(testCallDestroyRinging);
    CPPUNIT_TEST(testCallGetRemoteUserAgent);

    /*CPPUNIT_TEST(testCallBasicSecure);
    CPPUNIT_TEST(testCallHoldSecure);
    CPPUNIT_TEST(testCallSecurityCallerUnsupported);
    CPPUNIT_TEST(testCallSecurityCalleeUnsupported);
    CPPUNIT_TEST(testCallSecurityCalleeStepUp);
    CPPUNIT_TEST(testCallSecurityBadParams);*/

    // 
    // The following test cases allow you to manually test features and
    // verify that audio works.  You will need to modify the IP address 
    // to work for you.
    //
    // CPPUNIT_TEST(testManualCallDialtone) ;
    // CPPUNIT_TEST(testManualPlayFileNoCall);
#endif /* TEST_CALL ] */

#if TEST_CALL_HOLD /* [ */
    CPPUNIT_TEST(testCallHold) ;
    CPPUNIT_TEST(testCallHoldTCP) ;
    CPPUNIT_TEST(testCallHoldMultiple1) ;
    CPPUNIT_TEST(testCallHoldMultiple2) ;
     CPPUNIT_TEST(testCallHoldMultiple3) ;
    CPPUNIT_TEST(testCallHoldMultiple4) ;
    CPPUNIT_TEST(testCallHoldExceedingIdleTimeout);
#endif /* TEST_CALL_HOLD ] */
        
#if TEST_CONF /* [ */
    CPPUNIT_TEST(testConfBasic1) ;
    CPPUNIT_TEST(testConfBasic2) ;
    CPPUNIT_TEST(testConfBasic3) ;
    CPPUNIT_TEST(testConfBasic4) ;
    CPPUNIT_TEST(testConfBasic5) ;
    CPPUNIT_TEST(testConfBasic6) ;    
    CPPUNIT_TEST(testConfHoldIndividual) ;
    CPPUNIT_TEST(testConfJoin) ;
    CPPUNIT_TEST(testConfHoldNoBridge) ;
    CPPUNIT_TEST(testConfHoldBridge) ;
    CPPUNIT_TEST(testConfReAdd) ;
    CPPUNIT_TEST(testConferenceLegBusy);
    
//    CPPUNIT_TEST(testConferenceDisplayName);

    // 
    // The following test cases allow you to manually test join/split and
    // verify audio -- you will need to modify the IP address to work
    // for you.
    //

    // CPPUNIT_TEST(testManualConfBridge) ;
    // CPPUNIT_TEST(testManualConfSplit) ;
    // CPPUNIT_TEST(testManualConfJoin) ;
#endif /* TEST_CONF ] */

#if TEST_REG /* [ */
    CPPUNIT_TEST(testReRegistrationFailure);
    CPPUNIT_TEST(testRegistration);
    CPPUNIT_TEST(testReRegistration);
    CPPUNIT_TEST(testBadRegistrarRegistration);
#endif /* TEST_REG ] */

#if TEST_TRANSFER /* [ */
    CPPUNIT_TEST(testBlindTransferSuccess) ;
    CPPUNIT_TEST(testBlindTransferFailureBusy) ;
    CPPUNIT_TEST(testTransferSuccess) ;
    CPPUNIT_TEST(testTransferConferenceSuccess) ;
#endif /* TEST_TRANSFER ] */

#if TEST_TRANSFER_ADVANCED /* [ */
    CPPUNIT_TEST(testChainedConfTransfer1) ;
    CPPUNIT_TEST(testChainedConfTransfer2) ;
    CPPUNIT_TEST(testChainedConfTransfer3) ;
#endif /* TEST_TRANSFER_ADVANCED ] */

#if TEST_CONFIG /* [ */
    CPPUNIT_TEST(testGetVersion) ;
    CPPUNIT_TEST(testSeqPortSelection) ;    
    CPPUNIT_TEST(testAutoPortSelection) ;
    CPPUNIT_TEST(testSetCallback) ;   
#ifdef _WIN32
#ifdef VOICE_ENGINE
    CPPUNIT_TEST(testConfigOutOfBand) ;
#endif
#endif
    CPPUNIT_TEST(testConfigLog) ;
    CPPUNIT_TEST(testConfigEnableShortNames);
    CPPUNIT_TEST(testTeardown);
    CPPUNIT_TEST(testTeardown);
    CPPUNIT_TEST(testTeardown);
    CPPUNIT_TEST(testTeardown);
    CPPUNIT_TEST(testTeardown);
    CPPUNIT_TEST(testReinitializeSimple);
    CPPUNIT_TEST(testReinitializeCall);
    CPPUNIT_TEST(testReinitializeLine);    
    CPPUNIT_TEST(testReinitializeConference);
    CPPUNIT_TEST(testReinitializePub);
    CPPUNIT_TEST(testReinitializeSub);

    CPPUNIT_TEST(testConfigExternalTransport);
#ifdef VOICE_ENGINE /* [ */
//    CPPUNIT_TEST(testConfigCodecPreferences);
#endif /* VOICE_ENGINE ] */

#endif /* TEST_CONFIG ] */

#if TEST_SUBSCRIBE /* [ */ 
    // CPPUNIT_TEST(testPublishAndSubscribeCall); 
    CPPUNIT_TEST(testPublishAndSubscribeConfig); 
#endif /* TEST_SUBSCRIBE ] */ 

#if TEST_NAT /* [ */
    CPPUNIT_TEST(testConfigEnableStunSuccess);
    CPPUNIT_TEST(testConfigEnableStunNoResponse);
    CPPUNIT_TEST(testConfigEnableStunError);
    CPPUNIT_TEST(testConfigEnableStunDropOdd);
    CPPUNIT_TEST(testConfigEnableStunDropEven);
    CPPUNIT_TEST(testConfigEnableStunDelay);
    CPPUNIT_TEST(testConfigCrlfKeepAlive);
    CPPUNIT_TEST(testConfigStunKeepAlive);
    CPPUNIT_TEST(testConfigStunKeepAliveOnce);
    CPPUNIT_TEST(testConfigKeepAliveNoStop) ;
#endif  /* TEST_NAT ] */

#ifdef TEST_PROBLEMATIC_CASES /* [ */
    void testCallRapidCallAndHangup();
#endif /* TEST_PROBLEMATIC_CASES ] */

CPPUNIT_TEST_SUITE_END() ;

public:
    sipXtapiTestSuite();

    void setUp() ;
    void tearDown() ;

    void setupStunServer() ;
    void teardownStunServer() ;
    void setStunServerMode(STUN_TEST_MODE mode) ;
    void setStunServerValidator(EventValidator* pValidator) ;


    void testNothing() ;

    void testGainAPI() ;
    void testMuteAPI() ;
    void testVolumeAPI() ;
    void testAudioSettings();

    void testLineAPI_Add() ;
    void testLineAPI_Remove() ;
    void testLineAPI_Credential() ;
    void testLineAPI_Get() ;
    void testLineAPI_GetURI() ;

    void testCallMakeAPI() ;
    void testCallGetID() ;
    void testCallGetRemoteID() ;
    void testCallGetLocalID() ;
    
    void doCallBasic(SIPX_INST   hCallingInst,
                     const char* szCallingParty,
                     const char* szCallingLine,
                     SIPX_INST   hCalledInst,
                     const char* szCalledParty,
                     const char* szCalledLine,
                     ADDITIONALCALLTESTPROC pAdditionalProc,
					 bool bDisableRtcp = false);

    void doCallBasicSetup(SIPX_INST   hCallingInst,
                          const char* szCallingParty,
                          const char* szCallingLine,
                          SIPX_INST   hCalledInst,
                          const char* szCalledParty,
                          const char* szCalledLine,
                          SIPX_CALL&  hCallingCall,
                          SIPX_LINE&  hCallingLine,
                          SIPX_CALL&  hCalledCall,
                          SIPX_LINE&  hCalledLine) ;

    void doConfAddParty(SIPX_INST   hCallingInst,
                        SIPX_CONF   hCallingConf,
                        SIPX_LINE   hCallingLine,
                        SIPX_INST   hCalledInst,
                        const char* szCalledParty,
                        const char* szCalledLine,
                        SIPX_CALL&  hSourceCall,
                        SIPX_CALL&  hCalledCall,
                        SIPX_LINE&  hCalledLine) ;


    void doValidateEmptyConference(SIPX_CONF hConf) ;

    void doValidateConference(SIPX_CONF hConf,
                              SIPX_CALL hCall) ;    

    void doValidateConference(SIPX_CONF hConf,
                              SIPX_CALL hCall1,
                              SIPX_CALL hCall2) ;


    void doConfTransfer(SIPX_INST  hInstTC,          /**< Transfer Controller's instance */
                        SIPX_CALL  hTC,              /**< Transfer Controller */                        
                        SIPX_CALL  hTC_TT,           /**< Transfer Controller's Target */
                        SIPX_LINE  hLineTC,          /**< Transfer Controller's Line */
                        SIPX_INST  hInstTE,          /**< Transferee's instance */
                        SIPX_CALL  hTE,              /**< Transferee */
                        SIPX_LINE  hLineTE,          /**< Transferee's Line */
                        SIPX_INST  hInstTT,          /**< Transfer Target's instance */
                        SIPX_CALL  hTT,              /**< Transfer Target */
                        SIPX_LINE  hLineTT,          /**< Target Target Line */
                        bool       bTargetOnHold,
                        SIPX_CALL& hNewCallTT_TE,    /**< New call handle for TT's TE connection */
                        SIPX_CALL& hNewCallTE_TT) ;  /**< New call handle for TE's TT connection */

    void doRemoveProvisionedLine(SIPX_INST hInst, 
                                 SIPX_LINE hLine) ;

    void doDestroyActiveCalls(SIPX_INST hInst1, 
                              SIPX_CALL hCall1,
                              SIPX_CALL hLine1,
                              bool      bExpectBridgedEvent,
                              SIPX_INST hInst2,
                              SIPX_CALL hCall2,
                              SIPX_LINE hLine2) ;

    void doDestroyOneCallInConf(SIPX_INST hInstCall,                                
                                SIPX_CALL hCallCall,
                                SIPX_LINE hLineCall,
                                SIPX_INST hInstConf,                                
                                SIPX_CALL hCallConf,
                                SIPX_LINE hLineConf) ;

    void doRemoteHoldInConf(SIPX_INST hInstRemote,
                            SIPX_CALL hCallRemote,
                            SIPX_LINE hLineRemote,
                            SIPX_INST hInstConf,
                            SIPX_CALL hCallConf,
                            SIPX_LINE hLineConf) ;

    void doRemoteOffHold(SIPX_INST hInstRemote,
                         SIPX_CALL hCallRemote,
                         SIPX_LINE hLineRemote,
                         SIPX_INST hInstLocal,
                         SIPX_CALL hCallLocal,
                         SIPX_LINE hLineLocal) ;

    void testCallBasic() ;
    void testCallBasicTCP();
    void testCallBasicNoRtcp() ;
    void testCallBasic2() ;
    void testCallDestroyRinging();
    void testCallPlayAudioFile();
    
    void testCallBusy() ;
    void testCallHold() ;
    void testCallHoldX(bool bTcp) ;
    void testCallHoldTCP() ;
    void testCallHoldMultiple1() ;
    void testCallHoldMultiple2() ;
    void testCallHoldMultiple3() ;
    void testCallHoldMultiple4() ;
    void testCallHoldExceedingIdleTimeout();
    void testCallMute();
    void testCallRedirect();
    void testCallShutdown();
    void testCallCancel();
    void testCallGetRemoteUserAgent();
    void testCallBasicSecure();
    void testCallHoldSecure();
    void testCallSecurityCallerUnsupported();
    void testCallSecurityCalleeUnsupported();
    void testCallSecurityCalleeStepUp();
    void testCallSecurityBadParams();

    void testManualCallDialtone() ;

    void testConfBasic1() ;
    void testConfBasic2() ;
    void testConfBasic3() ;
    void testConfBasic4() ;
    void testConfBasic5() ;
    void testConfBasic6() ;
    void testConfJoin() ;
    void testConfHoldIndividual() ;    
    void testConfHoldNoBridge() ;
    void testConfHoldBridge() ;
    void testConfReAdd() ;
    void testConferenceDisplayName();
    void testManualConfSplit() ;
    void testManualConfJoin() ;
    void testManualConfBridge() ;
    void testConferenceLegBusy();

    void testLines() ;
    void testLineEvents();
    void testLineAliases();
    void testRegistration();
    void testBadRegistrarRegistration();
    void testReRegistration();
    void testReRegistrationFailure();

    void testBlindTransferSuccess() ;
    void testBlindTransferFailureBusy() ;
    void testTransferSuccess() ;
    void testTransferConferenceSuccess() ;

    void testChainedConfTransfer1() ;
    void testChainedConfTransfer2() ;
    void testChainedConfTransfer3() ;

    void testGetVersion() ;
    void testSendInfo();
    void testSendInfoExternalTransport();
    void testSendInfoFailure();
    void testSendInfoTimeout();
    void testSetCallback();

    void testAutoPortSelection() ;
    void testSeqPortSelection() ;
    void testConfigExternalTransport();
    void testConfigLog() ;
    void testConfigOutOfBand() ;
    void testTeardown() ;
    void testReinitializeSimple() ;
    void testReinitializeLine() ;
    void testReinitializeCall() ;
    void testReinitializeConference() ;
    void testReinitializePub() ;
    void testReinitializeSub() ;

    void testConfigCodecPreferences() ;

    void testConfigEnableStunSuccess() ;
    void testConfigEnableStunNoResponse() ;
    void testConfigEnableStunError() ;
    void testConfigEnableStunDropOdd() ;
    void testConfigEnableStunDropEven() ;
    void testConfigEnableStunDelay() ;
    void testConfigCrlfKeepAlive() ;
    void testConfigStunKeepAlive() ;
    void testConfigStunKeepAliveOnce() ;
    void testConfigKeepAliveNoStop() ;

    void testConfigEnableShortNames();
    
    void testPublishAndSubscribe(bool bCallContext, const char* szTestName);
    void testPublishAndSubscribeCall();
    void testPublishAndSubscribeConfig();

    void testCallRapidCallAndHangup();
protected:
    static void callMultipleProc1(SIPX_CALL hCallingParty, 
                                  SIPX_LINE hCallingPartyLine,
                                  EventValidator* pCallingPartyValidator,
                                  SIPX_CALL hCalledParty,
                                  SIPX_LINE hCalledPartyLine,
                                  EventValidator* pCalledPartyValidator);

    static void callMultipleProc2(SIPX_CALL hCallingParty, 
                                  SIPX_LINE hCallingPartyLine,
                                  EventValidator* pCallingPartyValidator,
                                  SIPX_CALL hCalledParty,
                                  SIPX_LINE hCalledPartyLine,
                                  EventValidator* pCalledPartyValidator);

    static void callMultipleProc3(SIPX_CALL hCallingParty, 
                                  SIPX_LINE hCallingPartyLine,
                                  EventValidator* pCallingPartyValidator,
                                  SIPX_CALL hCalledParty,
                                  SIPX_LINE hCalledPartyLine,
                                  EventValidator* pCalledPartyValidator);

    static void callMultipleProc4(SIPX_CALL hCallingParty, 
                                  SIPX_LINE hCallingPartyLine,
                                  EventValidator* pCallingPartyValidator,
                                  SIPX_CALL hCalledParty,
                                  SIPX_LINE hCalledPartyLine,
                                  EventValidator* pCalledPartyValidator);

    void createCall(SIPX_LINE hLine, SIPX_CALL* phCall) ;
    void destroyCall(SIPX_CALL& hCall) ;

    void createCall(SIPX_LINE* phLine, SIPX_CALL* phCall) ;
    void destroyCall(SIPX_LINE& hLine, SIPX_CALL& hCall) ;

    void checkForLeaks();
    void checkForCallLeaks(SIPX_INST hInst) ;
private:
#ifdef _WIN32
#ifdef SIPX_TEST_FOR_MEMORY_LEAKS
    _CrtMemState msBeforeTest, msAfterTest ;
#endif
#endif

} ;

#endif
