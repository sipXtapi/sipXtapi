// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/com_pingtel_util_SysLog.cpp#2 $
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
#include "pinger/Pinger.h"
#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"

/*
 * Class:     org_sipfoundry_util_SysLog
 *
 * This has the implementations for the JNI methods
 * to log messages to SysLog from the Java layer.
 *
 * Method:    JNI_sysLogError
 */
extern "C"
JNIEXPORT void JNICALL Java_org_sipfoundry_util_SysLog_JNI_1sysLogError
  (JNIEnv *pEnv, jclass clazz, jstring jsErrorMessage, jint unused)
{
	JNI_BEGIN_METHOD("Java_org_sipfoundry_util_SysLog_JNI_1sysLogError") ;

	char* szErrorMessage = JSTRTOCSTR(pEnv, jsErrorMessage) ;
   OsSysLog::add(FAC_JNI, PRI_ERR,  "%s", szErrorMessage);	
	RELEASECSTR(pEnv, jsErrorMessage, szErrorMessage) ;
	
	JNI_END_METHOD() ;
}







