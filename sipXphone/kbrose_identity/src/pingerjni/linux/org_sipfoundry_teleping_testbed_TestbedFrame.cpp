// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/linux/com_pingtel_teleping_testbed_TestbedFrame.cpp#2 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <pinger/Pinger.h>
#include <pinger/PingerInfo.h>
#include <pingerjni/JNIHelpers.h>
#include <jni.h>
#include <os/iostream>
#include <assert.h>
#include <sys/stat.h>

// APPLICATION INCLUDES
#include "os/OsDefs.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Class:     org_sipfoundry_sipxphone_testbed_TestbedFrame
 * Method:    JNI_pingerStart
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_sipfoundry_sipxphone_testbed_TestbedFrame_JNI_1pingerStart
  (JNIEnv *jenv, jclass clazz)
{
    if (!jniInitJVMReference()) 
    {
        LOOKUP_FAILURE() ;
    }

    Pinger::getPingerTask();	
}
/*
 * Class:     org_sipfoundry_sipxphone_testbed_TestbedFrame
 * Method:    JNI_pingerStart
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_sipfoundry_sipxphone_testbed_TestbedFrame_JNI_1restoreMinimizedWindow
  (JNIEnv *jenv, jclass clazz)
{
}

/*
 * Class:     org_sipfoundry_sipxphone_sys_util_PingerInfo
 * Method:    JNI_signalStartupFinished
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_sipfoundry_sipxphone_testbed_TestbedFrame_JNI_1signalStartupFinished
  (JNIEnv *jenv, jclass clazz)
{
    // osPrintf("signalStartupFinished was called.\n");
}

/*
 * Class:     org_sipfoundry_sipxphone_testbed_TestbedFrame
 * Method:    JNI_shutdownHook
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_sipfoundry_sipxphone_testbed_TestbedFrame_JNI_1shutdownHook
  (JNIEnv *jenv, jclass clazz)
{
    // osPrintf("shutdownHook was called");
}

#ifdef __cplusplus
}
#endif
