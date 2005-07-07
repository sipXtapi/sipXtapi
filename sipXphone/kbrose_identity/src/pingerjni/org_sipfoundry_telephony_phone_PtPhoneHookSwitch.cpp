// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/com_pingtel_telephony_phone_PtPhoneHookSwitch.cpp#2 $
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

#include "ptapi/PtPhoneHookswitch.h"
#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"


/*
 * Class:     org_sipfoundry_telephony_phone_PtPhoneHookSwitch
 * Method:    JNI_getHookswitchState
 * Signature: (J)I
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_telephony_phone_PtPhoneHookSwitch_JNI_1getHookswitchState
  (JNIEnv *jenv, jclass clazz, jlong lHandle)
{
	jint iRC = 0 ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_phone_PtPhoneHookSwitch_JNI_1getHookswitchState") ;

	PtPhoneHookswitch* pHookSwitch = (PtPhoneHookswitch*) (unsigned int) lHandle ;
	if (pHookSwitch != NULL) {
		int i ;
		PtStatus status = pHookSwitch->getHookswitchState(i) ;
		if (status != 0)
			API_FAILURE(status) ;

		iRC = i ;
	}

	JNI_END_METHOD() ;

	return iRC ;
}


/*
 * Class:     org_sipfoundry_telephony_phone_PtPhoneHookSwitch
 * Method:    JNI_setHookswitchState
 * Signature: (JI)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_phone_PtPhoneHookSwitch_JNI_1setHookswitchState
  (JNIEnv *jenv, jclass clazz, jlong lHandle, jint jiHookState)
{
	PtPhoneHookswitch* pHookSwitch = (PtPhoneHookswitch*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_phone_PtPhoneHookSwitch_JNI_1setHookswitchState") ;

	if (pHookSwitch != NULL) {
		PtStatus status = pHookSwitch->setHookswitchState(jiHookState) ;
		if (status != 0)
			API_FAILURE(status) ;
	}
	JNI_END_METHOD() ;

}
