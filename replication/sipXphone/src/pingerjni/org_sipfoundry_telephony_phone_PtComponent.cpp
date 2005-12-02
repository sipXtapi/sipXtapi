// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/com_pingtel_telephony_phone_PtComponent.cpp#2 $
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
#include "ptapi/PtPhoneButton.h"
#include "ptapi/PtPhoneHookswitch.h"
#include "ptapi/PtPhoneLamp.h"
#include "ptapi/PtPhoneMicrophone.h"
#include "ptapi/PtPhoneRinger.h"
#include "ptapi/PtPhoneSpeaker.h"
#include "ptapi/PtPhoneDisplay.h"

#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"

/*
 * Class:     org_sipfoundry_telephony_phone_PtComponent
 * Method:    JNI_getName
 * Signature: (J)Ljava/lang/String;
 */
extern "C"
JNIEXPORT jstring JNICALL Java_org_sipfoundry_telephony_phone_PtComponent_JNI_1getName
  (JNIEnv *jenv, jclass clazz, jlong lHandle)
{
	jstring jsName = NULL ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_phone_PtComponent_JNI_1getName") ;

	PtComponent* pComponent = (PtComponent*) (unsigned int) lHandle ;
	if (pComponent != NULL) {
		char  szName[64] ;

		PtStatus status = pComponent->getName(szName, sizeof(szName)) ;
		if (status != 0)
			API_FAILURE(status) ;


		// native -> java
		jsName = jenv->NewStringUTF(szName) ;
	}

	JNI_END_METHOD() ;

	return jsName ;
}

/*
 * Class:     org_sipfoundry_telephony_phone_PtComponent
 * Method:    JNI_getType
 * Signature: (J)I
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_telephony_phone_PtComponent_JNI_1getType
  (JNIEnv *jenv, jclass clazz, jlong lHandle)
{
	int iRC ;
	jint rc = -1 ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_phone_PtComponent_JNI_1getType") ;

	PtComponent* pComponent = (PtComponent*) (unsigned int) lHandle ;
	if (pComponent != NULL) {


		PtStatus status = pComponent->getType(iRC) ;
		if (status != 0)
			API_FAILURE(status) ;

		rc = iRC ;
	}

	JNI_END_METHOD() ;

	return rc ;
}


/*
 * Class:     org_sipfoundry_telephony_phone_PtComponent
 * Method:    JNI_finalize
 * Signature: (J)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_phone_PtComponent_JNI_1finalize
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	PtComponent* pComponent = (PtComponent*) (unsigned int) lHandle ;
	if (pComponent != NULL) {

#if DEBUG_FINALIZE
		if (IsJNIDebuggingEnabled())
			osPrintf("JNI: freeing component 0x%08X\n", lHandle) ;
#endif

		int iType ;
		if (pComponent->getType(iType) == 0) {
			switch (iType) {
				case PtComponent::BUTTON:
					delete (PtPhoneButton*) pComponent ;
					break ;
				case PtComponent::HOOKSWITCH:
					delete (PtPhoneHookswitch*) pComponent ;
					break ;
				case PtComponent::LAMP:
					delete (PtPhoneLamp*) pComponent ;
					break ;
				case PtComponent::MICROPHONE:
					delete (PtPhoneMicrophone*) pComponent ;
					break ;
				case PtComponent::RINGER:
					delete (PtPhoneRinger*) pComponent ;
					break ;
				case PtComponent::SPEAKER:
					delete (PtPhoneSpeaker*) pComponent ;
					break ;
				case PtComponent::DISPLAY:
					delete (PtPhoneDisplay*) pComponent ;
					break ;
				default:
					delete pComponent ;
					break ;
			}
		}
	}
}
