// $Id: //depot/OPENDEV/sipXphone/include/pingerjni/CallListenerHelpers.h#2 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _CALL_LISTENER_HELPERS_H
#define _CALL_LISTENER_HELPERS_H

// SYSTEM INCLUDES
#include <jni.h>
// APPLICATION INCLUDES

// DEFINES
#define CACHECID_TERM_CONN_EVENT                0
#define CACHECID_CONN_EVENT                     1
#define CACHECID_CALL_EVENT                     2
#define CACHECID_META_EVENT                     3
#define CACHECID_SINGLE_CALL_META_EVENT         4
#define CACHECID_MULTI_CALL_META_EVENT          5
#define CACHECID_MAX_INDEX                      6

#define CACHEMID_TERM_CONN_EVENT_INIT           0
#define CACHEMID_CONN_EVENT_INIT                1
#define CACHEMID_CALL_EVENT_INIT                2
#define CACHEMID_SINGLE_CALL_META_EVENT_INIT    3
#define CACHEMID_MULTI_CALL_META_EVENT_INIT     4
#define CACHEMID_MAX_INDEX                      5


#define CACHEFID_TERM_CONN_EVENT_CAUSE          0
#define CACHEFID_TERM_CONN_EVENT_SIP_CODE       1
#define CACHEFID_TERM_CONN_EVENT_SIP_TEXT       2
#define CACHEFID_TERM_CONN_EVENT_META           3
#define CACHEFID_CONN_EVENT_CAUSE               4
#define CACHEFID_CONN_EVENT_SIP_CODE            5
#define CACHEFID_CONN_EVENT_SIP_TEXT            6
#define CACHEFID_CONN_EVENT_META                7
#define CACHEFID_CALL_EVENT_CAUSE               8
#define CACHEFID_CALL_EVENT_SIP_CODE            9
#define CACHEFID_CALL_EVENT_SIP_TEXT            10
#define CACHEFID_CALL_EVENT_META                11
#define CACHEFID_META_EVENT_ID                  12
#define CACHEFID_MAX_INDEX                      13


#define EMASK_TERM_CONN_CREATED                 0x00000001
#define EMASK_TERM_CONN_IDLE                    0x00000002
#define EMASK_TERM_CONN_RINGING                 0x00000004
#define EMASK_TERM_CONN_DROPPED                 0x00000008
#define EMASK_TERM_CONN_UNKNOWN                 0x00000010
#define EMASK_TERM_CONN_HELD                    0x00000020
#define EMASK_TERM_CONN_TALKING                 0x00000040
#define EMASK_TERM_CONN_IN_USE                  0x00000080
#define EMASK_CONN_CREATED                      0x00000100
#define EMASK_CONN_ALERTING                     0x00000200
#define EMASK_CONN_DISCONNECTED                 0x00000400
#define EMASK_CONN_FAILED                       0x00000800
#define EMASK_CONN_UNKNOWN                      0x00001000
#define EMASK_CONN_DIALING                      0x00002000
#define EMASK_CONN_ESTABLISHED                  0x00004000
#define EMASK_CONN_INITIATED                    0x00008000
#define EMASK_CONN_NETWORK_ALERTING             0x00010000
#define EMASK_CONN_NETWORK_REACHED              0x00020000
#define EMASK_CONN_OFFERED                      0x00040000
#define EMASK_CONN_QUEUED                       0x00080000
#define EMASK_CALL_ACTIVE                       0x00100000
#define EMASK_CALL_INVALID                      0x00200000
#define EMASK_CALL_META                         0x00400000


// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class PtTerminalConnectionEvent ;
class PtConnectionEvent ;
class PtCallEvent ;
class PtEvent ;
class PtSingleCallMetaEvent ;
class PtMultiCallMetaEvent ;


void fireTerminalConnectionEvent(jobject jobjListener,
								 const char* zMethod,
								 const PtTerminalConnectionEvent& rEvent) ;
  //:Fire off a terminal connection event to the specified listener

void fireConnectionEvent(jobject jobjListener,
						 const char* szMethod,
						 const PtConnectionEvent& rEvent) ;
  //:Fire off a connection event to the specified listener

void fireCallEvent(jobject jobjListener,
				   const char* szMethod,
				   const PtCallEvent& rEvent) ;
  //:Fire off a call event to the specified listener

void fireSingleCallMetaEvent(jobject jobjListener,
                             const char* szMethod,
                             const PtSingleCallMetaEvent& rEvent) ;
  //:Fire off a single call meta event to the specified listener

void fireMultiCallMetaEvent(jobject jobjListener,
                             const char* szMethod,
                             const PtMultiCallMetaEvent& rEvent) ;

  //:Fire off a multi call meta event to the specified listener

jobject createMetaEvent(const PtEvent &rEvent, JNIEnv* pEnv) ;
  //:Create either a SingleCallMetaEvent or MultiCallMetaEvent depending 
  // on the runtime class of rEvent.getMetaEvent(...).

jobject createTerminalConnectionEvent(const PtTerminalConnectionEvent& rEvent, 
                                      JNIEnv* pEnv) ;
   //: Create a JTAPI TC event and populate it with data from the PTAPI event

jobject createConnectionEvent(const PtConnectionEvent& rEvent, JNIEnv* pEnv) ;
   //: Create a JTAPI connection event and populate it with data from the 
   // PTAPI event

jobject createCallEvent(const PtCallEvent& rEvent, JNIEnv* pEnv) ;
   //: Create a JTAPI call event and populate it with data from the PTAPI
   // event

jobject createSingleCallMetaEvent(const PtSingleCallMetaEvent& rSingleCallMetaEvent, JNIEnv* pEnv) ;
   //: Create a JTAPI single call meta event and populate it with data from 
   // the PTAPI event

jobject createMultiCallMetaEvent(const PtMultiCallMetaEvent& rMultiCallMetaEvent, JNIEnv* pEnv) ;
   //: Create a JTAPI multi call meta event and populate it with data from 
   // the PTAPI event

void initializeJNICaches(JNIEnv* pEnv) ;
   //: Initialize class/field/methiod cache for call events

JNIEnv* getJNIEnv() ;
   //: Gets a shared JVM for all TaoListenerTask events


#endif // #ifndef _CALL_LISTENER_HELPERS_H
