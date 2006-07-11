// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/Reboot.cpp#2 $
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
#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"


void JNI_SoftPhoneReboot(long ms)
{
	JavaVM* pVM ;
	JNIEnv* pEnv ;

	int iPriority = jniGetCurrentPriority() ; 

	JNI_BEGIN_METHOD("JNI_SoftPhoneReboot") ;

	// Create or attach to a VM
	if (jniGetVMReference(&pVM, &pEnv)) {

		jclass    cls ;
		jmethodID methodid ;

		cls = pEnv->FindClass("org/sipfoundry/sipxphone/testbed/SoftPhone") ;
		if (cls != NULL) {
			methodid = pEnv->GetStaticMethodID(cls, "SoftPhoneReboot", "(J)V") ;
			if (methodid != NULL) {
				pEnv->CallStaticVoidMethod(cls, methodid, ms) ;				
				if (pEnv->ExceptionOccurred()) {
					pEnv->ExceptionDescribe() ;
					pEnv->ExceptionClear() ;
				}
			} else {
				LOOKUP_FAILURE() ;
			}
		} else {
			LOOKUP_FAILURE() ;
		}
		jniReleaseVMReference(pVM) ;			
	}
	jniResetPriority(iPriority) ;

	JNI_END_METHOD() ;
}
