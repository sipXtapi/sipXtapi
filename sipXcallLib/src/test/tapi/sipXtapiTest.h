// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _SIPXTAPITEST_H
#define _SIPXTAPITEST_H

#include <cppunit/extensions/HelperMacros.h>

#include "utl/UtlSList.h"
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"

#define CALL_DELAY  500     /**< Delay where we need to wait for something in ms */

#define MAX_EVENTS  64 

class EventRecorder
{
protected:
    char* m_events[MAX_EVENTS] ;
    int   m_numEvents ;

    char* m_compareEvents[MAX_EVENTS] ;
    int   m_numCompareEvents ;


public:
    EventRecorder() ;
    ~EventRecorder() ;

    clear() ;

    void addEvent(SIPX_CALLSTATE_MAJOR eMajor, SIPX_CALLSTATE_MINOR eMinor) ;    
    void addCompareEvent(SIPX_CALLSTATE_MAJOR eMajor, SIPX_CALLSTATE_MINOR eMinor) ;   
    void addEvent(SIPX_LINE_EVENT_TYPE_MAJOR eMajor);    
    void addCompareEvent(SIPX_LINE_EVENT_TYPE_MAJOR eMajor);   
    bool compare() ;

protected:
    char* buildEventStr(char* array[], int nLength) ;
} ;

class sipXtapiTestSuite : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(sipXtapiTestSuite) ;

        CPPUNIT_TEST(testGainAPI) ;
	    CPPUNIT_TEST(testMuteAPI) ;
	    CPPUNIT_TEST(testVolumeAPI) ;
	    CPPUNIT_TEST(testLineAPI_Add) ;
	    CPPUNIT_TEST(testLineAPI_Remove) ;
	    CPPUNIT_TEST(testLineAPI_Credential) ;
	    CPPUNIT_TEST(testLineAPI_Get) ;
	    CPPUNIT_TEST(testLineAPI_GetURI) ;
    	CPPUNIT_TEST(testCallMakeAPI) ;    	

    	CPPUNIT_TEST(testCallGetID) ;
        CPPUNIT_TEST(testCallGetLocalID) ;
        
        CPPUNIT_TEST(testCallBasic) ;
        CPPUNIT_TEST(testCallBasic2) ;        
        CPPUNIT_TEST(testCallBusy) ;
        CPPUNIT_TEST(testCallHold) ;

        CPPUNIT_TEST(testConfHold) ;
        CPPUNIT_TEST(testConfBasic1) ;
        CPPUNIT_TEST(testConfBasic2) ;
        CPPUNIT_TEST(testConfBasic3) ;
        CPPUNIT_TEST(testConfBasic4) ;

        CPPUNIT_TEST(testConfJoin) ;
        
        CPPUNIT_TEST(testLines) ;
        CPPUNIT_TEST(testLineEvents);

        CPPUNIT_TEST(testBlindTransferSuccess) ;
        CPPUNIT_TEST(testBlindTransferFailureBusy) ;

	CPPUNIT_TEST_SUITE_END() ;

public:
    sipXtapiTestSuite();

	void setUp() ;
	void tearDown() ;

	void testGainAPI() ;
	void testMuteAPI() ;
	void testVolumeAPI() ;
	void testLineAPI_Add() ;
	void testLineAPI_Remove() ;
	void testLineAPI_Credential() ;
	void testLineAPI_Get() ;
	void testLineAPI_GetURI() ;

	void testCallMakeAPI() ;	
	void testCallGetID() ;
	void testCallGetLocalID() ;

    void testCallBasic() ;
    void testCallBasic2() ;
    void testCallBusy() ;
    void testCallHold() ;    

    void testConfHold() ;
    void testConfBasic1() ;
    void testConfBasic2() ;
    void testConfBasic3() ;
    void testConfBasic4() ;
    void testConfJoin() ;
    
    void testLines() ;
    void testLineEvents();

    void testBlindTransferSuccess() ;
    void testBlindTransferFailureBusy() ;

protected:
    void createCall(SIPX_LINE hLine, SIPX_CALL* phCall) ;
    void destroyCall(SIPX_CALL& hCall) ;

	void createCall(SIPX_LINE* phLine, SIPX_CALL* phCall) ;    
	void destroyCall(SIPX_LINE& hLine, SIPX_CALL& hCall) ;
    
} ;

#endif
