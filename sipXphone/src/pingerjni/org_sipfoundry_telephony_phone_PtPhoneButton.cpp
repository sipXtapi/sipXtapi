// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/com_pingtel_telephony_phone_PtPhoneButton.cpp#2 $
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

#include "ptapi/PtPhoneButton.h"
#include "ptapi/PtPhoneLamp.h"
#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"

/*
 * Class:     org_sipfoundry_telephony_phone_PtPhoneButton
 * Method:    JNI_buttonPress
 * Signature: (J)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_phone_PtPhoneButton_JNI_1buttonPress
  (JNIEnv *jenv, jclass clazz, jlong lHandle)
{
	PtPhoneButton* pPhoneButton = (PtPhoneButton*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_phone_PtPhoneButton_JNI_1buttonPress") ;

	if (pPhoneButton != NULL) {
		PtStatus status = pPhoneButton->buttonPress() ;
		if (status != 0)
			API_FAILURE(status) ;

	}

	JNI_END_METHOD() ;
}


/*
 * Class:     org_sipfoundry_telephony_phone_PtPhoneButton
 * Method:    JNI_getInfo
 * Signature: (J)Ljava/lang/String;
 */
extern "C"
JNIEXPORT jstring JNICALL Java_org_sipfoundry_telephony_phone_PtPhoneButton_JNI_1getInfo
  (JNIEnv *jenv, jclass clazz, jlong lHandle)
{
	jstring jsRC = NULL ;
	char    szInfo[64] ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_phone_PtPhoneButton_JNI_1getInfo") ;

	strcpy(szInfo, "") ;
	PtPhoneButton* pPhoneButton = (PtPhoneButton*) (unsigned int) lHandle ;
	if (pPhoneButton != NULL) {

		strcpy(szInfo, "") ;
		PtStatus status = pPhoneButton->getInfo(szInfo, sizeof(szInfo)) ;
		if (status != 0)
			API_FAILURE(status) ;
	}


	jsRC = jenv->NewStringUTF(szInfo) ;

	JNI_END_METHOD() ;

	return jsRC ;
}


/*
 * Class:     org_sipfoundry_telephony_phone_PtPhoneButton
 * Method:    JNI_setInfo2
 * Signature: (JLjava/lang/String;)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_phone_PtPhoneButton_JNI_1setInfo2
  (JNIEnv *jenv, jclass clazz, jlong lHandle, jstring jsInfo)
{
	PtPhoneButton* pPhoneButton = (PtPhoneButton*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_phone_PtPhoneButton_JNI_1setInfo2") ;

	if (pPhoneButton != NULL) {

		// java -> native
		char* szInfo = (jsInfo) ? (char *)jenv->GetStringUTFChars(jsInfo, 0) : NULL ;

//		osPrintf("JNI: PtPhoneButton:: setInfo (string): %s\n", szInfo) ;

		PtStatus status = pPhoneButton->setInfo(szInfo) ;
		if (status != 0)
			API_FAILURE(status) ;

		
		// clean up
		if (szInfo) jenv->ReleaseStringUTFChars(jsInfo, szInfo) ;
	}

	JNI_END_METHOD() ;

}


/*
 * Class:     org_sipfoundry_telephony_phone_PtPhoneButton
 * Method:    JNI_getAssociatedPhoneLamp
 * Signature: (J)J
 */
extern "C"
JNIEXPORT jlong JNICALL Java_org_sipfoundry_telephony_phone_PtPhoneButton_JNI_1getAssociatedPhoneLamp
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	long lRC = 0 ;
	PtPhoneButton* pPhoneButton = (PtPhoneButton*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_phone_PtPhoneButton_JNI_1getAssociatedPhoneLamp") ;
	
	if (pPhoneButton != NULL) {
		PtPhoneLamp *pLamp = new PtPhoneLamp() ;
		
		PtStatus status = pPhoneButton->getAssociatedPhoneLamp(*pLamp) ;
		if (status != 0)
			API_FAILURE(status) ;

		lRC = (long) pLamp ;
	}
	
	JNI_END_METHOD() ;
	return lRC ;
}
