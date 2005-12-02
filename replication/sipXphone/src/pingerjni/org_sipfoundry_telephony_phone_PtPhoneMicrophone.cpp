// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/com_pingtel_telephony_phone_PtPhoneMicrophone.cpp#2 $
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

#include "ptapi/PtComponent.h"
#include "ptapi/PtPhoneMicrophone.h"
#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"

/*
 * Class:     org_sipfoundry_telephony_phone_PtPhoneMicrophone
 * Method:    JNI_getGain
 * Signature: (J)I
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_telephony_phone_PtPhoneMicrophone_JNI_1getGain
  (JNIEnv *jenv, jclass clazz, jlong lHandle)
{
	jint jiRC = 0 ;
	int  iRC = 0 ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_phone_PtPhoneMicrophone_JNI_1getGain") ;

	PtPhoneMicrophone* pPhoneMicrophone = (PtPhoneMicrophone*) (unsigned int) lHandle ;
	if (pPhoneMicrophone != NULL) {
		PtStatus status = pPhoneMicrophone->getGain(iRC) ;
		if (status != 0)
			API_FAILURE(status) ;

		jiRC = iRC ;
	}

	JNI_END_METHOD() ;

	return jiRC ;

}

/*
 * Class:     org_sipfoundry_telephony_phone_PtPhoneMicrophone
 * Method:    JNI_setGain
 * Signature: (JI)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_phone_PtPhoneMicrophone_JNI_1setGain
  (JNIEnv *jenv, jclass clazz, jlong lHandle, jint jiGain)
{
	PtPhoneMicrophone* pPhoneMicrophone = (PtPhoneMicrophone*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_phone_PtPhoneMicrophone_JNI_1setGain") ;

	if (pPhoneMicrophone != NULL) {
		PtStatus status = pPhoneMicrophone->setGain(jiGain) ;
		if (status != 0)
			API_FAILURE(status) ;
	}

	JNI_END_METHOD() ;
}
