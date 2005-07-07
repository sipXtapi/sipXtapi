// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/VoicemailNotifyStateListener.cpp#3 $
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
#include <assert.h>
#include <jni.h>

// APPLICATION INCLUDES
#include <net/SipNotifyStateTask.h>
#include <pingerjni/VoicemailNotifyStateListener.h>
#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"
#include "os/OsConfigDb.h"
#include "pinger/Pinger.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */


#define INVOCATION_REBOOT_CLASS      "org/sipfoundry/sipxphone/sys/VoicemailNotifyStateListener"
#define INVOCATION_REBOOT_METHOD   "doReboot"
#define INVOCATION_REBOOT_SIGNATURE   "()V"

void VoicemailNotifyStateListener::doReboot()
{
   JNI_BEGIN_METHOD("VoicemailNotifyStateListener::doReboot") ;


   OsConfigDb* pConfig = Pinger::getPingerTask()->getConfigDb() ;  

   UtlString checkSync;
   pConfig->get("PHONESET_CHECK-SYNC", checkSync);
   if (checkSync.length() == 0)
      pConfig->get("PHONESET_CHECK_SYNC", checkSync);

   if ((checkSync.compareTo("enable" , UtlString::ignoreCase) == 0) 
      || (checkSync.compareTo("script" , UtlString::ignoreCase) == 0))
   {
      Pinger::getPingerTask()->restart(TRUE, 0, "configuration server initiated") ;
   }
   else
   {
      OsSysLog::add(FAC_AUTH, PRI_ERR, "check-sync not enabled") ;
   }
   
   JNI_END_METHOD() ;   
}


#define INVOCATION_BINARY_CLASS      "org/sipfoundry/sipxphone/sys/VoicemailNotifyStateListener"
#define INVOCATION_BINARY_METHOD   "doBinaryMessageWaiting"
#define INVOCATION_BINARY_SIGNATURE   "(Ljava/lang/String;Z)V"

void VoicemailNotifyStateListener::doBinaryMessageWaiting(const char* toUrl, 
        UtlBoolean newMessages)
{
   JNI_BEGIN_METHOD("VoicemailNotifyStateListener::doBinaryMessageWaiting") ;

   JavaVM* pVM ;
   JNIEnv* pEnv ;

   int iPriority = jniGetCurrentPriority() ;

   // Create or attach to a VM
   if (jniGetVMReference(&pVM, &pEnv)) {         

      jclass    cls ;
      jmethodID methodid ;

      cls = pEnv->FindClass(INVOCATION_BINARY_CLASS) ;
      if (cls != NULL) {
         methodid = pEnv->GetStaticMethodID(cls, INVOCATION_BINARY_METHOD, INVOCATION_BINARY_SIGNATURE) ;
         if (methodid != NULL) {
            jstring jsToURL ;

            // Massage arguments making sure we never use a null
            if (toUrl == NULL) {
               toUrl = "" ;
            }
            jsToURL = pEnv->NewStringUTF(toUrl) ;

            int iPriority = jniGetCurrentPriority() ;
            pEnv->CallStaticVoidMethod(cls, methodid, jsToURL, (jboolean) newMessages) ;
            jniResetPriority(iPriority) ;

            // If something goes wrong- yell about it.
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


/*
 * Class:     public org.sipfoundry.sipxphone.sys.VoicemailNotifyStateListener();
 * Method:    public static void doDetailMessageWaiting(java.lang.String, java.lang.String, boolean, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
 * Signature: (Ljava/lang/String;Ljava/lang/String;ZIIIIIIIIIIIIII)V
 */

#define INVOCATION_DETAIL_CLASS      "org/sipfoundry/sipxphone/sys/VoicemailNotifyStateListener"
#define INVOCATION_DETAIL_METHOD   "doDetailMessageWaiting"
#define INVOCATION_DETAIL_SIGNATURE   "(Ljava/lang/String;Ljava/lang/String;ZIIIIIIIIIIIIII)V"

void VoicemailNotifyStateListener::doDetailMessageWaiting(const char* toUrl,
        const char* messageMediaType,
        UtlBoolean absoluteValues,
        int totalNewMessages,
        int totalOldMessages,
        int totalUntouchedMessages,
        int urgentUntouchedMessages,
        int totalSkippedMessages,
        int urgentSkippedMessages,
        int totalFlaggedMessages,
        int urgentFlaggedMessages,
        int totalReadMessages,
        int urgentReadMessages,
        int totalAnsweredMessages,
        int urgentAnsweredMessages,
        int totalDeletedMessages,
        int urgentDeletedMessages)
{
   JNI_BEGIN_METHOD("VoicemailNotifyStateListener::doDetailMessageWaiting") ;

   JavaVM* pVM ;
   JNIEnv* pEnv ;

   int iPriority = jniGetCurrentPriority() ;

   // Create or attach to a VM
   if (jniGetVMReference(&pVM, &pEnv)) {         

      jclass    cls ;
      jmethodID methodid ;

      cls = pEnv->FindClass(INVOCATION_DETAIL_CLASS) ;
      if (cls != NULL) {
         methodid = pEnv->GetStaticMethodID(cls, INVOCATION_DETAIL_METHOD, INVOCATION_DETAIL_SIGNATURE) ;
         if (methodid != NULL) {
            jstring jsToURL ;
            jstring jsMessageMediaType ;

            // Massage arguments making sure we never use a null
            if (toUrl == NULL) {
               toUrl = "" ;
            } 
            jsToURL = pEnv->NewStringUTF(toUrl) ;

            if (messageMediaType == NULL)
               messageMediaType = "" ;
            jsMessageMediaType = pEnv->NewStringUTF(messageMediaType) ;

            int iPriority = jniGetCurrentPriority() ;
            pEnv->CallStaticVoidMethod(cls, methodid, jsToURL, 
                  jsMessageMediaType, absoluteValues, totalNewMessages,
                  totalOldMessages, totalUntouchedMessages, 
                  urgentUntouchedMessages, totalSkippedMessages, 
                  urgentSkippedMessages, totalFlaggedMessages, 
                  urgentFlaggedMessages, totalReadMessages, 
                  urgentReadMessages, totalAnsweredMessages, 
                  urgentAnsweredMessages, totalDeletedMessages, 
                  urgentDeletedMessages) ;

            jniResetPriority(iPriority) ;

            // If something goes wrong- yell about it.
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


/* ============================ CREATORS ================================== */

// Constructor
VoicemailNotifyStateListener::VoicemailNotifyStateListener()
{
}

// Copy constructor
VoicemailNotifyStateListener::VoicemailNotifyStateListener(const VoicemailNotifyStateListener& rVoicemailNotifyStateListener)
{
}

// Destructor
VoicemailNotifyStateListener::~VoicemailNotifyStateListener()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
VoicemailNotifyStateListener& 
VoicemailNotifyStateListener::operator=(const VoicemailNotifyStateListener& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */


/* ============================== DEBUG =================================== */

void JNI_TestRebootHandler()
{
   VoicemailNotifyStateListener::doReboot() ;
}


void JNI_TestBinaryHandler(int iValue)
{
   VoicemailNotifyStateListener::doBinaryMessageWaiting("randomURL", iValue) ;
}

void JNI_TestDetailHandler(int v1, int v2, int v3)
{
   VoicemailNotifyStateListener::doDetailMessageWaiting("randomURL",
        "Voicemail",
        v1,
        v2,
        v3,
        2,
        3,
        4,
        5,
        6,
        7,
        8,
        9,
        10,
        11,
        12,
        13) ;
}
