// $Id: //depot/OPENDEV/sipXconfig/profilepublisher/sipagent-jni/src/sipendpoint/JNIHelpers.cpp#5 $
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
 *  Originated from sipXphone/src/pingerjni/JNIHelpers.cpp
 */

#include <stdio.h>
#include <assert.h>

#include "os/OsDefs.h"
#include "os/OsBSem.h"
#include "os/OsTask.h"
#include "sipendpoint/JNIHelpers.h"

#include <jni.h>

// Globals
static OsBSem         g_semJVMAttach(OsBSem::Q_PRIORITY, OsBSem::FULL) ; // only let one person get a VM reference at a time 
static JDK1_1InitArgs g_jvmArgs ;         // JVM Arguments
static JavaVM*        g_pVM = NULL ;      // Java Virtual Machine
static JNIEnv*        g_pEnv = NULL ;     // JVM Environment

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
      if ((*pVM)->AttachCurrentThread((void **)pEnv, (void *)&g_jvmArgs) == 0) 
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


UtlBoolean jniCreateJVM(JavaVM** pVM, JNIEnv** pEnv)
{
   UtlBoolean bRC = false ;
   jsize nVMs = 0 ;
   int iRC = 0 ;

   // Get the JVM Args
   JNI_GetDefaultJavaVMInitArgs(&g_jvmArgs) ;

   // See if a JVM first exists (In IX, java starts first)
   if ((JNI_GetCreatedJavaVMs(pVM, 1, &nVMs) != 0) || (nVMs == 0)) 
   {   
      osPrintf("** JNIHelpers: Creating Java Virtual Machine (ver=%08X)...\n", g_jvmArgs.version) ;

      iRC = JNI_CreateJavaVM(pVM, (void **) pEnv, &g_jvmArgs) ;

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

// Validate that the string is sane
const char* assertValidString(const char* szValidString, const UtlBoolean bAllowNull)
{
    const char *szTraverse ;

    if (szValidString != NULL)
    {
        for (   szTraverse = szValidString; 
                *szTraverse != '\0';
                szTraverse++)
        {
         // The compiler complained, with good reason, about the
         // comparison with 0x80, since all SIGNED chars are less than
         // 0x80.  Apparently, we are trying to make sure that every
         // character in the string is a printable, 7-bit, ASCII
         // character in the range 0x20..0x7f.
         // 
         // Since the deep question of whether char variables are signed
         // or unsigned is a great unsolved problem in computer science,
         // I have replaced the original test with the less obvious, yet
         // more portable, test requiring that
         // 
         //   one or both of the two bits in 0x60 must be set.
         // 
         // assert((*szTraverse >= 0x20) && (*szTraverse < 0x80));
            assert(0 != (*szTraverse & 0x60));
        }        
    }
    else
    {
        assert(bAllowNull) ;
    }

    return szValidString ;
}
