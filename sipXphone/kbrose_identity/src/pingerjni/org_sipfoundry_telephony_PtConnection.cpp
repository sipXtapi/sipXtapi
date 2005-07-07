// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/com_pingtel_telephony_PtConnection.cpp#2 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#include <jni.h>

#include "os/OsDefs.h"

#include "ptapi/PtConnection.h"
#include "ptapi/PtTerminalConnection.h"
#include "ptapi/PtAddress.h"
#include "ptapi/PtCall.h"
#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"

/*
 * Class:     org_sipfoundry_telephony_PtConnection
 * Method:    JNI_getState
 * Signature: (J)I
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_telephony_PtConnection_JNI_1getState
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	PtConnection* pConnection = (PtConnection*) (unsigned int) lHandle ;
	int iState = PtConnection::UNKNOWN ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtConnection_JNI_1getState") ;

	if (pConnection != NULL) {

		PtStatus status = pConnection->getState(iState) ;
		if (status != 0)
         API_FAILURE(status) ;			
	}

	JNI_END_METHOD() ;

	return iState ;
}


/*
 * Class:     org_sipfoundry_telephony_PtConnection
 * Method:    JNI_disconnect
 * Signature: (J)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_PtConnection_JNI_1disconnect
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	PtConnection* pConnection = (PtConnection*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtConnection_JNI_1getState") ;

	if (pConnection != NULL) {
		PtStatus status = pConnection->disconnect() ;
		if (status != 0)
			API_FAILURE(status) ;
	}

   JNI_END_METHOD() ;
}


/*
 * Class:     org_sipfoundry_telephony_PtConnection
 * Method:    JNI_isCallControl
 * Signature: (J)Z
 */
extern "C"
JNIEXPORT jboolean JNICALL Java_org_sipfoundry_telephony_PtConnection_JNI_1isCallControl
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtConnection_JNI_1isCallControl") ;

	JNI_END_METHOD() ;

	return true ;
}



/*
 * Class:     org_sipfoundry_telephony_PtConnection
 * Method:    JNI_getTerminalConnections
 * Signature: (J)[J
 */
extern "C"
JNIEXPORT jlongArray JNICALL Java_org_sipfoundry_telephony_PtConnection_JNI_1getTerminalConnections
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	PtConnection* pConnection = (PtConnection*) (unsigned int) lHandle ;
	jlongArray    connections = NULL ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtConnection_JNI_1getTerminalConnections") ;

	if (pConnection != NULL) {
		// JTAPI world for an array of connections as a return value
		PtTerminalConnection native_connections[4] ;
		int nItems = 0 ;

		PtStatus status = pConnection->getTerminalConnections(native_connections, 4, nItems) ;
		if (status != 0)
			API_FAILURE(status) ;

//osPrintf("JNI: getTerminalConnections nItems=%d\n", nItems) ;
		
		// convert to an array of longs
		jlong longs[4] ;
		for (int i=0; i<nItems; i++) {
			PtTerminalConnection* pTerminalConnection = new PtTerminalConnection() ;
			*pTerminalConnection = native_connections[i] ;
			longs[i] = (jlong) (unsigned int) pTerminalConnection ;
		}

		// bulk copy into our array
		connections = pEnv->NewLongArray(nItems) ;
		pEnv->SetLongArrayRegion(connections, 0, nItems, longs) ;
	}

	JNI_END_METHOD() ;

	return connections ;	
}


/*
 * Class:     org_sipfoundry_telephony_PtConnection
 * Method:    JNI_getAddress
 * Signature: (J)J
 */
extern "C"
JNIEXPORT jlong JNICALL Java_org_sipfoundry_telephony_PtConnection_JNI_1getAddress
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	jlong	   lAddress = 0 ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtConnection_JNI_1getAddress") ;

	PtConnection* pConnection = (PtConnection*) (unsigned int) lHandle ;
	if (pConnection != NULL) {
		PtAddress* pAddress = new PtAddress() ;

		PtStatus status = pConnection->getAddress(*pAddress) ;
		if (status != 0)
			API_FAILURE(status) ;

		lAddress = (jlong) (unsigned int) pAddress ;
	}

	JNI_END_METHOD() ;

	return lAddress ;
}


/*
 * Class:     org_sipfoundry_telephony_PtConnection
 * Method:    JNI_getCall
 * Signature: (J)J
 */
extern "C"
JNIEXPORT jlong JNICALL Java_org_sipfoundry_telephony_PtConnection_JNI_1getCall
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	jlong   lCall = 0 ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtConnection_JNI_1getCall") ;
	
	PtConnection* pConnection = (PtConnection*) (unsigned int) lHandle ;
	if (pConnection != NULL) {
		PtCall* pCall = new PtCall() ;

		PtStatus status = pConnection->getCall(*pCall) ;
		if (status != 0)
			API_FAILURE(status) ;

		lCall = (jlong) (unsigned int) pCall ;
	}

	JNI_END_METHOD() ;

	return lCall ;
}


/*
 * Class:     org_sipfoundry_telephony_PtConnection
 * Method:    JNI_getToURI
 * Signature: (J)Ljava/lang/String;
 */
extern "C"
JNIEXPORT jstring JNICALL Java_org_sipfoundry_telephony_PtConnection_JNI_1getToURI
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
    jstring jsRC = NULL ;
	PtConnection* pConnection = (PtConnection*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtConnection_JNI_1getToURI") ;

	if (pConnection != NULL) {
		char szURI[256] ;
		strcpy(szURI, "") ;

        pConnection->getToField(szURI, sizeof(szURI)) ;		
        if (szURI) {
			jsRC = pEnv->NewStringUTF(szURI) ;
		} else {
			jsRC = pEnv->NewStringUTF("") ;
		}
	}

	JNI_END_METHOD() ;
    return jsRC ;
}




/*
 * Class:     org_sipfoundry_telephony_PtConnection
 * Method:    JNI_getFromURI
 * Signature: (J)Ljava/lang/String;
 */
extern "C" 
JNIEXPORT jstring JNICALL Java_org_sipfoundry_telephony_PtConnection_JNI_1getFromURI
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
    jstring jsRC = NULL ;
	PtConnection* pConnection = (PtConnection*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtConnection_JNI_1getFromURI") ;

	if (pConnection != NULL) {
		char szURI[256] ;
		strcpy(szURI, "") ;

        pConnection->getFromField(szURI, sizeof(szURI)) ;		
        if (szURI) {
			jsRC = pEnv->NewStringUTF(szURI) ;
		} else {
			jsRC = pEnv->NewStringUTF("") ;
		}
	}

	JNI_END_METHOD() ;
    return jsRC ;
}

/*
 * Class:     org_sipfoundry_telephony_PtConnection
 * Method:    JNI_getSession
 * Signature: (J)Lorg/sipfoundry/sip/SipSession;
 */
#define CLASS_SIP_SESSION  "org/sipfoundry/sip/SipSession"
#define CSIG_SIP_SESSION   "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V"

extern "C"
JNIEXPORT jobject JNICALL Java_org_sipfoundry_telephony_PtConnection_JNI_1getSession
  (JNIEnv* pEnv, jclass clazz, jlong lHandle)
{
    jobject objRC = NULL ;
	PtConnection* pConnection = (PtConnection*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtConnection_JNI_1getSession") ;

	if (pConnection != NULL) 
   {
      PtSessionDesc session ;
      PtStatus status = pConnection->getSessionInfo(session) ;

		if (status != 0)
      {
		   API_FAILURE(status) ;
      }
      else
      {
         UtlString callId ;
         UtlString toUrl ;
         UtlString fromUrl ;
         UtlString localContact ;

         session.getCallId(callId) ;
         session.getToUrl(toUrl) ;
         session.getFromUrl(fromUrl) ;
         session.getLocalContact(localContact) ;

         jclass objClass = pEnv->FindClass(CLASS_SIP_SESSION) ;
	      if (objClass != NULL) 
         {
            jmethodID cid = pEnv->GetMethodID(objClass, "<init>", CSIG_SIP_SESSION) ;
            if (cid != NULL) 
            {
               jstring jsCallId = CSTRTOJSTR(pEnv, callId.data()) ;
               jstring jsToUrl = CSTRTOJSTR(pEnv, toUrl.data()) ;
               jstring jsFromUrl = CSTRTOJSTR(pEnv, fromUrl.data()) ;
               jstring jsLocalContact = CSTRTOJSTR(pEnv, localContact.data()) ;
               objRC = pEnv->NewObject(objClass, cid, jsCallId, jsToUrl, jsFromUrl, jsLocalContact) ;
            }
         }
      }
   }

   JNI_END_METHOD() ;
   return objRC ;
}



/*
 * Class:     org_sipfoundry_telephony_PtConnection
 * Method:    JNI_finalize
 * Signature: (J)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_PtConnection_JNI_1finalize
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	if (lHandle != 0) {
#if DEBUG_FINALIZE
		if (IsJNIDebuggingEnabled())
			osPrintf("JNI: freeing connection 0x%08X\n", lHandle) ;
#endif
		delete (PtConnection*) (unsigned int) lHandle ;
	}
}

