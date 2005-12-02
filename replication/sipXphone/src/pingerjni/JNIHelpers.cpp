// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/JNIHelpers.cpp#2 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

/*****************************************************************************
 * JNIHelpers - Provides simple helpers to attach to VMs
 *
 */

#include <stdio.h>
#include <assert.h>

#include "os/OsDefs.h"
#include "os/OsBSem.h"
#include "os/OsFS.h"
#include "os/OsMutex.h"
#include "os/OsLock.h"
#include "os/OsTask.h"
#include "os/OsStatus.h"
#include "os/OsSocket.h"
#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"
#include "ptapi/PtProvider.h"

/*
 * If defined, this enables support for starting the Java layer from the 
 * native layer on platforms other than the xpressa. 
 */
//#define NATIVE_FIRST

/*
 * The method signatures are slightly different between 1.x JNI and 2.x JNI 
 * headers.  This method switchs those header styles.  Once we move to a 2.x
 * JVM this can be removed.
 */
/* #ifndef __pingtel_on_posix__ */
#define USE_JAVA2_SIGNATURE
/* #endif */

#include <jni.h>

// Globals
static OsBSem         g_semJVMAttach(OsBSem::Q_PRIORITY, OsBSem::FULL) ; // only let one person get a VM reference at a time 
static JDK1_1InitArgs g_jvmArgs ;         // JVM Arguments
static JavaVM*        g_pVM = NULL ;      // Java Virtual Machine
static JNIEnv*        g_pEnv = NULL ;     // JVM Environment

#ifdef USING_JVM_13                   
//these were swiped from the java 1.3 jni.h file so we could work on 1.3
// maybe I should refer to the actual jni...
typedef struct JavaVMOption {
    char *optionString;
    void *extraInfo;
} JavaVMOption;

typedef struct JavaVMInitArgs {
    jint version;

    jint nOptions;
    JavaVMOption *options;
    jboolean ignoreUnrecognized;
} JavaVMInitArgs;
#endif


// Locals
static UtlBoolean jniCreateJVM(JavaVM** pVM, JNIEnv** pEnv);

// Externs
extern "C" void InitializeJVMSettings(JDK1_1InitArgs* pJNIArgs) ;
#ifdef USING_ASYNC_GC
extern "C" void javaInterruptsPendingSet(bool interruptsPending) ;
#endif

extern jint (JNICALL *GetJavaVM)
      (JNIEnv *env, JavaVM **vm);


/****************************************************************************
 * Call to start the java pinger application.
 */
extern "C" void JPingerStart()
{
#ifdef PJ31
   createAWTPropertiesFile() ;
#endif

   OsSysLog::add(FAC_JNI, PRI_NOTICE, "Starting Java Virtual Machine") ;
   if (!jniInitJVMReference()) 
   {
      LOOKUP_FAILURE() ;
   }

   if (g_pEnv != NULL)
   {
      jniStartPingerApp(g_pEnv) ;
   }
}


/****************************************************************************
 * Get a reference to org.sipfoundry.PingerApp and invoking the static method
 * "startPingerApp".
 */
UtlBoolean jniStartPingerApp(JNIEnv* pEnv)
{
   jclass    cls ;
   jmethodID methodid ;
   UtlBoolean bSuccess = false ;

#ifndef NATIVE_FIRST
   cls = pEnv->FindClass("org/sipfoundry/sipxphone/sys/startup/PingerApp") ;
#else
   cls = pEnv->FindClass("org/sipfoundry/sipxphone/testbed/TestbedFrame") ;
#endif
   if (cls != NULL) {
#ifndef NATIVE_FIRST
      methodid = pEnv->GetStaticMethodID(cls, "startPingerApp", "()V") ;
#else
      methodid = pEnv->GetStaticMethodID(cls, "main", "([Ljava/lang/String;)V") ;
#endif
      if (methodid != NULL) {

       int iPriority = jniGetCurrentPriority() ;
         pEnv->CallStaticVoidMethod(cls, methodid) ;
       jniResetPriority(iPriority) ;

       // If something goes wrong- yell about it.
         if (pEnv->ExceptionOccurred()) {
            pEnv->ExceptionDescribe() ;
            pEnv->ExceptionClear() ;
         } else {
         bSuccess = TRUE ;
       }
      } else {
         LOOKUP_FAILURE() ;
      }
   } else {
      LOOKUP_FAILURE() ;
   }

   return bSuccess ;
}


UtlBoolean jniInitJVMReference()
{
   UtlBoolean bInited = true ;

   if (g_pVM == NULL)
   {
      JavaVM* pVM = NULL ;
      JNIEnv* pEnv = NULL ;

      bInited = jniCreateJVM(&g_pVM, &g_pEnv) ;
      jniGetVMReference(&pVM, &pEnv) ;
   }

   return bInited ;
}


UtlBoolean jniCreateJVM(JavaVM** pVM, JNIEnv** pEnv)
{
   UtlBoolean bRC = false ;
   jsize nVMs = 0 ;
   int iRC = 0 ;
#ifdef NATIVE_FIRST
   static char classpath[2048];
#endif

   // Get the JVM Args
   JNI_GetDefaultJavaVMInitArgs(&g_jvmArgs) ;

#ifdef NATIVE_FIRST
   // 06/25/2003: Bob: According to Mike, this is not used, however, we may
   //    want or need to use it later.
   snprintf(classpath, 2048, "%s:.:env:env/resources.jar:env/ixpressa.jar:env/jtapi13.jar:env/pingtel.jar:env/jndi1.2.1.jar:env/ldap.jar", args.classpath) ;
   g_jvmArgs.classpath = classpath ;
   g_jvmArgs.enableClassGC = 0 ;
#endif

   // See if a JVM first exists (In IX, java starts first)
   if ((JNI_GetCreatedJavaVMs(pVM, 1, &nVMs) != 0) || (nVMs == 0)) 
   {   
      osPrintf("** JNIHelpers: Creating Java Virtual Machine (ver=%08X)...\n", g_jvmArgs.version) ;

#if defined(USE_JAVA2_SIGNATURE)
      iRC = JNI_CreateJavaVM(pVM, (void **) pEnv, &g_jvmArgs) ;
#else
      iRC = JNI_CreateJavaVM(pVM, pEnv, &g_jvmArgs) ;
#endif

      if (iRC == 0) 
      {
         osPrintf("** JNIHelpers: JVM Created\n") ;
         bRC = true ;
      }
      else 
      {
         *pVM = NULL ;
         osPrintf("** JNIHelpers: FAILED to create JVM, rc=%d\n", iRC) ;
         LOOKUP_FAILURE() ;
      }
   }

   return bRC ;
}

#if DEBUG_GET_VM
static int s_iJVMCount = 0 ;
#endif

/*****************************************************************************
 * get a reference to the one and only Java Virtual Machine
 */
UtlBoolean jniGetVMReference(JavaVM** pVM, JNIEnv** pEnv)
{   
   UtlBoolean  bRC = false ;

   // Guard against multiple threads attach/creating a JVM at the same time.
   g_semJVMAttach.acquire() ;    

   *pVM = g_pVM ;

   if ((pVM != NULL) && (*pVM != NULL))
   {
#if defined(USE_JAVA2_SIGNATURE)
      if ((*pVM)->AttachCurrentThread((void **) pEnv, (void **) &g_jvmArgs) == 0) 
#elif defined(USING_JVM_13)
      JavaVMInitArgs vmArgs;
      if ((*pVM)->AttachCurrentThread(pEnv, (void **) &vmArgs) == 0) 
#else
      if ((*pVM)->AttachCurrentThread(pEnv, (void **) &g_jvmArgs) == 0) 
#endif
      {
         bRC = true ;
      }
      else 
      {
         osPrintf("** JNIHelpers: Unable to attach to JVM\n") ;
         LOOKUP_FAILURE() ;
      }

#if DEBUG_GET_VM
      osPrintf("** JNIHelpers: JVM Attach (%d): ver=%d, pEnv=%08X, pVM=%08X, success=%d\n", 
            ++s_iJVMCount, g_jvmArgs.version, *pEnv, *pVM, bRC) ;
#endif

   }
     
   g_semJVMAttach.release() ;

   return bRC ;
}


/**
 * release the reference to our VM
 */
UtlBoolean jniReleaseVMReference(JavaVM* pVM)
{   
   g_semJVMAttach.acquire() ;

#if DEBUG_GET_VM
   osPrintf("** JNIHelpers: JVM Detach (%d): pEnv=%08X\n", --s_iJVMCount, pVM) ;
#endif

   pVM->DetachCurrentThread() ;
   g_semJVMAttach.release() ;

   return TRUE;
}



/**
 * what is the current priority of this task?
 */
int jniGetCurrentPriority()
{
   int iPriority = -1 ;

   OsTaskBase* pTask = OsTask::getCurrentTask() ;
   if (pTask != NULL) {
      pTask->getPriority(iPriority) ;   
   }
   return iPriority ;
}


/**
 * reset the priority of this task
 */
void jniResetPriority(int iPriority)
{
   int iCurrentPriority ;

   if (iPriority != -1) {
      OsTaskBase* pTask = OsTask::getCurrentTask() ;
      if (pTask != NULL) {
         pTask->getPriority(iCurrentPriority) ;
         
         // We will only reset if the priority is lower.
         if (iPriority < iCurrentPriority) {
            pTask->setPriority(iPriority) ;
         }
      }
   }
}



#define JNI_EXCEPTION_CLASS "org/sipfoundry/telephony/PtJNIException"
#define JNI_EXCEPTION_CONSTRUCTOR_METHOD "<init>"
#define JNI_EXCEPTION_CONSTRUCTOR_SIG "(I)V"

void jniThrowException(JNIEnv* pEnv, int iStatus)
{
   jclass clsException = pEnv->FindClass(JNI_EXCEPTION_CLASS) ;
   if (clsException != NULL) {
      jmethodID cid = pEnv->GetMethodID(clsException, JNI_EXCEPTION_CONSTRUCTOR_METHOD, JNI_EXCEPTION_CONSTRUCTOR_SIG) ;
      if (cid != NULL) {
         jobject objException = pEnv->NewObject(clsException, cid, iStatus) ;
         if (objException != NULL) {
                pEnv->Throw((jthrowable) objException) ;
            } 
        } else {
            LOOKUP_FAILURE() ;
        }
    } else 
        LOOKUP_FAILURE() ;
}


// Validate that the string is sane
const char* assertValidString(const char* szValidString, const UtlBoolean bAllowNull)
{
    const unsigned char *szTraverse ;

    if (szValidString != NULL)
    {
        for (   szTraverse = (unsigned char*) szValidString; 
                *szTraverse != 0;
                szTraverse++)
        {
            assert((*szTraverse >= 0x20) && (*szTraverse < 0x80)) ;            
        }        
    }
    else
    {
        assert(bAllowNull) ;
    }

    return szValidString ;
}
