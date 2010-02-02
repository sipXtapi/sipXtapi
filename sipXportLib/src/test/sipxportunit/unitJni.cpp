//
// Copyright (C) 2010 SIPez LLC  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
// Author: Daniel Petrie
//         dpetrie AT SIPez DOT com
//////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <jni.h>
#include <os/OsSysLog.h>
#include <tapi/sipXtapi.h>
#include <SipxPortUnitTestEnvironment.h>
#include <com_sipez_unit_UnitTestActivity.h>

#include <unistd.h>
#include <fcntl.h>

// Static vars
JavaVM* sSipxJvmPtr = NULL;
jclass sCallbackClass = NULL;

extern "C"

// Native functions
jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
    sSipxJvmPtr = vm;

    return(JNI_VERSION_1_6);
}

void jniLogoutputMethod(const char* logMessage)
{
    if(sSipxJvmPtr && sCallbackClass)
    {
        JNIEnv* env = NULL;
        int version = JNI_VERSION_1_6;
        int jniRet = sSipxJvmPtr->GetEnv((void**)&env, version);

        jmethodID callBackMethodId =
            env->GetStaticMethodID(sCallbackClass, "outString", "(Ljava/lang/String;)V");

        if(callBackMethodId)
        {
            jstring myString = env->NewStringUTF(logMessage);

            env->CallStaticVoidMethod(sCallbackClass, callBackMethodId, myString);

            // Let go of the  local string
            env->DeleteLocalRef(myString);
        }
        else
        {
            env->ExceptionClear();
        }

    }
}

int runUnitTests()
{
    int result = 0;

    SipxPortUnitTestEnvironment::runTests();

    SipxPortUnitTestEnvironment::reportResults();

    result = SipxPortUnitTestEnvironment::getTestPointFailureCount();
    result += SipxPortUnitTestEnvironment::getTestAbortCount();

    return(result);
}

jint Java_com_sipez_unit_UnitTestActivity_runTests(JNIEnv* env, jclass thisClass)
{
    sCallbackClass = thisClass;

    SipxPortUnitTestEnvironment::setStringOutMethod(jniLogoutputMethod);

    runUnitTests();
}

