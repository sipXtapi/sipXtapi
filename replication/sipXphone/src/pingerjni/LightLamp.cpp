// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/LightLamp.cpp#2 $
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


void JNI_LightButton(long lampBits)
{
	JavaVM* pVM ;
	JNIEnv* pEnv ;

	int iPriority = jniGetCurrentPriority() ; 

	JNI_BEGIN_METHOD("JNI_LightButton") ;

	// Create or attach to a VM
	if (jniGetVMReference(&pVM, &pEnv)) {

		jclass    cls ;
		jmethodID methodid ;

		cls = pEnv->FindClass("org/sipfoundry/sipxphone/testbed/SoftPhone") ;
		if (cls != NULL) {
			methodid = pEnv->GetStaticMethodID(cls, "LightButton", "(J)V") ;
			if (methodid != NULL) {
				pEnv->CallStaticVoidMethod(cls, methodid, lampBits) ;				
				if (pEnv->ExceptionOccurred()) {
					pEnv->ExceptionDescribe() ;
					pEnv->ExceptionClear() ;
				}
			} else {
//				osPrintf("Unable to get doDisplayStatus static method on org/sipfoundry/sipxphone/testbed/SoftPhone\r\n") ;
			}
		} else {
//			osPrintf("Unable to find org/sipfoundry/sipxphone/testbed/SoftPhone\r\n") ;
		}
//		osPrintf("JNI_DisplayStatus: About to release the VM\n") ;
		jniReleaseVMReference(pVM) ;			
	}
//	osPrintf("JNI_DisplayStatus: About to reset the priority\n") ;
	jniResetPriority(iPriority) ;

	JNI_END_METHOD() ;
}
