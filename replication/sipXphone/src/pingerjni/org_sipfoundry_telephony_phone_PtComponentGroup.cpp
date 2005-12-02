// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/com_pingtel_telephony_phone_PtComponentGroup.cpp#2 $
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

#include "ptapi/PtComponentGroup.h"
#include "ptapi/PtComponent.h"
#include "ptapi/PtPhoneButton.h"
#include "ptapi/PtPhoneHookswitch.h"
#include "ptapi/PtPhoneMicrophone.h"
#include "ptapi/PtPhoneSpeaker.h"
#include "ptapi/PtPhoneRinger.h"
#include "ptapi/PtPhoneLamp.h"
#include "ptapi/PtPhoneDisplay.h"
#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"


/*
 * Class:     org_sipfoundry_telephony_phone_PtComponentGroup
 * Method:    JNI_activate
 * Signature: (J)Z
 */
extern "C"
JNIEXPORT jboolean JNICALL Java_org_sipfoundry_telephony_phone_PtComponentGroup_JNI_1activate
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	jboolean bRC = FALSE ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_phone_PtComponentGroup_JNI_1activate") ;

	PtComponentGroup* pGroup = (PtComponentGroup*) (unsigned int) lHandle ;
	if (pGroup != NULL) {
		bRC = (jboolean) pGroup->activate() ;
	}

	JNI_END_METHOD() ;

	return bRC ;
}


/*
 * Class:     org_sipfoundry_telephony_phone_PtComponentGroup
 * Method:    JNI_deactivate
 * Signature: (J)Z
 */
extern "C"
JNIEXPORT jboolean JNICALL Java_org_sipfoundry_telephony_phone_PtComponentGroup_JNI_1deactivate
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	jboolean bRC = FALSE ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_phone_PtComponentGroup_JNI_1deactivate") ;

	PtComponentGroup* pGroup = (PtComponentGroup*) (unsigned int) lHandle ;
	if (pGroup != NULL) {
		bRC = (jboolean) pGroup->deactivate() ;
	}

	JNI_END_METHOD() ;

	return bRC ;
}


/*
 * Class:     org_sipfoundry_telephony_phone_PtComponentGroup
 * Method:    JNI_getComponents
 * Signature: (J)[J
 */
extern "C"
JNIEXPORT jlongArray JNICALL Java_org_sipfoundry_telephony_phone_PtComponentGroup_JNI_1getComponents
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	jlongArray components = NULL ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_phone_PtComponentGroup_JNI_1getComponents") ;

	PtComponentGroup* pGroup= (PtComponentGroup*) (unsigned int) lHandle ;
	if (pGroup != NULL) {
		// We support a max of 8 components?!?
		PtComponent* comps[16] ;
		int size = 16 ;
		int items = 0 ;

		PtStatus status = pGroup->getComponents(comps, size, items) ;
		if (status != 0)
			API_FAILURE(status) ;


		// convert to an array of longs
		jlong longs[16] ;
		int j = 0 ;
		for (int i=0; i<items; i++) {
			PtComponent* pComponent = NULL ;
			int			 iType ;

			comps[i]->getType(iType) ;

			switch (iType) {
				case PtComponent::BUTTON:
					pComponent = new PtPhoneButton(*((PtPhoneButton*)comps[i])) ;
					break ;
				case PtComponent::HOOKSWITCH:
					pComponent = new PtPhoneHookswitch(*((PtPhoneHookswitch*)comps[i])) ;
					break ;
				case PtComponent::LAMP:
					pComponent = new PtPhoneLamp(*((PtPhoneLamp*)comps[i])) ;
					break ;
				case PtComponent::MICROPHONE:
					pComponent = new PtPhoneMicrophone(*((PtPhoneMicrophone*)comps[i])) ;
					break ;
				case PtComponent::RINGER:
					pComponent = new PtPhoneRinger(*((PtPhoneRinger*)comps[i])) ;
					break ;
				case PtComponent::SPEAKER:
					pComponent = new PtPhoneSpeaker(*((PtPhoneSpeaker*)comps[i])) ;
					break ;
				case PtComponent::DISPLAY:
					pComponent = new PtPhoneDisplay(*((PtPhoneDisplay*)comps[i])) ;
					break ;
				default:
					break ;
					// osPrintf("JNI: getComponents: Unexpected component type: %d\n", iType) ;
			}

			if (pComponent != NULL) {			
				longs[j++] = (jlong) (unsigned int) pComponent ;
			}
		}

		// bulk copy into our array
		components = pEnv->NewLongArray(j) ;
		pEnv->SetLongArrayRegion(components, 0, j, longs) ;
	} else {
		components = pEnv->NewLongArray(0) ;
	}

	JNI_END_METHOD() ;

	return components ;
}


/*
 * Class:     org_sipfoundry_telephony_phone_PtComponentGroup
 * Method:    JNI_getDescription
 * Signature: (J)Ljava/lang/String;
 */
extern "C"
JNIEXPORT jstring JNICALL Java_org_sipfoundry_telephony_phone_PtComponentGroup_JNI_1getDescription
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	jstring jsRC = NULL ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_phone_PtComponentGroup_JNI_1getDescription") ;
	
	PtComponentGroup* pGroup = (PtComponentGroup*) (unsigned int) lHandle ;
	if (pGroup != NULL) {
		char	szDescription[128] ;
		szDescription[0] = 0 ;
		pGroup->getDescription(szDescription, sizeof(szDescription)) ;

		// Create a java string
		if (szDescription != NULL) {
			jsRC = pEnv->NewStringUTF(szDescription) ;
		}
	}

	JNI_END_METHOD() ;

	return jsRC ;
}


/*
 * Class:     org_sipfoundry_telephony_phone_PtComponentGroup
 * Method:    JNI_getType
 * Signature: (J)I
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_telephony_phone_PtComponentGroup_JNI_1getType
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	jint iRC = (jint) PtComponentGroup::OTHER ;
	PtComponentGroup* pGroup = (PtComponentGroup*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_phone_PtComponentGroup_JNI_1getType") ;

	if (pGroup != NULL) {
		iRC = (jint) pGroup->getType() ;
	}

	JNI_END_METHOD() ;

	return iRC ;
}


/*
 * Class:     org_sipfoundry_telephony_phone_PtComponentGroup
 * Method:    JNI_activate_addr
 * Signature: (JJ)Z
 */
extern "C"
JNIEXPORT jboolean JNICALL Java_org_sipfoundry_telephony_phone_PtComponentGroup_JNI_1activate_1addr
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jlong lAddress)
{
	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_phone_PtComponentGroup_JNI_1activate_1addr") ;

	JNI_END_METHOD() ;

	return 0 ;
}


/*
 * Class:     org_sipfoundry_telephony_phone_PtComponentGroup
 * Method:    JNI_deactivate_addr
 * Signature: (JJ)Z
 */
extern "C"
JNIEXPORT jboolean JNICALL Java_org_sipfoundry_telephony_phone_PtComponentGroup_JNI_1deactivate_1addr
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jlong lAddress)
{
	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_phone_PtComponentGroup_JNI_1deactivate_1addr") ;

	JNI_END_METHOD() ;

	return 0 ;
}


/*
 * Class:     org_sipfoundry_telephony_phone_PtComponentGroup
 * Method:    JNI_finalize
 * Signature: (J)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_phone_PtComponentGroup_JNI_1finalize
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	if (lHandle != 0) {
#if DEBUG_FINALIZE
		if (IsJNIDebuggingEnabled())
			osPrintf("JNI: freeing component group 0x%08X\n", lHandle) ;
#endif
		delete (PtComponentGroup*) (unsigned int) lHandle ;
	}
}
