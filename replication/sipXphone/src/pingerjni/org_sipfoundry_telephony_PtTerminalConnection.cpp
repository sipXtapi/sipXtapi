// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/com_pingtel_telephony_PtTerminalConnection.cpp#2 $
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
#include "ptapi/PtTerminalConnection.h"
#include "ptapi/PtTerminal.h"
#include "ptapi/PtConnection.h"
#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"


/*
 * Class:     org_sipfoundry_telephony_PtTerminalConnection
 * Method:    JNI_getState
 * Signature: (J)I
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_telephony_PtTerminalConnection_JNI_1getState
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	PtTerminalConnection* pConnection = (PtTerminalConnection*) (unsigned int) lHandle ;
	jint iRC = PtConnection::IDLE ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtTerminalConnection_JNI_1getState") ;
	
	if (pConnection != NULL) {
		int iState = 0 ;
		PtStatus status = pConnection->getState(iState) ;
		if (status != 0) {			
			API_FAILURE(status) ;
		} else 
			iRC = iState ;
	}

	JNI_END_METHOD() ;

	return iRC ;
}



/*
 * Class:     org_sipfoundry_telephony_PtTerminalConnection
 * Method:    JNI_answer
 * Signature: (J)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_PtTerminalConnection_JNI_1answer
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	PtTerminalConnection* pConnection = (PtTerminalConnection*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtTerminalConnection_JNI_1answer") ;

	if (pConnection != NULL) {
		PtStatus status = pConnection->answer() ;
		if (status != 0)
			API_FAILURE(status) ;
	}
	JNI_END_METHOD() ;

}


/*
 * Class:     org_sipfoundry_telephony_PtTerminalConnection
 * Method:    JNI_startTone
 * Signature: (JIZZ)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_PtTerminalConnection_JNI_1startTone
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jint jiToneID, jboolean bLocal, jboolean bRemote)
{
	PtTerminalConnection* pConnection = (PtTerminalConnection*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtTerminalConnection_JNI_1startTone") ;


	if (pConnection != NULL) {
		PtStatus status = pConnection->startTone((int) jiToneID, bLocal, bRemote, NULL) ;
		if (status != 0)
			API_FAILURE(status) ;
	}
	JNI_END_METHOD() ;

}


/*
 * Class:     org_sipfoundry_telephony_PtTerminalConnection
 * Method:    JNI_stopTone
 * Signature: (J)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_PtTerminalConnection_JNI_1stopTone
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	PtTerminalConnection* pConnection = (PtTerminalConnection*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtTerminalConnection_JNI_1stopTone") ;

	if (pConnection != NULL) {
		PtStatus status = pConnection->stopTone() ;
		if (status != 0)
			API_FAILURE(status) ;
	}
	JNI_END_METHOD() ;

}



/*
 * Class:     org_sipfoundry_telephony_PtTerminalConnection
 * Method:    JNI_playFile
 * Signature: (JLjava/lang/String;ZZZ)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_PtTerminalConnection_JNI_1playFile
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jstring jsFile, jboolean bRepeat, jboolean bLocal, jboolean bRemote)
{
	PtTerminalConnection* pConnection = (PtTerminalConnection*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtTerminalConnection_JNI_1playFile") ;


	if (pConnection != NULL) {
		char* szFile = (jsFile) ? (char *)pEnv->GetStringUTFChars(jsFile, 0) : NULL ;
		
        pConnection->stopPlay(true) ;
		PtStatus status = pConnection->playFile(szFile, bRepeat, bLocal, bRemote) ;
		if (status != 0)
			API_FAILURE(status) ;
		
		if (szFile) 
			pEnv->ReleaseStringUTFChars(jsFile, szFile) ;
	}

	JNI_END_METHOD() ;

}


/*
 * Class:     org_sipfoundry_telephony_PtTerminalConnection
 * Method:    JNI_stopPlay
 * Signature: (JZ)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_PtTerminalConnection_JNI_1stopPlay
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jboolean bCloseFile)
{
	PtTerminalConnection* pConnection = (PtTerminalConnection*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtTerminalConnection_JNI_1stopPlay") ;

	if (pConnection != NULL) {
		PtStatus status = pConnection->stopPlay(bCloseFile) ;
		if (status != 0)
			API_FAILURE(status) ;
	}

	JNI_END_METHOD() ;

}


/*
 * Class:     org_sipfoundry_telephony_PtTerminalConnection
 * Method:    JNI_getConnection
 * Signature: (J)J
 */
extern "C"
JNIEXPORT jlong JNICALL Java_org_sipfoundry_telephony_PtTerminalConnection_JNI_1getConnection
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	jlong lConnection = 0 ;

	PtTerminalConnection* pTerminalConnection = (PtTerminalConnection*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtTerminalConnection_JNI_1getConnection") ;

	if (pTerminalConnection != NULL) {
		PtConnection* connection = new PtConnection() ;

		PtStatus status = pTerminalConnection->getConnection(*connection) ;
		if (status != 0)
			API_FAILURE(status) ;

		lConnection = (jlong) (unsigned int) connection ;
	}

	JNI_END_METHOD() ;

	return lConnection ;
}


/*
 * Class:     org_sipfoundry_telephony_PtTerminalConnection
 * Method:    JNI_getTerminal
 * Signature: (J)J
 */
extern "C"
JNIEXPORT jlong JNICALL Java_org_sipfoundry_telephony_PtTerminalConnection_JNI_1getTerminal
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	jlong lTerminal = 0 ;

	PtTerminalConnection* pTerminalConnection = (PtTerminalConnection*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtTerminalConnection_JNI_1getTerminal") ;

	if (pTerminalConnection != NULL) {
		PtTerminal *pTerminal = NULL ;

		pTerminal = new PtTerminal() ;
		PtStatus status = pTerminalConnection->getTerminal(*pTerminal) ;
		if (status != 0)
			API_FAILURE(status) ;

		lTerminal = (jlong) (unsigned int) pTerminal ;
	}

	JNI_END_METHOD() ;

	return lTerminal ;
}


/*
 * Class:     org_sipfoundry_telephony_PtTerminalConnection
 * Method:    JNI_isLocal
 * Signature: (J)Z
 */
extern "C"
JNIEXPORT jboolean JNICALL Java_org_sipfoundry_telephony_PtTerminalConnection_JNI_1isLocal
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	jboolean bLocal = false ;
	PtTerminalConnection* pConnection = (PtTerminalConnection*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_PtTerminalConnection_JNI_1isLocal") ;

	if (pConnection != NULL) {
		UtlBoolean bOsLocal ;
		pConnection->isLocal(bOsLocal) ;
		bLocal = (jboolean) bOsLocal ;
	}

	JNI_END_METHOD() ;

	return bLocal ;
}


/*
 * Class:     org_sipfoundry_telephony_PtTerminalConnection
 * Method:    JNI_finalize
 * Signature: (J)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_PtTerminalConnection_JNI_1finalize
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	if (lHandle != 0) {
#if DEBUG_FINALIZE
		if (IsJNIDebuggingEnabled())
			osPrintf("JNI: freeing terminal connection 0x%08X\n", lHandle) ;
#endif
		delete (PtTerminalConnection*) (unsigned int) lHandle ;
	}
}
