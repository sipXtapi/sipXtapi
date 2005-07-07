// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/com_pingtel_telephony_callcontrol_PtCallControlAddress.cpp#2 $
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


#define MAX_FORWARDING_ADDRESSES	16 // Max number of forwarding address that will support

/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlAddress
 * Method:    JNI_cancelForwarding
 * Signature: (J)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlAddress_JNI_1cancelForwarding
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)  
{
	PtAddress* pAddress = (PtAddress*) (unsigned int) lHandle;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlAddress_JNI_1cancelForwarding") ;

	if (pAddress != NULL) {
		PtStatus status = pAddress->cancelForwarding() ;
		if (status != 0)
         API_FAILURE(status) ;			
	}

	JNI_END_METHOD() ;
}

/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlAddress
 * Method:    JNI_getDoNotDisturb
 * Signature: (J)Z
 */
extern "C"
JNIEXPORT jboolean JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlAddress_JNI_1getDoNotDisturb
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)  
{
	PtAddress* pAddress = (PtAddress*) (unsigned int) lHandle;
	PtBoolean  bRC = false ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlAddress_JNI_1getDoNotDisturb") ;

	if (pAddress != NULL) {
		PtStatus status = pAddress->getDoNotDisturb(bRC) ;
		if (status != 0)
			API_FAILURE(status) ;
	}

	JNI_END_METHOD() ;

	return (jboolean) bRC ;
}


/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlAddress
 * Method:    JNI_getForwarding
 * Signature: (J)[J
 */
extern "C"
JNIEXPORT jlongArray JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlAddress_JNI_1getForwarding
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)  
{
	PtAddress* pAddress = (PtAddress*) (unsigned int) lHandle;
	jlongArray handles = NULL ;
	
	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlAddress_JNI_1getForwarding") ;

	if (pAddress != NULL) {
		PtAddressForwarding native_instructions[MAX_FORWARDING_ADDRESSES] ;
		int nItems ;

		PtStatus status = pAddress->getForwarding(native_instructions, MAX_FORWARDING_ADDRESSES, nItems) ;
		if (status != 0)
			API_FAILURE(status) ;

		// convert to an array of longs
		jlong longs[MAX_FORWARDING_ADDRESSES] ;
		for (int i=0; i<nItems; i++) {			

			PtAddressForwarding* pForwarding = new PtAddressForwarding() ;
			*pForwarding = native_instructions[i] ;
			longs[i] = (jlong) (unsigned int) pForwarding ;
		}

		// bulk copy into our array
		handles = pEnv->NewLongArray(nItems) ;
		pEnv->SetLongArrayRegion(handles, 0, nItems, longs) ;
	}

	JNI_END_METHOD() ;

	return handles ;
}


/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlAddress
 * Method:    JNI_getMessageWaiting
 * Signature: (J)Z
 */
extern "C"
JNIEXPORT jboolean JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlAddress_JNI_1getMessageWaiting
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)  
{
	PtAddress* pAddress = (PtAddress*) (unsigned int) lHandle;
	PtBoolean  bRC = false ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlAddress_JNI_1getMessageWaiting") ;

	if (pAddress != NULL) {
		PtStatus status = pAddress->getMessageWaiting(bRC) ;
		if (status != 0)
			API_FAILURE(status) ;
	}

	JNI_END_METHOD() ;

	return (jboolean) bRC ;
}


/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlAddress
 * Method:    JNI_setDoNotDisturb
 * Signature: (JZ)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlAddress_JNI_1setDoNotDisturb
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jboolean jbEnable)
{
	PtAddress* pAddress = (PtAddress*) (unsigned int) lHandle;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlAddress_JNI_1setDoNotDisturb") ;

	if (pAddress != NULL) {
		PtStatus status = pAddress->setDoNotDisturb((PtBoolean) jbEnable) ;
		if (status != 0)
			API_FAILURE(status) ;
	}
	JNI_END_METHOD() ;

}


/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlAddress
 * Method:    JNI_setForwarding
 * Signature: (J[J)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlAddress_JNI_1setForwarding
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jlongArray jlArrayForwarding)
{
	PtAddress* pAddress = (PtAddress*) (unsigned int) lHandle;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlAddress_JNI_1setForwarding") ;
	
	if (pAddress != NULL) {
		int iArrayLength = pEnv->GetArrayLength(jlArrayForwarding) ;
		if (iArrayLength > 0) {

			// Peel open the the Java Array
			jboolean bIsCopy ;
			jlong* pElements = pEnv->GetLongArrayElements(jlArrayForwarding, &bIsCopy) ;
			
//osPrintf("\n") ;
//osPrintf("Dumping Forwarding Addresses: %d\n", iArrayLength) ;

			// Repackage the array of pointers into a format that PTAPI can use
			PtAddressForwarding *pInstructions = new PtAddressForwarding[iArrayLength] ;
			for (int i=0; i<iArrayLength; i++) {
				pInstructions[i] = *((PtAddressForwarding *) (unsigned int) pElements[i]) ;

//char szAddress[64] ;
//char szSpecCaller[64] ;
//int  iFilterType ;
//int  iType ;

//strcpy(szAddress, "") ;
//strcpy(szSpecCaller, "") ;

//pInstructions[i].getDestinationAddress(szAddress, sizeof(szAddress)) ;
//pInstructions[i].getSpecificCaller(szSpecCaller, sizeof(szSpecCaller)) ;
//pInstructions[i].getFilter(iFilterType) ;
//pInstructions[i].getType(iType) ;

//osPrintf("  %02d: address=%s, caller=%s, type=%d, filter=%d\n", i, szAddress, szSpecCaller, iType, iFilterType) ;
			}

			// Commit!
			PtStatus status = pAddress->setForwarding(pInstructions, iArrayLength) ;
			if (status != 0)
				API_FAILURE(status) ;

			// Finally, clean up
			delete[] pInstructions ;
			if (bIsCopy == JNI_TRUE) {
				pEnv->ReleaseLongArrayElements(jlArrayForwarding, pElements, 0) ;
			}	
		}
	}
	JNI_END_METHOD() ;

}


/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlAddress
 * Method:    JNI_setMessageWaiting
 * Signature: (JZ)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlAddress_JNI_1setMessageWaiting
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jboolean jbEnable)  
{
	PtAddress* pAddress = (PtAddress*) (unsigned int) lHandle;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlAddress_JNI_1setMessageWaiting") ;


	if (pAddress != NULL) {
		PtStatus status = pAddress->setMessageWaiting((PtBoolean) jbEnable) ;
		if (status != 0)
			API_FAILURE(status) ;
	}

	JNI_END_METHOD() ;

}
