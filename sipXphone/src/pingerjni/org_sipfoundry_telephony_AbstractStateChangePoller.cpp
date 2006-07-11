// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/com_pingtel_telephony_AbstractStateChangePoller.cpp#2 $
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
#include "net/SipUserAgent.h"
#include "pingerjni/debug_jni.h"


/*
 * Class:     org_sipfoundry_telephony_
 * Method:    JNI_getSipStateTransactionTimeout
 * Signature: ()I
 */
extern "C"
JNIEXPORT jint JNICALL Java_org_sipfoundry_telephony_AbstractStateChangePoller_JNI_1getSipStateTransactionTimeout
  (JNIEnv *pEnv, jclass clazz)
{
	JNI_BEGIN_METHOD("Java_org_sipfoundry_telephony_AbstractStateChangePoller_JNI_1getSipStateTransactionTimeout") ;

	SipUserAgent* pUserAgent = Pinger::getPingerTask()->getSipUserAgent() ;
     
	jint iTimeout = (jint) pUserAgent->getSipStateTransactionTimeout() ;

	JNI_END_METHOD() ;

	return iTimeout ;
}







