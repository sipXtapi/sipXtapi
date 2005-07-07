// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/JXAPI.cpp#2 $
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
#include <jni.h>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"
#include "pingerjni/JXAPI.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS


/* ============================ FUNCTIONS ================================= */


/*****************************************************************************
 */
extern "C" void JXAPI_DisplayStatus(const char* szStatus)
{
   JavaVM* pVM ;   // Pointer to JVM if creating
   JNIEnv* pEnv ;  // Pointer to the JVM Environment / context
   bool    bUsingExternalEnv = false;  // Are we using an external pEnv?

   int iPriority = jniGetCurrentPriority() ; 

   JNI_BEGIN_METHOD("JXAPI_DisplayStatus") ;

    // Create or attach to a VM
    if (bUsingExternalEnv || jniGetVMReference(&pVM, &pEnv)) 
    {
      jclass    cls ;
      jmethodID methodid ;

      cls = pEnv->FindClass("org/sipfoundry/sipxphone/sys/Shell") ;
      if (cls != NULL) 
        {
         jobject jobjString = pEnv->NewStringUTF(szStatus) ;
         methodid = pEnv->GetStaticMethodID(cls, "doDisplayStatus", "(Ljava/lang/String;)V") ;
         if (methodid != NULL) 
            {
            pEnv->CallStaticVoidMethod(cls, methodid, jobjString) ;            
            if (pEnv->ExceptionOccurred()) 
                {
               pEnv->ExceptionDescribe() ;
               pEnv->ExceptionClear() ;
            }
         } else {
            LOOKUP_FAILURE() ;
         }
      } else {
         LOOKUP_FAILURE() ;
      }

        if (!bUsingExternalEnv)
          jniReleaseVMReference(pVM) ;         
   }

    
   jniResetPriority(iPriority) ;

   JNI_END_METHOD() ;
}

/*****************************************************************************
 */
extern "C" void JXAPI_ClearStatus()
{
   JavaVM* pVM ;   // Pointer to JVM if creating
   JNIEnv* pEnv ;  // Pointer to the JVM Environment / context
   bool    bUsingExternalEnv = false;  // Are we using an external pEnv?

   int iPriority = jniGetCurrentPriority() ; 

   JNI_BEGIN_METHOD("JXAPI_ClearStatus") ;

   // Create or attach to a VM
   if (bUsingExternalEnv || jniGetVMReference(&pVM, &pEnv)) 
   {
      jclass    cls ;
      jmethodID methodid ;

      cls = pEnv->FindClass("org/sipfoundry/sipxphone/sys/Shell") ;
      if (cls != NULL) {
         methodid = pEnv->GetStaticMethodID(cls, "doClearStatus", "()V") ;
         if (methodid != NULL) {            
            pEnv->CallStaticVoidMethod(cls, methodid) ;            
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

        if (!bUsingExternalEnv)
          jniReleaseVMReference(pVM) ;         
   }
   jniResetPriority(iPriority) ;

   JNI_END_METHOD() ;
}


/*****************************************************************************
 */
extern "C" void JXAPI_doGarbageCollect()
{
}

/*****************************************************************************
 */
extern "C" void JXAPI_onConfigurationChanged()
{
   JavaVM* pVM ;   // Pointer to JVM if creating
   JNIEnv* pEnv ;  // Pointer to the JVM Environment / context
   bool    bUsingExternalEnv = false;  // Are we using an external pEnv?


   int iPriority = jniGetCurrentPriority() ; 

   JNI_BEGIN_METHOD("JXAPI_onConfigurationChanged") ;

   // Create or attach to a VM
   if (bUsingExternalEnv || jniGetVMReference(&pVM, &pEnv)) 
   {
      jclass    cls ;
      jmethodID methodid ;

      cls = pEnv->FindClass("org/sipfoundry/sipxphone/sys/Shell") ;
      if (cls != NULL) {
         methodid = pEnv->GetStaticMethodID(cls, "onConfigurationChanged", "()V") ;
         if (methodid != NULL) {            
            pEnv->CallStaticVoidMethod(cls, methodid) ;            
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

      if (!bUsingExternalEnv)
         jniReleaseVMReference(pVM) ;         
   }
   jniResetPriority(iPriority) ;

   JNI_END_METHOD() ;
}



/*****************************************************************************
 */
extern "C" void JXAPI_SimpleDial(const char* szSIPURL, char* szCallID, int nCallID)
{
   JavaVM* pVM ;   // Pointer to JVM if creating
   JNIEnv* pEnv ;  // Pointer to the JVM Environment / context
   bool    bUsingExternalEnv = false;  // Are we using an external pEnv?


   int iPriority = jniGetCurrentPriority() ;

   JNI_BEGIN_METHOD("JXAPI_SimpleDial") ;

   // Create or attach to a VM
   if (bUsingExternalEnv || jniGetVMReference(&pVM, &pEnv)) 
   {
      jclass    cls ;
      jmethodID methodid ;

      cls = pEnv->FindClass("org/sipfoundry/sipxphone/sys/Shell") ;
      if (cls != NULL) {
         jobject jobjString = pEnv->NewStringUTF(szSIPURL) ;
         methodid = pEnv->GetStaticMethodID(cls, "simpleDial", "(Ljava/lang/String;)Ljava/lang/String;") ;
         if (methodid != NULL) {            
            jobject jobjCallID = pEnv->CallStaticObjectMethod(cls, methodid, jobjString) ;            
            if (jobjCallID != NULL) {
               char* szID = (jobjCallID) ? (char *)pEnv->GetStringUTFChars((jstring) jobjCallID, 0) : NULL ;
               memset(szCallID,0, nCallID) ;
               strncpy(szCallID, szID, nCallID) ;
               if (szID) pEnv->ReleaseStringUTFChars((jstring)   jobjCallID, szID) ;
            }

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

      if (!bUsingExternalEnv)
         jniReleaseVMReference(pVM) ;               
   }
   jniResetPriority(iPriority) ;

   JNI_END_METHOD() ;
}


/*****************************************************************************
 */
extern "C" void JXAPI_Transfer(const char* szSIPURL, const char* szCallID)
{
   JavaVM* pVM ;   // Pointer to JVM if creating
   JNIEnv* pEnv ;  // Pointer to the JVM Environment / context
   bool    bUsingExternalEnv = false;  // Are we using an external pEnv?


   int iPriority = jniGetCurrentPriority() ;

   JNI_BEGIN_METHOD("JXAPI_Transfer") ;

   // Create or attach to a VM
   if (bUsingExternalEnv || jniGetVMReference(&pVM, &pEnv)) 
   {
      jclass    cls ;
      jmethodID methodid ;

      cls = pEnv->FindClass("org/sipfoundry/sipxphone/sys/Shell") ;
      if (cls != NULL) {

         jobject jsURL    = (szSIPURL != NULL) ? pEnv->NewStringUTF(szSIPURL) : NULL ;
         jobject jsCallID = (szCallID != NULL) ? pEnv->NewStringUTF(szCallID) : NULL ;
         
         methodid = pEnv->GetStaticMethodID(cls, "simpleTransfer", "(Ljava/lang/String;Ljava/lang/String;)V") ;
         if (methodid != NULL) {            
            pEnv->CallStaticVoidMethod(cls, methodid, jsURL, jsCallID) ;
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
      if (!bUsingExternalEnv)
         jniReleaseVMReference(pVM) ;               
   }
   jniResetPriority(iPriority) ;

   JNI_END_METHOD() ;
}


/*****************************************************************************
 */
extern "C" void JXAPI_DropCall(const char* szCallId)
{
   JavaVM* pVM ;   // Pointer to JVM if creating
   JNIEnv* pEnv ;  // Pointer to the JVM Environment / context
   bool    bUsingExternalEnv = false;  // Are we using an external pEnv?


   int iPriority = jniGetCurrentPriority() ;

   JNI_BEGIN_METHOD("JXAPI_DropCall") ;

   // Create or attach to a VM
   if (bUsingExternalEnv || jniGetVMReference(&pVM, &pEnv)) 
   {
      jclass    cls ;
      jmethodID methodid ;

      cls = pEnv->FindClass("org/sipfoundry/sipxphone/sys/Shell") ;
      if (cls != NULL) {
         jobject jobjString = pEnv->NewStringUTF(szCallId) ;
         methodid = pEnv->GetStaticMethodID(cls, "dropCallByID", "(Ljava/lang/String;)V") ;
         if (methodid != NULL) {            
            pEnv->CallStaticVoidMethod(cls, methodid, jobjString) ;
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
      if (!bUsingExternalEnv)
         jniReleaseVMReference(pVM) ;               
   }
   jniResetPriority(iPriority) ;

   JNI_END_METHOD() ;
}



/*****************************************************************************
 */
extern "C" void JXAPI_Answer(const char* szCallId)
{
   JavaVM* pVM ;   // Pointer to JVM if creating
   JNIEnv* pEnv ;  // Pointer to the JVM Environment / context
   bool    bUsingExternalEnv = false;  // Are we using an external pEnv?


   int iPriority = jniGetCurrentPriority() ;

   JNI_BEGIN_METHOD("JXAPI_Answer") ;

   // Create or attach to a VM
   if (bUsingExternalEnv || jniGetVMReference(&pVM, &pEnv)) 
   {
      jclass    cls ;
      jmethodID methodid ;

      cls = pEnv->FindClass("org/sipfoundry/sipxphone/sys/Shell") ;
      if (cls != NULL) {
         jobject jobjString = pEnv->NewStringUTF(szCallId) ;
         methodid = pEnv->GetStaticMethodID(cls, "answer", "(Ljava/lang/String;)V") ;
         if (methodid != NULL) {            
            pEnv->CallStaticVoidMethod(cls, methodid, jobjString) ;
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
      if (!bUsingExternalEnv)
         jniReleaseVMReference(pVM) ;               
   }
   jniResetPriority(iPriority) ;

   JNI_END_METHOD() ;
}


/*****************************************************************************
 */
extern "C" void JXAPI_Hold(const char* szCallId, bool bEnable)
{
   JavaVM* pVM ;   // Pointer to JVM if creating
   JNIEnv* pEnv ;  // Pointer to the JVM Environment / context
   bool    bUsingExternalEnv = false;  // Are we using an external pEnv?


   int iPriority = jniGetCurrentPriority() ;

   JNI_BEGIN_METHOD("JXAPI_Hold") ;

   // Create or attach to a VM
   if (bUsingExternalEnv || jniGetVMReference(&pVM, &pEnv)) 
   {
      jclass    cls ;
      jmethodID methodid ;

      cls = pEnv->FindClass("org/sipfoundry/sipxphone/sys/Shell") ;
      if (cls != NULL) {
         jobject jobjString = pEnv->NewStringUTF(szCallId) ;
         jboolean jboolEnable = bEnable;
         methodid = pEnv->GetStaticMethodID(cls, "hold", "(Ljava/lang/String;Z)V") ;
         if (methodid != NULL) {            
            pEnv->CallStaticVoidMethod(cls, methodid, jobjString, jboolEnable) ;
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
      if (!bUsingExternalEnv)
         jniReleaseVMReference(pVM) ;               
   }
   jniResetPriority(iPriority) ;

   JNI_END_METHOD() ;
}


/*****************************************************************************
 */
extern "C" void JXAPI_RemoveParty(const char* szSIPUrl, const char* szCallId)
{
   JavaVM* pVM ;   // Pointer to JVM if creating
   JNIEnv* pEnv ;  // Pointer to the JVM Environment / context
   bool    bUsingExternalEnv = false;  // Are we using an external pEnv?


   int iPriority = jniGetCurrentPriority() ;

   JNI_BEGIN_METHOD("JXAPI_RemoveParty") ;

   // Create or attach to a VM
   if (bUsingExternalEnv || jniGetVMReference(&pVM, &pEnv)) 
   {
      jclass    cls ;
      jmethodID methodid ;

      cls = pEnv->FindClass("org/sipfoundry/sipxphone/sys/Shell") ;
      if (cls != NULL) {
         jobject jobjStrCallId = pEnv->NewStringUTF(szCallId) ;
         jobject jobjStrSIPUrl = pEnv->NewStringUTF(szSIPUrl) ;
         methodid = pEnv->GetStaticMethodID(cls, "removeParty", "(Ljava/lang/String;Ljava/lang/String;)V") ;
         if (methodid != NULL) {            
            pEnv->CallStaticVoidMethod(cls, methodid, jobjStrCallId, jobjStrSIPUrl) ;
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
      if (!bUsingExternalEnv)
         jniReleaseVMReference(pVM) ;               
   }
   jniResetPriority(iPriority) ;

   JNI_END_METHOD() ;
}


/*****************************************************************************
 */
extern "C" void JXAPI_AddParty(const char* szSIPUrl ,  const char* szCallId)
{
   JavaVM* pVM ;   // Pointer to JVM if creating
   JNIEnv* pEnv ;  // Pointer to the JVM Environment / context
   bool    bUsingExternalEnv = false;  // Are we using an external pEnv?


   int iPriority = jniGetCurrentPriority() ;

   JNI_BEGIN_METHOD("JXAPI_AddParty") ;

   // Create or attach to a VM
   if (bUsingExternalEnv || jniGetVMReference(&pVM, &pEnv)) 
   {
      jclass    cls ;
      jmethodID methodid ;

      cls = pEnv->FindClass("org/sipfoundry/sipxphone/sys/Shell") ;
      if (cls != NULL) {
         jobject jobjStrCallId = pEnv->NewStringUTF(szCallId) ;
         jobject jobjStrSIPUrl = pEnv->NewStringUTF(szSIPUrl) ;
         methodid = pEnv->GetStaticMethodID(cls, "addParty", "(Ljava/lang/String;Ljava/lang/String;)V") ;
         if (methodid != NULL) {            
            pEnv->CallStaticVoidMethod(cls, methodid, jobjStrCallId, jobjStrSIPUrl) ;
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
      if (!bUsingExternalEnv)
         jniReleaseVMReference(pVM) ;               
   }
   jniResetPriority(iPriority) ;

   JNI_END_METHOD() ;
}


/*****************************************************************************
 */
extern "C" int JXAPI_MessageBox(int         iType, 
                                const char* szTitle, 
                                const char* szText, 
                                UtlBoolean   bBlocking)
{
   JavaVM* pVM ;   // Pointer to JVM if creating
   JNIEnv* pEnv ;  // Pointer to the JVM Environment / context
   int     iRC = -1 ;  // Return code: message box status
   bool    bUsingExternalEnv = false;  // Are we using an external pEnv?


   int iPriority = jniGetCurrentPriority() ;

   JNI_BEGIN_METHOD("JXAPI_MessageBox") ;

   // Create or attach to a VM
   if (bUsingExternalEnv || jniGetVMReference(&pVM, &pEnv)) 
   {
      jclass    cls ;
      jmethodID methodid ;

      cls = pEnv->FindClass("org/sipfoundry/sipxphone/sys/Shell") ;
      if (cls != NULL) {
         jobject jobjTitle = pEnv->NewStringUTF(szTitle) ;
            jobject jobjText = pEnv->NewStringUTF(szText) ;

            methodid = pEnv->GetStaticMethodID(cls, "doDisplayMessageBox", "(ILjava/lang/String;Ljava/lang/String;Z)I") ;
         if (methodid != NULL) {

            iRC = pEnv->CallStaticIntMethod(cls, methodid, iType, jobjTitle, jobjText, bBlocking) ;
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
      if (!bUsingExternalEnv)
         jniReleaseVMReference(pVM) ;               
   }
   jniResetPriority(iPriority) ;

   JNI_END_METHOD() ;

    return iRC ;
}

extern "C" int JXAPI_AuthorizeProfiles(int passwordAttempts, char* username, 
                                       int maxUsernameLen, char* password,
                                       int maxPasswordLen)
{
   JavaVM* pVM ;   // Pointer to JVM if creating
   JNIEnv* pEnv ;  // Pointer to the JVM Environment / context
    int     iRC = -1 ;  // Return code: message box status
    bool    bUsingExternalEnv = false;  // Are we using an external pEnv?

   int iPriority = jniGetCurrentPriority() ;

   JNI_BEGIN_METHOD("JXAPI_LoginPrompt") ;

   // Create or attach to a VM
   if (bUsingExternalEnv || jniGetVMReference(&pVM, &pEnv)) 
    {
      jclass    cls;
      jmethodID methodid;

      cls = pEnv->FindClass("org/sipfoundry/sipxphone/sys/Shell");
      if (cls != NULL) 
        {
            methodid = pEnv->GetStaticMethodID(cls, "doDisplayAuthorizeProfiles", 
                "(I[Ljava/lang/String;)I");
         if (methodid != NULL) 
            {
                // Fill in default username and password
                jclass classString = pEnv->FindClass("java/lang/String") ;
                jobjectArray loginData = pEnv->NewObjectArray(2, classString, NULL);
                pEnv->SetObjectArrayElement(loginData, 0, CSTRTOJSTR(pEnv, username));
                pEnv->SetObjectArrayElement(loginData, 1, CSTRTOJSTR(pEnv, password));

            iRC = pEnv->CallStaticIntMethod(cls, methodid, passwordAttempts, loginData);
            if (pEnv->ExceptionOccurred()) 
                {
               pEnv->ExceptionDescribe();
               pEnv->ExceptionClear();
                    iRC = -1;
            }
                else if (iRC == 0)
                {
                    jstring jresponse;
                    char *response;

                    jresponse = (jstring)pEnv->GetObjectArrayElement(loginData, 0);
                    response = JSTRTOCSTR(pEnv, jresponse);
                    strncpy(username, response, maxUsernameLen);
                    RELEASECSTR(pEnv, jresponse, response);

                    jresponse = (jstring)pEnv->GetObjectArrayElement(loginData, 1);
                    response = JSTRTOCSTR(pEnv, jresponse);
                    strncpy(password, response, maxPasswordLen);
                    RELEASECSTR(pEnv, jresponse, response);
                }
         } 
            else 
            {
            LOOKUP_FAILURE();
         }
      } 
        else 
        {
         LOOKUP_FAILURE();
      }

        if (!bUsingExternalEnv)
        {
            jniReleaseVMReference(pVM);
        }
   }
   jniResetPriority(iPriority);
    
   JNI_END_METHOD();
    
    return iRC;
}


/*****************************************************************************
 */
extern "C" void JXAPI_setUploadingFile(UtlBoolean bUploadingFile)
{
   JavaVM* pVM ;   // Pointer to JVM if creating
   JNIEnv* pEnv ;  // Pointer to the JVM Environment / context
   bool    bUsingExternalEnv = false;  // Are we using an external pEnv?


   const char *className      = "org/sipfoundry/sipxphone/sys/Shell";
   const char *methodName   = "setUploadingFile";

   int iPriority = jniGetCurrentPriority() ; 

   JNI_BEGIN_METHOD("JXAPI_setUploadingFile") ;

   // Create or attach to a VM
   if (bUsingExternalEnv || jniGetVMReference(&pVM, &pEnv)) 
   {
      jclass    cls ;
      jmethodID methodid ;

      cls = pEnv->FindClass(className) ;
      if (cls != NULL) {
         methodid = pEnv->GetStaticMethodID(cls, methodName, "(Z)V") ;
         if (methodid != NULL) {            
            pEnv->CallStaticVoidMethod(cls, methodid,bUploadingFile) ;            
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
      if (!bUsingExternalEnv)
         jniReleaseVMReference(pVM) ;               
   }
   jniResetPriority(iPriority) ;

   JNI_END_METHOD() ;
}


extern "C" UtlBoolean JXAPI_GetCoreAppVersion(char *versionString) 
{
   JavaVM* pVM ;   // Pointer to JVM if creating
   JNIEnv* pEnv ;  // Pointer to the JVM Environment / context
   UtlBoolean retval = FALSE;   // return value: was the version obtained?
   bool    bUsingExternalEnv = false;  // Are we using an external pEnv?


   const char *className      = "org/sipfoundry/sipxphone/sys/Shell";
   const char *methodName   = "getCoreAppVersion";

   int iPriority = jniGetCurrentPriority() ; 

   JNI_BEGIN_METHOD("JXAPI_GetCoreAppVersion") ;

   // Create or attach to a VM
   if (bUsingExternalEnv || jniGetVMReference(&pVM, &pEnv)) 
   {
      jclass    cls ;
      jmethodID methodid ;

      cls = pEnv->FindClass(className) ;
      if (cls != NULL) {
         methodid = pEnv->GetStaticMethodID(cls, methodName, "()Ljava/lang/String;") ;
         if (methodid != NULL) {            
            
            jobject jobjCallID = pEnv->CallStaticObjectMethod(cls, methodid) ;            
            if (jobjCallID != NULL) {
               char* szVersion = (jobjCallID) ? (char *)pEnv->GetStringUTFChars((jstring) jobjCallID, 0) : NULL ;
               if (szVersion)
               {
                  strcpy(versionString, szVersion);
                  pEnv->ReleaseStringUTFChars((jstring)   jobjCallID, szVersion) ;
               }
            }

            if (pEnv->ExceptionOccurred()) {
               pEnv->ExceptionDescribe() ;
               pEnv->ExceptionClear() ;
            }
            else
               retval = TRUE;

         } else {
            LOOKUP_FAILURE() ;
         }
      } else {
         LOOKUP_FAILURE() ;
      }
      if (!bUsingExternalEnv)
         jniReleaseVMReference(pVM) ;               
   }
   jniResetPriority(iPriority) ;

   JNI_END_METHOD() ;

   return retval;
}




extern "C" UtlBoolean JXAPI_GetJVMFreeMemory(unsigned long& ulFreeMemory)
{
   JavaVM* pVM ;   // Pointer to JVM if creating
   JNIEnv* pEnv ;  // Pointer to the JVM Environment / context
   UtlBoolean retval = FALSE;   // return value: was the version obtained?
   bool    bUsingExternalEnv = false;  // Are we using an external pEnv?


   const char *className      = "org/sipfoundry/sipxphone/sys/Shell";
   const char *methodName   = "getJVMFreeMemory";

   int iPriority = jniGetCurrentPriority() ; 

   JNI_BEGIN_METHOD("JXAPI_GetJVMFreeMemory") ;

   // Create or attach to a VM
   if (bUsingExternalEnv || jniGetVMReference(&pVM, &pEnv)) 
   {
      jclass    cls ;
      jmethodID methodid ;

      cls = pEnv->FindClass(className) ;
      if (cls != NULL) {
         methodid = pEnv->GetStaticMethodID(cls, methodName, "()J") ;
         if (methodid != NULL) {            
            
            ulFreeMemory = pEnv->CallStaticLongMethod(cls, methodid) ;            

            if (pEnv->ExceptionOccurred()) {
               pEnv->ExceptionDescribe() ;
               pEnv->ExceptionClear() ;
            }
            else
               retval = TRUE;
         } else {
            LOOKUP_FAILURE() ;
         }
      } else {
         LOOKUP_FAILURE() ;
      }
      if (!bUsingExternalEnv)
         jniReleaseVMReference(pVM) ;               
   }
   jniResetPriority(iPriority) ;

   JNI_END_METHOD() ;

   return retval;

}



extern "C" UtlBoolean JXAPI_GetJVMTotalMemory(unsigned long& ulTotalMemory)
{
   JavaVM* pVM ;   // Pointer to JVM if creating
   JNIEnv* pEnv ;  // Pointer to the JVM Environment / context
   UtlBoolean retval = FALSE;   // return value: was the version obtained?
   bool    bUsingExternalEnv = false;  // Are we using an external pEnv?


   const char *className      = "org/sipfoundry/sipxphone/sys/Shell";
   const char *methodName   = "getJVMTotalMemory";

   int iPriority = jniGetCurrentPriority() ; 

   JNI_BEGIN_METHOD("JXAPI_GetJVMTotalMemory") ;

   // Create or attach to a VM
   if (bUsingExternalEnv || jniGetVMReference(&pVM, &pEnv)) 
   {
      jclass    cls ;
      jmethodID methodid ;

      cls = pEnv->FindClass(className) ;
      if (cls != NULL) {
         methodid = pEnv->GetStaticMethodID(cls, methodName, "()J") ;
         if (methodid != NULL) {            
            
            ulTotalMemory = pEnv->CallStaticLongMethod(cls, methodid) ;            

            if (pEnv->ExceptionOccurred()) {
               pEnv->ExceptionDescribe() ;
               pEnv->ExceptionClear() ;
            }
            else
               retval = TRUE;

         } else {
            LOOKUP_FAILURE() ;
         }
      } else {
         LOOKUP_FAILURE() ;
      }
      if (!bUsingExternalEnv)
         jniReleaseVMReference(pVM) ;               
   }
   jniResetPriority(iPriority) ;

   JNI_END_METHOD() ;

   return retval;
}

extern "C" void JXAPI_FlushCache()
{
   JavaVM* pVM ;   // Pointer to JVM if creating
   JNIEnv* pEnv ;  // Pointer to the JVM Environment / context
   bool    bUsingExternalEnv = false;  // Are we using an external pEnv?


   const char *className      = "org/sipfoundry/sipxphone/sys/Shell";
   const char *methodName   = "flushCache";

   int iPriority = jniGetCurrentPriority() ; 

   JNI_BEGIN_METHOD("JXAPI_FlushCache") ;

   // Create or attach to a VM
   if (bUsingExternalEnv || jniGetVMReference(&pVM, &pEnv)) 
   {
      jclass    cls ;
      jmethodID methodid ;

      cls = pEnv->FindClass(className) ;
      if (cls != NULL) {
         methodid = pEnv->GetStaticMethodID(cls, methodName, "()V") ;
         if (methodid != NULL) {            
            
            pEnv->CallStaticVoidMethod(cls, methodid) ;            

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
      if (!bUsingExternalEnv)
         jniReleaseVMReference(pVM) ;               
   }
   jniResetPriority(iPriority) ;

   JNI_END_METHOD() ;

}



extern "C" UtlBoolean JXAPI_GetXpressaResource(const char* szResource, char** pData, int& iLength) 
{
   JavaVM* pVM ;   // Pointer to JVM if creating
   JNIEnv* pEnv ;  // Pointer to the JVM Environment / context
   UtlBoolean retval = FALSE;   // return value: was the version obtained?
   bool    bUsingExternalEnv = false;  // Are we using an external pEnv?


   const char *className    = "org/sipfoundry/sipxphone/sys/Shell";
   const char *methodName   = "getXpressaResource";

   int iPriority = jniGetCurrentPriority() ; 

   JNI_BEGIN_METHOD("JXAPI_GetXpressaResource") ;

   // Create or attach to a VM
   if (bUsingExternalEnv || jniGetVMReference(&pVM, &pEnv)) 
   {      jclass    cls ;
      jmethodID methodid ;

      cls = pEnv->FindClass(className) ;
      if (cls != NULL) 
        {
         methodid = pEnv->GetStaticMethodID(cls, methodName, "(Ljava/lang/String;)[B") ;
         if (methodid != NULL) 
            {
                jstring jsResource = CSTRTOJSTR(pEnv, szResource) ;
            jbyteArray jdata = (jbyteArray) pEnv->CallStaticObjectMethod(cls, methodid, jsResource) ;
            if (jdata != NULL) 
                {
                    jboolean bIsCopy = false ;
                    char* pContent = (char*) pEnv->GetByteArrayElements(jdata, &bIsCopy) ;
                   iLength = pEnv->GetArrayLength(jdata) ;

                    *pData = new char[iLength] ;
                    memcpy(*pData, pContent, iLength) ;

                    if (bIsCopy)
                      pEnv->ReleaseByteArrayElements(jdata, (signed char*) pContent, JNI_ABORT) ;               
            }                
            if (pEnv->ExceptionOccurred()) 
                {
               pEnv->ExceptionDescribe() ;
               pEnv->ExceptionClear() ;
            }
            else
               retval = TRUE;
         } 
            else 
            {
            LOOKUP_FAILURE() ;
         }
      } 
        else 
        {
         LOOKUP_FAILURE() ;
      }
      if (!bUsingExternalEnv)
         jniReleaseVMReference(pVM) ;               
   }
   jniResetPriority(iPriority) ;

   JNI_END_METHOD() ;

   return retval;
}


/*****************************************************************************
 */
extern "C" UtlBoolean JXAPI_RequestRestart(int iSeconds, const char* szReason)
{
   JavaVM* pVM ;       // Pointer to JVM if creating
   JNIEnv* pEnv ;      // Pointer to the JVM Environment / context
   UtlBoolean retval = TRUE;   // return value: should we restart?
   bool    bUsingExternalEnv = false;  // Are we using an external pEnv?


   const char *className   = "org/sipfoundry/sipxphone/sys/Shell";
   const char *methodName  = "requestRestart";

   int iPriority = jniGetCurrentPriority() ; 

   JNI_BEGIN_METHOD("JXAPI_requestRestart") ;

   // Create or attach to a VM
   if (bUsingExternalEnv || jniGetVMReference(&pVM, &pEnv)) 
   {
      jclass    cls ;
      jmethodID methodid ;

      cls = pEnv->FindClass(className) ;
      if (cls != NULL) 
      {
         jstring jsReason = CSTRTOJSTR(pEnv, szReason) ;
         methodid = pEnv->GetStaticMethodID(cls, methodName, "(ILjava/lang/String;)Z") ;
         if (methodid != NULL) 
         {            
            retval = pEnv->CallStaticBooleanMethod(cls, methodid, iSeconds, jsReason) ;
            if (pEnv->ExceptionOccurred()) 
            {
               pEnv->ExceptionDescribe() ;
               pEnv->ExceptionClear() ;
            }
         } 
         else 
         {
            LOOKUP_FAILURE() ;
         }
      } 
      else 
      {
         LOOKUP_FAILURE() ;
      }
      if (!bUsingExternalEnv)
         jniReleaseVMReference(pVM) ;               
   }
   jniResetPriority(iPriority) ;

   JNI_END_METHOD() ;

   return retval;
}
    

/*****************************************************************************
 */
extern "C" UtlBoolean JXAPI_IsPhoneBusy()
{
   JavaVM* pVM ;       // Pointer to JVM if creating
   JNIEnv* pEnv ;      // Pointer to the JVM Environment / context
   UtlBoolean retval = FALSE;   // return value: is the phone busy?
   bool    bUsingExternalEnv = false;  // Are we using an external pEnv?


   const char *className   = "org/sipfoundry/sipxphone/sys/Shell";
   const char *methodName  = "isPhoneBusy";

   int iPriority = jniGetCurrentPriority() ; 

   JNI_BEGIN_METHOD("JXAPI_isPhoneBusy") ;

   // Create or attach to a VM
   if (bUsingExternalEnv || jniGetVMReference(&pVM, &pEnv)) 
   {
      jclass    cls ;
      jmethodID methodid ;

      cls = pEnv->FindClass(className) ;
      if (cls != NULL) 
      {
         methodid = pEnv->GetStaticMethodID(cls, methodName, "()Z") ;
         if (methodid != NULL) 
         {            
            retval = pEnv->CallStaticBooleanMethod(cls, methodid) ;            

            if (pEnv->ExceptionOccurred()) 
            {
               pEnv->ExceptionDescribe() ;
               pEnv->ExceptionClear() ;
            }
         } 
         else 
         {
            LOOKUP_FAILURE() ;
         }
      } 
      else 
      {
         LOOKUP_FAILURE() ;
      }
      if (!bUsingExternalEnv)
         jniReleaseVMReference(pVM) ;               
   }
   jniResetPriority(iPriority) ;

   JNI_END_METHOD() ;

   return retval;
}
