// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/com_pingtel_xpressa_sys_util_NetUtilities.cpp#2 $
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

#include "os/OsUtil.h"
#include "pinger/PingerInfo.h"


/*
 * Class:     org_sipfoundry_sipxphone_sys_util_NetUtilities
 * Method:    JNI_checkIPAddress
 * Signature: (JLjava/lang/String;)Z
 */
extern "C"
JNIEXPORT jboolean JNICALL Java_org_sipfoundry_sipxphone_sys_util_NetUtilities_JNI_1checkIPAddress
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jstring jsIPAddress)
{
	jboolean bRC = false ;

	char* szIPAddress = (jsIPAddress) ? (char *) pEnv->GetStringUTFChars(jsIPAddress, 0) : NULL ;
	if (szIPAddress != NULL) {
		bRC = OsUtil::checkIpAddress(szIPAddress) ;
		pEnv->ReleaseStringUTFChars(jsIPAddress, szIPAddress) ;
	}

	return bRC ;
}


/*
 * Class:     org_sipfoundry_sipxphone_sys_util_NetUtilities
 * Method:    JNI_checkNetmask
 * Signature: (JLjava/lang/String;)Z
 */
extern "C"
JNIEXPORT jboolean JNICALL Java_org_sipfoundry_sipxphone_sys_util_NetUtilities_JNI_1checkNetmask
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jstring jsNetmask)
{
	jboolean bRC = false ;

	char* szNetAddress = (jsNetmask) ? (char *) pEnv->GetStringUTFChars(jsNetmask, 0) : NULL ;
	if (szNetAddress != NULL) {
		bRC = OsUtil::checkNetmask(szNetAddress) ;
		pEnv->ReleaseStringUTFChars(jsNetmask, szNetAddress) ;
	}

	return bRC ;
}


/*
 * Class:     org_sipfoundry_sipxphone_sys_util_NetUtilities
 * Method:    JNI_isSameNetwork
 * Signature: (JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z
 */
extern "C"
JNIEXPORT jboolean JNICALL Java_org_sipfoundry_sipxphone_sys_util_NetUtilities_JNI_1isSameNetwork
  (JNIEnv *pEnv, jclass clazz, jlong lHandle, jstring jsDestIP, jstring jsIP, jstring jsNetmask)
{
	jboolean bRC = false ;

	char* szDestIP = (jsDestIP) ? (char *) pEnv->GetStringUTFChars(jsDestIP, 0) : NULL ;
	char* szIP = (jsIP) ? (char *) pEnv->GetStringUTFChars(jsIP, 0) : NULL ;
	char* szNetmask = (jsNetmask) ? (char *) pEnv->GetStringUTFChars(jsNetmask, 0) : NULL ;

	if ((szDestIP != NULL) && (szIP != NULL) && (szNetmask != NULL)) {
		bRC = OsUtil::isSameNetwork(szDestIP, szIP, szNetmask) ;		
	}

	if (szDestIP != NULL)
		pEnv->ReleaseStringUTFChars(jsDestIP, szDestIP) ;
	if (szIP != NULL)
		pEnv->ReleaseStringUTFChars(jsIP, szIP) ;
	if (szNetmask != NULL)
		pEnv->ReleaseStringUTFChars(jsNetmask, szNetmask) ;

	return bRC ;
}
