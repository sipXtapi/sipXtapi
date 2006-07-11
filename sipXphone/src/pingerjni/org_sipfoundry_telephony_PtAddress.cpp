// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/com_pingtel_telephony_PtAddress.cpp#2 $
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

#include "ptapi/PtAddress.h"
#include "ptapi/PtTerminal.h"
#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"

/*
 * Class:     org_sipfoundry_telephony_PtAddress
 * Method:    JNI_getName
 * Signature: (J)Ljava/lang/String;
 */
extern "C"
JNIEXPORT jstring JNICALL Java_org_sipfoundry_telephony_PtAddress_JNI_1getName
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	char    szName[128] ;
	jstring jsRC = NULL ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtAddress_JNI_1getName") ;

	PtAddress* pAddress = (PtAddress*) (unsigned int) lHandle ;
	if (pAddress != NULL) {
		szName[0] = 0 ;
		PtStatus status = pAddress->getName(szName, sizeof(szName)) ;
		if (status != 0)
			API_FAILURE(status) ;

		jsRC = pEnv->NewStringUTF(szName) ;
	} else
		jsRC = pEnv->NewStringUTF("") ;
	
	JNI_END_METHOD() ;

	return jsRC ;
}


/*
 * Class:     org_sipfoundry_telephony_PtAddress
 * Method:    JNI_getTerminals
 * Signature: (J)[J
 */
extern "C"
JNIEXPORT jlongArray JNICALL Java_org_sipfoundry_telephony_PtAddress_JNI_1getTerminals
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	jlongArray    terminals = NULL ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtAddress_JNI_1getTerminals") ;

	PtAddress* pAddress = (PtAddress*) (unsigned int) lHandle ;
	if (pAddress != NULL) {
		PtTerminal native_terminals[8] ;
		int nItems = 0 ;

		PtStatus status = pAddress->getTerminals(native_terminals, 8, nItems) ;
		if (status != 0)
			API_FAILURE(status) ;

		// convert to an array of longs
		jlong longs[8] ;
		for (int i=0; i<nItems; i++) {
			PtTerminal *pTerminal = new PtTerminal() ;
			*pTerminal = native_terminals[i] ;
			longs[i] = (jlong) (unsigned int) pTerminal ;
		}

		// bulk copy into our array
		terminals = pEnv->NewLongArray(nItems) ;
		pEnv->SetLongArrayRegion(terminals, 0, nItems, longs) ;
	}

	JNI_END_METHOD() ;

	return terminals ;
}


/*
 * Class:     org_sipfoundry_telephony_PtAddress
 * Method:    JNI_finalize
 * Signature: (J)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_PtAddress_JNI_1finalize
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	if (lHandle != 0) {
#if DEBUG_FINALIZE
		if (IsJNIDebuggingEnabled())
			osPrintf("JNI: freeing address 0x%08X\n", lHandle) ;
#endif
		delete (PtAddress*) (unsigned int) lHandle ;
	}
}
