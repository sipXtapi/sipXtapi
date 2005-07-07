// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/com_pingtel_telephony_callcontrol_PtCallControlForwardingAdapter.cpp#2 $
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

#include "ptapi/PtCall.h"
#include "ptapi/PtAddress.h"
#include "ptapi/PtAddressForwarding.h"
#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"


/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlForwardingAdapter
 * Method:    JNI_constructInternalExternalType
 * Signature: (Ljava/lang/String;IZ)J
 */
extern "C"
JNIEXPORT jlong JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlForwardingAdapter_JNI_1constructInternalExternalType
  (JNIEnv *pEnv, jclass clazz, jstring jsDestination, jint jiType, jboolean jbInternal, jint jiNoAnswerTimeout)
{
	long lRC = 0 ;
	char* szDestination = (jsDestination) ? (char *)pEnv->GetStringUTFChars(jsDestination, 0) : NULL ;


	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlForwardingAdapter_JNI_1constructInternalExternalType") ;

    PtAddressForwarding* pAddressForwarding = new PtAddressForwarding((const char*) szDestination, (int) jiType, (PtBoolean) jbInternal, (int) jiNoAnswerTimeout) ;
    lRC = (unsigned int) pAddressForwarding ;
	
    // Cleanup
    if (szDestination) 
        pEnv->ReleaseStringUTFChars(jsDestination, szDestination) ;

	JNI_END_METHOD() ;

    return lRC ;	
}


/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlForwardingAdapter
 * Method:    JNI_constructSpecific
 * Signature: (Ljava/lang/String;ILjava/lang/String;)J
 */
extern "C"
JNIEXPORT jlong JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlForwardingAdapter_JNI_1constructSpecific
  (JNIEnv *pEnv, jclass clazz, jstring jsDestination, jint jiType,  jstring jsCallerAddress, jint jiTimeout)
{
	long lRC = 0 ;
    char* szDestination = (jsDestination) ? (char *)pEnv->GetStringUTFChars(jsDestination, 0) : NULL ;
    char* szCaller      = (jsCallerAddress) ? (char *)pEnv->GetStringUTFChars(jsCallerAddress, 0) : NULL ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlForwardingAdapter_JNI_1constructSpecific") ;

//	osPrintf("JNI: creating PtAddressForwarding(address=%s, type=%d, caller=%s)\n", szDestination, jiType, szCaller) ;
    PtAddressForwarding* pAddressForwarding = new PtAddressForwarding((const char*) szDestination, (int) jiType, (const char*) szCaller, (int) jiTimeout) ;
    lRC = (unsigned int) pAddressForwarding ;


    // Cleanup
    if (szDestination) 
        pEnv->ReleaseStringUTFChars(jsDestination, szDestination) ;
    if (szCaller) 
        pEnv->ReleaseStringUTFChars(jsCallerAddress, szCaller) ;

	JNI_END_METHOD() ;

    return lRC ;
}


/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlForwardingAdapter
 * Method:    JNI_constructType
 * Signature: (Ljava/lang/String;I)J
 */
extern "C"
JNIEXPORT jlong JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlForwardingAdapter_JNI_1constructType
  (JNIEnv *pEnv, jclass clazz, jstring jsDestination, jint jiType, jint jiNoAnswerTimeout)
{
	long lRC = 0 ;
    char* szDestination = (jsDestination) ? (char *)pEnv->GetStringUTFChars(jsDestination, 0) : NULL ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlForwardingAdapter_JNI_1constructType") ;

//	osPrintf("JNI: creating PtAddressForwarding(address=%s, type=%d)\n", szDestination, jiType) ;
    PtAddressForwarding* pAddressForwarding = new PtAddressForwarding((const char*) szDestination, (int) jiType, (int) jiNoAnswerTimeout) ;
    lRC = (unsigned int) pAddressForwarding ;

    // Cleanup
    if (szDestination) 
        pEnv->ReleaseStringUTFChars(jsDestination, szDestination) ;

	JNI_END_METHOD() ;

    return lRC ;
}


/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlForwardingAdapter
 * Method:    JNI_constructUnconditional
 * Signature: (Ljava/lang/String;)J
 */
extern "C"
JNIEXPORT jlong JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlForwardingAdapter_JNI_1constructUnconditional
  (JNIEnv *pEnv, jclass clazz, jstring jsDestination)
{
	long lRC = 0 ;
    char* szDestination = (jsDestination) ? (char *)pEnv->GetStringUTFChars(jsDestination, 0) : NULL ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlForwardingAdapter_JNI_1constructUnconditional") ;

//	osPrintf("JNI: BEGIN creating PtAddressForwarding(address=%s)\n", szDestination) ;
    PtAddressForwarding* pAddressForwarding = new PtAddressForwarding((const char*) szDestination) ;
	lRC = (unsigned int) pAddressForwarding ;

    // Cleanup
    if (szDestination) 
        pEnv->ReleaseStringUTFChars(jsDestination, szDestination) ;

	JNI_END_METHOD() ;

    return lRC ;	
}


/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlForwardingAdapter
 * Method:    JNI_getDestinationAddress
 * Signature: (J)Ljava/lang/String;
 */
extern "C"
JNIEXPORT jstring JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlForwardingAdapter_JNI_1getDestinationAddress
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	PtAddressForwarding* pAddressForwarding = (PtAddressForwarding*) (unsigned int) lHandle ;
	jstring jsRC = NULL ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlForwardingAdapter_JNI_1getDestinationAddress") ;

	if (pAddressForwarding != NULL) {
        char szDestination[256] ;
        memset(szDestination, 0, sizeof(szDestination)) ;

		PtStatus status = pAddressForwarding->getDestinationAddress(szDestination, sizeof(szDestination)-1) ;
//		osPrintf("JNI PtCallControlForwardingAdapter::getDestinationAddress: %s\n", szDestination) ;
		if (status != 0)
			API_FAILURE(status) ;

		if (szDestination != NULL) {
			jsRC = pEnv->NewStringUTF(szDestination) ;
		}
	}

	// Seems like the VM doesn't handle nulls very well
	if (jsRC == NULL) {
		jsRC = pEnv->NewStringUTF("") ;
	}

	JNI_END_METHOD() ;
	return jsRC ;
}

/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlForwardingAdapter
 * Method:    JNI_getFilter
 * Signature: (J)I
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlForwardingAdapter_JNI_1getFilter
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	PtAddressForwarding* pAddressForwarding = (PtAddressForwarding*) (unsigned int) lHandle ;
	jint jiFilter = 0 ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlForwardingAdapter_JNI_1getFilter") ;

	if (pAddressForwarding != NULL) {
        int iFilter = 0 ;
		PtStatus status = pAddressForwarding->getFilter(iFilter) ;
		if (status != 0)
			API_FAILURE(status) ;

        jiFilter = (jint) iFilter ;
	}

	JNI_END_METHOD() ;

	return jiFilter ;
}

/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlForwardingAdapter
 * Method:    JNI_getSpecificCaller
 * Signature: (J)Ljava/lang/String;
 */
extern "C"
JNIEXPORT jstring JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlForwardingAdapter_JNI_1getSpecificCaller
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	PtAddressForwarding* pAddressForwarding = (PtAddressForwarding*) (unsigned int) lHandle ;
	jstring jsRC = NULL ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlForwardingAdapter_JNI_1getSpecificCaller") ;

	if (pAddressForwarding != NULL) {
        char szSpecificCaller[256] ;
        memset(szSpecificCaller, 0, sizeof(szSpecificCaller)) ;
		PtStatus status = pAddressForwarding->getSpecificCaller(szSpecificCaller, sizeof(szSpecificCaller)-1) ;
//		osPrintf("JNI PtCallControlForwardingAdapter::getSpecificCaller: %s\n", szSpecificCaller) ;
		if (status != 0)
			API_FAILURE(status) ;

		if (szSpecificCaller != NULL) {
			jsRC = pEnv->NewStringUTF(szSpecificCaller) ;
		}
	}

	// Seems like the VM doesn't handle nulls very well
	if (jsRC == NULL) {
		jsRC = pEnv->NewStringUTF("") ;
	}
	JNI_END_METHOD() ;

	return jsRC ;
}


/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlForwardingAdapter
 * Method:    JNI_getType
 * Signature: (J)I
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlForwardingAdapter_JNI_1getType
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	PtAddressForwarding* pAddressForwarding = (PtAddressForwarding*) (unsigned int) lHandle ;
	jint jiRC = 0 ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlForwardingAdapter_JNI_1getType") ;

	if (pAddressForwarding != NULL) {
        int iType = 0 ;
		PtStatus status = pAddressForwarding->getType(iType) ;
//		osPrintf("JNI PtCallControlForwardingAdapter::getType: %d\n", iType) ;
		if (status != 0)
			API_FAILURE(status) ;

        jiRC = (jint) iType ;
	}
	JNI_END_METHOD() ;

	return jiRC ;
}



/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlForwardingAdapter
 * Method:    JNI_getNoAnswerTimeout
 * Signature: (J)I
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlForwardingAdapter_JNI_1getNoAnswerTimeout
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	PtAddressForwarding* pAddressForwarding = (PtAddressForwarding*) (unsigned int) lHandle ;
	jint jiRC = 0 ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlForwardingAdapter_JNI_1getNoAnswerTimeout") ;

	if (pAddressForwarding != NULL) {
        int iNoAnswerTimeout = 0 ;
		PtStatus status = pAddressForwarding->getNoAnswerTimeout(iNoAnswerTimeout) ;
		if (status != 0)
			API_FAILURE(status) ;

        jiRC = (jint) iNoAnswerTimeout ;
	}
	JNI_END_METHOD() ;

	return jiRC ;
}

/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlForwardingAdapter
 * Method:    JNI_finalize
 * Signature: (J)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlForwardingAdapter_JNI_1finalize
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	if (lHandle != 0) {
#if DEBUG_FINALIZE
		if (IsJNIDebuggingEnabled())
			osPrintf("JNI: freeing forwarding AddressForwarding 0x%08X\n", lHandle) ;
#endif
		delete (PtAddressForwarding*) (unsigned int) lHandle ;
	}
}

