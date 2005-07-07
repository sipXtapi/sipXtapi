// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/com_pingtel_xpressa_sys_user_PUserManager.cpp#2 $
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
#include "web/Webui.h"
#include <pinger/PingerInfo.h>
#include <pinger/Pinger.h>


extern "C" {
/*
 * Class:     org_sipfoundry_sipxphone_sys_user_PUserManager
 * Method:    JNI_addUser
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_org_sipfoundry_sipxphone_sys_user_PUserManager_JNI_1addUser
  (JNIEnv * pEnv, jobject, jstring jstrUser, jstring jstrPassword)
{
	UtlString username((const char*) pEnv->GetStringUTFChars(jstrUser, 0)) ;
	UtlString password((const char*) pEnv->GetStringUTFChars(jstrPassword, 0)) ;
	int i= 1 ;

	Pinger *pPinger = Pinger::getPingerTask();
	if (pPinger)
		i = pPinger->addUser(username, password) ;

	return i ;
}

/*
 * Class:     org_sipfoundry_sipxphone_sys_user_PUserManager
 * Method:    JNI_authenticateUser
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_org_sipfoundry_sipxphone_sys_user_PUserManager_JNI_1authenticateUser
  (JNIEnv * pEnv, jobject, jstring jstrUser, jstring jstrPassword)
{
	UtlString username((const char*) pEnv->GetStringUTFChars(jstrUser, 0)) ;
	UtlString password((const char*) pEnv->GetStringUTFChars(jstrPassword, 0)) ;
	int i = 1 ;

	Pinger *pPinger = Pinger::getPingerTask() ;
	if (pPinger)
		i = pPinger->authenticateUser(username, password) ;

	return i ;
}

/*
 * Class:     org_sipfoundry_sipxphone_sys_user_PUserManager
 * Method:    JNI_changeUserPassword
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_org_sipfoundry_sipxphone_sys_user_PUserManager_JNI_1changeUserPassword
  (JNIEnv * pEnv, jobject, jstring jstrUser, jstring jstrPassword)
{
	UtlString username((const char*) pEnv->GetStringUTFChars(jstrUser, 0)) ;
	UtlString password((const char*) pEnv->GetStringUTFChars(jstrPassword, 0)) ;
	int i= 1 ;

	i = Webui::getWebuiTask()->changeUserPassword(username, password) ;

	return i ;
}

/*
 * Class:     org_sipfoundry_sipxphone_sys_user_PUserManager
 * Method:    JNI_deleteUser
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_org_sipfoundry_sipxphone_sys_user_PUserManager_JNI_1deleteUser
  (JNIEnv * pEnv, jobject, jstring jstrUser, jstring jstrPassword)
{
	UtlString username((const char*) pEnv->GetStringUTFChars(jstrUser, 0)) ;
	UtlString password((const char*) pEnv->GetStringUTFChars(jstrPassword, 0)) ;
	int i= 1 ;

	i = Webui::getWebuiTask()->deleteUser(username, password) ;

	return i ;

}

// end extern "C"
}
