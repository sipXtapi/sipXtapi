// $Id: //depot/OPENDEV/sipXphone/include/pingerjni/JNIHelpers.h#2 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////


/**
 * JNIEventProxy is used to send events from the C/C++ world to Java.  This 
 * code will be replaced as TAO/JTAPI matures.
 */

#ifndef _JNIHelpers_h_
#define _JNIHelpers_h_

#include <jni.h>
#include "os/OsSysLog.h"
#include "pingerjni/debug_jni.h"

extern "C" void JPingerStart() ;

UtlBoolean jniStartPingerApp(JNIEnv* pEnv) ;
    //:Starts up the pinger application.

UtlBoolean jniInitJVMReference() ;
    //:Initializes the JVM reference (finds it or creates it)

UtlBoolean jniGetVMReference(JavaVM** pVM, JNIEnv** pEnv) ;
    //:Get a reference to the JVM.  If a JVM is already created, then a 
    //!reference to that VM is returned and the bCreated flag is set.

UtlBoolean jniReleaseVMReference(JavaVM* pVM) ;
    //:Release a reference to a JVM previous obtained by invoking 
    //!jniGetVMReference.

int  jniGetCurrentPriority() ;
    //:The the priority of the current thread.

void jniResetPriority(int iPriority) ;
    //:Reset the current thread's priority to the specified priority.

void jniThrowException(JNIEnv* env, int iStatus) ;
    //:Thrown an new exception of type [status]

const char* assertValidString(const char* szValidString, const UtlBoolean bAllowNull) ;
    //: Assert if the specified string is invalid

#if DEBUG_VALID_STRINGS
#  define VALIDCSTR(cstr) (assertValidString((cstr), FALSE))
#else
#  define VALIDCSTR(cstr) (cstr)
#endif

#define JSTRTOCSTR(pEnv, jstr) ((jstr) ? (char*) (pEnv)->GetStringUTFChars((jstr), 0) : NULL)
#define CSTRTOJSTR(pEnv, cstr) ((cstr) ? (pEnv)->NewStringUTF((VALIDCSTR(cstr))) : (pEnv)->NewStringUTF("")) 




#define RELEASECSTR(pEnv, jstr, cstr) if (cstr) ((pEnv)->ReleaseStringUTFChars((jstr), (cstr))) 

#define API_FAILURE(code) OsSysLog::add(FAC_JNI, PRI_ERR, "API failure #%d in %s, line %d", code, __FILE__, __LINE__)
#define LOOKUP_FAILURE() OsSysLog::add(FAC_JNI, PRI_ERR, "Look up failure in %s, line %d", __FILE__, __LINE__)

#endif  // _JNIHelpers_h
