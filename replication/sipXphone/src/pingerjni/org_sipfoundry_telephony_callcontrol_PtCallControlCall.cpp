// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/com_pingtel_telephony_callcontrol_PtCallControlCall.cpp#2 $
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
#include "ptapi/PtTerminal.h"
#include "ptapi/PtSessionDesc.h"
#include "ptapi/PtConnection.h"
#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"
#include "cp/CallManager.h"


/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlCall
 * Method:    JNI_addParty
 * Signature: (JLjava/lang/String;)J
 */
extern "C"
JNIEXPORT jlong JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlCall_JNI_1addParty
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jstring jsSIPURL)
{
	PtCall*       pCall = (PtCall*) (unsigned int) lHandle ;
	char*         szSIPURL = (jsSIPURL) ? (char *) pEnv->GetStringUTFChars(jsSIPURL, 0) : NULL ;
	PtSessionDesc desc ;
	PtConnection* pConnection = NULL ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlCall_JNI_1addParty") ;

	if (pCall != NULL) 
    {
		pConnection = new PtConnection() ;
		PtStatus status = pCall->addParty(szSIPURL, &desc, *pConnection) ;
        if (status != 0)
        {
            API_FAILURE(status) ;
            jniThrowException(pEnv, status) ;
        }
	}

	if (szSIPURL) 
		pEnv->ReleaseStringUTFChars(jsSIPURL, szSIPURL) ;

	JNI_END_METHOD() ;

	return (jlong) (unsigned int) pConnection ;
}


/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlCall
 * Method:    JNI_drop
 * Signature: (J)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlCall_JNI_1drop
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	PtCall* pCall = (PtCall*) (unsigned int) lHandle ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlCall_JNI_1drop") ;

	if (pCall != NULL) {
		PtStatus status = pCall->drop() ;
		if (status != 0)
			API_FAILURE(status) ;
	}

	JNI_END_METHOD() ;
}


/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlCall
 * Method:    JNI_getCalledAddress
 * Signature: (J)J
 */
extern "C"
JNIEXPORT jlong JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlCall_JNI_1getCalledAddress
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	jlong address = 0 ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlCall_JNI_1getCalledAddress") ;

	PtCall* pCall = (PtCall*) (unsigned int) lHandle ;
	if (pCall != NULL) {
		PtAddress* pAddress = new PtAddress() ;

		PtStatus status = pCall->getCalledAddress(*pAddress) ;
		if (status != 0)
			API_FAILURE(status) ;

		address = (jlong) (unsigned int) pAddress ;

	}

	JNI_END_METHOD() ;

	return address ;
}


/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlCall
 * Method:    JNI_getCallingAddress
 * Signature: (J)J
 */
extern "C"
JNIEXPORT jlong JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlCall_JNI_1getCallingAddress
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	jlong address = 0 ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlCall_JNI_1getCallingAddress") ;

	PtCall* pCall = (PtCall*) (unsigned int) lHandle ;
	if (pCall != NULL) {
		PtAddress* pAddress = new PtAddress();

		PtStatus status = pCall->getCallingAddress(*pAddress) ;
		if (status != 0)
			API_FAILURE(status) ;

		address = (jlong) (unsigned int) pAddress ;
	}

	JNI_END_METHOD() ;


	return address ;
}


/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlCall
 * Method:    JNI_getCallingTerminal
 * Signature: (J)J
 */
extern "C"
JNIEXPORT jlong JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlCall_JNI_1getCallingTerminal
  (JNIEnv *pEnv, jclass clazz, jlong lHandle)
{
	jlong terminal = 0 ;

	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlCall_JNI_1getCallingTerminal") ;

	PtCall* pCall = (PtCall*) (unsigned int) lHandle ;
	if (pCall != NULL) {
		PtTerminal* pTerminal = new PtTerminal() ;

		PtStatus status = pCall->getCallingTerminal(*pTerminal) ;
		if (status != 0)
			API_FAILURE(status) ;

		terminal = (jlong) (unsigned int) pTerminal ;
	}

	JNI_END_METHOD() ;

	return terminal ;
}


/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlCall
 * Method:    JNI_transfer_toAddress
 * Signature: (JLjava/lang/String;)J
 */
extern "C"
JNIEXPORT jlong JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlCall_JNI_1transfer_1toAddress
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jstring jsAddress)
{
	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlCall_JNI_1transfer_1toAddress") ;

	jlong lRC = 0 ;

	PtCall* pCall = (PtCall*) (unsigned int) lHandle ;
	if (pCall != NULL) 
   {
      PtConnection connectionRC ;

		char* szAddress = (jsAddress) ? (char *)pEnv->GetStringUTFChars(jsAddress, 0) : NULL ;
		PtStatus status = pCall->transfer(szAddress, NULL, connectionRC) ;
      if (status != 0) 
      {
         API_FAILURE(status) ;
         jniThrowException(pEnv, status) ;
      } 
      else 
      {
			// Pick out the remote connection
			PtConnection* pNative_connection = new PtConnection(connectionRC) ;
			lRC = (jlong) (unsigned int) pNative_connection ;
		}

		if (szAddress)
			pEnv->ReleaseStringUTFChars(jsAddress, szAddress) ;
	}
	JNI_END_METHOD() ;

	return lRC ;
}


/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlCall
 * Method:    JNI_transfer_toCall
 * Signature: (JJ)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlCall_JNI_1transfer_1toCall
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jlong lCall)
{
	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlCall_JNI_1transfer_1toCall") ;

	PtCall* pCall = (PtCall*) (unsigned int) lHandle ;
	if (pCall != NULL) 
   {
		PtCall* pCallTransferTo = (PtCall*) (unsigned int) lCall ;

		PtStatus status = pCall->transfer(*pCallTransferTo) ;
      if (status != 0) 
      {
         API_FAILURE(status) ;
         jniThrowException(pEnv, status) ;
      }
   }
	JNI_END_METHOD() ;
}


/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlCall
 * Method:    JNI_consult
 * Signature: (JJLjava/lang/String;)[J
 */
extern "C"
JNIEXPORT jlongArray JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlCall_JNI_1consult
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jlong lHandleTC, jstring jsAddress)
{
	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlCall_JNI_1consult") ;

   PtCall*               pCall = (PtCall*) (unsigned int) lHandle ;
   PtTerminalConnection* pTerminalConnection = (PtTerminalConnection*) (unsigned int) lHandleTC ;
   char*                 szAddress = (jsAddress) ? (char *) pEnv->GetStringUTFChars(jsAddress, 0) : NULL ;
   PtSessionDesc         desc ;
   jlongArray            connections = NULL ;

   PtConnection          srcConnection ;
   PtConnection          tgtConnection ;

   PtStatus status = pCall->consult(*pTerminalConnection, szAddress, &desc, srcConnection, tgtConnection) ;
   if (status != 0) 
   {
      API_FAILURE(status) ;
      jniThrowException(pEnv, status) ;
   } 
   else 
   {
      jlong longs[2] ;
      PtConnection* pConnection ;

        // The Source connection is at position [0]
      pConnection = new PtConnection() ;
		*pConnection = srcConnection ;
		longs[0] = (jlong) (unsigned int) pConnection ;

        // The Target connection is at position [1]
		pConnection = new PtConnection() ;
		*pConnection = tgtConnection ;
		longs[1] = (jlong) (unsigned int) pConnection ;
     
      connections = pEnv->NewLongArray(2) ;
      pEnv->SetLongArrayRegion(connections, 0, 2, longs) ;
   }

   JNI_END_METHOD() ;

   return connections ;
}


/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlCall
 * Method:    forceCodecRenegotiation
 * Signature: (JI)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlCall_JNI_1forceCodecRenegotiation
  (JNIEnv* pEnv, jclass clazz, jlong lHandle, jint unused)
{
   JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlCall_forceCodecRenegotiation") ;

   PtCall* pCall = (PtCall*) (unsigned int) lHandle ;
   if (pCall != NULL)
   {
      pCall->forceCodecRenegotiation() ;
   }

   JNI_END_METHOD() ;
}


/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlCall
 * Method:    setCodecCPULimit
 * Signature: (JIZ)V
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlCall_JNI_1setCodecCPULimit
  (JNIEnv* pEnv, jclass clazz, jlong lHandle, jint iLimit, jboolean bAutoRenegotiate)
{
   JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlCall_setCodecCPULimit") ;

   PtCall* pCall = (PtCall*) (unsigned int) lHandle ;
   if (pCall != NULL)
   {
      pCall->setCodecCPULimit((const int) iLimit, 
            (const UtlBoolean) bAutoRenegotiate) ;
   }

   JNI_END_METHOD() ;
}


/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlCall
 * Method:    getCodecCPUCost
 * Signature: (JI)I
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlCall_JNI_1getCodecCPUCost
  (JNIEnv* pEnv, jclass clazz, jlong lHandle, jint unused)
{   
   JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlCall_JNI_1getCodecCPUCost") ;

   int     cost = -1;   
   PtCall* pCall = (PtCall*) (unsigned int) lHandle ;
   if (pCall != NULL)
   {      
      pCall->getCodecCPUCost(cost) ;
   }

   JNI_END_METHOD() ;

   return (jint) cost ;
}

/*
 * Class:     org_sipfoundry_telephony_callcontrol_PtCallControlCall
 * Method:    getCodecCPULimit
 * Signature: (JI)I
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_telephony_callcontrol_PtCallControlCall_JNI_1getCodecCPULimit
  (JNIEnv* pEnv, jclass clazz, jlong lHandle, jint unused)
{   
   JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_callcontrol_PtCallControlCall_JNI_1getCodecCPULimit") ;

   int     cost = -1;   
   PtCall* pCall = (PtCall*) (unsigned int) lHandle ;
   if (pCall != NULL)
   {      
      pCall->getCodecCPULimit(cost) ;
   }

   JNI_END_METHOD() ;

   return (jint) cost ;
}
