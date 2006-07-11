// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/com_pingtel_telephony_PtProvider.cpp#4 $
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
#include "os/OsSocket.h"

#include "ptapi/PtProvider.h"
#include "ptapi/PtAddress.h"
#include "ptapi/PtTerminal.h"
#include "ptapi/PtCall.h"
#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"
#include "pinger/Pinger.h"

// extern UtlString g_strExtension ;

/*
 * Class:     org_sipfoundry_telephony_PtProvider
 * Method:    JNI_getProvider
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)J
 */
extern "C"
JNIEXPORT jlong JNICALL Java_org_sipfoundry_telephony_PtProvider_JNI_1getProvider
  (JNIEnv *jenv, jclass, jstring jsName, jstring jsPasswd, jstring jsServer)
{
	char*		   szName ;
	char*		   szPasswd ;
	char*		   szServer ;
	PtProvider*	   pProvider ;
	CpCallManager* pCallMgr = NULL ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtProvider_JNI_1getProvider") ;

	// java -> native
    szName   = (jsName)   ? (char *)jenv->GetStringUTFChars(jsName, 0)   : NULL ;
    szPasswd = (jsPasswd) ? (char *)jenv->GetStringUTFChars(jsPasswd, 0) : NULL ;
	szServer = (jsServer) ? (char *)jenv->GetStringUTFChars(jsServer, 0) : NULL ;

	UtlString strServer ;
	OsSocket::getHostIp(&strServer) ;
	strServer += ":9000" ;

// osPrintf("JNI: getProvider(name=%s,passwd=%s,server=%s\n", szName, szPasswd, strServer.data()) ;
	Pinger* pinger = Pinger::getPingerTask();
	if (pinger)
	{				
		pCallMgr = (CpCallManager*) pinger->getCallMgrTask();
	}

	PtStatus status = PtProvider::getProvider(szName, szPasswd, strServer.data(), "", pCallMgr, pProvider) ;
	if (status != 0)
		API_FAILURE(status) ;

// osPrintf("JNI: getProvider return %d (provider=0x%08X)\n", status, pProvider) ;

	// clean up
    if (szName)   jenv->ReleaseStringUTFChars(jsName,   szName) ;
    if (szPasswd) jenv->ReleaseStringUTFChars(jsPasswd, szPasswd) ;
    if (szServer) jenv->ReleaseStringUTFChars(jsServer, szServer) ;

	JNI_END_METHOD() ;

	return (jlong) (unsigned int) pProvider ;
}


/*
 * Class:     org_sipfoundry_telephony_PtProvider
 * Method:    JNI_getTerminal
 * Signature: (JLjava/lang/String;)J
 */
extern "C"
JNIEXPORT jlong JNICALL Java_org_sipfoundry_telephony_PtProvider_JNI_1getTerminal
  (JNIEnv *jenv, jclass clazz, jlong lHandle, jstring jsName)
{
	PtProvider* pProvider = (PtProvider*) (unsigned int) lHandle ;
	char*		szName ;
	PtTerminal* pTerminal = NULL ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtProvider_JNI_1getTerminal") ;

	// java -> native
	szName   = (jsName)   ? (char *)jenv->GetStringUTFChars(jsName, 0) : NULL ;

	if (pProvider != NULL) {
		pTerminal = new PtTerminal() ;
		PtStatus status = pProvider->getTerminal(szName, *pTerminal) ;
		if (status != 0) {
			API_FAILURE(status) ;
			pTerminal = NULL ;
		}
	}

	// clean up
    if (szName)   jenv->ReleaseStringUTFChars(jsName, szName) ;

	JNI_END_METHOD() ;

	return (jlong) (unsigned int) pTerminal ;
}


/*
 * Class:     org_sipfoundry_telephony_PtProvider
 * Method:    JNI_getAddress
 * Signature: (JLjava/lang/String;)J
 */
extern "C"
JNIEXPORT jlong JNICALL Java_org_sipfoundry_telephony_PtProvider_JNI_1getAddress
  (JNIEnv *jenv, jclass clazz, jlong lHandle, jstring jsAddress)
{
	PtProvider* pProvider = (PtProvider*) (unsigned int) lHandle ;
	char*		szAddress ;
	PtAddress* pAddress = NULL ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtProvider_JNI_1getAddress") ;

	// java -> native
	szAddress   = (jsAddress)   ? (char *)jenv->GetStringUTFChars(jsAddress, 0) : NULL ;

	if (pProvider != NULL) {
		pAddress = new PtAddress() ;
		PtStatus status = pProvider->getAddress(szAddress, *pAddress) ;
		if (status != 0) {
			API_FAILURE(status) ;
            delete pAddress ;
			pAddress = NULL ;
		}
	}

	// clean up
    if (szAddress)   jenv->ReleaseStringUTFChars(jsAddress, szAddress) ;

	JNI_END_METHOD() ;

	return (jlong) (unsigned int) pAddress ;
}



/*
 * Class:     org_sipfoundry_telephony_PtProvider
 * Method:    JNI_getTerminals
 * Signature: (J)[J
 */
#define MAX_TERMINALS 8
extern "C"
JNIEXPORT jlongArray JNICALL Java_org_sipfoundry_telephony_PtProvider_JNI_1getTerminals
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	jlongArray terminals = NULL ;
	PtProvider* pProvider = (PtProvider*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtProvider_JNI_1getTerminals") ;

	if (pProvider != NULL) {
		PtTerminal native_terminals[MAX_TERMINALS] ;
		int		   nItems = 0 ;

		PtStatus status = pProvider->getTerminals(native_terminals, MAX_TERMINALS, nItems) ;

		if ((status != 0) || (nItems <= 0))
			API_FAILURE(status) ;

		// Convert to an array of longs
		jlong longs[MAX_TERMINALS] ;
		for (int i=0; i<nItems; i++) {
			longs[i] = (jlong) (unsigned int) new PtTerminal(native_terminals[i]) ;
		}

		// bulk copy into our array
		terminals = pEnv->NewLongArray(nItems) ;
		pEnv->SetLongArrayRegion(terminals, 0, nItems, longs) ;
	}


	JNI_END_METHOD() ;

	return terminals ;

}

/*
 * Class:     org_sipfoundry_telephony_PtProvider
 * Method:    JNI_getTerminals
 * Signature: (J)[J
 */
#define MAX_ADDRESSES 8
extern "C"
JNIEXPORT jlongArray JNICALL Java_org_sipfoundry_telephony_PtProvider_JNI_1getAddresses
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	jlongArray addresses = NULL ;
	PtProvider* pProvider = (PtProvider*) (unsigned int) lHandle ;
	
	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtProvider_JNI_1getAddresses") ;

	if (pProvider != NULL) {
		PtAddress native_addresses[MAX_ADDRESSES] ;
		int		   nItems = 0 ;

		PtStatus status = pProvider->getAddresses(native_addresses, MAX_ADDRESSES, nItems) ;

		if ((status != 0) || (nItems <= 0))
			API_FAILURE(status) ;

		// Convert to an array of longs
		jlong longs[MAX_ADDRESSES] ;
		for (int i=0; i<nItems; i++) {
			longs[i] = (jlong) (unsigned int) new PtAddress(native_addresses[i]) ;
		}

		// bulk copy into our array
		addresses = pEnv->NewLongArray(nItems) ;
		pEnv->SetLongArrayRegion(addresses, 0, nItems, longs) ;
	}


	JNI_END_METHOD() ;

	return addresses ;

}



/*
 * Class:     org_sipfoundry_telephony_PtProvider
 * Method:    JNI_createCall
 * Signature: (J)J
 */
extern "C"
JNIEXPORT jlong JNICALL Java_org_sipfoundry_telephony_PtProvider_JNI_1createCall
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	PtProvider* pProvider = (PtProvider*) (unsigned int) lHandle ;
	PtCall *pCall = new PtCall() ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtProvider_JNI_1createCall") ;

	PtStatus status = pProvider->createCall(*pCall) ;
	if (status != 0)
		API_FAILURE(status) ;

	JNI_END_METHOD() ;

	return (jlong) (unsigned int) pCall ;
}


/*
 * Class:     org_sipfoundry_telephony_PtProvider
 * Method:    JNI_finalize
 * Signature: (J)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_PtProvider_JNI_1finalize
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	lHandle = 0 ;
}

