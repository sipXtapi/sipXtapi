// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/com_pingtel_telephony_phone_PtPhoneLamp.cpp#2 $
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

#define JTAPI_LAMPMODE_OFF				0
#define JTAPI_LAMPMODE_FLASH			1
#define JTAPI_LAMPMODE_STEADY			2
#define JTAPI_LAMPMODE_FLUTTER			3
#define JTAPI_LAMPMODE_BROKENFLUTTER	4
#define JTAPI_LAMPMODE_WINK				5

int toPTAPIMode(int iJTAPIMode) ;
int toJTAPIMode(int iPTAPIMode) ;


/*
 * Class:     org_sipfoundry_telephony_phone_PtPhoneLamp
 * Method:    JNI_getAssociatedPhoneButton
 * Signature: (J)J
 */
extern "C"
JNIEXPORT jlong JNICALL Java_org_sipfoundry_telephony_phone_PtPhoneLamp_JNI_1getAssociatedPhoneButton
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	long         lRC = 0 ;
	PtPhoneLamp* pPhoneLamp = (PtPhoneLamp*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_phone_PtPhoneLamp_JNI_1getAssociatedPhoneButton") ;

	if (pPhoneLamp != NULL) {
		PtPhoneButton *pButton = new PtPhoneButton() ;
		PtStatus status = pPhoneLamp->getAssociatedPhoneButton(*pButton) ;
		if (status != 0)
			API_FAILURE(status) ;

		lRC = (long) pButton ;
	}

	JNI_END_METHOD() ;

	return lRC ;
}


/*
 * Class:     org_sipfoundry_telephony_phone_PtPhoneLamp
 * Method:    JNI_getMode
 * Signature: (J)I
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_telephony_phone_PtPhoneLamp_JNI_1getMode
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	int iMode = 0 ;
	PtPhoneLamp* pPhoneLamp = (PtPhoneLamp*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_phone_PtPhoneLamp_JNI_1getMode") ;

	if (pPhoneLamp != NULL) {		
		PtStatus status = pPhoneLamp->getMode(iMode) ;
		if (status != 0)
			API_FAILURE(status) ;
	}

	JNI_END_METHOD() ;

	return (jint) toJTAPIMode(iMode) ;  
}


/*
 * Class:     org_sipfoundry_telephony_phone_PtPhoneLamp
 * Method:    JNI_getSupportedModes
 * Signature: (J)[I
 */
extern "C"
JNIEXPORT jintArray JNICALL Java_org_sipfoundry_telephony_phone_PtPhoneLamp_JNI_1getSupportedModes
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	PtPhoneLamp* pPhoneLamp = (PtPhoneLamp*) (unsigned int) lHandle ;
	jintArray supported ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_phone_PtPhoneLamp_JNI_1getSupportedModes") ;

	if (pPhoneLamp != NULL) {
		int  iModeMask = 0 ;
		jint jisupported[6] ;
		int  iCount = 0 ;

		PtStatus status = pPhoneLamp->getSupportedModes(iModeMask) ;
		if (status != 0)
			API_FAILURE(status) ;
	
		jisupported[iCount++] = toJTAPIMode(PtPhoneLamp::MODE_OFF) ;
		if (iModeMask & PtPhoneLamp::MODE_STEADY)
			jisupported[iCount++] = toJTAPIMode(PtPhoneLamp::MODE_STEADY) ;
		if (iModeMask & PtPhoneLamp::MODE_FLASH)
			jisupported[iCount++] = toJTAPIMode(PtPhoneLamp::MODE_FLASH) ;
		if (iModeMask & PtPhoneLamp::MODE_FLUTTER)
			jisupported[iCount++] = toJTAPIMode(PtPhoneLamp::MODE_FLUTTER) ;
		if (iModeMask & PtPhoneLamp::MODE_BROKENFLUTTER)
			jisupported[iCount++] = toJTAPIMode(PtPhoneLamp::MODE_BROKENFLUTTER) ;
		if (iModeMask & PtPhoneLamp::MODE_WINK)
			jisupported[iCount++] = toJTAPIMode(PtPhoneLamp::MODE_WINK) ;


		supported = pEnv->NewIntArray(iCount) ;
		pEnv->SetIntArrayRegion(supported, 0, iCount, jisupported) ;
	}
    else
    {
        supported = pEnv->NewIntArray(0) ;
    }

	JNI_END_METHOD() ;

	return supported ; 
}


/*
 * Class:     org_sipfoundry_telephony_phone_PtPhoneLamp
 * Method:    JNI_setMode
 * Signature: (JI)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_phone_PtPhoneLamp_JNI_1setMode
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jint jiMode)
{
	PtPhoneLamp* pPhoneLamp = (PtPhoneLamp*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_phone_PtPhoneLamp_JNI_1setMode") ;

	if (pPhoneLamp != NULL) {

		PtStatus status = pPhoneLamp->setMode(toPTAPIMode((int) jiMode)) ;
		if (status != 0)
			API_FAILURE(status) ;
	}

	JNI_END_METHOD() ;
}


/*
 * Converts a JTAPI lamp mode id into a PTAPI lamp mode id
 */
int toPTAPIMode(int iJTAPIMode)
{
	int iPTAPIMode = PtPhoneLamp::MODE_STEADY ;

	switch (iJTAPIMode) {
		case JTAPI_LAMPMODE_OFF:
			iPTAPIMode = PtPhoneLamp::MODE_OFF ;
			break ;
		case JTAPI_LAMPMODE_FLASH:
			iPTAPIMode = PtPhoneLamp::MODE_FLASH ;
			break ;
		case JTAPI_LAMPMODE_STEADY:
			iPTAPIMode = PtPhoneLamp::MODE_STEADY ;
			break ;
		case JTAPI_LAMPMODE_FLUTTER:
			iPTAPIMode = PtPhoneLamp::MODE_FLUTTER ;
			break ;
		case JTAPI_LAMPMODE_BROKENFLUTTER:
			iPTAPIMode = PtPhoneLamp::MODE_BROKENFLUTTER ;
			break ;
		case JTAPI_LAMPMODE_WINK:
			iPTAPIMode = PtPhoneLamp::MODE_WINK ;
			break ;
	}  
	return iPTAPIMode ;
}


/*
 * Converts a PTAPI lamp mode id into a JTAPI lamp mode id
 */
int toJTAPIMode(int iPTAPIMode)
{
	int iJTAPIMode = JTAPI_LAMPMODE_STEADY ;

	switch (iPTAPIMode) {
		case PtPhoneLamp::MODE_OFF:
			iJTAPIMode = JTAPI_LAMPMODE_OFF ;
			break ;
		case PtPhoneLamp::MODE_FLASH:
			iJTAPIMode = JTAPI_LAMPMODE_FLASH ;
			break ;
		case PtPhoneLamp::MODE_STEADY:
			iJTAPIMode = JTAPI_LAMPMODE_STEADY ;
			break ;
		case PtPhoneLamp::MODE_FLUTTER:
			iJTAPIMode = JTAPI_LAMPMODE_FLUTTER ;
			break ;
		case PtPhoneLamp::MODE_BROKENFLUTTER:
			iJTAPIMode = JTAPI_LAMPMODE_BROKENFLUTTER ;
			break ;
		case PtPhoneLamp::MODE_WINK:
			iJTAPIMode = JTAPI_LAMPMODE_WINK ;
			break ;
	}  
	return iJTAPIMode ;
}

