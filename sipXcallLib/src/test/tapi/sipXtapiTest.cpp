// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include "sipXtapiTest.h"

SIPX_INST g_hInst = NULL ;
EventRecorder g_recorder ;
EventRecorder g_lineRecorder; 

SIPX_INST g_hInst2 = NULL ;
EventRecorder g_recorder2 ;

SIPX_INST g_hInst3 = NULL ;
EventRecorder g_recorder3 ;

int main(int argc, char* argv[])
{
	// Get the top level suite from the registry
	CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

	// Adds the test to the list of tests to run
	CppUnit::TextUi::TestRunner runner ;
	runner.addTest(suite) ;

	// Change the default outputter to a compiler error format outputter
	runner.setOutputter(new CppUnit::CompilerOutputter(&runner.result(), std::cerr)) ;

	// Run the tests.
	bool wasSuccessful = runner.run() ;

	// Return error code 1 if one of the tests failed.
	return wasSuccessful ? 0 : 1 ;
}


CPPUNIT_TEST_SUITE_REGISTRATION( sipXtapiTestSuite );

EventRecorder::EventRecorder() 
{
    int i ;

    // Init events
    for (i=0;i<MAX_EVENTS;i++)
    {
        m_events[i] = NULL ;
    }
    m_numEvents = 0 ;

    // Init compare events
    for (i=0;i<MAX_EVENTS;i++)
    {
        m_compareEvents[i] = NULL ;
    }
    m_numCompareEvents = 0 ;
}


EventRecorder::~EventRecorder() 
{
    clear() ;
}


EventRecorder::clear()
{
    int i ;

    // Clear events
    for (i=0; i<MAX_EVENTS; i++)
    {
        if (m_events[i])
        {
            free(m_events[i]) ;
            m_events[i] = NULL ;
        }
    }
    m_numEvents = 0 ;

    // Clear compare events
    for (i=0; i<MAX_EVENTS; i++)
    {
        if (m_compareEvents[i])
        {
            free(m_compareEvents[i]) ;
            m_compareEvents[i] = NULL ;
        }
    }
    m_numCompareEvents = 0 ;

}


void EventRecorder::addEvent(SIPX_CALLSTATE_MAJOR eMajor, SIPX_CALLSTATE_MINOR eMinor) 
{
    char szBuffer[128] ;

    m_events[m_numEvents++] = strdup(sipxEventToString(eMajor, eMinor, szBuffer, sizeof(szBuffer))) ;
}

void EventRecorder::addCompareEvent(SIPX_CALLSTATE_MAJOR eMajor, SIPX_CALLSTATE_MINOR eMinor) 
{
    char szBuffer[128] ;

    m_compareEvents[m_numCompareEvents++] = strdup(sipxEventToString(eMajor, eMinor, szBuffer, sizeof(szBuffer))) ;
}

void EventRecorder::addEvent(SIPX_LINE_EVENT_TYPE_MAJOR eMajor) 
{
    char szBuffer[128] ;
    m_events[m_numEvents++] = strdup(sipxLineEventToString(eMajor, szBuffer, sizeof(szBuffer))) ;
}

void EventRecorder::addCompareEvent(SIPX_LINE_EVENT_TYPE_MAJOR eMajor)
{
    char szBuffer[128] ;
    m_compareEvents[m_numCompareEvents++] = strdup(sipxLineEventToString(eMajor, szBuffer, sizeof(szBuffer))) ;
}



bool EventRecorder::compare() 
{    
    bool bMatch = false ;    

    char* szRecorded = buildEventStr(m_events, m_numEvents) ;
    char* szCompare = buildEventStr(m_compareEvents, m_numCompareEvents) ;

    if (strcmp(szRecorded, szCompare) == 0)
    {
        bMatch = true ;
    }
    else
    {
        printf("verifyEvent Failed\n") ;
        printf("Recorded:\n%s\n", szRecorded) ;
        printf("Expected:\n%s\n", szCompare) ;        
    }

    free(szRecorded) ;
    free(szCompare) ;
    
    return bMatch ;
}


char* EventRecorder::buildEventStr(char* array[], int nLength)
{
    char* szRC = NULL ;
    int iEvents = nLength ;
    int iSize, i ;

    // Pass 1: calc size
    iSize = 1 ; // For null
    for (i=0; i<iEvents; i++)
    {
        iSize += strlen(array[i]) + 2 ;  // "\t\n"
    }

    // Pass 2: Build String
    szRC = (char*) calloc(iSize, 1) ;
    for (i=0; i<iEvents; i++)
    {
        strcat(szRC, "\t") ;
        strcat(szRC, array[i]) ;
        if ((i+1) < iEvents)
        {
            strcat(szRC, "\n") ;
        }        
    }
  
    return szRC ;
}


sipXtapiTestSuite::sipXtapiTestSuite()    
{

}

void sipXtapiTestSuite::setUp()
{
    if (g_hInst == NULL)
    {
	    sipxInitialize(&g_hInst, 8000, 8000, 8050) ;
    }

    if (g_hInst2 == NULL)
    {
	    sipxInitialize(&g_hInst2, 9000, 9000, 9050) ;
    }

    if (g_hInst3 == NULL)
    {
	    sipxInitialize(&g_hInst3, 10000, 10000, 10050) ;
    }
} 


void sipXtapiTestSuite::tearDown()
{
    if (g_hInst == NULL)
    {
	    sipxInitialize(&g_hInst);
    }

    if (g_hInst2 == NULL)
    {
	    sipxInitialize(&g_hInst2);
    }

    if (g_hInst3 == NULL)
    {
	    sipxInitialize(&g_hInst3);
    }
	
}


/**
 * Test valid bounds: min gain, mid gain, and max gain. 
 */
void sipXtapiTestSuite::testGainAPI() 
{
	int iGainLevel ;	

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


/**
 * Verify mute state and that gain is not modified
 */
void sipXtapiTestSuite::testMuteAPI()
{
	int iGainLevel ;
	bool bMuted ;

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


/*
 * Test valid bounds: min, mid, and max. 
 */
void sipXtapiTestSuite::testVolumeAPI() 
{
	SPEAKER_TYPE type ;
	int iLevel ;

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


void sipXtapiTestSuite::testLineAPI_Add() 
{
	SIPX_LINE	hLine = NULL ;
	SIPX_LINE	hLine2 = NULL ;

	// Add Line
	CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, "sip:bandreasen@pingtel.com", false, &hLine), SIPX_RESULT_SUCCESS) ;	
	CPPUNIT_ASSERT(hLine) ;

	// Re-Add Line
	CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, "sip:bandreasen@pingtel.com", false, &hLine2), SIPX_RESULT_FAILURE) ;
	CPPUNIT_ASSERT(hLine2 == NULL) ;

	CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS) ;
}


void sipXtapiTestSuite::testLineAPI_Remove() 
{
	SIPX_LINE	hLine = NULL ;

	// Add a line to remove
	CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, "sip:removeme@pingtel.com", false, &hLine), SIPX_RESULT_SUCCESS) ;	
	CPPUNIT_ASSERT(hLine) ;

	// Remove it and remove it again
	CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS) ;
	CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_FAILURE) ;

	// Remove something invalid
	CPPUNIT_ASSERT_EQUAL(sipxLineRemove(NULL), SIPX_RESULT_INVALID_ARGS) ;
}


void sipXtapiTestSuite::testLineAPI_Credential() 
{
	SIPX_LINE	hLine = NULL ;

	// Add a line to remove
	CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, "sip:credential@pingtel.com", false, &hLine), SIPX_RESULT_SUCCESS) ;
	CPPUNIT_ASSERT(hLine) ;

	CPPUNIT_ASSERT_EQUAL(sipxLineAddCredential(hLine, "userID", "passwd", "pingtel.com"), SIPX_RESULT_SUCCESS) ;
	CPPUNIT_ASSERT_EQUAL(sipxLineAddCredential(hLine, "userID", "passwd", "pingtel2.com"), SIPX_RESULT_SUCCESS) ;

	CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS) ;
}

void sipXtapiTestSuite::testLineAPI_Get() 
{
	SIPX_LINE	hLine = NULL ;
	SIPX_LINE hLines[32] ;
	size_t nLines ;
	const char* szURI = "sip:removeme@pingtel.com" ;

	// First clear any lines
	CPPUNIT_ASSERT_EQUAL(sipxLineGet(g_hInst, hLines, 32, nLines), SIPX_RESULT_SUCCESS) ;
	for (size_t i=0; i<nLines; i++) 
	{
		CPPUNIT_ASSERT(hLines[i] != NULL) ;
		CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLines[i]), SIPX_RESULT_SUCCESS) ;
	}

	// Make sure the list is empty
	nLines = 203431 ;
	CPPUNIT_ASSERT_EQUAL(sipxLineGet(g_hInst, hLines, 32, nLines), SIPX_RESULT_SUCCESS) ;
	CPPUNIT_ASSERT_EQUAL(nLines, (size_t) 0) ;

	// Add and element and verify it is present
	CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, szURI, false, &hLine), SIPX_RESULT_SUCCESS) ;	
	CPPUNIT_ASSERT(hLine) ;
	
	CPPUNIT_ASSERT_EQUAL(sipxLineGet(g_hInst, hLines, 32, nLines), SIPX_RESULT_SUCCESS) ;
	CPPUNIT_ASSERT_EQUAL(nLines, (size_t) 1) ;	
	CPPUNIT_ASSERT(hLines[0] != NULL) ;

	char cBuf[256] ;
	size_t nActual ;
	CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(hLine, cBuf, sizeof(cBuf), nActual), SIPX_RESULT_SUCCESS) ;
	CPPUNIT_ASSERT(stricmp(cBuf, szURI) == 0) ;
	CPPUNIT_ASSERT_EQUAL(strlen(cBuf)+1, nActual) ;

	// Clean up
	CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLines[i]), SIPX_RESULT_SUCCESS) ;
}


void sipXtapiTestSuite::testLineAPI_GetURI() 
{
	SIPX_LINE	hLine = NULL ;
	const char* szURI = "sip:removeme@pingtel.com" ;

	// Add and element
	CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, szURI, false, &hLine), SIPX_RESULT_SUCCESS) ;	
	CPPUNIT_ASSERT(hLine) ;


	// Standard get
	char cBuf[256] ;
	size_t nActual ;
	CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(hLine, cBuf, sizeof(cBuf), nActual), SIPX_RESULT_SUCCESS) ;
	CPPUNIT_ASSERT(stricmp(cBuf, szURI) == 0) ;
	CPPUNIT_ASSERT_EQUAL(strlen(cBuf)+1, nActual) ;

	// Ask for length
	CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(hLine, NULL, sizeof(cBuf), nActual), SIPX_RESULT_SUCCESS) ;
	CPPUNIT_ASSERT_EQUAL(nActual, strlen(szURI) + 1) ;

	// Small Buffer (doesn't stomp, etc)
	strcpy(cBuf, "1234567890") ;
	CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(hLine, cBuf, 5, nActual), SIPX_RESULT_SUCCESS) ;
	CPPUNIT_ASSERT(stricmp(cBuf, "sip:") == 0) ;
	CPPUNIT_ASSERT(cBuf[5] == '6') ;

	// Clean up
	CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS) ;
}


void sipXtapiTestSuite::testCallMakeAPI()
{
	SIPX_LINE hLine = NULL;
	SIPX_CALL hCall = NULL ;

	CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, "sip:bandreasen@pingtel.com", false, &hLine), SIPX_RESULT_SUCCESS) ;
	CPPUNIT_ASSERT(hLine) ;

	CPPUNIT_ASSERT_EQUAL(sipxCallCreate(g_hInst, hLine, &hCall), SIPX_RESULT_SUCCESS) ;
	CPPUNIT_ASSERT(hCall) ;

    CPPUNIT_ASSERT_EQUAL(sipxCallDestroy(hCall), SIPX_RESULT_SUCCESS) ;	
	CPPUNIT_ASSERT(hCall == NULL) ;

	CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS) ;
}


void sipXtapiTestSuite::testCallGetID() 
{
	SIPX_LINE hLine = NULL ;
	SIPX_CALL hCall = NULL ;

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


void sipXtapiTestSuite::testCallGetLocalID()
{
	SIPX_LINE hLine = NULL ;
	SIPX_CALL hCall = NULL ;

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


void sipXtapiTestSuite::createCall(SIPX_LINE* phLine, SIPX_CALL* phCall)
{
	CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, "sip:bandreasen@pingtel.com", false, phLine), SIPX_RESULT_SUCCESS) ;
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

	CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS) ;
}

void sipXtapiTestSuite::destroyCall(SIPX_CALL& hCall) 
{
    CPPUNIT_ASSERT_EQUAL(sipxCallDestroy(hCall), SIPX_RESULT_SUCCESS) ;	
	CPPUNIT_ASSERT(hCall == NULL) ;
}



void basicCall_CallBack_Place(SIPX_CALL hCall, 
                              SIPX_LINE hLine, 
					          SIPX_CALLSTATE_MAJOR eMajor, 
					          SIPX_CALLSTATE_MINOR eMinor, 
                              void* pUserData)
{
    g_recorder.addEvent(eMajor, eMinor) ;
}


void basicCall_CallBack_Receive(SIPX_CALL hCall, 
                                SIPX_LINE hLine, 
					            SIPX_CALLSTATE_MAJOR eMajor, 
					            SIPX_CALLSTATE_MINOR eMinor, 
                                void* pUserData)
{ 
    g_recorder2.addEvent(eMajor, eMinor) ;

    // If we have user data verify the line url against it
    if (pUserData)
    {
        char szBuffer[500] ; 
        size_t nBuffer ;

        if (strlen((const char*) pUserData))
        {
            CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(hLine, szBuffer, sizeof(szBuffer), nBuffer), SIPX_RESULT_SUCCESS) ;

            // printf("comparing %s to %s\n", pUserData, szBuffer) ;
            CPPUNIT_ASSERT(strcmp((char*) pUserData, szBuffer) == 0) ;
        }
        else
        {
            CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(hLine, szBuffer, sizeof(szBuffer), nBuffer), SIPX_RESULT_FAILURE) ;
        }
    }

    switch(eMajor)
    {
        case OFFERING:
            sipxCallAccept(hCall) ;
            break ;
        case ALERTING:
            sipxCallAnswer(hCall) ;
            break ;
        case DISCONNECTED:
            sipxCallDestroy(hCall) ; 
            break ;
    }
}


void basicCall_CallBack_Receive3(SIPX_CALL hCall, 
                                SIPX_LINE hLine, 
					            SIPX_CALLSTATE_MAJOR eMajor, 
					            SIPX_CALLSTATE_MINOR eMinor, 
                                void* pUserData)
{ 
    g_recorder3.addEvent(eMajor, eMinor) ;

    // If we have user data verify the line url against it
    if (pUserData)
    {
        char szBuffer[500] ; 
        size_t nBuffer ;

        if (strlen((const char*) pUserData))
        {
            CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(hLine, szBuffer, sizeof(szBuffer), nBuffer), SIPX_RESULT_SUCCESS) ;

            // printf("comparing %s to %s\n", pUserData, szBuffer) ;
            CPPUNIT_ASSERT(strcmp((char*) pUserData, szBuffer) == 0) ;
        }
        else
        {
            CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(hLine, szBuffer, sizeof(szBuffer), nBuffer), SIPX_RESULT_FAILURE) ;
        }
    }

    switch(eMajor)
    {
        case OFFERING:
            sipxCallAccept(hCall) ;
            break ;
        case ALERTING:
            sipxCallAnswer(hCall) ;
            break ;
        case DISCONNECTED:
            sipxCallDestroy(hCall) ; 
            break ;
    }
}



void basicCall_CallBack_Receive3_hangup(SIPX_CALL hCall, 
                                        SIPX_LINE hLine, 
					                    SIPX_CALLSTATE_MAJOR eMajor, 
					                    SIPX_CALLSTATE_MINOR eMinor, 
                                        void* pUserData)
{ 
    g_recorder3.addEvent(eMajor, eMinor) ;

    // If we have user data verify the line url against it
    if (pUserData)
    {
        char szBuffer[500] ; 
        size_t nBuffer ;

        if (strlen((const char*) pUserData))
        {
            CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(hLine, szBuffer, sizeof(szBuffer), nBuffer), SIPX_RESULT_SUCCESS) ;

            // printf("comparing %s to %s\n", pUserData, szBuffer) ;
            CPPUNIT_ASSERT(strcmp((char*) pUserData, szBuffer) == 0) ;
        }
        else
        {
            CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(hLine, szBuffer, sizeof(szBuffer), nBuffer), SIPX_RESULT_FAILURE) ;
        }
    }

    switch(eMajor)
    {
        case OFFERING:
            sipxCallAccept(hCall) ;
            break ;
        case ALERTING:
            sipxCallAnswer(hCall) ;
            break ;
        case CONNECTED:
            sipxCallDestroy(hCall) ;
            break ;
        case DISCONNECTED:            
            break ;
    }
}


void basicCall_CallBack_Receive3_busy(SIPX_CALL hCall, 
                                        SIPX_LINE hLine, 
					                    SIPX_CALLSTATE_MAJOR eMajor, 
					                    SIPX_CALLSTATE_MINOR eMinor, 
                                        void* pUserData)
{ 
    g_recorder3.addEvent(eMajor, eMinor) ;

    // If we have user data verify the line url against it
    if (pUserData)
    {
        char szBuffer[500] ; 
        size_t nBuffer ;

        if (strlen((const char*) pUserData))
        {
            CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(hLine, szBuffer, sizeof(szBuffer), nBuffer), SIPX_RESULT_SUCCESS) ;

            // printf("comparing %s to %s\n", pUserData, szBuffer) ;
            CPPUNIT_ASSERT(strcmp((char*) pUserData, szBuffer) == 0) ;
        }
        else
        {
            CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(hLine, szBuffer, sizeof(szBuffer), nBuffer), SIPX_RESULT_FAILURE) ;
        }
    }

    switch(eMajor)
    {
        case OFFERING:
            sipxCallReject(hCall) ;
            break ;
    }
}




// A calls B, B answers, A hangs up
void sipXtapiTestSuite::testCallBasic() 
{    
    g_recorder.clear() ;
    g_recorder2.clear() ;    

    // Setup Auto-answer call back
    sipxListenerAdd(g_hInst2, basicCall_CallBack_Receive, NULL) ;

    SIPX_CALL hCall ;
    SIPX_LINE hLine ;

    sipxListenerAdd(g_hInst, basicCall_CallBack_Place, NULL) ;

    createCall(&hLine, &hCall) ;

    sipxCallConnect(hCall, "sip:foo@127.0.0.1:9000") ;
    Sleep(CALL_DELAY*2) ;
    destroyCall(hLine, hCall) ;
    Sleep(CALL_DELAY) ;

    sipxListenerRemove(g_hInst, basicCall_CallBack_Place, NULL) ;
    sipxListenerRemove(g_hInst2, basicCall_CallBack_Receive, NULL) ;

    g_recorder.addCompareEvent(DIALTONE, DIALTONE_UNKNOWN) ;
    g_recorder.addCompareEvent(REMOTE_OFFERING, REMOTE_OFFERING_NORMAL) ;
    g_recorder.addCompareEvent(REMOTE_ALERTING, REMOTE_ALERTING_NORMAL) ;
    g_recorder.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;    
    CPPUNIT_ASSERT(g_recorder.compare()) ;

    g_recorder2.addCompareEvent(NEWCALL, NEW_CALL_NORMAL) ;
    g_recorder2.addCompareEvent(OFFERING, OFFERING_ACTIVE) ;
    g_recorder2.addCompareEvent(ALERTING, ALERTING_NORMAL) ;
    g_recorder2.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder2.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder2.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;
    CPPUNIT_ASSERT(g_recorder2.compare()) ;
}


void basicCall_CallBack_Receive_Hangup(SIPX_CALL hCall, 
                                       SIPX_LINE hLine, 
					                   SIPX_CALLSTATE_MAJOR eMajor, 
					                   SIPX_CALLSTATE_MINOR eMinor, 
                                       void* pUserData)
{ 
    g_recorder2.addEvent(eMajor, eMinor) ;

    switch(eMajor)
    {
        case OFFERING:
            sipxCallAccept(hCall) ;
            break ;
        case ALERTING:
            sipxCallAnswer(hCall) ;
            Sleep(CALL_DELAY) ;
            sipxCallDestroy(hCall) ;
            break ;
        case DISCONNECTED:            
            break ;
    }
}


// A calls B, B answers, B hangs up
void sipXtapiTestSuite::testCallBasic2() 
{
    g_recorder.clear() ;
    g_recorder2.clear() ;

    // Setup Auto-answer call back
    sipxListenerAdd(g_hInst, basicCall_CallBack_Place, NULL) ;
    sipxListenerAdd(g_hInst2, basicCall_CallBack_Receive_Hangup, NULL) ;
    

    SIPX_CALL hCall ;
    SIPX_LINE hLine ;

    createCall(&hLine, &hCall) ;

    sipxCallConnect(hCall, "sip:foo@127.0.0.1:9000") ;
    Sleep(CALL_DELAY*2) ;    
    destroyCall(hLine, hCall) ;
    Sleep(CALL_DELAY) ;

    sipxListenerRemove(g_hInst, basicCall_CallBack_Place, NULL) ;
    sipxListenerRemove(g_hInst2, basicCall_CallBack_Receive_Hangup, NULL) ;

    g_recorder.addCompareEvent(DIALTONE, DIALTONE_UNKNOWN) ;
    g_recorder.addCompareEvent(REMOTE_OFFERING, REMOTE_OFFERING_NORMAL) ;
    g_recorder.addCompareEvent(REMOTE_ALERTING, REMOTE_ALERTING_NORMAL) ;
    g_recorder.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;
    CPPUNIT_ASSERT(g_recorder.compare()) ;

    g_recorder2.addCompareEvent(NEWCALL, NEW_CALL_NORMAL) ;
    g_recorder2.addCompareEvent(OFFERING, OFFERING_ACTIVE) ;
    g_recorder2.addCompareEvent(ALERTING, ALERTING_NORMAL) ;
    g_recorder2.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder2.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder2.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;
    CPPUNIT_ASSERT(g_recorder2.compare()) ;
}


void basicCall_CallBack_Receive_Reject(SIPX_CALL hCall, 
                                       SIPX_LINE hLine, 
					                   SIPX_CALLSTATE_MAJOR eMajor, 
					                   SIPX_CALLSTATE_MINOR eMinor, 
                                       void* pUserData)
{ 
    g_recorder2.addEvent(eMajor, eMinor) ;

    switch(eMajor)
    {
        case OFFERING:
            sipxCallReject(hCall) ;
            break ;
        case ALERTING:
            break ;
        case DISCONNECTED:            
            break ;
    }
}



// A calls B, B answers, B hangs up
void sipXtapiTestSuite::testCallBusy() 
{
    g_recorder.clear() ;
    g_recorder2.clear() ;

    // Setup Auto-answer call back
    sipxListenerAdd(g_hInst2, basicCall_CallBack_Receive_Reject, NULL) ;

    SIPX_CALL hCall ;
    SIPX_LINE hLine ;

    sipxListenerAdd(g_hInst, basicCall_CallBack_Place, NULL) ;

    createCall(&hLine, &hCall) ;

    sipxCallConnect(hCall, "sip:foo@127.0.0.1:9000") ;
    Sleep(CALL_DELAY*2) ;
    destroyCall(hLine, hCall) ;
    Sleep(CALL_DELAY) ;

    sipxListenerRemove(g_hInst, basicCall_CallBack_Place, NULL) ;
    sipxListenerRemove(g_hInst2, basicCall_CallBack_Receive_Reject, NULL) ;

    g_recorder.addCompareEvent(DIALTONE, DIALTONE_UNKNOWN) ;
    g_recorder.addCompareEvent(REMOTE_OFFERING, REMOTE_OFFERING_NORMAL) ;
    g_recorder.addCompareEvent(DISCONNECTED, DISCONNECTED_BUSY) ;
    g_recorder.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;
    CPPUNIT_ASSERT(g_recorder.compare()) ;

    g_recorder2.addCompareEvent(NEWCALL, NEW_CALL_NORMAL) ;
    g_recorder2.addCompareEvent(OFFERING, OFFERING_ACTIVE) ;
    g_recorder2.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder2.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;
    CPPUNIT_ASSERT(g_recorder2.compare()) ;
}


// A calls B, B answers, B hangs up
void sipXtapiTestSuite::testCallHold() 
{
    g_recorder.clear() ;
    g_recorder2.clear() ;

    // Setup Auto-answer call back
    sipxListenerAdd(g_hInst2, basicCall_CallBack_Receive, NULL) ;

    SIPX_CALL hCall ;
    SIPX_LINE hLine ;

    sipxListenerAdd(g_hInst, basicCall_CallBack_Place, NULL) ;

    createCall(&hLine, &hCall) ;

    sipxCallConnect(hCall, "sip:foo@127.0.0.1:9000") ;
    Sleep(CALL_DELAY*2) ;
    sipxCallHold(hCall) ;
    Sleep(CALL_DELAY*2) ;
    sipxCallUnhold(hCall) ;
    Sleep(CALL_DELAY*2) ;
    destroyCall(hLine, hCall) ;
    Sleep(CALL_DELAY) ;

    sipxListenerRemove(g_hInst, basicCall_CallBack_Place, NULL) ;
    sipxListenerRemove(g_hInst2, basicCall_CallBack_Receive, NULL) ;

    g_recorder.addCompareEvent(DIALTONE, DIALTONE_UNKNOWN) ;
    g_recorder.addCompareEvent(REMOTE_OFFERING, REMOTE_OFFERING_NORMAL) ;
    g_recorder.addCompareEvent(REMOTE_ALERTING, REMOTE_ALERTING_NORMAL) ;
    g_recorder.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder.addCompareEvent(CONNECTED, CONNECTED_INACTIVE) ;
    g_recorder.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;
    CPPUNIT_ASSERT(g_recorder.compare()) ;

    g_recorder2.addCompareEvent(NEWCALL, NEW_CALL_NORMAL) ;
    g_recorder2.addCompareEvent(OFFERING, OFFERING_ACTIVE) ;
    g_recorder2.addCompareEvent(ALERTING, ALERTING_NORMAL) ;
    g_recorder2.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder2.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder2.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;
    CPPUNIT_ASSERT(g_recorder2.compare()) ;
}


void linetest_verify_lineurl(SIPX_CALL hCall, 
                             SIPX_LINE hLine, 
					         SIPX_CALLSTATE_MAJOR eMajor, 
					         SIPX_CALLSTATE_MINOR eMinor, 
                             void* pUserData)
{
    char szBuffer[500] ; 
    size_t nBuffer ;
    CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(hLine, szBuffer, sizeof(szBuffer), nBuffer), SIPX_RESULT_SUCCESS) ;

    // printf("comparing %s to %s\n", pUserData, szBuffer) ;
    CPPUNIT_ASSERT(strcmp((char*) pUserData, szBuffer) == 0) ;
}


/**
 * Source Call:
 *   Line1: george@thejungle.com
 *   Line2: jane@thejungle.com
 *
 * Target Call:
 *   Line1: bob@127.0.0.1:9000
 *   Line2: jill@127.0.0.1:9000
 *
 * This test verifies that the line handles return with the sipX events are sane
 */
#define URL_GEORGE_AT_JUNGLE        "sip:george@thejungle.com"
#define URL_JANE_AT_JUNGLE          "sip:jane@thejungle.com"
#define URL_BOB_AT_LOCALHOST        "sip:bob@127.0.0.1:9000"
#define URL_JILL_AT_LOCALHOST       "sip:jill@127.0.0.1:9000"
#define URL_BOGUS_AT_LOCALHOST      "sip:bogus@127.0.0.1:9000"
#define URL_EMPTY_STRING            ""
void sipXtapiTestSuite::testLines() 
{
    SIPX_CALL hCall ;
    SIPX_LINE hLineGeorge ;
    SIPX_LINE hLineJane ;
    SIPX_LINE hLineBob ;
    SIPX_LINE hLineJill ;

    CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, URL_GEORGE_AT_JUNGLE, false, &hLineGeorge), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, URL_JANE_AT_JUNGLE, false, &hLineJane), SIPX_RESULT_SUCCESS) ;

    CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst2, URL_BOB_AT_LOCALHOST, false, &hLineBob), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst2, URL_JILL_AT_LOCALHOST, false, &hLineJill), SIPX_RESULT_SUCCESS) ;

    /**
     * Make call as George to Jill
     */
    // Setup Auto-answer call back
    sipxListenerAdd(g_hInst, linetest_verify_lineurl, URL_GEORGE_AT_JUNGLE) ;
    sipxListenerAdd(g_hInst2, basicCall_CallBack_Receive, URL_JILL_AT_LOCALHOST) ;

    createCall(hLineGeorge, &hCall) ;

    sipxCallConnect(hCall, URL_JILL_AT_LOCALHOST) ;
    Sleep(CALL_DELAY*2) ;
    destroyCall(hCall) ;
    Sleep(CALL_DELAY) ;

    sipxListenerRemove(g_hInst, linetest_verify_lineurl, URL_GEORGE_AT_JUNGLE) ;
    sipxListenerRemove(g_hInst2, basicCall_CallBack_Receive, URL_JILL_AT_LOCALHOST) ;

    /**
     * Make call as Jane to Bob
     */ 
    sipxListenerAdd(g_hInst, linetest_verify_lineurl, URL_JANE_AT_JUNGLE) ;
    sipxListenerAdd(g_hInst2, basicCall_CallBack_Receive, URL_BOB_AT_LOCALHOST) ;
    createCall(hLineJane, &hCall) ;

    sipxCallConnect(hCall, URL_BOB_AT_LOCALHOST) ;
    Sleep(CALL_DELAY*2) ;
    destroyCall(hCall) ;
    Sleep(CALL_DELAY) ;

    sipxListenerRemove(g_hInst, linetest_verify_lineurl, URL_JANE_AT_JUNGLE) ;
    sipxListenerRemove(g_hInst2, basicCall_CallBack_Receive, URL_BOB_AT_LOCALHOST) ;

    /**
     * Make call as Jane to BOGUS (line not defined)
     */
    sipxListenerAdd(g_hInst, linetest_verify_lineurl, URL_JANE_AT_JUNGLE) ;
    sipxListenerAdd(g_hInst2, basicCall_CallBack_Receive, URL_EMPTY_STRING) ;
    createCall(hLineJane, &hCall) ;

    sipxCallConnect(hCall, URL_BOGUS_AT_LOCALHOST) ;
    Sleep(CALL_DELAY*2) ;
    destroyCall(hCall) ;
    Sleep(CALL_DELAY) ;

    sipxListenerRemove(g_hInst, linetest_verify_lineurl, URL_JANE_AT_JUNGLE) ;
    sipxListenerRemove(g_hInst2, basicCall_CallBack_Receive, URL_EMPTY_STRING) ;


    CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLineGeorge), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLineJane), SIPX_RESULT_SUCCESS) ; 
    CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLineBob), SIPX_RESULT_SUCCESS) ; 
    CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLineJill), SIPX_RESULT_SUCCESS) ; 
}


// Setup conference and drop conference
void sipXtapiTestSuite::testConfBasic1()
{
    SIPX_LINE   hLine ;
    SIPX_CONF   hConf ;
    SIPX_CALL   hCall ;
    SIPX_CALL   hCall2 ;

    g_recorder.clear() ;
    g_recorder2.clear() ;    
    g_recorder3.clear() ;    
    
    CPPUNIT_ASSERT_EQUAL(sipxListenerAdd(g_hInst, basicCall_CallBack_Place, NULL), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxListenerAdd(g_hInst2, basicCall_CallBack_Receive, NULL), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxListenerAdd(g_hInst3, basicCall_CallBack_Receive3, NULL), SIPX_RESULT_SUCCESS) ;

    CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, "sip:foobar@127.0.0.1:8000", false, &hLine), SIPX_RESULT_SUCCESS) ;

    CPPUNIT_ASSERT_EQUAL(sipxConferenceCreate(g_hInst, &hConf), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxConferenceAdd(hConf, hLine, "sip:blah@127.0.0.1:9000", &hCall), SIPX_RESULT_SUCCESS) ;
    Sleep(CALL_DELAY) ;
    CPPUNIT_ASSERT_EQUAL(sipxConferenceAdd(hConf, hLine, "sip:blah2@127.0.0.1:10000", &hCall2), SIPX_RESULT_SUCCESS) ;
    Sleep(CALL_DELAY) ;    

    SIPX_CALL hCalls[10] ;
    size_t nCalls ;
    CPPUNIT_ASSERT_EQUAL(sipxConferenceGetCalls(hConf, hCalls, 10, nCalls), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT(nCalls == 2) ;
    CPPUNIT_ASSERT((hCalls[0] == hCall) || (hCalls[0] == hCall2)) ;
    CPPUNIT_ASSERT((hCalls[1] == hCall) || (hCalls[1] == hCall2)) ;
    CPPUNIT_ASSERT(hCalls[0] != hCalls[1]) ;

    Sleep(CALL_DELAY*2) ;
    CPPUNIT_ASSERT_EQUAL(sipxConferenceDestroy(hConf), SIPX_RESULT_SUCCESS) ;
    Sleep(CALL_DELAY*2) ;    

    CPPUNIT_ASSERT_EQUAL(sipxListenerRemove(g_hInst, basicCall_CallBack_Place, NULL), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxListenerRemove(g_hInst2, basicCall_CallBack_Receive, NULL), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxListenerRemove(g_hInst3, basicCall_CallBack_Receive3, NULL), SIPX_RESULT_SUCCESS) ;  
    
    CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS) ;  

    g_recorder.addCompareEvent(DIALTONE, DIALTONE_CONFERENCE) ;
    g_recorder.addCompareEvent(REMOTE_OFFERING, REMOTE_OFFERING_NORMAL) ;
    g_recorder.addCompareEvent(REMOTE_ALERTING, REMOTE_ALERTING_NORMAL) ;
    g_recorder.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder.addCompareEvent(DIALTONE, DIALTONE_CONFERENCE) ;
    g_recorder.addCompareEvent(REMOTE_OFFERING, REMOTE_OFFERING_NORMAL) ;
    g_recorder.addCompareEvent(REMOTE_ALERTING, REMOTE_ALERTING_NORMAL) ;
    g_recorder.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;    
    g_recorder.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;    
    CPPUNIT_ASSERT(g_recorder.compare()) ;

    g_recorder2.addCompareEvent(NEWCALL, NEW_CALL_NORMAL) ;
    g_recorder2.addCompareEvent(OFFERING, OFFERING_ACTIVE) ;
    g_recorder2.addCompareEvent(ALERTING, ALERTING_NORMAL) ;
    g_recorder2.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder2.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder2.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;
    CPPUNIT_ASSERT(g_recorder2.compare()) ;

    g_recorder3.addCompareEvent(NEWCALL, NEW_CALL_NORMAL) ;
    g_recorder3.addCompareEvent(OFFERING, OFFERING_ACTIVE) ;
    g_recorder3.addCompareEvent(ALERTING, ALERTING_NORMAL) ;
    g_recorder3.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder3.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder3.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;
    CPPUNIT_ASSERT(g_recorder3.compare()) ;
}

// Setup conference, drop party 1, drop party 2
void sipXtapiTestSuite::testConfBasic2()
{
    SIPX_LINE   hLine ;
    SIPX_CONF   hConf ;
    SIPX_CALL   hCall ;
    SIPX_CALL   hCall2 ;

    g_recorder.clear() ;
    g_recorder2.clear() ;    
    g_recorder3.clear() ;    
    
    CPPUNIT_ASSERT_EQUAL(sipxListenerAdd(g_hInst, basicCall_CallBack_Place, NULL), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxListenerAdd(g_hInst2, basicCall_CallBack_Receive, NULL), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxListenerAdd(g_hInst3, basicCall_CallBack_Receive3, NULL), SIPX_RESULT_SUCCESS) ;

    CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, "sip:foobar@127.0.0.1:8000", false, &hLine), SIPX_RESULT_SUCCESS) ;

    CPPUNIT_ASSERT_EQUAL(sipxConferenceCreate(g_hInst, &hConf), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxConferenceAdd(hConf, hLine, "sip:blah@127.0.0.1:9000", &hCall), SIPX_RESULT_SUCCESS) ;
    Sleep(CALL_DELAY) ;
    CPPUNIT_ASSERT_EQUAL(sipxConferenceAdd(hConf, hLine, "sip:blah2@127.0.0.1:10000", &hCall2), SIPX_RESULT_SUCCESS) ;
    Sleep(CALL_DELAY) ;        

    SIPX_CALL hCalls[10] ;
    size_t nCalls ;
    CPPUNIT_ASSERT_EQUAL(sipxConferenceGetCalls(hConf, hCalls, 10, nCalls), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT(nCalls == 2) ;
    CPPUNIT_ASSERT((hCalls[0] == hCall) || (hCalls[0] == hCall2)) ;
    CPPUNIT_ASSERT((hCalls[1] == hCall) || (hCalls[1] == hCall2)) ;
    CPPUNIT_ASSERT(hCalls[0] != hCalls[1]) ;

    CPPUNIT_ASSERT_EQUAL(sipxConferenceRemove(hConf, hCall), SIPX_RESULT_SUCCESS) ;
    Sleep(CALL_DELAY*2) ;
    CPPUNIT_ASSERT_EQUAL(sipxConferenceRemove(hConf, hCall2), SIPX_RESULT_SUCCESS) ;
    Sleep(CALL_DELAY*2) ;

    CPPUNIT_ASSERT_EQUAL(sipxConferenceGetCalls(hConf, hCalls, 10, nCalls), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT(nCalls == 0) ;

    CPPUNIT_ASSERT_EQUAL(sipxListenerRemove(g_hInst, basicCall_CallBack_Place, NULL), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxListenerRemove(g_hInst2, basicCall_CallBack_Receive, NULL), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxListenerRemove(g_hInst3, basicCall_CallBack_Receive3, NULL), SIPX_RESULT_SUCCESS) ;    

    CPPUNIT_ASSERT_EQUAL(sipxConferenceDestroy(hConf), SIPX_RESULT_SUCCESS) ;
    
    sipxLineRemove(hLine) ;    

    g_recorder.addCompareEvent(DIALTONE, DIALTONE_CONFERENCE) ;
    g_recorder.addCompareEvent(REMOTE_OFFERING, REMOTE_OFFERING_NORMAL) ;
    g_recorder.addCompareEvent(REMOTE_ALERTING, REMOTE_ALERTING_NORMAL) ;
    g_recorder.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder.addCompareEvent(DIALTONE, DIALTONE_CONFERENCE) ;
    g_recorder.addCompareEvent(REMOTE_OFFERING, REMOTE_OFFERING_NORMAL) ;
    g_recorder.addCompareEvent(REMOTE_ALERTING, REMOTE_ALERTING_NORMAL) ;
    g_recorder.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;    
    g_recorder.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;    
    CPPUNIT_ASSERT(g_recorder.compare()) ;

    g_recorder2.addCompareEvent(NEWCALL, NEW_CALL_NORMAL) ;
    g_recorder2.addCompareEvent(OFFERING, OFFERING_ACTIVE) ;
    g_recorder2.addCompareEvent(ALERTING, ALERTING_NORMAL) ;
    g_recorder2.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder2.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder2.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;
    CPPUNIT_ASSERT(g_recorder2.compare()) ;

    g_recorder3.addCompareEvent(NEWCALL, NEW_CALL_NORMAL) ;
    g_recorder3.addCompareEvent(OFFERING, OFFERING_ACTIVE) ;
    g_recorder3.addCompareEvent(ALERTING, ALERTING_NORMAL) ;
    g_recorder3.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder3.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder3.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;
    CPPUNIT_ASSERT(g_recorder3.compare()) ;
}


// Setup conference, drop party 2, drop party 1
void sipXtapiTestSuite::testConfBasic3()
{
     SIPX_LINE   hLine ;
    SIPX_CONF   hConf ;
    SIPX_CALL   hCall ;
    SIPX_CALL   hCall2 ;

    g_recorder.clear() ;
    g_recorder2.clear() ;    
    g_recorder3.clear() ;    
    
    CPPUNIT_ASSERT_EQUAL(sipxListenerAdd(g_hInst, basicCall_CallBack_Place, NULL), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxListenerAdd(g_hInst2, basicCall_CallBack_Receive, NULL), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxListenerAdd(g_hInst3, basicCall_CallBack_Receive3, NULL), SIPX_RESULT_SUCCESS) ;

    CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, "sip:foobar@127.0.0.1:8000", false, &hLine), SIPX_RESULT_SUCCESS) ;

    CPPUNIT_ASSERT_EQUAL(sipxConferenceCreate(g_hInst, &hConf), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxConferenceAdd(hConf, hLine, "sip:blah@127.0.0.1:9000", &hCall), SIPX_RESULT_SUCCESS) ;
    Sleep(CALL_DELAY) ;
    CPPUNIT_ASSERT_EQUAL(sipxConferenceAdd(hConf, hLine, "sip:blah2@127.0.0.1:10000", &hCall2), SIPX_RESULT_SUCCESS) ;

    Sleep(CALL_DELAY*2) ;

    SIPX_CALL hCalls[10] ;
    size_t nCalls ;
    CPPUNIT_ASSERT_EQUAL(sipxConferenceGetCalls(hConf, hCalls, 10, nCalls), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT(nCalls == 2) ;
    CPPUNIT_ASSERT((hCalls[0] == hCall) || (hCalls[0] == hCall2)) ;
    CPPUNIT_ASSERT((hCalls[1] == hCall) || (hCalls[1] == hCall2)) ;
    CPPUNIT_ASSERT(hCalls[0] != hCalls[1]) ;

    CPPUNIT_ASSERT_EQUAL(sipxConferenceRemove(hConf, hCall2), SIPX_RESULT_SUCCESS) ;
    Sleep(CALL_DELAY*2) ;
    CPPUNIT_ASSERT_EQUAL(sipxConferenceRemove(hConf, hCall), SIPX_RESULT_SUCCESS) ;
    Sleep(CALL_DELAY*2) ;

    CPPUNIT_ASSERT_EQUAL(sipxConferenceGetCalls(hConf, hCalls, 10, nCalls), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT(nCalls == 0) ;

    CPPUNIT_ASSERT_EQUAL(sipxListenerRemove(g_hInst, basicCall_CallBack_Place, NULL), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxListenerRemove(g_hInst2, basicCall_CallBack_Receive, NULL), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxListenerRemove(g_hInst3, basicCall_CallBack_Receive3, NULL), SIPX_RESULT_SUCCESS) ;    

    CPPUNIT_ASSERT_EQUAL(sipxConferenceDestroy(hConf), SIPX_RESULT_SUCCESS) ;

    sipxLineRemove(hLine) ;    

    g_recorder.addCompareEvent(DIALTONE, DIALTONE_CONFERENCE) ;
    g_recorder.addCompareEvent(REMOTE_OFFERING, REMOTE_OFFERING_NORMAL) ;
    g_recorder.addCompareEvent(REMOTE_ALERTING, REMOTE_ALERTING_NORMAL) ;
    g_recorder.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder.addCompareEvent(DIALTONE, DIALTONE_CONFERENCE) ;
    g_recorder.addCompareEvent(REMOTE_OFFERING, REMOTE_OFFERING_NORMAL) ;
    g_recorder.addCompareEvent(REMOTE_ALERTING, REMOTE_ALERTING_NORMAL) ;
    g_recorder.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;    
    g_recorder.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;    
    CPPUNIT_ASSERT(g_recorder.compare()) ;

    g_recorder2.addCompareEvent(NEWCALL, NEW_CALL_NORMAL) ;
    g_recorder2.addCompareEvent(OFFERING, OFFERING_ACTIVE) ;
    g_recorder2.addCompareEvent(ALERTING, ALERTING_NORMAL) ;
    g_recorder2.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder2.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder2.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;
    CPPUNIT_ASSERT(g_recorder2.compare()) ;

    g_recorder3.addCompareEvent(NEWCALL, NEW_CALL_NORMAL) ;
    g_recorder3.addCompareEvent(OFFERING, OFFERING_ACTIVE) ;
    g_recorder3.addCompareEvent(ALERTING, ALERTING_NORMAL) ;
    g_recorder3.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder3.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder3.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;
    CPPUNIT_ASSERT(g_recorder3.compare()) ;
}

// Setup conference, drop party 2, redial party 2, drop conference
void sipXtapiTestSuite::testConfBasic4()
{
     SIPX_LINE   hLine ;
    SIPX_CONF   hConf ;
    SIPX_CALL   hCall ;
    SIPX_CALL   hCall2 ;

    g_recorder.clear() ;
    g_recorder2.clear() ;    
    g_recorder3.clear() ;    
    
    CPPUNIT_ASSERT_EQUAL(sipxListenerAdd(g_hInst, basicCall_CallBack_Place, NULL), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxListenerAdd(g_hInst2, basicCall_CallBack_Receive, NULL), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxListenerAdd(g_hInst3, basicCall_CallBack_Receive3, NULL), SIPX_RESULT_SUCCESS) ;

    CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, "sip:foobar@127.0.0.1:8000", false, &hLine), SIPX_RESULT_SUCCESS) ;

    CPPUNIT_ASSERT_EQUAL(sipxConferenceCreate(g_hInst, &hConf), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxConferenceAdd(hConf, hLine, "sip:blah@127.0.0.1:9000", &hCall), SIPX_RESULT_SUCCESS) ;
    Sleep(CALL_DELAY) ;
    CPPUNIT_ASSERT_EQUAL(sipxConferenceAdd(hConf, hLine, "sip:blah2@127.0.0.1:10000", &hCall2), SIPX_RESULT_SUCCESS) ;

    Sleep(CALL_DELAY*2) ;

    SIPX_CALL hCalls[10] ;
    size_t nCalls ;
    CPPUNIT_ASSERT_EQUAL(sipxConferenceGetCalls(hConf, hCalls, 10, nCalls), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT(nCalls == 2) ;
    CPPUNIT_ASSERT((hCalls[0] == hCall) || (hCalls[0] == hCall2)) ;
    CPPUNIT_ASSERT((hCalls[1] == hCall) || (hCalls[1] == hCall2)) ;
    CPPUNIT_ASSERT(hCalls[0] != hCalls[1]) ;


    CPPUNIT_ASSERT_EQUAL(sipxConferenceRemove(hConf, hCall2), SIPX_RESULT_SUCCESS) ;
    Sleep(CALL_DELAY*2) ;

    CPPUNIT_ASSERT_EQUAL(sipxConferenceGetCalls(hConf, hCalls, 10, nCalls), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT(nCalls == 1) ;
    CPPUNIT_ASSERT((hCalls[0] == hCall)) ;

    CPPUNIT_ASSERT_EQUAL(sipxConferenceAdd(hConf, hLine, "sip:blah2@127.0.0.1:10000", &hCall2), SIPX_RESULT_SUCCESS) ;
    Sleep(CALL_DELAY*2) ;    

    CPPUNIT_ASSERT_EQUAL(sipxConferenceGetCalls(hConf, hCalls, 10, nCalls), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT(nCalls == 2) ;
    CPPUNIT_ASSERT((hCalls[0] == hCall) || (hCalls[0] == hCall2)) ;
    CPPUNIT_ASSERT((hCalls[1] == hCall) || (hCalls[1] == hCall2)) ;
    CPPUNIT_ASSERT(hCalls[0] != hCalls[1]) ;

    CPPUNIT_ASSERT_EQUAL(sipxConferenceDestroy(hConf), SIPX_RESULT_SUCCESS) ;
    Sleep(CALL_DELAY*2) ;    

    CPPUNIT_ASSERT_EQUAL(sipxListenerRemove(g_hInst, basicCall_CallBack_Place, NULL), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxListenerRemove(g_hInst2, basicCall_CallBack_Receive, NULL), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxListenerRemove(g_hInst3, basicCall_CallBack_Receive3, NULL), SIPX_RESULT_SUCCESS) ;    

    sipxLineRemove(hLine) ;    

    g_recorder.addCompareEvent(DIALTONE, DIALTONE_CONFERENCE) ;
    g_recorder.addCompareEvent(REMOTE_OFFERING, REMOTE_OFFERING_NORMAL) ;
    g_recorder.addCompareEvent(REMOTE_ALERTING, REMOTE_ALERTING_NORMAL) ;
    g_recorder.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder.addCompareEvent(DIALTONE, DIALTONE_CONFERENCE) ;
    g_recorder.addCompareEvent(REMOTE_OFFERING, REMOTE_OFFERING_NORMAL) ;
    g_recorder.addCompareEvent(REMOTE_ALERTING, REMOTE_ALERTING_NORMAL) ;
    g_recorder.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;    
    g_recorder.addCompareEvent(DIALTONE, DIALTONE_CONFERENCE) ;
    g_recorder.addCompareEvent(REMOTE_OFFERING, REMOTE_OFFERING_NORMAL) ;
    g_recorder.addCompareEvent(REMOTE_ALERTING, REMOTE_ALERTING_NORMAL) ;
    g_recorder.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;    
    g_recorder.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;    
    CPPUNIT_ASSERT(g_recorder.compare()) ;

    g_recorder2.addCompareEvent(NEWCALL, NEW_CALL_NORMAL) ;
    g_recorder2.addCompareEvent(OFFERING, OFFERING_ACTIVE) ;
    g_recorder2.addCompareEvent(ALERTING, ALERTING_NORMAL) ;
    g_recorder2.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder2.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder2.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;
    CPPUNIT_ASSERT(g_recorder2.compare()) ;

    g_recorder3.addCompareEvent(NEWCALL, NEW_CALL_NORMAL) ;
    g_recorder3.addCompareEvent(OFFERING, OFFERING_ACTIVE) ;
    g_recorder3.addCompareEvent(ALERTING, ALERTING_NORMAL) ;
    g_recorder3.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder3.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder3.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;
    g_recorder3.addCompareEvent(NEWCALL, NEW_CALL_NORMAL) ;
    g_recorder3.addCompareEvent(OFFERING, OFFERING_ACTIVE) ;
    g_recorder3.addCompareEvent(ALERTING, ALERTING_NORMAL) ;
    g_recorder3.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder3.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder3.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;
    CPPUNIT_ASSERT(g_recorder3.compare()) ;
}


void sipXtapiTestSuite::testConfHold()
{
    SIPX_LINE   hLine ;
    SIPX_CONF   hConf ;
    SIPX_CALL   hCall ;
    SIPX_CALL   hCall2 ;

    g_recorder.clear() ;
    g_recorder2.clear() ;    
    g_recorder3.clear() ;    
    
    CPPUNIT_ASSERT_EQUAL(sipxListenerAdd(g_hInst, basicCall_CallBack_Place, NULL), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxListenerAdd(g_hInst2, basicCall_CallBack_Receive, NULL), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxListenerAdd(g_hInst3, basicCall_CallBack_Receive3, NULL), SIPX_RESULT_SUCCESS) ;

    CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, "sip:foobar@127.0.0.1:8000", false, &hLine), SIPX_RESULT_SUCCESS) ;

    CPPUNIT_ASSERT_EQUAL(sipxConferenceCreate(g_hInst, &hConf), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxConferenceAdd(hConf, hLine, "sip:blah@127.0.0.1:9000", &hCall), SIPX_RESULT_SUCCESS) ;
    Sleep(CALL_DELAY) ;
    CPPUNIT_ASSERT_EQUAL(sipxConferenceAdd(hConf, hLine, "sip:blah2@127.0.0.1:10000", &hCall2), SIPX_RESULT_SUCCESS) ;
    Sleep(CALL_DELAY) ;
    CPPUNIT_ASSERT_EQUAL(sipxCallHold(hCall), SIPX_RESULT_SUCCESS) ;
    Sleep(CALL_DELAY*2) ;
    CPPUNIT_ASSERT_EQUAL(sipxCallUnhold(hCall), SIPX_RESULT_SUCCESS) ;
    Sleep(CALL_DELAY*2) ;
    CPPUNIT_ASSERT_EQUAL(sipxConferenceDestroy(hConf), SIPX_RESULT_SUCCESS) ;
    Sleep(CALL_DELAY*2) ;    

    CPPUNIT_ASSERT_EQUAL(sipxListenerRemove(g_hInst, basicCall_CallBack_Place, NULL), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxListenerRemove(g_hInst2, basicCall_CallBack_Receive, NULL), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxListenerRemove(g_hInst3, basicCall_CallBack_Receive3, NULL), SIPX_RESULT_SUCCESS) ;  
    
    sipxLineRemove(hLine) ;    

    g_recorder.addCompareEvent(DIALTONE, DIALTONE_CONFERENCE) ;
    g_recorder.addCompareEvent(REMOTE_OFFERING, REMOTE_OFFERING_NORMAL) ;
    g_recorder.addCompareEvent(REMOTE_ALERTING, REMOTE_ALERTING_NORMAL) ;
    g_recorder.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder.addCompareEvent(DIALTONE, DIALTONE_CONFERENCE) ;
    g_recorder.addCompareEvent(REMOTE_OFFERING, REMOTE_OFFERING_NORMAL) ;
    g_recorder.addCompareEvent(REMOTE_ALERTING, REMOTE_ALERTING_NORMAL) ;
    g_recorder.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder.addCompareEvent(CONNECTED, CONNECTED_INACTIVE) ;
    g_recorder.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;    
    g_recorder.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;    
    CPPUNIT_ASSERT(g_recorder.compare()) ;

    g_recorder2.addCompareEvent(NEWCALL, NEW_CALL_NORMAL) ;
    g_recorder2.addCompareEvent(OFFERING, OFFERING_ACTIVE) ;
    g_recorder2.addCompareEvent(ALERTING, ALERTING_NORMAL) ;
    g_recorder2.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder2.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder2.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;
    CPPUNIT_ASSERT(g_recorder2.compare()) ;

    g_recorder3.addCompareEvent(NEWCALL, NEW_CALL_NORMAL) ;
    g_recorder3.addCompareEvent(OFFERING, OFFERING_ACTIVE) ;
    g_recorder3.addCompareEvent(ALERTING, ALERTING_NORMAL) ;
    g_recorder3.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder3.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder3.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;
    CPPUNIT_ASSERT(g_recorder3.compare()) ;
}


// Setup conference and drop conference
void sipXtapiTestSuite::testConfJoin()
{
    SIPX_LINE   hLine ;
    SIPX_CONF   hConf ;
    SIPX_CALL   hCall ;
    SIPX_CALL   hCall2 ;

    g_recorder.clear() ;
    g_recorder2.clear() ;    
    g_recorder3.clear() ;    
    
    CPPUNIT_ASSERT_EQUAL(sipxListenerAdd(g_hInst, basicCall_CallBack_Place, NULL), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxListenerAdd(g_hInst2, basicCall_CallBack_Receive, NULL), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxListenerAdd(g_hInst3, basicCall_CallBack_Receive3, NULL), SIPX_RESULT_SUCCESS) ;

    CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, "sip:foobar@127.0.0.1:8000", false, &hLine), SIPX_RESULT_SUCCESS) ;

    CPPUNIT_ASSERT_EQUAL(sipxCallCreate(g_hInst, hLine, &hCall), SIPX_RESULT_SUCCESS) ; 
    CPPUNIT_ASSERT_EQUAL(sipxCallConnect(hCall, "sip:blah@127.0.0.1:9000"), SIPX_RESULT_SUCCESS) ; 
    Sleep(CALL_DELAY*2) ;

    CPPUNIT_ASSERT_EQUAL(sipxConferenceCreate(g_hInst, &hConf), SIPX_RESULT_SUCCESS) ;

    CPPUNIT_ASSERT_EQUAL(sipxConferenceJoin(hConf, hCall), SIPX_RESULT_SUCCESS) ;
    Sleep(CALL_DELAY) ;
    CPPUNIT_ASSERT_EQUAL(sipxConferenceAdd(hConf, hLine, "sip:blah2@127.0.0.1:10000", &hCall2), SIPX_RESULT_SUCCESS) ;
    Sleep(CALL_DELAY) ;    

    SIPX_CALL hCalls[10] ;
    size_t nCalls ;
    CPPUNIT_ASSERT_EQUAL(sipxConferenceGetCalls(hConf, hCalls, 10, nCalls), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT(nCalls == 2) ;
    CPPUNIT_ASSERT((hCalls[0] == hCall) || (hCalls[0] == hCall2)) ;
    CPPUNIT_ASSERT((hCalls[1] == hCall) || (hCalls[1] == hCall2)) ;
    CPPUNIT_ASSERT(hCalls[0] != hCalls[1]) ;

    Sleep(CALL_DELAY*2) ;
    CPPUNIT_ASSERT_EQUAL(sipxConferenceDestroy(hConf), SIPX_RESULT_SUCCESS) ;
    Sleep(CALL_DELAY*2) ;    

    CPPUNIT_ASSERT_EQUAL(sipxListenerRemove(g_hInst, basicCall_CallBack_Place, NULL), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxListenerRemove(g_hInst2, basicCall_CallBack_Receive, NULL), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT_EQUAL(sipxListenerRemove(g_hInst3, basicCall_CallBack_Receive3, NULL), SIPX_RESULT_SUCCESS) ;  
    
    CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS) ;  

    g_recorder.addCompareEvent(DIALTONE, DIALTONE_UNKNOWN) ;
    g_recorder.addCompareEvent(REMOTE_OFFERING, REMOTE_OFFERING_NORMAL) ;
    g_recorder.addCompareEvent(REMOTE_ALERTING, REMOTE_ALERTING_NORMAL) ;
    g_recorder.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder.addCompareEvent(DIALTONE, DIALTONE_CONFERENCE) ;
    g_recorder.addCompareEvent(REMOTE_OFFERING, REMOTE_OFFERING_NORMAL) ;
    g_recorder.addCompareEvent(REMOTE_ALERTING, REMOTE_ALERTING_NORMAL) ;
    g_recorder.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;    
    g_recorder.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;    
    g_recorder.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;    
    CPPUNIT_ASSERT(g_recorder.compare()) ;

    g_recorder2.addCompareEvent(NEWCALL, NEW_CALL_NORMAL) ;
    g_recorder2.addCompareEvent(OFFERING, OFFERING_ACTIVE) ;
    g_recorder2.addCompareEvent(ALERTING, ALERTING_NORMAL) ;
    g_recorder2.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder2.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder2.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;
    CPPUNIT_ASSERT(g_recorder2.compare()) ;

    g_recorder3.addCompareEvent(NEWCALL, NEW_CALL_NORMAL) ;
    g_recorder3.addCompareEvent(OFFERING, OFFERING_ACTIVE) ;
    g_recorder3.addCompareEvent(ALERTING, ALERTING_NORMAL) ;
    g_recorder3.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder3.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder3.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;
    CPPUNIT_ASSERT(g_recorder3.compare()) ;
}


void sipXtapiTestSuite::testBlindTransferSuccess() 
{
    g_recorder.clear() ;
    g_recorder2.clear() ;
    g_recorder3.clear() ;

    // Setup Auto-answer call back
    sipxListenerAdd(g_hInst, basicCall_CallBack_Place, NULL) ;
    sipxListenerAdd(g_hInst2, basicCall_CallBack_Receive, NULL) ;
    sipxListenerAdd(g_hInst3, basicCall_CallBack_Receive3_hangup, NULL) ;
    
    SIPX_CALL hCall ;
    SIPX_LINE hLine ;

    createCall(&hLine, &hCall) ;

    sipxCallConnect(hCall, "sip:foo@127.0.0.1:9000") ;
    Sleep(CALL_DELAY*2) ;

    sipxCallBlindTransfer(hCall, "sip:foo@127.0.0.1:10000") ;
    
    Sleep(CALL_DELAY*10) ;
    
    destroyCall(hLine, hCall) ;
    Sleep(CALL_DELAY) ;

    sipxListenerRemove(g_hInst, basicCall_CallBack_Place, NULL) ;
    sipxListenerRemove(g_hInst2, basicCall_CallBack_Receive, NULL) ;
    sipxListenerRemove(g_hInst3, basicCall_CallBack_Receive3_hangup, NULL) ;
   
    g_recorder.addCompareEvent(DIALTONE, DIALTONE_UNKNOWN) ;
    g_recorder.addCompareEvent(REMOTE_OFFERING, REMOTE_OFFERING_NORMAL) ;
    g_recorder.addCompareEvent(REMOTE_ALERTING, REMOTE_ALERTING_NORMAL) ;
    g_recorder.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder.addCompareEvent(CONNECTED, CONNECTED_INACTIVE) ;
    g_recorder.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;
    CPPUNIT_ASSERT(g_recorder.compare()) ;

    g_recorder2.addCompareEvent(NEWCALL, NEW_CALL_NORMAL) ;
    g_recorder2.addCompareEvent(OFFERING, OFFERING_ACTIVE) ;
    g_recorder2.addCompareEvent(ALERTING, ALERTING_NORMAL) ;
    g_recorder2.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder2.addCompareEvent(CONNECTED, CONNECTED_INACTIVE) ;
    g_recorder2.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder2.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;
    CPPUNIT_ASSERT(g_recorder2.compare()) ;

    g_recorder3.addCompareEvent(NEWCALL, NEW_CALL_NORMAL) ;
    g_recorder3.addCompareEvent(OFFERING, OFFERING_ACTIVE) ;
    g_recorder3.addCompareEvent(ALERTING, ALERTING_NORMAL) ;
    g_recorder3.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder3.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder3.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;
    CPPUNIT_ASSERT(g_recorder3.compare()) ;
}


void sipXtapiTestSuite::testBlindTransferFailureBusy() 
{
    g_recorder.clear() ;
    g_recorder2.clear() ;
    g_recorder3.clear() ;

    // Setup Auto-answer call back
    sipxListenerAdd(g_hInst, basicCall_CallBack_Place, NULL) ;
    sipxListenerAdd(g_hInst2, basicCall_CallBack_Receive, NULL) ;
    sipxListenerAdd(g_hInst3, basicCall_CallBack_Receive3_busy, NULL) ;
    
    SIPX_CALL hCall ;
    SIPX_LINE hLine ;

    createCall(&hLine, &hCall) ;

    sipxCallConnect(hCall, "sip:foo@127.0.0.1:9000") ;
    Sleep(CALL_DELAY*2) ;

    sipxCallBlindTransfer(hCall, "sip:foo@127.0.0.1:10000") ;
    
    Sleep(CALL_DELAY*10) ;
    
    destroyCall(hLine, hCall) ;
    Sleep(CALL_DELAY) ;

    sipxListenerRemove(g_hInst, basicCall_CallBack_Place, NULL) ;
    sipxListenerRemove(g_hInst2, basicCall_CallBack_Receive, NULL) ;
    sipxListenerRemove(g_hInst3, basicCall_CallBack_Receive3_busy, NULL) ;
   
    g_recorder.addCompareEvent(DIALTONE, DIALTONE_UNKNOWN) ;
    g_recorder.addCompareEvent(REMOTE_OFFERING, REMOTE_OFFERING_NORMAL) ;
    g_recorder.addCompareEvent(REMOTE_ALERTING, REMOTE_ALERTING_NORMAL) ;
    g_recorder.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder.addCompareEvent(CONNECTED, CONNECTED_INACTIVE) ;
    g_recorder.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;
    CPPUNIT_ASSERT(g_recorder.compare()) ;

    g_recorder2.addCompareEvent(NEWCALL, NEW_CALL_NORMAL) ;
    g_recorder2.addCompareEvent(OFFERING, OFFERING_ACTIVE) ;
    g_recorder2.addCompareEvent(ALERTING, ALERTING_NORMAL) ;
    g_recorder2.addCompareEvent(CONNECTED, CONNECTED_ACTIVE) ;
    g_recorder2.addCompareEvent(CONNECTED, CONNECTED_INACTIVE) ;
    g_recorder2.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder2.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;
    CPPUNIT_ASSERT(g_recorder2.compare()) ;

    g_recorder3.addCompareEvent(NEWCALL, NEW_CALL_NORMAL) ;
    g_recorder3.addCompareEvent(OFFERING, OFFERING_ACTIVE) ;
    g_recorder3.addCompareEvent(DISCONNECTED, DISCONNECTED_NORMAL) ;
    g_recorder3.addCompareEvent(DESTROYED, DESTROYED_NORMAL) ;
    CPPUNIT_ASSERT(g_recorder3.compare()) ;
}

void lineCallback(SIPX_LINE hLine,
                            SIPX_LINE_EVENT_TYPE_MAJOR eMajor,
                            void* pUser)
{
    g_lineRecorder.addEvent(eMajor);
}


void sipXtapiTestSuite::testLineEvents() 
{    
    g_lineRecorder.clear() ;

    CPPUNIT_ASSERT_EQUAL(sipxLineListenerAdd(g_hInst, lineCallback, NULL), SIPX_RESULT_SUCCESS);

    // TODO - we need to figure out how to better test registrar interaction.  
    // It might require creating a test registrar that responds to REGISTER messages.
        
     
    SIPX_LINE hLine;
    CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, "sip:115@hopefullythisisnotarealdomain.com", true, &hLine), SIPX_RESULT_SUCCESS) ;
    g_lineRecorder.addCompareEvent(SIPX_LINE_EVENT_REGISTER_FAILED);
    
    Sleep(500);
    
    CPPUNIT_ASSERT(g_lineRecorder.compare()) ;
    
    CPPUNIT_ASSERT_EQUAL(sipxLineListenerRemove(g_hInst, lineCallback, NULL), SIPX_RESULT_SUCCESS);

}

