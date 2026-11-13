//
// Copyright (C) 2006-2026 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#include <sipxunittests.h>

#include "TestRegistrar.h"
#include "sipXtapiTest.h"
#include "EventRecorder.h"
#include "EventValidator.h"
#include "callbacks.h"
#include <sipxunit/RscBracket.h>

extern EventRecorder g_recorder ;
bool g_bCallbackCalled = false;

extern SIPX_INST g_hInst;
extern EventRecorder g_recorder ;
extern EventRecorder g_lineRecorder; 
extern EventRecorder g_lineRecorder2; 

extern SIPX_INST g_hInst2;
extern EventRecorder g_recorder2 ;

extern SIPX_INST g_hInst3;
extern EventRecorder g_recorder3 ;

extern SIPX_INST g_hInst4;

extern bool g_bCallbackCalled;

extern SIPX_INST g_hInstInfo;
extern EventRecorder g_recorderInfo;
extern SIPX_CALL ghCallHangup;

/**
 * Test valid bounds: min gain, mid gain, and max gain. 
 */
void sipXtapiTestSuite::testGainAPI() 
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        int iGainLevel ;    

        printf("\ntestGainAPI (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        // Set to min
        CPPUNIT_ASSERT_EQUAL(sipxAudioSetGain(g_hInst, GAIN_MIN), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxAudioGetGain(g_hInst, iGainLevel), SIPX_RESULT_SUCCESS) ;    
        CPPUNIT_ASSERT_EQUAL(iGainLevel, GAIN_MIN) ;

        // Set to default
        CPPUNIT_ASSERT_EQUAL(sipxAudioSetGain(g_hInst, GAIN_DEFAULT), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxAudioGetGain(g_hInst, iGainLevel), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(iGainLevel, GAIN_DEFAULT) ;
        
        // set to max
        CPPUNIT_ASSERT_EQUAL(sipxAudioSetGain(g_hInst, GAIN_MAX), SIPX_RESULT_SUCCESS) ;    
        CPPUNIT_ASSERT_EQUAL(sipxAudioGetGain(g_hInst, iGainLevel), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(iGainLevel, GAIN_MAX) ;

        // set to max again
        CPPUNIT_ASSERT_EQUAL(sipxAudioSetGain(g_hInst, GAIN_MAX), SIPX_RESULT_SUCCESS) ;    
        CPPUNIT_ASSERT_EQUAL(sipxAudioGetGain(g_hInst, iGainLevel), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(iGainLevel, GAIN_MAX) ;    
    }

    // Does not create a call -- no need to pause
    // OsTask::delay(TEST_DELAY) ;    
    checkForLeaks();
}

/**
 * Verify mute state and that gain is not modified
 */
void sipXtapiTestSuite::testMuteAPI()
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        int iGainLevel ;
        bool bMuted ;

        printf("\ntestMuteAPI (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        for (int i = 0; i < 10; i++)
        {
            // Set gain to known value
            CPPUNIT_ASSERT_EQUAL(sipxAudioSetGain(g_hInst, GAIN_DEFAULT), SIPX_RESULT_SUCCESS) ;
            CPPUNIT_ASSERT_EQUAL(sipxAudioGetGain(g_hInst, iGainLevel), SIPX_RESULT_SUCCESS) ;
            CPPUNIT_ASSERT_EQUAL(iGainLevel, GAIN_DEFAULT) ;

            // Test Mute API
            CPPUNIT_ASSERT_EQUAL(sipxAudioMute(g_hInst, true), SIPX_RESULT_SUCCESS) ;
            CPPUNIT_ASSERT_EQUAL(sipxAudioIsMuted(g_hInst, bMuted), SIPX_RESULT_SUCCESS) ;
            CPPUNIT_ASSERT_EQUAL(bMuted, true) ;
            CPPUNIT_ASSERT_EQUAL(sipxAudioGetGain(g_hInst, iGainLevel), SIPX_RESULT_SUCCESS) ;
            CPPUNIT_ASSERT_EQUAL(iGainLevel, GAIN_DEFAULT) ;

            // Test Unmute API
            CPPUNIT_ASSERT_EQUAL(sipxAudioMute(g_hInst, false), SIPX_RESULT_SUCCESS) ;
            CPPUNIT_ASSERT_EQUAL(sipxAudioIsMuted(g_hInst, bMuted), SIPX_RESULT_SUCCESS) ;
            CPPUNIT_ASSERT_EQUAL(bMuted, false) ;
            CPPUNIT_ASSERT_EQUAL(sipxAudioGetGain(g_hInst, iGainLevel), SIPX_RESULT_SUCCESS) ;
            CPPUNIT_ASSERT_EQUAL(iGainLevel, GAIN_DEFAULT) ;

            // Test Unmute again
            CPPUNIT_ASSERT_EQUAL(sipxAudioMute(g_hInst, false), SIPX_RESULT_SUCCESS) ;
            CPPUNIT_ASSERT_EQUAL(sipxAudioIsMuted(g_hInst, bMuted), SIPX_RESULT_SUCCESS) ;
            CPPUNIT_ASSERT_EQUAL(bMuted, false) ;
            CPPUNIT_ASSERT_EQUAL(sipxAudioGetGain(g_hInst, iGainLevel), SIPX_RESULT_SUCCESS) ;
            CPPUNIT_ASSERT_EQUAL(iGainLevel, GAIN_DEFAULT) ;
        }
    }
    
    // Does not create a call -- no need to pause
    // OsTask::delay(TEST_DELAY) ;    
    checkForLeaks() ;
}

/*
 * Test valid bounds: min, mid, and max. 
 */
void sipXtapiTestSuite::testVolumeAPI() 
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        SPEAKER_TYPE type ;
        int iLevel ;

        printf("\ntestVolumeAPI (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        // Test Enable Speaker
        CPPUNIT_ASSERT_EQUAL(sipxAudioEnableSpeaker(g_hInst, SPEAKER), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxAudioGetEnabledSpeaker(g_hInst, type), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(type, SPEAKER) ;
        CPPUNIT_ASSERT_EQUAL(sipxAudioEnableSpeaker(g_hInst, RINGER), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxAudioGetEnabledSpeaker(g_hInst, type), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(type, RINGER) ;

        // Set both RINGER and SPEAKER to know states (cloned below)
        CPPUNIT_ASSERT_EQUAL(sipxAudioSetVolume(g_hInst, SPEAKER, VOLUME_DEFAULT), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxAudioSetVolume(g_hInst, RINGER, VOLUME_DEFAULT), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxAudioGetVolume(g_hInst, SPEAKER, iLevel), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(iLevel, VOLUME_DEFAULT) ;
        CPPUNIT_ASSERT_EQUAL(sipxAudioGetVolume(g_hInst, RINGER, iLevel), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(iLevel, VOLUME_DEFAULT) ;

        // Test SPEAKER making sure RINGER doesn't change
        CPPUNIT_ASSERT_EQUAL(sipxAudioSetVolume(g_hInst, SPEAKER, VOLUME_MIN), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxAudioGetVolume(g_hInst, SPEAKER, iLevel), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(iLevel, VOLUME_MIN) ;
        CPPUNIT_ASSERT_EQUAL(sipxAudioGetVolume(g_hInst, RINGER, iLevel), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(iLevel, VOLUME_DEFAULT) ;
        CPPUNIT_ASSERT_EQUAL(sipxAudioSetVolume(g_hInst, SPEAKER, VOLUME_MAX), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxAudioGetVolume(g_hInst, SPEAKER, iLevel), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(iLevel, VOLUME_MAX) ;
        
        // Set both RINGER and SPEAKER to know states (clone of above)
        CPPUNIT_ASSERT_EQUAL(sipxAudioSetVolume(g_hInst, SPEAKER, VOLUME_DEFAULT), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxAudioSetVolume(g_hInst, RINGER, VOLUME_DEFAULT), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxAudioGetVolume(g_hInst, SPEAKER, iLevel), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(iLevel, VOLUME_DEFAULT) ;
        CPPUNIT_ASSERT_EQUAL(sipxAudioGetVolume(g_hInst, RINGER, iLevel), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(iLevel, VOLUME_DEFAULT) ;

        // Test RINGER making sure SPEAKER doesn't change
        CPPUNIT_ASSERT_EQUAL(sipxAudioSetVolume(g_hInst, RINGER, VOLUME_MIN), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxAudioGetVolume(g_hInst, RINGER, iLevel), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(iLevel, VOLUME_MIN) ;
        CPPUNIT_ASSERT_EQUAL(sipxAudioGetVolume(g_hInst, SPEAKER, iLevel), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(iLevel, VOLUME_DEFAULT) ;
        CPPUNIT_ASSERT_EQUAL(sipxAudioSetVolume(g_hInst, RINGER, VOLUME_MAX), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxAudioGetVolume(g_hInst, RINGER, iLevel), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(iLevel, VOLUME_MAX) ;
    }

    OsTask::delay(TEST_DELAY) ;    
    checkForLeaks() ;
}

void sipXtapiTestSuite::testAudioSettings()
{
    size_t  i;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestAudioSettings (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        // Test Enable AEC

        SIPX_AEC_MODE mode ;
        CPPUNIT_ASSERT_EQUAL( sipxAudioSetAECMode(g_hInst, SIPX_AEC_DISABLED), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL( sipxAudioGetAECMode(g_hInst, mode), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL( mode, SIPX_AEC_DISABLED) ;

        CPPUNIT_ASSERT_EQUAL( sipxAudioSetAECMode(g_hInst, SIPX_AEC_SUPPRESS), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL( sipxAudioGetAECMode(g_hInst, mode), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL( mode, SIPX_AEC_SUPPRESS) ;

        CPPUNIT_ASSERT_EQUAL( sipxAudioSetAECMode(g_hInst, SIPX_AEC_CANCEL), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL( sipxAudioGetAECMode(g_hInst, mode), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL( mode, SIPX_AEC_CANCEL) ;

        CPPUNIT_ASSERT_EQUAL( sipxAudioSetAECMode(g_hInst, SIPX_AEC_CANCEL_AUTO), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL( sipxAudioGetAECMode(g_hInst, mode), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL( mode, SIPX_AEC_CANCEL_AUTO) ;

        // Test Noise Reduction
        SIPX_NOISE_REDUCTION_MODE nrMode ;
        CPPUNIT_ASSERT_EQUAL( sipxAudioSetNoiseReductionMode(g_hInst, SIPX_NOISE_REDUCTION_DISABLED), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL( sipxAudioGetNoiseReductionMode(g_hInst, nrMode), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL( nrMode, SIPX_NOISE_REDUCTION_DISABLED) ;

        CPPUNIT_ASSERT_EQUAL( sipxAudioSetNoiseReductionMode(g_hInst, SIPX_NOISE_REDUCTION_LOW), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL( sipxAudioGetNoiseReductionMode(g_hInst, nrMode), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL( nrMode, SIPX_NOISE_REDUCTION_LOW) ;

        CPPUNIT_ASSERT_EQUAL( sipxAudioSetNoiseReductionMode(g_hInst, SIPX_NOISE_REDUCTION_MEDIUM), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL( sipxAudioGetNoiseReductionMode(g_hInst, nrMode), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL( nrMode, SIPX_NOISE_REDUCTION_MEDIUM) ;

        CPPUNIT_ASSERT_EQUAL( sipxAudioSetNoiseReductionMode(g_hInst, SIPX_NOISE_REDUCTION_HIGH), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL( sipxAudioGetNoiseReductionMode(g_hInst, nrMode), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL( nrMode, SIPX_NOISE_REDUCTION_HIGH) ;

        size_t numOfDevices;
        const char* szDevice;
        const char* checker = NULL;
        //Test sipxAudioGetNumInputDevices
        CPPUNIT_ASSERT_EQUAL( sipxAudioGetNumInputDevices(g_hInst, numOfDevices), SIPX_RESULT_SUCCESS);
        for(i = 0; i < numOfDevices; i++) 
        {
            CPPUNIT_ASSERT_EQUAL( sipxAudioGetInputDevice(g_hInst, (int)i, szDevice), SIPX_RESULT_SUCCESS);
        }
        //checks if there are no more devices
        CPPUNIT_ASSERT_EQUAL( sipxAudioGetInputDevice(g_hInst, (int)i, szDevice), SIPX_RESULT_SUCCESS);
        //szDevice should be NULL
        CPPUNIT_ASSERT_EQUAL(szDevice, checker);
        numOfDevices = 0;
        //Test sipxAudioGetNumOutputDevices
        CPPUNIT_ASSERT_EQUAL( sipxAudioGetNumOutputDevices(g_hInst, numOfDevices), SIPX_RESULT_SUCCESS);
        for(i = 0; i < numOfDevices; i++) 
        {
            CPPUNIT_ASSERT_EQUAL( sipxAudioGetOutputDevice(g_hInst, (int)i, szDevice), SIPX_RESULT_SUCCESS);
        }
        //checks if there are no more devices
        CPPUNIT_ASSERT_EQUAL( sipxAudioGetOutputDevice(g_hInst, (int)i, szDevice), SIPX_RESULT_SUCCESS);
        //checker is NULL
        CPPUNIT_ASSERT_EQUAL(szDevice, checker);
    }

    // Does not create a call -- no need to pause
    // OsTask::delay(TEST_DELAY) ;    
    checkForLeaks() ;
}

// Try the GetVersion API, one time with large buffer,
// then with very small buffer
void sipXtapiTestSuite::testGetVersion() 
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        char szBuffer[64];

        printf("\ntestGetVersion (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        CPPUNIT_ASSERT_EQUAL(sipxConfigGetVersion(szBuffer, 64), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT(strstr(szBuffer, "sipXtapi")!=NULL);
        CPPUNIT_ASSERT_EQUAL(sipxConfigGetVersion(szBuffer, 2), SIPX_RESULT_INSUFFICIENT_BUFFER);
    }

    // Does not create a call -- no need to pause
    // OsTask::delay(TEST_DELAY) ;    
    checkForLeaks() ;
}

void testCallback(const char* szPriority, const char *szSource, const char* szMsg)
{
    if (strcmp(szPriority, "DEBUG") == 0)
    {
        if ( strstr(szMsg, "**Testing callbacks**") != NULL )
        {
            g_bCallbackCalled = true;
        }
    }
}

void sipXtapiTestSuite::testSetCallback()
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        g_bCallbackCalled = false;

        printf("\ntestSetCallback (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        // Enable logging at DEBUG level
        sipxConfigSetLogLevel( LOG_LEVEL_DEBUG );

        // Set callback function
        sipxConfigSetLogCallback( testCallback );

        // Send special log messafe at DEBUG level
        sipxLogEntryAdd(PRI_DEBUG, "**Testing callbacks**");
        OsTask::delay(200);

        CPPUNIT_ASSERT_EQUAL(g_bCallbackCalled, true);

        // Disable logging
        sipxConfigSetLogCallback( NULL );
        sipxConfigSetLogLevel( LOG_LEVEL_NONE );
    }
    
    // Does not create a call -- no need to pause
    // OsTask::delay(TEST_DELAY) ;    
    checkForLeaks() ;
}


void sipXtapiTestSuite::testAutoPortSelection() 
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        SIPX_INST hHandle ;
        SIPX_RESULT rc ;
        int iPort ;

        printf("\ntestAutoPortSelection (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        rc = sipxInitialize(&hHandle, SIPX_PORT_AUTO, SIPX_PORT_AUTO, SIPX_PORT_AUTO,
                            1234) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;


        rc = sipxConfigGetLocalSipUdpPort(hHandle, &iPort) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(portIsValid(iPort)) ;

        rc = sipxConfigGetLocalSipTcpPort(hHandle, &iPort) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(portIsValid(iPort)) ;
    #if 0
        // Bob 2005-04-06: Wait until TLS is supported/enabled for this
        // this test case

        rc = sipxConfigGetLocalSipTlsPort(hHandle, &iPort) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(portIsValid(iPort)) ;
    #endif
        SIPX_CONTACT_ADDRESS addresses[32];
        size_t actualNum = 0;
        
        sipxConfigGetLocalContacts(hHandle, addresses, 32, actualNum);
        for (size_t i = 0; i < actualNum; i++)
        {
            CPPUNIT_ASSERT(addresses[i].iPort > 0);
        }
        
        rc = sipxUnInitialize(hHandle) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;        
    }

    // Does not create a call -- no need to pause
    // OsTask::delay(TEST_DELAY) ;    
    checkForLeaks() ;
}

void sipXtapiTestSuite::testSeqPortSelection() 
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        SIPX_INST hHandle ;
        SIPX_INST hHandle2 ;
        SIPX_RESULT rc ;
        int iPort ;

        printf("\ntestSeqPortSelection (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        rc = sipxInitialize(&hHandle, 8100, 8100, 8101, 9999, DEFAULT_CONNECTIONS, DEFAULT_IDENTITY, DEFAULT_BIND_ADDRESS, TRUE) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

        rc = sipxConfigGetLocalSipUdpPort(hHandle, &iPort) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(iPort, 8100) ;

        rc = sipxConfigGetLocalSipTcpPort(hHandle, &iPort) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(iPort, 8100) ;

    #if 0
        // Bob 2005-04-06: Wait until TLS is supported/enabled for this
        // this test case

        rc = sipxConfigGetLocalSipTlsPort(hHandle, &iPort) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(iPort, 8101) ;
    #endif
        rc = sipxInitialize(&hHandle2, 8100, 8100, 8101, 9999, DEFAULT_CONNECTIONS, DEFAULT_IDENTITY, DEFAULT_BIND_ADDRESS, TRUE) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

        rc = sipxConfigGetLocalSipUdpPort(hHandle2, &iPort) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(portIsValid(iPort) && iPort > 8100) ;

        rc = sipxConfigGetLocalSipTcpPort(hHandle2, &iPort) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(portIsValid(iPort) && iPort > 8100) ;
        
        SIPX_CONTACT_ADDRESS addresses[32];
        size_t actualNum = 0;
        
        sipxConfigGetLocalContacts(hHandle2, addresses, 32, actualNum);
        for (size_t i = 0; i < actualNum; i++)
        {
            CPPUNIT_ASSERT(addresses[i].iPort > 8100);
        }
        

    #if 0
        // Bob 2005-04-06: Wait until TLS is supported/enabled for this
        // this test case

        rc = sipxConfigGetLocalSipTlsPort(hHandle2, &iPort) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(portIsValid(iPort) && iPort > 8100) ;
    #endif

        rc = sipxUnInitialize(hHandle) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        rc = sipxUnInitialize(hHandle2) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;    
    }

    // Does not create a call -- no need to pause
    // OsTask::delay(TEST_DELAY) ;    
    checkForLeaks() ;
}

void sipXtapiTestSuite::testConfigLog() 
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestConfigLog (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);
        //tests sipxConfigSetLogLevel
        CPPUNIT_ASSERT_EQUAL( sipxConfigSetLogLevel(LOG_LEVEL_ERR), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL( sipxConfigSetLogLevel(LOG_LEVEL_DEBUG), SIPX_RESULT_SUCCESS);
        //tests sipxConfigSetLogFile
        CPPUNIT_ASSERT_EQUAL( sipxConfigSetLogFile(NULL), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL( sipxConfigSetLogFile("C:\\example.log"), SIPX_RESULT_SUCCESS);
        sipxLogCallback pcallBack = NULL;
        CPPUNIT_ASSERT_EQUAL( sipxConfigSetLogCallback(pcallBack), SIPX_RESULT_SUCCESS);        
    }
    
    // Does not create a call -- no need to pause
    // OsTask::delay(TEST_DELAY) ;    
    checkForLeaks() ;
}

void sipXtapiTestSuite::testConfigExternalTransport() 
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\nConfigExternalTransport (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        SIPX_TRANSPORT hTransport = SIPX_TRANSPORT_NULL;

        SIPX_CONTACT_ADDRESS addresses[32] ;
        memset(addresses, 0, sizeof(SIPX_CONTACT_ADDRESS)*32);
        size_t nContacts ;
        size_t origContacts;
        sipxConfigGetLocalContacts(g_hInst, addresses, 32, origContacts) ;

        sipxConfigExternalTransportAdd(g_hInst,
                                       hTransport, 
                                       true, 
                                       "flibble", 
                                       "127.0.0.1", 
                                       -1,                                        
                                       FlibbleTransportCallback,
                                       "fibble");
        sipxConfigExternalTransportRouteByUser(hTransport, false) ;
        CPPUNIT_ASSERT((int)hTransport > 3);
        
        memset(addresses, 0, sizeof(SIPX_CONTACT_ADDRESS)*32);
        sipxConfigGetLocalContacts(g_hInst, addresses, 32, nContacts) ;
        CPPUNIT_ASSERT(nContacts == origContacts + 1 );

        sipxConfigExternalTransportRemove(hTransport);

        memset(addresses, 0, sizeof(SIPX_CONTACT_ADDRESS)*32);
        sipxConfigGetLocalContacts(g_hInst, addresses, 32, nContacts) ;
        CPPUNIT_ASSERT(nContacts == origContacts);
    }
    checkForLeaks() ;
}

void sipXtapiTestSuite::testConfigOutOfBand()
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestConfigOutOfBand (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);
        //tests sipxConfigEnableOutOfBandDTMF and sipxConfigIsOutOfBandDTMFEnabled
        CPPUNIT_ASSERT_EQUAL( sipxConfigEnableOutOfBandDTMF(g_hInst,true), SIPX_RESULT_SUCCESS);
        bool test = false;
        CPPUNIT_ASSERT_EQUAL( sipxConfigIsOutOfBandDTMFEnabled(g_hInst, test), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(test, true);
        CPPUNIT_ASSERT_EQUAL( sipxConfigEnableOutOfBandDTMF(g_hInst, false), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL( sipxConfigIsOutOfBandDTMFEnabled(g_hInst, test), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(test, false);

    }

    // Does not create a call -- no need to pause
    // OsTask::delay(TEST_DELAY) ;    
    checkForLeaks() ;
}

void sipXtapiTestSuite::testTeardown()
{
    SIPX_INST hInst1 = NULL ;
    SIPX_INST hInst2 = NULL ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestTeardown (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);
        // printf("\nInitializing 2 instances\n");
        sipxInitialize(&hInst1, 8000, 8000, 8001, 8050) ;
        sipxInitialize(&hInst2, 9100, 9100, 9101, 9050) ;
        // printf("hInst1: %08X, hInst2: %08X\n", hInst1, hInst2);

        // printf("Uninitializing 2 instances\n");
        sipxUnInitialize(hInst2);
        sipxUnInitialize(hInst1);
    }

    // Does not create a call -- no need to pause
    // OsTask::delay(TEST_DELAY) ;    

    checkForLeaks() ;
} 

// The shared media factory is refcounted across instances; it must
// survive until the last one is gone. The double-release bug deleted it
// on the first uninitialize with two instances alive.
void sipXtapiTestSuite::testMultiInstanceUninitKeepsMediaFactory()
{
   SIPX_INST hA = NULL, hB = NULL, hC = NULL;
   CPPUNIT_ASSERT_EQUAL(SIPX_RESULT_SUCCESS,
      sipxInitialize(&hA, 14000, 14000, 14001, 14050, 32, "sip:a@127.0.0.1:14000", "127.0.0.1",
                     false, NULL, NULL, NULL, true, INTERNAL_SAMPLE_RATE, DEVICE_SAMPLE_RATE));
   CPPUNIT_ASSERT_EQUAL(SIPX_RESULT_SUCCESS,
      sipxInitialize(&hB, 14100, 14100, 14101, 14150, 32, "sip:b@127.0.0.1:14100", "127.0.0.1",
                     false, NULL, NULL, NULL, true, INTERNAL_SAMPLE_RATE, DEVICE_SAMPLE_RATE));
   CPPUNIT_ASSERT_EQUAL(SIPX_RESULT_SUCCESS,
      sipxInitialize(&hC, 14200, 14200, 14201, 14250, 32, "sip:c@127.0.0.1:14200", "127.0.0.1",
                     false, NULL, NULL, NULL, true, INTERNAL_SAMPLE_RATE, DEVICE_SAMPLE_RATE));

   size_t n = 0;
   CPPUNIT_ASSERT_EQUAL(SIPX_RESULT_SUCCESS, sipxUnInitialize(hA, true));
   // B must still reach the media factory after A is gone.
   CPPUNIT_ASSERT_EQUAL_MESSAGE("media factory gone after first instance uninitialized",
                                SIPX_RESULT_SUCCESS, sipxAudioGetNumInputDevices(hB, n));
   CPPUNIT_ASSERT_EQUAL(SIPX_RESULT_SUCCESS, sipxUnInitialize(hB, true));
   CPPUNIT_ASSERT_EQUAL_MESSAGE("media factory gone after second instance uninitialized",
                                SIPX_RESULT_SUCCESS, sipxAudioGetNumInputDevices(hC, n));
   CPPUNIT_ASSERT_EQUAL(SIPX_RESULT_SUCCESS, sipxUnInitialize(hC, true));
}

// Open the sipX log for reading. With LOG_TO_FILE, which only the Windows
// projects define, sipxInitialize hardcodes sipXtapi.log; otherwise setUp's
// sipxConfigSetLogFile name is used.
static FILE* knownDefectOpenLog()
{
    FILE* logFile = fopen("sipXtapi.log", "rb") ;
    if (logFile == NULL)
    {
        logFile = fopen("sipXtapiTests.txt", "rb") ;
    }
    return logFile ;
}

// Flowgraph count from the first "MpMediaTask::sendToAllFlowgraphs sent
// message[n] to N flowgraphs" line after the line containing marker, plus
// any "failed to send" lines in between (a flowgraph that exists but could
// not take the message). -1 when the marker or the count line is not in
// the log yet. The whole file is scanned each time: the DLL holds the log
// open, and an offset taken on a separate handle is not reliable (see
// testLoggingSurvivesOtherInstanceUninit).
static int knownDefectFlowgraphCountAfter(const char* marker)
{
    FILE* logFile = knownDefectOpenLog() ;
    if (logFile == NULL)
    {
        return -1 ;
    }
    char logLine[8192] ;
    UtlBoolean seenMarker = FALSE ;
    int failedSends = 0 ;
    int count = -1 ;
    while (count < 0 && fgets(logLine, sizeof(logLine), logFile) != NULL)
    {
        if (!seenMarker)
        {
            seenMarker = (strstr(logLine, marker) != NULL) ;
            continue ;
        }
        if (strstr(logLine, "MpMediaTask::sendToAllFlowgraphs failed to send") != NULL)
        {
            failedSends++ ;
            continue ;
        }
        const char* s = strstr(logLine, "MpMediaTask::sendToAllFlowgraphs sent message[") ;
        if (s != NULL)
        {
            const char* t = strstr(s, "] to ") ;
            if (t != NULL)
            {
                count = atoi(t + 5) + failedSends ;
            }
        }
    }
    fclose(logFile) ;
    return count ;
}

// Initialize an instance on fixed ports, retrying briefly: ports can
// linger in TIME_WAIT right after an uninitialize.
static SIPX_RESULT knownDefectInitialize(SIPX_INST* phInst, int sipPort, int rtpPort,
                                         const char* identity)
{
    SIPX_RESULT rc = SIPX_RESULT_FAILURE ;
    *phInst = NULL ;
    for (int attempt = 0; attempt < 20; attempt++)
    {
        rc = sipxInitialize(phInst, sipPort, sipPort, sipPort + 1, rtpPort, 32, identity,
                            "127.0.0.1", false, NULL, NULL, NULL, true,
                            INTERNAL_SAMPLE_RATE, DEVICE_SAMPLE_RATE) ;
        if (rc == SIPX_RESULT_SUCCESS && *phInst != NULL)
        {
            break ;
        }
        OsTask::delay(500) ;
    }
    return rc ;
}

// End the surviving side of a call whose other instance was
// force-uninitialized, and wait for its DESTROYED.
static bool knownDefectEndSurvivor(SIPX_INST hInst, SIPX_LINE hLine, SIPX_CALL& hCall)
{
    EventValidator validatorEnd("knownDefect.survivor.end") ;
    validatorEnd.ignoreMediaEvent(MEDIA_INPUT_DEVICE_NOT_PRESENT) ;
    validatorEnd.ignoreMediaEvent(MEDIA_OUTPUT_DEVICE_NOT_PRESENT) ;
    if (sipxEventListenerAdd(hInst, UniversalEventValidatorCallback, &validatorEnd)
        != SIPX_RESULT_SUCCESS)
    {
        return false ;
    }
    SIPX_CALL hEndedCall = hCall ;
    bool ok = (sipxCallDestroy(hCall) == SIPX_RESULT_SUCCESS) ;
    ok = ok && validatorEnd.waitForCallEvent(hLine, hEndedCall, CALLSTATE_DESTROYED,
                                             CALLSTATE_CAUSE_NORMAL, false, 10) ;
    sipxEventListenerRemove(hInst, UniversalEventValidatorCallback, &validatorEnd) ;
    return ok ;
}

// KNOWN DEFECT 8 (phase 2, not fixed for delivery): a forced
// sipxUnInitialize with a call up never deletes the in-focus call
// (~CallManager deletes only the calls on its call stack), so the call's
// media interface and flowgraph leak. While another instance keeps the
// media factory alive, the leaked flowgraph keeps running. This test
// asserts the contract -- no flowgraph left once both calls are gone --
// and fails until the defect is fixed. Do not relax it.
//
// The flowgraph count comes from the DEBUG log line the media task writes
// whenever a call device is selected; the test selects the first input
// device to make it write one. No audio input device: skip, loudly.
void sipXtapiTestSuite::testForcedUninitReleasesCallFlowgraph()
{
    SIPX_INST hA = NULL ;
    SIPX_INST hB = NULL ;
    CPPUNIT_ASSERT_EQUAL(SIPX_RESULT_SUCCESS,
        knownDefectInitialize(&hA, 14500, 14550, "sip:d8calling@127.0.0.1:14500")) ;
    CPPUNIT_ASSERT_EQUAL(SIPX_RESULT_SUCCESS,
        knownDefectInitialize(&hB, 14600, 14650, "sip:d8called@127.0.0.1:14600")) ;

    size_t numInputs = 0 ;
    const char* inputName = NULL ;
    if (sipxAudioGetNumInputDevices(hB, numInputs) != SIPX_RESULT_SUCCESS || numInputs == 0
        || sipxAudioGetInputDevice(hB, 0, inputName) != SIPX_RESULT_SUCCESS || inputName == NULL)
    {
        sipxUnInitialize(hA, true) ;
        sipxUnInitialize(hB, true) ;
        printf("SIPX_TEST_SKIP: KNOWN DEFECT 8 test needs an audio input device to select\n") ;
        fflush(stdout) ;
        CPPUNIT_ASSERT_MESSAGE("SIPX_TEST_SKIP: no audio input device", 0) ;
        return ;
    }
    UtlString inputDevice(inputName) ;

    SIPX_CALL hCallingCall = SIPX_CALL_NULL ;
    SIPX_LINE hCallingLine = SIPX_LINE_NULL ;
    SIPX_CALL hCalledCall = SIPX_CALL_NULL ;
    SIPX_LINE hCalledLine = SIPX_LINE_NULL ;
    doCallBasicSetup(hA, "sip:d8calling@127.0.0.1:14500", "sip:d8calling@127.0.0.1:14500",
                     hB, "sip:d8called@127.0.0.1:14600", "sip:d8called@127.0.0.1:14600",
                     hCallingCall, hCallingLine, hCalledCall, hCalledLine) ;
    OsTask::delay(1000) ;

    // The customer's call: forced uninitialize with the call up.
    CPPUNIT_ASSERT_EQUAL(SIPX_RESULT_SUCCESS, sipxUnInitialize(hA, true)) ;
    hA = NULL ;

    // End the other side normally; its flowgraph must go.
    bool bEnded = knownDefectEndSurvivor(hB, hCalledLine, hCalledCall) ;
    CPPUNIT_ASSERT_MESSAGE("called side not DESTROYED within 10 s after the other side's forced uninitialize",
                           bEnded) ;
    sipxLineRemove(hCalledLine) ;
    // The call manager releases the media interface after DESTROYED.
    OsTask::delay(1000) ;

    // Marker in the log, unique per run, then a device select so the media
    // task logs how many flowgraphs it still runs.
    char marker[128] ;
    sprintf(marker, "sip:d8probe%u@127.0.0.1:14600",
            (unsigned) OsDateTime::getSecsSinceEpoch()) ;
    SIPX_LINE hProbeLine = SIPX_LINE_NULL ;
    CPPUNIT_ASSERT_EQUAL(SIPX_RESULT_SUCCESS, sipxLineAdd(hB, marker, &hProbeLine)) ;
    sipxLineRemove(hProbeLine) ;
    CPPUNIT_ASSERT_EQUAL(SIPX_RESULT_SUCCESS,
                         sipxAudioSetCallInputDevice(hB, inputDevice.data())) ;

    int flowgraphs = -1 ;
    for (int attempt = 0; attempt < 50 && flowgraphs < 0; attempt++)
    {
        OsTask::delay(100) ;
        flowgraphs = knownDefectFlowgraphCountAfter(marker) ;
    }
    printf("KNOWN DEFECT 8 probe: flowgraphs still running after both calls ended: %d\n",
           flowgraphs) ;
    fflush(stdout) ;

    sipxUnInitialize(hB, true) ;

    CPPUNIT_ASSERT_MESSAGE("no flowgraph count line in the sipX log after a device select "
                           "(log not at DEBUG, or log file not found)", flowgraphs >= 0) ;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("KNOWN DEFECT 8: flowgraph still running after a forced "
                                 "sipxUnInitialize with a call up", 0, flowgraphs) ;
    OsTask::delay(TEST_DELAY) ;
    checkForLeaks() ;
}

// Records the line handle of the first call event an instance reports.
struct KnownDefectLineRecord
{
    volatile long events ;
    SIPX_LINE firstLine ;
} ;

static bool SIPX_CALLING_CONVENTION knownDefectLineRecorder(SIPX_EVENT_CATEGORY category,
                                                            void* pInfo, void* pUserData)
{
    if (category == EVENT_CATEGORY_CALLSTATE && pInfo && pUserData)
    {
        KnownDefectLineRecord* pRecord = (KnownDefectLineRecord*) pUserData ;
        if (pRecord->events == 0)
        {
            pRecord->firstLine = ((SIPX_CALLSTATE_INFO*) pInfo)->hLine ;
        }
        pRecord->events++ ;
    }
    return true ;
}

// KNOWN DEFECT 2 (phase 2): a forced sipxUnInitialize with a call and line
// up leaves the dead instance's line in the global line map. Call events
// look their line up by URI across all instances
// (sipxLineLookupHandleByURI), so after the instance is re-created and the
// same line URI added again, the new line's call events carry the dead
// line's handle. This test asserts the contract -- events carry the new
// line's handle -- and fails until the defect is fixed. Do not relax it.
void sipXtapiTestSuite::testForcedUninitReleasesLineHandles()
{
    const char* callingUri = "sip:d2calling@127.0.0.1:14700" ;
    const char* calledUri = "sip:d2called@127.0.0.1:14800" ;
    SIPX_INST hA = NULL ;
    SIPX_INST hB = NULL ;
    CPPUNIT_ASSERT_EQUAL(SIPX_RESULT_SUCCESS, knownDefectInitialize(&hA, 14700, 14750, callingUri)) ;
    CPPUNIT_ASSERT_EQUAL(SIPX_RESULT_SUCCESS, knownDefectInitialize(&hB, 14800, 14850, calledUri)) ;

    SIPX_CALL hCallingCall = SIPX_CALL_NULL ;
    SIPX_LINE hCallingLine = SIPX_LINE_NULL ;
    SIPX_CALL hCalledCall = SIPX_CALL_NULL ;
    SIPX_LINE hCalledLine = SIPX_LINE_NULL ;
    doCallBasicSetup(hA, callingUri, callingUri, hB, calledUri, calledUri,
                     hCallingCall, hCallingLine, hCalledCall, hCalledLine) ;
    SIPX_LINE hDeadLine = hCallingLine ;

    // Forced uninitialize with the call and line up, as the customer does.
    CPPUNIT_ASSERT_EQUAL(SIPX_RESULT_SUCCESS, sipxUnInitialize(hA, true)) ;
    hA = NULL ;
    bool bEnded = knownDefectEndSurvivor(hB, hCalledLine, hCalledCall) ;
    CPPUNIT_ASSERT_MESSAGE("called side not DESTROYED within 10 s after the other side's forced uninitialize",
                           bEnded) ;
    sipxLineRemove(hCalledLine) ;

    // Re-create the instance and add the same line URI again.
    CPPUNIT_ASSERT_EQUAL(SIPX_RESULT_SUCCESS, knownDefectInitialize(&hA, 14700, 14750, callingUri)) ;
    SIPX_LINE hNewLine = SIPX_LINE_NULL ;
    CPPUNIT_ASSERT_EQUAL(SIPX_RESULT_SUCCESS, sipxLineAdd(hA, callingUri, &hNewLine)) ;

    // Any call event on the new line must name the new line.
    KnownDefectLineRecord record = { 0, SIPX_LINE_NULL } ;
    CPPUNIT_ASSERT_EQUAL(SIPX_RESULT_SUCCESS,
                         sipxEventListenerAdd(hA, knownDefectLineRecorder, &record)) ;
    SIPX_CALL hCall = SIPX_CALL_NULL ;
    CPPUNIT_ASSERT_EQUAL(SIPX_RESULT_SUCCESS, sipxCallCreate(hA, hNewLine, &hCall)) ;
    for (int attempt = 0; attempt < 50 && record.events == 0; attempt++)
    {
        OsTask::delay(100) ;
    }
    SIPX_LINE firstLine = record.firstLine ;
    long events = record.events ;
    printf("KNOWN DEFECT 2 probe: dead line %d, new line %d, first call event names line %d (%ld events)\n",
           (int) hDeadLine, (int) hNewLine, (int) firstLine, events) ;
    fflush(stdout) ;

    sipxEventListenerRemove(hA, knownDefectLineRecorder, &record) ;
    sipxCallDestroy(hCall) ;
    sipxLineRemove(hNewLine) ;
    OsTask::delay(500) ;
    sipxUnInitialize(hA, true) ;
    sipxUnInitialize(hB, true) ;

    CPPUNIT_ASSERT_MESSAGE("no call event after sipxCallCreate on the new line", events > 0) ;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("KNOWN DEFECT 2: call event names the dead instance's line "
                                 "after a forced sipxUnInitialize and re-adding the same line URI",
                                 (int) hNewLine, (int) firstLine) ;
    OsTask::delay(TEST_DELAY) ;
    checkForLeaks() ;
}

// One call between the suite's first two instances, hung up from the
// called side (the side that owns the input device in the device departure
// tests), both calls destroyed and both lines removed.
void sipXtapiTestSuite::rscCallOnce(int index)
{
    char callingUri[64] ;
    char calledUri[64] ;
    sprintf(callingUri, "sip:rsccalling%d@127.0.0.1:8000", index) ;
    sprintf(calledUri, "sip:rsccalled%d@127.0.0.1:9100", index) ;

    SIPX_CALL hCallingCall = SIPX_CALL_NULL ;
    SIPX_LINE hCallingLine = SIPX_LINE_NULL ;
    SIPX_CALL hCalledCall = SIPX_CALL_NULL ;
    SIPX_LINE hCalledLine = SIPX_LINE_NULL ;
    doCallBasicSetup(g_hInst, callingUri, callingUri, g_hInst2, calledUri, calledUri,
                     hCallingCall, hCallingLine, hCalledCall, hCalledLine) ;
    OsTask::delay(500) ;

    CPPUNIT_ASSERT_MESSAGE("called side not DESTROYED within 10 s",
                           knownDefectEndSurvivor(g_hInst2, hCalledLine, hCalledCall)) ;
    CPPUNIT_ASSERT_MESSAGE("calling side not DESTROYED within 10 s",
                           knownDefectEndSurvivor(g_hInst, hCallingLine, hCallingCall)) ;
    sipxLineRemove(hCalledLine) ;
    sipxLineRemove(hCallingLine) ;
}

// Resource accounting (phase 2, stage 1), report only: what does one call
// setup and hang-up leave behind? The RSC lines are the measurement; this
// test asserts only that the calls themselves worked. Run at two sizes: a
// leak grows with the number of calls, background noise does not.
void sipXtapiTestSuite::testRscCallSetupHangup()
{
    int index = 0 ;
    rscCallOnce(index++) ;     // warm-up
    const int sizes[] = {5, 10} ;
    for (int s = 0; s < 2; s++)
    {
        RscBracket bracket("call setup/hangup") ;
        for (int i = 0; i < sizes[s]; i++)
        {
            rscCallOnce(index++) ;
        }
        bracket.report(sizes[s], 1500) ;
    }
}

// Resource accounting (phase 2, stage 2), report only: are the per-message
// UDP SipClient objects ever freed while the instances stay up?
//
// Every UDP send currently creates a new SipClient (an OsTask with its own
// semaphores and mutexes), about 7 per call. SipUserAgent garbage
// collection deletes clients idle for 40 s or more, but it runs only while
// the user agent is handling messages, at most every 40 s. This test makes
// 10 calls and reports, then keeps SIP traffic going for about 60 s (one
// call every 15 s) and reports again against the same starting snapshot.
//
// Reading the second RSC line (14 calls in all, about 7 clients each):
//   OsTask near +98: garbage collection freed none of them; a process that
//                    stays up accumulates them (a true leak).
//   OsTask near +21 to +28: the clients of the first 10 calls were freed;
//                    only the last 40 s or so of traffic is held.
// The sipX log shows the collector's own lines ("removeOldClients
// deleting N of M SipClients"). Takes about 100 s: run with --timeout 600.
void sipXtapiTestSuite::testRscCallGarbageCollection()
{
    int index = 1000 ;
    rscCallOnce(index++) ;     // warm-up

    RscBracket bracket("call x10 then 60 s of traffic") ;
    for (int i = 0; i < 10; i++)
    {
        rscCallOnce(index++) ;
    }
    bracket.report(10, 1500) ;

    for (int i = 0; i < 4; i++)
    {
        OsTask::delay(15000) ;
        rscCallOnce(index++) ;
    }
    bracket.report(14, 1500) ;
}

// Resource accounting (phase 2, stage 1), report only: what does
// initializing and uninitializing an idle instance leave behind, while
// the suite's other instances keep the media factory alive? Each cycle
// uses its own ports to stay clear of TIME_WAIT.
void sipXtapiTestSuite::testRscIdleInitUninit()
{
    int cycle = 0 ;
    const int sizes[] = {0, 3, 6} ;     // 0: the warm-up cycle
    for (int s = 0; s < 3; s++)
    {
        RscBracket bracket("idle instance init/uninit") ;
        int count = (sizes[s] == 0) ? 1 : sizes[s] ;
        for (int i = 0; i < count; i++, cycle++)
        {
            SIPX_INST hInst = NULL ;
            int sipPort = 15100 + 20 * cycle ;
            char identity[64] ;
            sprintf(identity, "sip:rscidle%d@127.0.0.1:%d", cycle, sipPort) ;
            CPPUNIT_ASSERT_EQUAL(SIPX_RESULT_SUCCESS,
                knownDefectInitialize(&hInst, sipPort, sipPort + 10, identity)) ;
            CPPUNIT_ASSERT_EQUAL(SIPX_RESULT_SUCCESS, sipxUnInitialize(hInst, true)) ;
        }
        if (sizes[s] > 0)
        {
            bracket.report(sizes[s], 1000) ;
        }
    }
}

void sipXtapiTestSuite::testLoggingSurvivesOtherInstanceUninit()
{
    // This method's own assertions pass, but the suite's tearDown crashes
    // in CpMediaInterfaceFactory::removeDispatcher, so the runner may
    // report this test as failed regardless.  See the dispatcher defect
    // introduced by uncommenting that call.

    // Uninitialize a bystander instance while the four suite globals stay
    // up.  sipxUnInitialize used to call OsSysLog::shutdown() regardless of
    // how many instances remained, and after that OsSysLog::add() silently
    // drops every entry, so all surviving instances lose logging.
    //

    // Check the log file rather than OsSysLog's state directly.  On Windows
    // sipXtapi is a DLL and sipXportLib is static, so this executable has
    // its own copy of the OsSysLog statics and cannot see the DLL's.  The
    // file is observable from either side on both platforms.

    SIPX_INST hInstB = NULL ;
    SIPX_RESULT rc ;
    SIPX_LINE hLine = SIPX_LINE_NULL ;

    rc = sipxInitialize(&hInstB, 14000, 14000, 14001, 14050, 32, NULL, "127.0.0.1") ;
    CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

    rc = sipxUnInitialize(hInstB) ;
    CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

    // Unique per run, so a marker from an earlier run cannot be matched.
    char probeUri[128] ;
    sprintf(probeUri, "sip:logprobe%u@127.0.0.1:8000",
            (unsigned) OsDateTime::getSecsSinceEpoch()) ;

    // Exercise a surviving instance through APIs that log at PRI_INFO.
    rc = sipxLineAdd(g_hInst, probeUri, &hLine) ;
    CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
    rc = sipxLineRemove(hLine) ;
    CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

    // Scan the whole file.  Seeking to an offset recorded before the probe
    // does not work: the DLL holds this file open and ftell on a separate
    // handle has been observed to report a position past entries that were
    // later found earlier in the file.
    //
    // Poll, because OsSysLog::add only queues the entry; the log task
    // thread writes and flushes it, and this executable cannot flush the
    // DLL's log task.
    UtlBoolean foundProbe = FALSE ;
    long logSize = 0 ;
    char logLine[2048] ;
    int probeAttempt ;

    for (probeAttempt = 0; probeAttempt < 20 && !foundProbe; probeAttempt++)
    {
// The log filename depends on the build.  With LOG_TO_FILE, which
        // only the Windows projects define, sipxInitialize hardcodes
        // sipXtapi.log and sipxConfigSetLogFile is compiled out.  Without
        // it, the name setUp passes to sipxConfigSetLogFile is used.
        FILE* logFile = fopen("sipXtapi.log", "rb") ;
        if (logFile == NULL)
        {
            logFile = fopen("sipXtapiTests.txt", "rb") ;
        }

        if (logFile == NULL)
        {
            OsTask::delay(100) ;
            continue ;
        }

        while (fgets(logLine, sizeof(logLine), logFile) != NULL)
        {
            if (strstr(logLine, probeUri) != NULL)
            {
                foundProbe = TRUE ;
                break ;
            }
        }

        fseek(logFile, 0, SEEK_END) ;
        logSize = ftell(logFile) ;
        fclose(logFile) ;

        if (!foundProbe)
        {
            OsTask::delay(100) ;
        }
    }

    char probeMsg[512] ;
    sprintf(probeMsg,
            "no log output after another instance uninitialized -- OsSysLog "
            "was shut down while instances were still live; probeUri=%s "
            "attempts=%d logSize=%ld",
            probeUri, probeAttempt, logSize) ;
    CPPUNIT_ASSERT_MESSAGE(probeMsg, foundProbe) ;

    checkForLeaks() ;
}

void sipXtapiTestSuite::testReinitializeSimple() 
{
    SIPX_INST hInst1 = NULL ;
    SIPX_INST hInst2 = NULL ;
    SIPX_RESULT rc ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestReinitializeSimple (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        rc = sipxInitialize(&hInst1, 8000, 8000, 8001, 8050, 32, NULL, "127.0.0.1") ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        rc = sipxInitialize(&hInst2, 9100, 9100, 9101, 9050, 32, NULL, "127.0.0.1") ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

        rc = sipxReInitialize(&hInst1, 8000, 8000, 8001, 8050, 32, NULL, "127.0.0.1") ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        rc = sipxReInitialize(&hInst2, 9100, 9100, 9101, 9050, 32, NULL, "127.0.0.1") ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

        rc = sipxUnInitialize(hInst2);
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        rc = sipxUnInitialize(hInst1);
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
    }

    checkForLeaks() ;
}


void sipXtapiTestSuite::testReinitializeCall() 
{
    SIPX_RESULT rc ;
    bool        bRC ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestReinitializeCall (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        SIPX_INST hInst1 ;
        SIPX_INST hInst2 ;

        /*
         * Create instances
         */

        rc = sipxInitialize(&hInst1, 8001, 8001, 8002, 8050, 32, NULL, "127.0.0.1") ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

        rc = sipxInitialize(&hInst2, 8003, 8003, 8004, 8060, 32, NULL, "127.0.0.1") ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

        /*
         * Verify we can make a call
         */
        doCallBasic(hInst1, "sip:calling@127.0.0.1:8001", "sip:calling@127.0.0.1:8001",
                    hInst2, "sip:called@127.0.0.1:8003", "sip:called@127.0.0.1:8003", 
                    NULL) ;


        /*
         * Setup a call, but leave it up
         */

        SIPX_CALL hCallingCall, hCalledCall ;
        SIPX_LINE hCallingLine, hCalledLine ;

        doCallBasicSetup(hInst1, "sip:calling@127.0.0.1:8001", "sip:calling@127.0.0.1:8001",
                         hInst2, "sip:called@127.0.0.1:8003", "sip:called@127.0.0.1:8003", 
                         hCallingCall, hCallingLine,
                         hCalledCall, hCalledLine) ;

        OsTask::delay(TEST_DELAY) ;

        EventValidator validatorCalling("testReinitializeCall.calling") ;
        EventValidator validatorCalled("testReinitializeCall.called") ;

        rc = sipxEventListenerAdd(hInst1, UniversalEventValidatorCallback, &validatorCalling) ;
        CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;

        rc = sipxEventListenerAdd(hInst2, UniversalEventValidatorCallback, &validatorCalled) ;        
        CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;
                    
        /*
         * Reinitialize again
         */
        rc = sipxReInitialize(&hInst1, 8001, 8001, 8002, 8050, 32, NULL, "127.0.0.1") ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

        rc = sipxReInitialize(&hInst2, 8003, 8003, 8004, 8060, 32, NULL, "127.0.0.1") ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

        rc = sipxEventListenerAdd(hInst1, UniversalEventValidatorCallback, &validatorCalling) ;
        CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;

        rc = sipxEventListenerAdd(hInst2, UniversalEventValidatorCallback, &validatorCalled) ;        
        CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;

        bRC = validatorCalling.waitForCallEvent(hCallingLine, hCallingCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_SHUTDOWN, true) ;
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalled.waitForCallEvent(hCalledLine, hCalledCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_SHUTDOWN, true) ;
        CPPUNIT_ASSERT(bRC) ;

        rc = sipxEventListenerRemove(hInst1, UniversalEventValidatorCallback, &validatorCalling) ;
        CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;
        rc = sipxEventListenerRemove(hInst2, UniversalEventValidatorCallback, &validatorCalled) ;
        CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;   

        /*
         * Verify we can continue to make calls
         */
        doCallBasic(hInst1, "sip:calling@127.0.0.1:8001", "sip:calling@127.0.0.1:8001",
                    hInst2, "sip:called@127.0.0.1:8003", "sip:called@127.0.0.1:8003", 
                    NULL) ;



        /**
         * Cleanup
         */
        rc = sipxUnInitialize(hInst1) ;
        rc = sipxUnInitialize(hInst2) ;
    }

    OsTask::delay(TEST_DELAY) ;
    checkForLeaks() ;
}

void sipXtapiTestSuite::testReinitializeLine() 
{
    SIPX_INST hInst = NULL ;
    SIPX_RESULT rc ;
    SIPX_LINE hLine;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestReinitializeLine (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        TestRegistrar testRegistrar;
        testRegistrar.init();

        rc = sipxInitialize(&hInst, 8000, 8000, 8001, 8050, 32, NULL, "127.0.0.1") ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

        rc = sipxConfigSetOutboundProxy(hInst, "127.0.0.1:5070") ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        
        rc = sipxLineAdd(hInst, "sip:mike@127.0.0.1:8000", &hLine) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        
        rc = sipxLineAddCredential(hLine, "mike", "1234", "TestRegistrar") ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

        rc = sipxLineRegister(hLine, true) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

        rc = sipxLineRegister(hLine, false) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

        sipxReInitialize(&hInst, 8000, 8000, 8001, 8050, 32, NULL, "127.0.0.1") ;

        rc = sipxConfigSetOutboundProxy(hInst, "127.0.0.1:5070") ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

        rc = sipxLineAdd(hInst, "sip:mike@127.0.0.1:8000", &hLine) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        
        rc = sipxLineAddCredential(hLine, "mike", "1234", "TestRegistrar") ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

        rc = sipxLineRegister(hLine, true) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

        rc = sipxLineRegister(hLine, false) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

        rc = sipxLineRemove(hLine) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        
        sipxUnInitialize(hInst);
    }

    checkForLeaks() ;
}


void sipXtapiTestSuite::testReinitializeConference() 
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestReinitializeConference (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);
    }

    checkForLeaks() ;
}

void sipXtapiTestSuite::testReinitializePub() 
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestReinitializePub (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);
    }

    checkForLeaks() ;
}

void sipXtapiTestSuite::testReinitializeSub() 
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestReinitializeSub (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);
    }

    checkForLeaks() ;
}


bool SIPX_CALLING_CONVENTION config_callback(SIPX_EVENT_CATEGORY category,
                     void* pInfo,
                     void* pUserData)
{
    if (category == EVENT_CATEGORY_CONFIG)
    {
        SIPX_CONFIG_INFO* pConfigInfo = (SIPX_CONFIG_INFO*) pInfo;

        g_recorder.addEvent(pConfigInfo);
    }
    
    return true;
}


bool SIPX_CALLING_CONVENTION codec_CallBack_Place(SIPX_EVENT_CATEGORY category,
                          void* pInfo,
                          void* pUserData)
{
    // TODO - re-write this for the new Media events
    /*
    if (category == EVENT_CATEGORY_CALLSTATE)
    {
        SIPX_CALLSTATE_INFO* pCallInfo = (SIPX_CALLSTATE_INFO*)pInfo;
        g_recorder.addEvent(pCallInfo->hLine, pCallInfo->event, pCallInfo->cause) ;
        if (pCallInfo->cause == CALLSTATE_CAUSE_AUDIO_START)
        {
            char szMsg[128];

            sprintf(szMsg, "Codec %s", pCallInfo->codecs.audioCodec.cName);
            g_recorder.addMsgString(pCallInfo->hLine, (const char*)szMsg);
        }
    }
    */
    return true;
}

bool SIPX_CALLING_CONVENTION codec_CallBack_Receive(SIPX_EVENT_CATEGORY category, 
                            void* pInfo, 
                            void* pUserData)
{
     
    if (category == EVENT_CATEGORY_CALLSTATE)
    {
        SIPX_CALLSTATE_INFO* pCallInfo = (SIPX_CALLSTATE_INFO*)pInfo;
        SIPX_LINE hLine = pCallInfo->hLine;
        SIPX_CALL hCall = pCallInfo->hCall;
        
        g_recorder2.addEvent(hLine, pCallInfo->event, pCallInfo->cause) ;

        // If we have user data verify the line url against it
        if (pUserData)
        {
            char szBuffer[500] ; 
            size_t nBuffer ;

            if (strlen((const char*) pUserData))
            {
                if (hLine)  // hLine can be 0, and therefore, sipxLineGetURI should fail)
                {
                    CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(hLine, szBuffer, sizeof(szBuffer), nBuffer), SIPX_RESULT_SUCCESS) ;

                    // an event can come in with what appears to be the "wrong" user data.
                    // for instance, if a listener is removed and a new listener is added immediately with new user data.
                    // printf("comparing %s to %s\n", pUserData, szBuffer) ;
                    //CPPUNIT_ASSERT(strcmp((char*) pUserData, szBuffer) == 0) ;
                }
            }
            //else
            //{
                //CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(hLine, szBuffer, sizeof(szBuffer), nBuffer), SIPX_RESULT_FAILURE) ;
            //}
        }

        switch(pCallInfo->event)
        {
            case CALLSTATE_OFFERING:
                sipxCallAccept(hCall) ;
                break ;
            case CALLSTATE_ALERTING:
                sipxCallAnswer(hCall) ;
                break ;
            case CALLSTATE_DISCONNECTED:
                sipxCallDestroy(hCall) ; 
                break ;
/*
            case CALLSTATE_AUDIO_EVENT:
                if (pCallInfo->cause == CALLSTATE_CAUSE_AUDIO_START)
                {
                    char szMsg[128];

                    sprintf(szMsg, "Codec %s", pCallInfo->codecs.audioCodec.cName);
                    g_recorder2.addMsgString(pCallInfo->hLine, (const char*)szMsg);
                }
                break;
*/
            default:
                break ;
        }
    }     
    
    return true;
}


void sipXtapiTestSuite::testConfigCodecPreferences()
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestConfigCodecPreferences (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);
        SIPX_CALL hCall ;
        SIPX_LINE hLine ;
        SIPX_LINE hReceivingLine;

        sipxConfigSetLogLevel(LOG_LEVEL_DEBUG);
        sipxConfigSetLogFile("codec.log");
    
        g_recorder.clear() ;
        g_recorder2.clear() ;    

        sipxConfigSetAudioCodecPreferences(g_hInst, AUDIO_CODEC_BW_HIGH);
        sipxConfigSetAudioCodecPreferences(g_hInst2, AUDIO_CODEC_BW_HIGH);

        // Setup Auto-answer call back
        sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine, CONTACT_AUTO);
        sipxEventListenerAdd(g_hInst2, codec_CallBack_Receive, NULL) ;

        sipxEventListenerAdd(g_hInst, codec_CallBack_Place, NULL) ;

        createCall(&hLine, &hCall) ;

        sipxCallConnect(hCall, "sip:foo@127.0.0.1:9100") ;
        OsTask::delay(CALL_DELAY*2) ;
        
        int connectionId = -1;
        
        CPPUNIT_ASSERT_EQUAL(sipxCallGetConnectionId(hCall, connectionId), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT(connectionId != -1) ;
                
        destroyCall(hCall) ;
        OsTask::delay(CALL_DELAY * 4) ;

        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, codec_CallBack_Place, NULL), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, codec_CallBack_Receive, NULL), SIPX_RESULT_SUCCESS) ;

        g_recorder.addCompareEvent(hLine, CALLSTATE_DIALTONE, CALLSTATE_CAUSE_NORMAL) ;
        g_recorder.addCompareEvent(hLine, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL) ;
        g_recorder.addCompareEvent(hLine, CALLSTATE_REMOTE_ALERTING, CALLSTATE_CAUSE_NORMAL) ;
        g_recorder.addCompareEvent(hLine, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL) ;
//        g_recorder.addCompareEvent(hLine, CALLSTATE_AUDIO_EVENT, CALLSTATE_CAUSE_AUDIO_START) ;
//        g_recorder.addCompareMsgString(hLine, "Codec IPCMWB");
//        g_recorder.addCompareEvent(hLine, CALLSTATE_BRIDGED, CALLSTATE_CAUSE_NORMAL) ;
//        g_recorder.addCompareEvent(hLine, CALLSTATE_AUDIO_EVENT, CALLSTATE_CAUSE_AUDIO_STOP) ;
        g_recorder.addCompareEvent(hLine, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL) ;
        g_recorder.addCompareEvent(hLine, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL) ;    
        CPPUNIT_ASSERT(g_recorder.compare()) ;

        g_recorder2.addCompareEvent(hReceivingLine, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_NORMAL) ;
        g_recorder2.addCompareEvent(hReceivingLine, CALLSTATE_OFFERING, CALLSTATE_CAUSE_NORMAL) ;
        g_recorder2.addCompareEvent(hReceivingLine, CALLSTATE_ALERTING, CALLSTATE_CAUSE_NORMAL) ;
        g_recorder2.addCompareEvent(hReceivingLine, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL) ;
//        g_recorder2.addCompareEvent(hReceivingLine, CALLSTATE_AUDIO_EVENT, CALLSTATE_CAUSE_AUDIO_START) ;
//        g_recorder2.addCompareMsgString(hReceivingLine, "Codec IPCMWB");
//        g_recorder2.addCompareEvent(hReceivingLine, CALLSTATE_AUDIO_EVENT, CALLSTATE_CAUSE_AUDIO_STOP) ;
        g_recorder2.addCompareEvent(hReceivingLine, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL) ;
        g_recorder2.addCompareEvent(hReceivingLine, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL) ;
        CPPUNIT_ASSERT(g_recorder2.compare()) ;
    
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine), SIPX_RESULT_SUCCESS);
    }
    OsTask::delay(TEST_DELAY) ;
} 


void sipXtapiTestSuite::testConfigEnableStunSuccess() 
{
    SIPX_INST hInst = NULL ;
    SIPX_RESULT rc ;
    bool bRC ;

    EventValidator validator("validator") ;
    validator.ignoreEventCategory(EVENT_CATEGORY_MEDIA);

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestConfigEnableStunSuccess (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);        

        setupStunServer() ;
        setStunServerValidator(&validator) ;
        setStunServerMode(TEST_NORMAL) ;        
        
        rc = sipxInitialize(&hInst, 11234, 11234, 8031, 8050) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);
        rc = sipxEventListenerAdd(hInst, UniversalEventValidatorCallback, &validator) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);

        SIPX_CONTACT_ADDRESS addresses[10] ;
        size_t nContacts ;
        sipxConfigGetLocalContacts(hInst, addresses, 10, nContacts) ;

        validator.addMarker("Waiting for STUN success") ;
        rc = sipxConfigEnableStun(hInst, "127.0.0.1", DEFAULT_STUN_PORT, 2) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        
        validator.setMaxLookhead(20) ;

        // Get Initial
        for (size_t i=0; i<nContacts; i++)
        {
            if ((strcmp(addresses[i].cInterface, "loopback")) != 0 && 
                    addresses[i].eTransportType == TRANSPORT_UDP)
            {
                // SUCCESSes may come in in a different rate
                bRC = validator.waitForMessage(0, "TestStunServerTask::handleStunMessage", false) ;
                CPPUNIT_ASSERT(bRC) ;
                bRC = validator.waitForMessage(0, "TestStunServerTask::Sending Response", false) ;
                CPPUNIT_ASSERT(bRC) ;
                bRC = validator.waitForConfigEvent(CONFIG_STUN_SUCCESS, false) ;
                CPPUNIT_ASSERT(bRC) ;
            }
        }

        // Wait for a few refreshes
        for (size_t loop=0; loop<2; loop++)
        {
            for (size_t i=0; i<nContacts; i++)
            {
                if ((strcmp(addresses[i].cInterface, "loopback")) != 0 && 
                    addresses[i].eTransportType == TRANSPORT_UDP)
                {
                    bRC = validator.waitForMessage(0, "TestStunServerTask::handleStunMessage", false) ;
                    CPPUNIT_ASSERT(bRC) ;
                    bRC = validator.waitForMessage(0, "TestStunServerTask::Sending Response", false) ;            
                    CPPUNIT_ASSERT(bRC) ;
                }
            }            
        }

        setStunServerValidator(NULL) ;

        rc = sipxEventListenerRemove(hInst, UniversalEventValidatorCallback, &validator) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);

        rc = sipxUnInitialize(hInst);
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);

        teardownStunServer() ;
    }

    // Does not create a call -- no need to pause
    // OsTask::delay(TEST_DELAY) ;    
    checkForLeaks() ;
}


void sipXtapiTestSuite::testConfigEnableStunNoResponse() 
{
    SIPX_INST hInst = NULL ;
    SIPX_RESULT rc ;
    bool bRC ;

    EventValidator validator("validator") ;
    validator.ignoreEventCategory(EVENT_CATEGORY_MEDIA);

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestConfigEnableStunNoResponse (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);        

        setupStunServer() ;
        setStunServerValidator(&validator) ;
        setStunServerMode(TEST_NO_RESPONSE) ;        
        
        rc = sipxInitialize(&hInst, 11234, 11234, 8031, 8050) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);
        rc = sipxEventListenerAdd(hInst, UniversalEventValidatorCallback, &validator) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);

        SIPX_CONTACT_ADDRESS addresses[10] ;
        size_t nContacts ;
        sipxConfigGetLocalContacts(hInst, addresses, 10, nContacts) ;

        validator.addMarker("Waiting for STUN failure") ;
        rc = sipxConfigEnableStun(hInst, "127.0.0.1", DEFAULT_STUN_PORT, 10) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

        validator.setMaxLookhead(20) ;

        // Get Initial
        for (size_t i=0; i<nContacts; i++)
        {
            if ((strcmp(addresses[i].cInterface, "loopback")) != 0 && 
                    addresses[i].eTransportType == TRANSPORT_UDP)
            {
                // SUCCESSes may come in in a different rate
                bRC = validator.waitForMessage(0, "TestStunServerTask::handleStunMessage", false) ;
                CPPUNIT_ASSERT(bRC) ;
                bRC = validator.waitForMessage(0, "TestStunServerTask::Dropping Response", false) ;
                CPPUNIT_ASSERT(bRC) ;
                bRC = validator.waitForMessage(0, "TestStunServerTask::handleStunMessage", false) ;
                CPPUNIT_ASSERT(bRC) ;
                bRC = validator.waitForMessage(0, "TestStunServerTask::Dropping Response", false) ;
                CPPUNIT_ASSERT(bRC) ;
                bRC = validator.waitForMessage(0, "TestStunServerTask::handleStunMessage", false) ;
                CPPUNIT_ASSERT(bRC) ;
                bRC = validator.waitForMessage(0, "TestStunServerTask::Dropping Response", false) ;
                CPPUNIT_ASSERT(bRC) ;
                bRC = validator.waitForConfigEvent(CONFIG_STUN_FAILURE, false) ;
                CPPUNIT_ASSERT(bRC) ;
            }
        }
      
        setStunServerValidator(NULL) ;

        rc = sipxEventListenerRemove(hInst, UniversalEventValidatorCallback, &validator) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);

        rc = sipxUnInitialize(hInst);
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);

        teardownStunServer() ;
    }

    // Does not create a call -- no need to pause
    // OsTask::delay(TEST_DELAY) ;    
    checkForLeaks() ;
}


void sipXtapiTestSuite::testConfigEnableStunError() 
{
    SIPX_INST hInst = NULL ;
    SIPX_RESULT rc ;
    bool bRC ;

    EventValidator validator("validator") ;
    validator.ignoreEventCategory(EVENT_CATEGORY_MEDIA);

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestConfigEnableStunError (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);        

        setupStunServer() ;
        setStunServerValidator(&validator) ;
        setStunServerMode(TEST_RETURN_ERROR) ;        
        
        rc = sipxInitialize(&hInst, 11234, 11234, 8031, 8050) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);
        rc = sipxEventListenerAdd(hInst, UniversalEventValidatorCallback, &validator) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);

        SIPX_CONTACT_ADDRESS addresses[10] ;
        size_t nContacts ;
        sipxConfigGetLocalContacts(hInst, addresses, 10, nContacts) ;

        validator.addMarker("Waiting for STUN failure") ;
        rc = sipxConfigEnableStun(hInst, "127.0.0.1", DEFAULT_STUN_PORT, 10) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

        validator.setMaxLookhead(20) ;

        // Get Initial
        for (size_t i=0; i<nContacts; i++)
        {
            if ((strcmp(addresses[i].cInterface, "loopback")) != 0 && 
                    addresses[i].eTransportType == TRANSPORT_UDP)
            {
                // SUCCESSes may come in in a different rate
                bRC = validator.waitForMessage(0, "TestStunServerTask::handleStunMessage", false) ;
                CPPUNIT_ASSERT(bRC) ;
                bRC = validator.waitForMessage(0, "TestStunServerTask::Send Error", false) ;
                CPPUNIT_ASSERT(bRC) ;
                bRC = validator.waitForConfigEvent(CONFIG_STUN_FAILURE, false) ;
                CPPUNIT_ASSERT(bRC) ;
            }
        }
      
        setStunServerValidator(NULL) ;

        rc = sipxEventListenerRemove(hInst, UniversalEventValidatorCallback, &validator) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);

        rc = sipxUnInitialize(hInst);
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);

        teardownStunServer() ;
    }

    // Does not create a call -- no need to pause
    // OsTask::delay(TEST_DELAY) ;    
    checkForLeaks() ;
}


void sipXtapiTestSuite::testConfigEnableStunDropOdd() 
{
    SIPX_INST hInst = NULL ;
    SIPX_RESULT rc ;
    bool bRC ;

    EventValidator validator("validator") ;
    validator.ignoreEventCategory(EVENT_CATEGORY_MEDIA);

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestConfigEnableStunDropOdd (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);        

        setupStunServer() ;
        setStunServerValidator(&validator) ;
        setStunServerMode(TEST_DROP_ODD_REQUEST) ;
        
        rc = sipxInitialize(&hInst, 11234, 11234, 8031, 8050) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        rc = sipxEventListenerAdd(hInst, UniversalEventValidatorCallback, &validator) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

        SIPX_CONTACT_ADDRESS addresses[10] ;
        size_t nContacts ;
        sipxConfigGetLocalContacts(hInst, addresses, 10, nContacts) ;

        if (nContacts == 3)
        {
            validator.addMarker("Waiting for STUN success") ;
            rc = sipxConfigEnableStun(hInst, "127.0.0.1", DEFAULT_STUN_PORT, 2) ;
            CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

            // Get Initial
            bRC = validator.waitForMessage(0, "TestStunServerTask::handleStunMessage", false) ;
            CPPUNIT_ASSERT(bRC) ;
            bRC = validator.waitForMessage(0, "TestStunServerTask::Sending Response (odd)", false) ;
            CPPUNIT_ASSERT(bRC) ;
            bRC = validator.waitForConfigEvent(CONFIG_STUN_SUCCESS, false) ;
            CPPUNIT_ASSERT(bRC) ;

            // Wait for a few refreshes
            bRC = validator.waitForMessage(0, "TestStunServerTask::handleStunMessage", false) ;
            CPPUNIT_ASSERT(bRC) ;
            bRC = validator.waitForMessage(0, "TestStunServerTask::Dropping Response (odd)", false) ;            
            CPPUNIT_ASSERT(bRC) ;
            bRC = validator.waitForMessage(0, "TestStunServerTask::handleStunMessage", false) ;
            CPPUNIT_ASSERT(bRC) ;
            bRC = validator.waitForMessage(0, "TestStunServerTask::Sending Response (odd)", false) ;            
            CPPUNIT_ASSERT(bRC) ;
            bRC = validator.waitForMessage(0, "TestStunServerTask::handleStunMessage", false) ;
            CPPUNIT_ASSERT(bRC) ;
            bRC = validator.waitForMessage(0, "TestStunServerTask::Dropping Response (odd)", false) ;            
            CPPUNIT_ASSERT(bRC) ;
            bRC = validator.waitForMessage(0, "TestStunServerTask::handleStunMessage", false) ;
            CPPUNIT_ASSERT(bRC) ;
            bRC = validator.waitForMessage(0, "TestStunServerTask::Sending Response (odd)", false) ;            
            CPPUNIT_ASSERT(bRC) ;
        }
        else
        {
            printf("\nWARNING: testConfigEnableStunDropOdd skipped -- doesn't work with multiple interfaces\n") ;
        }

        setStunServerValidator(NULL) ;

        rc = sipxEventListenerRemove(hInst, UniversalEventValidatorCallback, &validator) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);

        rc = sipxUnInitialize(hInst);
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);

        teardownStunServer() ;
    }

    // Does not create a call -- no need to pause
    // OsTask::delay(TEST_DELAY) ;    
    checkForLeaks() ;
}

void sipXtapiTestSuite::testConfigEnableStunDropEven() 
{
    SIPX_INST hInst = NULL ;
    SIPX_RESULT rc ;
    bool bRC ;

    EventValidator validator("validator") ;
    validator.ignoreEventCategory(EVENT_CATEGORY_MEDIA);

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestConfigEnableStunDropEven (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);        

        setupStunServer() ;
        setStunServerValidator(&validator) ;
        setStunServerMode(TEST_DROP_EVEN_REQUEST) ;        
        
        rc = sipxInitialize(&hInst, 11234, 11234, 8031, 8050) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);
        rc = sipxEventListenerAdd(hInst, UniversalEventValidatorCallback, &validator) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);

        SIPX_CONTACT_ADDRESS addresses[10] ;
        size_t nContacts ;
        sipxConfigGetLocalContacts(hInst, addresses, 10, nContacts) ;

        if (nContacts == 3)
        {
            validator.addMarker("Waiting for STUN success") ;
            rc = sipxConfigEnableStun(hInst, "127.0.0.1", DEFAULT_STUN_PORT, 2) ;
            CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

            // Get Initial
            bRC = validator.waitForMessage(0, "TestStunServerTask::handleStunMessage", false) ;
            CPPUNIT_ASSERT(bRC) ;
            bRC = validator.waitForMessage(0, "TestStunServerTask::Dropping Response (even)", false) ;            
            CPPUNIT_ASSERT(bRC) ;
            bRC = validator.waitForMessage(0, "TestStunServerTask::handleStunMessage", false) ;
            CPPUNIT_ASSERT(bRC) ;
            bRC = validator.waitForMessage(0, "TestStunServerTask::Sending Response (even)", false) ;
            CPPUNIT_ASSERT(bRC) ;
            bRC = validator.waitForConfigEvent(CONFIG_STUN_SUCCESS, false) ;
            CPPUNIT_ASSERT(bRC) ;

            // Wait for a few refreshes
            bRC = validator.waitForMessage(0, "TestStunServerTask::handleStunMessage", false) ;
            CPPUNIT_ASSERT(bRC) ;
            bRC = validator.waitForMessage(0, "TestStunServerTask::Dropping Response (even)", false) ;            
            CPPUNIT_ASSERT(bRC) ;
            bRC = validator.waitForMessage(0, "TestStunServerTask::handleStunMessage", false) ;
            CPPUNIT_ASSERT(bRC) ;
            bRC = validator.waitForMessage(0, "TestStunServerTask::Sending Response (even)", false) ;            
            CPPUNIT_ASSERT(bRC) ;
            bRC = validator.waitForMessage(0, "TestStunServerTask::handleStunMessage", false) ;
            CPPUNIT_ASSERT(bRC) ;
            bRC = validator.waitForMessage(0, "TestStunServerTask::Dropping Response (even)", false) ;            
            CPPUNIT_ASSERT(bRC) ;
            bRC = validator.waitForMessage(0, "TestStunServerTask::handleStunMessage", false) ;
            CPPUNIT_ASSERT(bRC) ;
            bRC = validator.waitForMessage(0, "TestStunServerTask::Sending Response (even)", false) ;            
            CPPUNIT_ASSERT(bRC) ;
        }
        else
        {
            printf("\nWARNING: testConfigEnableStunDropOdd skipped -- doesn't work with multiple interfaces\n") ;
        }

        setStunServerValidator(NULL) ;

        rc = sipxEventListenerRemove(hInst, UniversalEventValidatorCallback, &validator) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);

        rc = sipxUnInitialize(hInst);
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);

        teardownStunServer() ;
    }

    // Does not create a call -- no need to pause
    // OsTask::delay(TEST_DELAY) ;    
    checkForLeaks() ;
}

void sipXtapiTestSuite::testConfigEnableStunDelay() 
{
    SIPX_INST hInst = NULL ;
    SIPX_RESULT rc ;
    bool bRC ;

    EventValidator validator("validator") ;
    validator.ignoreEventCategory(EVENT_CATEGORY_MEDIA);

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestConfigEnableStunDelay (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);        

        setupStunServer() ;
        setStunServerValidator(&validator) ;
        setStunServerMode(TEST_DELAY_RESPONSE) ;
        validator.ignoreMessages() ;
        
        rc = sipxInitialize(&hInst, 11234, 11234, 8031, 8050) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);
        rc = sipxEventListenerAdd(hInst, UniversalEventValidatorCallback, &validator) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);

        SIPX_CONTACT_ADDRESS addresses[10] ;
        size_t nContacts ;
        sipxConfigGetLocalContacts(hInst, addresses, 10, nContacts) ;

        validator.addMarker("Waiting for STUN success") ;
        rc = sipxConfigEnableStun(hInst, "127.0.0.1", DEFAULT_STUN_PORT, 2) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;             

        // Get Initial
        for (size_t i=0; i<nContacts; i++)
        {
            if ((strcmp(addresses[i].cInterface, "loopback")) != 0 && 
                    addresses[i].eTransportType == TRANSPORT_UDP)
            {
                // SUCCESSes may come in in a different rate
                bRC = validator.waitForConfigEvent(CONFIG_STUN_SUCCESS, true) ;
                CPPUNIT_ASSERT(bRC) ;
            }
        }

        setStunServerValidator(NULL) ;

        rc = sipxEventListenerRemove(hInst, UniversalEventValidatorCallback, &validator) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);

        rc = sipxUnInitialize(hInst);
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);

        teardownStunServer() ;
    }

    // Does not create a call -- no need to pause
    // OsTask::delay(TEST_DELAY) ;    
    checkForLeaks() ;
}

void sipXtapiTestSuite::testConfigCrlfKeepAlive() 
{
    SIPX_INST hInst = NULL ;
    SIPX_RESULT rc ;
    UtlString serverAddress ;

    OsSocket::getHostIpByName("sbc.pingtel.com", &serverAddress) ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestConfigCrlfKeepAlive (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);        

        rc = sipxInitialize(&hInst, 0, 0, 8031, 8050) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);

        rc = sipxConfigKeepAliveAdd(hInst, 1, SIPX_KEEPALIVE_CRLF, serverAddress, 5060, 1) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);

        OsTask::delay(5000) ;

        rc = sipxConfigKeepAliveRemove(hInst, 1, SIPX_KEEPALIVE_STUN, serverAddress, 5060) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_FAILURE);

        rc = sipxConfigKeepAliveRemove(hInst, 1, SIPX_KEEPALIVE_CRLF, serverAddress, 5060) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);

        rc = sipxConfigKeepAliveRemove(hInst, 1, SIPX_KEEPALIVE_CRLF, serverAddress, 5060) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_FAILURE);

        rc = sipxUnInitialize(hInst);
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);
    }

    // Does not create a call -- no need to pause
    // OsTask::delay(TEST_DELAY) ;    
    checkForLeaks() ;
}


void sipXtapiTestSuite::testConfigStunKeepAlive()
{
    SIPX_INST hInst = NULL ;
    SIPX_RESULT rc ;
    bool bRC ;

    EventValidator validator("validator") ;
    validator.ignoreEventCategory(EVENT_CATEGORY_MEDIA);

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestConfigStunKeepAlive (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);        

        setupStunServer() ;
        setStunServerValidator(&validator) ;
        setStunServerMode(TEST_NORMAL) ;        
        
        rc = sipxInitialize(&hInst, 11234, 11234, 8031, 8050) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);
        rc = sipxEventListenerAdd(hInst, UniversalEventValidatorCallback, &validator) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);

        rc = sipxConfigKeepAliveAdd(hInst, 1, SIPX_KEEPALIVE_STUN, "127.0.0.1", 3478, 1) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);
        
        bRC = validator.waitForMessage(0, "TestStunServerTask::handleStunMessage", true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validator.waitForMessage(0, "TestStunServerTask::Sending Response", true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validator.waitForMessage(0, "TestStunServerTask::handleStunMessage", true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validator.waitForMessage(0, "TestStunServerTask::Sending Response", true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validator.waitForMessage(0, "TestStunServerTask::handleStunMessage", true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validator.waitForMessage(0, "TestStunServerTask::Sending Response", true) ;
        CPPUNIT_ASSERT(bRC) ;

        setStunServerValidator(NULL) ;

        rc = sipxEventListenerRemove(hInst, UniversalEventValidatorCallback, &validator) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);

        rc = sipxUnInitialize(hInst);
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);

        teardownStunServer() ;
    }

    // Does not create a call -- no need to pause
    // OsTask::delay(TEST_DELAY) ;    
    checkForLeaks() ;
}


void sipXtapiTestSuite::testConfigStunKeepAliveOnce()
{
    SIPX_INST hInst = NULL ;
    SIPX_RESULT rc ;
    bool bRC ;

    EventValidator validator("validator") ;
    validator.ignoreEventCategory(EVENT_CATEGORY_MEDIA);

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestConfigStunKeepAliveOnce (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);        

        setupStunServer() ;
        setStunServerValidator(&validator) ;
        setStunServerMode(TEST_NORMAL) ;        
        
        rc = sipxInitialize(&hInst, 11234, 11234, 8031, 8050) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);
        rc = sipxEventListenerAdd(hInst, UniversalEventValidatorCallback, &validator) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);

        rc = sipxConfigKeepAliveAdd(hInst, 1, SIPX_KEEPALIVE_STUN, "127.0.0.1", 3478, 0) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);
        
        bRC = validator.waitForMessage(0, "TestStunServerTask::handleStunMessage", true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validator.waitForMessage(0, "TestStunServerTask::Sending Response", true) ;
        CPPUNIT_ASSERT(bRC) ;

        OsTask::delay(1000) ;

        bRC = !validator.validateNoWaitingEvent() ;
        CPPUNIT_ASSERT(bRC) ;

        setStunServerValidator(NULL) ;

        rc = sipxEventListenerRemove(hInst, UniversalEventValidatorCallback, &validator) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);

        rc = sipxUnInitialize(hInst);
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);

        teardownStunServer() ;
    }

    // Does not create a call -- no need to pause
    // OsTask::delay(TEST_DELAY) ;    
    checkForLeaks() ;
}

void sipXtapiTestSuite::testConfigKeepAliveNoStop() 
{
    SIPX_INST hInst = NULL ;
    SIPX_RESULT rc ;
    UtlString serverAddress ;

    OsSocket::getHostIpByName("sbc.pingtel.com", &serverAddress) ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestConfigKeepAliveNoStop (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);        

        rc = sipxInitialize(&hInst, 0, 0, 8031, 8050) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);

        rc = sipxConfigKeepAliveAdd(hInst, 1, SIPX_KEEPALIVE_CRLF, serverAddress, 5060, 1) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);

        OsTask::delay(1000) ;

        rc = sipxUnInitialize(hInst);
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS);
    }

    // Does not create a call -- no need to pause
    // OsTask::delay(TEST_DELAY) ;    
    checkForLeaks() ;
}

// A calls B, B answers, A hangs up
void sipXtapiTestSuite::testConfigEnableShortNames() 
{
    bool bRC ;
    EventValidator validatorCalling("testConfigEnableShortNames.calling") ;
    EventValidator validatorCalled("testConfigEnableShortNames.called") ;
    validatorCalling.ignoreEventCategory(EVENT_CATEGORY_MEDIA);
    validatorCalled.ignoreEventCategory(EVENT_CATEGORY_MEDIA);

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestConfigEnableShortNames (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);
        SIPX_CALL hCall ;
        SIPX_LINE hLine ;
        SIPX_LINE hReceivingLine;     

        validatorCalling.reset() ;
        validatorCalled.reset() ;
        validatorCalling.ignoreEventCategory(EVENT_CATEGORY_MEDIA);
        validatorCalled.ignoreEventCategory(EVENT_CATEGORY_MEDIA);

        sipxConfigEnableSipShortNames(g_hInst, true);

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
//        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_CAUSE_AUDIO_START, true) ;
//        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
//        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_CAUSE_AUDIO_START, true) ;
//        CPPUNIT_ASSERT(bRC) ;
                
        int connectionId = -1;
        
        CPPUNIT_ASSERT_EQUAL(sipxCallGetConnectionId(hCall, connectionId), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT(connectionId != -1) ;
                
        SIPX_CALL hDestroyedCall = hCall ;
        destroyCall(hCall) ;

        // Validate Calling Side
//        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_CAUSE_AUDIO_STOP, true) ;
//        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
//        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_CAUSE_AUDIO_STOP, true) ;
//        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoAnswerCallback, NULL), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled), SIPX_RESULT_SUCCESS) ;
    
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine), SIPX_RESULT_SUCCESS);

        sipxConfigEnableSipShortNames(g_hInst, true);
    }

    OsTask::delay(TEST_DELAY) ;

    checkForLeaks();
}


void sipXtapiTestSuite::testUtilUrlParse() 
{
    char szUsername[100] ;
    char szHostname[100] ;
    int  iPort ;
    SIPX_RESULT rc ;

    rc = sipxUtilUrlParse("sip:user@host:1000", szUsername, szHostname, &iPort) ;
    CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT(strcmp(szUsername, "user") == 0) ;
    CPPUNIT_ASSERT(strcmp(szHostname, "host") == 0) ;
    CPPUNIT_ASSERT(iPort == 1000) ;

    rc = sipxUtilUrlParse("sip:user2@host2", szUsername, szHostname, &iPort) ;
    CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT(strcmp(szUsername, "user2") == 0) ;
    CPPUNIT_ASSERT(strcmp(szHostname, "host2") == 0) ;
    CPPUNIT_ASSERT(iPort == -1) ;

    rc = sipxUtilUrlParse(NULL, szUsername, szHostname, &iPort) ;
    CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_FAILURE) ;

    rc = sipxUtilUrlParse("", szUsername, szHostname, &iPort) ;
    CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_FAILURE) ;

    rc = sipxUtilUrlParse("sip:user3@host3", NULL, NULL, NULL) ;
    CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
}


void sipXtapiTestSuite::testUtilUrlUpdate() 
{
    char        szUrl[100] ;
    size_t      nLength ;
    SIPX_RESULT rc ;

    strcpy(szUrl, "<sip:user@host:5060;urlparam=1234>;otherparam=3422") ;
    nLength = sizeof(szUrl) ;
    rc = sipxUtilUrlUpdate(szUrl, nLength, "user2", "host2", 2222) ;
    CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT(strcmp(szUrl, "<sip:user2@host2:2222;urlparam=1234>;otherparam=3422") == 0) ;

    strcpy(szUrl, "<sip:user@host:5060;urlparam=1234>;otherparam=3422") ;
    nLength = sizeof(szUrl) ;
    rc = sipxUtilUrlUpdate(szUrl, nLength, NULL, NULL, -1) ;
    CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT(strcmp(szUrl, "<sip:user@host:5060;urlparam=1234>;otherparam=3422") == 0) ;

    strcpy(szUrl, "<sip:user@host;urlparam=1234>;otherparam=3422") ;
    nLength = sizeof(szUrl) ;
    rc = sipxUtilUrlUpdate(szUrl, nLength, NULL, NULL, -1) ;
    CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT(strcmp(szUrl, "<sip:user@host;urlparam=1234>;otherparam=3422") == 0) ;

    nLength = sizeof(szUrl) ;
    rc = sipxUtilUrlUpdate(NULL, nLength, NULL, NULL, -1) ;
    CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_FAILURE) ;
}


void sipXtapiTestSuite::testUtilUrlGetUrlParam() 
{
    char        szValue[100] ;
    SIPX_RESULT rc ;
 
    rc = sipxUtilUrlGetUrlParam("sip:user@host", "missing", 0, szValue, sizeof(szValue)) ;
    CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_FAILURE) ;
    rc = sipxUtilUrlGetUrlParam("sip:user@host", "missing", 1, szValue, sizeof(szValue)) ;
    CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_FAILURE) ;
    
    rc = sipxUtilUrlGetUrlParam("sip:user@host;param=1234", "param", 0, szValue, sizeof(szValue)) ;
    CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT(strcmp(szValue, "1234") == 0) ;
    rc = sipxUtilUrlGetUrlParam("sip:user@host;param=1234", "missing", 1, szValue, sizeof(szValue)) ;
    CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_FAILURE) ;

    rc = sipxUtilUrlGetUrlParam("<sip:user@host;param=1234>", "param", 0, szValue, sizeof(szValue)) ;
    CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT(strcmp(szValue, "1234") == 0) ;
    rc = sipxUtilUrlGetUrlParam("<sip:user@host;param=1234>", "missing", 1, szValue, sizeof(szValue)) ;
    CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_FAILURE) ;

    rc = sipxUtilUrlGetUrlParam("<sip:user@host>;param=1234", "param", 0, szValue, sizeof(szValue)) ;
    CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_FAILURE) ;
    rc = sipxUtilUrlGetUrlParam("<sip:user@host>param=1234", "missing", 1, szValue, sizeof(szValue)) ;
    CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_FAILURE) ;

    rc = sipxUtilUrlGetUrlParam("<sip:user@host;param=1234;other=2;param=5678>", "param", 0, szValue, sizeof(szValue)) ;
    CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT(strcmp(szValue, "1234") == 0) ;
    rc = sipxUtilUrlGetUrlParam("<sip:user@host;param=1234;other=2;param=5678>", "param", 1, szValue, sizeof(szValue)) ;
    CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT(strcmp(szValue, "5678") == 0) ;
    rc = sipxUtilUrlGetUrlParam("<sip:user@host;param=1234;other=2;param=5678>", "param", 2, szValue, sizeof(szValue)) ;
    CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_FAILURE) ;
    rc = sipxUtilUrlGetUrlParam("<sip:user@host;param=1234;other=2;param=5678>", "other", 0, szValue, sizeof(szValue)) ;
    CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT(strcmp(szValue, "2") == 0) ;

//@}


}
