// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/CallListenerHelpers.cpp#3 $
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
#include "pingerjni/CallListenerHelpers.h"

#include "os/OsLock.h"
#include "os/OsMutex.h"
#include "ptapi/PtAddress.h"
#include "ptapi/PtTerminalConnection.h"
#include "ptapi/PtTerminalConnectionEvent.h"
#include "ptapi/PtTerminalConnectionListener.h"
#include "ptapi/PtConnection.h"
#include "ptapi/PtConnectionListener.h"
#include "ptapi/PtConnectionEvent.h"
#include "ptapi/PtCall.h"
#include "ptapi/PtCallListener.h"
#include "ptapi/PtEvent.h"
#include "ptapi/PtMetaEvent.h"
#include "ptapi/PtMultiCallMetaEvent.h"
#include "ptapi/PtSingleCallMetaEvent.h"
#include "ptapi/PtCallEvent.h"
#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"

static UtlBoolean gInitializedJNICache = false ;
static jclass    gClassCache[CACHECID_MAX_INDEX] ;
static jmethodID gMethodIdCache[CACHEMID_MAX_INDEX] ;
static jfieldID  gFieldIdCache[CACHEFID_MAX_INDEX] ;
   

/*****************************************************************************
 */
void fireTerminalConnectionEvent(jobject jobjListener,
                                 const char*   szMethod,
                                 const PtTerminalConnectionEvent& rEvent)
{     
   int iPriority = jniGetCurrentPriority() ;
     
   JNI_BEGIN_CALLBACK("fireTerminalConnectionEvent", szMethod) ;
   
   if (jobjListener != NULL) 
   {
      JavaVM* pVM ;
      JNIEnv* pEnv ;
      
      if (jniGetVMReference(&pVM, &pEnv))
      {
         // What is the method ID of this method?
         jclass clazz = pEnv->GetObjectClass(jobjListener) ;
         if (clazz != NULL) 
         {
            jmethodID mid = pEnv->GetMethodID(clazz, szMethod, "(Ljavax/telephony/TerminalConnectionEvent;)V") ;
            if (mid != NULL) 
            {
               jobject objEvent = createTerminalConnectionEvent(rEvent, pEnv) ;               
               if (objEvent != NULL) 
               {
                  pEnv->CallVoidMethod(jobjListener, mid, objEvent) ;
                  if (pEnv->ExceptionOccurred()) 
                  {
                     pEnv->ExceptionDescribe() ;
                     pEnv->ExceptionClear() ;
                  }
                  pEnv->DeleteLocalRef(objEvent) ;
               } 
               else
                  LOOKUP_FAILURE() ;
                  
            } 
            else
               LOOKUP_FAILURE() ;

            pEnv->DeleteLocalRef(clazz) ;
         }
         jniReleaseVMReference(pVM) ;         
      }
   }

   jniResetPriority(iPriority) ;

   JNI_END_METHOD() ;
}


/*****************************************************************************
 */
void fireConnectionEvent(jobject jobjListener,
                         const char*   szMethod,
                         const   PtConnectionEvent& rEvent)
{
   int iPriority = jniGetCurrentPriority() ;

   JNI_BEGIN_CALLBACK("fireConnectionEvent", szMethod) ;

   if (jobjListener != NULL) 
   {
      JavaVM* pVM ;
      JNIEnv* pEnv ;
      
      if (jniGetVMReference(&pVM, &pEnv))
      {
         // What is the method ID of this method?
         jclass clazz = pEnv->GetObjectClass(jobjListener) ;
         if (clazz != NULL) 
         {
            jmethodID mid = pEnv->GetMethodID(clazz, szMethod, "(Ljavax/telephony/ConnectionEvent;)V") ;            
            if (mid != NULL) 
            {
               jobject objEvent = createConnectionEvent(rEvent, pEnv) ;
               if (objEvent != NULL) 
               {
                  pEnv->CallVoidMethod(jobjListener, mid, objEvent) ;
                  if (pEnv->ExceptionOccurred()) 
                  {
                     pEnv->ExceptionDescribe() ;
                     pEnv->ExceptionClear() ;
                  }
                  pEnv->DeleteLocalRef(objEvent) ;
               } 
               else
                  LOOKUP_FAILURE() ;
            } 
            else
               LOOKUP_FAILURE() ;

            pEnv->DeleteLocalRef(clazz) ;
         }      
         jniReleaseVMReference(pVM) ;
      }
   }

   jniResetPriority(iPriority) ;

   JNI_END_METHOD()
} 


/*****************************************************************************
 */
void fireCallEvent(jobject     jobjListener,
               const char*        szMethod,
               const PtCallEvent& rEvent)

{
   int iPriority = jniGetCurrentPriority() ;

   JNI_BEGIN_CALLBACK("fireCallEvent", szMethod) ;

   if (jobjListener != NULL) 
   {
      JavaVM* pVM ;
      JNIEnv* pEnv ;
      
      if (jniGetVMReference(&pVM, &pEnv))
      {
         // What is the method ID of this method?
         jclass clazz = pEnv->GetObjectClass(jobjListener) ;
         if (clazz != NULL) 
         {
            jmethodID mid = pEnv->GetMethodID(clazz, szMethod, "(Ljavax/telephony/CallEvent;)V") ;
            if (mid != NULL) 
            {
               jobject objEvent = createCallEvent(rEvent, pEnv) ;
               
               if (objEvent != NULL) 
               {
                  pEnv->CallVoidMethod(jobjListener, mid, objEvent) ;                  
                  if (pEnv->ExceptionOccurred()) 
                  {
                     pEnv->ExceptionDescribe() ;
                     pEnv->ExceptionClear() ;
                  }
                  pEnv->DeleteLocalRef(objEvent) ;
               } 
               else
                  LOOKUP_FAILURE() ;
            } 
            else
               LOOKUP_FAILURE() ;

            pEnv->DeleteLocalRef(clazz) ;
         }                  
         jniReleaseVMReference(pVM) ;
      }
   }
   jniResetPriority(iPriority) ;

   JNI_END_METHOD() ;
}


/*****************************************************************************
 */
void fireSingleCallMetaEvent(jobject jobjListener,
                             const char*   szMethod,
                             const PtSingleCallMetaEvent& rEvent)
{
   int iPriority = jniGetCurrentPriority() ;

   JNI_BEGIN_CALLBACK("fireSingleCallMetaEvent", szMethod) ;

   if (jobjListener != NULL) 
   {
      JavaVM* pVM ;
      JNIEnv* pEnv ;
      
      if (jniGetVMReference(&pVM, &pEnv))
      {
         // What is the method ID of this method?
         jclass clazz = pEnv->GetObjectClass(jobjListener) ;
         if (clazz != NULL) 
         {
            jmethodID mid = pEnv->GetMethodID(clazz, szMethod, "(Ljavax/telephony/MetaEvent;)V") ;
            if (mid != NULL) 
            {
               jobject objEvent = createSingleCallMetaEvent(rEvent, pEnv) ;               
               if (objEvent != NULL) 
               {
                  pEnv->CallVoidMethod(jobjListener, mid, objEvent) ;                  
                  if (pEnv->ExceptionOccurred()) 
                  {
                     pEnv->ExceptionDescribe() ;
                     pEnv->ExceptionClear() ;
                  }
                  pEnv->DeleteLocalRef(objEvent) ;
               } 
               else
                  LOOKUP_FAILURE() ;
            } 
            else
               LOOKUP_FAILURE() ;

            pEnv->DeleteLocalRef(clazz) ;
         }         
         jniReleaseVMReference(pVM) ;
      }
   }
   jniResetPriority(iPriority) ;

   JNI_END_METHOD() ;
}


/*****************************************************************************
 */
void fireMultiCallMetaEvent(jobject jobjListener,
                            const char*   szMethod,
                            const PtMultiCallMetaEvent& rEvent)
{
   int iPriority = jniGetCurrentPriority() ;

   JNI_BEGIN_CALLBACK("fireMultiCallMetaEvent", szMethod) ;

   if (jobjListener != NULL) 
   {
      JavaVM* pVM ;
      JNIEnv* pEnv ;
      
      if (jniGetVMReference(&pVM, &pEnv))
      {
         // What is the method ID of this method?
         jclass clazz = pEnv->GetObjectClass(jobjListener) ;
         if (clazz != NULL) 
         {
            jmethodID mid = pEnv->GetMethodID(clazz, szMethod, "(Ljavax/telephony/MetaEvent;)V") ;
            if (mid != NULL) 
            {
               jobject objEvent = createMultiCallMetaEvent(rEvent, pEnv) ;               
               if (objEvent != NULL) 
               {
                  pEnv->CallVoidMethod(jobjListener, mid, objEvent) ;                  
                  if (pEnv->ExceptionOccurred()) 
                  {
                     pEnv->ExceptionDescribe() ;
                     pEnv->ExceptionClear() ;
                  }
                  pEnv->DeleteLocalRef(objEvent) ;
               } 
               else
                  LOOKUP_FAILURE() ;
            } 
            else
               LOOKUP_FAILURE() ;
            pEnv->DeleteLocalRef(clazz) ;
         }
         jniReleaseVMReference(pVM) ;         
      }
   }
   jniResetPriority(iPriority) ;

   JNI_END_METHOD() ;
}


/*****************************************************************************
 */
jobject createTerminalConnectionEvent(const PtTerminalConnectionEvent& rEvent, JNIEnv* pEnv)
{  
   jobject objEvent = NULL ;

   // Continue only if we can contruct an event
   if ((gClassCache[CACHECID_TERM_CONN_EVENT] != NULL) && 
         (gMethodIdCache[CACHEMID_TERM_CONN_EVENT_INIT] != NULL))
   {
      PtEvent::PtEventCause enumCauseCode = PtEvent::CAUSE_UNKNOWN ;
      PtTerminalConnection* pTerminalConnection = new PtTerminalConnection() ;
      PtCall*               pCall = new PtCall() ;
      int                   iLocal = rEvent.isLocal() ;
      int                   iResponseCode = PtEvent::EVENT_INVALID ;
      UtlString              strResponseText ;
      PtStatus              status ;
      UtlString              jni_terminal_name ;
      UtlBoolean             bIsLocal ;

      // Get PTAPI call object
      status = ((PtTerminalConnectionEvent &) rEvent).getCall(*pCall) ;
      if (status != 0)
         API_FAILURE(status) ;
      JNI_DUMP_CALL_CALLID(pCall) ; 
      
      // Get PTAPI connection object
      status = ((PtTerminalConnectionEvent &) rEvent).getTerminalConnection(*pTerminalConnection) ;
      if (status != 0)   
         API_FAILURE(status) ;            
      JNI_DUMP_TC_NAME(pTerminalConnection) ;

      // Get PTAPI cause code
      status = ((PtTerminalConnectionEvent &) rEvent).getCause(enumCauseCode) ;
      if (status != 0)
         API_FAILURE(status) ;         
      JNI_DUMP_CAUSECODE(enumCauseCode) ;

      // Get Terminal Name
      pTerminalConnection->getTerminalName(jni_terminal_name) ;
      pTerminalConnection->isLocal(bIsLocal) ;

      // Get SIP Response Code
      status = ((PtConnectionEvent &) rEvent).getSipResponseCode(iResponseCode, strResponseText) ;
      if (status != 0)
         API_FAILURE(status) ;

      objEvent = pEnv->NewObject(
            gClassCache[CACHECID_TERM_CONN_EVENT], 
            gMethodIdCache[CACHEMID_TERM_CONN_EVENT_INIT], 
            (jlong) (unsigned int) pCall, (jlong) (unsigned int) pTerminalConnection, (jint) iLocal) ;

      // Set Data Members
      if (objEvent != NULL)  
      {
         if (gFieldIdCache[CACHEFID_CONN_EVENT_CAUSE] > 0)
         {
            pEnv->SetIntField(objEvent, 
                  gFieldIdCache[CACHEFID_CONN_EVENT_CAUSE], 
                  (jint) enumCauseCode) ;
         }                 

         if (gFieldIdCache[CACHEFID_CONN_EVENT_SIP_CODE] > 0)
         {
            pEnv->SetIntField(objEvent, 
                  gFieldIdCache[CACHEFID_CONN_EVENT_SIP_CODE], 
                  (jint) iResponseCode) ;
         }

         if (gFieldIdCache[CACHEFID_CONN_EVENT_SIP_TEXT] > 0)
         {
            jobject jstrResponseText = 
               pEnv->NewStringUTF(strResponseText.data()) ;
            pEnv->SetObjectField(objEvent, 
                  gFieldIdCache[CACHEFID_CONN_EVENT_SIP_TEXT], 
                  jstrResponseText);
            if( jstrResponseText != NULL )
               pEnv->DeleteLocalRef(jstrResponseText) ;
         }

         if (gFieldIdCache[CACHEFID_CONN_EVENT_META] > 0)
         {
            jobject jobjMetaEvent = createMetaEvent(rEvent, pEnv) ;
            pEnv->SetObjectField(objEvent, 
                  gFieldIdCache[CACHEFID_CONN_EVENT_META], 
                  jobjMetaEvent) ;
            pEnv->DeleteLocalRef(jobjMetaEvent) ;
         }         
      }   
   }

   return objEvent ;
}


/*****************************************************************************
 */
jobject createConnectionEvent(const PtConnectionEvent& rEvent, JNIEnv* pEnv)
{   
   jobject objEvent = NULL ;

   // Continue only if we can contruct an event
   if ((gClassCache[CACHECID_CONN_EVENT] != NULL) && 
         (gMethodIdCache[CACHEMID_CONN_EVENT_INIT] != NULL))
   {
      PtEvent::PtEventCause enumCauseCode = PtEvent::CAUSE_UNKNOWN ;
      PtConnection*         pConnection = new PtConnection() ;
      PtCall*               pCall = new PtCall() ;
      int                   iLocal = rEvent.isLocal() ;
      int                   iResponseCode  = PtEvent::EVENT_INVALID ; 
      UtlString              strResponseText ;
      PtStatus              status ;

      // Get PTAPI call object
      status = ((PtConnectionEvent &) rEvent).getCall(*pCall) ;
      if (status != 0)
         API_FAILURE(status) ;

      JNI_DUMP_CALL_CALLID(pCall) ; 

      // Get PTAPI connection object
      status = ((PtConnectionEvent &) rEvent).getConnection(*pConnection) ;
      if (status != 0) 
         API_FAILURE(status) ;

      JNI_DUMP_CONNECTION_ADDRESS(pConnection) ;

      // Get PTAPI cause code
      status = ((PtConnectionEvent &) rEvent).getCause(enumCauseCode) ;
      if (status != 0)
         API_FAILURE(status) ;
      JNI_DUMP_CAUSECODE(enumCauseCode) ;

      // Get SIP Response Code
      status = ((PtConnectionEvent &) rEvent).getSipResponseCode(iResponseCode, strResponseText) ;
      if (status != 0)
         API_FAILURE(status) ;      

      // Create the new object
      objEvent = pEnv->NewObject(
            gClassCache[CACHECID_CONN_EVENT], 
            gMethodIdCache[CACHEMID_CONN_EVENT_INIT], 
            (jlong) (unsigned int) pCall, (jlong) (unsigned int) pConnection, (jint) iLocal) ;


      // Set Data Members
      if (objEvent != NULL)  
      {
         if (gFieldIdCache[CACHEFID_CONN_EVENT_CAUSE] > 0)
         {
            pEnv->SetIntField(objEvent, 
                  gFieldIdCache[CACHEFID_CONN_EVENT_CAUSE], 
                  (jint) enumCauseCode) ;
         }                 

         if (gFieldIdCache[CACHEFID_CONN_EVENT_SIP_CODE] > 0)
         {
            pEnv->SetIntField(objEvent, 
                  gFieldIdCache[CACHEFID_CONN_EVENT_SIP_CODE], 
                  (jint) iResponseCode) ;
         }

         if (gFieldIdCache[CACHEFID_CONN_EVENT_SIP_TEXT] > 0)
         {
            jobject jstrResponseText = 
                  pEnv->NewStringUTF(strResponseText.data());
            pEnv->SetObjectField(objEvent, 
                  gFieldIdCache[CACHEFID_CONN_EVENT_SIP_TEXT], 
                  jstrResponseText) ;
            if( jstrResponseText != NULL )
               pEnv->DeleteLocalRef(jstrResponseText) ;

         }

         if (gFieldIdCache[CACHEFID_CONN_EVENT_META] > 0)
         {
            jobject jobjMetaEvent = createMetaEvent(rEvent, pEnv) ;
            pEnv->SetObjectField(objEvent, 
                  gFieldIdCache[CACHEFID_CONN_EVENT_META], 
                  jobjMetaEvent) ;
            pEnv->DeleteLocalRef(jobjMetaEvent) ;
         }         
      }
   }            
   return objEvent ;
}


/*****************************************************************************
 */
jobject createCallEvent(const PtCallEvent& rEvent, JNIEnv* pEnv)
{   
   jobject objEvent = NULL ;

   // Continue only if we can contruct an event
   if ((gClassCache[CACHECID_CALL_EVENT] != NULL) && 
         (gMethodIdCache[CACHEMID_CALL_EVENT_INIT] != NULL))
   {
      PtEvent::PtEventCause enumCauseCode = PtEvent::CAUSE_UNKNOWN ;
      PtCall*               pCall = new PtCall();
      int                   iLocal = rEvent.isLocal() ;
      int                   iResponseCode  = PtEvent::EVENT_INVALID ; 
      UtlString              strResponseText;
      PtStatus              status ;

      // Get PTAPI call object
      status = ((PtCallEvent &) rEvent).getCall(*pCall) ;
      if (status != 0)
         OsSysLog::add(FAC_JNI, PRI_ERR, "createCallEvent::getCall returned %d", status) ;
      JNI_DUMP_CALL_CALLID(pCall) ; 

      // Get PTAPI cause code
      status = ((PtCallEvent &) rEvent).getCause(enumCauseCode) ;
      if (status != 0)
         OsSysLog::add(FAC_JNI, PRI_ERR, "createCallEvent::getCause returned %d", status) ;
      JNI_DUMP_CAUSECODE(enumCauseCode) ;

      // Get SIP Response Code
      status = ((PtConnectionEvent &) rEvent).getSipResponseCode(iResponseCode, strResponseText) ;
      if (status != 0)
         API_FAILURE(status) ;

      // Create the java event object
      objEvent = pEnv->NewObject(
         gClassCache[CACHECID_CALL_EVENT], 
         gMethodIdCache[CACHEMID_CALL_EVENT_INIT], 
         (jlong) (unsigned int) pCall, (jlong) (unsigned int) iLocal) ;

      // Set Data Members
      if (objEvent != NULL)  
      {
         if (gFieldIdCache[CACHEFID_CONN_EVENT_CAUSE] > 0)
         {
            pEnv->SetIntField(objEvent, 
                  gFieldIdCache[CACHEFID_CONN_EVENT_CAUSE], 
                  (jint) enumCauseCode) ;
         }                 

         if (gFieldIdCache[CACHEFID_CONN_EVENT_SIP_CODE] > 0)
         {
            pEnv->SetIntField(objEvent, 
                  gFieldIdCache[CACHEFID_CONN_EVENT_SIP_CODE], 
                  (jint) iResponseCode) ;
         }

         if (gFieldIdCache[CACHEFID_CONN_EVENT_SIP_TEXT] > 0)
         {
            jobject jstrResponseText = 
               pEnv->NewStringUTF(strResponseText.data()) ;
            pEnv->SetObjectField(objEvent, 
                  gFieldIdCache[CACHEFID_CONN_EVENT_SIP_TEXT], 
                  jstrResponseText);
            if( jstrResponseText != NULL )
               pEnv->DeleteLocalRef(jstrResponseText) ;

         }

         if (gFieldIdCache[CACHEFID_CONN_EVENT_META] > 0)
         {
            jobject jobjMetaEvent = createMetaEvent(rEvent, pEnv) ;
            pEnv->SetObjectField(objEvent, 
                  gFieldIdCache[CACHEFID_CONN_EVENT_META], 
                  jobjMetaEvent) ;
            pEnv->DeleteLocalRef(jobjMetaEvent) ;
         }         
      }
   }
   return objEvent ;
}


/*****************************************************************************
 */
jobject createMetaEvent(const PtEvent &rEvent, JNIEnv* pEnv)
{
    jobject jobjRC = NULL ;
    PtBoolean bContainsMetaEvent = false ;
    PtMetaEvent *pMetaEvent = NULL ;
   
    rEvent.getMetaEvent(bContainsMetaEvent, pMetaEvent) ;
    if (((UtlBoolean)bContainsMetaEvent) == TRUE)
    {
        if (pMetaEvent->isInstanceOf("PtSingleCallMetaEvent"))
        {
            PtSingleCallMetaEvent* pSingleCallMetaEvent = 
                    (PtSingleCallMetaEvent*) pMetaEvent ;

            jobjRC = createSingleCallMetaEvent(*pSingleCallMetaEvent, pEnv) ;
        }
        else if (pMetaEvent->isInstanceOf("PtMultiCallMetaEvent"))
        {
            PtMultiCallMetaEvent* pMultiCallMetaEvent = 
                    (PtMultiCallMetaEvent*) pMetaEvent ;

            jobjRC = createMultiCallMetaEvent(*pMultiCallMetaEvent, pEnv) ;
        }
        else
        {
            OsSysLog::add(FAC_JNI, PRI_ERR, "Unknown event passed to createMetaEvent: %s", (char*) pMetaEvent->className) ;
        }

        if (jobjRC != NULL)
        {
            if (gFieldIdCache[CACHEFID_CONN_EVENT_CAUSE] > 0)
            {
                int iID = -1 ;               
                pMetaEvent->getMetaCode(iID) ;
                pEnv->SetIntField(jobjRC, 
                        gFieldIdCache[CACHEFID_META_EVENT_ID], 
                        (jint) iID) ;
            }                 
        }
    }

    if (pMetaEvent) 
        delete pMetaEvent;

    return jobjRC ;
}

/*****************************************************************************
 */
jobject createSingleCallMetaEvent(const PtSingleCallMetaEvent& rSingleCallMetaEvent, JNIEnv* pEnv)
{
   jobject objEvent = NULL ;

   // Continue only if we can contruct an event
   if ((gClassCache[CACHECID_SINGLE_CALL_META_EVENT] != NULL) && 
         (gMethodIdCache[CACHEMID_SINGLE_CALL_META_EVENT_INIT] != NULL))
   {
      PtCall* pCall = new PtCall() ;
      PtStatus status = rSingleCallMetaEvent.getCall(*pCall) ;
      if (status != 0)
         API_FAILURE(status) ;

      objEvent = pEnv->NewObject(
            gClassCache[CACHECID_SINGLE_CALL_META_EVENT], 
            gMethodIdCache[CACHEMID_SINGLE_CALL_META_EVENT_INIT], 
            (jlong) (unsigned int) pCall) ;
   }   
   return objEvent ;
}



/*****************************************************************************
 */
#define MAX_MULTI_CALL_CALLS  4
jobject createMultiCallMetaEvent(const PtMultiCallMetaEvent& rMultiCallMetaEvent, JNIEnv* pEnv)
{
   jobject objEvent = NULL ;

   // Continue only if we can contruct an event
   if ((gClassCache[CACHECID_MULTI_CALL_META_EVENT] != NULL) && 
         (gMethodIdCache[CACHEMID_MULTI_CALL_META_EVENT_INIT] != NULL))
   {
      PtCall* pCall = new PtCall() ;
      PtStatus status = rMultiCallMetaEvent.getNewCall(*pCall) ;
      if (status != 0)
         API_FAILURE(status) ;

      PtCall calls[MAX_MULTI_CALL_CALLS] ;
      jlong longs[MAX_MULTI_CALL_CALLS] ;
      int nItems = 0 ;
      status = rMultiCallMetaEvent.getOldCalls(calls, MAX_MULTI_CALL_CALLS, nItems) ;
      if (status != 0)
         API_FAILURE(status) ;

      for (int i=0; i<nItems; i++) 
      {
         PtCall* pTmpCall = new PtCall() ;
         *pTmpCall = calls[i] ;
         longs[i] = (jlong) (unsigned int) pTmpCall ;
      }

      jlongArray jlaCalls = pEnv->NewLongArray(nItems) ;
      pEnv->SetLongArrayRegion(jlaCalls, 0, nItems, longs) ;

      // Create the java event object
      objEvent = pEnv->NewObject(
            gClassCache[CACHECID_MULTI_CALL_META_EVENT], 
            gMethodIdCache[CACHEMID_MULTI_CALL_META_EVENT_INIT], 
            (jlong) (unsigned int) pCall, jlaCalls) ;

      pEnv->DeleteLocalRef(jlaCalls) ;
   }   

   return objEvent ;
}


//: Initialize class/field/method cache for call events
void initializeJNICaches(JNIEnv* pEnv) 
{
   jclass clazz ;
   int    i ;

   if (gInitializedJNICache == FALSE)
   {
      gInitializedJNICache = true ;
            
      /**
       ** Load Classes
       **/

      for (i=0; i<CACHECID_MAX_INDEX; i++)
      {
         gClassCache[i] = 0 ;        
      }


      // CACHECID_TERM_CONN_EVENT
      clazz = pEnv->FindClass(
            "org/sipfoundry/telephony/PtTerminalConnectionEvent") ;
      if (clazz > 0)
      {
         gClassCache[CACHECID_TERM_CONN_EVENT] = (jclass) 
               pEnv->NewGlobalRef(clazz) ;
         pEnv->DeleteLocalRef(clazz) ;
      }
      else
      {
         LOOKUP_FAILURE() ;
      }      

      // CACHECID_CONN_EVENT
      clazz = pEnv->FindClass(
            "org/sipfoundry/telephony/PtConnectionEvent") ;
      if (clazz > 0)
      {
         gClassCache[CACHECID_CONN_EVENT] = (jclass) 
               pEnv->NewGlobalRef(clazz) ;
         pEnv->DeleteLocalRef(clazz) ;
      }
      else
      {
         LOOKUP_FAILURE() ;
      }      

      // CACHECID_CALL_EVENT
      clazz = pEnv->FindClass(
            "org/sipfoundry/telephony/PtCallEvent") ;
      if (clazz > 0)
      {
         gClassCache[CACHECID_CALL_EVENT] = (jclass) 
               pEnv->NewGlobalRef(clazz) ;
         pEnv->DeleteLocalRef(clazz) ;
      }
      else
      {
         LOOKUP_FAILURE() ;
      }

      // CACHECID_SINGLE_CALL_META_EVENT
      clazz = pEnv->FindClass(
            "org/sipfoundry/telephony/PtSingleCallMetaEvent") ;
      if (clazz > 0)
      {
         gClassCache[CACHECID_SINGLE_CALL_META_EVENT] = (jclass) 
               pEnv->NewGlobalRef(clazz) ;
         pEnv->DeleteLocalRef(clazz) ;
      }
      else
      {
         LOOKUP_FAILURE() ;
      }
      


      // CACHECID_META_EVENT
      clazz = pEnv->FindClass(
            "org/sipfoundry/telephony/PtMetaEvent") ;
      if (clazz > 0)
      {
         gClassCache[CACHECID_META_EVENT] = (jclass) 
               pEnv->NewGlobalRef(clazz) ;
         pEnv->DeleteLocalRef(clazz) ;
      }
      else
      {
         LOOKUP_FAILURE() ;
      }
      



      // CACHECID_MULTI_CALL_META_EVENT
      clazz = pEnv->FindClass(
            "org/sipfoundry/telephony/PtMultiCallMetaEvent") ;
      if (clazz > 0)
      {
         gClassCache[CACHECID_MULTI_CALL_META_EVENT] = (jclass) 
               pEnv->NewGlobalRef(clazz) ;
         pEnv->DeleteLocalRef(clazz) ;
      }
      else
      {
         LOOKUP_FAILURE() ;
      }

      /**
       ** Load Methods
       **/

      for (i=0; i<CACHEMID_MAX_INDEX; i++)
      {
         gMethodIdCache[i] = 0 ;        
      }

      // CACHEMID_TERM_CONN_EVENT_INIT
      if (gClassCache[CACHECID_TERM_CONN_EVENT] != 0)
      {
         gMethodIdCache[CACHEMID_TERM_CONN_EVENT_INIT] = pEnv->GetMethodID(
               gClassCache[CACHECID_TERM_CONN_EVENT], 
               "<init>", 
               "(JJI)V") ;
         if (gMethodIdCache[CACHEMID_TERM_CONN_EVENT_INIT] <= 0)
            LOOKUP_FAILURE() ;
      }

      // CACHEMID_CONN_EVENT_INIT
      if (gClassCache[CACHECID_CONN_EVENT] != 0)
      {
         gMethodIdCache[CACHEMID_CONN_EVENT_INIT] = pEnv->GetMethodID(
               gClassCache[CACHECID_CONN_EVENT], 
               "<init>", 
               "(JJI)V") ;
         if (gMethodIdCache[CACHEMID_CONN_EVENT_INIT] <= 0)
            LOOKUP_FAILURE() ;
      }

      // CACHEMID_CALL_EVENT_INIT
      if (gClassCache[CACHECID_CALL_EVENT] != 0)
      {
         gMethodIdCache[CACHEMID_CALL_EVENT_INIT] = pEnv->GetMethodID(
               gClassCache[CACHECID_CALL_EVENT], 
               "<init>", 
               "(JI)V") ;
         if (gMethodIdCache[CACHEMID_CALL_EVENT_INIT] <= 0)
            LOOKUP_FAILURE() ;
      }

      // CACHEMID_SINGLE_CALL_EVENT_INIT
      if (gClassCache[CACHECID_SINGLE_CALL_META_EVENT] != 0)
      {
         gMethodIdCache[CACHEMID_SINGLE_CALL_META_EVENT_INIT] = pEnv->GetMethodID(
               gClassCache[CACHECID_SINGLE_CALL_META_EVENT], 
               "<init>", 
               "(J)V") ;
         if (gMethodIdCache[CACHEMID_SINGLE_CALL_META_EVENT_INIT] <= 0)
            LOOKUP_FAILURE() ;
      }

      // CACHEMID_MULTI_CALL_EVENT_INIT
      if (gClassCache[CACHECID_MULTI_CALL_META_EVENT] != 0)
      {
         gMethodIdCache[CACHEMID_MULTI_CALL_META_EVENT_INIT] = pEnv->GetMethodID(
               gClassCache[CACHECID_MULTI_CALL_META_EVENT], 
               "<init>", 
               "(J[J)V") ;
         if (gMethodIdCache[CACHEMID_MULTI_CALL_META_EVENT_INIT] <= 0)
            LOOKUP_FAILURE() ;
      }


      /**
       ** Load Fields
       **/

      for (i=0; i<CACHEFID_MAX_INDEX; i++)
      {
         gFieldIdCache[i] = 0 ;        
      }

      if (gClassCache[CACHECID_TERM_CONN_EVENT] > 0)
      {
         // CACHEFID_TERM_CONN_EVENT_CAUSE
         gFieldIdCache[CACHEFID_TERM_CONN_EVENT_CAUSE] = pEnv->GetFieldID(      
               gClassCache[CACHECID_TERM_CONN_EVENT], 
               "m_iCause", 
               "I") ;
         if (gFieldIdCache[CACHEFID_TERM_CONN_EVENT_CAUSE] <= 0)
            LOOKUP_FAILURE() ;

         // CACHEFID_TERM_CONN_EVENT_SIP_CODE
         gFieldIdCache[CACHEFID_TERM_CONN_EVENT_SIP_CODE] = pEnv->GetFieldID(      
               gClassCache[CACHECID_TERM_CONN_EVENT], 
               "m_iResponseCode", 
               "I") ;
         if (gFieldIdCache[CACHEFID_TERM_CONN_EVENT_SIP_CODE] <= 0)
            LOOKUP_FAILURE() ;

         // CACHEFID_TERM_CONN_EVENT_SIP_TEXT
         gFieldIdCache[CACHEFID_TERM_CONN_EVENT_SIP_TEXT] = pEnv->GetFieldID(      
               gClassCache[CACHECID_TERM_CONN_EVENT], 
               "m_strResponseText",
               "Ljava/lang/String;") ;
         if (gFieldIdCache[CACHEFID_TERM_CONN_EVENT_SIP_TEXT] <= 0)
            LOOKUP_FAILURE() ;

         // CACHEFID_TERM_CONN_EVENT_META
         gFieldIdCache[CACHEFID_TERM_CONN_EVENT_META] = pEnv->GetFieldID(      
               gClassCache[CACHECID_TERM_CONN_EVENT], 
               "m_metaEvent", 
               "Lorg/sipfoundry/telephony/PtMetaEvent;") ;               
         if (gFieldIdCache[CACHEFID_TERM_CONN_EVENT_META] <= 0)
            LOOKUP_FAILURE() ;
      }



      if (gClassCache[CACHECID_CONN_EVENT] > 0)
      {
         // CACHEFID_CONN_EVENT_CAUSE
         gFieldIdCache[CACHEFID_CONN_EVENT_CAUSE] = pEnv->GetFieldID(      
               gClassCache[CACHECID_CONN_EVENT], 
               "m_iCause", 
               "I") ;
         if (gFieldIdCache[CACHEFID_CONN_EVENT_CAUSE] <= 0)
            LOOKUP_FAILURE() ;

         // CACHEFID_CONN_EVENT_SIP_CODE
         gFieldIdCache[CACHEFID_CONN_EVENT_SIP_CODE] = pEnv->GetFieldID(      
               gClassCache[CACHECID_CONN_EVENT], 
               "m_iResponseCode", 
               "I") ;
         if (gFieldIdCache[CACHEFID_CONN_EVENT_SIP_CODE] <= 0)
            LOOKUP_FAILURE() ;

         // CACHEFID_CONN_EVENT_SIP_TEXT
         gFieldIdCache[CACHEFID_CONN_EVENT_SIP_TEXT] = pEnv->GetFieldID(      
               gClassCache[CACHECID_CONN_EVENT], 
               "m_strResponseText",
               "Ljava/lang/String;") ;
         if (gFieldIdCache[CACHEFID_CONN_EVENT_SIP_TEXT] <= 0)
            LOOKUP_FAILURE() ;

         // CACHEFID_CONN_EVENT_META
         gFieldIdCache[CACHEFID_CONN_EVENT_META] = pEnv->GetFieldID(      
               gClassCache[CACHECID_CONN_EVENT], 
               "m_metaEvent", 
               "Lorg/sipfoundry/telephony/PtMetaEvent;") ;               
         if (gFieldIdCache[CACHEFID_CONN_EVENT_META] <= 0)
            LOOKUP_FAILURE() ;
      }



      if (gClassCache[CACHECID_CALL_EVENT] > 0)
      {
         // CACHEFID_CALL_EVENT_CAUSE
         gFieldIdCache[CACHEFID_CALL_EVENT_CAUSE] = pEnv->GetFieldID(      
               gClassCache[CACHECID_CALL_EVENT], 
               "m_iCause", 
               "I") ;
         if (gFieldIdCache[CACHEFID_CALL_EVENT_CAUSE] <= 0)
            LOOKUP_FAILURE() ;

         // CACHEFID_CALL_EVENT_SIP_CODE
         gFieldIdCache[CACHEFID_CALL_EVENT_SIP_CODE] = pEnv->GetFieldID(      
               gClassCache[CACHECID_CALL_EVENT], 
               "m_iResponseCode", 
               "I") ;
         if (gFieldIdCache[CACHEFID_CALL_EVENT_SIP_CODE] <= 0)
            LOOKUP_FAILURE() ;

         // CACHEFID_CALL_EVENT_SIP_TEXT
         gFieldIdCache[CACHEFID_CALL_EVENT_SIP_TEXT] = pEnv->GetFieldID(      
               gClassCache[CACHECID_CALL_EVENT], 
               "m_strResponseText",
               "Ljava/lang/String;") ;
         if (gFieldIdCache[CACHEFID_CALL_EVENT_SIP_TEXT] <= 0)
            LOOKUP_FAILURE() ;

         // CACHEFID_CALL_EVENT_META
         gFieldIdCache[CACHEFID_CALL_EVENT_META] = pEnv->GetFieldID(      
               gClassCache[CACHECID_CALL_EVENT], 
               "m_metaEvent", 
               "Lorg/sipfoundry/telephony/PtMetaEvent;") ;               
         if (gFieldIdCache[CACHEFID_CALL_EVENT_META] <= 0)
            LOOKUP_FAILURE() ;


         // CACHEFID_META_EVENT_ID
         gFieldIdCache[CACHEFID_META_EVENT_ID] = pEnv->GetFieldID(      
               gClassCache[CACHECID_META_EVENT], 
               "m_iID", 
               "I") ;               
         if (gFieldIdCache[CACHEFID_META_EVENT_ID] <= 0)
            LOOKUP_FAILURE() ;

      }
   }
}
   

