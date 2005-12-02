// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/com_pingtel_telephony_phone_PtPhoneSpeaker.cpp#2 $
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

#include "ptapi/PtPhoneSpeaker.h"
#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"

/*
 * Class:     org_sipfoundry_telephony_phone_PtPhoneSpeaker
 * Method:    JNI_getVolume
 * Signature: (J)I
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_telephony_phone_PtPhoneSpeaker_JNI_1getVolume
  (JNIEnv *jenv, jclass clazz, jlong lHandle)
{
	jint jiRC = 0 ;
	int  iRC = 0 ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_phone_PtPhoneSpeaker_JNI_1getVolume") ;

	PtPhoneSpeaker* pPhoneSpeaker = (PtPhoneSpeaker*) (unsigned int) lHandle ;
	if (pPhoneSpeaker != NULL) {		
		PtStatus status = pPhoneSpeaker->getVolume(iRC) ;
		if (status != 0)
			API_FAILURE(status) ;

		jiRC = iRC ;
	}

	JNI_END_METHOD() ;

	return jiRC ;
}


/*
 * Class:     org_sipfoundry_telephony_phone_PtPhoneSpeaker
 * Method:    JNI_getNominalVolume
 * Signature: (J)I
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_telephony_phone_PtPhoneSpeaker_JNI_1getNominalVolume
  (JNIEnv *jenv, jclass clazz, jlong lHandle)
{
	jint jiRC = 0 ;
	int  iRC = 0 ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_phone_PtPhoneSpeaker_JNI_1getNominalVolume") ;

	PtPhoneSpeaker* pPhoneSpeaker = (PtPhoneSpeaker*) (unsigned int) lHandle ;
	if (pPhoneSpeaker != NULL) {		
		PtStatus status = pPhoneSpeaker->getNominalVolume(iRC) ;
		if (status != 0)
			API_FAILURE(status) ;

		jiRC = iRC ;
	}

	JNI_END_METHOD() ;

	return jiRC ;
}



/*
 * Class:     org_sipfoundry_telephony_phone_PtPhoneSpeaker
 * Method:    JNI_setVolume
 * Signature: (JI)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_phone_PtPhoneSpeaker_JNI_1setVolume
  (JNIEnv *jenv, jclass clazz, jlong lHandle, jint jiVolume)
{
	PtPhoneSpeaker* pPhoneSpeaker = (PtPhoneSpeaker*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_phone_PtPhoneSpeaker_JNI_1setVolume") ;

	if (pPhoneSpeaker != NULL) {
//		osPrintf("JNI: PtPhoneSpeaker setVolume  volume=%d, groupType=%d\n", (int) jiVolume, pPhoneSpeaker->mGroupType) ;		

		PtStatus status = pPhoneSpeaker->setVolume(jiVolume) ;
		if (status != 0)
			API_FAILURE(status) ;
	}

	JNI_END_METHOD() ;

}
