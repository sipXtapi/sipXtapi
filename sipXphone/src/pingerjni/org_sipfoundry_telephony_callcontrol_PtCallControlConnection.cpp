// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/com_pingtel_telephony_callcontrol_PtCallControlConnection.cpp#2 $
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
#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"

/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlConnection
 * Method:    JNI_accept
 * Signature: (J)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlConnection_JNI_1accept
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	PtConnection* pConnection = (PtConnection*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlConnection_JNI_1accept") ;

	if (pConnection != NULL) {
		PtStatus status = pConnection->accept() ;
		if (status != 0)
			API_FAILURE(status) ;
	}

	JNI_END_METHOD() ;
}


/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlConnection
 * Method:    JNI_reject
 * Signature: (J)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlConnection_JNI_1reject
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	PtConnection* pConnection = (PtConnection*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlConnection_JNI_1reject") ;

	if (pConnection != NULL) {
		PtStatus status = pConnection->reject() ;
		if (status != 0)
			API_FAILURE(status) ;
	}
	JNI_END_METHOD() ;

}

/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlConnection
 * Method:    JNI_redirect
 * Signature: (JLjava/lang/String;)J
 */
extern "C"
JNIEXPORT jlong JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlConnection_JNI_1redirect
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jstring jsDestination)
{
	jlong		  lRC = 0 ;
	PtConnection* pConnection = (PtConnection*) (unsigned int) lHandle ;


	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlConnection_JNI_1redirect") ;

	if (pConnection != NULL) {
		char* szDestination = (jsDestination) ? (char *)pEnv->GetStringUTFChars(jsDestination, 0) : NULL ;
		PtConnection* pNewConnection = new PtConnection() ;

		PtStatus status = pConnection->redirect(szDestination, *pNewConnection) ;
		if (status != 0)
			API_FAILURE(status) ;

		lRC = (jlong) (unsigned int) pNewConnection ;

		if (szDestination) 
			pEnv->ReleaseStringUTFChars(jsDestination, szDestination) ;
	}

	JNI_END_METHOD() ;

	return lRC ;
}

