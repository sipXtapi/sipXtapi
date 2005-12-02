// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/com_pingtel_telephony_callcontrol_PtCallControlTerminalConnection.cpp#2 $
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

#include "ptapi/PtTerminalConnection.h"
#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"


/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlTerminalConnection
 * Method:    JNI_getCallControlState
 * Signature: (J)I
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlTerminalConnection_JNI_1getCallControlState
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	PtTerminalConnection* pTerminalConnection = (PtTerminalConnection*) (unsigned int) lHandle ;
	int iState = 0 ;

	JNI_BEGIN_METHOD("org_sipfoundry_telephony_callcontrol_PtCallControlTerminalConnection") ;

	if (pTerminalConnection != NULL) {
		PtStatus status = pTerminalConnection->getState(iState) ;
		if (status != 0)
			API_FAILURE(status) ;
	}	

	JNI_END_METHOD() ;

	return iState ;
}

/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlTerminalConnection
 * Method:    JNI_hold
 * Signature: (J)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlTerminalConnection_JNI_1hold
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	PtTerminalConnection* pTerminalConnection = (PtTerminalConnection*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlTerminalConnection_JNI_1hold") ;

	if (pTerminalConnection != NULL) {
		PtStatus status = pTerminalConnection->hold() ;
		if (status != 0)
			API_FAILURE(status) ;
	}	

	JNI_END_METHOD() ;

}


/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlTerminalConnection
 * Method:    JNI_join
 * Signature: (J)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlTerminalConnection_JNI_1join
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)

{
	PtTerminalConnection* pTerminalConnection = (PtTerminalConnection*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlTerminalConnection_JNI_1join") ;

	if (pTerminalConnection != NULL) {
		//PtStatus status = pTerminalConnection->join() ;
		//if (status != 0)
			//osPrintf("JNI: PtCallControlTerminalConnection::hold returned %d\n", status) ;
	}	

	JNI_END_METHOD() ;

}

/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlTerminalConnection
 * Method:    JNI_leave
 * Signature: (J)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlTerminalConnection_JNI_1leave
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	PtTerminalConnection* pTerminalConnection = (PtTerminalConnection*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlTerminalConnection_JNI_1leave") ;

	if (pTerminalConnection != NULL) {
		//PtStatus status = pTerminalConnection->join() ;
		//if (status != 0)
			//osPrintf("JNI: PtCallControlTerminalConnection::hold returned %d\n", status) ;
	}	

	JNI_END_METHOD() ;

}


/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlTerminalConnection
 * Method:    JNI_unhold
 * Signature: (J)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlTerminalConnection_JNI_1unhold
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	PtTerminalConnection* pTerminalConnection = (PtTerminalConnection*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlTerminalConnection_JNI_1unhold") ;

	if (pTerminalConnection != NULL) {
		PtStatus status = pTerminalConnection->unhold() ;
		if (status != 0)
			API_FAILURE(status) ;
	}	

	JNI_END_METHOD() ;

}
