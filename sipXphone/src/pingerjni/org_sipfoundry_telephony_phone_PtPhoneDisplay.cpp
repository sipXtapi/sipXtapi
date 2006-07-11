// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/com_pingtel_telephony_phone_PtPhoneDisplay.cpp#3 $
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

#include "ptapi/PtPhoneDisplay.h"
#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"


// Setting this to TRUE will dump out all the getters and setters coming 
// through this level.
#undef DEBUG_LCD_CONTRAST_SETTINGS


/*
 * Class:     org_sipfoundry_telephony_phone_PtPhoneDisplay
 * Method:    JNI_getContrast
 * Signature: (J)I
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_telephony_phone_PtPhoneDisplay_JNI_1getContrast
  (JNIEnv *pEnv, jclass clazz, long lHandle)
{	
	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_phone_PtPhoneDisplay_JNI_1getContrast") ;

	jint			iRC = 0 ;
	PtPhoneDisplay* pDisplay = (PtPhoneDisplay*) (unsigned int) lHandle ;

	if (pDisplay != NULL) {
		int iLevel ;
		int iLow ;
		int iHigh ;
		int iNominal ;

		PtStatus status = pDisplay->getContrast(iLevel, iLow, iHigh, iNominal) ;
#ifdef DEBUG_LCD_CONTRAST_SETTINGS
		osPrintf("Get Contrast Level: %d\n", iLevel) ;
#endif
		iRC = (jint) iLevel ;
		if (status != 0)
			API_FAILURE(status) ;
	}

	JNI_END_METHOD() ;

	return iRC ;
}


/*
 * Class:     org_sipfoundry_telephony_phone_PtPhoneDisplay
 * Method:    JNI_getContrastHigh
 * Signature: (J)I
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_telephony_phone_PtPhoneDisplay_JNI_1getContrastHigh
  (JNIEnv *pEnv, jclass clazz, long lHandle)
{
	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_phone_PtPhoneDisplay_JNI_1getContrastHigh") ;

	jint			iRC = 0 ;
	PtPhoneDisplay* pDisplay = (PtPhoneDisplay*) (unsigned int) lHandle ;

	if (pDisplay != NULL) {
		int iLevel ;
		int iLow ;
		int iHigh ;
		int iNominal ;

		PtStatus status = pDisplay->getContrast(iLevel, iLow, iHigh, iNominal) ;
#ifdef DEBUG_LCD_CONTRAST_SETTINGS
		osPrintf("Get Contrast High: %d\n", iHigh) ;
#endif
		iRC = (jint) iHigh ;
		if (status != 0)
			API_FAILURE(status) ;
	}

	JNI_END_METHOD() ;

	return iRC ;
}


/*
 * Class:     org_sipfoundry_telephony_phone_PtPhoneDisplay
 * Method:    JNI_getContrastLow
 * Signature: (J)I
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_telephony_phone_PtPhoneDisplay_JNI_1getContrastLow
  (JNIEnv *pEnv, jclass clazz, long lHandle)
{
	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_phone_PtPhoneDisplay_JNI_1getContrastLow") ;

	jint			iRC = 0 ;
	PtPhoneDisplay* pDisplay = (PtPhoneDisplay*) (unsigned int) lHandle ;

	if (pDisplay != NULL) {
		int iLevel ;
		int iLow ;
		int iHigh ;
		int iNominal ;

		PtStatus status = pDisplay->getContrast(iLevel, iLow, iHigh, iNominal) ;
#ifdef DEBUG_LCD_CONTRAST_SETTINGS
		osPrintf("Get Contrast High: %d\n", iLow) ;
#endif
		iRC = (jint) iLow ;
		if (status != 0)
			API_FAILURE(status) ;
	}

	JNI_END_METHOD() ;

	return iRC ;
}

/*
 * Class:     org_sipfoundry_telephony_phone_PtPhoneDisplay
 * Method:    JNI_getContrastNominal
 * Signature: (J)I
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_telephony_phone_PtPhoneDisplay_JNI_1getContrastNominal
  (JNIEnv *pEnv, jclass clazz, long lHandle)
{
	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_phone_PtPhoneDisplay_JNI_1getContrastNominal") ;

	jint			iRC = 0 ;
	PtPhoneDisplay* pDisplay = (PtPhoneDisplay*) (unsigned int) lHandle ;

	if (pDisplay != NULL) {
		int iLevel ;
		int iLow ;
		int iHigh ;
		int iNominal ;

		PtStatus status = pDisplay->getContrast(iLevel, iLow, iHigh, iNominal) ;
#ifdef DEBUG_LCD_CONTRAST_SETTINGS
		osPrintf("Get Contrast Nominal: %d\n", iNominal) ;
#endif
		iRC = (jint) iNominal ;

		if (status != 0)
			API_FAILURE(status) ;
	}


	JNI_END_METHOD() ;

	return iRC ;
}

/*
 * Class:     org_sipfoundry_telephony_phone_PtPhoneDisplay
 * Method:    JNI_setContrast
 * Signature: (JI)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_phone_PtPhoneDisplay_JNI_1setContrast
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jint iLevel) 
{
	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_phone_PtPhoneDisplay_JNI_1setContrast") ;

	PtPhoneDisplay* pDisplay = (PtPhoneDisplay*) (unsigned int) lHandle ;

	if (pDisplay != NULL) {
		PtStatus status = pDisplay->setContrast((int) iLevel) ;
#ifdef DEBUG_LCD_CONTRAST_SETTINGS
		osPrintf("Set Contrast Level: %d\n", iLevel) ;
#endif

		if (status != 0)
			API_FAILURE(status) ;
	}


	JNI_END_METHOD() ;

}
